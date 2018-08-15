/**
* \file
*
* \brief Functions used to configure and detect the button press
*
* Copyright (c) 2016 Atmel Corporation. All rights reserved.
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
*    Atmel micro controller product.
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
*Support</a>
*/


#include <asf.h>
#include "stdio.h"
#include "string.h"
#include "button.h"
//#include "nvm_api.h"
#include "winc15x0.h"
#include "conf_board.h"
#include "main.h"
#include "nvm_handle.h"
#include "led.h"

extern wifi_nvm_data_t wifi_nvm_data;
extern uint8 gMacaddr[M2M_MAC_ADDRES_LEN];
extern uint8 gDefaultSSID[M2M_MAX_SSID_LEN];
extern uint8 gAuthType;
extern uint8 gDefaultKey[M2M_MAX_PSK_LEN];
extern uint8 gUuid[AWS_COGNITO_UUID_LEN];


void initialise_button(void)
{
	
	/* Set buttons as inputs */
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_DOWN;
	port_pin_set_config(SW1_PIN, &config_port_pin);
	port_pin_set_config(SW2_PIN, &config_port_pin);
	port_pin_set_config(SW3_PIN, &config_port_pin);

}

void buttonInitCheck()
{
	
	if(SW2_ACTIVE == port_pin_get_input_level(SW2_PIN)){	// Enter WINC1500 FW programming mode
		
		led_ctrl_set_color(LED_COLOR_GREEN, LED_MODE_BLINK_NORMAL);
		while(1) {
			
		}
	}
	
	if(SW1_ACTIVE == port_pin_get_input_level(SW1_PIN)){
		setWiFiStates(WIFI_TASK_SWITCH_TO_AP);
		printf("Set as AP mode\r\n");
	}
	
}
void buttonTaskInit()
{
	return;	
}
void buttonTaskExecute(uint32 tick)
{
	static uint32 pre_tick = 0;
	uint32 press_time = 0;
	static uint32 idx_5s = 1;
	
	bool pin_lvl = port_pin_get_output_level(SW1_PIN);
	
	if(SW1_ACTIVE == pin_lvl){
		for (int i=0; i<MAX_CB_INDEX; i++)
		{
			if (button_detect_cb[i]!=NULL)
			button_detect_cb[i]();
		}
	}
	if(SW1_ACTIVE == pin_lvl && pre_tick == 0){
		pre_tick = tick;
		
	}
	else if(SW1_ACTIVE == pin_lvl && pre_tick != 0){
		if (tick > pre_tick)
		press_time = tick - pre_tick;
		
		if (press_time >= idx_5s*TIMEOUT_COUNTER_5S)
		{
			idx_5s++;
			for (int i=0; i<MAX_CB_INDEX; i++)
			{
				if (button_5s_timeout_cb[i]!=NULL)
					button_5s_timeout_cb[i]();
			}
		}
	}
	else
	{
		pre_tick = 0;
		idx_5s = 0;
	}
}

int regButtonPressDetectCallback(void* cb)
{
	for (int i=0; i<MAX_CB_INDEX; i++)
	{
		if (button_detect_cb[i]==NULL)
		{
			button_detect_cb[i] = cb;
			return i;
		}
	}
	
	printf("[%s] No quota...\n", __func__);
	return -1;
}
int unRegButtonPressDetectCallback(int sock)
{
	if (button_detect_cb[sock]!=NULL)
	{
			button_detect_cb[sock] = NULL;
			return 0;
	}
	else
		printf("[%s] Cannot find the related cb..\n", __func__);
	
	return -1;
}

int regButtonPress5sTimeoutCallback(void* cb)
{
	for (int i=0; i<MAX_CB_INDEX; i++)
	{
		if (button_5s_timeout_cb[i]==NULL)
		{
			button_5s_timeout_cb[i] = cb;
			return i;
		}
	}
	
	printf("[%s] No quota...\n", __func__);
	return -1;
}
int unRegButtonPress5sTimeoutCallback(int sock)
{
	
	if (button_5s_timeout_cb[sock]!=NULL)
	{
		button_5s_timeout_cb[sock] = NULL;
		return 0;
	}
	else
	printf("[%s] Cannot find the related cb..\n", __func__);
	return -1;
}
