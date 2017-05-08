/**
 ****************************************************************************************
 *
 * @file i2c_driver.h
 *
 * @brief i2c driver header file
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <black.orca.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */
#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#include "FreeRTOS.h"
#include "hw_i2c.h"
#include "ad_i2c.h"
 

typedef struct i2c_event
{
        OS_EVENT                os_event;
        HW_I2C_ABORT_SOURCE     error;
        bool                    success;
} i2c_event_t;

/**
 * \brief Write data into i2c device.
 *
 * \note
 * low byte of each uint16_t is filled with one-byte valid data, length is bytes of valid data.
 * e.g. write bytes 0x01 0x22 should call
 * uint16_t data[2] = 0x01, 0x22;
 * app_write_i2c(DEVICE_ID, data, 2, pI2cEvent);
 *
 * \param [in]  deviceId         Device macro defined in platform_devices.h, e.g.BH1750.
 * \param [in]  pData            data to write
 * \param [out] val              Pointer of read out data buffer.
 * \param [in]  len              Valid bytes to write.
 * \param [in]  pI2cEvent        Pointer of i2c event, one sensor should have one dedicated event instance, for both write and read.
 *
 * \return result code
 */
bool app_write_i2c(const i2c_device_id deviceId, uint16_t* pData, uint32_t len,
        i2c_event_t* pI2cEvent);


/**
 * \brief Read data from specific i2c device register.
 *
 * \note
 * e.g. write register XX to read out 2 bytes data
 * uint8_t data[2];
 * app_transact_i2c(DEVICE_ID, XX, data, sizeof(data), pI2cEvent, 0);
 *
 * \param [in]  deviceId         Device macro defined in platform_devices.h, e.g.BH1750.
 * \param [in]  reg              i2c device register to read.
 * \param [out] val              Pointer of read out data buffer.
 * \param [in]  len              Bytes to read.
 * \param [in]  pI2cEvent        Pointer of i2c event, one sensor should have one dedicated event instance, for both write and read.
 * \param [in]  waitInMsBeforeReadingData       Some sensors need to wait for specific time after writing
 *                               register address in order to read out data.
 *
 * \return result code
 */
bool app_transact_i2c(const i2c_device_id deviceId, uint8_t reg, uint8_t *val, uint16_t len,
        i2c_event_t* pI2cEvent, uint32_t waitInMsBeforeReadingData);


/**
 * \brief Read data from i2c device.
 *
 * \note
 * e.g. read out 2 bytes data
 * uint8_t data[2];
 * app_read_i2c(DEVICE_ID, data, sizeof(data), pI2cEvent);
 *
 * \param [in]  deviceId         Device macro defined in platform_devices.h, e.g.BH1750.
 * \param [out] val              Pointer of read out data buffer.
 * \param [in]  len              Bytes to read.
 * \param [in]  pI2cEvent        Pointer of i2c event, one sensor should have one dedicated event instance, for both write and read.
 *
 * \return result code
 */
bool app_read_i2c(const i2c_device_id deviceId, uint8_t *val, uint16_t len, i2c_event_t* pI2cEvent);

#endif


