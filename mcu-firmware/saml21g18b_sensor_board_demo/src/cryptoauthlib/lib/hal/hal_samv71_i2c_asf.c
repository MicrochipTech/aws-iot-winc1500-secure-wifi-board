/**
 * \file
 * \brief ATCA Hardware abstraction layer for SAMV71 I2C over ASF drivers.
 *
 * This code is structured in two parts.  Part 1 is the connection of the ATCA HAL API to the physical I2C
 * implementation. Part 2 is the ASF I2C primitives to set up the interface.
 *
 * Prerequisite: add SERCOM I2C Master Polled support to application in Atmel Studio
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

#include <asf.h>
#include <string.h>
#include <stdio.h>

#include "atca_hal.h"
#include "hal_samv71_i2c_asf.h"
#include "atca_device.h"

/** \defgroup hal_ Hardware abstraction layer (hal_)
 *
 * \brief
 * These methods define the hardware abstraction layer for communicating with a CryptoAuth device
 * using I2C driver of ASF.
 *
   @{ */

/** \brief logical to physical bus mapping structure */
ATCAI2CMaster_t *i2c_hal_data[MAX_I2C_BUSES];   // map logical, 0-based bus number to index
int i2c_bus_ref_ct = 0;                         // total in-use count across buses

/** \brief discover i2c buses available for this hardware
 * this maintains a list of logical to physical bus mappings freeing the application
 * of the a-priori knowledge
 * \param[in] i2c_buses - an array of logical bus numbers
 * \param[in] max_buses - maximum number of buses the app wants to attempt to discover
 * \return ATCA_SUCCESS
 */

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
    /* logical bus numbers 0-2 map to the V71 i2c buses:
       ID_TWIHS0
       ID_TWIHS1
       ID_TWIHS2

       until ASF driver supports timeouts, only the TWIHS0 bus will respond on the SAMV71 Xplained Pro board
       otherwise, driver hangs on waiting for txready.
       for( i = 0; i < MAX_I2C_BUSES && i < max_buses; i++ )
        i2c_buses[i] = i;
     */
    i2c_buses[0] = 0;
#if MAX_I2C_BUSES == 3
    i2c_buses[1] = -1;
    i2c_buses[2] = -1;
#endif
    return ATCA_SUCCESS;
}

