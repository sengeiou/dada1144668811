
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "data.h"
#include "ad_nvms.h"

//last A-xyz values
float a_xyz_old=0;
float a_xyz_new=0;

float min_a_xyz=11;
float max_a_xyz=19.6;

// time tick 51 == 100ms
long time_of_last_peak=0L;
long time_of_this_peak=0L;

long time_of_now=0L;

#define TICK_TO_MS (100/51)


float peak_wave=0;
float valley_wave=0;
float initial_value=1.7;
float thread_value=2.0f;


static long current_step=0;
static long current_run_count=0;
static long current_dash_count=0;




#define VALUE_NUM 5
int thread_temp_count=0;
float thread_temp_value[VALUE_NUM]={0};



int last_status=0;
int is_direction_up=0;

int continue_up_count=0;
int continue_up_former_count=0;




nvms_t nvms_rble_result_storage_handle;
uint32_t rble_result_data_addr_offset=0;
uint8_t rble_sample_result_data[RBLE_RESULT_DATA_BUF_LENGTH]={0};
int rble_smp_reslut_count=0;

static int detect_peak(float new_value,float old_value)
{
    last_status=is_direction_up;
    if(new_value>old_value){
        is_direction_up=1;
        continue_up_count++;
    }else{
        continue_up_former_count=continue_up_count;
        continue_up_count=0;
        is_direction_up=0;
    }

    if((is_direction_up==0)&& (last_status==1)&&(continue_up_former_count >=2 &&(old_value>=min_a_xyz&&
        old_value<=max_a_xyz))){
        peak_wave=old_value;
        return 1;
    }else if((last_status==0) && (is_direction_up==1)){
        valley_wave=old_value;
        return 0;
    }else{
        return 0;
    }
}




static float average_value(float value[],int n)
{
    float ave = 0;
    int i=0;
    for(i=0;i<n;i++){
        ave+=value[i];
    }

    ave=ave/VALUE_NUM;

    if(ave >=8){
        ave=4.3;
    }else if(ave >=7 && ave <8){
        ave=3.3;
    }else if(ave >=4 && ave <7){
        ave=2.3;
    }else if(ave >=3 && ave <4){
        ave=2.0;
    }else{
        ave=1.7;
    }

    return ave;

}

static float peak_valley_thread(float value)
{
    float temp_thread=thread_value;
    if(thread_temp_count <VALUE_NUM){
        thread_temp_value[thread_temp_count]=value;
        thread_temp_count++;
    }else{
        temp_thread=average_value(thread_temp_value,VALUE_NUM);
        int i;
        for(i=1;i<VALUE_NUM;i++){
            thread_temp_value[i-1]=thread_temp_value[i];
        }
        thread_temp_value[VALUE_NUM-1]=value;
    }

    return temp_thread;
}


static void write_to_flash(long step,long run,long dash)
{
    nvms_rble_result_storage_handle=ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
    
    memset(rble_sample_result_data,0,RBLE_RESULT_DATA_BUF_LENGTH);
    rble_smp_reslut_count=0;
    memcpy(rble_sample_result_data,RBLE_RESULT_DATA_LABLE_SC,RBLE_RESULT_DATA_LABLE_LENGTH);
	rble_smp_reslut_count+=RBLE_RESULT_DATA_LABLE_LENGTH;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,&step,RBLE_RESULT_INT_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_INT_SIZE;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,RBLE_RESULT_DATA_LABLE_RC,RBLE_RESULT_DATA_LABLE_LENGTH);
	rble_smp_reslut_count+=RBLE_RESULT_DATA_LABLE_LENGTH;
    
    memcpy(rble_sample_result_data+rble_smp_reslut_count,&run,RBLE_RESULT_INT_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_INT_SIZE;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,RBLE_RESULT_DATA_LABLE_DC,RBLE_RESULT_DATA_LABLE_LENGTH);
	rble_smp_reslut_count+=RBLE_RESULT_DATA_LABLE_LENGTH;
    
    memcpy(rble_sample_result_data+rble_smp_reslut_count,&dash,RBLE_RESULT_INT_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_INT_SIZE;

    ad_nvms_write(nvms_rble_result_storage_handle, rble_result_data_addr_offset, rble_sample_result_data,rble_smp_reslut_count);
    
}


void detect_new_step(float values)
{
    if(a_xyz_old == 0){
        a_xyz_old=values;
    }else{
        if(detect_peak(values,a_xyz_old)){
            time_of_last_peak=time_of_this_peak;
            time_of_now=inv_get_tick_count();

            if((time_of_now - time_of_last_peak)*TICK_TO_MS >=150
                && (peak_wave-valley_wave)>=thread_value
                && (time_of_now-time_of_last_peak)*TICK_TO_MS <= 2000 ){

                time_of_this_peak=time_of_now;
                //step +1;
                current_step++;
                
                //run
                if((time_of_this_peak-time_of_last_peak)*TICK_TO_MS<250){
                    current_dash_count++;
                }else if((time_of_this_peak-time_of_last_peak)*TICK_TO_MS<400){
                    current_run_count++;
                }

                
                printf("wzb current step=%d\r\n",current_step);
                printf("wzb current run step=%d\r\n",current_run_count);
                printf("wzb current dash step=%d\r\n",current_dash_count);

                write_to_flash(current_step,current_run_count,current_dash_count);
            }

            if((time_of_now-time_of_last_peak)*TICK_TO_MS >= 150
                && (peak_wave-valley_wave) >= initial_value){
                time_of_this_peak=time_of_now;
                thread_value=peak_valley_thread(peak_wave-valley_wave);
            }
        }
    }

    a_xyz_old=values;
}


