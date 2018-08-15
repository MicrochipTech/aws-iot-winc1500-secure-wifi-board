/**
 * \file
 * \brief Microchip CryptoAuthentication device command builder - this is the main object that builds the command
 * byte strings for the given device.  It does not execute the command.  The basic flow is to call
 * a command method to build the command you want given the parameters and then send that byte string
 * through the device interface.
 *
 * The primary goal of the command builder is to wrap the given parameters with the correct packet size and CRC.
 * The caller should first fill in the parameters required in the ATCAPacket parameter given to the command.
 * The command builder will deal with the mechanics of creating a valid packet using the parameter information.
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
#include <string.h>
#include "atca_command.h"
#include "atca_devtypes.h"

/*Execution times for ATSHA204A supported commands...*/
static const device_execution_time_t device_execution_time_204[] = {
    { ATCA_CHECKMAC,     38},
    { ATCA_DERIVE_KEY,   62},
    { ATCA_GENDIG,       43},
    { ATCA_HMAC,         69},
    { ATCA_INFO,         2},
    { ATCA_LOCK,         24},
    { ATCA_MAC,          35},
    { ATCA_NONCE,        60},
    { ATCA_PAUSE,        2},
    { ATCA_RANDOM,       50},
    { ATCA_READ,         5},
    { ATCA_SHA,          22},
    { ATCA_UPDATE_EXTRA, 12},
    { ATCA_WRITE,        42}
};

/*Execution times for ATECC108A supported commands...*/
static const device_execution_time_t device_execution_time_108[] = {
    { ATCA_CHECKMAC,     13},
    { ATCA_COUNTER,      20},
    { ATCA_DERIVE_KEY,   50},
    { ATCA_GENDIG,       11},
    { ATCA_GENKEY,       115},
    { ATCA_HMAC,         23},
    { ATCA_INFO,         2},
    { ATCA_LOCK,         32},
    { ATCA_MAC,          14},
    { ATCA_NONCE,        29},
    { ATCA_PAUSE,        3},
    { ATCA_PRIVWRITE,    48},
    { ATCA_RANDOM,       23},
    { ATCA_READ,         5},
    { ATCA_SHA,          9},
    { ATCA_SIGN,         60},
    { ATCA_UPDATE_EXTRA, 10},
    { ATCA_VERIFY,       72},
    { ATCA_WRITE,        26}
};

/*Execution times for ATECC508A supported commands...*/
static const device_execution_time_t device_execution_time_508[] = {
    { ATCA_CHECKMAC,     13},
    { ATCA_COUNTER,      20},
    { ATCA_DERIVE_KEY,   50},
    { ATCA_ECDH,         58},
    { ATCA_GENDIG,       11},
    { ATCA_GENKEY,       115},
    { ATCA_HMAC,         23},
    { ATCA_INFO,         2},
    { ATCA_LOCK,         32},
    { ATCA_MAC,          14},
    { ATCA_NONCE,        29},
    { ATCA_PAUSE,        3},
    { ATCA_PRIVWRITE,    48},
    { ATCA_RANDOM,       23},
    { ATCA_READ,         5},
    { ATCA_SHA,          9},
    { ATCA_SIGN,         60},
    { ATCA_UPDATE_EXTRA, 10},
    { ATCA_VERIFY,       72},
    { ATCA_WRITE,        26}
};

/*Execution times for ATECC608A-M0 supported commands...*/
static const device_execution_time_t device_execution_time_608_m0[] = {
    { ATCA_AES,          27},
    { ATCA_CHECKMAC,     40},
    { ATCA_COUNTER,      25},
    { ATCA_DERIVE_KEY,   50},
    { ATCA_ECDH,         60},
    { ATCA_GENDIG,       25},
    { ATCA_GENKEY,       115},
    { ATCA_INFO,         5},
    { ATCA_KDF,          165},
    { ATCA_LOCK,         35},
    { ATCA_MAC,          55},
    { ATCA_NONCE,        20},
    { ATCA_PRIVWRITE,    50},
    { ATCA_RANDOM,       23},
    { ATCA_READ,         5},
    { ATCA_SECUREBOOT,   80},
    { ATCA_SELFTEST,     250},
    { ATCA_SHA,          36},
    { ATCA_SIGN,         115},
    { ATCA_UPDATE_EXTRA, 10},
    { ATCA_VERIFY,       105},
    { ATCA_WRITE,        45}
};

