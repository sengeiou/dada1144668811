/**
 ****************************************************************************************
 *
 * @file ble_peripheral_task.c
 *
 * @brief BLE peripheral task
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "osal.h"
#include "sys_watchdog.h"
#include "ble_att.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "ble_gatts.h"
#include "ble_service.h"
#include "ble_uuid.h"
#include "ble_peripheral_config.h"
#if CFG_DEBUG_SERVICE
#include "dlg_debug.h"
#endif /* CFG_DEBUG_SERVICE */
#include "bas.h"
#include "cts.h"
#include "dis.h"
#include "nvds.h"
#ifdef dg_configSUOTA_SUPPORT
#include "dlg_suota.h"
#endif /* dg_configSUOTA_SUPPORT */
#include "sw_version.h"
#include "scps.h"
#include "test.h"
#include "ad_nvms.h"
#include "sensor_task.h"
#include "data.h"
static ble_service_t *test_service;
/*
 * Notification bits reservation
 * bit #0 is always assigned to BLE event queue notification
 */
#define CTS_SET_TIME_NOTIF (1 << 2)
#define BCS_TIMER_NOTIF (1 << 3)

#if defined(RBLE_SENSOR_CTRL_BY_APP)
extern OS_TASK task_sensor_sample;
#endif
/*
 * BLE peripheral advertising data
 */

#if 1
//test_r
static const uint8_t adv_data[] = {
        0x13, GAP_DATA_TYPE_LOCAL_NAME,
        'W', 'B', 'I', 'N', '_', 'T', ' ', 'P', 'e', 'r', 'i', 'p', 'h', 'e', 'r', 'a', 'l',
        0x01
        #ifdef dg_configSUOTA_SUPPORT
        + 0x02
#endif /* dg_configSUOTA_SUPPORT */
        ,
        GAP_DATA_TYPE_UUID16_LIST_INC,
#ifdef dg_configSUOTA_SUPPORT
        0xF5, 0xFE,     // = 0xFEF5 (DIALOG SUOTA UUID)
#endif /* dg_configSUOTA_SUPPORT */        
};

#else
static const uint8_t adv_data[] = {
        0x12, GAP_DATA_TYPE_LOCAL_NAME,
        'D', 'i', 'a', 'l', 'o', 'g', ' ', 'P', 'e', 'r', 'i', 'p', 'h', 'e', 'r', 'a', 'l'
};
#endif

static uint8_t scan_rsp_data[] = {
        0x06, GAP_DATA_TYPE_MANUFACTURER_SPEC, 0xD2, 0x00, 0xB1, 0xB2, 0xB3,
};

/*
 typedef enum {
 ADV_INTERVAL_FAST = 0,
 } adv_setting_t;

 static const struct {
 uint16_t min;
 uint16_t max;
 } adv_intervals[] = {
 [ADV_INTERVAL_FAST] =
 {
 .min = BLE_ADV_INTERVAL_FROM_MS(FAST_ADV_INTERVAL),
 .max = BLE_ADV_INTERVAL_FROM_MS(FAST_ADV_INTERVAL),
 },
 };
 */

typedef enum {
        CONN_INTERVAL_FAST = 0,
        CONN_INTERVAL_SLOW = 1,
} conn_setting_t;

static const struct {
        uint16_t min;
        uint16_t max;
} conn_intervals[] = {
        [CONN_INTERVAL_FAST] =
        {
                .min = BLE_CONN_INTERVAL_FROM_MS(FAST_CONN_INTV_MIN),
                .max = BLE_CONN_INTERVAL_FROM_MS(FAST_CONN_INTV_MAX),
        },
        [CONN_INTERVAL_SLOW] =
        {
                .min = BLE_CONN_INTERVAL_FROM_MS(SLOW_CONN_INTV_MIN),
                .max = BLE_CONN_INTERVAL_FROM_MS(SLOW_CONN_INTV_MAX),
        }
};

typedef struct ble_task_env {
        OS_TASK ble_task_p;
        uint16_t conn_idx;              ///< Connection index
        uint16_t conn_intv;             ///< Connection interval
        uint16_t mtu;
        uint8_t ble2app_id;

} ble_task_env_t;

PRIVILEGED_DATA static ble_task_env_t ble_task_env;

static OS_TASK ble_peripheral_task_handle;

