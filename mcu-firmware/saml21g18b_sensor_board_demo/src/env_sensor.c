/**
* \file
*
* \brief Environment Sensor Functions
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

/**
* \mainpage
* \section preface Preface
* This is the reference manual for the Multi-Role/Multi-Connect Application
*/
/*- Includes ---------------------------------------------------------------*/
#include <asf.h>
#include "wearable.h"
#include "env_sensor.h"
#include "bme280\bme280_support.h"
#include "conf_sensor.h"
#include "veml60xx\veml60xx.h"
#include "bhi160.h"
#include "bhi160\bhy_uc_driver.h"

#define TEMP_DISP_COUNTER	1	// update when same value is get 1 time
#define HUM_DISP_COUNTER	1
#define UV_DISP_COUNTER 1
#define PRESSURE_DISP_COUNTER 10

int16_t gi16Disp_temperature;
uint8_t gu8Disp_humidity;
uint32_t gu32Disp_uv;
uint16_t gu16Disp_pressure;
//environment data to BLE
environment_data_t environ_data;

static void (*env_sensor_update_cb)(environment_data_t, unsigned char);




static void get_env_sensor_data_from_chip(environment_data_t *env_data)
{
	s32 env_temperature;
	u32 env_pressure;
	u32 env_humidity;
	uint16_t als_data;
	
	bme280_set_power_mode(BME280_FORCED_MODE);
	
	if(ERROR == bme280_read_pressure_temperature_humidity(&env_pressure, &env_temperature, &env_humidity)){
		//error
		DBG_LOG("Reading BME280 has failed");
	}
	if(STATUS_OK != veml60xx_read_alsdata(&als_data)){
		//error
		DBG_LOG("Reading VEML60xx has failed");
	}
	
	/* Temperature: No data processing required for Temperature data. Data with resolution 0.01(x100) directly txd*/
	env_data->temperature = (int16_t)env_temperature;
	/* Pressure: Returns pressure in Pa as unsigned 32 bit integer. Output value of ?6386?equals 96386 Pa = 963.86 hPa*/
	env_data->pressure = (uint16_t)(env_pressure / 100);
	/*ALS: lx/step = 0.07 */
	///env_data->uv = ((uint32_t)(als_data) * 7000 / 100000));
	env_data->uv = ((uint32_t)als_data * 7000);
	//printf("\r\ntest=%d\r\n",(uint32_t) als_data);
	/* Humidity: An output value of 42313 represents 42313 / 1024 = 41.321 %rH*/
	env_data->humidity = (uint8_t)(env_humidity / 1024);
}

void env_sensor_data_init()
{
	environment_data_t environment_data;
	
	get_env_sensor_data_from_chip(&environment_data);
	
	gi16Disp_temperature = environment_data.temperature;
	gu8Disp_humidity = environment_data.humidity;
	gu32Disp_uv	= environment_data.uv;
	gu16Disp_pressure = environment_data.pressure;
	
	return;
}



void env_sensor_execute()
{
	static int16_t pre_temp = 0;
	static uint8_t pre_hum = 0;
	static uint32_t pre_uv = 0;
	static uint16_t pre_pressure = 0;
	
	static int temp_cnt;
	static int hum_cnt;
	static int uv_cnt;
	static int pressure_cnt;
	
	static int uv_not_equal_cnt;
	
	unsigned char updateFlag = 0;
	
	environment_data_t environment_data;
	get_env_sensor_data_from_chip(&environment_data);
	printf("DBG2: temperature = %d, humidity = %d, uv = %lu, pressure = %d\r\n", environment_data.temperature, environment_data.humidity, environment_data.uv, environment_data.pressure);
	
	// check temperature
	if (pre_temp != environment_data.temperature)
	{
		temp_cnt = 0;
		pre_temp = environment_data.temperature;
	}
	else
		temp_cnt++;
	
	int temp_update = 0;
	if ((temp_cnt >= TEMP_DISP_COUNTER) && gi16Disp_temperature!= pre_temp)
	{
		if (gi16Disp_temperature > pre_temp )
		{
			if (gi16Disp_temperature - pre_temp > 10)
				temp_update = 1;
		}
		else
		{
			if (pre_temp - gi16Disp_temperature > 10)
				temp_update = 1;
		}
		
		if (temp_update)
		{
			gi16Disp_temperature = pre_temp;
			updateFlag |= TEMP_UPDATE_BIT; 
		}
	}
	
	// check humidity
	if (pre_hum != environment_data.humidity)
	{
		hum_cnt = 0;
		pre_hum = environment_data.humidity;
	}
	else
		hum_cnt++;
	
	if ((hum_cnt >= HUM_DISP_COUNTER) && gu8Disp_humidity!= pre_hum)
	{
		gu8Disp_humidity = pre_hum;
		updateFlag |= HUM_UPDATE_BIT; 
	}
	
	// check uv
	if (pre_uv != environment_data.uv)
	{
		uv_cnt = 0;
		pre_uv = environment_data.uv;
		uv_not_equal_cnt++;
	}
	else
	{
		uv_cnt++;
		uv_not_equal_cnt = 0;
	}
	//printf("test = %d\n", pre_uv/1000000);
	
	//if (((uv_cnt >= UV_DISP_COUNTER) && gu32Disp_uv != pre_uv) || (uv_not_equal_cnt >1))
	//if (((uv_cnt >= UV_DISP_COUNTER) && gu32Disp_uv!= pre_uv))
	int update_uv = 0;
	if (((gu32Disp_uv/1000000) > (pre_uv/1000000)))
	{
		if ((gu32Disp_uv/1000000) - (pre_uv/1000000) > 4)
			update_uv = 1;
	}
	else
	{
		if ((pre_uv/1000000) - (gu32Disp_uv/1000000) > 4)
			update_uv = 1;
	}
	
	if (update_uv)
	{
		gu32Disp_uv = pre_uv;
		updateFlag |= UV_UPDATE_BIT; 
		uv_not_equal_cnt = 0;
	}
	
	// check pressure
	if (pre_pressure != environment_data.pressure)
	{
		pressure_cnt = 0;
		pre_pressure = environment_data.pressure;
	}
	else
		pressure_cnt++;
	
	if ((pressure_cnt >= PRESSURE_DISP_COUNTER) && gu16Disp_pressure!= pre_pressure)
	{
		gu16Disp_pressure = pre_pressure;
		updateFlag |= PRESSURE_UPDATE_BIT; 
	}
		
	if (updateFlag > 0)
		env_sensor_update_cb(environment_data, updateFlag);
	
}

void get_env_sensor_data_for_display(environment_data_t *env_data)
{
	env_data->temperature = gi16Disp_temperature;
	env_data->humidity = gu8Disp_humidity;
	env_data->uv = gu32Disp_uv;
	env_data->pressure = gu16Disp_pressure;
	
}

void register_env_sensor_udpate_callback_handler(void* cb)
{
	env_sensor_update_cb = cb;
}