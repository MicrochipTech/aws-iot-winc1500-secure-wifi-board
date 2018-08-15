/**
 * \file
 *
 * \brief WINC1500 Functions
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

#define USE_SHADOW

#ifndef __WIFI_APP_H__
#define __WIFI_APP_H__
#include "datapoint.h"
#include "socket/include/m2m_socket_host_if.h"

#define SUBSCRIBE_TOPIC "dataControl"
#define PUBLISH_TOPIC	"dataReport"
#define SUBSCRIBE_TOPIC_SHADOW "shadow/update/delta"
#define PUBLISH_TOPIC_SHADOW	"shadow/update"
#define SUBSCRIBE_SEARCH_TOPIC "search"
#define PUBLISH_SEARCH_RESP_TOPIC "searchResp"
#define DEV_SEARCH_SUBSCRIBE_TOPIC "winc1500Iot/search"

#if USE_WEP
#define AP_WLAN_AUTH           M2M_WIFI_SEC_WEP /* < Security manner */
#define MAIN_WLAN_WEP_KEY        "6174000000"     /* < Security Key in WEP Mode - in 10 or 26 bytes hex */
#define MAIN_WLAN_WEP_KEY_OFFSET 4                /* < Attach MAC addr after this offset in WEP key */
#define MAIN_WLAN_WEP_KEY_INDEX  (1)
#else
#define AP_WLAN_AUTH           M2M_WIFI_SEC_OPEN /* < Security manner */
#endif

#define HEX2ASCII(x) (((x) >= 10) ? (((x) - 10) + 'A') : ((x) + '0'))
#define MAIN_WLAN_SSID_OFFSET    14                     /* < Attach MAC addr after this offset in SSID */
#define AP_WLAN_SSID	"iGatewayF8F005F3659B"


#define AP_TCP_SERVER_PORT				8899
#define MQTT_CLIENT_ID_LEN				13
#define MQTT_CHANNEL_LEN				90

#ifdef AWS_JITR
char g_mqtt_client_id[129];
char g_thing_name[129];
#endif

typedef enum
{
	APP_STA,
	APP_AP,
	APP_P2P,
} wifi_mode;

typedef enum 
{
	WIFI_TASK_IDLE,
	WIFI_TASK_SWITCHING_TO_STA,
	WIFI_TASK_CONNECT_CLOUD,
	WIFI_TASK_MQTT_SUBSCRIBE,
	WIFI_TASK_MQTT_RUNNING,
	WIFI_TASK_CONNECT_CLOUD_FINISH,
	WIFI_TASK_SWITCH_TO_AP,
	WIFI_TASK_SWITCHING_TO_AP,
	WIFI_TASK_AP_CONNECTED,
	WIFI_TASK_STA_DISCONNECTED,

	WIFI_TASK_MAX
} wifi_FSM_states;


char gAwsMqttClientId[MQTT_CLIENT_ID_LEN];
char gSubscribe_Channel[MQTT_CHANNEL_LEN];
char gPublish_Channel[MQTT_CHANNEL_LEN];

char gSubscribe_Channel_shadow[MQTT_CHANNEL_LEN];
char gPublish_Channel_shadow[MQTT_CHANNEL_LEN];


char gSearch_Channel[MQTT_CHANNEL_LEN];
char gSearchResp_Channel[MQTT_CHANNEL_LEN];


int wifiInit(void);
int wifiTaskExecute(void);
void detectWiFiMode(void);
void setWiFiStates(wifi_FSM_states state);
wifi_FSM_states getWiFiStates(void);
wifi_mode getWiFiMode(void);

#endif /*__WIFI_APP_H__*/
