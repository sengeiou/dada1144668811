
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "osal.h"
#include "resmgmt.h"
#include "hw_cpm.h"
#include "hw_gpio.h"
#include "hw_watchdog.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"

#include "platform_devices.h"

#include "sensor_task.h"

#include "inv_mems.h"

#include "math.h"

//#include "invn/common/mlmath.h"

#include "qfplib/qfplib.h"
#include "qfplib/qfpio.h"

#if defined(RBLE_DATA_STORAGE_IN_FLASH)
#include "ad_nvms.h"
#endif

/**
 * @brief Template task increases a counter every mainCOUNTER_FREQUENCY_MS ms
 */

#define RBLE_FLOAT_STR_LENTH   64
char print_float_str[RBLE_FLOAT_STR_LENTH];


#if defined(RBLE_DATA_STORAGE_IN_FLASH)

nvms_t nvms_rble_storage_handle;
uint32_t rble_data_addr_offset=0;
uint8_t rble_sample_data[RBLE_DATA_BUF_LENGTH]={0};
int rble_smp_count=0;

bool rble_data_patition_not_full=true;


#endif

OS_TIMER rble_sample_timer_id;


RbleSensorControlStruct_t rble_sensor_control_env;
 
 i2c_device i2c_gsensor_dev;


struct hal_s_ {
	long report;          // What to report?
	unsigned char  debug_reg_on;     // with '\' as a command this turns ON
	int expected_data;
	volatile unsigned char new_gyro;
};
static struct hal_s_ hal = { 0 };

/* Every time new gyro data is available, this function is called in an
* ISR context. In this example, it sets a flag protecting the FIFO read
* function.
*/
void gyro_data_ready_cb(void)
{
	hal.new_gyro = 1;
}
/*******************************************************************************/
#define PRINT_ACCEL         (0x01)
#define PRINT_GYRO          (0x02)
#define PRINT_RAW_GYRO      (0x04)
#define PRINT_COMPASS       (0x08)
#define PRINT_RAW_COMPASS   (0x10)
#define PRINT_RV            (0x20)
#define PRINT_GRV           (0x40)
#define PRINT_ORIENT        (0x80)
#define PRINT_LINEAR_ACCEL  (0x100)
#define PRINT_GRAVITY       (0x200)
#define PRINT_STEP_COUNTER  (0x400)
#define PRINT_STEP_DETECTOR (0x800)
#define PRINT_SMD           (0x1000)
#define PRINT_GEOMAG        (0x2000)
#define PRINT_PRESSURE      (0x8000)
#define PRINT_CUBE_GRV      (0x10000)
#define PRINT_CUBE_RV       (0x20000)
#define PRINT_CUBE_GEOMAG   (0x40000)
#define PRINT_LPQ           (0x80000)           
#define PRINT_BAC			(0x100000)
#define PRINT_FLIP_PICKUP	(0x200000)
#define PRINT_TILT			(0x400000)
#define PRINT_PROXIMITY		(0x800000)
#define PRINT_HRM			(0x1000000)
#define PRINT_SHAKE			(0x2000000)
#define PRINT_B2S			(0x4000000)

#ifdef MEMS_20609
#define DMP_INT_SMD		0x0400
#define DMP_INT_PED		0x0800
#endif 

#ifdef _WIN32
#define INV_SPRINTF(str, len, ...) sprintf_s(str, len, __VA_ARGS__)
#else
#define INV_SPRINTF(str, len, ...) sprintf(str, __VA_ARGS__)
#endif


int self_test_result = 0;
int dmp_bias[9] = { 0 };

unsigned short accel_data_was_set = 0;
unsigned short gyro_data_was_set = 0;
unsigned short raw_gyro_data_was_set = 0;
unsigned short compass_data_was_set = 0;
unsigned short raw_compass_data_was_set = 0;
unsigned short quat6_data_was_set = 0;
unsigned short quat9_data_was_set = 0;
unsigned short rv_quat_was_set = 0;
unsigned short gmrv_quat_was_set = 0;
unsigned short bac_data_was_set = 0;
unsigned short flip_data_was_set = 0;
unsigned short tilt_data_was_set = 0;


float grv_float[4];
signed long cube_grv[4] = { 0, 0, 0, 0 };
float accel_float[3];
float rv_float[4];
signed long cube_rv[4] = { 0, 0, 0, 0 };
float gyro_float[3];
float gyro_raw_float[3];
float gyro_bias_float[3];
long previous_bias[3];
float compass_float[3];
float compass_raw_float[3];
float compass_bias[3];
#if (MEMS_CHIP == HW_ICM20648) 
float gmrv_float[4];
signed long cube_gmrv[4] = { 0, 0, 0, 0 };
#endif
int accel_accuracy = 0;
int gyro_accuracy = 0;
int rv_accuracy = 0;
int compass_accuracy = 0;
#if (MEMS_CHIP == HW_ICM20648)
int gmrv_accuracy = 0;
#endif

long long ts = 0;

signed long result_quat[4] = { 0, 0, 0, 0 };

int a_average[3] = { 0, 0, 0 };
int g_average[3] = { 0, 0, 0 };
#if defined MEMS_AUGMENTED_SENSORS
float linAccFloat[3];
float gravityFloat[3];
float orientationFloat[3];
#endif

unsigned long steps = 0;
uint16_t bac_state = 0;
uint8_t tilt_state = 0;
long bac_ts = 0;
uint16_t flip_pickup = 0;
uint8_t step_detected = 0;
//float current_output_rate = 5;
//float current_output_rate = 100;
float current_output_rate = 50;


/** @brief Set of flags for BAC state */
#define BAC_DRIVE	0x01
#define BAC_WALK	0x02
#define BAC_RUN		0x04
#define BAC_BIKE	0x08
#define BAC_TILT	0x10
#define BAC_STILL	0x20


signed char ACCEL_GYRO_ORIENTATION[] = {0,-1,0,1,0,0,0,0,1};
/* Change the COMPASS_SLAVE_ID to the correct ID of compass used. You can find the defines in inv_mems_hw_config.h*/
const unsigned char COMPASS_SLAVE_ID = HW_AK09912; 

/* Change COMPASS_CHIP_ADDR to 0x0C for ICM20698 which uses internal AKM 9916 */
/* Change COMPASS_CHIP_ADDR to 0x0E for other AKM 9912/9911/9913/8963*/

//const unsigned char COMPASS_CHIP_ADDR = 0x0E;
const unsigned char COMPASS_CHIP_ADDR = 0x0C;

const unsigned char PRESSURE_CHIP_ADDR = 0x00;

signed char COMPASS_ORIENTATION[] = {0,-1,0,1,0,0,0,0,1};

long SOFT_IRON_MATRIX[] = {1073741824,0,0,0,1073741824,0,0,0,1073741824};

#define INV_TST_LEN 256
char tst[INV_TST_LEN] = { 0 };

//test_r
#define MAX_BUF_LENGTH  (18)
//#define MAX_BUF_LENGTH  (19)

enum packet_type_e {
	PACKET_TYPE_ACCEL,
	PACKET_TYPE_GYRO,
	PACKET_TYPE_QUAT,
	PACKET_TYPE_COMPASS = 7
};

/* Send data to the Python client application.
* Data is formatted as follows:
* packet[0]    = $
* packet[1]    = packet type (see packet_type_e)
* packet[2+]   = data
*/
void send_data_packet(char packet_type, void *data)
{
	long *ldata = (long*)data;
	char buf[MAX_BUF_LENGTH], length = 0;

	memset(buf, 0, MAX_BUF_LENGTH);
	buf[0] = '$';
	buf[1] = packet_type;

	switch (packet_type)
	{
	case PACKET_TYPE_ACCEL:
	case PACKET_TYPE_GYRO:
	case PACKET_TYPE_COMPASS:
		buf[2] = (char)(ldata[0] >> 24);
		buf[3] = (char)(ldata[0] >> 16);
		buf[4] = (char)(ldata[0] >> 8);
		buf[5] = (char)ldata[0];
		buf[6] = (char)(ldata[1] >> 24);
		buf[7] = (char)(ldata[1] >> 16);
		buf[8] = (char)(ldata[1] >> 8);
		buf[9] = (char)ldata[1];
		buf[10] = (char)(ldata[2] >> 24);
		buf[11] = (char)(ldata[2] >> 16);
		buf[12] = (char)(ldata[2] >> 8);
		buf[13] = (char)ldata[2];
		length = 14;
		break;
	case PACKET_TYPE_QUAT:
		buf[2] = (char)(ldata[0] >> 24);
		buf[3] = (char)(ldata[0] >> 16);
		buf[4] = (char)(ldata[0] >> 8);
		buf[5] = (char)ldata[0];
		buf[6] = (char)(ldata[1] >> 24);
		buf[7] = (char)(ldata[1] >> 16);
		buf[8] = (char)(ldata[1] >> 8);
		buf[9] = (char)ldata[1];
		buf[10] = (char)(ldata[2] >> 24);
		buf[11] = (char)(ldata[2] >> 16);
		buf[12] = (char)(ldata[2] >> 8);
		buf[13] = (char)ldata[2];
		buf[14] = (char)(ldata[3] >> 24);
		buf[15] = (char)(ldata[3] >> 16);
		buf[16] = (char)(ldata[3] >> 8);
		buf[17] = (char)ldata[3];
		length = 18;
		break;
	}

	#if 1
	//test_r
	if(length>0)
	{
	        /*
		buf[length]=0;
		#if defined(RBLE_UART_DEBUG)
		printf(buf);	
		#endif
		*/
	}
	
	#else
	if (length) {
		for (int ii = 0; ii < length; ii++)
			fputc(buf[ii]); // will go to USART0
	}
	#endif
}


void print_data_console(char * str)
{
	int32_t i = 0;
	static int32_t data_console_in_use;

	if (data_console_in_use == 1) return; //to avoid interleaved message print
	data_console_in_use = 1;

#if 1
//test_r
#if defined(RBLE_UART_DEBUG)

	printf(str);
#endif
#else
	while (str[i])
	{
		if (str[i] == NULL)
			break;
		fputc6(str[i++]);
	}
#endif	
	data_console_in_use = 0;
}

void print_command_console(char * str)
{
	int32_t i = 0;
	static int32_t data_console_in_use;

	if (data_console_in_use == 1) return; //to avoid interleaved message print
	data_console_in_use = 1;

#if 1
//test_r
#if defined(RBLE_UART_DEBUG)

	printf(str);
#endif
#else
	while (str[i])
	{
		if (str[i] == NULL)
			break;
		fputc6(str[i++]);
	}
#endif

	data_console_in_use = 0;
}

