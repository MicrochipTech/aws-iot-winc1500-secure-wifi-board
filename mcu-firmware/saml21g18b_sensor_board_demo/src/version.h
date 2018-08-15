/**
 * \file version.h
 *
 * \brief gesture file
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

#ifndef _VERSION_H_
#define _VERSION_H_

/* Release version */
#define SW_VERSION_MAJOR	1

/* Development version */
#define SW_VERSION_MINOR	0


#define CRYPTO_LIB_MAJOR_VERSION  2    //! AWS IoT Zero Touch major version number
#define CRYPTO_LIB_API_VERSION    2    //! AWS IoT Zero Touch API version number
#define CRYPTO_LIB_PATCH_VERSION  4    //! AWS IoT Zero Touch patch version number

#define STRING2(x)  #x
#define STRING(x)   STRING2(x)

//! AWS IoT Zero Touch version string (Example: "1.0.0")
#define VERSION_STRING       STRING(CRYPTO_LIB_MAJOR_VERSION) "." \
STRING(CRYPTO_LIB_API_VERSION) "." \
STRING(CRYPTO_LIB_PATCH_VERSION)

//! AWS IoT Zero Touch long version string (Example: "AWS IoT Zero Touch Demo v1.0.0")
#define VERSION_STRING_LONG  "AWS IoT Zero Touch Demo v" VERSION_STRING


#endif // _VERSION_H_
