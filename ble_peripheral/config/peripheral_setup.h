/**
 ****************************************************************************************
 *
 * @file peripheral_setup.h
 *
 * @brief Peripherals setup header file.
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

#ifndef PERIPHERAL_SETUP_H_
#define PERIPHERAL_SETUP_H_


#include "hw_gpio.h"


/**
 * I2C 1 configuration
 */
#define I2C1_SCL_PORT           ( HW_GPIO_PORT_1 )
#define I2C1_SCL_PIN            ( HW_GPIO_PIN_2  )

#define I2C1_SDA_PORT           ( HW_GPIO_PORT_1 )
#define I2C1_SDA_PIN            ( HW_GPIO_PIN_0  )



#endif /* PERIPHERAL_SETUP_H_ */