inv_error_t set_output_rates(float rate)
{
#if (MEMS_CHIP == HW_ICM20609)
  unsigned short base_rate = 200; //DMP runs at 200Hz always
#elif (MEMS_CHIP == HW_ICM20648)
  unsigned short base_rate = 1125; 
#endif
  dmp_reset_fifo();
  inv_error_t result=0;
          if (hal.report & PRINT_ACCEL)
                result |= inv_set_odr(ANDROID_SENSOR_ACCELEROMETER, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_GYRO)
                result |= inv_set_odr(ANDROID_SENSOR_GYROSCOPE, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_RAW_GYRO)
                result |= inv_set_odr(ANDROID_SENSOR_GYROSCOPE_UNCALIBRATED, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_GRV)
                result |= inv_set_odr(ANDROID_SENSOR_GAME_ROTATION_VECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_CUBE_GRV)
                result |= inv_set_odr(ANDROID_SENSOR_GAME_ROTATION_VECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_STEP_DETECTOR)
                result |= inv_set_odr(ANDROID_SENSOR_STEP_DETECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_SMD)
                result |= inv_set_odr(ANDROID_SENSOR_WAKEUP_SIGNIFICANT_MOTION, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_STEP_COUNTER)
                result |= inv_set_odr(ANDROID_SENSOR_STEP_COUNTER, (unsigned short)(base_rate / rate));
#if (MEMS_CHIP == HW_ICM20648)
          if (hal.report & PRINT_LINEAR_ACCEL)
                result |= inv_set_odr(ANDROID_SENSOR_LINEAR_ACCELERATION, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_GRAVITY)
                result |= inv_set_odr(ANDROID_SENSOR_GRAVITY, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_COMPASS)
                result |= inv_set_odr(ANDROID_SENSOR_GEOMAGNETIC_FIELD, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_RAW_COMPASS)
                result |= inv_set_odr(ANDROID_SENSOR_MAGNETIC_FIELD_UNCALIBRATED, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_RV)
                result |= inv_set_odr(ANDROID_SENSOR_ROTATION_VECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_GEOMAG)
                result |= inv_set_odr(ANDROID_SENSOR_GEOMAGNETIC_ROTATION_VECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_BAC)
                result |= inv_set_odr(ANDROID_SENSOR_ACTIVITY_CLASSIFICATON, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_TILT)
                result |= inv_set_odr(ANDROID_SENSOR_WAKEUP_TILT_DETECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_FLIP_PICKUP)
                result |= inv_set_odr(ANDROID_SENSOR_FLIP_PICKUP, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_CUBE_RV)
                result |= inv_set_odr(ANDROID_SENSOR_ROTATION_VECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_CUBE_GEOMAG)
                result |= inv_set_odr(ANDROID_SENSOR_GEOMAGNETIC_ROTATION_VECTOR, (unsigned short)(base_rate / rate));
          if (hal.report & PRINT_ORIENT)
                result |= inv_set_odr(ANDROID_SENSOR_ORIENTATION, (unsigned short)(base_rate / rate));
#endif
          dmp_reset_odr_counters();
   return result;
}

int handle_char_input(char c)
{
#if (MEMS_CHIP != HW_ICM20609)
        inv_error_t result;
#endif

	switch (c)
	{
	case 'a':
		dmp_reset_fifo();
		hal.report ^= PRINT_ACCEL;
                inv_enable_sensor(ANDROID_SENSOR_ACCELEROMETER, !!(hal.report & PRINT_ACCEL));
                dmp_reset_odr_counters();
                if(hal.report & PRINT_ACCEL) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "ACCEL....output toggled, now is: %s.\r\n", (hal.report & PRINT_ACCEL) ? "ON" : "OFF"); print_command_console(tst);
		break;
	case 'g':
		dmp_reset_fifo();
		hal.report ^= PRINT_GYRO;
		inv_enable_sensor(ANDROID_SENSOR_GYROSCOPE, !!(hal.report & PRINT_GYRO));
                dmp_reset_odr_counters();
                if(hal.report & PRINT_GYRO) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "GYRO....output toggled, now is: %s.\r\n", (hal.report & PRINT_GYRO) ? "ON" : "OFF"); print_command_console(tst);
		break;
	case 'G':
		dmp_reset_fifo();
		hal.report ^= PRINT_RAW_GYRO;
		inv_enable_sensor(ANDROID_SENSOR_GYROSCOPE_UNCALIBRATED, !!(hal.report & PRINT_RAW_GYRO));
                dmp_reset_odr_counters();
                if(hal.report & PRINT_RAW_GYRO) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "RAW GYRO....output toggled, now is: %s.\r\n", (hal.report & PRINT_RAW_GYRO) ? "ON" : "OFF"); print_command_console(tst);
		break;
#if (MEMS_CHIP != HW_ICM20609)
        case 'c':
		dmp_reset_fifo();
		hal.report ^= PRINT_COMPASS;
		inv_enable_sensor(ANDROID_SENSOR_GEOMAGNETIC_FIELD, !!(hal.report & PRINT_COMPASS));
                dmp_reset_odr_counters();
                if(hal.report & PRINT_COMPASS) set_output_rates(current_output_rate);
		if (hal.report & PRINT_COMPASS)
			result = inv_resume_akm();
		else
			result = inv_suspend_akm();
		if (result)
			print_data_console("Compass resume/suspend error \n");
		INV_SPRINTF(tst, INV_TST_LEN, "Compass....output toggled, now is: %s.\r\n", (hal.report & PRINT_COMPASS) ? "ON" : "OFF"); print_command_console(tst);
		break;
	case 'C':
		dmp_reset_fifo();
		hal.report ^= PRINT_RAW_COMPASS;
		inv_enable_sensor(ANDROID_SENSOR_MAGNETIC_FIELD_UNCALIBRATED, !!(hal.report & PRINT_RAW_COMPASS));
                dmp_reset_odr_counters();
                if(hal.report & PRINT_RAW_COMPASS) set_output_rates(current_output_rate);
		if (hal.report & PRINT_RAW_COMPASS)
			result = inv_resume_akm();
		else
			result = inv_suspend_akm();
		if (result)
			print_data_console("Compass resume/suspend error \n");
		INV_SPRINTF(tst, INV_TST_LEN, "Raw Compass....output toggled, now is: %s.\r\n", (hal.report & PRINT_RAW_COMPASS) ? "ON" : "OFF"); print_command_console(tst);
		break;
#endif
	case 'r':
		dmp_reset_fifo();
		hal.report ^= PRINT_GRV;
		inv_enable_sensor(ANDROID_SENSOR_GAME_ROTATION_VECTOR, !!(hal.report & PRINT_GRV));
                dmp_reset_odr_counters();
                if(hal.report & PRINT_GRV) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "Game RV....output toggled, now is: %s.\r\n", (hal.report & PRINT_GRV) ? "ON" : "OFF");
                print_command_console(tst);
                break;
        case 'q':
		dmp_reset_fifo();
		hal.report ^= PRINT_CUBE_GRV;
		inv_enable_sensor(ANDROID_SENSOR_GAME_ROTATION_VECTOR, !!(hal.report & PRINT_CUBE_GRV));
                dmp_reset_odr_counters();
		if(hal.report & PRINT_CUBE_GRV) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "Cube Game RV....output toggled, now is: %s.\r\n", (hal.report & PRINT_CUBE_GRV) ? "ON" : "OFF");
		print_command_console(tst);
		break;
        case 'd':
		dmp_reset_fifo();
		hal.report ^= PRINT_STEP_DETECTOR;
		inv_enable_sensor(ANDROID_SENSOR_STEP_DETECTOR, !!(hal.report & PRINT_STEP_DETECTOR));
                dmp_reset_odr_counters();
#if (MEMS_CHIP == HW_ICM20609)
                if(hal.report & PRINT_STEP_DETECTOR) set_output_rates(50);
#elif (MEMS_CHIP == HW_ICM20630)
                if(hal.report & PRINT_STEP_DETECTOR) set_output_rates(51);
#else                 
		// pedometer always runs at half the rate of BAC, to run pedometer at 56Hz, run BAC at 112Hz as pedometer divider is always 2
		if(hal.report & PRINT_STEP_DETECTOR) set_output_rates(112.5);
#endif
		INV_SPRINTF(tst, INV_TST_LEN, "Step Detector....output toggled, now is: %s.\r\n", (hal.report & PRINT_STEP_DETECTOR) ? "ON" : "OFF");
		print_command_console(tst);
		break;
	case 'm':
		dmp_reset_fifo();
		hal.report ^= PRINT_SMD;
		inv_enable_sensor(ANDROID_SENSOR_WAKEUP_SIGNIFICANT_MOTION, !!(hal.report & PRINT_SMD));
                dmp_reset_odr_counters();
#if (MEMS_CHIP == HW_ICM20609)
                if(hal.report & PRINT_SMD) set_output_rates(50);
#else               
		// pedometer always runs at half the rate of BAC, to run pedometer at 56Hz, run BAC at 112Hz as pedometer divider is always 2
		if(hal.report & PRINT_SMD) set_output_rates(112.5);
#endif
		INV_SPRINTF(tst, INV_TST_LEN, "SMD....output toggled, now is: %s.\r\n", (hal.report & PRINT_SMD) ? "ON" : "OFF");
		print_command_console(tst);
		break;

	case 'p':
		dmp_reset_fifo();
		hal.report ^= PRINT_STEP_COUNTER;
		inv_enable_sensor(ANDROID_SENSOR_STEP_COUNTER, !!(hal.report & PRINT_STEP_COUNTER));
                dmp_reset_odr_counters();
#if (MEMS_CHIP == HW_ICM20609)
                if(hal.report & PRINT_STEP_COUNTER) set_output_rates(50);
#else                 
		// pedometer always runs at half the rate of BAC, to run pedometer at 56Hz, run BAC at 112Hz as pedometer divider is always 2
		if(hal.report & PRINT_STEP_COUNTER) set_output_rates(112.5);
#endif
		INV_SPRINTF(tst, INV_TST_LEN, "Step Counter....output toggled, now is: %s.\r\n", (hal.report & PRINT_STEP_COUNTER) ? "ON" : "OFF");
		print_command_console(tst);
		break;
