/**
 * \file
 *
 * \brief main file
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel
 * Support</a>
 */

/*- Includes ---------------------------------------------------------------*/
#include <asf.h>
#include "platform.h"
#include "at_ble_api.h"
#include "console_serial.h"
#include "timer_hw.h"
#include "conf_serialdrv.h"
#include "conf_board.h"
#include "wearable.h"
#include "touch_api_ptc.h"
#include "touch_app.h"
#include "rtc.h"
#include "bme280\bme280_support.h"
#include "conf_sensor.h"
#include "veml60xx\veml60xx.h"
#include "bhi160.h"
#include "bhi160\bhy_uc_driver.h"
#include "i2c.h"
#include "serial_drv.h"
#include "adc_measure.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"
#include "main.h"
#include "led.h"
#include "env_sensor.h"
#include "motion_sensor.h"
#include "button.h"
#include "nvm_handle.h"
#include "winc15x0.h"
#include "usb_hid.h"
#include "ecc_provisioning_task.h"

/* enum variable */
enum status_code veml60xx_sensor_status = STATUS_ERR_NOT_INITIALIZED;

/* variables */
BME280_RETURN_FUNCTION_TYPE bme280_sensor_status = ERROR;
	
/* function prototypes */
void configure_wdt(void);

void enable_gclk1(void);

/** SysTick counter to avoid busy wait delay. */
//volatile uint32_t ms_ticks = 0;



/* Watchdog configuration */
void configure_wdt(void)
{
	/* Create a new configuration structure for the Watchdog settings and fill
	* with the default module settings. */
	struct wdt_conf config_wdt;
	wdt_get_config_defaults(&config_wdt);
	/* Set the Watchdog configuration settings */
	config_wdt.always_on = false;
	//config_wdt.clock_source = GCLK_GENERATOR_4;
	config_wdt.timeout_period = WDT_PERIOD_2048CLK;
	/* Initialize and enable the Watchdog with the user settings */
	wdt_set_config(&config_wdt);
}



void enable_gclk1(void)
{
	struct system_gclk_gen_config gclk_conf;
	
	system_gclk_init();	
	gclk_conf.high_when_disabled = false;
	gclk_conf.source_clock       = GCLK_SOURCE_OSC16M;
	gclk_conf.division_factor = 1;
	gclk_conf.run_in_standby  = true;
	gclk_conf.output_enable   = false;
	system_gclk_gen_set_config(2, &gclk_conf);
	system_gclk_gen_enable(2);
}





/**
 * \brief SysTick handler used to measure precise delay. 
 */
//void SysTick_Handler(void)
//{
//	ms_ticks++;
//	printf("DBG log1\r\n");
//}


void initialise_gpio(void)
{

	
	/* led port pin initialization */
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(PIN_PB22, &config_port_pin);
	port_pin_set_config(PIN_PB23, &config_port_pin);

	port_pin_set_config(PIN_PA17, &config_port_pin);
	port_pin_set_config(PIN_PA20, &config_port_pin);
	port_pin_set_config(PIN_PA21, &config_port_pin);
	port_pin_set_output_level(PIN_PB22, 0);
	port_pin_set_output_level(PIN_PB23, 0);
	port_pin_set_output_level(PIN_PA17, 0);
	port_pin_set_output_level(PIN_PA20, 1);
	port_pin_set_output_level(PIN_PA21, 0);
}

/* main function */
int main(void)
{

	/* Initialize RTC */
	rtc_init();
	
	/* initialize LED */
	initialise_led();
	
	/* initialize LED */
	initialise_gpio();
	
	led_ctrl_set_color(LED_COLOR_BLUE, LED_MODE_BLINK_NORMAL);	
		
	/* system clock initialization */
	system_init();

	//i2c configure
	configure_sensor_i2c();
	//Initialize BHI160
	bhy_driver_init(_bhi_fw, _bhi_fw_len);
	
	/* delay routine initialization */
	delay_init();

	/* configure adc for battery measurement */
	configure_adc();
 
#ifdef DEBUG_SUPPORT
	/* Initialize serial console for debugging */
	serial_console_init();
#endif

	
	DBG_LOG("Initializing Wearable Demo Device");
	DBG_LOG("cpu_freq=%d\n",(int)system_cpu_clock_get_hz());
	DBG_LOG("Firmware version: %s.%s", FIRMWARE_MAJOR_VER, FIRMWARE_MINOR_VER);
	
	/* Initialize the BSP. */
	nm_bsp_init();
	
	nvm_init();
	
	initialise_button();
	
	buttonInitCheck();

	/* Hardware timer */
	hw_timer_init();
	//Initialize bme280
	wearable_bme280_init();
	//Initialize veml60xx
	veml60xx_init();	

#ifdef AWS_JITR
	// Initialize the USB HID interface
	usb_hid_init();

	ecc_provisioning_task();
#endif
	
#ifdef ECC508 
	while (1) {
		zero_touch_provisioning_task();
			led_ctrl_set_color(LED_COLOR_YELLOW, LED_MODE_BLINK_NORMAL);	
		//break;
	}
#endif

	wifiInit();

	env_sensor_data_init();
	enable_rotation_vector();
	while (1) {
                zero_touch_provisioning_task();
		/* Handle pending events from network controller. */
		wifiTaskExecute();
		if(tick_rotation_data)
		{
			tick_rotation_data = 0;
			if (getWiFiMode()==APP_STA)
				motion_sensor_execute();
		}
		if(tick_2second == 1)
		{
			tick_2second = 0;
			if (getWiFiMode()==APP_STA && getWiFiStates() > WIFI_TASK_MQTT_SUBSCRIBE)
				env_sensor_execute();
			
		}
		//led_ctrl_execute();
		
		
	}


}


