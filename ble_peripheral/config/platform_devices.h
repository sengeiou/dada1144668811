/**
 ****************************************************************************************
 *
 * @file platfrom_devices.h
 *
 * @brief Configuration of devices connected to board
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

#ifndef PLATFORM_DEVICES_H_
#define PLATFORM_DEVICES_H_


#include <ad_i2c.h>
#include <ad_gpadc.h>


#include "peripheral_setup.h"


/* Platform devices configuration */
#ifdef __cplusplus
extern "C" {
#endif

/* GPIO pins configuration array */
static const gpio_config shoes_gpio_cfg[] = {

        /* I2C1 */
        HW_GPIO_PINCONFIG(I2C1_SCL_PORT        , I2C1_SCL_PIN        , OUTPUT_OPEN_DRAIN, I2C_SCL , false),
        HW_GPIO_PINCONFIG(I2C1_SDA_PORT        , I2C1_SDA_PIN        , OUTPUT_OPEN_DRAIN, I2C_SDA , false),
        HW_GPIO_PINCONFIG_END // important!!!
};




#define ICM20648_ACC_ADDR    (0x68)
///(0x69)

#define ICM20648_GRY_ADDR    (0x68)
#define AK09916_MAG_ADDR    (0x0C)


I2C_BUS(I2C1)
	I2C_SLAVE_DEVICE_DMA(I2C1, ICM20648_ACC  , ICM20648_ACC_ADDR  , HW_I2C_ADDRESSING_7B, HW_I2C_SPEED_STANDARD    , HW_DMA_CHANNEL_2);
	I2C_SLAVE_DEVICE_DMA(I2C1, ICM20648_GRY  , ICM20648_GRY_ADDR  , HW_I2C_ADDRESSING_7B, HW_I2C_SPEED_STANDARD    , HW_DMA_CHANNEL_2);
	I2C_SLAVE_DEVICE_DMA(I2C1, AK09916_MAG	 , AK09916_MAG_ADDR   , HW_I2C_ADDRESSING_7B, HW_I2C_SPEED_STANDARD    , HW_DMA_CHANNEL_2);
I2C_BUS_END




I2C_BUS(I2C2)
I2C_BUS_END

#if dg_configGPADC_ADAPTER

/*
 * Define sources connected to GPADC
 */

GPADC_SOURCE(TEMP_SENSOR, HW_GPADC_CLOCK_INTERNAL, HW_GPADC_INPUT_MODE_SINGLE_ENDED,
                                HW_GPADC_INPUT_SE_TEMPSENS, 5, false, HW_GPADC_OVERSAMPLING_1_SAMPLE,
                                HW_GPADC_INPUT_VOLTAGE_UP_TO_1V2)

GPADC_SOURCE(BATTERY_LEVEL, HW_GPADC_CLOCK_INTERNAL, HW_GPADC_INPUT_MODE_SINGLE_ENDED,
                                HW_GPADC_INPUT_SE_VBAT, 15, true, HW_GPADC_OVERSAMPLING_16_SAMPLES,
                                HW_GPADC_INPUT_VOLTAGE_UP_TO_1V2)

#endif /* dg_configGPADC_ADAPTER */


#ifdef __cplusplus
extern }
#endif

#endif /* PLATFORM_DEVICES_H_ */