#if (MEMS_CHIP != HW_ICM20609)
	case 'R':
		dmp_reset_fifo();
		hal.report ^= PRINT_RV;
		inv_enable_sensor(ANDROID_SENSOR_ROTATION_VECTOR, !!(hal.report & PRINT_RV));
                dmp_reset_odr_counters();
		if(hal.report & PRINT_RV) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "RV....output toggled, now is: %s.\r\n", (hal.report & PRINT_RV) ? "ON" : "OFF");
		print_command_console(tst);
		break;
	case 'e':
		dmp_reset_fifo();
		hal.report ^= PRINT_GEOMAG;
		inv_enable_sensor(ANDROID_SENSOR_GEOMAGNETIC_ROTATION_VECTOR, !!(hal.report & PRINT_GEOMAG));
                dmp_reset_odr_counters();
		if(hal.report & PRINT_GEOMAG) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "GeoMAG RV....output toggled, now is: %s.\r\n", (hal.report & PRINT_GEOMAG) ? "ON" : "OFF");
		print_command_console(tst);
		break;
#if (MEMS_CHIP != HW_ICM20630)
	case 'b':
		dmp_reset_fifo();
                if(!(hal.report & PRINT_BAC))
                    dmp_reset_bac_states();
		hal.report ^= PRINT_BAC;
		inv_enable_sensor(ANDROID_SENSOR_ACTIVITY_CLASSIFICATON, !!(hal.report & PRINT_BAC));
                dmp_reset_odr_counters();
		if(hal.report & PRINT_BAC) set_output_rates(112.5);
		INV_SPRINTF(tst, INV_TST_LEN, "BAC....output toggled, now is: %s.\r\n", (hal.report & PRINT_BAC) ? "ON" : "OFF"); print_command_console(tst);
		break;
	case 'T':
		dmp_reset_fifo();
		hal.report ^= PRINT_TILT;
		inv_enable_sensor(ANDROID_SENSOR_WAKEUP_TILT_DETECTOR, !!(hal.report & PRINT_TILT));
                dmp_reset_odr_counters();
		if(hal.report & PRINT_TILT) set_output_rates(112.5);
		INV_SPRINTF(tst, INV_TST_LEN, "Tilt....output toggled, now is: %s.\r\n", (hal.report & PRINT_TILT) ? "ON" : "OFF"); print_command_console(tst);
		break;
	case 'F':
		dmp_reset_fifo();
		hal.report ^= PRINT_FLIP_PICKUP;
		inv_enable_sensor(ANDROID_SENSOR_FLIP_PICKUP, !!(hal.report & PRINT_FLIP_PICKUP));
                dmp_reset_odr_counters();
		if(hal.report & PRINT_FLIP_PICKUP) set_output_rates(112.5);
		INV_SPRINTF(tst, INV_TST_LEN, "Flip Pickup....output toggled, now is: %s.\r\n", (hal.report & PRINT_FLIP_PICKUP) ? "ON" : "OFF"); print_command_console(tst);
		break;
#endif
#endif            
	
        case '0':
          set_output_rates(1);
          current_output_rate = 1;
          break;
        case '1':
          set_output_rates(5);
          current_output_rate = 5;
          break;
        case '2':
          set_output_rates(15);
          current_output_rate = 15;
          break;
        case '3':
#if (MEMS_CHIP==HW_ICM20609)
          set_output_rates(50);
          current_output_rate = 50;
#elif (MEMS_CHIP==HW_ICM20648)
          set_output_rates(30);
          current_output_rate = 30;
#endif
          break;
        case '4':
#if (MEMS_CHIP==HW_ICM20609)
          set_output_rates(100);
          current_output_rate = 100;
#elif (MEMS_CHIP==HW_ICM20648)
          set_output_rates(51);
          current_output_rate = 51;
#endif
          break;
#if (MEMS_CHIP==HW_ICM20648)          
        case '5':
          set_output_rates(56.5);
          current_output_rate = 56.5;
          break;
        case '6':
          set_output_rates(60);
          current_output_rate = 60;
          break;
        case '7':
          set_output_rates(102);
          current_output_rate = 102;
          break;
        case '8':
          set_output_rates(112.5);
          current_output_rate = 112.5;
          break;
        case '9':
          set_output_rates(225);
          current_output_rate = 225;
          break;
                
	case 'k':
		INV_SPRINTF(tst, INV_TST_LEN, "Getting the DMP biases...\r\n"); print_data_console(tst);
		memset(dmp_bias, 0, sizeof(dmp_bias));
		dmp_get_bias(dmp_bias);
		INV_SPRINTF(tst, INV_TST_LEN, "DMP   Accel Bias: X=%d, Y=%d, Z=%d\r\n", dmp_bias[0], dmp_bias[1], dmp_bias[2]); print_data_console(tst);
		INV_SPRINTF(tst, INV_TST_LEN, "DMP   Gyro  Bias: X=%d, Y=%d, Z=%d\r\n", dmp_bias[3], dmp_bias[4], dmp_bias[5]); print_data_console(tst);
		break;

	case 'Q':
		dmp_reset_fifo();
		hal.report ^= PRINT_CUBE_RV;
		inv_enable_sensor(ANDROID_SENSOR_ROTATION_VECTOR, !!(hal.report & PRINT_CUBE_RV));
                dmp_reset_odr_counters();
		if (hal.report & PRINT_CUBE_RV) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "Cube RV....output toggled, now is: %s.\r\n", (hal.report & PRINT_CUBE_RV) ? "ON" : "OFF");
		print_command_console(tst);
		break;
	case '@':
		dmp_reset_fifo();
		hal.report ^= PRINT_CUBE_GEOMAG;
		inv_enable_sensor(ANDROID_SENSOR_GEOMAGNETIC_ROTATION_VECTOR, !!(hal.report & PRINT_CUBE_GEOMAG));
                dmp_reset_odr_counters();
		if (hal.report & PRINT_CUBE_GEOMAG) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "Cube GeoMAG RV....output toggled, now is: %s.\r\n", (hal.report & PRINT_CUBE_GEOMAG) ? "ON" : "OFF");
		print_command_console(tst);
		break;
#endif
	
	case 'v':
                dmp_reset_fifo();
		hal.report ^= PRINT_GRAVITY;
		inv_enable_sensor(ANDROID_SENSOR_GRAVITY, !!(hal.report & PRINT_GRAVITY));
                dmp_reset_odr_counters();
                if (hal.report & PRINT_GRAVITY) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "Gravity....output toggled, now is: %s.\r\n", (hal.report & PRINT_GRAVITY) ? "ON" : "OFF");
		print_command_console(tst);
		break;
	case 'l':
                dmp_reset_fifo();
		hal.report ^= PRINT_LINEAR_ACCEL;
		inv_enable_sensor(ANDROID_SENSOR_LINEAR_ACCELERATION, !!(hal.report & PRINT_LINEAR_ACCEL));
                dmp_reset_odr_counters();
                if (hal.report & PRINT_LINEAR_ACCEL) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "Linear Accel....output toggled, now is: %s.\r\n", (hal.report & PRINT_LINEAR_ACCEL) ? "ON" : "OFF");
		print_command_console(tst);
		break;
#if (MEMS_CHIP != HW_ICM20609)
	case 'o':
                dmp_reset_fifo();
		hal.report ^= PRINT_ORIENT;
		inv_enable_sensor(ANDROID_SENSOR_ORIENTATION, !!(hal.report & PRINT_ORIENT));
                dmp_reset_odr_counters();
                if (hal.report & PRINT_ORIENT) set_output_rates(current_output_rate);
		INV_SPRINTF(tst, INV_TST_LEN, "Orientation....output toggled, now is: %s.\r\n", (hal.report & PRINT_ORIENT) ? "ON" : "OFF");
		print_command_console(tst);
		break;

	case 't':
		dmp_reset_fifo();
		{
			int sensors_bias[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			char pass_str[] = { "PASS" };
			char fail_str[] = { "FAIL" };

			INV_SPRINTF(tst, INV_TST_LEN, "Selftest...Started\r\n");
			print_command_console(tst);

			self_test_result = inv_mems_run_selftest();
			//            self_test_result = 0x4 | 0x0 | 0x0;
			dmp_get_bias_20648(sensors_bias);

			dmp_bias[0] = a_average[0] * (1 << 11);   // Change from LSB to format expected by DMP
			dmp_bias[1] = a_average[1] * (1 << 11);
			dmp_bias[2] = (a_average[2] - 16384)*(1 << 11); //remove the gravity and scale (FSR=2 in selftest)
			int scale = 2000 / 250; //self-test uses 250dps FSR, main() set the FSR to 2000dps
			dmp_bias[3] = (g_average[0] / scale)*(1 << 15);
			dmp_bias[4] = (g_average[1] / scale)*(1 << 15);
			dmp_bias[5] = (g_average[2] / scale)*(1 << 15);

			INV_SPRINTF(tst, INV_TST_LEN, "Selftest...Done...Ret=%d\r\n", self_test_result);
			print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "Result: Compass=%s, Accel=%s, Gyro=%s\r\n", (self_test_result & 0x04) ? pass_str : fail_str, (self_test_result & 0x02) ? pass_str : fail_str, (self_test_result & 0x01) ? pass_str : fail_str);
			print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "Accel Average (LSB@FSR 2g)\r\n");
			print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "\tX:%d Y:%d Z:%d\r\n", a_average[0], a_average[1], a_average[2]);
			print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "Gyro Average (LSB@FSR 250dps)\r\n");
			print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "\tX:%d Y:%d Z:%d\r\n", g_average[0], g_average[1], g_average[2]);
			print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "Factory Cal - Accel DMP biases: \tX:%d Y:%d Z:%d\r\n", dmp_bias[0], dmp_bias[1], dmp_bias[2]);
			print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "Factory Cal - Gyro DMP biases:  \tX:%d Y:%d Z:%d\r\n", dmp_bias[3], dmp_bias[4], dmp_bias[5]);
			print_command_console(tst);

			if ((self_test_result & 0x3) == 0x3) {//Acc-Gyro self-test is passed already      
				INV_SPRINTF(tst, INV_TST_LEN, "\r\nSetting the DMP biases with one-axis factory calibration values...done\r\n"); print_command_console(tst);
				dmp_set_bias_20648(dmp_bias); //update the DMP biases
			}
                        
                        print_command_console("\n\n**RESTART BOARD**\n");

		}
		break;
#endif
		//case SHOW_COMMANDS:
	default:
	print_command_console("\r\n");
        print_command_console("Press 'a' to toggle ACCEL output....................\r\n");        
        print_command_console("Press 'g' to toggle Gyro output.....................\r\n");
        print_command_console("Press 'G' to toggle Raw Gyro output.................\r\n");        
        print_command_console("Press 'p' to toggle Step Counter output.............\r\n");     
        print_command_console("Press 'd' to toggle Step Detector output............\r\n");
        print_command_console("Press 'm' to toggle SMD Significant Motion output...\r\n");