/** \brief discover any CryptoAuth devices on a given logical bus number
 * \param[in]  bus_num  Logical bus number on which to look for CryptoAuth devices
 * \param[out] cfg      Pointer to head of an array of interface config structures which get filled in by this method
 * \param[out] found    Number of devices found on this bus
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_discover_devices(int bus_num, ATCAIfaceCfg cfg[], int *found)
{
    ATCAIfaceCfg *head = cfg;
    uint8_t slave_address = 0x01;
    ATCADevice device;
    ATCAIface discover_iface;
    ATCACommand command;
    ATCAPacket packet;
    ATCA_STATUS status;
    uint8_t revs608[1][4] = { { 0x00, 0x00, 0x60, 0x01 } };
    uint8_t revs508[1][4] = { { 0x00, 0x00, 0x50, 0x00 } };
    uint8_t revs108[1][4] = { { 0x80, 0x00, 0x10, 0x01 } };
    uint8_t revs204[3][4] = { { 0x00, 0x02, 0x00, 0x08 },
                              { 0x00, 0x02, 0x00, 0x09 },
                              { 0x00, 0x04, 0x05, 0x00 } };
    int i;

    /** \brief default configuration, to be reused during discovery process */
    ATCAIfaceCfg discoverCfg = {
        .iface_type             = ATCA_I2C_IFACE,
        .devtype                = ATECC508A,
        .atcai2c.slave_address  = 0x07,
        .atcai2c.bus            = bus_num,
        .atcai2c.baud           = 400000,
        .wake_delay             = 800,
        .rx_retries             = 3
    };

    ATCAHAL_t hal;

    if (bus_num < 0)
    {
        return ATCA_COMM_FAIL;
    }

    hal_i2c_init(&hal, &discoverCfg);
    device = newATCADevice(&discoverCfg);
    discover_iface = atGetIFace(device);
    command = atGetCommands(device);

    // iterate through all addresses on given i2c bus
    // all valid 7-bit addresses go from 0x07 to 0x78
    for (slave_address = 0x07; slave_address <= 0x78; slave_address++)
    {
        discoverCfg.atcai2c.slave_address = slave_address << 1;  // turn it into an 8-bit address which is what the rest of the i2c HAL is expecting when a packet is sent

        // wake up device
        // If it wakes, send it a dev rev command.  Based on that response, determine the device type
        // BTW - this will wake every cryptoauth device living on the same bus (ecc508a, sha204a)

        if (hal_i2c_wake(discover_iface) == ATCA_SUCCESS)
        {
            (*found)++;
            memcpy( (uint8_t*)head, (uint8_t*)&discoverCfg, sizeof(ATCAIfaceCfg));

            memset(&packet, 0x00, sizeof(packet));

            // get devrev info and set device type accordingly
            atInfo(command, &packet);
            if ((status = atGetExecTime(packet.opcode, command)) != ATCA_SUCCESS)
            {
                continue;
            }

            // send the command
            if ( (status = atsend(discover_iface, (uint8_t*)&packet, packet.txsize)) != ATCA_SUCCESS)
            {
                printf("packet send error\r\n");
                continue;
            }

            // delay the appropriate amount of time for command to execute
            atca_delay_ms((command->execution_time_msec) + 1);

            // receive the response
            if ( (status = atreceive(discover_iface, &(packet.data[0]), &(packet.rxsize) )) != ATCA_SUCCESS)
            {
                continue;
            }

            if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS)
            {
                continue;
            }

            // determine device type from common info and dev rev response byte strings
            for (i = 0; i < (int)sizeof(revs608) / 4; i++)
            {
                if (memcmp(&packet.data[1], &revs608[i], 4) == 0)
                {
                    discoverCfg.devtype = ATECC608A;
                    break;
                }
            }

            for (i = 0; i < (int)sizeof(revs508) / 4; i++)
            {
                if (memcmp(&packet.data[1], &revs508[i], 4) == 0)
                {
                    discoverCfg.devtype = ATECC508A;
                    break;
                }
            }

            for (i = 0; i < (int)sizeof(revs204) / 4; i++)
            {
                if (memcmp(&packet.data[1], &revs204[i], 4) == 0)
                {
                    discoverCfg.devtype = ATSHA204A;
                    break;
                }
            }

            for (i = 0; i < (int)sizeof(revs108) / 4; i++)
            {
                if (memcmp(&packet.data[1], &revs108[i], 4) == 0)
                {
                    discoverCfg.devtype = ATECC108A;
                    break;
                }
            }

            atca_delay_ms(15);
            // now the device type is known, so update the caller's cfg array element with it
            head->devtype = discoverCfg.devtype;
            head++;
        }

        hal_i2c_idle(discover_iface);
    }

    // hal_i2c_release(&hal);

    return ATCA_SUCCESS;
}

/** \brief
    - this HAL implementation assumes you've included the ASF Twi libraries in your project, otherwise,
    the HAL layer will not compile because the ASF TWI drivers are a dependency *
 */

