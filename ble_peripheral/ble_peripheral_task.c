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
#include "ad_ble.h"

#include "sensor_task.h"
#include "data.h"
#if defined(RBLE_BAT_MEASURE)
#include "ad_battery.h"
#endif


#if defined(CUSTOM_CONFIG_SERIAL_NUMBER_DEFINE)
#include "ad_nvparam.h"
#include "platform_nvparam.h"
#endif

static ble_service_t *test_service;
static void reboot(void);
static int write_mac_addr(const void *data);
static int write_sn(const void *data);
static int erase_result_part(size_t size);
static void test_flash();
static void read_test();

static int read_result_id();
static void set_result_id(uint8_t id);

#if defined(CUSTOM_REBOOT_VOL_RECORD)
static void save_cur_sys_vol_value(void);
static void update_reboot_times(void);
static void reset_reboot_times(void);
#endif

/*
 * Notification bits reservation
 * bit #0 is always assigned to BLE event queue notification
 */
#define CTS_SET_TIME_NOTIF (1 << 2)
#define BCS_TIMER_NOTIF (1 << 3)

#if defined(RBLE_SENSOR_CTRL_BY_APP)
extern OS_TASK task_sensor_sample;
#endif

extern acc_gyr_ori[5];
/*
 * BLE peripheral advertising data
 */

#if defined(RBLE_BAT_MEASURE)

/* Timer used for battery monitoring */
PRIVILEGED_DATA static OS_TIMER bas_tim;
PRIVILEGED_DATA static ble_service_t *bas;

#endif

#if 1
#if defined(CUSTOM_CONFIG_SERIAL_NUMBER_DEFINE)
//#define SERIAL_NUMBER_NAME		"AS0170518ECG0001"
#define SERIAL_NUMBER_INVAID	"AS017518ECG0004"//"AS017518ECG####"
#define MODEL_TYPY_STR 			"AS0"
#define SERIAL_NUMBER_LEN	16
char serial_number[SERIAL_NUMBER_LEN];

