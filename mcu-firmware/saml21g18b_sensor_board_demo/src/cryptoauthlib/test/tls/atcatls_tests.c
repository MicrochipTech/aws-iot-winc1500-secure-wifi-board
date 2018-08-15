/**
 * \file
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

#include "cryptoauthlib.h"
#include "basic/atca_basic.h"
#include "atcacert/atcacert_host_hw.h"
#include "tls/atcatls.h"
#include "tls/atcatls_cfg.h"
#include "test/tls/atcatls_tests.h"

/*---------------------------------------------------------------------------------------------
   GenKey Command Sent:
   07 40 00 00 00 00 05
   GenKeyCmd Received:
   43
   3C 30 7F 3A 1B 05 96 19 21 EB 35 50 09 1D 1D 48 5C 68 D4 A4 40 21 05 90 21 F6 A7 F2 A4 7F 2B 8E
   DD 28 1B 0A A8 F4 5A F4 AC DC 85 D9 9A D0 34 6B 60 B1 7F E6 D8 43 26 D9 39 48 C6 34 CF 45 DE 81
   70 0B
   Nonce Command Sent:
   27 16 03 00 00
   8C 61 64 CE FD 38 06 05 F7 29 44 E3 B6 5B 9A 33 34 94 63 2D 2E 16 FD 9E 77 98 F6 F2 67 32 A1 76
   0B 11
   NonceCommand Received:
   04 00 03 40
   Sign Command Sent:
   07 41 80 00 00 28 05
   SignCmd Received:
   43
   CC 58 BC B6 7D 8D 82 28 6B F4 9A 22 88 71 2B 57 99 73 51 56 9E E6 98 0C 06 CD 70 EB 82 B5 4D 58
   D1 06 F0 BE DF BC 9E 00 3E 56 53 C6 33 6D FA 9E B5 3E C1 7E 37 E6 66 E8 68 CF B7 7E 49 1E BA BB
   51 20
   Nonce Command Sent:
   27 16 03 00 00
   8C 61 64 CE FD 38 06 05 F7 29 44 E3 B6 5B 9A 33 34 94 63 2D 2E 16 FD 9E 77 98 F6 F2 67 32 A1 76
   0B 11
   NonceCommand Received:
   04 00 03 40
   Verify Command Sent:
   87 45 02 04 00
   CC 58 BC B6 7D 8D 82 28 6B F4 9A 22 88 71 2B 57 99 73 51 56 9E E6 98 0C 06 CD 70 EB 82 B5 4D 58
   D1 06 F0 BE DF BC 9E 00 3E 56 53 C6 33 6D FA 9E B5 3E C1 7E 37 E6 66 E8 68 CF B7 7E 49 1E BA BB
   3C 30 7F 3A 1B 05 96 19 21 EB 35 50 09 1D 1D 48 5C 68 D4 A4 40 21 05 90 21 F6 A7 F2 A4 7F 2B 8E
   DD 28 1B 0A A8 F4 5A F4 AC DC 85 D9 9A D0 34 6B 60 B1 7F E6 D8 43 26 D9 39 48 C6 34 CF 45 DE 81
   0E FF
   VerifyCmd Received:
   04 00 03 40
   -----------------------------------------------------------------------------------------------
 */

// Test vectors
uint8_t serverPubKey[] =
{
    // X coordinate of the elliptic curve.
    0xF4, 0x57, 0x47, 0x0E, 0x47, 0x8A, 0x0C, 0xC1, 0xC2, 0x0F, 0x06, 0x58, 0x36, 0x82, 0x55, 0xBD,
    0xAE, 0x39, 0x44, 0x24, 0x1E, 0xFF, 0xB2, 0x2F, 0x8D, 0xD3, 0xA9, 0x8B, 0x05, 0x8D, 0xBA, 0xF1,

    // Y coordinate of the elliptic curve.
    0xFA, 0x8E, 0x4A, 0xD1, 0x87, 0xDB, 0x01, 0xB9, 0xB8, 0xF4, 0xB2, 0x1F, 0x2F, 0x18, 0x03, 0xEB,
    0x64, 0xD8, 0x98, 0xE2, 0xCB, 0x3B, 0xC4, 0x97, 0x84, 0xEB, 0x64, 0xB4, 0x90, 0xE3, 0x78, 0x12
};

uint8_t pubKey1[] =
{
    // X coordinate of the elliptic curve.
    0x3C, 0x30, 0x7F, 0x3A, 0x1B, 0x05, 0x96, 0x19, 0x21, 0xEB, 0x35, 0x50, 0x09, 0x1D, 0x1D, 0x48,
    0x5C, 0x68, 0xD4, 0xA4, 0x40, 0x21, 0x05, 0x90, 0x21, 0xF6, 0xA7, 0xF2, 0xA4, 0x7F, 0x2B, 0x8E,

    // Y coordinate of the elliptic curve.
    0xDD, 0x28, 0x1B, 0x0A, 0xA8, 0xF4, 0x5A, 0xF4, 0xAC, 0xDC, 0x85, 0xD9, 0x9A, 0xD0, 0x34, 0x6B,
    0x60, 0xB1, 0x7F, 0xE6, 0xD8, 0x43, 0x26, 0xD9, 0x39, 0x48, 0xC6, 0x34, 0xCF, 0x45, 0xDE, 0x81
};