#if CFG_DEBUG_SERVICE
PRIVILEGED_DATA static ble_service_t *dbgs;
#endif /* CFG_DEBUG_SERVICE */

/*
 * Battery Service data
 */
#if CFG_BAS
#if CFG_BAS_MULTIPLE
static const bas_battery_info_t bas_bat1 = {
        .namespace = 0x01,      // Bluetooth SIG
        .descriptor = 0x0106,   // main
        };

static const bas_battery_info_t bas_bat2 = {
        .namespace = 0x01,      // Bluetooth SIG
        .descriptor = 0x0109,   // supplementary
        };
#endif

#endif // CFG_BAS && CFG_BAS_MULTIPLE

/*
 * Current Time Service data
 */
#if CFG_CTS
INITIALISED_PRIVILEGED_DATA static cts_current_time_t cts_time = {
        .date_time = {
                .year = 1970,
                .month = 1,
                .day = 1,
                .hours = 0,
                .minutes = 0,
                .seconds = 0,
        },
        .day_of_week = 4,
        .fractions_256 = 0,
        .adjust_reason = CTS_AR_NO_CHANGE,
};

PRIVILEGED_DATA static OS_TIMER cts_timer;

static void cts_timer_cb(OS_TIMER timer)
{
        cts_time.date_time.seconds++;
        if (cts_time.date_time.seconds > 59) {
                cts_time.date_time.seconds -= 60;
                cts_time.date_time.minutes += 1;
        }
        if (cts_time.date_time.minutes > 59) {
                cts_time.date_time.minutes -= 60;
                cts_time.date_time.hours += 1;
        }
        if (cts_time.date_time.hours > 23) {
                cts_time.date_time.hours -= 24;
        }
}

static void cts_get_time_cb(ble_service_t *svc, uint16_t conn_idx)
{
        cts_current_time_t time = cts_time;

        // for read we return 'no change' adjust reason
        time.adjust_reason = CTS_AR_NO_CHANGE;

        cts_get_time_cfm(svc, conn_idx, ATT_ERROR_OK, &time);
}

static void cts_set_time_cb(ble_service_t *svc, uint16_t conn_idx, const cts_current_time_t *time)
{
#if CFG_DEBUG_SERVICE
        dlgdebug_notify_str(dbgs, conn_idx, "T %04d%02d%02d %02d%02d%02d %d %02x",
                time->date_time.year, time->date_time.month, time->date_time.day,
                time->date_time.hours, time->date_time.minutes, time->date_time.seconds,
                time->day_of_week, time->adjust_reason);
#endif /* CFG_DEBUG_SERVICE */
        if (!cts_is_current_time_valid(time)) {
                cts_set_time_cfm(svc, conn_idx, CTS_ERROR_DATA_FIELD_IGNORED);
                return;
        }

        cts_time = *time;

        cts_set_time_cfm(svc, conn_idx, ATT_ERROR_OK);

        // notify other clients that time has changed
        cts_time.adjust_reason = CTS_AR_MANUAL_TIME_UPDATE;
        OS_TASK_NOTIFY(ble_peripheral_task_handle, CTS_SET_TIME_NOTIF, eSetBits);
}

static void cts_set_local_time_info_cb(ble_service_t *svc, uint16_t conn_idx,
        const cts_local_time_info_t *info)
{
#if CFG_DEBUG_SERVICE
        dlgdebug_notify_str(dbgs, conn_idx, "L tz=%d dst=%d", info->time_zone, info->dst);
#endif /* CFG_DEBUG_SERVICE */
        if (!cts_is_local_time_info_valid(info)) {
                cts_set_local_time_info_cfm(svc, conn_idx, CTS_ERROR_DATA_FIELD_IGNORED);
                return;
        }

        cts_set_local_time_info(svc, info);

        cts_set_local_time_info_cfm(svc, conn_idx, ATT_ERROR_OK);
}

static void cts_get_ref_time_info_cb(ble_service_t *svc, uint16_t conn_idx)
{
        cts_ref_time_info_t rti;

        rti.source = CTS_RTS_MANUAL;
        rti.accuracy = 100;
        rti.days_since_update = 0;
        rti.hours_since_update = 1;

        cts_get_ref_time_info_cfm(svc, conn_idx, ATT_ERROR_OK, &rti);
}

