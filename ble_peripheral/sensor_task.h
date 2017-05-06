





#include <string.h>
#include <stdio.h>
#include <time.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "osal.h"
#include "ble_service.h"

#include <ad_i2c.h>

#include "data_v5.h"



//#define mainCOUNTER_FREQUENCY_MS                OS_MS_2_TICKS(200)

#define mainCOUNTER_FREQUENCY_MS                OS_MS_2_TICKS(10)


#define RBLE_GET_ALL_SENSOR_INTERVAL            (100)
//#define REG_WHO_AM_I              0x75

#define RBLE_DATA_ACCEL_LABLE  "gh"
#define RBLE_DATA_GYRO_LABLE  "yr"
#define RBLE_DATA_COMPS_LABLE  "ps"
#define RBLE_DATA_ORI_LABLE  "ri"
#define RBLE_DATA_TK_LABLE  "tk"
#define RBLE_DATA_CNT_LABLE  "nx"

#define RBLE_DATA_LABLE_LENGTH  2

#define RBLE_DATA_BUF_LENGTH    100
#define RBLE_FLOAT_SIZE   4
#define RBLE_LONG_SIZE   4
#define RBLE_CNT_SIZE   2

#define RBLE_DATA_PATITION_SIZE  (0x00F000)
#define RBLE_DATA_RESULT_PATITION_SIZE  (0x05B000)

#define RBLE_SAMPLE_TIMER_NOTIF (1 << 2) 
#define RBLE_SENSOR_START_SAMPLE_NOTIF (1 << 3) 
#define RBLE_SENSOR_STOP_SAMPLE_NOTIF (1 << 4) 


#define RBLE_RECEIVE_DATA_HEADER (0xFF)
#define RBLE_START_SENSOR_CMD (0x01)
#define RBLE_RECEIVE_DATA_SEND (0x02)
#define RBLE_STOP_SAMPLE_CMD (0x07)
#define RBLE_SAVE_SAMPLE_CMD (0x08)
#define RBLE_START_CAL_CMD (0x05)
#define RBLE_SEND_RESULT_CMD (0x09)


#define RBLE_GSENSOR_ID_OFFSET  (0)
#define RBLE_MSENSOR_ID_OFFSET  (1)


typedef struct RbleSensorControlStruct
{
	OS_TIMER tim_sensor_all_get;
	i2c_device dev;
	
}RbleSensorControlStruct_t;


typedef enum {
        SENSOR_OK,
        SENSOR_NOT_FOUND,
        SENSOR_ERROR,
} sensor_err_t;


