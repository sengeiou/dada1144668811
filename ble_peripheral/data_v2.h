/*
 * data.h
 *
 *  Created on: 2017Äê2ÔÂ27ÈÕ
 *      Author: Administrator
 */

#ifndef DATA_H_V2
#define DATA_H_V2

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


typedef struct step_env{
    uint8_t flag;
    uint8_t mode;   //step mode: walk,run,dash
    uint8_t type; //0:x,1:y,2:z
    uint8_t frequency;
    uint8_t last_status_is_up;
    uint8_t direction_is_up;
    float acc_x_old;
    float acc_y_old;
    float acc_z_old;
    float acc_x_new;
    float acc_y_new;
    float acc_z_new;
    float acc_x2_old;
    float acc_y2_old;
    float acc_z2_old;
    float acc_x2_new;
    float acc_y2_new;
    float acc_z2_new;

    float min_acc_value;
    float max_acc_value;

    float peak_wave;
    float valley_wave;

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
    
    
    long time_of_last_peak;
    long time_of_this_peak;
    long time_of_now;

    int continue_up_count;
    int continue_up_former_count;
    
    
}step_env_t;


void init_step_env();
void detect_new_step_v2(float acc_x2,float acc_y2,float acc_z2);


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