#endif
static const uint8_t adv_data[] = {
        0x11, GAP_DATA_TYPE_LOCAL_NAME,
        'S', 'm', 'a', 'r', 't', ' ', 'B', 'a','s', 'k', 'e', 't', 'b', 'a', 'l', 'l',
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

#if 0
static uint8_t scan_rsp_data[] = {
        0x06, GAP_DATA_TYPE_MANUFACTURER_SPEC, 0xD2, 0x00, 0xB1, 0xB2, 0xB3,
};
#endif
static uint8_t scan_rsp_data[] = {
        0x16, GAP_DATA_TYPE_MANUFACTURER_SPEC, 0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,0xB1, 0xB2, 0xB3,0xb4,0xb5,0xb6
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

#if defined(CUSTOM_CONNECTION)
static const struct {
        uint16_t min;
        uint16_t max;
} adv_intervals[2] = {
        // "fast connection" interval values
        {
                .min = BLE_ADV_INTERVAL_FROM_MS(20),      // 20ms
                .max = BLE_ADV_INTERVAL_FROM_MS(30),      // 30ms

        },
        // "reduced power" interval values
        {
                 .min = BLE_ADV_INTERVAL_FROM_MS(800),    // 800ms
                 .max = BLE_ADV_INTERVAL_FROM_MS(1000),    // 1000ms
                // .min = BLE_ADV_INTERVAL_FROM_MS(1000),    // 1000ms
               // .max = BLE_ADV_INTERVAL_FROM_MS(1500),    // 1500ms
               //  .min = BLE_ADV_INTERVAL_FROM_MS(4000),    // 4000ms
              //   .max = BLE_ADV_INTERVAL_FROM_MS(4000),    // 4000ms
        },

                 // "normal" interval values
        {
                .min = BLE_ADV_INTERVAL_FROM_MS(330),    // 1000ms
                .max = BLE_ADV_INTERVAL_FROM_MS(330),    // 1500ms
        }
};

typedef enum {
        ADV_INTERVAL_FAST = 0,
        ADV_INTERVAL_POWER = 1,
        ADV_INTERVAL_NORMAL = 2,
} adv_setting_t;

#define ADV_TMO_NOTIF   (1 << 9)

#define FLASH_TEST_TMO_NOTIF   (1 << 8)
PRIVILEGED_DATA static OS_TIMER flash_test_tim;

/* Timer used to switch from "fast connection" to "reduced power" advertising intervals */
PRIVILEGED_DATA static OS_TIMER adv_tim;

#endif

#if defined(STOP_SENSOR_AFTER_THREE_HOUR)
PRIVILEGED_DATA static OS_TIMER stop_sensor_tim;
#define STOP_SENSOR_TMO_NOTIF   (1 << 13)

static void stop_sensor_tim_cb(OS_TIMER timer)
{
    OS_TASK task = (OS_TASK)OS_TIMER_GET_TIMER_ID(timer);

    OS_TASK_NOTIFY(task, STOP_SENSOR_TMO_NOTIF, OS_NOTIFY_SET_BITS);
    printf("stop_sensor_tim_cb\r\n");
}
#endif

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
        int ble2app_id;
        uint8_t test_rx_data_id;

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



#if defined(RBLE_BAT_MEASURE)
#if dg_configUSE_SOC
static uint8_t read_battery_level(void)
{
        int16_t level;

        /*
         * The return value from soc_get_soc is from 0(0%) to 1000(100.0%).
         * The input parameter of bas_set_level is from 0(0%) to 100(100%).
         */
        level = (socf_get_soc() + 5) / 10;

#if 1  //defined(RBLE_UART_DEBUG)
		printf("read_battery_level level=%d\n",level);
        fflush(stdout);

#endif


        return level;
}
#else
/*
 * The values depend on the battery type.
 * MIN_BATTERY_LEVEL (in mVolts) must correspond to dg_configBATTERY_LOW_LEVEL (in ADC units)
 */
//#define MAX_BATTERY_LEVEL 3400
#define MAX_BATTERY_LEVEL 3200
//#define MIN_BATTERY_LEVEL 2000
#define MIN_BATTERY_LEVEL 2600

static uint8_t bat_level(uint16_t voltage)
{
        if (voltage >= MAX_BATTERY_LEVEL) {
                return 100;
        } else if (voltage <= MIN_BATTERY_LEVEL) {
                return 0;
        }

        /*
         * For demonstration purposes discharging (Volt vs Capacity) is approximated
         * by a linear function. The exact formula depends on the specific battery being used.
         */
        return (uint8_t) ((int) (voltage - MIN_BATTERY_LEVEL) * 100 /
                (MAX_BATTERY_LEVEL - MIN_BATTERY_LEVEL));
}

static uint8_t read_battery_level(void)
{
        uint8_t level;

        battery_source bat = ad_battery_open();
        uint16_t bat_voltage = ad_battery_raw_to_mvolt(bat, ad_battery_read(bat));


        level = bat_level(bat_voltage);
        ad_battery_close(bat);

		#if 1  //defined(RBLE_UART_DEBUG)
				printf("read_battery_level level=%d,bat_voltage=%d\n",level,bat_voltage);
				fflush(stdout);

		#endif

        return level;
}
#endif

static void bas_update(void)
{
        uint8_t level;

        level = read_battery_level();

        bas_set_level(bas, level, true);
#if defined(CUSTOM_REBOOT_VOL_RECORD)
		save_cur_sys_vol_value();
#endif
}


static void bas_tim_cb(OS_TIMER timer)
{
        OS_TASK task = (OS_TASK)OS_TIMER_GET_TIMER_ID(timer);

        OS_TASK_NOTIFY(task, RBLE_BAS_TMO_NOTIF, OS_NOTIFY_SET_BITS);
}
#endif

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

        //test
        char sn_uuid[37];
        int i=0;
        for(i=0;i<4;i++){
            sn_uuid[i]='0';
        }
        sn_uuid[4]=serial_number[2];
        sn_uuid[5]='0';
        sn_uuid[6]=serial_number[3];
        sn_uuid[7]='0';

        sn_uuid[8]='-';

        sn_uuid[9]=serial_number[4];
        sn_uuid[10]='0';
        sn_uuid[11]=serial_number[5];
        sn_uuid[12]='0';

        sn_uuid[13]='-';

        sn_uuid[14]=serial_number[6];
        sn_uuid[15]='0';
        sn_uuid[16]=serial_number[7];
        sn_uuid[17]='0';

        sn_uuid[18]='-';

        sn_uuid[19]='0';
        sn_uuid[20]='0';
        sn_uuid[21]='0';
        sn_uuid[22]='0';

        sn_uuid[23]='-';

        for(i=24;i<28;i++){
            sn_uuid[i]='0';
        }

        sn_uuid[28]=serial_number[11];
        sn_uuid[29]='0';
        sn_uuid[30]=serial_number[12];
        sn_uuid[31]='0';
        sn_uuid[32]=serial_number[13];
        sn_uuid[33]='0';
        sn_uuid[34]=serial_number[14];
        sn_uuid[35]='0';
        sn_uuid[36]='\0';


       // ble_uuid_from_string("91a7608d-4456-479d-b9b1-4706e8711cf8", &uuid);
       printf("len=%d,sn_uuid=%s\r\n",strlen(sn_uuid),sn_uuid);
        ble_uuid_from_string(sn_uuid, &uuid);
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
        .fw_revision = BLACKORCA_FW_VERSION,
        .sw_revision = BLACKORCA_SW_VERSION,
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
        uint8_t str[] = { 0xB1, 0xB2, 0xB3 ,0xB4,0xB5,0xB6};
        uint8_t bd_addr[NVDS_LEN_BD_ADDRESS] = { 0 };
        nvds_tag_len_t bd_addr_len = NVDS_LEN_BD_ADDRESS;

#if 0
        ptr = memchr(scan_rsp_data, str[0], scan_rsp_size);
        if (!ptr)
                return;

        if (memcmp(ptr, str, sizeof(str)))
                return;
#endif
        if (nvds_get(NVDS_TAG_BD_ADDRESS, &bd_addr_len, bd_addr) != NVDS_OK)
                return;

        //copy sn and mac
        memcpy(scan_rsp_data+2,serial_number,15);
        memcpy(scan_rsp_data+17,bd_addr,6);
#if 0
        *ptr = bd_addr[5];
        *(ptr + 1) = bd_addr[4];
        *(ptr + 2) = bd_addr[3];
        *(ptr + 3) = bd_addr[2];
        *(ptr + 4) = bd_addr[1];
        *(ptr + 5) = bd_addr[0];
#endif
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
#if defined(CUSTOM_CONNECTION)
/* Advertising intervals change timeout timer callback */
static void adv_tim_cb(OS_TIMER timer)
{
        OS_TASK task = (OS_TASK)OS_TIMER_GET_TIMER_ID(timer);

        OS_TASK_NOTIFY(task, ADV_TMO_NOTIF, OS_NOTIFY_SET_BITS);
}


static void set_advertising_interval(adv_setting_t setting)
{
        uint16_t min = adv_intervals[setting].min;
        uint16_t max = adv_intervals[setting].max;

        ble_gap_adv_intv_set(min, max);
}
#endif


static void flash_test_tim_cb(OS_TIMER timer)
{
        OS_TASK task = (OS_TASK)OS_TIMER_GET_TIMER_ID(timer);
        printf("flash_test_tim_cb\r\n");
        OS_TASK_NOTIFY(task, FLASH_TEST_TMO_NOTIF, OS_NOTIFY_SET_BITS);
        //read_test();

}




static void handle_evt_gap_connected(ble_evt_gap_connected_t *evt)
{
        /**
         * Manage connection information
         */
        ble_task_env.conn_idx = evt->conn_idx;
        ble_task_env.conn_intv = evt->conn_params.interval_max;

#if defined(CUSTOM_CONNECTION)
        OS_TIMER_STOP(adv_tim, OS_TIMER_FOREVER);
        set_advertising_interval(ADV_INTERVAL_POWER);

        //ble_gap_adv_stop();


#endif

#if defined(RBLE_BAT_MEASURE)
#if defined(CUSTOM_REBOOT_VOL_RECORD)

#else /*CUSTOM_REBOOT_VOL_RECORD*/
		if (!OS_TIMER_IS_ACTIVE(bas_tim)) {
                bas_update();
                OS_TIMER_START(bas_tim, OS_TIMER_FOREVER);
        }
#endif/*CUSTOM_REBOOT_VOL_RECORD*/
#endif
}
#if defined(RBLE_BAT_MEASURE)

static void handle_evt_gap_disconnected(ble_evt_gap_disconnected_t *evt)
{
        size_t num_connected;


        /*
         * Device is still in the list if disconnect happened before timer expired.
         * In this case stop the timer and free memory.
         */

        ble_task_env.conn_idx = BLE_CONN_IDX_INVALID;

        //for error ff08
        ble_task_env.test_rx_data_id=0xff;


#if defined(CUSTOM_CONNECTION)
        /* Switch back to fast advertising interval. */
        set_advertising_interval(ADV_INTERVAL_FAST);
        ble_gap_adv_stop();
        OS_TIMER_START(adv_tim, OS_TIMER_FOREVER);



        //set_advertising_interval(ADV_INTERVAL_NORMAL);
        //ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);

#endif

        /*
         * Stop monitoring battery level if no one is connected.
         */
        ble_gap_get_devices(GAP_DEVICE_FILTER_CONNECTED, NULL, &num_connected, NULL);
        if (num_connected == 0) {

			#if 1 //defined(RBLE_BAT_MEASURE)
                OS_TIMER_STOP(bas_tim, OS_TIMER_FOREVER);
			#endif
        }
}

#endif


static void handle_evt_gap_adv_completed(ble_evt_gap_adv_completed_t *evt)
{
        // restart advertising so we can connect again
        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);

        /*if(ble_task_env.conn_idx == BLE_CONN_IDX_INVALID){
                ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
        }else{
                //stop adv
        }*/
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
extern bool rble_write_result_flash_cmd;
extern int user_length;

extern bool rble_start_cal_cmd;
uint8_t create_id_data[52];
int create_id_data_rx_len=0;
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
        //reset
        if(length==20){
            int rx_state=0;
            for (i = 0; i < length; i++) {
                if(*(value+i)!=0xff){
                    rx_state=1;
                }
            }
            if(rx_state==0){
                ble_task_env.test_rx_data_id=0xff;
                create_id_data_rx_len=0;
            }
        }

        printf("test_rx_data_id=%x,create_id_data_rx_len=%d\r\n",ble_task_env.test_rx_data_id,create_id_data_rx_len);
        test_data td;
        td.id1 = 0x00;
        memset(bd.id1, 0, sizeof(bd.id1));
        memset(bd.id2, 0, sizeof(bd.id1));
        memset(bd.id3, 0, sizeof(bd.id1));

        if(ble_task_env.test_rx_data_id==0x08){
            //
            if(read_result_id()==0x01){
                 int ret=-1;
                 hw_watchdog_freeze();
                 nvms_t nvms_rble_result_storage_handle__;
                 nvms_rble_result_storage_handle__ = ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
                 if(ad_nvms_erase_region(nvms_rble_result_storage_handle__, 0,
                                    RBLE_DATA_RESULT_PATITION_SIZE)){
                    ret=0;
                 }
                 hw_watchdog_unfreeze();
                 set_result_id(0xff);
                 // ret=erase_result_part(20);
                 printf("ff08 ad_nvms_erase_region RESULT :%d\r\n",ret);
            }
            
            memcpy(create_id_data+create_id_data_rx_len,value,length);
            create_id_data_rx_len+=length;
            printf("11 create_id_data_rx_len=%d\r\n",create_id_data_rx_len);
            if(create_id_data_rx_len>=52){
                ble_task_env.test_rx_data_id=0xff;
                create_id_data_rx_len=0;
                nvms_t nvms_rble_result_storage_handle;
                nvms_rble_result_storage_handle = ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
                int write_num=ad_nvms_write(nvms_rble_result_storage_handle,48,create_id_data,52);
                printf("ff08 wirte num=%d\r\n",write_num);
                bd.id1 = 0xFF;
	            bd.id2 = 0x08;
	            bd.id3 = 0xFF;
                rble_write_flash_cmd = true;
                rble_write_result_flash_cmd = true;
                //for test
                //write_track_to_flash();
               // write_result_to_flash();
                //end
                write_result_to_flash();
                set_result_id(0x01);
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0xFF;
            }
            return;
        }

#if 1
        value_h = *(value);
        value_cmd = *(value + 1);

        if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_START_SENSOR_CMD))
                {
                //start_collect_data =true;
                ble_task_env.test_rx_data_id=0xff;
            rble_write_result_flash_cmd=false;
            rble_write_flash_cmd=false;
            if(length>2)user_length=*(value+2);
#if defined(RBLE_SENSOR_CTRL_BY_APP)
                OS_TASK_NOTIFY(task_sensor_sample, RBLE_SENSOR_STOP_SAMPLE_NOTIF,
                        OS_NOTIFY_SET_BITS);

#endif
            inv_sleep(1500);

#if defined(RBLE_SENSOR_CTRL_BY_APP)
                OS_TASK_NOTIFY(task_sensor_sample, RBLE_SENSOR_START_SAMPLE_NOTIF,
                        OS_NOTIFY_SET_BITS);
#endif
                #if defined(STOP_SENSOR_AFTER_THREE_HOUR)
                OS_TIMER_STOP(stop_sensor_tim,OS_TIMER_FOREVER);
                OS_TIMER_START(stop_sensor_tim, OS_TIMER_FOREVER);
                #endif

                bd.id1 = 0xFF;
                bd.id2 = 0x01;
                bd.id3 = 0xFF;

                if(read_result_id()==0x01){
                    bd.id3=0x01;
                }else{
                    bd.id3=0x00;
                }
#if defined(CUSTOM_REBOOT_VOL_RECORD)
				reset_reboot_times();
#endif
                uint8_t battery[4]={0};
                memset(battery,0,sizeof(battery));
                battery[0]=bd.id1;
                battery[1]=bd.id2;
                battery[2]=bd.id3;
                battery[3]=read_battery_level();
                test_tx_data(svc, conn_idx, battery, sizeof(battery));
                //test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0xFF;
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && value_cmd == 0x03) {
                ble_task_env.test_rx_data_id=0xff;
                ble_task_env.ble2app_id = 0xFF;
                bd.id1 = 0xFF;
                bd.id2 = 0x03;
                bd.id3 = 0xFF;

               if(read_result_id()==0x01){
                     bd.id3=0x01;
               }else{
                     bd.id3=0x00;
               }
               test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_RECEIVE_DATA_SEND))
                {
                ble_task_env.test_rx_data_id=0xff;
                bd.id1 = 0xFF;
                bd.id2 = 0x02;
                bd.id3 = 0xFF;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0x02;          //read all test data
                rble_read_data_addr_offset = 0;
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_STOP_SAMPLE_CMD))
                {
                ble_task_env.test_rx_data_id=0xff;
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
                rble_write_result_flash_cmd=false;
                rble_write_flash_cmd=false;

        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_SAVE_SAMPLE_CMD))
                {
					bd.id1 = 0xFF;
	                bd.id2 = 0x08;
	                bd.id3 = 0xFF;
                    ble_task_env.test_rx_data_id=0x08;
                    init_yaw_offset(acc_gyr_ori[4]);
                    create_id_data_rx_len=0;
                    memset(create_id_data,0,sizeof(create_id_data));
                    if(length>2){
                        memcpy(create_id_data,value+2,length-2);
                        create_id_data_rx_len+=(length-2);
                    }
	                //test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                    //ble_task_env.ble2app_id = 0xFF;
					//rble_write_flash_cmd = true;
                    //rble_write_result_flash_cmd = true;

        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_START_CAL_CMD))
                {
                ble_task_env.test_rx_data_id=0xff;
                rble_start_cal_cmd = true;
                bd.id1 = 0xFF;
                bd.id2 = 0x05;
                bd.id3 = 0xFF;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0xFF;
               // nvms_t _nvms_rble_result_storage_handle;
                // _nvms_rble_result_storage_handle = ad_nvms_open(NVMS_IMAGE_DATA_STORAGE_PART);
                 // ad_nvms_erase_region(_nvms_rble_result_storage_handle, 0,
                                       // RBLE_DATA_RESULT_PATITION_SIZE);
        }

        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && (value_cmd == RBLE_SEND_RESULT_CMD))
                {
                ble_task_env.test_rx_data_id=0xff;
                bd.id1 = 0xFF;
                bd.id2 = 0x09;
                bd.id3 = 0xFF;
                rble_write_result_flash_cmd=false;
                rble_write_flash_cmd=false;
                rble_read_result_data_addr_offset=0;
                test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                ble_task_env.ble2app_id = 0x09;
        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && value_cmd == 0xff && length == 3 && (*(value + 2) == 0x01)) {
            ble_task_env.test_rx_data_id==0xff;

                        bd.id1 = bd.id2 = 0xff;
                        bd.id3 = 0x00;          //start flag
                        test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
                        ble_task_env.ble2app_id = 0x03;
                        t_rble_read_result_data_addr_offset = 0;

        }
        else if ((value_h == RBLE_RECEIVE_DATA_HEADER) && value_cmd == 0x69) {
            ble_task_env.test_rx_data_id=0xff;
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
        else if((*value)==0x0a && (*(value+1)==0xff) && (*(value+2)==0xff)
            && (*(value+3)==0xff) && (*(value+4)==0xff) && length==11){
            //write mac addr
            ble_task_env.test_rx_data_id=0xff;
            bd.id1=0x0a;
            bd.id2=0xff;
            if(write_mac_addr(value+5)==-1){
                bd.id3=0x00;
            }else{
                bd.id3=0x01;
            }
            ble_task_env.ble2app_id = 0x0aFF;
            uint8_t mac_v[9]={0};
            memset(mac_v,0,sizeof(mac_v));
            memcpy(mac_v+3,value+5,6);
            mac_v[0]=bd.id1;
            mac_v[1]=bd.id2;
            mac_v[2]=bd.id3;
            test_tx_data(svc, conn_idx, mac_v, sizeof(mac_v));
            //test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
        }
        else if((*value)==0x0b && (*(value+1)==0xff) && (*(value+2)==0xff)
            && (*(value+3)==0xff) && (*(value+4)==0xff) && length==20){
            //write sn
            ble_task_env.test_rx_data_id=0xff;
            bd.id1=0x0b;
            bd.id2=0xff;
            if(write_sn(value+5)== -1){
                bd.id3=0x0;
            }else{
                bd.id3=0x01;
            }
            ble_task_env.ble2app_id = 0x0bFF;
            uint8_t sn_v[18]={0};
            memset(sn_v,0,sizeof(sn_v));
            memcpy(sn_v+3,value+5,15);
            sn_v[0]=bd.id1;
            sn_v[1]=bd.id2;
            sn_v[2]=bd.id3;
            test_tx_data(svc, conn_idx, sn_v, sizeof(sn_v));

        }

        else if((*value)==0xaa && (*(value+1)==0x17) && (*(value+2)==0x00)
            && (*(value+3)==0xc1) && length==4){
            #if defined(RBLE_SENSOR_CTRL_BY_APP)
                OS_TASK_NOTIFY(task_sensor_sample, RBLE_SENSOR_START_SAMPLE_NOTIF,
                        OS_NOTIFY_SET_BITS);
            #endif
            //report sensor value
            ble_task_env.test_rx_data_id=0xff;
            ble_task_env.ble2app_id = 0xaa17;
            bd.id1=0xaa;
            bd.id2=0x97;
            bd.id3=0x06;
            test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));
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
                printf("ff09 rble_read_result_data_addr_offset=%d\r\n",rble_read_result_data_addr_offset);
                uint8_t rble_sample_result_data[20] = { 0 };
                nvms_t nvms_rble_result_storage_handle;
                int i;
                bool read_result = false;
                nvms_rble_result_storage_handle = ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
                memset(rble_sample_result_data, 0, sizeof(rble_sample_result_data));
                if (rble_read_result_data_addr_offset < RBLE_DATA_RESULT_PATITION_SIZE)
                        {
                        if (rble_read_result_data_addr_offset >= 100)
                                {
                                ad_nvms_read(nvms_rble_result_storage_handle,
                                        rble_read_result_data_addr_offset, rble_sample_result_data,
                                        sizeof(rble_sample_result_data));
                                read_result = true;
                                for (i = 0; i < 20; i++){
                                    printf("rble_sample_result_data[i=%d]=%x\r\n",i,rble_sample_result_data[i]);
                                    if (rble_sample_result_data[i] != 0xff){
                                        read_result = false;
                                    }


                                }
                                if (read_result)
                                {
                                         ble2app_data bd;
                                         bd.id1 = 0x65;
                                         bd.id2 = 0x6e;
                                         bd.id3 = 0x64;
                                         ble_task_env.ble2app_id = 0xff;
                                        rble_read_result_data_addr_offset = 0;
                                        int ret=-1;
#if 1
                                        hw_watchdog_freeze();
                                        if(ad_nvms_erase_region(nvms_rble_result_storage_handle, 0,
                                                RBLE_DATA_RESULT_PATITION_SIZE)){
                                            ret=0;
                                        }
                                        hw_watchdog_unfreeze();
#endif
                                        set_result_id(0xff);
                                       // ret=erase_result_part(20);
                                        printf("ad_nvms_erase_region RESULT :%d\r\n",ret);
                                         test_tx_data(svc, conn_idx, (uint8_t *)&bd, sizeof(bd));


                                        //return;

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
                        else
                        {
                                ad_nvms_read(nvms_rble_result_storage_handle,
                                        rble_read_result_data_addr_offset, rble_sample_result_data,
                                        20);
                                test_tx_data(svc, conn_idx, rble_sample_result_data, 20);
                                ble_task_env.ble2app_id = 0x09;
                                rble_read_result_data_addr_offset += 20;
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
        else if(ble_task_env.ble2app_id == 0x0aff){
            ble_task_env.ble2app_id = 0xff;
            inv_sleep(1500);
            reboot();
        }
        else if(ble_task_env.ble2app_id == 0x0bff){
            ble_task_env.ble2app_id = 0xff;
           // reboot();
        }

        else if(ble_task_env.ble2app_id == 0xaa17){
            ble_task_env.ble2app_id = 0xff;
            uint8_t tx_sensor_data[20] = { 0 };
            memset(tx_sensor_data, 0, 20);
            memcpy(tx_sensor_data, &(acc_gyr_ori[0]),4);
            memcpy(tx_sensor_data+4, &(acc_gyr_ori[1]),4);
            memcpy(tx_sensor_data+8, &(acc_gyr_ori[2]),4);
            memcpy(tx_sensor_data+12, &(acc_gyr_ori[3]),4);
            memcpy(tx_sensor_data+16, &(acc_gyr_ori[4]),4);
            test_tx_data(svc, conn_idx, tx_sensor_data, 20);
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


static int read_result_id()
{
    uint8_t buf[1]={0};
    nvms_t t=ad_nvms_open(NVMS_IMAGE_CUSTOM_CONFIG_PART);
    memset(buf,0,sizeof(buf));
    ad_nvms_read(t,10,buf,sizeof(buf));
    printf("read_result_id=%x\r\n",buf[0]);
    return buf[0];
}

static void set_result_id(uint8_t id)
{
    uint8_t value=id;
    nvms_t t=ad_nvms_open(NVMS_IMAGE_CUSTOM_CONFIG_PART);
    int ret=ad_nvms_write(t,10,&value,1);
    printf("set_result_id ret=%x\r\n",ret);
}


static void read_test(){
    nvms_t t=ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
    uint8_t buf[20] = { 0 };
    memset(buf,0,sizeof(buf));
    int i;
    bool j=true;
    ad_nvms_read(t,0, buf,sizeof(buf));
            for (i = 0; i < 20; i++){
                printf("erase_result_part buf[i=%d]=%x\r\n",i,buf[i]);
                if (buf[i] != 0xff){
                       j = false;
                }
            }

            OS_TIMER_STOP(flash_test_tim, OS_TIMER_FOREVER);
            if(!j){
                printf("erase fail\r\n");
            }else{
                printf("erase ok\r\n");
                test_flash();
            }
}

static int erase_result_part(size_t size)
{
        nvms_t t=ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
        uint8_t buf[20] = { 0 };
        memset(buf,0,sizeof(buf));
        int i;
        bool j=true;
        int k=3;
        while(k>0){
            hw_watchdog_freeze();
            if(ad_nvms_erase_region(t, 0,size)){
                printf("erase_result_part ok k=%x\r\n",k);
            }
            hw_watchdog_unfreeze();
            memset(buf,0,sizeof(buf));
            ad_nvms_read(t,0, buf,sizeof(buf));
            for (i = 0; i < 20; i++){
                printf("erase_result_part buf[i=%d]=%x\r\n",i,buf[i]);
                if (buf[i] != 0xff){
                       j = false;
                }
            }
            k--;
            if(j){
                break;
            }
        }

        if(j){
            return 0;
        }else{
            return -1;
        }


}

#if defined(CUSTOM_REBOOT_VOL_RECORD)
static void save_cur_sys_vol_value(void)
{
	unsigned int vol_high,vol_low = 0;
	uint8_t vol_buf[2]={0};
	unsigned int nvrm_vol_offset = 102;
	int ret = 0;

	battery_source bat = ad_battery_open();
	uint16_t bat_voltage = ad_battery_raw_to_mvolt(bat, ad_battery_read(bat));
	vol_high = bat_voltage / 100 ;
	vol_low = bat_voltage % 100 ;
	printf("bat_voltage=%d  vol_high =%d vol_low =%d \r\n",bat_voltage,vol_high ,vol_low);

	memset(vol_buf,0,sizeof(vol_buf));

	nvms_t t = ad_nvms_open(NVMS_IMAGE_CUSTOM_CONFIG_PART);
	memset(vol_buf,0,sizeof(vol_buf));
	ad_nvms_read(t,nvrm_vol_offset,vol_buf,sizeof(vol_buf));

	printf("vol_buf[0]=%x vol_buf[1]=%x\r\n",vol_buf[0],vol_buf[1]);
	vol_buf[0] = vol_high;
	vol_buf[1] = vol_low;

	printf("vol_buf[0]=%x  vol_buf[1]=%x  \r\n",vol_buf[0],vol_buf[1]);

	ret = ad_nvms_write(t,nvrm_vol_offset,vol_buf,2);

	printf("ad_nvms_write ret=%d\r\n",ret);

}


static void update_reboot_times(void)
{
    uint8_t reboot_buf[1]={0};
	int ret = 0;
	unsigned int nvrm_reboot_offset = 101;

	nvms_t t = ad_nvms_open(NVMS_IMAGE_CUSTOM_CONFIG_PART);
	memset(reboot_buf,0,sizeof(reboot_buf));
	ad_nvms_read(t,nvrm_reboot_offset,reboot_buf,sizeof(reboot_buf));
	printf("read_result_id=%x\r\n",reboot_buf[0]);

 	if (reboot_buf[0] == 0xff) reboot_buf[0] = 0x00;
	else
		++reboot_buf[0];

	printf("reboot_buf[0]=%x\r\n",reboot_buf[0]);

	ret = ad_nvms_write(t,nvrm_reboot_offset,reboot_buf,1);

    printf("ad_nvms_write ret=%d\r\n",ret);
}

static void reset_reboot_times(void)
{
	uint8_t reboot_buf[1]={0};

	int ret = 0;
	unsigned int nvrm_reboot_offset = 101;

	nvms_t t = ad_nvms_open(NVMS_IMAGE_CUSTOM_CONFIG_PART);
	memset(reboot_buf,0,sizeof(reboot_buf));
	ad_nvms_read(t,nvrm_reboot_offset,reboot_buf,sizeof(reboot_buf));
	printf("reset_reboot_times=%x\r\n",reboot_buf[0]);

	reboot_buf[0] = 0xff;

	ret = ad_nvms_write(t,nvrm_reboot_offset,reboot_buf,1);

    printf("ad_nvms_write ret=%d\r\n",ret);
}

#endif /*CUSTOM_REBOOT_VOL_RECORD*/


/* Buffer must have length at least max_len + 1 */
#if defined(CUSTOM_CONFIG_SERIAL_NUMBER_DEFINE)

static void read_serial_number(void)
{
    static const uint8_t default_sn[SERIAL_NUMBER_LEN] = SERIAL_NUMBER_INVAID;

#if dg_configNVPARAM_ADAPTER
	//char serial_number[SERIAL_NUMBER_LEN]; /* 1 byte for '\0' character */
	int serial_len=0;
	uint8_t valid = 0xFF;
	uint16_t read_len = 0,write_len=0;
	uint16_t param_len;
	nvparam_t param;
int i =0;
	memset(serial_number, 0, sizeof(serial_number));

	param = ad_nvparam_open("ble_platform");
#if 0
	write_len = ad_nvparam_write(param, TAG_BLE_PLATFORM_SERIAL_NUMBER,
                                                        sizeof(serial_number), SERIAL_NUMBER_NAME);

        /* Parameter length shall be long enough to store address and validity flag */
        param_len = ad_nvparam_get_length(param, TAG_BLE_PLATFORM_SERIAL_NUMBER, NULL);
printf("wzb read_serial_number param_len=%d tag=%d default_sn=%d  valid=%d\r\n",param_len,TAG_BLE_PLATFORM_SERIAL_NUMBER,sizeof(default_sn),sizeof(valid));
       // if (param_len < sizeof(default_sn) + sizeof(valid)) {
              //  goto done;
        //}
#endif
		read_len = ad_nvparam_read(param, TAG_BLE_PLATFORM_SERIAL_NUMBER,
											sizeof(serial_number), serial_number);

printf("wzb read_serial_number 0 serial_number=%s read_len=%d  MODEL_TYPY_STR=%s\r\n",serial_number,read_len,MODEL_TYPY_STR);

        /* Read serial number from nvparam only if validity flag is set to 0x00 */
       if ((strstr(serial_number, MODEL_TYPY_STR)) == NULL){
		   write_len = ad_nvparam_write(param, TAG_BLE_PLATFORM_SERIAL_NUMBER,
												SERIAL_NUMBER_LEN, SERIAL_NUMBER_INVAID);
	printf("wzb read_serial_number write_len=%d\r\n",write_len);
        }

	   read_len = ad_nvparam_read(param, TAG_BLE_PLATFORM_SERIAL_NUMBER,
										   sizeof(serial_number), serial_number);
	   printf("wzb read_serial_number 1 serial_number=%s read_len=%d\r\n",serial_number,read_len);

done:
        /* If address read from nvparam is invalid, replace with default one */
        if (read_len != sizeof(default_sn)) {
                memcpy(serial_number, SERIAL_NUMBER_INVAID, sizeof(serial_number));
        }
#else
        static const uint8_t empty_sn[SERIAL_NUMBER_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        nvms_t nvms;

        nvms = ad_nvms_open(NVMS_PARAM_PART);

        ad_nvms_read(nvms, NVMS_PARAMS_TAG_BD_ADDRESS_OFFSET, (uint8_t *) serial_number,
                                                                        sizeof(serial_number));

        /* If address is read from empty flash, replace with default one */
        if (!memcmp(serial_number, empty_sn, sizeof(serial_number))) {
                memcpy(serial_number, SERIAL_NUMBER_NAME, sizeof(serial_number));
        }
#endif
}

#endif /*CUSTOM_CONFIG_SERIAL_NUMBER_DEFINE*/

static bool read_nvms_param(uint8_t* param, uint8_t len, uint8_t nvparam_tag, uint32_t nvms_addr)
{
#if (dg_configNVMS_ADAPTER == 1)
#if (dg_configNVPARAM_ADAPTER == 1)
            uint16_t read_len = 0;
            uint16_t param_len;
            uint8_t valid;
            nvparam_t ble_parameters=ad_nvparam_open("ble_platform");

            read_len = ad_nvparam_read(ble_parameters, nvparam_tag,
                                    len, param);
            if (read_len == len) {
                    return true; /* Success */
            }


#else
            nvms_t nvms;
            int i;

            nvms = ad_nvms_open(NVMS_PARAM_PART);

            ad_nvms_read(nvms, nvms_addr, (uint8_t *) param, len);

            for (i = 0; i < len; i++) {
                    if (param[i] != 0xFF) {
                            return true; /* Success */
                    }
            }
#endif /* (dg_configNVPARAM_ADAPTER == 1) */
#endif /* (dg_configNVMS_ADAPTER == 1) */

            return false; /* Failure */


}

static void reboot(void)
{
        /*
         * Reset platform
         */
        __disable_irq();
        REG_SETF(CRG_TOP, SYS_CTRL_REG, SW_RESET, 1);
}

static int write_mac_addr(const void *data)
{
    nvparam_t custm_param=ad_nvparam_open("ble_platform");
    uint8_t custom_addr[BD_ADDR_LEN]={0x22, 0x00, 0x80, 0xCA, 0xEA, 0x80};
    memcpy(custom_addr,data,6);
    printf("write_mac_addr:%x:%x:%x:%x:%x:%x\r\n",custom_addr[0],custom_addr[1],custom_addr[2],custom_addr[3],custom_addr[4],custom_addr[5]);
    if (ad_nvparam_write(custm_param, TAG_BLE_PLATFORM_BD_ADDRESS,6, custom_addr)!=6){
            return -1;
    }
    return 0;
}

static int write_sn(const void *data)
{
    nvparam_t custm_param=ad_nvparam_open("ble_platform");
    uint8_t custom_sn[SERIAL_NUMBER_LEN] = "AS017425ECG0009";
    memcpy(custom_sn,data,15);
    custom_sn[15]='\0';
    printf("write_sn custom_sn=%s\r\n",custom_sn);
    if (ad_nvparam_write(custm_param, TAG_BLE_PLATFORM_SERIAL_NUMBER,SERIAL_NUMBER_LEN, custom_sn)!=16){
            return -1;
    }
    return 0;
}

static void test_flash(){
    bool flag=true;

        write_track_to_flash();
        write_result_to_flash();
        inv_sleep(1000);
        //if(erase_result_part(RBLE_DATA_RESULT_PATITION_SIZE)==-1){
            //flag=false;
        //}
        nvms_t t=ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
        hw_watchdog_freeze();
        if(ad_nvms_erase_region(t, 0,RBLE_DATA_RESULT_PATITION_SIZE)){
             printf("test_flash erase_result_part ok \r\n");
        }
        hw_watchdog_unfreeze();
        //inv_sleep(1000);
        OS_TIMER_START(flash_test_tim, OS_TIMER_FOREVER);


}


void ble_peripheral_task(void *params)
{
        int i = 0;
#if defined(CUSTOM_CONFIG_SERIAL_NUMBER_DEFINE)
       char serial_number[SERIAL_NUMBER_LEN]; /* 1 byte for '\0' character */
       int serial_len=0;
#endif

#if defined(RBLE_BAT_MEASURE)
        bas_tim = OS_TIMER_CREATE("bas", OS_MS_2_TICKS(RBLE_BATTERY_CHECK_INTERVAL), true,
                (void *) OS_GET_CURRENT_TASK(), bas_tim_cb);
#endif

        for (i = 0; i < 5; i++) {
                printf("wzb SW_VERSION=%s    SW_VERSION_DATE=%s\r\n", BLACKORCA_SW_VERSION,
                        BLACKORCA_SW_VERSION_DATE);
        }

        //test write mac and sn
        #if 0
        nvparam_t custm_param=ad_nvparam_open("ble_platform");
        uint8_t custom_addr[BD_ADDR_LEN]={0x22, 0x00, 0x80, 0xCA, 0xEA, 0x80};
        if (ad_nvparam_write(custm_param, TAG_BLE_PLATFORM_BD_ADDRESS,6, custom_addr)!=6){
            printf("write mac fail\r\n");
        }
        uint8_t custom_sn[SERIAL_NUMBER_LEN] = "AS017425ECG0009";

        if(ad_nvparam_write(custm_param, TAG_BLE_PLATFORM_SERIAL_NUMBER,SERIAL_NUMBER_LEN, custom_sn)!=16){
            printf("write sn fail \r\n");
        }

        bool valid;
        uint8_t test_address[6]={0};
        valid = read_nvms_param(test_address, 6, TAG_BLE_PLATFORM_BD_ADDRESS,
                                                                0x0000);
        if(valid){
            printf("read_nvms_param ok \r\n");
        }
        else{
            printf("read_nvms_param err \r\n");
        }
        int j=0;
        for(j=0;j<6;j++)printf("test addr=%x\r\n",test_address[j]);
        reboot();
        #endif
        //end


#if defined(CUSTOM_CONFIG_SERIAL_NUMBER_DEFINE)
/* Get serial number from nvparam if exist or default otherwise */
      read_serial_number();
#endif

#if defined(CUSTOM_REBOOT_VOL_RECORD)
		update_reboot_times();
#endif

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
        ble_task_env.test_rx_data_id=0xff;

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


#if defined(RBLE_BAT_MEASURE)
					bas = svc;
#endif

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

#if defined(STOP_SENSOR_AFTER_THREE_HOUR)
    stop_sensor_tim= OS_TIMER_CREATE("stop", OS_MS_2_TICKS(THREE_HOUR), OS_TIMER_FAIL,
                (void *) OS_GET_CURRENT_TASK(), stop_sensor_tim_cb);
#endif

#if defined(CUSTOM_CONNECTION)
        /*
         * Create timer for switching from "fast connection" to "reduced power" advertising
         * intervals after 30 seconds.
         */
        adv_tim = OS_TIMER_CREATE("adv", OS_MS_2_TICKS(30000), OS_TIMER_FAIL,
                (void *) OS_GET_CURRENT_TASK(), adv_tim_cb);
        set_advertising_interval(ADV_INTERVAL_FAST);
#endif
        app_calc_scan_rsp_data(scan_rsp_data, sizeof(scan_rsp_data));
        printf("scan_rsp_data=%x,%x,%x\r\n",scan_rsp_data[4],scan_rsp_data[5],scan_rsp_data[6]);
        ble_gap_adv_data_set(sizeof(adv_data), adv_data, sizeof(scan_rsp_data), scan_rsp_data);

        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
#if defined(CUSTOM_CONNECTION)
        OS_TIMER_START(adv_tim, OS_TIMER_FOREVER);
#endif

        flash_test_tim= OS_TIMER_CREATE("flash", OS_MS_2_TICKS(5000), OS_TIMER_FAIL,
                        (void *) OS_GET_CURRENT_TASK(), flash_test_tim_cb);

        printf("wzb for(;;)\r\n");

#if defined(CUSTOM_REBOOT_VOL_RECORD)
#if defined(RBLE_BAT_MEASURE)
		if (!OS_TIMER_IS_ACTIVE(bas_tim)) {
				bas_update();
				OS_TIMER_START(bas_tim, OS_TIMER_FOREVER);
		}
#endif
#endif /*CUSTOM_REBOOT_VOL_RECORD*/
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
						#if defined(RBLE_BAT_MEASURE)
                        case BLE_EVT_GAP_DISCONNECTED:
                                handle_evt_gap_disconnected(
                                        (ble_evt_gap_disconnected_t *)hdr);
                                break;
						#endif

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

	#if defined(RBLE_BAT_MEASURE)
		if (notif & RBLE_BAS_TMO_NOTIF) {
								/*
								 * Read battery level, and notify clients if level changed.
								 */
								bas_update();
						}
	#endif

    #if defined(STOP_SENSOR_AFTER_THREE_HOUR)
        if(notif & STOP_SENSOR_TMO_NOTIF){
             OS_TASK_NOTIFY(task_sensor_sample, RBLE_SENSOR_STOP_SAMPLE_NOTIF,
                        OS_NOTIFY_SET_BITS);
             rble_write_result_flash_cmd=false;
             rble_write_flash_cmd=false;
        }
    #endif

    #if defined(CUSTOM_CONNECTION)
    /* Notified from advertising timer? */
                    if (notif & ADV_TMO_NOTIF) {
                            /*
                             * Change interval values and stop advertising. Once it's stopped, it will
                             * start again with new parameters.
                             */

                            set_advertising_interval(ADV_INTERVAL_POWER);

                            ble_gap_adv_stop();
                    }


    #endif

    //test
        if(notif & FLASH_TEST_TMO_NOTIF){
               read_test();
        }


        }
}