static const cts_callbacks_t cts_callbacks = {
        .get_time = cts_get_time_cb,
        .set_time = cts_set_time_cb,
        .set_local_time_info = cts_set_local_time_info_cb,
        .get_ref_time_info = cts_get_ref_time_info_cb,
};
#endif // CFG_CTS

/*
 * Custom service data
 */
#if CFG_USER_SERVICE
static void myservice_init(ble_service_t *include_svc)
{
        att_uuid_t uuid;

        /*
         * This service does absolutely nothing, it just checks that it's possible to use 128-bit
         * UUIDs for services, characteristics and descriptors.
         */

        ble_uuid_from_string("91a7608d-4456-479d-b9b1-4706e8711cf8", &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, ble_gatts_get_num_attr(1, 1, 1));

        if (include_svc) {
                ble_gatts_add_include(include_svc->start_h, NULL);
        }

        ble_uuid_from_string("25047e64-657c-4856-afcf-e315048a965b", &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_NONE, ATT_PERM_NONE, 1, 0, NULL, NULL);

        ble_uuid_from_string("6b09fe25-eed7-41fc-8da7-1ec89fab7ecb", &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_NONE, 1, 0, NULL);

        ble_gatts_register_service(NULL, 0);
}
#endif // CFG_USER_SERVICE

/*
 * Device Information Service data
 */
#if CFG_DIS
#if CFG_DIS_FULL
static const dis_system_id_t dis_sys_id = {
        .oui = { 0x80, 0xEA, 0xCA },    // Dialog Semiconductor Hellas SA
        .manufacturer = { 0x0A, 0x0B, 0x0C, 0x0D, 0x0E },
};

// copied from "Personal Health Devices Transcoding White Paper", available on bluetooth.org
static const uint8_t dis_reg_cert[] = {
        0x00, 0x02, 0x00, 0x12, 0x02, 0x01, 0x00, 0x08, 0x01,
        0x05, 0x00, 0x01, 0x00, 0x02, 0x80, 0x08, 0x02, 0x02,
        0x00, 0x02, 0x00, 0x00,
};

static const dis_pnp_id_t dis_pnp_id = {
        .vid_source = 0x01,     // Bluetooth SIG
        .vid = 0x00D2,          //  Dialog Semiconductor B.V.
        .pid = 0x0001,
        .version = 0x0001,
};
#endif

static const dis_device_info_t dis_info = {
        .manufacturer = "Dialog Semiconductor",
        .model_number = "Dialog BLE",
        #if CFG_DIS_FULL
        // sample data
        .serial_number = "SN123456",
        .hw_revision = "Rev.D",
        .fw_revision = "1.0",
        .sw_revision = "1.0.0.2",
        .system_id = &dis_sys_id,
        .reg_cert_length = sizeof(dis_reg_cert),
        .reg_cert = dis_reg_cert,
        .pnp_id = &dis_pnp_id,
#endif
        };
#endif // CFG_DIS

/*
 * Scan Parameter Service data
 */
#if CFG_SCPS
static const scps_callbacks_t scps_callbacks = {
};
#endif // CFG_SCPS

/*
 * Debug handlers
 */
#if (CFG_BAS && CFG_DEBUG_SERVICE)
static void dbg_bas_set(uint16_t conn_idx, int argc, char **argv, void *ud)
{
        ble_service_t *svc = ud;
        uint8_t level = atoi(argv[0]);

        bas_set_level(svc, level, true);
}
#endif /* (CFG_BAS && CFG_DEBUG_SERVICE) */

#if (CFG_CTS && CFG_DEBUG_SERVICE)
static void dbg_cts_adjust(uint16_t conn_idx, int argc, char **argv, void *ud)
{
        ble_service_t *svc = ud;
        cts_current_time_t time;

        memset(&time, 0, sizeof(time));

        if (!strcmp(argv[0], "manual")) {
                time.adjust_reason = CTS_AR_MANUAL_TIME_UPDATE;
        } else if (!strcmp(argv[0], "tz")) {
                time.adjust_reason = CTS_AR_CHANGE_OF_TIME_ZONE;
        } else if (!strcmp(argv[0], "dst")) {
                time.adjust_reason = CTS_AR_CHANGE_OF_DST;
        } else if (!strcmp(argv[0], "reftime")) {
                time.adjust_reason = CTS_AR_EXTERNAL_REFERENCE_TIME_UPDATE;
        }

        if (time.adjust_reason != CTS_AR_NO_CHANGE) {
                cts_notify_time_all(svc, &time);
        }
}
#endif /* (CFG_CTS && CFG_DEBUG_SERVICE) */

