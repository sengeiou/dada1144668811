#pragma once
#if defined(BLE_USE_DATA_V5)

#ifndef DATA_H_V5
#define DATA_H_V5

#define DATA_ABS(x) ((x)>=0?(x):(-x))
//x2 y2 z2 means |x| |y| |z|

#define FIFO_OFFSET_TICK (20*512/1000)

#define DATA_JUMP_HEIGHT(t) (0.5)*9.8*(t)*(t)
#define TICK_TO_MS (1000.0f/512.0f)

#define LINE_TO_TICK (20.0f*512.0f/1000.0f)

#define MAX(a,b) ((a)>(b)?(a):(b))
typedef  unsigned char uint8_t;

typedef enum {
	WALK = 0x0,
	RUN = 0x1,
	DASH = 0x2,
}step_mode_t;

typedef enum {
	VERTICAL = 0x0,
	HORIZONTAL = 0x1,
	JUMP = 0x2,
}step_type_t;

typedef enum {
	LEFT = 0x1,
	RIGHT = 0x2,
	FORWARD = 0x3,
	BACKWARD = 0x4,
}step_ori_t;

typedef enum {
	ACC_PEAK_X = 0x0,
	ACC_PEAK_Y = 0x1,
	ACC_PEAK_Z = 0x2,
	ACC_PEAK_X2 = 0x3,
	ACC_PEAK_Y2 = 0x4,
	ACC_PEAK_Z2 = 0x5,
}detect_peak_acc_mode_t;



typedef struct {
	float acc_x_old;
	float acc_y_old;
	float acc_z_old;
	float acc_x2_old;
	float acc_y2_old;
	float acc_z2_old;

}acc_value_mode_t;


typedef struct {
	uint8_t acc_x;
	uint8_t acc_y;
	uint8_t acc_z;
	uint8_t acc_x2;
	uint8_t acc_y2;
	uint8_t acc_z2;

}last_status_is_up_t;

typedef struct {
	uint8_t acc_x;
	uint8_t acc_y;
	uint8_t acc_z;
	uint8_t acc_x2;
	uint8_t acc_y2;
	uint8_t acc_z2;

}direction_is_up_t;

typedef struct {
	int acc_x;
	int acc_y;
	int acc_z;
	int acc_x2;
	int acc_y2;
	int acc_z2;

}continue_up_count_t;

typedef struct {
	int acc_x;
	int acc_y;
	int acc_z;
	int acc_x2;
	int acc_y2;
	int acc_z2;

}continue_up_former_count_t;

typedef struct {
	float acc_x;
	float acc_y;
	float acc_z;
	float acc_x2;
	float acc_y2;
	float acc_z2;

}peak_wave_t;

typedef struct {
	float acc_x;
	float acc_y;
	float acc_z;
	float acc_x2;
	float acc_y2;
	float acc_z2;

}valley_wave_t;

typedef struct {
	uint8_t flag;
	long fir_peak_time;
	long sec_peak_time;
	long  count;
	float time;
	float air_time;
	float height;
	float value;
	float fir_peak_value;
	float sec_peak_value;
	float jump_ori;
}jump_t;

typedef struct {
	uint8_t flag;
	long fir_peak_time;
	long sec_peak_time;
	long  count;
	float time;
	float air_time;
	float height;
	float value;
	float jump_ori;

}last_jump_t;

typedef struct {
	uint8_t flag;
	long fir_peak_time;
	long sec_peak_time;
	long  count;
	float time;
	float air_time;
	float height;
	float value;
	float jump_ori;

}temp_jump_t;

typedef struct
{
	uint8_t mode;
	uint8_t flag;
	uint8_t ori;
	long fir_peak_time;
	long sec_peak_time;
	float fir_peak_value;
	float sec_peak_value;
	float value;

}x_step_t;

typedef struct
{
	uint8_t mode;
	uint8_t flag;
	uint8_t ori;
	long fir_peak_time;
	long sec_peak_time;
	float fir_peak_value;
	float sec_peak_value;
	float value;

}y_step_t;


typedef struct {
	uint8_t ori;
	uint8_t type;
	uint8_t mode;
	long time;
	float air_time;
	float length;
	long fir_peak_time;
	long sec_peak_time;
	float value;

}last_step_t;

typedef struct {
	uint8_t flag;
	uint8_t ori;
	uint8_t type;
	uint8_t mode;
	long time;
	float air_time;
	float jump_ori;
	float length;
	long fir_peak_time;
	long sec_peak_time;
	float value;

}temp_step_t;





typedef struct step_env {
	uint8_t flag;
	uint8_t mode;   //step mode: walk,run,dash
	uint8_t type; //0:x,1:y,2:z
	uint8_t ori;
	int jump_air_count;
	long time;
	long fir_peak_time;
	long sec_peak_time;
	float value;
	float frequency;
	uint8_t detect_peak_mode;
	last_status_is_up_t last_status_is_up;
	direction_is_up_t direction_is_up;

	acc_value_mode_t acc_value_mode;

	float min_acc_value;
	float max_acc_value;

	peak_wave_t peak_wave;
	valley_wave_t valley_wave;



	long total_step;
	long total_run;
	long total_dash;
	long h_step;
	long h_run;
	long h_dash;
	long cod_num;
	long left_num;
	long right_num;
	long forward_num;
	long backward_num;
	jump_t jump;
	float jump_ori;
	float jump_air_time;
	float jump_height;
	float jump_time;
	long jump_count;
	x_step_t x_step;
	y_step_t y_step;
	temp_step_t temp_step;
	last_jump_t last_jump;
	temp_jump_t temp_jump;

	float stride;
	float distance;
	float h_distance;

	float max_v;
	long  step_change;
	long  step_change_time;


	float coord_x;
	float coord_y;
	float yaw_old;
    float yaw_offset;

	long time_of_now;

	last_step_t last_step;

	continue_up_count_t continue_up_count;
	continue_up_former_count_t continue_up_former_count;


}step_env_t;

step_env_t step_env;
void init_step_env();
void init_yaw_offset(float f);
//void detect_new_step_v2(float acc_x, float acc_y, float acc_z, int line);
void detect_new_step_v5(float acc_x,float acc_y,float acc_z,float gyr_y,float yaw,unsigned short fifo_id);
void print_result();
void write_track_to_flash();
void write_result_to_flash();

#if 0
void detect_new_step(float values);

#define RBLE_RESULT_DATA_PATITION_SIZE  (0x005000)
#define RBLE_RESULT_DATA_BUF_LENGTH    32
#define RBLE_RESULT_FLOAT_SIZE   4
#define RBLE_RESULT_INT_SIZE   4

#define RBLE_RESULT_DATA_LABLE_LENGTH  2
#define RBLE_RESULT_DATA_LABLE_SC "sc" //step count
#define RBLE_RESULT_DATA_LABLE_RC "rc" //run count
#define RBLE_RESULT_DATA_LABLE_DC "dc" //dash count
#endif




#endif /* DATA_H_V5 */
#endif