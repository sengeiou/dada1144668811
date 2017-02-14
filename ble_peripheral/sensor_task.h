





#include <string.h>
#include <stdio.h>
#include <time.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "osal.h"
#include "ble_service.h"

#include <ad_i2c.h>





#define mainCOUNTER_FREQUENCY_MS                OS_MS_2_TICKS(200)

#define RBLE_GET_ALL_SENSOR_INTERVAL            (100)
#define REG_WHO_AM_I              0x75


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