#if (MEMS_CHIP != HW_ICM20609)        
        print_command_console("Press 'c' to toggle Compass output..................\r\n"); 
        print_command_console("Press 'C' to toggle Raw Compass output..............\r\n");
#endif
#if (MEMS_CHIP != HW_ICM20630)
#if (MEMS_CHIP != HW_ICM20609)
        print_command_console("Press 'b' to toggle Basic Activity Classifier output...\r\n");
        print_command_console("Press 'T' to toggle Tilt output...\r\n");
        print_command_console("Press 'F' to toggle Flip Pickup output...\r\n");
#endif
#endif
        print_command_console("Press 'r' to toggle Game Rotation Vector output.....\r\n");        
#if (MEMS_CHIP != HW_ICM20609)        
        print_command_console("Press 'R' to toggle Rotation Vector output..........\r\n");
        print_command_console("Press 'e' to toggle GeoMag Vector output............\r\n");
#if defined MEMS_AUGMENTED_SENSORS
        print_command_console("Press 'l' to toggle Linear Acceleration output............\r\n");
        print_command_console("Press 'v' to toggle Gravity output............\r\n");
        print_command_console("Press 'o' to toggle Orientation output............\r\n");
#endif

        print_command_console("Press 't' to invoke Self test.......................\r\n");
        print_command_console("Press 'k' to get the DMP Biases.................\r\n");
#endif
        print_command_console("Press '0' to set ODR @ 1Hz..........................\r\n");
        print_command_console("Press '1' to set ODR @ 5Hz..........................\r\n");
	print_command_console("Press '2' to set ODR @ 15Hz.........................\r\n");
#if (MEMS_CHIP == HW_ICM20609) 
        print_command_console("Press '3' to set ODR @ 50Hz.........................\r\n");
        print_command_console("Press '4' to set ODR @ 100Hz.........................\r\n");
        print_command_console("Press '5' to set ODR @ 200Hz.........................\r\n");
#endif
#if (MEMS_CHIP != HW_ICM20609)         
        print_command_console("Press '3' to set ODR @ 30Hz.........................\r\n");
        print_command_console("Press '4' to set ODR @ 51Hz.........................\r\n");
        print_command_console("Press '5' to set ODR @ 56Hz.........................\r\n");
        print_command_console("Press '6' to set ODR @ 60Hz.........................\r\n");
        print_command_console("Press '7' to set ODR @ 102Hz.........................\r\n");
        print_command_console("Press '8' to set ODR @ 112Hz.........................\r\n");
        print_command_console("Press '9' to set ODR @ 225Hz.........................\r\n");
        print_command_console("Press 'Q' to toggle Cube Rotation Vector output........\r\n");
        print_command_console("Press '@' to toggle Cube GeoMAG Vector output..........\r\n");      
#endif
        print_command_console("Press 'q' to toggle Cube Game Rotation Vector output...\r\n");
        print_command_console("\r\n");
        break;
	}

	return 1;
}

#if defined(RBLE_DATA_STORAGE_IN_FLASH)

/*
int rble_write_sample_data(nvms_t handle, uint32_t addr, const uint8_t *buf, uint32_t size)
{
	if()
	ad_nvms_write( handle,  addr,   *buf,  size);
}*/

bool rble_could_write_data_to_patition(uint32_t addr_offset,int count)
{
	if((addr_offset+count)<RBLE_DATA_PATITION_SIZE)
	{
		rble_data_patition_not_full=true;
		
	}
	else
	{
		rble_data_patition_not_full=false;
		
	}

	return rble_data_patition_not_full;
}
	
#endif