/**
 * Function to prepare scan response data (replaces the 3 last bytes with the last bytes of BD address)
 */
void app_calc_scan_rsp_data(uint8_t *scan_rsp_data, int scan_rsp_size)
{
        uint8_t *ptr = NULL;
        uint8_t str[] = { 0xB1, 0xB2, 0xB3 };
        uint8_t bd_addr[NVDS_LEN_BD_ADDRESS] = { 0 };
        nvds_tag_len_t bd_addr_len = NVDS_LEN_BD_ADDRESS;

        ptr = memchr(scan_rsp_data, str[0], scan_rsp_size);
        if (!ptr)
                return;

        if (memcmp(ptr, str, sizeof(str)))
                return;

        if (nvds_get(NVDS_TAG_BD_ADDRESS, &bd_addr_len, bd_addr) != NVDS_OK)
                return;

        *ptr = bd_addr[2];
        *(ptr + 1) = bd_addr[1];
        *(ptr + 2) = bd_addr[0];
}

/**
 * Function to set the provided connection parameters setting
 */
static void req_conn_param_update(uint16_t conn_idx, conn_setting_t setting)
{
        gap_conn_params_t params = {
                .interval_max = conn_intervals[setting].max,
                .interval_min = conn_intervals[setting].min,
                .slave_latency = 0,
                .sup_timeout = BLE_SUPERVISION_TMO_FROM_MS(CONN_SUPERVISION_TMO)
        };

        if (ble_task_env.conn_intv < BLE_CONN_INTERVAL_FROM_MS(40) && setting == CONN_INTERVAL_FAST)
                return;

        ble_gap_conn_param_update(conn_idx, &params);

#if CFG_DEBUG_SERVICE
        dlgdebug_notify_str(dbgs, conn_idx,
                "Connection Parameters Update Request, Min: %d.%dms, Max: %d.%dms, SUP: %dms\r\n",
                BLE_CONN_INTERVAL_TO_MS(params.interval_min),
                BLE_CONN_INTERVAL_TO_MS(params.interval_min * 10) % 10,
                BLE_CONN_INTERVAL_TO_MS(params.interval_max),
                BLE_CONN_INTERVAL_TO_MS(params.interval_max * 10) % 10,
                BLE_SUPERVISION_TMO_TO_MS(params.sup_timeout));
#endif /* CFG_DEBUG_SERVICE */
}

/**
 * SUOTA service callbacks
 */
#ifdef dg_configSUOTA_SUPPORT
static bool suota_ready_cb(void)
{
        /*
         * This callback is used so application can accept/block SUOTA.
         * Also, before SUOTA starts, user might want to do some actions
         * e.g. disable sleep mode.
         *
         * If true is returned, then advertising is stopped and SUOTA
         * is started. Otherwise SUOTA is canceled.
         *
         */
        req_conn_param_update(ble_task_env.conn_idx, CONN_INTERVAL_FAST);

        return true;
}

static void suota_status_changed_cb(uint8_t status, uint8_t error_code)
{
        if(status == SUOTA_DONE || status == SUOTA_ERROR)
        req_conn_param_update(ble_task_env.conn_idx, CONN_INTERVAL_SLOW);
}

static const suota_callbacks_t suota_cb = {
        .suota_ready = suota_ready_cb,
        .suota_status = suota_status_changed_cb,
};
#endif /* dg_configSUOTA_SUPPORT */

/*
 * Main code
 */
static void handle_evt_gap_connected(ble_evt_gap_connected_t *evt)
{
        /**
         * Manage connection information
         */
        ble_task_env.conn_idx = evt->conn_idx;
        ble_task_env.conn_intv = evt->conn_params.interval_max;

}

static void handle_evt_gap_adv_completed(ble_evt_gap_adv_completed_t *evt)
{
        // restart advertising so we can connect again
        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
}

typedef struct test_data
{
        //common properties
        uint8_t id1;
        uint8_t id2;
        //properties specific to this sensor
        int16_t val_x;
        int16_t val_y;
        int16_t val_z;
} test_data;

