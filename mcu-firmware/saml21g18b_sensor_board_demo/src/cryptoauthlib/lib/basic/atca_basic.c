/**
 * \file
 * \brief CryptoAuthLib Basic API methods. These methods provide a simpler way
 *        to access the core crypto methods.
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


#include "atca_basic.h"
#include "host/atca_host.h"

char atca_version[] = { "20171122" };  // change for each release, yyyymmdd
ATCADevice _gDevice = NULL;
#define MAX_BUSES   4

/** \brief basic API methods are all prefixed with atcab_  (Atmel CryptoAuth Basic)
 *  the fundamental premise of the basic API is it is based on a single interface
 *  instance and that instance is global, so all basic API commands assume that
 *  one global device is the one to operate on.
 */

/** \brief returns a version string for the CryptoAuthLib release.
 *  The format of the version string returned is "yyyymmdd"
 * \param[out] ver_str ptr to space to receive version string
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_version(char *ver_str)
{
    strcpy(ver_str, atca_version);
    return ATCA_SUCCESS;
}


/** \brief atcab_init is called once for the life of the application and creates a global ATCADevice object used by Basic API.
 *  This method builds a global ATCADevice instance behinds the scenes that's used for all Basic API operations
 *  \param[in] cfg      pointer that holds the interface configuration.  This is usually a predefined configuration found in atca_cfgs.h
 *  \return    ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_init(ATCAIfaceCfg *cfg)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;

    if (_gDevice)       // if there's already a device created, release it
    {
        atcab_release();
    }
    _gDevice = newATCADevice(cfg);

    if ((_gDevice == NULL) || (_gDevice->mIface == NULL) || (_gDevice->mCommands == NULL))
    {
        return ATCA_GEN_FAIL;  // Device creation failed

    }
    if (cfg->devtype == ATECC608A)
    {

        if ((status = atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, ATCA_CHIPMODE_OFFSET, &_gDevice->mCommands->clock_divider, 1)) != ATCA_SUCCESS)
        {
            return status;
        }
        _gDevice->mCommands->clock_divider &= ATCA_CHIPMODE_CLOCK_DIV_MASK;
    }

    return ATCA_SUCCESS;
}

/** \brief Initialize the global ATCADevice object to point to one of your
 *         choosing for use with all the atcab_ basic API.
 *  \param[in] ca_device  ATCADevice instance to use as the global Basic API
 *                        crypto device instance
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_init_device(ATCADevice ca_device)
{
    if (ca_device == NULL)
    {
        return ATCA_BAD_PARAM;
    }

    if (atGetCommands(ca_device) == NULL || atGetIFace(ca_device) == NULL)
    {
        return ATCA_GEN_FAIL;
    }

    // if there's already a device created, release it
    if (_gDevice)
    {
        atcab_release();
    }

    _gDevice = ca_device;

    return ATCA_SUCCESS;
}

/** \brief release (free) the global ATCADevice instance.
 *  This must be called in order to release or free up the interface.
 *  \return Returns ATCA_SUCCESS .
 */
ATCA_STATUS atcab_release(void)
{
    deleteATCADevice(&_gDevice);
    return ATCA_SUCCESS;
}

/** \brief Get the global device object.
 *  \return instance of global ATCADevice
 */
ATCADevice atcab_get_device(void)
{
    return _gDevice;
}

/** \brief wakeup the CryptoAuth device
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_wakeup(void)
{
    if (_gDevice == NULL)
    {
        return ATCA_GEN_FAIL;
    }

    return atwake(_gDevice->mIface);
}

/** \brief idle the CryptoAuth device
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_idle(void)
{
    if (_gDevice == NULL)
    {
        return ATCA_GEN_FAIL;
    }

    return atidle(_gDevice->mIface);
}

/** \brief invoke sleep on the CryptoAuth device
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_sleep(void)
{
    if (_gDevice == NULL)
    {
        return ATCA_GEN_FAIL;
    }

    return atsleep(_gDevice->mIface);
}


/** \brief auto discovery of crypto auth devices
 *
 * Calls interface discovery functions and fills in cfg_array up to the maximum
 * number of configurations either found or the size of the array. The cfg_array
 * can have a mixture of interface types (ie: some I2C, some SWI or UART) depending upon
 * which interfaces you've enabled
 *
 * \param[out] cfg_array ptr to an array of interface configs
 * \param[in] max_ifaces maximum size of cfg_array
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */


ATCA_STATUS atcab_cfg_discover(ATCAIfaceCfg cfg_array[], int max_ifaces)
{
    int iface_num = 0, i;
    int found = 0;

// this cumulatively gathers all the interfaces enabled by #defines

#ifdef ATCA_HAL_I2C
    int i2c_buses[MAX_BUSES];
    memset(i2c_buses, -1, sizeof(i2c_buses));
    hal_i2c_discover_buses(i2c_buses, MAX_BUSES);

    for (i = 0; i < MAX_BUSES && iface_num < max_ifaces; i++)
    {
        if (i2c_buses[i] != -1)
        {
            hal_i2c_discover_devices(i2c_buses[i], &cfg_array[iface_num], &found);
            iface_num += found;
        }
    }
#endif

#ifdef ATCA_HAL_SWI
    int swi_buses[MAX_BUSES];
    memset(swi_buses, -1, sizeof(swi_buses));
    hal_swi_discover_buses(swi_buses, MAX_BUSES);
    for (i = 0; i < MAX_BUSES && iface_num < max_ifaces; i++)
    {
        if (swi_buses[i] != -1)
        {
            hal_swi_discover_devices(swi_buses[i], &cfg_array[iface_num], &found);
            iface_num += found;
        }
    }

#endif

#ifdef ATCA_HAL_UART
    int uart_buses[MAX_BUSES];
    memset(uart_buses, -1, sizeof(uart_buses));
    hal_uart_discover_buses(uart_buses, MAX_BUSES);
    for (i = 0; i < MAX_BUSES && iface_num < max_ifaces; i++)
    {
        if (uart_buses[i] != -1)
        {
            hal_uart_discover_devices(uart_buses[i], &cfg_array[iface_num], &found);
            iface_num += found;
        }
    }
#endif

#ifdef ATCA_HAL_KIT_CDC
    int cdc_buses[MAX_BUSES];
    memset(cdc_buses, -1, sizeof(cdc_buses));
    hal_kit_cdc_discover_buses(cdc_buses, MAX_BUSES);
    for (i = 0; i < MAX_BUSES && iface_num < max_ifaces; i++)
    {
        if (cdc_buses[i] != -1)
        {
            hal_kit_cdc_discover_devices(cdc_buses[i], &cfg_array[iface_num++], &found);
            iface_num += found;
        }
    }
#endif

#ifdef ATCA_HAL_KIT_HID
    int hid_buses[MAX_BUSES];
    memset(hid_buses, -1, sizeof(hid_buses));
    hal_kit_hid_discover_buses(hid_buses, MAX_BUSES);
    for (i = 0; i < MAX_BUSES && iface_num < max_ifaces; i++)
    {
        if (hid_buses[i] != -1)
        {
            hal_kit_hid_discover_devices(hid_buses[i], &cfg_array[iface_num++], &found);
            iface_num += found;
        }
    }
#endif
    return ATCA_SUCCESS;
}