uint8_t msg1[] =
{
    0x8C, 0x61, 0x64, 0xCE, 0xFD, 0x38, 0x06, 0x05, 0xF7, 0x29, 0x44, 0xE3, 0xB6, 0x5B, 0x9A, 0x33,
    0x34, 0x94, 0x63, 0x2D, 0x2E, 0x16, 0xFD, 0x9E, 0x77, 0x98, 0xF6, 0xF2, 0x67, 0x32, 0xA1, 0x76
};

uint8_t sig1[] =
{
    // R coordinate of the signature.
    0xCC, 0x58, 0xBC, 0xB6, 0x7D, 0x8D, 0x82, 0x28, 0x6B, 0xF4, 0x9A, 0x22, 0x88, 0x71, 0x2B, 0x57,
    0x99, 0x73, 0x51, 0x56, 0x9E, 0xE6, 0x98, 0x0C, 0x06, 0xCD, 0x70, 0xEB, 0x82, 0xB5, 0x4D, 0x58,

    // S coordinate of the signature.
    0xD1, 0x06, 0xF0, 0xBE, 0xDF, 0xBC, 0x9E, 0x00, 0x3E, 0x56, 0x53, 0xC6, 0x33, 0x6D, 0xFA, 0x9E,
    0xB5, 0x3E, 0xC1, 0x7E, 0x37, 0xE6, 0x66, 0xE8, 0x68, 0xCF, 0xB7, 0x7E, 0x49, 0x1E, 0xBA, 0xBB
};

// The global configuration that is used for all tests
ATCAIfaceCfg* g_pCfg = NULL;

// Certificate definitions
const uint8_t g_signer_1_ca_public_key_t[];
const atcacert_def_t g_cert_def_1_signer_t;
const atcacert_def_t g_cert_def_0_device_t;
const atcacert_def_t g_DeviceCsrDef;


/** \brief Main entry point to run the TLS tests
 *  \param[in] pCfg The ATCAIfaceCfg configuration that defines the HAL layer interface
 *  \return void
 */
void atcatls_test_runner(ATCAIfaceCfg* pCfg)
{
    UnityBegin("atcatls_tests.c");

    g_pCfg = pCfg;

    // Call get SN first
    RUN_TEST(test_atcatls_get_sn);

    // Configure the device
    RUN_TEST(test_atcatls_config_default);

    // Call random after configuration
    RUN_TEST(test_atcatls_random);

    // TLS API Init/finish
    RUN_TEST(test_atcatls_init_finish);
    RUN_TEST(test_atcatls_init_enc_key);

    // Core TLS definitions
    RUN_TEST(test_atcatls_create_key);
    RUN_TEST(test_atcatls_sign);
    RUN_TEST(test_atcatls_verify);
    RUN_TEST(test_atcatls_ecdh);
    RUN_TEST(test_atcatls_ecdhe);
    RUN_TEST(test_atcatls_calc_pubkey);
    RUN_TEST(test_atcatls_read_pubkey);

    RUN_TEST(test_atcatls_verify_cert_chain);
    RUN_TEST(test_atcatls_ca_pubkey_write_read);
    RUN_TEST(test_atcatls_create_csr);

    RUN_TEST(test_atcatls_enc_write_read);
    RUN_TEST(test_atcatls_enc_rsakey_write_read);

    UnityEnd();
}

/** \brief This test will write a default configuration and lock a blank device for testing.
 * If the part is already locked, it will verify the configuration zone against the default configuraion.
 *  \return void
 */
