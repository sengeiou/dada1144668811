/**
 ****************************************************************************************
 *
 * @file i2c_driver.c
 *
 * @brief i2c driver source file
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
#include "FreeRTOS.h"
#include "osal.h"
#include "hw_i2c.h"
#include "i2c_driver.h"
#include "ad_i2c.h"
#include <hw_dma.h>
 
#define I2C_ACCESS_WAIT_TIME_OUT_IN_OS_TICKS    OS_TIME_TO_TICKS(2000)

static __RETAINED_CODE void i2c_dma_transfer_done(HW_I2C_ID id, void *user_data, uint16_t len ,bool success)
{
        i2c_event_t* pEvent = (i2c_event_t *)user_data;
        if (success == false)
        {
                pEvent->error = hw_i2c_get_abort_source(id);
        }
        pEvent->success = success;

        OS_EVENT_SIGNAL_FROM_ISR(pEvent->os_event);
}

bool app_write_i2c(const i2c_device_id deviceId, uint16_t* pData, uint32_t length,
        i2c_event_t* pI2cEvent)
{
        bool ret = true;

        /*
         * The first writing byte informs to which register rest data will be written.
         */
        i2c_device dev = ad_i2c_open(deviceId);
        ad_i2c_device_acquire(dev);
        ad_i2c_bus_acquire(dev);

        i2c_device_config *pI2cDevice = (i2c_device_config *)dev;
        OS_EVENT_WAIT(pI2cDevice->bus_data->event, OS_EVENT_NO_WAIT);
        pI2cEvent->os_event = pI2cDevice->bus_data->event;
        pI2cEvent->error = 0;
        pI2cEvent->success = false;

        hw_i2c_write_buffer_dma_ex(pI2cDevice->bus_id, pI2cDevice->dma_channel, pData, length,
                i2c_dma_transfer_done, pI2cEvent, true);

        BaseType_t retValue_debug = OS_EVENT_WAIT(pI2cDevice->bus_data->event,
        I2C_ACCESS_WAIT_TIME_OUT_IN_OS_TICKS);
        if (OS_EVENT_SIGNALED != retValue_debug)
        {
                hw_dma_channel_stop(pI2cDevice->dma_channel + 1);
                ret = false;
        }
        else
        {
                ret = pI2cEvent->success;
        }

        ad_i2c_bus_release(dev);
        ad_i2c_device_release(dev);
        ad_i2c_close(dev);

        return ret;
}

bool app_transact_i2c(const i2c_device_id deviceId, uint8_t reg, uint8_t *val, uint16_t len,
        i2c_event_t* pI2cEvent, uint32_t waitInMsBeforeReadingData)
{
        bool ret = true;

        i2c_device dev = ad_i2c_open(deviceId);
        ad_i2c_device_acquire(dev);
        ad_i2c_bus_acquire(dev);

        i2c_device_config *pI2cDevice = (i2c_device_config *)dev;
        OS_EVENT_WAIT(pI2cDevice->bus_data->event, OS_EVENT_NO_WAIT);
        pI2cEvent->os_event = pI2cDevice->bus_data->event;
        pI2cEvent->error = 0;
        pI2cEvent->success = false;

        vPortEnterCritical();
        hw_i2c_write_byte(pI2cDevice->bus_id, reg);
        vPortExitCritical();
        OS_DELAY_MS(waitInMsBeforeReadingData);
        hw_i2c_read_buffer_dma_ex(pI2cDevice->bus_id, pI2cDevice->dma_channel, val, len,
                i2c_dma_transfer_done, pI2cEvent);

        BaseType_t retValue_debug = OS_EVENT_WAIT(pI2cDevice->bus_data->event,
        I2C_ACCESS_WAIT_TIME_OUT_IN_OS_TICKS);
        if (OS_EVENT_SIGNALED != retValue_debug)
        {
                hw_dma_channel_stop(pI2cDevice->dma_channel);
                ret = false;
        }
        else
        {
                ret = pI2cEvent->success;
        }

        ad_i2c_bus_release(dev);
        ad_i2c_device_release(dev);
        ad_i2c_close(dev);

        return ret;
}

bool app_read_i2c(const i2c_device_id deviceId, uint8_t *val, uint16_t len, i2c_event_t* pI2cEvent)
{
        bool ret = true;

        i2c_device dev = ad_i2c_open(deviceId);
        ad_i2c_device_acquire(dev);
        ad_i2c_bus_acquire(dev);

        i2c_device_config *pI2cDevice = (i2c_device_config *)dev;
        OS_EVENT_WAIT(pI2cDevice->bus_data->event, OS_EVENT_NO_WAIT);
        pI2cEvent->os_event = pI2cDevice->bus_data->event;
        pI2cEvent->error = 0;
        pI2cEvent->success = false;

        hw_i2c_read_buffer_dma_ex(pI2cDevice->bus_id, pI2cDevice->dma_channel, val, len,
                i2c_dma_transfer_done, pI2cEvent);

        BaseType_t retValue_debug = OS_EVENT_WAIT(pI2cDevice->bus_data->event,
        I2C_ACCESS_WAIT_TIME_OUT_IN_OS_TICKS);
        if (OS_EVENT_SIGNALED != retValue_debug)
                {
                hw_dma_channel_stop(pI2cDevice->dma_channel);
                ret = false;
        }
        else
        {
                ret = pI2cEvent->success;
        }

        ad_i2c_bus_release(dev);
        ad_i2c_device_release(dev);
        ad_i2c_close(dev);

        return ret;
}

