/**
 *
 * \file
 *
 * \brief AWS Status Functions
 *
 * Copyright (c) 2016-2017 Atmel Corporation. All rights reserved.
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

#include <string.h>
#include <stdio.h>

#include "aws_status.h"
#include "hex_dump.h"

// Global variables
static struct aws_iot_status g_aws_iot_status;

/**
 * \brief Clears the global AWS IoT status information
 */
void aws_iot_clear_status(void)
{
    g_aws_iot_status.aws_state  = AWS_STATE_UNKNOWN;
    g_aws_iot_status.aws_status = AWS_STATUS_SUCCESS;

    memset(&g_aws_iot_status.aws_message[0], 0, sizeof(g_aws_iot_status.aws_message));
}

/**
 * \brief Gets the global AWS IoT status information
 *
 * \note Do not free the pointer returned from this function
 */
struct aws_iot_status * aws_iot_get_status(void)
{
    return &g_aws_iot_status;
}

/**
 * \brief Sets the global AWS IoT status information
 *
 * param[in] state                  The AWS IoT error state
 * param[in] status                 The AWS IoT error status
 * param[in] message                The AWS IoT error message
 */
void aws_iot_set_status(uint32_t state, uint32_t status, const char *message)
{
    g_aws_iot_status.aws_state  = state;
    g_aws_iot_status.aws_status = status;

    memset(&g_aws_iot_status.aws_message[0], 0, 
           sizeof(g_aws_iot_status.aws_message));
    strncpy(&g_aws_iot_status.aws_message[0], &message[0], 
            sizeof(g_aws_iot_status.aws_message));
}


/**
 * \brief Prints the current AWS IoT status information to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 * \param[in] error                 The AWS IoT status information to be printed
 */
void console_print_aws_status(const char *message, const struct aws_iot_status *status)
{
    // Obtain the console mutex

    printf("\r\nSTATUS: %s\r\n", message);
    
    // Print the AWS status information
    printf("Current AWS IoT State:   %lu\r\n", status->aws_state);
    printf("Current AWS IoT Status:  %lu\r\n", status->aws_status);
    printf("Current AWS IoT Message: %s\r\n", status->aws_message);

    printf("\r\n");
    
}

/**
 * \brief Prints the AWS IoT message information to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 * \param[in] buffer                The data buffer containing the information
 *                                  to be printed
 * \param[in] length                The length, in bytes, of the data buffer
 */
void console_print_aws_message(const char *message, const void *buffer, size_t length)
{

    printf("\r\n%s\r\n", message);

    // Print the hex dump of the information in the data buffer
    print_hex_dump(buffer, length, true, true, 16);

    printf("\r\n");
    
}