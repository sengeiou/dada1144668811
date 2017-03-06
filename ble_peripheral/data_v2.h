/*
 * data.h
 *
 *  Created on: 2017Äê2ÔÂ27ÈÕ
 *      Author: Administrator
 */

#ifndef DATA_H_V2
#define DATA_H_V2

#define DATA_ABS(x) (x)>0?(x):(-x)
//x2 y2 z2 means |x| |y| |z|

#define FIFO_OFFSET_TICK (20*512/1000)

typedef enum {
    WALK=0x0,
    RUN=0x1,
    DASH=0x2,
}step_mode_t;

typedef enum {
    VERTICAL=0x0,
    HORIZONTAL=0x1,
    JUMP=0x2,
}step_type_t;

typedef enum {
    ACC_PEAK_X=0x0,
    ACC_PEAK_Y=0x1,
    ACC_PEAK_Z=0x2,
    ACC_PEAK_X2=0x3,
    ACC_PEAK_Y2=0x4,
    ACC_PEAK_Z2=0x5,
}detect_peak_acc_mode_t;



typedef struct{
    float acc_x_old;
    float acc_y_old;
    float acc_z_old;
    float acc_x2_old;
    float acc_y2_old;
    float acc_z2_old;
}acc_value_mode_t;


typedef struct{
    uint8_t acc_x;
    uint8_t acc_y;
    uint8_t acc_z;
    uint8_t acc_x2;
    uint8_t acc_y2;
    uint8_t acc_z2;

}last_status_is_up_t;

typedef struct{
    uint8_t acc_x;
    uint8_t acc_y;
    uint8_t acc_z;
    uint8_t acc_x2;
    uint8_t acc_y2;
    uint8_t acc_z2;

}direction_is_up_t;

typedef struct{
    int acc_x;
    int acc_y;
    int acc_z;
    int acc_x2;
    int acc_y2;
    int acc_z2;

}continue_up_count_t;

typedef struct{
    int acc_x;
    int acc_y;
    int acc_z;
    int acc_x2;
    int acc_y2;
    int acc_z2;

}continue_up_former_count_t;

typedef struct{
        float acc_x;
        float acc_y;
        float acc_z;
        float acc_x2;
        float acc_y2;
        float acc_z2;

}peak_wave_t;

typedef struct{
        float acc_x;
        float acc_y;
        float acc_z;
        float acc_x2;
        float acc_y2;
        float acc_z2;

}valley_wave_t;



typedef struct step_env{
    uint8_t flag;
    uint8_t mode;   //step mode: walk,run,dash
    uint8_t type; //0:x,1:y,2:z
    uint8_t frequency;
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
    long jump;

    float stride;
    long distance;
    long h_distance;
    
    
    long eff_time_of_last_peak;
    long eff_time_of_this_peak;
    long time_of_now;

    continue_up_count_t continue_up_count;
    continue_up_former_count_t continue_up_former_count;
    
    
}step_env_t;


void init_step_env();
void detect_new_step_v2(float acc_x2,float acc_y2,float acc_z,unsigned short fifo_id);


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




#endif /* DATA_H_V2 */