void test_atcatls_config_default(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_config_default();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief This test will initialize and finish the communication to an ECC508.
 *  \return void
 */
void test_atcatls_init_finish(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;

    // The atcatls_init() function will call atcatls_com_init(), test for success
    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // The atcatls_finish() function will call atcatls_com_release(), test for success
    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test that the serial number can be read from the ECC508.  This test should always succeed.
 *  \return void
 */
void test_atcatls_get_sn(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t snOut[ATCA_SERIAL_NUM_SIZE] = { 0 };
    uint8_t snOutNull[ATCA_SERIAL_NUM_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_get_sn(snOut);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare snOut memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(snOut, snOutNull, ATCA_SERIAL_NUM_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper creation of a key in the ECDH slot of the default configuration.
 *  \return void
 */
void test_atcatls_create_key(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t pubkey[ATCA_PUB_KEY_SIZE] = { 0 };
    uint8_t pubkeyNull[ATCA_PUB_KEY_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_create_key(TLS_SLOT_ECDHE_PRIV, pubkey);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare pubkey memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(pubkey, pubkeyNull, ATCA_PUB_KEY_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test that a signature is produced from the authentication slot of the default configuration.
 *  \return void
 */
void test_atcatls_sign(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t signature[ATCA_SIG_SIZE] = { 0 };
    uint8_t sigNull[ATCA_SIG_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_sign(TLS_SLOT_AUTH_PRIV, msg1, signature);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare signature memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(signature, sigNull, ATCA_SIG_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test that the part can be used for an external verify using known valid vectors.
 *  \return void
 */
void test_atcatls_verify(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    bool verified = false;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_verify(msg1, sig1, pubKey1, &verified);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // See if the buffers actually verified
    TEST_ASSERT_TRUE(verified);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper ECDH operation using the ECDH slot of the default configuration.
 *  \return void
 */
void test_atcatls_ecdh(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t pmk[ATCA_KEY_SIZE] = { 0 };
    uint8_t pmkNull[ATCA_KEY_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_ecdh(TLS_SLOT_ECDH_PRIV, pubKey1, pmk);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare pmk memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(pmk, pmkNull, ATCA_KEY_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper ECDHE operation after a GenKey using the ECDH slot of the default configuration.
 *  \return void
 */
void test_atcatls_ecdhe(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t pmk[ATCA_KEY_SIZE] = { 0 };
    uint8_t pmkNull[ATCA_KEY_SIZE] = { 0 };
    uint8_t pubKeyRet[ATCA_PUB_KEY_SIZE] = { 0 };
    uint8_t pubKeyNull[ATCA_PUB_KEY_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_ecdhe(TLS_SLOT_ECDH_PRIV, pubKey1, pubKeyRet, pmk);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare pmk memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(pmk, pmkNull, ATCA_KEY_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    // Compare pmk memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(pubKeyRet, pubKeyNull, ATCA_PUB_KEY_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper Random Command operation using the ECC508.
 *  \return void
 */
void test_atcatls_random(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t randomNum[RANDOM_RSP_SIZE] = { 0 };
    uint8_t randomNumNull[RANDOM_RSP_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_random(randomNum);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare randomNum memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(randomNum, randomNumNull, RANDOM_RSP_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper public key calculation using the GenKey operation on a private key slot of the authentication slot of the default configuration.
 *  \return void
 */
void test_atcatls_calc_pubkey(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t pubkey[ATCA_PUB_KEY_SIZE] = { 0 };
    uint8_t pubkeyNull[ATCA_PUB_KEY_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    atcatls_calc_pubkey(TLS_SLOT_AUTH_PRIV, pubkey);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare randomNum memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(pubkey, pubkeyNull, ATCA_PUB_KEY_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper public key retrieval from a clear read slot of the default configuration.
 *  \return void
 */
void test_atcatls_read_pubkey(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t pubkey[ATCA_PUB_KEY_SIZE] = { 0 };
    uint8_t pubkeyNull[ATCA_PUB_KEY_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    atcatls_read_pubkey(TLS_SLOT_MFRCA_PUBKEY, pubkey);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare randomNum memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(pubkey, pubkeyNull, ATCA_PUB_KEY_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper public key retrieval after writing from a clear read/write slot of the default configuration.
 *  \return void
 */
void test_atcatls_ca_pubkey_write_read(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t pubkey[ATCA_PUB_KEY_SIZE] = { 0 };
    uint8_t pubkeyNull[ATCA_PUB_KEY_SIZE] = { 0 };
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    atcatls_read_pubkey(TLS_SLOT_MFRCA_PUBKEY, pubkey);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Compare randomNum memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(pubkey, pubkeyNull, ATCA_PUB_KEY_SIZE);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

// Testing the write of the encryption key
ATCA_STATUS get_enc_key(uint8_t* enckey, int16_t keysize);
// global variable for key
uint8_t _enckeytest[ATCA_KEY_SIZE] = { 0 };

/** \brief A callback function implementation to return the platform encryption key.
 *   This callback must be implemented on the target platform.
 *  \return void
 */
ATCA_STATUS get_enc_key(uint8_t* enckey, int16_t keysize)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;

    if (enckey == NULL || (size_t)keysize < sizeof(_enckeytest))
    {
        return status;
    }
    memcpy(enckey, _enckeytest, sizeof(_enckeytest));
    return ATCA_SUCCESS;
}

/** \brief Test proper initialization of the encryption key.  Write the random value to the encryption key slot of the default configuration.
 *  \return void
 */
void test_atcatls_init_enc_key(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t enckey_ret[ATCA_KEY_SIZE] = { 0 };
    bool lock = false;
    uint8_t enckeyId = TLS_SLOT_ENC_PARENT;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatlsfn_set_get_enckey(&get_enc_key);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Save the return value of _enckeytest to the application
    // Return _enckeytest when get_enc_key is called
    status = atcatls_init_enckey(_enckeytest, enckeyId, lock);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_get_enckey(enckey_ret);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL_MEMORY(enckey_ret, _enckeytest, ATCA_KEY_SIZE);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper encrypted write/read using the default configuration.
 *  \return void
 */
void test_atcatls_enc_write_read(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t writeBytes[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                             0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19 };
    int16_t writeSize = sizeof(writeBytes);
    uint8_t readBytes[ATCA_BLOCK_SIZE] = { 0 };
    int16_t readSize = sizeof(readBytes);
    uint8_t enckeyId = TLS_SLOT_ENC_PARENT;
    uint8_t slotId = TLS_SLOT8_ENC_STORE;
    uint8_t block = 0;


    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_enc_write(slotId, block, enckeyId, writeBytes, writeSize);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_enc_read(slotId, block, enckeyId, readBytes, &readSize);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Check the equivalence of the buffers
    TEST_ASSERT_EQUAL(readSize, writeSize);
    TEST_ASSERT_EQUAL_MEMORY(readBytes, writeBytes, readSize);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test proper encrypted write/read of an RSA key using the default configuration.
 *  \return void
 */
void test_atcatls_enc_rsakey_write_read(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t rsakey[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                         0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                         0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                         0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
                         0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
                         0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
                         0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
                         0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
                         0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
                         0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
                         0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
                         0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
                         0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
                         0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
                         0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
                         0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9 };
    uint8_t readBytes[RSA2048_KEY_SIZE] = { 0 };
    uint8_t enckeyId = TLS_SLOT_ENC_PARENT;
    int16_t keysize = RSA2048_KEY_SIZE;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Write the RSA key
    status = atcatls_enc_rsakey_write(enckeyId, rsakey, keysize);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Read the RSA key
    status = atcatls_enc_rsakey_read(enckeyId, readBytes, &keysize);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL(keysize, RSA2048_KEY_SIZE);

    // Check the equivalence of the buffers
    TEST_ASSERT_EQUAL_MEMORY(readBytes, rsakey, RSA2048_KEY_SIZE);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test the return value of the CA certificate.
 *  \return void
 */
void test_atcatls_create_csr(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    char csr[1024] = { 0 };
    char csrNull[1024] = { 0 };
    size_t csrSize = 1024;
    int cmpResult = 0;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Get the CSR
    status = atcatls_create_csr(&g_DeviceCsrDef, csr, &csrSize);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Print the CSR to the console
    printf("%s", csr);

    // Compare certificate memory, it should have changed.  If all bytes are equal memcmp will return 0.
    cmpResult = memcmp(csr, csrNull, csrSize);
    TEST_ASSERT_NOT_EQUAL(cmpResult, 0);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief Test chain verification using certificates that were read from a properly configured ECC508.
 *  The device certificate and signer certificate are verified.
 *  \return void
 */
void test_atcatls_verify_cert_chain(void)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t signerPubkey[64] = { 0 };
    uint8_t caPubkey[64] = { 0 };
    uint8_t signerCert[1024] = { 0 };
    uint8_t deviceCert[1024] = { 0 };
    size_t signerCertSize = 1024;
    size_t deviceCertSize = 1024;

    status = atcatls_init(g_pCfg);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Get the CA public key
    memcpy(caPubkey, g_signer_1_ca_public_key_t, sizeof(caPubkey));

    // Get the signer certificate
    status = atcatls_get_cert(&g_cert_def_1_signer_t, g_signer_1_ca_public_key_t, signerCert, &signerCertSize);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Get the signer public key from the signer certificate
    status = atcacert_get_subj_public_key(&g_cert_def_1_signer_t, signerCert, signerCertSize, signerPubkey);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Get the device certificate
    status = atcatls_get_cert(&g_cert_def_0_device_t, signerPubkey, deviceCert, &deviceCertSize);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Verify the signer certificate
    status = atcacert_verify_cert_hw(&g_cert_def_1_signer_t, signerCert, signerCertSize, caPubkey);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    // Verify the device certificate
    status = atcacert_verify_cert_hw(&g_cert_def_0_device_t, deviceCert, deviceCertSize, signerPubkey);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcatls_finish();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Cert Constants

// Signer
const uint8_t g_signer_1_ca_public_key_t[64] =
{
    0xBD, 0x14, 0x1C, 0x5D, 0xB1, 0xAC, 0xCE, 0x0F, 0xCC, 0xF1, 0xC2, 0x25, 0x21, 0xEB, 0x80, 0xA4,
    0x8B, 0xFB, 0x4D, 0xEB, 0x69, 0xC7, 0x76, 0x58, 0xED, 0x55, 0x7B, 0x7E, 0xDC, 0x71, 0x5D, 0x57,
    0x82, 0xCB, 0x82, 0x77, 0x80, 0xEE, 0x13, 0xBF, 0x18, 0xAA, 0x87, 0x4F, 0xDA, 0x2A, 0x6A, 0xA5,
    0x83, 0x4A, 0x09, 0x1B, 0xA8, 0x6B, 0x0D, 0x36, 0xD1, 0x98, 0x05, 0x57, 0xE6, 0x8E, 0x89, 0xA0
};

const uint8_t g_cert_template_1_signer_t[] =
{
    0x30, 0x82, 0x01, 0xC3, 0x30, 0x82, 0x01, 0x69, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x03, 0x40,
    0x00, 0x02, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x38,
    0x31, 0x1A, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x11, 0x41, 0x74, 0x6D, 0x65, 0x6C,
    0x20, 0x4F, 0x70, 0x65, 0x6E, 0x53, 0x53, 0x4C, 0x20, 0x44, 0x65, 0x76, 0x31, 0x1A, 0x30, 0x18,
    0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x11, 0x41, 0x54, 0x45, 0x43, 0x43, 0x35, 0x30, 0x38, 0x41,
    0x20, 0x52, 0x6F, 0x6F, 0x74, 0x20, 0x43, 0x41, 0x30, 0x20, 0x17, 0x0D, 0x31, 0x35, 0x31, 0x30,
    0x31, 0x32, 0x31, 0x35, 0x30, 0x30, 0x30, 0x30, 0x5A, 0x18, 0x0F, 0x39, 0x39, 0x39, 0x39, 0x31,
    0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x48, 0x31, 0x1A, 0x30, 0x18,
    0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x11, 0x41, 0x74, 0x6D, 0x65, 0x6C, 0x20, 0x4F, 0x70, 0x65,
    0x6E, 0x53, 0x53, 0x4C, 0x20, 0x44, 0x65, 0x76, 0x31, 0x2A, 0x30, 0x28, 0x06, 0x03, 0x55, 0x04,
    0x03, 0x0C, 0x21, 0x4F, 0x70, 0x65, 0x6E, 0x53, 0x53, 0x4C, 0x20, 0x44, 0x65, 0x76, 0x20, 0x41,
    0x54, 0x45, 0x43, 0x43, 0x35, 0x30, 0x38, 0x41, 0x20, 0x53, 0x69, 0x67, 0x6E, 0x65, 0x72, 0x20,
    0x30, 0x30, 0x30, 0x32, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02,
    0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x91,
    0x9C, 0xFA, 0xB8, 0x3A, 0x7B, 0xDD, 0xCC, 0x61, 0x6E, 0x15, 0x34, 0xB3, 0xC8, 0x0F, 0x7B, 0x15,
    0x55, 0xE3, 0xA3, 0x4E, 0xA0, 0xE5, 0x47, 0xAC, 0x69, 0xD8, 0xB4, 0xC1, 0x5D, 0xD6, 0x3C, 0xC8,
    0x48, 0x32, 0x9D, 0x54, 0x05, 0xD3, 0x88, 0xD7, 0xB1, 0xC6, 0xE7, 0xC5, 0x27, 0x11, 0xD0, 0x6E,
    0x5D, 0xDF, 0x09, 0xA2, 0x84, 0x03, 0xE0, 0x01, 0x01, 0xF5, 0x85, 0xFC, 0xB5, 0x28, 0xAC, 0xA3,
    0x50, 0x30, 0x4E, 0x30, 0x0C, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01,
    0xFF, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x69, 0xF4, 0xD5, 0x12,
    0x5A, 0x44, 0x49, 0x7A, 0xCC, 0x75, 0x11, 0xF5, 0x4A, 0x30, 0x86, 0xA5, 0xD4, 0xAB, 0xA3, 0x9C,
    0x30, 0x1F, 0x06, 0x03, 0x55, 0x1D, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x25, 0x28, 0x33,
    0xA6, 0xDF, 0x72, 0xFD, 0x06, 0x96, 0xD1, 0xA4, 0x56, 0x7E, 0x33, 0xF0, 0x17, 0x5C, 0x46, 0xBC,
    0x6F, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00,
    0x30, 0x45, 0x02, 0x21, 0x00, 0x82, 0x86, 0x82, 0xEE, 0x1E, 0x71, 0x0C, 0xDC, 0x31, 0x83, 0x46,
    0x1E, 0xE8, 0x86, 0x9A, 0x67, 0xD0, 0xA6, 0x06, 0xE0, 0x89, 0xEA, 0xF9, 0x5B, 0x6B, 0x41, 0x32,
    0x1B, 0xA8, 0x73, 0x02, 0x8D, 0x02, 0x20, 0x05, 0x76, 0x28, 0x14, 0xE2, 0x05, 0xB7, 0xFF, 0x6D,
    0xAC, 0x42, 0x9B, 0x12, 0x3C, 0x62, 0x79, 0x1F, 0xE7, 0x1C, 0x52, 0x56, 0x3F, 0xFD, 0x34, 0x01,
    0xF4, 0x6E, 0x69, 0xAD, 0x0D, 0x8D, 0x8F
};

const atcacert_def_t g_cert_def_1_signer_t =
{
    .type                   = CERTTYPE_X509,
    .template_id            = 1,
    .chain_id               = 0,
    .private_key_slot       = 0,
    .sn_source              = SNSRC_SIGNER_ID,
    .cert_sn_dev_loc        =
    {
        .zone               = DEVZONE_NONE,
        .slot               = 0,
        .is_genkey          = 0,
        .offset             = 0,
        .count              = 0
    },
    .issue_date_format      = DATEFMT_RFC5280_UTC,
    .expire_date_format     = DATEFMT_RFC5280_GEN,
    .tbs_cert_loc           =
    {
        .offset             = 4,
        .count              = 365
    },
    .expire_years           = 0,
    .public_key_dev_loc     =
    {
        .zone               = DEVZONE_DATA,
        .slot               = 11,
        .is_genkey          = 0,
        .offset             = 0,
        .count              = 72
    },
    .comp_cert_dev_loc      =
    {
        .zone               = DEVZONE_DATA,
        .slot               = 12,
        .is_genkey          = 0,
        .offset             = 0,
        .count              = 72
    },
    .std_cert_elements      =
    {
        {   // STDCERT_PUBLIC_KEY
            .offset         = 223,
            .count          = 64
        },
        {   // STDCERT_SIGNATURE
            .offset         = 381,
            .count          = 73
        },
        {   // STDCERT_ISSUE_DATE
            .offset         = 92,
            .count          = 13
        },
        {   // STDCERT_EXPIRE_DATE
            .offset         = 105,
            .count          = 0
        },
        {   // STDCERT_SIGNER_ID
            .offset         = 192,
            .count          = 4
        },
        {   // STDCERT_CERT_SN
            .offset         = 15,
            .count          = 3
        },
        {   // STDCERT_AUTH_KEY_ID
            .offset         = 349,
            .count          = 20
        },
        {   // STDCERT_SUBJ_KEY_ID
            .offset         = 316,
            .count          = 20
        }
    },
    .cert_elements          = NULL,
    .cert_elements_count    = 0,
    .cert_template          = g_cert_template_1_signer_t,
    .cert_template_size     = sizeof(g_cert_template_1_signer_t),
};

// Device
const uint8_t g_cert_template_0_device_t[] =
{
    0x30, 0x82, 0x01, 0xA8, 0x30, 0x82, 0x01, 0x4E, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x0A, 0x40,
    0x01, 0x23, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xEE, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48,
    0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x48, 0x31, 0x1A, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x0A,
    0x0C, 0x11, 0x41, 0x74, 0x6D, 0x65, 0x6C, 0x20, 0x4F, 0x70, 0x65, 0x6E, 0x53, 0x53, 0x4C, 0x20,
    0x44, 0x65, 0x76, 0x31, 0x2A, 0x30, 0x28, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x21, 0x4F, 0x70,
    0x65, 0x6E, 0x53, 0x53, 0x4C, 0x20, 0x44, 0x65, 0x76, 0x20, 0x41, 0x54, 0x45, 0x43, 0x43, 0x35,
    0x30, 0x38, 0x41, 0x20, 0x53, 0x69, 0x67, 0x6E, 0x65, 0x72, 0x20, 0x30, 0x30, 0x30, 0x32, 0x30,
    0x20, 0x17, 0x0D, 0x31, 0x35, 0x30, 0x37, 0x33, 0x31, 0x30, 0x30, 0x31, 0x32, 0x31, 0x36, 0x5A,
    0x18, 0x0F, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39,
    0x5A, 0x30, 0x43, 0x31, 0x1A, 0x30, 0x18, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x11, 0x41, 0x74,
    0x6D, 0x65, 0x6C, 0x20, 0x4F, 0x70, 0x65, 0x6E, 0x53, 0x53, 0x4C, 0x20, 0x44, 0x65, 0x76, 0x31,
    0x25, 0x30, 0x23, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x1C, 0x4F, 0x70, 0x65, 0x6E, 0x53, 0x53,
    0x4C, 0x20, 0x44, 0x65, 0x76, 0x20, 0x41, 0x54, 0x45, 0x43, 0x43, 0x35, 0x30, 0x38, 0x41, 0x20,
    0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE,
    0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00,
    0x04, 0xC3, 0xDC, 0x86, 0xE9, 0xCC, 0x59, 0xA1, 0xFA, 0xF8, 0xE6, 0x02, 0xB3, 0x44, 0x89, 0xD1,
    0x70, 0x4A, 0x3B, 0x44, 0x04, 0x52, 0xAA, 0x11, 0x93, 0x35, 0xA9, 0xBE, 0x6F, 0x68, 0x32, 0xDC,
    0x59, 0xCE, 0x5E, 0x74, 0x73, 0xB8, 0x44, 0xBD, 0x08, 0x4D, 0x5D, 0x3D, 0xE5, 0xDE, 0x21, 0xC3,
    0x4F, 0x8D, 0xC1, 0x61, 0x4F, 0x17, 0x27, 0xAF, 0x6D, 0xC4, 0x9C, 0x42, 0x83, 0xEE, 0x36, 0xE2,
    0x31, 0xA3, 0x23, 0x30, 0x21, 0x30, 0x1F, 0x06, 0x03, 0x55, 0x1D, 0x23, 0x04, 0x18, 0x30, 0x16,
    0x80, 0x14, 0x69, 0xF4, 0xD5, 0x12, 0x5A, 0x44, 0x49, 0x7A, 0xCC, 0x75, 0x11, 0xF5, 0x4A, 0x30,
    0x86, 0xA5, 0xD4, 0xAB, 0xA3, 0x9C, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04,
    0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x41, 0x5E, 0x13, 0x59, 0x05, 0x08, 0xDA,
    0x03, 0xFD, 0x94, 0x1B, 0xAF, 0xEF, 0x8A, 0x3D, 0xC8, 0x24, 0xE5, 0x49, 0x07, 0xB8, 0xA2, 0xBD,
    0x36, 0x60, 0x26, 0x14, 0x75, 0x27, 0x36, 0x66, 0xE1, 0x02, 0x21, 0x00, 0x96, 0xFF, 0x2B, 0xDF,
    0x34, 0x54, 0x9B, 0x7C, 0x56, 0x8F, 0x54, 0x44, 0x4F, 0xE6, 0xAD, 0x3B, 0xFE, 0x63, 0xBD, 0xD2,
    0x93, 0x65, 0xF2, 0x65, 0x59, 0x22, 0xC6, 0x25, 0x90, 0x7A, 0xEC, 0x19
};

const atcacert_def_t g_cert_def_0_device_t =
{
    .type                   = CERTTYPE_X509,
    .template_id            = 0,
    .chain_id               = 0,
    .private_key_slot       = 0,
    .sn_source              = SNSRC_DEVICE_SN,
    .cert_sn_dev_loc        =
    {
        .zone               = DEVZONE_NONE,
        .slot               = 0,
        .is_genkey          = 0,
        .offset             = 0,
        .count              = 0
    },
    .issue_date_format      = DATEFMT_RFC5280_UTC,
    .expire_date_format     = DATEFMT_RFC5280_GEN,
    .tbs_cert_loc           =
    {
        .offset             = 4,
        .count              = 338
    },
    .expire_years           = 0,
    .public_key_dev_loc     =
    {
        .zone               = DEVZONE_DATA,
        .slot               = 0,
        .is_genkey          = 1,
        .offset             = 0,
        .count              = 64
    },
    .comp_cert_dev_loc      =
    {
        .zone               = DEVZONE_DATA,
        .slot               = 10,
        .is_genkey          = 0,
        .offset             = 0,
        .count              = 72
    },
    .std_cert_elements      =
    {
        {   // STDCERT_PUBLIC_KEY
            .offset         = 241,
            .count          = 64
        },
        {   // STDCERT_SIGNATURE
            .offset         = 354,
            .count          = 73
        },
        {   // STDCERT_ISSUE_DATE
            .offset         = 115,
            .count          = 13
        },
        {   // STDCERT_EXPIRE_DATE
            .offset         = 116,
            .count          = 0
        },
        {   // STDCERT_SIGNER_ID
            .offset         = 107,
            .count          = 4
        },
        {   // STDCERT_CERT_SN
            .offset         = 15,
            .count          = 10
        },
        {   // STDCERT_AUTH_KEY_ID
            .offset         = 322,
            .count          = 20
        },
        {   // STDCERT_SUBJ_KEY_ID
            .offset         = 0,
            .count          = 0
        }
    },
    .cert_elements          = NULL,
    .cert_elements_count    = 0,
    .cert_template          = g_cert_template_0_device_t,
    .cert_template_size     = sizeof(g_cert_template_0_device_t),
};

///////////////////////////////////////////////////////////////////////////////////////
// CSR Structures

uint8_t g_DeviceCsr[] =
{
    0x30, 0x82, 0x01, 0x35, 0x30, 0x81, 0xDC, 0x02, 0x01, 0x00, 0x30, 0x7A, 0x31, 0x0B, 0x30, 0x09,
    0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55,
    0x04, 0x08, 0x0C, 0x08, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x61, 0x64, 0x6F, 0x31, 0x19, 0x30, 0x17,
    0x06, 0x03, 0x55, 0x04, 0x07, 0x0C, 0x10, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x61, 0x64, 0x6F, 0x20,
    0x53, 0x70, 0x72, 0x69, 0x6E, 0x67, 0x73, 0x31, 0x0E, 0x30, 0x0C, 0x06, 0x03, 0x55, 0x04, 0x0A,
    0x0C, 0x05, 0x41, 0x74, 0x6D, 0x65, 0x6C, 0x31, 0x18, 0x30, 0x16, 0x06, 0x03, 0x55, 0x04, 0x0B,
    0x0C, 0x0F, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20, 0x50, 0x72, 0x6F, 0x64, 0x75, 0x63, 0x74,
    0x73, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x0A, 0x41, 0x57, 0x53, 0x20,
    0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE,
    0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00,
    0x04, 0x68, 0x94, 0x9A, 0x24, 0x35, 0xCB, 0xE5, 0x98, 0x3A, 0x35, 0x16, 0xDA, 0x7A, 0x0B, 0x61,
    0x75, 0x8A, 0x63, 0x21, 0xEF, 0x50, 0xE1, 0x54, 0x45, 0x24, 0x11, 0x0D, 0x10, 0xA0, 0x53, 0x1B,
    0x5F, 0x6F, 0x50, 0x0D, 0xBF, 0xBA, 0x0D, 0x01, 0xFA, 0x20, 0x01, 0x4D, 0x59, 0x92, 0xCE, 0xF9,
    0x3F, 0xBB, 0xD2, 0x3D, 0xAA, 0x9A, 0x48, 0xAA, 0x98, 0x6A, 0xDC, 0x3C, 0xC8, 0x97, 0xD3, 0xDD,
    0xAC, 0xA0, 0x00, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03,
    0x48, 0x00, 0x30, 0x45, 0x02, 0x21, 0x00, 0xB8, 0x01, 0x0D, 0xF0, 0xF9, 0x86, 0xD9, 0x33, 0xB6,
    0x03, 0x7C, 0x72, 0x50, 0x64, 0x67, 0x53, 0x77, 0x51, 0x1F, 0xE1, 0x1A, 0x16, 0xEB, 0xFE, 0xD6,
    0x3C, 0x9D, 0xEC, 0x05, 0x35, 0xE5, 0xDD, 0x02, 0x20, 0x70, 0x96, 0x1E, 0x4C, 0x5F, 0xB6, 0x0E,
    0xE2, 0x28, 0xB8, 0x8D, 0x6C, 0xE4, 0x02, 0x63, 0x15, 0x79, 0x7C, 0x6A, 0x29, 0x3F, 0x7E, 0xEB,
    0x48, 0x0F, 0x8F, 0x41, 0x15, 0x92, 0x4B, 0xF4, 0xB5

};


const atcacert_def_t g_DeviceCsrDef =
{
    .type                = CERTTYPE_X509,
    .template_id         = 0,
    .chain_id            = 0,
    .private_key_slot    = 0,
    .sn_source           = SNSRC_DEVICE_SN,
    .cert_sn_dev_loc     =
    {
        .zone            = DEVZONE_NONE,
        .slot            = 0,
        .is_genkey       = 0,
        .offset          = 0,
        .count           = 0
    },
    .issue_date_format   = DATEFMT_RFC5280_UTC,
    .expire_date_format  = DATEFMT_RFC5280_GEN,
    .tbs_cert_loc        =
    {
        .offset          = 4,
        .count           = 223
    },
    .expire_years        = 0,
    .public_key_dev_loc  =
    {
        .zone            = DEVZONE_DATA,
        .slot            = 0,
        .is_genkey       = 1,
        .offset          = 0,
        .count           = 64
    },
    .comp_cert_dev_loc   =
    {
        .zone            = DEVZONE_DATA,
        .slot            = 10,
        .is_genkey       = 0,
        .offset          = 0,
        .count           = 72
    },
    .std_cert_elements   =
    {
        {   // STDCERT_PUBLIC_KEY
            .offset      = 161,
            .count       = 64
        },
        {   // STDCERT_SIGNATURE
            .offset      = 239,
            .count       = 72
        },
        {   // STDCERT_ISSUE_DATE
            .offset      = 0,
            .count       = 0
        },
        {   // STDCERT_EXPIRE_DATE
            .offset      = 0,
            .count       = 0
        },
        {   // STDCERT_SIGNER_ID
            .offset      = 0,
            .count       = 0
        },
        {   // STDCERT_CERT_SN
            .offset      = 0,
            .count       = 0
        },
        {   // STDCERT_AUTH_KEY_ID
            .offset      = 0,
            .count       = 0
        },
        {   // STDCERT_SUBJ_KEY_ID
            .offset      = 0,
            .count       = 0
        }
    },
    .cert_elements       = NULL,
    .cert_elements_count = 0,
    .cert_template       = g_DeviceCsr,
    .cert_template_size  = sizeof(g_DeviceCsr),
};