/*Execution times for ATECC608A-M1 supported commands...*/
static const device_execution_time_t device_execution_time_608_m1[] = {
    { ATCA_AES,          27},
    { ATCA_CHECKMAC,     40},
    { ATCA_COUNTER,      25},
    { ATCA_DERIVE_KEY,   50},
    { ATCA_ECDH,         140},
    { ATCA_GENDIG,       35},
    { ATCA_GENKEY,       215},
    { ATCA_INFO,         5},
    { ATCA_KDF,          165},
    { ATCA_LOCK,         35},
    { ATCA_MAC,          55},
    { ATCA_NONCE,        20},
    { ATCA_PRIVWRITE,    50},
    { ATCA_RANDOM,       23},
    { ATCA_READ,         5},
    { ATCA_SECUREBOOT,   151},
    { ATCA_SELFTEST,     590},
    { ATCA_SHA,          42},
    { ATCA_SIGN,         220},
    { ATCA_UPDATE_EXTRA, 10},
    { ATCA_VERIFY,       295},
    { ATCA_WRITE,        45}
};

/*Execution times for ATECC608A-M2 supported commands...*/
static const device_execution_time_t device_execution_time_608_m2[] = {
    { ATCA_AES,          27},
    { ATCA_CHECKMAC,     40},
    { ATCA_COUNTER,      25},
    { ATCA_DERIVE_KEY,   50},
    { ATCA_ECDH,         455},
    { ATCA_GENDIG,       35},
    { ATCA_GENKEY,       630},
    { ATCA_INFO,         5},
    { ATCA_KDF,          165},
    { ATCA_LOCK,         35},
    { ATCA_MAC,          55},
    { ATCA_NONCE,        20},
    { ATCA_PRIVWRITE,    50},
    { ATCA_RANDOM,       23},
    { ATCA_READ,         5},
    { ATCA_SECUREBOOT,   451},
    { ATCA_SELFTEST,     2200},
    { ATCA_SHA,          75},
    { ATCA_SIGN,         665},
    { ATCA_UPDATE_EXTRA, 10},
    { ATCA_VERIFY,       1085},
    { ATCA_WRITE,        45}
};

// full superset of commands goes here