void process_sensor_output()
{
	
	signed long  long_quat[3] = { 0 };

	if (hal.report & PRINT_ACCEL) {
		if (accel_data_was_set == 1) {
			if (self_test_result && (accel_accuracy == 0)) //self-test is done already
				accel_accuracy = 1; //accuracy reaches intermediate level after one-axis factory cal--yd
			//INV_SPRINTF(tst, INV_TST_LEN, "Accel Data\t %8.5f, %8.5f, %8.5f, %d, %lld\r\n", accel_float[0], accel_float[1], accel_float[2], accel_accuracy, ts); print_command_console(tst);

			#if 1 //defined(RBLE_UART_DEBUG)
			memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
			qfp_float2str(accel_float[0],print_float_str,0);

			//printf("tk:%d",inv_get_tick_count());
			printf("acl0:");
			printf(print_float_str);
			fflush(stdout);
			#endif

			#if 1 //defined(RBLE_UART_DEBUG)
			memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
			
			qfp_float2str(accel_float[1],print_float_str,0);
			
			printf("acl1:");
			printf(print_float_str);
			fflush(stdout);
			#endif

			#if 1 //defined(RBLE_UART_DEBUG)
			memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
			qfp_float2str(accel_float[2],print_float_str,0);

			
			printf("acl2:");
			printf(print_float_str);
			printf("\n");
			fflush(stdout);
			#endif

			
			#if defined(RBLE_DATA_STORAGE_IN_FLASH)
				if(rble_data_patition_not_full)
				{
					memset(rble_sample_data,0,RBLE_DATA_BUF_LENGTH);
					rble_smp_count=0;
					//rble_sample_data[rble_smp_count]=RBLE_DATA_ACCEL_LABLE;
					//rble_smp_count++;
					memcpy(rble_sample_data,RBLE_DATA_ACCEL_LABLE,RBLE_DATA_LABLE_LENGTH);
					rble_smp_count+=RBLE_DATA_LABLE_LENGTH;
						
					memcpy((rble_sample_data+rble_smp_count),accel_float,(3*RBLE_FLOAT_SIZE));
					rble_smp_count+=(3*RBLE_FLOAT_SIZE);

					if(rble_could_write_data_to_patition(rble_data_addr_offset,rble_smp_count))
					{
						ad_nvms_write(nvms_rble_storage_handle, rble_data_addr_offset, rble_sample_data,rble_smp_count);
						rble_data_addr_offset+=rble_smp_count;
					}
				}
			#endif
			
			accel_data_was_set = 0;
		}
	}

	if (hal.report & PRINT_GYRO) {
		if (gyro_data_was_set == 1) {
			if (self_test_result && (gyro_accuracy == 0)) //self-test is done already
				gyro_accuracy = 1; //accuracy reaches intermediate level after one-axis factory cal--yd
                        INV_SPRINTF(tst, INV_TST_LEN, "Gyro Data\t %7.5f, %7.5f, %7.5f, %d, %lld\r\n", gyro_float[0], gyro_float[1], gyro_float[2], gyro_accuracy, ts); print_command_console(tst);

			#if 1 //defined(RBLE_UART_DEBUG)
			memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
						qfp_float2str(gyro_float[0],print_float_str,0);
			
			
						//printf("tk:%d",inv_get_tick_count());
						printf("gyro0:");
						printf(print_float_str);
						//printf("\n");
						fflush(stdout);
			#endif

			#if 1 //defined(RBLE_UART_DEBUG)
						memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
						
						qfp_float2str(gyro_float[1],print_float_str,0);
			
						printf("gyro1:");
						printf(print_float_str);
						//printf("\n");
						fflush(stdout);
			#endif

			#if 1 //defined(RBLE_UART_DEBUG)
						memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
						qfp_float2str(gyro_float[2],print_float_str,0);
			
			
						printf("gyro2:");
						printf(print_float_str);
						printf("\n");
						fflush(stdout);
			#endif

			#if defined(RBLE_DATA_STORAGE_IN_FLASH)
			if(rble_data_patition_not_full)
			{
				memset(rble_sample_data,0,RBLE_DATA_BUF_LENGTH);
				rble_smp_count=0;
				
				memcpy(rble_sample_data,RBLE_DATA_GYRO_LABLE,RBLE_DATA_LABLE_LENGTH);
				rble_smp_count+=RBLE_DATA_LABLE_LENGTH;
					
				memcpy((rble_sample_data+rble_smp_count),gyro_float,(3*RBLE_FLOAT_SIZE));
				rble_smp_count+=(3*RBLE_FLOAT_SIZE);

				if(rble_could_write_data_to_patition(rble_data_addr_offset,rble_smp_count))
				{
					ad_nvms_write(nvms_rble_storage_handle, rble_data_addr_offset, rble_sample_data,rble_smp_count);
					rble_data_addr_offset+=rble_smp_count;
				}
			}
			#endif


			gyro_data_was_set = 0;
		}
	}

	if (hal.report & PRINT_RAW_GYRO) {
#if ((MEMS_CHIP == HW_ICM30630) || (MEMS_CHIP == HW_ICM20648) || (MEMS_CHIP == HW_ICM20609))
		if (raw_gyro_data_was_set == 1) {
			INV_SPRINTF(tst, INV_TST_LEN, "Raw Gyro Data\t %7.5f, %7.5f, %7.5f,%d, %lld\r\n", gyro_raw_float[0], gyro_raw_float[1], gyro_raw_float[2], 0, ts); print_command_console(tst);
                        INV_SPRINTF(tst, INV_TST_LEN, "Gyro Bias\t %7.5f, %7.5f, %7.5f\r\n", gyro_bias_float[0], gyro_bias_float[1], gyro_bias_float[2]); print_command_console(tst);
			raw_gyro_data_was_set = 0;
		}
#endif
	}

	if (quat6_data_was_set == 1){
		if (hal.report & PRINT_GRV) {
			INV_SPRINTF(tst, INV_TST_LEN, "Game RV\t %7.5f, %7.5f, %7.5f, %7.5f, %lld\r\n", grv_float[0], grv_float[1], grv_float[2], grv_float[3], ts);
			print_data_console(tst);
                }
                if (hal.report & PRINT_CUBE_GRV) {
			dmp_get_6quaternion(long_quat);
			inv_compute_scalar_part(long_quat, result_quat);
			send_data_packet(PACKET_TYPE_QUAT, (void *)result_quat);
                }
#if (MEMS_CHIP != HW_ICM20609)                
#if defined MEMS_AUGMENTED_SENSORS       
		if (hal.report & PRINT_GRAVITY) {
			INV_SPRINTF(tst, INV_TST_LEN, "Gravity\t %7.5f, %7.5f, %7.5f, %lld\r\n", gravityFloat[0], gravityFloat[1], gravityFloat[2], ts);
			print_data_console(tst);
		}
		if (hal.report & PRINT_LINEAR_ACCEL) {
			INV_SPRINTF(tst, INV_TST_LEN, "LinearAcc\t %7.5f, %7.5f, %7.5f, %lld\r\n", linAccFloat[0], linAccFloat[1], linAccFloat[2], ts);
			print_data_console(tst);
		}
                
#endif      
#endif
                quat6_data_was_set = 0;
	}
#if (MEMS_CHIP != HW_ICM20609)
        if (hal.report & PRINT_COMPASS) {
		if (compass_data_was_set == 1) {
			INV_SPRINTF(tst, INV_TST_LEN, "Compass Data\t %7.5f, %7.5f, %7.5f,\t%d, %lld\r\n", compass_float[0], compass_float[1], compass_float[2], compass_accuracy, ts); print_command_console(tst);


		#if 1 //defined(RBLE_UART_DEBUG)

			memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
					qfp_float2str(compass_float[0],print_float_str,0);
					//printf("tk:%d",inv_get_tick_count());
					printf("cps0:");
					printf(print_float_str);
					//printf("\n");
					fflush(stdout);
		#endif

		#if 1 //defined(RBLE_UART_DEBUG)
					memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
					qfp_float2str(compass_float[1],print_float_str,0);
					printf("cps1:");
					printf(print_float_str);
					//printf("\n");
					fflush(stdout);
		#endif

		#if 1 //defined(RBLE_UART_DEBUG)
					memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
					qfp_float2str(compass_float[2],print_float_str,0);
					printf("cps2:");
					printf(print_float_str);
					printf("\n");
					fflush(stdout);
		#endif

		
		#if defined(RBLE_DATA_STORAGE_IN_FLASH)
		if(rble_data_patition_not_full)
		{
			memset(rble_sample_data,0,RBLE_DATA_BUF_LENGTH);
			rble_smp_count=0;
			
			memcpy(rble_sample_data,RBLE_DATA_COMPS_LABLE,RBLE_DATA_LABLE_LENGTH);
			rble_smp_count+=RBLE_DATA_LABLE_LENGTH;
				
			memcpy((rble_sample_data+rble_smp_count),compass_float,(3*RBLE_FLOAT_SIZE));
			rble_smp_count+=(3*RBLE_FLOAT_SIZE);
			if(rble_could_write_data_to_patition(rble_data_addr_offset,rble_smp_count))
			{
				ad_nvms_write(nvms_rble_storage_handle, rble_data_addr_offset, rble_sample_data,rble_smp_count);
				rble_data_addr_offset+=rble_smp_count;
			}
		}
		#endif

			compass_data_was_set = 0;
		}
	}

	if (hal.report & PRINT_RAW_COMPASS) {
		if (raw_compass_data_was_set == 1) {
			INV_SPRINTF(tst, INV_TST_LEN, "Raw Compass Data\t %7.5f, %7.5f, %7.5f,\t%d, %lld\r\n", compass_raw_float[0], compass_raw_float[1], compass_raw_float[2], 0, ts); print_command_console(tst);
			INV_SPRINTF(tst, INV_TST_LEN, "Compass Bias\t %7.3f, %7.3f, %7.3f\r\n", compass_bias[0], compass_bias[1], compass_bias[2]); print_command_console(tst);
			raw_compass_data_was_set = 0;
		}
	}

		
	if (quat9_data_was_set == 1) {
		if (hal.report & PRINT_RV) {
			INV_SPRINTF(tst, INV_TST_LEN, "RV\t %7.5f, %7.5f, %7.5f, %7.5f, %d, %lld\r\n", rv_float[0], rv_float[1], rv_float[2], rv_float[3], rv_accuracy, ts);
			print_data_console(tst);
		}
		if (hal.report &  PRINT_CUBE_RV) {
                  long temp_long_quat[3];
			dmp_get_9quaternion(long_quat);
                        inv_convert_rotation_vector_1(long_quat, temp_long_quat);
			inv_compute_scalar_part(temp_long_quat, result_quat);
			send_data_packet(PACKET_TYPE_QUAT, (void *)result_quat);

		}
#if defined MEMS_AUGMENTED_SENSORS
		if (hal.report & PRINT_ORIENT) {
			/*
			INV_SPRINTF(tst, INV_TST_LEN, "Orientation\t %7.5f, %7.5f, %7.5f, %lld\r\n", orientationFloat[0], orientationFloat[1], orientationFloat[2], ts);
			print_data_console(tst);
			*/
			
			#if 1 //defined(RBLE_UART_DEBUG)
				 // printf("fifo_handler,orientationFloat=%d,%d,%d\n",orientationFloat[0],orientationFloat[1],orientationFloat[2]);
				memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
				qfp_float2str(orientationFloat[0],print_float_str,0);
				printf("ori0:");
				printf(print_float_str);
				fflush(stdout);
				
				memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
				qfp_float2str(orientationFloat[1],print_float_str,0);
				printf("ori1:");
				printf(print_float_str);
				fflush(stdout);
				
				memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
				qfp_float2str(orientationFloat[2],print_float_str,0);
				printf("ori2:");
				printf(print_float_str);
				printf("\n");
				fflush(stdout);
			#endif
  
			
			#if defined(RBLE_DATA_STORAGE_IN_FLASH)
			if(rble_data_patition_not_full)
			{
				memset(rble_sample_data,0,RBLE_DATA_BUF_LENGTH);
				rble_smp_count=0;
				
				memcpy(rble_sample_data,RBLE_DATA_ORI_LABLE,RBLE_DATA_LABLE_LENGTH);
				rble_smp_count+=RBLE_DATA_LABLE_LENGTH;
					
				memcpy((rble_sample_data+rble_smp_count),orientationFloat,(3*RBLE_FLOAT_SIZE));
				rble_smp_count+=(3*RBLE_FLOAT_SIZE);

				if(rble_could_write_data_to_patition(rble_data_addr_offset,rble_smp_count))
				{
					ad_nvms_write(nvms_rble_storage_handle, rble_data_addr_offset, rble_sample_data,rble_smp_count);
					rble_data_addr_offset+=rble_smp_count;
				}
			}
			#endif
		}
		quat9_data_was_set = 0;
#endif
	}
      
	if (gmrv_quat_was_set == 1) {
		if (hal.report & PRINT_GEOMAG)
		{
			INV_SPRINTF(tst, INV_TST_LEN, "GeoMAG RV\t %7.5f, %7.5f, %7.5f, %7.5f, %d, %lld\r\n", gmrv_float[0], gmrv_float[1], gmrv_float[2], gmrv_float[3], gmrv_accuracy, ts);
			print_data_console(tst);
		}
                if (hal.report & PRINT_CUBE_GEOMAG) {
			send_data_packet(PACKET_TYPE_QUAT, (void *)cube_gmrv);
		}
                gmrv_quat_was_set = 0;
	}

	if (hal.report & PRINT_BAC) {
		if (bac_data_was_set == 1)
		{
			INV_SPRINTF(tst, INV_TST_LEN, "BAC Ts:\t %ld\r\n", bac_ts);
			print_data_console(tst);
			if ((bac_state >> 8) & BAC_DRIVE)
				print_data_console("\t Enter Drive\r\n");
			if ((bac_state >> 8) & BAC_WALK)
				print_data_console("\t Enter Walk\r\n");
			if ((bac_state >> 8) & BAC_RUN)
				print_data_console("\t Enter Run\r\n");
			if ((bac_state >> 8) & BAC_BIKE)
				print_data_console("\t Enter Bike\r\n");
			if ((bac_state >> 8) & BAC_TILT)
				print_data_console("\t Enter Tilt\r\n");
			if ((bac_state >> 8) & BAC_STILL)
				print_data_console("\t Enter Still\r\n");
			if (bac_state & BAC_DRIVE)
				print_data_console("\t Exit Drive\r\n");
			if (bac_state & BAC_WALK)
				print_data_console("\t Exit Walk\r\n");
			if (bac_state & BAC_RUN)
				print_data_console("\t Exit Run\r\n");
			if (bac_state & BAC_BIKE)
				print_data_console("\t Exit Bike\r\n");
			if (bac_state & BAC_TILT)
				print_data_console("\t Exit tilt\r\n");
			if (bac_state & BAC_STILL)
				print_data_console("\t Exit Still\r\n");

			bac_data_was_set = 0;
			//bac_prev_ts = ts;
		}
	}

	if (hal.report & PRINT_FLIP_PICKUP) {
		if (flip_data_was_set == 1) {
			if (flip_pickup == 1)
				print_data_console("\t Flip Detected\r\n");
			else if (flip_pickup == 2)
				print_data_console("\t Pickup Detected\r\n");
			flip_data_was_set = 0;
		}
	}

	if (hal.report & PRINT_TILT) {
		if (tilt_data_was_set == 1) {
			if (tilt_state == 2)
				print_data_console("\t Tilt Started\r\n");
			else if (tilt_state == 1)
				print_data_console("\t Tilt Ended\r\n");
			tilt_data_was_set = 0;
		}
	}
#endif     
        if (hal.report & PRINT_STEP_DETECTOR) {
                if (step_detected == 1)
                    print_data_console("Step Detected>>>>\r\n");
                step_detected = 0;
        }

	
}