/** \brief hal_i2c_init manages requests to initialize a physical interface.  it manages use counts so when an interface
 * has released the physical layer, it will disable the interface for some other use.
 * You can have multiple ATCAIFace instances using the same bus, and you can have multiple ATCAIFace instances on
 * multiple i2c buses, so hal_i2c_init manages these things and ATCAIFace is abstracted from the physical details.
 */

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
    int bus = cfg->atcai2c.bus;   // 0-based logical bus number
    ATCAHAL_t *phal = (ATCAHAL_t*)hal;
    twihs_options_t twi_options;

    if (i2c_bus_ref_ct == 0)       // power up state, no i2c buses will have been used
    {
        for (int i = 0; i < MAX_I2C_BUSES; i++)
        {
            i2c_hal_data[i] = NULL;
        }
    }

    i2c_bus_ref_ct++;  // total across buses

    if (bus >= 0 && bus < MAX_I2C_BUSES)
    {
        // if this is the first time this bus and interface has been created, do the physical work of enabling it
        if (i2c_hal_data[bus] == NULL)
        {
            i2c_hal_data[bus] = malloc(sizeof(ATCAI2CMaster_t) );
            i2c_hal_data[bus]->ref_ct = 1;  // buses are shared, this is the first instance

            /* Configure the options of TWI driver */
            twi_options.master_clk = sysclk_get_cpu_hz() / CONFIG_SYSCLK_DIV;
            twi_options.speed = cfg->atcai2c.baud;

            switch (bus)
            {
            case 0: /* Enable the peripheral clock for TWI */
                pmc_enable_periph_clk(ID_TWIHS0);
                if (twihs_master_init(TWIHS0, &twi_options) != TWIHS_SUCCESS)
                {
                    return ATCA_COMM_FAIL;
                }
                i2c_hal_data[bus]->twi_module = (uint32_t)TWIHS0;
                break;

            case 1: /* Enable the peripheral clock for TWI */
                pmc_enable_periph_clk(ID_TWIHS1);
                if (twihs_master_init(TWIHS1, &twi_options) != TWIHS_SUCCESS)
                {
                    return ATCA_COMM_FAIL;
                }
                i2c_hal_data[bus]->twi_module = (uint32_t)TWIHS1;
                break;

            case 2: /* Enable the peripheral clock for TWI */
                pmc_enable_periph_clk(ID_TWIHS2);
                if (twihs_master_init(TWIHS2, &twi_options) != TWIHS_SUCCESS)
                {
                    return ATCA_COMM_FAIL;
                }
                i2c_hal_data[bus]->twi_module = (uint32_t)TWIHS2;
                break;
            }

            // store this for use during the release phase
            i2c_hal_data[bus]->bus_index = bus;
        }
        else
        {
            // otherwise, another interface already initialized the bus, so this interface will share it and any different
            // cfg parameters will be ignored...first one to initialize this sets the configuration
            i2c_hal_data[bus]->ref_ct++;
        }

        phal->hal_data = i2c_hal_data[bus];

        return ATCA_SUCCESS;
    }

    return ATCA_COMM_FAIL;
}

/** \brief HAL implementation of I2C post init
 * \param[in] iface  instance
 * \return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    return ATCA_SUCCESS;
}


/** \brief HAL implementation of I2C send over ASF
 * \param[in] iface     instance
 * \param[in] txdata    pointer to space to bytes to send
 * \param[in] txlength  number of bytes to send
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    Twihs *twihs_device = (Twihs*)(i2c_hal_data[bus]->twi_module);

    twihs_packet_t packet = {
        .addr[0]        = 0,
        .addr[1]        = 0,
        .addr_length    = 0,                              //very important, since cryptoauthdevices do not require addressing;
        .chip           = cfg->atcai2c.slave_address >> 1,
        .buffer         = txdata,
    };

    // for this implementation of I2C with CryptoAuth chips, txdata is assumed to have ATCAPacket format

    // other device types that don't require i/o tokens on the front end of a command need a different hal_i2c_send and wire it up instead of this one
    // this covers devices such as ATSHA204A and ATECCx08A that require a word address value pre-pended to the packet
    // txdata[0] is using _reserved byte of the ATCAPacket
    txdata[0] = 0x03;   // insert the Word Address Value, Command token
    txlength++;         // account for word address value byte.
    packet.length = txlength;

    if (twihs_master_write(twihs_device, &packet) != STATUS_OK)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C receive function for ASF I2C
 * \param[in] iface     instance
 * \param[out] rxdata    pointer to space to receive the data
 * \param[in] rxlength  ptr to expected number of receive bytes to request
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    int retries = cfg->rx_retries;
    int status = !STATUS_OK;
    Twihs *twihs_device = (Twihs*)(i2c_hal_data[bus]->twi_module);

    twihs_packet_t packet = {
        .chip   = cfg->atcai2c.slave_address >> 1, // use 7-bit address
        .buffer = rxdata,
        .length = *rxlength
    };

    while (retries-- > 0 && status != STATUS_OK)
    {
        if (twihs_master_read(twihs_device, &packet) != TWIHS_SUCCESS)
        {
            status = ATCA_COMM_FAIL;
        }
        else
        {
            status = ATCA_SUCCESS;
        }
    }

    if (status != STATUS_OK)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief method to change the bus speed of I2C
 * \param[in] iface  interface on which to change bus speed
 * \param[in] speed  baud rate (typically 100000 or 400000)
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS change_i2c_speed(ATCAIface iface, uint32_t speed)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    Twihs *twihs_device = (Twihs*)(i2c_hal_data[bus]->twi_module);

    // if necessary, revert baud rate to what came in.
    if (twihs_set_speed(twihs_device, speed, sysclk_get_cpu_hz() / CONFIG_SYSCLK_DIV) == FAIL)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief wake up CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to wakeup
 * \return ATCA_SUCCESS on success, otherwise an error code.

 */

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    uint16_t rxlength;
    uint32_t bdrt = cfg->atcai2c.baud;
    int status = !STATUS_OK;
    uint8_t data[4], expected[4] = { 0x04, 0x11, 0x33, 0x43 };
    Twihs *twihs_device = (Twihs*)(i2c_hal_data[bus]->twi_module);

    if (bdrt != 100000)    // if not already at 100KHz, change it

    {
        if (twihs_set_speed(twihs_device, 100000, sysclk_get_cpu_hz() / CONFIG_SYSCLK_DIV) == FAIL)
        {
            return ATCA_COMM_FAIL;
        }
    }

    twihs_packet_t packet = {
        .addr[0]        = 0,
        .addr[1]        = 0,
        .addr_length    = 0,                              //very important, since cryptoauthdevices do not require addressing;
        .chip           = cfg->atcai2c.slave_address >> 1,
        .buffer         =  &data[0],
        .length         = 1
    };

    twihs_master_write(twihs_device, &packet);

    atca_delay_us(cfg->wake_delay);   // wait tWHI + tWLO which is configured based on device type and configuration structure

    // look for wake response
    rxlength = 4;
    memset(data, 0x00, rxlength);
    status = hal_i2c_receive(iface, data, &rxlength);

    // if necessary, revert baud rate to what came in.
    if (bdrt != 100000)
    {
        if (twihs_set_speed(twihs_device, bdrt, sysclk_get_cpu_hz() / CONFIG_SYSCLK_DIV) == FAIL)
        {
            return ATCA_COMM_FAIL;
        }
    }

    if (status != STATUS_OK)
    {
        return ATCA_COMM_FAIL;
    }

    if (memcmp(data, expected, 4) == 0)
    {
        return ATCA_SUCCESS;
    }

    return ATCA_COMM_FAIL;
}