typedef struct ble2app_data
{
        //common properties
        uint8_t id1;
        uint8_t id2;
        uint8_t id3;
} ble2app_data;
uint32_t rble_read_data_addr_offset = 0;
uint32_t rble_read_result_data_addr_offset = 0;
uint32_t t_rble_read_result_data_addr_offset = 0;


extern float orientationFloat[3];
extern int rv_accuracy;
extern int compass_accuracy;
extern bool rble_write_flash_cmd;
extern bool rble_start_cal_cmd;

static void test_rx_data_cb(ble_service_t *svc, uint16_t conn_idx, const uint8_t *value,
        uint16_t length)
{
        printf("wzb %s,rec len=%x\r\n", __func__, length);
        nvms_t nvms_rble_storage_handle;
        nvms_rble_storage_handle = ad_nvms_open(NVMS_IMAGE_DATA_STORAGE_PART);

        uint32_t value_h;
        uint32_t value_cmd;
        int i;
        ble2app_data bd;
        for (i = 0; i < length; i++) {
                printf("%x\r\n", *(value + i));
        }
        test_data td;
        td.id1 = 0x00;
        memset(bd.id1, 0, sizeof(bd.id1));
        memset(bd.id2, 0, sizeof(bd.id1));
        memset(bd.id3, 0, sizeof(bd.id1));

#if 1
        value_h = *(value);
        value_cmd = *(value + 1);
        if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_START_SENSOR_CMD))
                {
                //start_collect_data =true;

#if defined(RBLE_SENSOR_CTRL_BY_APP)
                OS_TASK_NOTIFY(task_sensor_sample, RBLE_SENSOR_START_SAMPLE_NOTIF,
                        OS_NOTIFY_SET_BITS);
#endif
                bd.id1 = 0xFF;
                bd.id2 = 0x01;
                bd.id3 = 0xFF;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0xFF;
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_RECEIVE_DATA_SEND))
                {
                bd.id1 = 0xFF;
                bd.id2 = 0x02;
                bd.id3 = 0xFF;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0x02;          //read all test data
                rble_read_data_addr_offset = 0;
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_STOP_SAMPLE_CMD))
                {

                rble_write_flash_cmd = false;

                bd.id1 = 0xFF;
                bd.id2 = 0x07;
                bd.id3 = 0xFF;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0xFF;
#if defined(RBLE_SENSOR_CTRL_BY_APP)
                OS_TASK_NOTIFY(task_sensor_sample, RBLE_SENSOR_STOP_SAMPLE_NOTIF,
                        OS_NOTIFY_SET_BITS);

#endif
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_SAVE_SAMPLE_CMD))
                {
					bd.id1 = 0xFF;
	                bd.id2 = 0x05;
	                bd.id3 = 0xFF;
	                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                    ble_task_env.ble2app_id = 0xFF;
					rble_write_flash_cmd = true;

        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_START_CAL_CMD))
                {
                rble_start_cal_cmd = true;
                bd.id1 = 0xFF;
                bd.id2 = 0x08;
                bd.id3 = 0xFF;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0xFF;
        }

        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_SEND_RESULT_CMD))
                {
                bd.id1 = 0xFF;
                bd.id2 = 0x09;
                bd.id3 = 0xFF;
                rble_read_result_data_addr_offset=0;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0x09;
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && value_cmd == 0xff) {
                if (length == 3 && (*(value + 2) == 0x01)) {
                        bd.id1 = bd.id2 = 0xff;
                        bd.id3 = 0x00;          //start flag
                        test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                        ble_task_env.ble2app_id = 0x03;
                        t_rble_read_result_data_addr_offset = 0;
                }
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && value_cmd == 0x69) {
                float x = 0, y = 0, z = 0;
                //char float_str[12];
                char float_str[20];
                long cps_acy = (long)compass_accuracy;
                long rv_acy = (long)rv_accuracy;

                memset(float_str, 0, sizeof(float_str));

                //memcpy(float_str,&x,sizeof(x));
                //memcpy(float_str+sizeof(x),&y,sizeof(y));
                //memcpy(float_str+sizeof(x)+sizeof(y),&z,sizeof(z));

                memcpy(float_str, orientationFloat, sizeof(float));
                memcpy(float_str + sizeof(float), orientationFloat + 1, sizeof(float));
                memcpy(float_str + 2 * sizeof(float), orientationFloat + 2, sizeof(float));
                memcpy(float_str + 3 * sizeof(float), &cps_acy, sizeof(long));
                memcpy(float_str + 3 * sizeof(float) + sizeof(long), &rv_acy, sizeof(long));

                //ble2app_data bd;
                bd.id1 = 0xFF;
                bd.id2 = 0x69;
                bd.id3 = cps_acy;				//start flag
                //test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                //test_tx_data(svc, conn_idx, (uint8_t *)float_str, sizeof(float_str));
                ble_task_env.ble2app_id = 0xFF;

        }
