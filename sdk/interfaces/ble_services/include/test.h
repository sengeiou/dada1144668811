/**
 ****************************************************************************************
 *
 * @file test.h
 *
 * @brief Serial Port Service sample implementation API
 *
 * Copyright (C) 2015. Dialog Semiconductor, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor.  All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <black.orca.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef TEST_H_
#define TEST_H_

#include "ble_service.h"

/**
 * TEST Flow Control flags values
 */
typedef enum {
        TEST_FLOW_CONTROL_ON = 0x01,
        TEST_FLOW_CONTROL_OFF = 0x02,
} test_flow_control_t;

typedef void (* test_set_flow_control_cb_t) (ble_service_t *svc, uint16_t conn_idx, test_flow_control_t value);

typedef void (* test_rx_data_cb_t) (ble_service_t *svc, uint16_t conn_idx, const uint8_t *value,
                                                                                uint16_t length);

typedef void (* test_tx_done_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint16_t length);

/**
 * TEST application callbacks
 */
typedef struct {
        /** Remote client wrote new value of flow control characteristic */
        test_set_flow_control_cb_t set_flow_control;
        /** Data received from remote client */
        test_rx_data_cb_t          rx_data;
        /** Service finished TX transaction */
        test_tx_done_cb_t          tx_done;
} test_callbacks_t;

/**
 * \brief Register Serial Port Service instance
 *
 * Function registers TEST instance
 *
 * \param [in] cb               application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *test_init(test_callbacks_t *cb);

/**
 * \brief Set flow control value
 *
 * Function updates flow control value.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] value            flow control value
 *
 */
void test_set_flow_control(ble_service_t *svc, uint16_t conn_idx, test_flow_control_t value);

/**
 * \brief TX data available
 *
 * Function notifies new data is available for client. After sending data, service
 * will call tx_done callback.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] data             tx data
 * \param [in] length           tx data length
 *
 */
void test_tx_data(ble_service_t *svc, uint16_t conn_idx, uint8_t *data, uint16_t length);

#endif /* TEST_H_ */