/** \brief ATCACommand CheckMAC method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atCheckMAC(ATCACommand ca_cmd, ATCAPacket *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_CHECKMAC;
    packet->txsize = CHECKMAC_COUNT;
    packet->rxsize = CHECKMAC_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Counter method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atCounter(ATCACommand ca_cmd, ATCAPacket *packet)
{
    if (!atIsECCFamily(ca_cmd->dt) )
    {
        return ATCA_BAD_OPCODE;
    }

    // Set the opcode & parameters
    packet->opcode = ATCA_COUNTER;
    packet->txsize = COUNTER_COUNT;
    packet->rxsize = COUNTER_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand DeriveKey method
 * \param[in] ca_cmd   instance
 * \param[in] packet   pointer to the packet containing the command being built
 * \param[in] has_mac  hasMAC determines if MAC data is present in the packet input
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atDeriveKey(ATCACommand ca_cmd, ATCAPacket *packet, bool has_mac)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_DERIVE_KEY;

    // hasMAC must be given since the packet does not have any implicit information to
    // know if it has a mac or not unless the size is preset
    if (has_mac)
    {
        packet->txsize = DERIVE_KEY_COUNT_LARGE;
    }
    else
    {
        packet->txsize = DERIVE_KEY_COUNT_SMALL;
    }

    packet->rxsize = DERIVE_KEY_RSP_SIZE;
    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand ECDH method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atECDH(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_ECDH;
    packet->txsize = ECDH_COUNT;
    packet->rxsize = ECDH_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Generate Digest method
 * \param[in] ca_cmd         instance
 * \param[in] packet         pointer to the packet containing the command being built
 * \param[in] is_no_mac_key  Should be true if GenDig is being run on a slot that has its SlotConfig.NoMac bit set
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atGenDig(ATCACommand ca_cmd, ATCAPacket *packet, bool is_no_mac_key)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_GENDIG;

    if (packet->param1 == GENDIG_ZONE_SHARED_NONCE) // shared nonce mode
    {
        packet->txsize = GENDIG_COUNT + 32;
    }
    else if (is_no_mac_key)
    {
        packet->txsize = GENDIG_COUNT + 4;  // noMac keys use 4 bytes of OtherData in calculation
    }
    else
    {
        packet->txsize = GENDIG_COUNT;
    }

    packet->rxsize = GENDIG_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Generate Key method
 * \param[in] ca_cmd     instance
 * \param[in] packet    pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atGenKey(ATCACommand ca_cmd, ATCAPacket *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_GENKEY;

    if (packet->param1 & GENKEY_MODE_PUBKEY_DIGEST)
    {
        packet->txsize = GENKEY_COUNT_DATA;
        packet->rxsize = GENKEY_RSP_SIZE_SHORT;
    }
    else
    {
        packet->txsize = GENKEY_COUNT;
        packet->rxsize = GENKEY_RSP_SIZE_LONG;
    }

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand HMAC method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atHMAC(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_HMAC;
    packet->txsize = HMAC_COUNT;
    packet->rxsize = HMAC_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Info method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atInfo(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_INFO;
    packet->txsize = INFO_COUNT;
    packet->rxsize = INFO_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Lock method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atLock(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_LOCK;
    packet->txsize = LOCK_COUNT;
    packet->rxsize = LOCK_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand MAC method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atMAC(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    // variable packet size
    packet->opcode = ATCA_MAC;
    if (!(packet->param1 & MAC_MODE_BLOCK2_TEMPKEY))
    {
        packet->txsize = MAC_COUNT_LONG;
    }
    else
    {
        packet->txsize = MAC_COUNT_SHORT;
    }

    packet->rxsize = MAC_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Nonce method
 * \param[in] ca_cmd   instance
 * \param[in] packet   pointer to the packet containing the command being built
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atNonce(ATCACommand ca_cmd, ATCAPacket *packet)
{
    // Set the opcode & parameters
    // variable packet size
    uint8_t calc_mode = packet->param1 & NONCE_MODE_MASK;

    packet->opcode = ATCA_NONCE;

    if ((calc_mode == NONCE_MODE_SEED_UPDATE || calc_mode == NONCE_MODE_NO_SEED_UPDATE))
    {
        // Calculated nonce mode, 20 byte NumInm
        packet->txsize = NONCE_COUNT_SHORT;
        packet->rxsize = NONCE_RSP_SIZE_LONG; // 32-byte output
    }
    else if (calc_mode == NONCE_MODE_PASSTHROUGH)
    {
        // PAss-through nonce mode
        if ((packet->param1 & NONCE_MODE_INPUT_LEN_MASK) == NONCE_MODE_INPUT_LEN_64)
        {
            // 64 byte NumIn
            packet->txsize = NONCE_COUNT_LONG_64;
        }
        else
        {
            // 32 byte NumIn
            packet->txsize = NONCE_COUNT_LONG;
        }
        packet->rxsize = NONCE_RSP_SIZE_SHORT; // Status-only output
    }
    else
    {
        return ATCA_BAD_PARAM;
    }

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Pause method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atPause(ATCACommand ca_cmd, ATCAPacket *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_PAUSE;
    packet->txsize = PAUSE_COUNT;
    packet->rxsize = PAUSE_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand PrivWrite method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atPrivWrite(ATCACommand ca_cmd, ATCAPacket *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_PRIVWRITE;
    packet->txsize = PRIVWRITE_COUNT;
    packet->rxsize = PRIVWRITE_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Random method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atRandom(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_RANDOM;
    packet->txsize = RANDOM_COUNT;
    packet->rxsize = RANDOM_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Read method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atRead(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_READ;
    packet->txsize = READ_COUNT;

    // variable response size based on read type
    if ((packet->param1 & 0x80) == 0)
    {
        packet->rxsize = READ_4_RSP_SIZE;
    }
    else
    {
        packet->rxsize = READ_32_RSP_SIZE;
    }

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand SecureBoot method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atSecureBoot(ATCACommand ca_cmd, ATCAPacket *packet)
{
    packet->opcode = ATCA_SECUREBOOT;
    packet->txsize = ATCA_CMD_SIZE_MIN;

    //variable transmit size based on mode encoding
    switch (packet->param1 & SECUREBOOT_MODE_MASK)
    {
    case SECUREBOOT_MODE_FULL:
    case SECUREBOOT_MODE_FULL_COPY:
        packet->txsize += (SECUREBOOT_DIGEST_SIZE + SECUREBOOT_SIGNATURE_SIZE);
        break;

    case SECUREBOOT_MODE_FULL_STORE:
        packet->txsize += SECUREBOOT_DIGEST_SIZE;
        break;

    default:
        return ATCA_BAD_PARAM;
        break;
    }
    ;

    //variable response size based on MAC setting
    if (packet->param1 & SECUREBOOT_MODE_ENC_MAC_FLAG)
    {
        packet->rxsize = SECUREBOOT_RSP_SIZE_MAC;
    }
    else
    {
        packet->rxsize = SECUREBOOT_RSP_SIZE_NO_MAC;
    }

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand SHA method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \param[in] write_context_size  the length of the sha write_context data
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atSHA(ATCACommand ca_cmd, ATCAPacket *packet, uint16_t write_context_size)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_SHA;

    switch (packet->param1 & SHA_MODE_MASK)
    {
    case SHA_MODE_SHA256_START:     // START
    case SHA_MODE_HMAC_START:
    case SHA_MODE_SHA256_PUBLIC:
        packet->rxsize = SHA_RSP_SIZE_SHORT;
        packet->txsize = ATCA_CMD_SIZE_MIN;
        break;

    case SHA_MODE_SHA256_UPDATE:                                           // UPDATE
        if (ca_cmd->dt == ATSHA204A)
        {
            packet->rxsize = ATCA_SHA_DIGEST_SIZE + ATCA_PACKET_OVERHEAD;  // ATSHA devices return the digest with this command
        }
        else
        {
            packet->rxsize = SHA_RSP_SIZE_SHORT;
        }
        packet->txsize = ATCA_CMD_SIZE_MIN + packet->param2;
        break;

    case SHA_MODE_SHA256_END:     // END
    case SHA_MODE_HMAC_END:
        packet->rxsize = SHA_RSP_SIZE_LONG;
        // check the given packet for a size variable in param2.  If it is > 0, it should
        // be 0-63, incorporate that size into the packet
        packet->txsize = ATCA_CMD_SIZE_MIN + packet->param2;
        break;

    case SHA_MODE_READ_CONTEXT:
        packet->rxsize = SHA_CONTEXT_MAX_SIZE + ATCA_PACKET_OVERHEAD;
        packet->txsize = ATCA_CMD_SIZE_MIN;
        break;

    case SHA_MODE_WRITE_CONTEXT:
        packet->rxsize = SHA_RSP_SIZE_SHORT;
        packet->txsize = ATCA_CMD_SIZE_MIN + write_context_size;
        break;

    default:
        return ATCA_BAD_PARAM;
    }

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Sign method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atSign(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_SIGN;
    packet->txsize = SIGN_COUNT;

    // could be a 64 or 72 byte response depending upon the key configuration for the KeyID
    packet->rxsize = ATCA_RSP_SIZE_64;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand UpdateExtra method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atUpdateExtra(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_UPDATE_EXTRA;
    packet->txsize = UPDATE_COUNT;
    packet->rxsize = UPDATE_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand ECDSA Verify method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atVerify(ATCACommand ca_cmd, ATCAPacket *packet)
{

    // Set the opcode & parameters
    packet->opcode = ATCA_VERIFY;

    // variable packet size based on mode
    switch (packet->param1 & VERIFY_MODE_MASK)
    {
    case VERIFY_MODE_STORED:
        packet->txsize = VERIFY_256_STORED_COUNT;
        if (packet->param1 & VERIFY_MODE_MAC_FLAG)
        {
            packet->rxsize = VERIFY_RSP_SIZE_MAC;
        }
        else
        {
            packet->rxsize = VERIFY_RSP_SIZE;
        }
        break;

    case VERIFY_MODE_VALIDATE_EXTERNAL:
        packet->txsize = VERIFY_256_EXTERNAL_COUNT;
        packet->rxsize = VERIFY_RSP_SIZE;
        break;

    case VERIFY_MODE_EXTERNAL:
        packet->txsize = VERIFY_256_EXTERNAL_COUNT;
        if (packet->param1 & VERIFY_MODE_MAC_FLAG)
        {
            packet->rxsize = VERIFY_RSP_SIZE_MAC;
        }
        else
        {
            packet->rxsize = VERIFY_RSP_SIZE;
        }
        break;

    case VERIFY_MODE_VALIDATE:
    case VERIFY_MODE_INVALIDATE:
        packet->txsize = VERIFY_256_VALIDATE_COUNT;
        packet->rxsize = VERIFY_RSP_SIZE;
        break;

    default:
        return ATCA_BAD_PARAM;
    }

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand Write method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \param[in] has_mac  Flag to indicate whether a mac is present or not
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atWrite(ATCACommand ca_cmd, ATCAPacket *packet, bool has_mac)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_WRITE;

    packet->txsize = 7;
    if (packet->param1 & ATCA_ZONE_READWRITE_32)
    {
        packet->txsize += ATCA_BLOCK_SIZE;
    }
    else
    {
        packet->txsize += ATCA_WORD_SIZE;
    }
    if (has_mac)
    {
        packet->txsize += WRITE_MAC_SIZE;
    }

    packet->rxsize = WRITE_RSP_SIZE;
    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand AES method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atAES(ATCACommand ca_cmd, ATCAPacket *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_AES;
    packet->txsize = ATCA_CMD_SIZE_MIN;

    if ((packet->param1 & AES_MODE_OP_MASK) == AES_MODE_GFM)
    {
        packet->txsize += ATCA_AES_GFM_SIZE;
    }
    else
    {
        packet->txsize += AES_DATA_SIZE;
    }
    packet->rxsize = AES_RSP_SIZE;
    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief ATCACommand AES method
 * \param[in] ca_cmd   instance
 * \param[in] packet  pointer to the packet containing the command being built
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atSelfTest(ATCACommand ca_cmd, ATCAPacket *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATCA_SELFTEST;
    packet->txsize = ATCA_CMD_SIZE_MIN;
    packet->rxsize = SELFTEST_RSP_SIZE;

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}



/** \brief ATCACommand KDF method
 * \param[in]  ca_cmd          Instance
 * \param[in]  packet          Pointer to the packet containing the command
 *                             being built.
 * \param[out] out_data_size   Expected size of OutData is returned here. Can be
 *                             NULL if not required.
 * \param[out] out_nonce_size  Expected size of OutNonce is returned here. Can
 *                             be NULL if not required.
 * \return ATCA_SUCCESS
 */