void fifo_handler()
{
	short              int_read_back = 0;
	unsigned short     header = 0, header2 = 0;
	int data_left_in_fifo = 0;
	short short_data[3] = { 0 };
	signed long  long_data[3] = { 0 };
	signed long  long_quat[3] = { 0 };
	static mpu_time_t lastIrqTimeUs = 0;
	static mpu_time_t currentIrqTimeUs = 0;
	unsigned short sample_cnt_array[GENERAL_SENSORS_MAX] = { 0 };



		
	// Process Incoming INT and Get/Pack FIFO Data
	inv_identify_interrupt(&int_read_back);


#if 1//defined(RBLE_UART_DEBUG)
		printf("fifo=0x%x\n",int_read_back);
	fflush(stdout);

#endif

#if (MEMS_CHIP != HW_ICM20609)
	if (int_read_back & (BIT_MSG_DMP_INT | BIT_MSG_DMP_INT_0 | BIT_MSG_DMP_INT_2 | BIT_MSG_DMP_INT_5)) 
#else
        if (int_read_back & BIT_DMP_INT_CI)
#endif
        {
		// Read FIFO contents and parse it.
		unsigned short total_sample_cnt = 0;
		currentIrqTimeUs = inv_get_tick_count();	
                        
                        do {
			
        
			if (inv_mems_fifo_swmirror(&data_left_in_fifo, &total_sample_cnt, sample_cnt_array))
                          break;

			
				#if 1 //defined(RBLE_UART_DEBUG)
					printf("dleft=%d,tal_cnt=%d,arr[3]=%d\n",data_left_in_fifo,total_sample_cnt,sample_cnt_array[3]);
				#endif

			
#if defined(RBLE_DATA_STORAGE_IN_FLASH)
{
		long tmp_tick=0;

		if(rble_data_patition_not_full)
		{
			tmp_tick=inv_get_tick_count();
		
					memset(rble_sample_data,0,RBLE_DATA_BUF_LENGTH);
					rble_smp_count=0;
					//rble_sample_data[rble_smp_count]=RBLE_DATA_TK_LABLE;
					memcpy(rble_sample_data,RBLE_DATA_TK_LABLE,RBLE_DATA_LABLE_LENGTH);
					rble_smp_count+=RBLE_DATA_LABLE_LENGTH;
					
					memcpy((rble_sample_data+rble_smp_count),&tmp_tick,RBLE_LONG_SIZE);
					rble_smp_count+=RBLE_LONG_SIZE;

					memcpy((rble_sample_data+rble_smp_count),RBLE_DATA_CNT_LABLE,RBLE_DATA_LABLE_LENGTH);
					rble_smp_count+=RBLE_DATA_LABLE_LENGTH;
										
					memcpy((rble_sample_data+rble_smp_count),&total_sample_cnt,RBLE_CNT_SIZE);
					rble_smp_count+=RBLE_CNT_SIZE;

				if(rble_could_write_data_to_patition(rble_data_addr_offset,rble_smp_count))
				{
					ad_nvms_write(nvms_rble_storage_handle, rble_data_addr_offset, rble_sample_data,rble_smp_count);
					rble_data_addr_offset+=rble_smp_count;
				}
		}
}
#endif
						
			if (lastIrqTimeUs != 0)
				ts = (currentIrqTimeUs - lastIrqTimeUs) / total_sample_cnt;
                        
			lastIrqTimeUs = currentIrqTimeUs;
                        while (total_sample_cnt--){
				if (inv_mems_fifo_pop(&header, &header2, &data_left_in_fifo))
					break;

			#if defined(RBLE_UART_DEBUG)
					printf("hd=0x%x,hd2=0x%x\n",header,header2);
			#endif





				if (header & ACCEL_SET) {
					float scale;

					accel_data_was_set = 1;
					dmp_get_accel(long_data);

					accel_accuracy = inv_get_accel_accuracy();
					scale = (1 << inv_get_accel_fullscale()) * 2.f / (1L << 30); // Convert from raw units to g's
					scale *= 9.80665f; // Convert to m/s^2
					inv_convert_dmp3_to_body(long_data, scale, accel_float);

				}
				


				if (header & GYRO_SET) {
					float scale;
					signed long  raw_data[3] = { 0 };
#if ((MEMS_CHIP == HW_ICM30630) || (MEMS_CHIP == HW_ICM20648) || (MEMS_CHIP == HW_ICM20609))
					signed long  bias_data[3] = { 0 };
#endif

					raw_gyro_data_was_set = 1;
					dmp_get_raw_gyro(short_data);
                                        scale = (1 << inv_get_gyro_fullscale()) * 250.f / (1L << 15); // From raw to dps
					scale *= (float)M_PI / 180.f; // Convert to radian.
                                        raw_data[0] = (long)short_data[0];
                                        raw_data[1] = (long)short_data[1];
                                        raw_data[2] = (long)short_data[2];
                                        inv_convert_dmp3_to_body(raw_data, scale, gyro_raw_float);
                                        
#if ((MEMS_CHIP == HW_ICM30630) || (MEMS_CHIP == HW_ICM20648))
                                      // We have gyro bias data in raw units, scaled by 2^5
                                        scale = (1 << inv_get_gyro_fullscale()) * 250.f / (1L << 20); // From raw to dps
					scale *= (float)M_PI / 180.f; // Convert to radian.
					dmp_get_gyro_bias(short_data);
					bias_data[0] = (long)short_data[0];
					bias_data[1] = (long)short_data[1];
					bias_data[2] = (long)short_data[2];
					inv_convert_dmp3_to_body(bias_data, scale, gyro_bias_float);

                                      if(hal.report & PRINT_GYRO){
					// shift to Q20 to do all operations in Q20
					raw_data[0] = raw_data[0] << 5;
					raw_data[1] = raw_data[1] << 5;
					raw_data[2] = raw_data[2] << 5;
					inv_mems_dmp_get_calibrated_gyro(long_data, raw_data, bias_data);
					inv_convert_dmp3_to_body(long_data, scale, gyro_float);
                                        gyro_data_was_set = 1;
                                      }
                                      gyro_accuracy = inv_get_gyro_accuracy();
                                      
#elif (MEMS_CHIP == HW_ICM20609)           
                                      dmp_get_gyro_bias(bias_data);
                                        scale = (1 << inv_get_gyro_fullscale()) * 250.f / (1L << 20); // From raw to dps
					scale *= (float)M_PI / 180.f; // Convert to radian.
                                        inv_convert_dmp3_to_body(bias_data, scale, gyro_bias_float);
                                        
                                      if(hal.report & PRINT_GYRO){                                        
                                        inv_mems_dmp_get_calibrated_gyro(long_data, short_data, bias_data);
					// shift to Q30 to do all operations in Q30
					scale = (1 << inv_get_gyro_fullscale()) * 250.f / (1L << 30); // From raw to dps
					scale *= (float)M_PI / 180.f; // Convert to radian.
					
					inv_convert_dmp3_to_body(long_data, scale, gyro_float);
                                        
                                        gyro_data_was_set = 1;
                                      }
                                      if((bias_data[0] != 0) && (bias_data[1] != 0) && (bias_data[2] != 0))
                                        gyro_accuracy = 3;
#endif
				}



				if (header & GYRO_CALIBR_SET) {
#if ((MEMS_CHIP != HW_ICM30630) && (MEMS_CHIP != HW_ICM20648))
					float scale;
					gyro_data_was_set = 1;
					dmp_get_calibrated_gyro(long_data);

					gyro_accuracy = inv_get_gyro_accuracy();
					// We have gyro data in raw units, scaled by 2^15
					scale = (1 << inv_get_gyro_fullscale()) * 250.f / (1L << 30); // From raw to dps
					scale *= (float)M_PI / 180.f; // Convert to radian.
					inv_convert_dmp3_to_body(long_data, scale, gyro_float);
#endif
				}
#if (MEMS_CHIP != HW_ICM20609)
				if (header & CPASS_CALIBR_SET) {
					float scale;
					compass_data_was_set = 1;
					dmp_get_calibrated_compass(long_data);
					compass_accuracy = inv_get_mag_accuracy();
					scale = 1.52587890625e-005f; //COMPASS_CONVERSION
					inv_convert_dmp3_to_body(long_data, scale, compass_float);
				}

				if (header & CPASS_SET) {
					// Raw compass [DMP]
					dmp_get_raw_compass(long_data);
					compass_raw_float[0] = long_data[0] * 1.52587890625e-005f;
					compass_raw_float[1] = long_data[1] * 1.52587890625e-005f;
					compass_raw_float[2] = long_data[2] * 1.52587890625e-005f;
                                        compass_bias[0] = compass_raw_float[0] - compass_float[0];
                                        compass_bias[1] = compass_raw_float[1] - compass_float[1];
                                        compass_bias[2] = compass_raw_float[2] - compass_float[2];
					raw_compass_data_was_set = 1;
				}
#endif



#if 1 //defined(RBLE_QUAT6_SWITCH)
				//test_r


				if (header & QUAT6_SET) {

#if defined MEMS_AUGMENTED_SENSORS
			long gravityQ16[3], temp_gravityQ16[3];
			long linAccQ16[3];
			long accelQ16[3];
#endif
					
					quat6_data_was_set = 1;
					
					dmp_get_6quaternion(long_quat);
                                        inv_convert_rotation_vector(long_quat, grv_float);
#if defined MEMS_AUGMENTED_SENSORS

                                        /*Calculate Gravity*/
                                          inv_convert_rotation_vector_1(long_quat, temp_gravityQ16);
                                          inv_mems_augmented_sensors_get_gravity(gravityQ16, temp_gravityQ16);
                                          gravityFloat[0] = inv_q16_to_float(gravityQ16[0]);
                                          gravityFloat[1] = inv_q16_to_float(gravityQ16[1]);
                                          gravityFloat[2] = inv_q16_to_float(gravityQ16[2]);
                                        /*Calculate Linear Acceleration*/
                                          accelQ16[0] = (int32_t)((float)(accel_float[0])*(1ULL << 16) + ((accel_float[0] >= 0) - 0.5f));
                                          accelQ16[1] = (int32_t)((float)(accel_float[1])*(1ULL << 16) + ((accel_float[1] >= 0) - 0.5f));
                                          accelQ16[2] = (int32_t)((float)(accel_float[2])*(1ULL << 16) + ((accel_float[2] >= 0) - 0.5f));
                                          inv_mems_augmented_sensors_get_linearacceleration(linAccQ16, gravityQ16, accelQ16);
                                          linAccFloat[0] = inv_q16_to_float(linAccQ16[0]);
                                          linAccFloat[1] = inv_q16_to_float(linAccQ16[1]);
                                          linAccFloat[2] = inv_q16_to_float(linAccQ16[2]);
#endif                
				}
#endif

//test_r dump 



				
#if (MEMS_CHIP != HW_ICM20609)
				/* 9axis orientation quaternion sample available from DMP FIFO */

//test_r
#if 1
				if (header & QUAT9_SET) {
#if defined MEMS_AUGMENTED_SENSORS
					long orientationQ16[3],temp_orientationQ16[3];
#endif

					quat9_data_was_set = 1;
					dmp_get_9quaternion(long_quat);


					
#if (MEMS_CHIP == HW_ICM20630 || MEMS_CHIP == HW_ICM20648)
					rv_accuracy = (int)((float)inv_get_rv_accuracy() / (float)(1ULL << (29)));
#else
					compass_accuracy = inv_get_mag_accuracy();
#endif
					inv_convert_rotation_vector(long_quat, rv_float);



#if defined MEMS_AUGMENTED_SENSORS
                                        if(hal.report & PRINT_ORIENT){

										#if 1	
                                          inv_convert_rotation_vector_1(long_quat, temp_orientationQ16);
										
                                          inv_mems_augmented_sensors_get_orientation(orientationQ16, temp_orientationQ16);
										#endif  



											
											#if defined(RBLE_UART_DEBUG)
													//printf("fifo_handler,orientationQ16=%d,%d,%d\n",orientationQ16[0],orientationQ16[1],orientationQ16[2]);
													printf("abcd\n");
											#endif

										#if 0
										orientationQ16[0]=262144;
										orientationQ16[1]=262144;
										orientationQ16[2]=262144;
										#endif

										#if 1
										
										
                                          orientationFloat[0] = inv_q16_to_float(orientationQ16[0]);
                                          orientationFloat[1] = inv_q16_to_float(orientationQ16[1]);
                                          orientationFloat[2] = inv_q16_to_float(orientationQ16[2]);
										#endif
										  
											#if 0 //defined(RBLE_UART_DEBUG)
												 // printf("fifo_handler,orientationFloat=%d,%d,%d\n",orientationFloat[0],orientationFloat[1],orientationFloat[2]);
											memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
											qfp_float2str(orientationFloat[0],print_float_str,0);
											printf("ori0:");
											printf(print_float_str);
											fflush(stdout);

											memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
											qfp_float2str(orientationFloat[1],print_float_str,0);
											printf("ori1:");
											printf(print_float_str);
											fflush(stdout);

											memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
											qfp_float2str(orientationFloat[2],print_float_str,0);
											printf("ori2:");
											printf(print_float_str);
											printf("\n");
											fflush(stdout);
											#endif

											#if 0 //defined(RBLE_ORI_GET_ACCEL)
												inv_accel_read_hw_reg_data(rble_accel_raw);

												#if 1 //defined(RBLE_UART_DEBUG)
													printf("accel raw:%d,%d,%d\n",rble_accel_raw[0],
													rble_accel_raw[1],rble_accel_raw[2]);
												#endif

												inv_gyro_read_hw_reg_data(rble_gyro_raw);

												#if 1 //defined(RBLE_UART_DEBUG)
													printf("accel raw:%d,%d,%d\n",rble_gyro_raw[0],
													rble_gyro_raw[1],rble_gyro_raw[2]);
												#endif
											#endif
                                        }
#endif
				}

#endif

  
//test_r dump 	



#if (MEMS_CHIP == HW_ICM20648) 
				/* 6axis AM orientation quaternion sample available from DMP FIFO */
				if (header & GEOMAG_SET) {
					/* Read 6 axis quaternion out of DMP FIFO in Q30 and convert it to Android format */
					dmp_get_gmrvquaternion(long_quat);
                                        if(hal.report & PRINT_CUBE_GEOMAG)
                                                inv_compute_scalar_part(long_quat, cube_gmrv);
                                        if(hal.report & PRINT_GEOMAG){
                                                inv_convert_rotation_vector(long_quat, gmrv_float);
                                                /* Read geomagnetic rotation vector heading accuracy out of DMP FIFO in Q29*/
                                                gmrv_accuracy = (int)((float)inv_get_gmrv_accuracy() / (float)(1ULL << (29)));
                                        }
                                        gmrv_quat_was_set = 1;
				}
#endif

//////////test_r dump

  ///test_r dump                              
#if (MEMS_CHIP == HW_ICM20645e) || (MEMS_CHIP == HW_ICM20648) 
				/* Activity recognition sample available from DMP FIFO */


				if (header2 & ACT_RECOG_SET) {
					/* Read activity type and associated timestamp out of DMP FIFO
					activity type is a set of 2 bytes :
					- high byte indicates activity start
					- low byte indicates activity end */
					dmp_get_bac_state(&bac_state);
					dmp_get_bac_ts(&bac_ts);

					if (hal.report & PRINT_TILT) {
						/* Tilt information is inside BAC, so extract it out */

						/* Check if bit tilt is set for activity start byte */
						if ((bac_state >> 8) & BAC_TILT) {
							/* Start of tilt detected */
							tilt_state = 2;
						}
						/* Check if bit tilt is set for activity end byte */
						else if (bac_state & BAC_TILT) {
							/* End of tilt detected */
							tilt_state = 1;
						}

					}

					if (hal.report & PRINT_BAC)
						bac_data_was_set = 1;
					if (hal.report & PRINT_TILT)
						tilt_data_was_set = 1;
				}





				if (header2 & FLIP_PICKUP_SET) {
					if (hal.report & PRINT_FLIP_PICKUP) {
						dmp_get_flip_pickup_state(&flip_pickup);
						flip_data_was_set = 1;
					}
				}
#endif
#endif

//test_r dump	
#if 1
				process_sensor_output();
#endif


			}
			if (!data_left_in_fifo)
				break;

		} while (data_left_in_fifo);

#if (MEMS_CHIP == HW_ICM20609)                
		if (int_read_back & DMP_INT_SMD) {
                  if (hal.report & PRINT_SMD){
				print_command_console(">> SMD Interrupt *********\r\n");
                  }
                        
		}
#endif
#if (MEMS_CHIP != HW_ICM20609)                
                if (int_read_back & BIT_MSG_DMP_INT_3) {
#else
		if (int_read_back & DMP_INT_PED) {
#endif                  
                  if (hal.report & PRINT_STEP_DETECTOR){
				print_command_console("Step Detected>>>>>>>\r\n");
                                
                  }
                }
#if (MEMS_CHIP != HW_ICM20609)                
                if (header & PED_STEPDET_SET){
#else
		if (int_read_back & DMP_INT_PED) {
#endif
                        if (hal.report & PRINT_STEP_COUNTER){
				unsigned long steps = 0;
				static unsigned long old_steps;
				dmp_get_pedometer_num_of_steps(&steps);

				if (steps != old_steps){
					INV_SPRINTF(tst, INV_TST_LEN, "\tStep Counter %d\r\n", steps);
					print_command_console(tst);
					old_steps = steps;
				}
                        }
		}

#if (MEMS_CHIP == HW_ICM20648)
                if (int_read_back & BIT_MSG_DMP_INT_2) {
			if (hal.report & PRINT_SMD)
				print_command_console(">> SMD Interrupt *********\r\n");
		}
#endif
       }
}


//////////////////////////

#if 0
double rble_asin(double x)
{
    return asin(x);
}

double rble_atan(double x)
{
    return atan(x);
}

double rble_atan2(double x, double y)
{
    return atan2(x, y);
}

double rble_log(double x)
{
    return log(x);
}

double rble_sqrt(double x)
{
    return sqrt(x);
}

double rble_ceil(double x)
{
    return ceil(x);
}

double rble_floor(double x)
{
    return floor(x);
}

double rble_cos(double x)
{
    return cos(x);
}

double rble_sin(double x)
{
    //return sin(x);
    return  qfp_fsin(x);
}
 
double rble_acos(double x)
{
    return acos(x);
}

double rble_pow(double x, double y)
{
    return pow(x, y);
}
#endif

///////////////////

 
void RbleGetAllSensorData(void)
{
	
    #if defined(RBLE_UART_DEBUG)
    	printf("RbleGetAllSensorData\n");
    #endif
}


int sensor_write_register(i2c_device dev, uint8_t reg, uint8_t value)
{
        sensor_err_t res;
        uint8_t data[2] = {
                        reg,
                        value,
        };

        res = ad_i2c_write(dev, data, 2);

        return res;
}



sensor_err_t sensor_read_register(i2c_device dev, uint8_t reg, uint8_t *value, uint8_t length)
{
        sensor_err_t res;
        res = ad_i2c_transact(dev, &reg, 1, value, length);
        return res;
}


int sensor_write_registerb( uint8_t reg, uint8_t value)
{
		i2c_device dev;
        sensor_err_t res;
        uint8_t data[2] = {
                        reg,
                        value,
        };

		dev = ad_i2c_open(ICM20648_ACC);
        res = ad_i2c_write(dev, data, 2);
		ad_i2c_close(dev);

        return res;
}



sensor_err_t sensor_read_registerb( uint8_t reg, uint8_t *value, uint8_t length)
{
		i2c_device dev;
        sensor_err_t res;

		dev = ad_i2c_open(ICM20648_ACC);
        res = ad_i2c_transact(dev, &reg, 1, value, length);
		ad_i2c_close(dev);
        return res;
}



int rble_i2c_test_read(i2c_device dev, uint16_t *value)
{
        uint8_t buf[2];
        int err;

        err = ad_i2c_read(dev, buf, sizeof(buf));
        if (0 == err) {
                *value = (buf[0] << 8) | buf[1];
        }
        return err;
}


#if 1

uint8_t RbleReadAccId(void)
{
	uint8_t id=0;
	int tmp=7;
	uint8_t data_tmp=0;
	int i=0;


	//sensor_write_registerb(0x06,0x41);
	
	sensor_read_registerb( 0x06, &data_tmp, 1);
	//sensor_read_registerb( 0x03, &data_tmp, 1);
	#if defined(RBLE_UART_DEBUG)
		printf("RbleReadAccId f,data_tmp=0x%x\n",data_tmp);
	#endif
	//sensor_write_registerb(0x06,0x41);
	sensor_write_registerb(0x06,0x40);
	//sensor_write_registerb(0x03,0x80);
	
	data_tmp=0;
	
	sensor_read_registerb( 0x06, &data_tmp, 1);
	//sensor_read_registerb( 0x03, &data_tmp, 1);
	#if defined(RBLE_UART_DEBUG)
		printf("RbleReadAccId i,data_tmp=0x%x\n",data_tmp);
	#endif

	sensor_write_registerb(0x06,0x41);
	//sensor_write_registerb(0x06,0x40);
	//sensor_write_registerb(0x03,0x40);

	data_tmp=0;

	//sensor_read_registerb( 0x06, &data_tmp, 1);
	sensor_read_registerb( 0x03, &data_tmp, 1);
	
	#if defined(RBLE_UART_DEBUG)
		printf("RbleReadAccId j,data_tmp=0x%x\n",data_tmp);
	#endif


	//sensor_read_registerb( 0x75, &id, 1);
	sensor_read_registerb( 0x00, &id, 1);

    #if defined(RBLE_UART_DEBUG)
    	//printf("RbleReadAccId,tmp=%d\n",tmp);
    	printf("RbleReadAccId,id=%d\n",id);
    	//printf("RbleReadAccId,tmp=%s\n","abc");
	//printf("RbleReadAccId aaaaa\n");

		fflush(stdout);

	#if 0
	if(0==id)
	{
		printf("RbleReadAccId id,0\n");
	}
	else if(0xe0==id)
	{
		printf("RbleReadAccId id 0xe0\n");
	}
	else
	{
		printf("RbleReadAccId id not 0\n");
	}
	#endif
	
	fflush(stdout);

        #endif

	//ad_i2c_close(rble_sensor_control_env.dev);

	return id;
}

#else

uint8_t RbleReadAccId(void)
{
	uint8_t id=0;
	int tmp=7;
	uint8_t data_tmp=0;
	int i=0;

	//rble_sensor_control_env.dev = ad_i2c_open(ICM20648_ACC);

	/*
	sensor_write_register(rble_sensor_control_env.dev,0x06,0x80);
	for(;i<100;i++)
	{
		data_tmp=0;
	}
	*/
	
	//sensor_write_register(rble_sensor_control_env.dev,0x06,0x41);
	
	sensor_read_register(rble_sensor_control_env.dev, 0x06, &data_tmp, 1);
	#if defined(RBLE_UART_DEBUG)
		printf("RbleReadAccId b,data_tmp=0x%x\n",data_tmp);
	#endif
	sensor_write_register(rble_sensor_control_env.dev,0x06,0x41);
	//sensor_write_register(rble_sensor_control_env.dev,0x06,0x40);
	
	data_tmp=0;
	
	sensor_read_register(rble_sensor_control_env.dev, 0x06, &data_tmp, 1);
	#if defined(RBLE_UART_DEBUG)
		printf("RbleReadAccId i,data_tmp=0x%x\n",data_tmp);
	#endif

	//sensor_write_register(rble_sensor_control_env.dev,0x06,0x41);
	sensor_write_register(rble_sensor_control_env.dev,0x06,0x40);

	data_tmp=0;

	sensor_read_register(rble_sensor_control_env.dev, 0x06, &data_tmp, 1);
	#if defined(RBLE_UART_DEBUG)
		printf("RbleReadAccId j,data_tmp=0x%x\n",data_tmp);
	#endif


	//sensor_read_register(rble_sensor_control_env.dev, 0x75, &id, 1);
	sensor_read_register(rble_sensor_control_env.dev, 0x00, &id, 1);

    #if defined(RBLE_UART_DEBUG)
    	//printf("RbleReadAccId,tmp=%d\n",tmp);
    	printf("RbleReadAccId,id=%d\n",id);
    	//printf("RbleReadAccId,tmp=%s\n","abc");
	//printf("RbleReadAccId aaaaa\n");

		fflush(stdout);

	#if 0
	if(0==id)
	{
		printf("RbleReadAccId id,0\n");
	}
	else if(0xe0==id)
	{
		printf("RbleReadAccId id 0xe0\n");
	}
	else
	{
		printf("RbleReadAccId id not 0\n");
	}
	#endif
	
	fflush(stdout);

        #endif

	//ad_i2c_close(rble_sensor_control_env.dev);

	return id;
}
#endif

void RbleTest(void)
{
	float a=3.1415;
	double b= 2.987;
	double c= a*b;
	double d=a/b;
	char str[100];

	memset(str,0,100);
	sprintf(str,"c:%f,d:%f",c,d);
		
	#if defined(RBLE_UART_DEBUG)
		printf("RbleTest a=%f,b=%f,c=%f,d=%f\n",a,b,c,d);
		printf("str:%s",str);
		printf(str);

		if(c>9)
		{
		        printf("c>9");
		}
		else
		{
		    printf("c<9");
		}
	#endif
}


void RbleSensorControlInit(void)
{
float a=3.1415;
float b=4.8;
float c = 3.2;
float d=3.f;

	#if defined(RBLE_UART_DEBUG)
	printf("RbleSensorControlInit a\n");
	#endif

	c=a+b;

#if defined(RBLE_UART_DEBUG)

	if(c>7.f)
	{
	        printf("RbleSensorControlInit c>7.f\n");
	}
	else
	{
	        printf("RbleSensorControlInit c<<<<<<<\n");
	}

	fflush(stdout);
#endif

	RbleReadAccId();


#if 0
	/* Initialize timers */
        rble_sensor_control_env.tim_sensor_all_get = OS_TIMER_CREATE( "sensor_all_get", OS_MS_2_TICKS(RBLE_GET_ALL_SENSOR_INTERVAL),
                OS_TIMER_SUCCESS, (void *) OS_GET_CURRENT_TASK(), RbleGetAllSensorData );

	OS_TIMER_START(rble_sensor_control_env.tim_sensor_all_get,  OS_TIMER_FOREVER);
	//OS_TIMER_RESET(rble_sensor_control_env.tim_sensor_all_get,  RBLE_GET_ALL_SENSOR_INTERVAL);

#endif

}


#if defined(RBLE_SAMPLE_TIMER_SWITCH)

void rble_sample_timer_cb(OS_TIMER timer)
{
	
	OS_TASK task = (OS_TASK) OS_TIMER_GET_TIMER_ID(timer);
	
	

	#if defined(RBLE_UART_DEBUG)
		//printf("tmr tick=%d\n",tmp_tick);
		printf("timer");
		fflush(stdout);	
	#endif

	

    OS_TASK_NOTIFY(task, RBLE_SAMPLE_TIMER_NOTIF, OS_NOTIFY_SET_BITS);
        
}


void rble_sample_timer_enable(bool enabled)
{
        if (enabled) {
                OS_TIMER_START(rble_sample_timer_id, OS_TIMER_FOREVER);
        } else {
                OS_TIMER_STOP(rble_sample_timer_id, OS_TIMER_FOREVER);
        }
}


void rble_sample_start_timer(void)
{

	TickType_t period_tick= 51;         //51 (100ms);
	//10 / OS_PERIOD_MS; ///500;   ////      10 / OS_PERIOD_MS;   //

	#if 1 //defined(RBLE_UART_DEBUG)
			printf("period_tick=%d\n",period_tick);
			fflush(stdout); 
	#endif

	rble_sample_timer_id = OS_TIMER_CREATE("rblesampletimer", period_tick, OS_TIMER_SUCCESS,
                                                (void *) OS_GET_CURRENT_TASK(), rble_sample_timer_cb);

	rble_sample_timer_enable(true);
}

#endif

void RbleSensorControlTask( void *pvParameters )
{

        OS_TICK_TIME xNextWakeTime;
        static uint32_t test_counter=0;
		int8_t wdog_id;

        float a=3.14159265;
        //float b= sin(a/6);
        //float e=rble_sin(a/4);
        float d=sqrt(9.1);
		

		inv_error_t result;
		char c ='a';
		//char c ='t';
		//char c ='r';
		//char c ='o';
		
			////'p';

			//e=rble_sin(a/6);
			//e=qfp_fsin(3.14159265/6);
			
			//e=sin(a);
			//memset(print_float_str,0,RBLE_FLOAT_STR_LENTH);
			//qfp_float2str(e,print_float_str,0);
        #if defined(RBLE_UART_DEBUG)
               // printf("RbleSensorControlTask e=%d\n kkkk",e);
			//printf("RbleSensorControlTask a=%s\n","2.22266");
			//printf(print_float_str);
        #endif
			//d=a+e;
			///rble_sensor_control_env.dev = ad_i2c_open(ICM20648_ACC);

			/////i2c_gsensor_dev=ad_i2c_open(ICM20648_ACC);


	//RbleSensorControlInit();
	//RbleTest();


#if defined(RBLE_DATA_STORAGE_IN_FLASH)
	nvms_rble_storage_handle=ad_nvms_open(NVMS_IMAGE_DATA_STORAGE_PART);

#endif




#if 1

#if 1  //defined(RBLE_UART_DEBUG)
		printf("RbleSensorControlTask aa\n");
	fflush(stdout);

#endif

	print_command_console("\nWelcome on shoes board !!!\r\n");

#if defined(RBLE_UART_DEBUG)
		printf("RbleSensorControlTask bb\n");
#endif

	/* Setup accel and gyro mounting matrix and associated angle for current board */
	inv_set_chip_to_body_axis_quaternion(ACCEL_GYRO_ORIENTATION, 0.0);
	result = inv_initialize_lower_driver(SERIAL_INTERFACE_I2C, 0);
#if defined MEMS_SECONDARY_DEVICE
	inv_set_slave_compass_id(0x24);
#endif

	if (result) {
		print_command_console("Could not initialize.\r\n");
	}

#if defined(RBLE_UART_DEBUG)
	printf("RbleSensorControlTask result=%d\n",result);
#endif


	//handle_char_input(c);

	handle_char_input('a');
	handle_char_input('g');
	handle_char_input('c');

	handle_char_input('o');


#if defined(RBLE_UART_DEBUG)
		printf("RbleSensorControlTask cccx\n");
#endif

//test_r
#if 0
	if(0)  ////(0)//(1)// (hal.new_gyro == 1)
	{
		hal.new_gyro = 0;
		inv_sleep(10);

		
		#if defined(RBLE_UART_DEBUG)
			printf("RbleSensorControlTask dddx\n");
		#endif
		
		fifo_handler();
	}
#endif


#if defined(RBLE_UART_DEBUG)
	printf("RbleSensorControlTask eeex\n");
#endif

#endif

#if defined(RBLE_SAMPLE_TIMER_SWITCH)
rble_sample_start_timer();
#endif

#if defined(RBLE_SAMPLE_TIMER_SWITCH)
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
                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(ret == OS_OK);

                /* resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                /* Notified from BLE Manager? */
                

                
                

                if (notif & RBLE_SAMPLE_TIMER_NOTIF) {
					long tmp_tick=0;

					
						#if 1 //defined(RBLE_UART_DEBUG)
						tmp_tick=inv_get_tick_count();
						   printf("tick=%d\n",tmp_tick);
						   //printf("timer");
						   fflush(stdout); 
						#endif

                       fifo_handler();
                    

					   
                }

                
        }


#else


        /* Initialise xNextWakeTime - this only needs to be done once. */
        xNextWakeTime = OS_GET_TICK_COUNT();

	for( ;; ) {
                /* Place this task in the blocked state until it is time to run again.
                   The block time is specified in ticks, the constant used converts ticks
                   to ms.  While in the Blocked state this task will not consume any CPU
                   time. */
                vTaskDelayUntil( &xNextWakeTime, mainCOUNTER_FREQUENCY_MS );
                test_counter++;

                if (test_counter % (1000 / OS_TICKS_2_MS(mainCOUNTER_FREQUENCY_MS)) == 0) 
				{

					//RbleSensorControlInit();
					fifo_handler();
					
					
		#if defined(RBLE_UART_DEBUG)
                        printf("#");
						printf("*");
						printf("8");
                        fflush(stdout);
						
		#endif
                }
        }
#endif

OS_TASK_DELETE(OS_GET_CURRENT_TASK());


}

