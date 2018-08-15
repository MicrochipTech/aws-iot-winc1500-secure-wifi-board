/**
 * \file
 * \brief Unity tests for the cryptoauthlib Verify Command
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
#include <stdlib.h>
#include "atca_test.h"
#include "basic/atca_basic.h"
#include "host/atca_host.h"
#include "test/atca_tests.h"


TEST(atca_cmd_unit_test, gendig)
{
    ATCA_STATUS status;
    ATCAPacket packet;
    uint16_t keyID = 0x0004;

    unit_test_assert_config_is_locked();

    //build a nonce command (pass through mode)
    packet.param1 = NONCE_MODE_PASSTHROUGH;
    packet.param2 = 0x0000;
    memset(packet.data, 0x55, 32);    // a 32-byte nonce

    status = atNonce(gCommandObj, &packet);
    TEST_ASSERT_EQUAL_INT(NONCE_COUNT_LONG, packet.txsize);
    TEST_ASSERT_EQUAL_INT(NONCE_RSP_SIZE_SHORT, packet.rxsize);
    status = send_command(gCommandObj, gIface, &packet);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // check for nonce response for pass through mode
    TEST_ASSERT_EQUAL_INT8(ATCA_SUCCESS, packet.data[1]);

    //build a gendig command
    packet.param1 = GENDIG_ZONE_DATA;
    packet.param2 = keyID;

    status = atGenDig(gCommandObj, &packet, false);
    TEST_ASSERT_EQUAL_INT(GENDIG_COUNT, packet.txsize);
    TEST_ASSERT_EQUAL_INT(NONCE_RSP_SIZE_SHORT, packet.rxsize);
    status = send_command(gCommandObj, gIface, &packet);

    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL(0x00, packet.data[ATCA_RSP_DATA_IDX]);
}

TEST(atca_cmd_basic_test, gendig)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t random_number[32];
    uint16_t key_id = 0x04;
    uint8_t dummy[32];

    status = atcab_init(gCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    test_assert_data_is_locked();

    status = atcab_random(random_number);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_nonce(random_number);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_gendig(GENDIG_ZONE_DATA, key_id, dummy, sizeof(dummy));
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_release();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

t_test_case_info gendig_basic_test_info[] =
{
    { REGISTER_TEST_CASE(atca_cmd_basic_test, gendig), DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { (fp_test_case)NULL,                     (uint8_t)0 },/* Array Termination element*/
};

t_test_case_info gendig_unit_test_info[] =
{
    { REGISTER_TEST_CASE(atca_cmd_unit_test, gendig), DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { (fp_test_case)NULL,                    (uint8_t)0 },/* Array Termination element*/
};