ATCA_STATUS atKDF(ATCACommand ca_cmd, ATCAPacket *packet, uint16_t* out_data_size, uint16_t* out_nonce_size)
{
    uint8_t target;
    uint32_t details;

    // Set the opcode & parameters
    packet->opcode = ATCA_KDF;

    // Set TX size
    if ((packet->param1 & KDF_MODE_ALG_MASK) == KDF_MODE_ALG_AES)
    {
        // AES algorithm has a fixed message size
        packet->txsize = ATCA_CMD_SIZE_MIN + KDF_DETAILS_SIZE + AES_DATA_SIZE;
    }
    else
    {
        // All other algorithms encode message size in the last byte of details
        packet->txsize = ATCA_CMD_SIZE_MIN + KDF_DETAILS_SIZE + packet->data[3];
    }

    target = (packet->param1 & KDF_MODE_TARGET_MASK);
    details = ((uint32_t)packet->data[0] <<  0) |
              ((uint32_t)packet->data[1] <<  8) |
              ((uint32_t)packet->data[2] << 16) |
              ((uint32_t)packet->data[3] << 24);

    // Set RX size
    packet->rxsize = ATCA_PACKET_OVERHEAD;
    switch (packet->param1 & KDF_MODE_ALG_MASK)
    {
    case KDF_MODE_ALG_PRF:
        if (target == KDF_MODE_TARGET_OUTPUT || target == KDF_MODE_TARGET_OUTPUT_ENC)
        {
            // KDF result will be returned in the command response
            // Check TargetLen in the Details parameter to find how big the response will be
            if ((details & KDF_DETAILS_PRF_TARGET_LEN_MASK) == KDF_DETAILS_PRF_TARGET_LEN_64)
            {
                packet->rxsize += 64;
            }
            else
            {
                packet->rxsize += 32;
            }
        }
        else if ((details & KDF_DETAILS_PRF_AEAD_MASK) == KDF_DETAILS_PRF_AEAD_MODE3)
        {
            // This AEAD processing mode will return 32 bytes even when the
            // output buffer isn't the target
            packet->rxsize += 32;
        }
        else
        {
            // Only a status message returned
            packet->rxsize += 1;
        }
        break;

    case KDF_MODE_ALG_AES:
    case KDF_MODE_ALG_HKDF:
        if (target == KDF_MODE_TARGET_OUTPUT || target == KDF_MODE_TARGET_OUTPUT_ENC)
        {
            packet->rxsize += 32;  // AES and HKDF always return 32 bytes
        }
        else
        {
            packet->rxsize += 1;   // Only a status message returned
        }
        break;

    default:
        return ATCA_BAD_PARAM;
    }

    if (out_data_size != NULL)
    {
        if (packet->rxsize > ATCA_RSP_SIZE_MIN)
        {
            *out_data_size = packet->rxsize - ATCA_PACKET_OVERHEAD;
        }
        else
        {
            *out_data_size = 0;
        }
    }

    if (target == KDF_MODE_TARGET_OUTPUT_ENC)
    {
        packet->rxsize += OUTNONCE_SIZE; // Encrypted output also supplies OutNonce

        if (out_nonce_size != NULL)
        {
            *out_nonce_size = OUTNONCE_SIZE;
        }
    }
    else if (out_nonce_size != NULL)
    {
        *out_nonce_size = 0;
    }

    atCalcCrc(packet);
    return ATCA_SUCCESS;
}