/** \brief idle CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to idle
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    uint8_t data[4];
    Twihs *twihs_device = (Twihs*)(i2c_hal_data[bus]->twi_module);

    data[0] = 0x02;  // idle word address value

    twihs_packet_t packet = {
        .addr[0]        = 0,
        .addr[1]        = 0,
        .addr_length    = 0,                              //very important, since cryptoauthdevices do not require addressing;
        .chip           = cfg->atcai2c.slave_address >> 1,
        .buffer         = data,
    };

    packet.length = 1;

    if (twihs_master_write(twihs_device, &packet) != STATUS_OK)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief sleep CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to sleep
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int bus = cfg->atcai2c.bus;
    uint8_t data[4];
    Twihs *twihs_device = (Twihs*)(i2c_hal_data[bus]->twi_module);

    data[0] = 0x01;  // sleep word address value

    twihs_packet_t packet = {
        .addr[0]        = 0,
        .addr[1]        = 0,
        .addr_length    = 0,                              //very important, since cryptoauthdevices do not require addressing;
        .chip           = cfg->atcai2c.slave_address >> 1,
        .buffer         = data,
    };

    packet.length = 1;

    if (twihs_master_write(twihs_device, &packet) != STATUS_OK)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief manages reference count on given bus and releases resource if no more refences exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS
 */

ATCA_STATUS hal_i2c_release(void *hal_data)
{
    ATCAI2CMaster_t *hal = (ATCAI2CMaster_t*)hal_data;
    int bus = hal->bus_index;
    Twihs *twihs_device = (Twihs*)(i2c_hal_data[bus]->twi_module);

    i2c_bus_ref_ct--;  // track total i2c bus interface instances for consistency checking and debugging

    // if the use count for this bus has gone to 0 references, disable it.  protect against an unbracketed release
    if (hal && --(hal->ref_ct) <= 0 && i2c_hal_data[bus] != NULL)
    {
        twihs_disable_master_mode(twihs_device);
        free(i2c_hal_data[hal->bus_index]);
        i2c_hal_data[hal->bus_index] = NULL;
    }

    return ATCA_SUCCESS;
}

/** @} */
