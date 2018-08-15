/**
 * \file
 * \brief Utilities to create and verify a JSON Web Token (JWT)
 *
 * \copyright (c) 2017 Microchip Technology Inc. and its subsidiaries.
 *            You may use this software and any derivatives exclusively with
 *            Microchip products.
 *
 * \page License
 *
 * (c) 2017 Microchip Technology Inc. and its subsidiaries. You may use this
 * software and any derivatives exclusively with Microchip products.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIPS TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
 * TERMS.
 */

#ifndef ATCA_JWT_H_
#define ATCA_JWT_H_

#include "cryptoauthlib.h"

/** \brief Structure to hold metadata information about the jwt being built */
typedef struct
{
    char*    buf;          /* Input buffer */
    uint16_t buflen;       /* Total buffer size */
    uint16_t cur;          /* Current location in the buffer */
} atca_jwt_t;

ATCA_STATUS atca_jwt_init(atca_jwt_t* jwt, char* buf, uint16_t buflen);
ATCA_STATUS atca_jwt_add_claim_string(atca_jwt_t* jwt, const char* claim, const char* value);
ATCA_STATUS atca_jwt_add_claim_numeric(atca_jwt_t* jwt, const char* claim, int32_t value);
ATCA_STATUS atca_jwt_finalize(atca_jwt_t* jwt, uint16_t key_id);

ATCA_STATUS atca_jwt_verify(const char* buf, uint16_t buflen, const uint8_t* pubkey);

#endif /* ATCA_JWT_H_ */