/** \brief constructor for ATCACommand
 * \param[in] device_type - specifies which set of commands and execution times should be associated with this command object
 * \return ATCACommand instance
 */
ATCACommand newATCACommand(ATCADeviceType device_type)    // constructor
{
    ATCA_STATUS status = ATCA_SUCCESS;
    ATCACommand ca_cmd;

    ca_cmd = (ATCACommand)malloc(sizeof(atca_command));
    ca_cmd->dt = device_type;
    ca_cmd->clock_divider = 0;
    if (status != ATCA_SUCCESS)
    {
        free(ca_cmd);
        ca_cmd = NULL;
    }

    return ca_cmd;
}

/** \brief ATCACommand destructor
 * \param[in] ca_cmd instance of a command object
 */
void deleteATCACommand(ATCACommand *ca_cmd)    // destructor
{
    if (*ca_cmd)
    {
        free((void*)*ca_cmd);
    }

    *ca_cmd = NULL;
}



/** \brief return the typical execution type for the given command
 *  \param[in] opcode  Opcode value of the command
 *  \param[in] ca_cmd  Command object for which the execution times are associated
 *  \return ATCA_SUCCESS
 */
ATCA_STATUS atGetExecTime(uint8_t opcode, ATCACommand ca_cmd)
{
    ATCA_STATUS status = ATCA_SUCCESS;
    const device_execution_time_t *execution_times;
    uint8_t i, no_of_commands;


    switch (ca_cmd->dt)
    {
    case ATSHA204A:
        execution_times = device_execution_time_204;
        no_of_commands = sizeof(device_execution_time_204) / sizeof(device_execution_time_t);
        break;

    case ATECC108A:
        execution_times = device_execution_time_108;
        no_of_commands = sizeof(device_execution_time_108) / sizeof(device_execution_time_t);
        break;

    case ATECC508A:
        execution_times = device_execution_time_508;
        no_of_commands = sizeof(device_execution_time_508) / sizeof(device_execution_time_t);
        break;

    case ATECC608A:
        if (ca_cmd->clock_divider == ATCA_CHIPMODE_CLOCK_DIV_M1)
        {
            execution_times = device_execution_time_608_m1;
            no_of_commands = sizeof(device_execution_time_608_m1) / sizeof(device_execution_time_t);
        }
        else if (ca_cmd->clock_divider == ATCA_CHIPMODE_CLOCK_DIV_M2)
        {
            execution_times = device_execution_time_608_m2;
            no_of_commands = sizeof(device_execution_time_608_m2) / sizeof(device_execution_time_t);
        }
        else
        {
            // Assume default M0 clock divider
            execution_times = device_execution_time_608_m0;
            no_of_commands = sizeof(device_execution_time_608_m0) / sizeof(device_execution_time_t);
        }
        break;

    default:
        no_of_commands = 0;
        execution_times = NULL;
        break;
    }

    ca_cmd->execution_time_msec = UNSUPPORTED;

    for (i = 0; i < no_of_commands; i++)
    {
        if (execution_times[i].opcode == opcode)
        {
            ca_cmd->execution_time_msec = execution_times[i].execution_time_msec;
            break;
        }
    }

    if (ca_cmd->execution_time_msec == UNSUPPORTED)
    {
        status = ATCA_BAD_OPCODE;
    }

    return status;
}