/** \brief common cleanup code which idles the device after any operation
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS _atcab_exit(void)
{
    return atcab_idle();
}

/** \brief Compute the address given the zone, slot, block, and offset
 *  \param[in] zone   Zone to get address from. Config(0), OTP(1), or
 *                    Data(2) which requires a slot.
 *  \param[in] slot   Slot Id number for data zone and zero for other zones.
 *  \param[in] block  Block number within the data or configuration or OTP zone .
 *  \param[in] offset Offset Number within the block of data or configuration or OTP zone.
 *  \param[out] addr  Pointer to the address of data or configuration or OTP zone.
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_get_addr(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t* addr)
{
    ATCA_STATUS status = ATCA_SUCCESS;
    uint8_t mem_zone = zone & 0x03;

    if (addr == NULL)
    {
        return ATCA_BAD_PARAM;
    }
    if ((mem_zone != ATCA_ZONE_CONFIG) && (mem_zone != ATCA_ZONE_DATA) && (mem_zone != ATCA_ZONE_OTP))
    {
        return ATCA_BAD_PARAM;
    }
    do
    {
        // Initialize the addr to 00
        *addr = 0;
        // Mask the offset
        offset = offset & (uint8_t)0x07;
        if ((mem_zone == ATCA_ZONE_CONFIG) || (mem_zone == ATCA_ZONE_OTP))
        {
            *addr = block << 3;
            *addr |= offset;
        }
        else     // ATCA_ZONE_DATA
        {
            *addr = slot << 3;
            *addr  |= offset;
            *addr |= block << 8;
        }
    }
    while (0);

    return status;
}

/** \brief Gets the size of the specified zone in bytes.
 *
 * \param[in]  zone  Zone to get size information from. Config(0), OTP(1), or
 *                   Data(2) which requires a slot.
 * \param[in]  slot  If zone is Data(2), the slot to query for size.
 * \param[out] size  Zone size is returned here.
 *
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_get_zone_size(uint8_t zone, uint16_t slot, size_t* size)
{
    ATCA_STATUS status = ATCA_SUCCESS;

    if (size == NULL)
    {
        return ATCA_BAD_PARAM;
    }

    if (_gDevice->mIface->mIfaceCFG->devtype == ATSHA204A)
    {
        switch (zone)
        {
        case ATCA_ZONE_CONFIG: *size = 88; break;
        case ATCA_ZONE_OTP:    *size = 64; break;
        case ATCA_ZONE_DATA:   *size = 32; break;
        default: status = ATCA_BAD_PARAM; break;
        }
    }
    else
    {
        switch (zone)
        {
        case ATCA_ZONE_CONFIG: *size = 128; break;
        case ATCA_ZONE_OTP:    *size = 64; break;
        case ATCA_ZONE_DATA:
            if (slot < 8)
            {
                *size = 36;
            }
            else if (slot == 8)
            {
                *size = 416;
            }
            else if (slot < 16)
            {
                *size = 72;
            }
            else
            {
                status = ATCA_BAD_PARAM;
            }
            break;
        default: status = ATCA_BAD_PARAM; break;
        }
    }

    return status;
}

/** \brief It performs the transmission of the packet to device ,waits for the device execution time and recevies the response from the device.
 *
 * \param[in,out]  packet  pointer to hold the packet to/from device.
 *
 *
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS atcab_execute_command(ATCAPacket* packet)
{
    ATCA_STATUS status;
    ATCACommand ca_cmd = _gDevice->mCommands;
    ATCAIface ca_iface = _gDevice->mIface;

    if ((status = atGetExecTime(packet->opcode, ca_cmd)) != ATCA_SUCCESS)
    {
        return status;
    }
    do
    {
        if ((status = atcab_wakeup()) != ATCA_SUCCESS)
        {
            break;
        }

        // send the command
        if ((status = atsend(ca_iface, (uint8_t*)packet, packet->txsize)) != ATCA_SUCCESS)
        {
            break;
        }

        // delay the appropriate amount of time for command to execute
        atca_delay_ms(ca_cmd->execution_time_msec);

        // receive the response
        if ((status = atreceive(ca_iface, packet->data, &(packet->rxsize))) != ATCA_SUCCESS)
        {
            break;
        }

        // Check response size
        if (packet->rxsize < 4)
        {
            if (packet->rxsize > 0)
            {
                status = ATCA_RX_FAIL;
            }
            else
            {
                status = ATCA_RX_NO_RESPONSE;
            }
            break;
        }

        if ((status = atCheckCrc(packet->data)) != ATCA_SUCCESS)
        {
            break;
        }

        if ((status = isATCAError(packet->data)) != ATCA_SUCCESS)
        {
            break;
        }
    }
    while (0);

    _atcab_exit();
    return status;
}