#endif

}

static void test_tx_done_cb(ble_service_t *svc, uint16_t conn_idx, uint16_t length)
{

        printf("wzb ble_task_env.ble2app_id=%d\r\n",ble_task_env.ble2app_id);
        if (ble_task_env.ble2app_id == 0x02) {
                nvms_t nvms_rble_storage_handle;
                //uint32_t rble_data_addr_offset=0;
                uint8_t rble_sample_data[20] = { 0 };
                nvms_rble_storage_handle = ad_nvms_open(NVMS_IMAGE_DATA_STORAGE_PART);
                memset(rble_sample_data, 0, sizeof(rble_sample_data));

                if (rble_read_data_addr_offset < RBLE_DATA_PATITION_SIZE)
                {
                        ad_nvms_read(nvms_rble_storage_handle, rble_read_data_addr_offset,
                                rble_sample_data, sizeof(rble_sample_data));
                        test_tx_data(svc, conn_idx, rble_sample_data, sizeof(rble_sample_data));
                        ble_task_env.ble2app_id = 0x02;                    //read all test data
                        rble_read_data_addr_offset += sizeof(rble_sample_data);
                }
                else
                {
                        rble_read_data_addr_offset = 0;
                        //return;
                }
        }
         else if(ble_task_env.ble2app_id == 0x03){
            //for test
           uint8_t rble_sample_result_data[20] = { 0 };
                nvms_t nvms_rble_result_storage_handle;
                nvms_rble_result_storage_handle = ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
                memset(rble_sample_result_data, 0, sizeof(rble_sample_result_data));
                printf("wzb t_rble_read_result_data_addr_offset=%d\r\n",t_rble_read_result_data_addr_offset);
                 if (t_rble_read_result_data_addr_offset != 40)
                                {
                                ad_nvms_read(nvms_rble_result_storage_handle,
                                        t_rble_read_result_data_addr_offset, rble_sample_result_data,
                                        sizeof(rble_sample_result_data));
                    
                          
                                        test_tx_data(svc, conn_idx, rble_sample_result_data,
                                                sizeof(rble_sample_result_data));
                                        ble_task_env.ble2app_id = 0x03;
                                        t_rble_read_result_data_addr_offset +=
                                                sizeof(rble_sample_result_data);
                                
                        }
                        else if (t_rble_read_result_data_addr_offset == 40)
                                {
                                ad_nvms_read(nvms_rble_result_storage_handle,
                                        t_rble_read_result_data_addr_offset, rble_sample_result_data,
                                        8);
                                ble_task_env.ble2app_id = 0xff;
                                test_tx_data(svc, conn_idx, rble_sample_result_data, 8);
                                t_rble_read_result_data_addr_offset += 8;
                                
                        }
        }
        else if (ble_task_env.ble2app_id == 0x09) {

                uint8_t rble_sample_result_data[20] = { 0 };
                nvms_t nvms_rble_result_storage_handle;
                int i;
                bool read_result = false;
                nvms_rble_result_storage_handle = ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
                memset(rble_sample_result_data, 0, sizeof(rble_sample_result_data));
                if (rble_read_result_data_addr_offset < RBLE_DATA_RESULT_PATITION_SIZE)
                        {
                        if (rble_read_result_data_addr_offset != 40)
                                {
                                ad_nvms_read(nvms_rble_result_storage_handle,
                                        rble_read_result_data_addr_offset, rble_sample_result_data,
                                        sizeof(rble_sample_result_data));
                                for (i = 0; i < 20; i++)
                                        {
                                        if (rble_sample_result_data + i != 0xff)
                                                read_result = false;
                                        else
                                                read_result = true;
                                }
                                if (read_result)
                                {
                                         ble2app_data bd;
                                         bd.id1 = 0x65;
                                         bd.id2 = 0x6e;
                                         bd.id3 = 0x64;
                                         test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                                        rble_read_result_data_addr_offset = 0;
                                        ble_task_env.ble2app_id = 0xff;
                                        return;

                                }
                                else
                                {
                                        test_tx_data(svc, conn_idx, rble_sample_result_data,
                                                sizeof(rble_sample_result_data));
                                        ble_task_env.ble2app_id = 0x09;
                                        rble_read_result_data_addr_offset +=
                                                sizeof(rble_sample_result_data);
                                }
                        }
                        else if (rble_read_result_data_addr_offset == 40)
                                {
                                ad_nvms_read(nvms_rble_result_storage_handle,
                                        rble_read_result_data_addr_offset, rble_sample_result_data,
                                        8);
                                test_tx_data(svc, conn_idx, rble_sample_result_data, 8);
                                ble_task_env.ble2app_id = 0x09;
                                rble_read_result_data_addr_offset += 8;
                        }
                }
                else
                {
                        rble_read_result_data_addr_offset = 0;
                        ble2app_data bd;
                        bd.id1 = 0x65;
                        bd.id2 = 0x6e;
                        bd.id3 = 0x64;
                        test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                        ble_task_env.ble2app_id = 0xff;
                        return;
                }


        }

        printf("wzb test_tx_done_cb\r\n");
}