/** \brief Calculates CRC over the given raw data and returns the CRC in
 *         little-endian byte order.
 *
 * \param[in]  length  Size of data not including the CRC byte positions
 * \param[in]  data    Pointer to the data over which to compute the CRC
 * \param[out] crc_le  Pointer to the place where the two-bytes of CRC will be
 *                     returned in little-endian byte order.
 */
void atCRC(size_t length, const uint8_t *data, uint8_t *crc_le)
{
    size_t counter;
    uint16_t crc_register = 0;
    uint16_t polynom = 0x8005;
    uint8_t shift_register;
    uint8_t data_bit, crc_bit;

    for (counter = 0; counter < length; counter++)
    {
        for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
        {
            data_bit = (data[counter] & shift_register) ? 1 : 0;
            crc_bit = crc_register >> 15;
            crc_register <<= 1;
            if (data_bit != crc_bit)
            {
                crc_register ^= polynom;
            }
        }
    }
    crc_le[0] = (uint8_t)(crc_register & 0x00FF);
    crc_le[1] = (uint8_t)(crc_register >> 8);
}


/** \brief This function calculates CRC and adds it to the correct offset in the packet data
 * \param[in] packet Packet to calculate CRC data for
 */

void atCalcCrc(ATCAPacket *packet)
{
    uint8_t length, *crc;

    length = packet->txsize - ATCA_CRC_SIZE;
    // computer pointer to CRC in the packet
    crc = &(packet->txsize) + length;

    // stuff CRC into packet
    atCRC(length, &(packet->txsize), crc);
}


/** \brief This function checks the consistency of a response.
 * \param[in] response pointer to response
 * \return ATCA_SUCCESS on success, otherwise ATCA_RX_CRC_ERROR
 */

ATCA_STATUS atCheckCrc(const uint8_t *response)
{
    uint8_t crc[ATCA_CRC_SIZE];
    uint8_t count = response[ATCA_COUNT_IDX];

    count -= ATCA_CRC_SIZE;
    atCRC(count, response, crc);

    return (crc[0] == response[count] && crc[1] == response[count + 1]) ? ATCA_SUCCESS : ATCA_RX_CRC_ERROR;
}


/** \brief determines if a given device type is a SHA device or a superset of a SHA device
 * \param[in] device_type  Type of device to check for family type
 * \return boolean indicating whether the given device is a SHA family device.
 */

bool atIsSHAFamily(ATCADeviceType device_type)
{
    switch (device_type)
    {
    case ATSHA204A:
    case ATECC108A:
    case ATECC508A:
    case ATECC608A:
        return true;
        break;
    default:
        return false;
        break;
    }
}

/** \brief determines if a given device type is an ECC device or a superset of a ECC device
 * \param[in] device_type  Type of device to check for family type
 * \return boolean indicating whether the given device is an ECC family device.
 */
bool atIsECCFamily(ATCADeviceType device_type)
{
    switch (device_type)
    {
    case ATECC108A:
    case ATECC508A:
    case ATECC608A:
        return true;
        break;
    default:
        return false;
        break;
    }
}

/** \brief checks for basic error frame in data
 * \param[in] data pointer to received data - expected to be in the form of a CA device response frame
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS isATCAError(uint8_t *data)
{
    uint8_t good[4] = { 0x04, 0x00, 0x03, 0x40 };

    if (memcmp(data, good, 4) == 0)
    {
        return ATCA_SUCCESS;
    }

    if (data[0] == 0x04)        // error packets are always 4 bytes long
    {
        switch (data[1])
        {
        case 0x01: // checkmac or verify failed
            return ATCA_CHECKMAC_VERIFY_FAILED;
            break;
        case 0x03: // command received byte length, opcode or parameter was illegal
            return ATCA_PARSE_ERROR;
            break;
        case 0x05: // computation error during ECC processing causing invalid results
            return ATCA_STATUS_ECC;
            break;
        case 0x07: // chip is in self test failure mode
            return ATCA_STATUS_SELFTEST_ERROR;
            break;
        case 0x0f: // chip can't execute the command
            return ATCA_EXECUTION_ERROR;
            break;
        case 0x11: // chip was successfully woken up
            return ATCA_WAKE_SUCCESS;
            break;
        case 0xff: // bad crc found (command not properly received by device) or other comm error
            return ATCA_STATUS_CRC;
            break;
        default:
            return ATCA_GEN_FAIL;
            break;
        }
    }
    else
    {
        return ATCA_SUCCESS;
    }
}

/** @} */