static void test_set_flow_control_cb(ble_service_t *svc, uint16_t conn_idx,
        test_flow_control_t value)
{
        printf("wzb test_set_flow_control_cb\r\n");
        test_set_flow_control(svc, conn_idx, value);
}

static test_callbacks_t test_callbacks = {
        .rx_data = test_rx_data_cb,
        .tx_done = test_tx_done_cb,
        .set_flow_control = test_set_flow_control_cb,
};

void ble_peripheral_task(void *params)
{
        int i = 0;
        for (i = 0; i < 5; i++) {
                printf("wzb SW_VERSION=%s    SW_VERSION_DATE=%s\r\n", BLACKORCA_SW_VERSION,
                        BLACKORCA_SW_VERSION_DATE);
        }

        int8_t wdog_id;
#if CFG_CTS
        ble_service_t *cts;
        cts_local_time_info_t cts_lti = {
                /* Example time zone, should be taken from permanent storage or RTC */
                .dst = CTS_DST_DAYLIGHT_TIME,
                .time_zone = cts_get_time_zone(+3, 0), // UTC + 3 Athens
                };
#endif
#if CFG_SCPS
        ble_service_t *scps;
#endif

#ifdef dg_configSUOTA_SUPPORT
        ble_service_t *suota;
#endif /* dg_configSUOTA_SUPPORT */

        ble_service_t *svc;

        // in case services which do not use svc are all disabled, just surpress -Wunused-variable
        (void)svc;

        /* register ble_peripheral task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        ble_peripheral_task_handle = OS_GET_CURRENT_TASK();

        /* Initialize environmental variables */
        ble_task_env.ble_task_p = OS_GET_CURRENT_TASK();
        ble_task_env.conn_idx = BLE_CONN_IDX_INVALID;
        ble_task_env.ble2app_id = 0xff;

        srand(time(NULL));

        ble_peripheral_start();
        ble_register_app();

        ble_gap_device_name_set("wzb", ATT_PERM_READ);

#if CFG_DEBUG_SERVICE
        /* register debug service */
        dbgs = dlgdebug_init(NULL);
        ble_service_add(dbgs);
#endif /* CFG_DEBUG_SERVICE */

#if CFG_BAS
#if CFG_BAS_MULTIPLE
        /* register BAS (1st instance) */
        svc = bas_init(NULL, &bas_bat1);
        ble_service_add(svc);
#if CFG_DEBUG_SERVICE
        dlgdebug_register_handler(dbgs, "bas", "set", dbg_bas_set, svc);
#endif /* CFG_DEBUG_SERVICE */
        bas_set_level(svc, 90, false);

        /* register BAS (2nd instance) */
        svc = bas_init(NULL, &bas_bat2);
        ble_service_add(svc);
        bas_set_level(svc, 60, false);
#else
        /* register BAS */
        svc = bas_init(NULL, NULL);
        ble_service_add(svc);
#if CFG_DEBUG_SERVICE
        dlgdebug_register_handler(dbgs, "bas", "set", dbg_bas_set, svc);
#endif /* CFG_DEBUG_SERVICE */
        bas_set_level(svc, 90, false);
#endif
#endif

#if CFG_CTS
        /* register CTS */
        cts = cts_init(&cts_lti, &cts_callbacks);
        ble_service_add(cts);
#if CFG_DEBUG_SERVICE
        dlgdebug_register_handler(dbgs, "cts", "adjust", dbg_cts_adjust, cts);
#endif /* CFG_DEBUG_SERVICE */
#endif

#if CFG_USER_SERVICE
        /* register custom service */
#if CFG_CTS
        myservice_init(cts);
#else
        myservice_init(NULL);
#endif
#endif

#if CFG_DIS
        /* register DIS */
        svc = dis_init(NULL, &dis_info);
#endif

#if CFG_SCPS
        /* register ScPS */
        scps = scps_init(&scps_callbacks);
        ble_service_add(scps);
#endif

#ifdef dg_configSUOTA_SUPPORT
        /* register SUOTA Service */
        suota = suota_init(&suota_cb);
        ble_service_add(suota);
#endif /* dg_configSUOTA_SUPPORT */

        //add test service by wzb
        test_service = test_init(&test_callbacks);
        ble_service_add(test_service);

#if CFG_CTS
        /* create timer for CTS, this will be used to update current time every second */
        cts_timer = OS_TIMER_CREATE("cts", portCONVERT_MS_2_TICKS(1000), OS_TIMER_SUCCESS,
                (void *) OS_GET_CURRENT_TASK(), cts_timer_cb);
        OS_ASSERT(cts_timer);
        OS_TIMER_START(cts_timer, OS_TIMER_FOREVER);
#endif

        app_calc_scan_rsp_data(scan_rsp_data, sizeof(scan_rsp_data));
        ble_gap_adv_data_set(sizeof(adv_data), adv_data, sizeof(scan_rsp_data), scan_rsp_data);

        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
        printf("wzb for(;;)\r\n");
        for (;;) {
                OS_BASE_TYPE ret;
                uint32_t notif;

                /* notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                /*
                 * Wait on any of the notification bits, then clear them all
                 */
                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif,
                        OS_TASK_NOTIFY_FOREVER);
                /* Blocks forever waiting for task notification. The return value must be OS_OK */
                OS_ASSERT(ret == OS_OK);

                /* resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                /* notified from BLE manager, can get event */
                if (notif & BLE_APP_NOTIFY_MASK) {
                        ble_evt_hdr_t *hdr;

                        hdr = ble_get_event(false);
                        if (!hdr) {
                                goto no_event;
                        }

                        if (ble_service_handle_event(hdr)) {
                                goto handled;
                        }
                        printf("wzb hdr->evt_code=%d\r\n", hdr->evt_code);
                        switch (hdr->evt_code) {
                        case BLE_EVT_GAP_CONNECTED:
                                handle_evt_gap_connected((ble_evt_gap_connected_t *)hdr);
                                break;
                        case BLE_EVT_GAP_ADV_COMPLETED:
                                handle_evt_gap_adv_completed((ble_evt_gap_adv_completed_t *)hdr);
                                break;
                        case BLE_EVT_GAP_PAIR_REQ:
                                {
                                ble_evt_gap_pair_req_t *evt = (ble_evt_gap_pair_req_t *)hdr;
                                ble_gap_pair_reply(evt->conn_idx, true, evt->bond);
                                break;
                        }
                        default:
                                ble_handle_event_default(hdr);
                                break;
                        }

                        handled:
                        OS_FREE(hdr);

                        no_event:
                        // notify again if there are more events to process in queue
                        if (ble_has_event()) {
                                OS_TASK_NOTIFY(OS_GET_CURRENT_TASK(), BLE_APP_NOTIFY_MASK,
                                        eSetBits);
                        }
                }
#if CFG_CTS
                if (notif & CTS_SET_TIME_NOTIF) {
                        cts_notify_time_all(cts, &cts_time);
                }
#endif
        }
}
