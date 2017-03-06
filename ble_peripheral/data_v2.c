#if defined(BLE_USE_DATA_V2)

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "data_v2.h"
#include "data_common.h"
#include "ad_nvms.h"

static step_env_t step_env;

nvms_t nvms_rble_result_storage_handle;
uint32_t rble_result_data_addr_offset=0;
uint8_t rble_sample_result_data[RBLE_RESULT_DATA_BUF_LENGTH]={0};
int rble_smp_reslut_count=0;

///initialization parameter
float acc_x_abs_min_normal=28.2f;
float acc_y_abs_min_normal=28.2f;
float acc_x_abs_min_run=37.4f;
float acc_y_abs_min_run=37.4f;


static int detect_peak(float new_value,float old_value)
{
    if(step_env.detect_peak_mode == ACC_PEAK_X2){
        step_env.last_status_is_up.acc_x2=step_env.direction_is_up.acc_x2;
        if(new_value>old_value){
            step_env.direction_is_up.acc_x2=1;
            step_env.continue_up_count.acc_x2++;
        }else{
            step_env.continue_up_former_count.acc_x2=step_env.continue_up_count.acc_x2;
            step_env.continue_up_count.acc_x2=0;
            step_env.direction_is_up.acc_x2=0;
        }
        //printf("wzb detect peak old v=%d,new v=%d\r\n",(int)old_value,(int)new_value);
        if((step_env.direction_is_up.acc_x2==0)&& (step_env.last_status_is_up.acc_x2==1)&&(step_env.continue_up_former_count.acc_x2>=2 &&(old_value>=step_env.min_acc_value))){
            step_env.peak_wave.acc_x2=old_value;
            //printf("wzb detect peak 11\r\n");
            return 1;
        }else if((step_env.last_status_is_up.acc_x2==0) && (step_env.direction_is_up.acc_x2==1)){
            step_env.valley_wave.acc_x2=old_value;
            return 0;
        }else{
            return 0;
        }
    }
    else if(step_env.detect_peak_mode == ACC_PEAK_Y2){
        step_env.last_status_is_up.acc_y2=step_env.direction_is_up.acc_y2;
        if(new_value>old_value){
            step_env.direction_is_up.acc_y2=1;
            step_env.continue_up_count.acc_y2++;
        }else{
            step_env.continue_up_former_count.acc_y2=step_env.continue_up_count.acc_y2;
            step_env.continue_up_count.acc_y2=0;
            step_env.direction_is_up.acc_y2=0;
        }
        //printf("wzb detect peak old v=%d,new v=%d\r\n",(int)old_value,(int)new_value);
        if((step_env.direction_is_up.acc_y2==0)&& (step_env.last_status_is_up.acc_y2==1)&&(step_env.continue_up_former_count.acc_y2>=2 &&(old_value>=step_env.min_acc_value))){
            step_env.peak_wave.acc_y2=old_value;
            //printf("wzb detect peak 11\r\n");
            return 1;
        }else if((step_env.last_status_is_up.acc_y2==0) && (step_env.direction_is_up.acc_y2==1)){
            step_env.valley_wave.acc_y2=old_value;
            return 0;
        }else{
            return 0;
        }

    }
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

static void write_to_flash_v2(long total_step,long total_run,long total_dash,long h_step,long h_run,long h_dash,long total_distance,long h_distance)
{
    nvms_rble_result_storage_handle=ad_nvms_open(NVMS_IMAGE_RESULT_DATA_STORAGE_PART);
    
    memset(rble_sample_result_data,0,RBLE_RESULT_DATA_BUF_LENGTH);
    rble_smp_reslut_count=0;


    memcpy(rble_sample_result_data+rble_smp_reslut_count,&total_step,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;


    memcpy(rble_sample_result_data+rble_smp_reslut_count,&total_run,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;

    
    memcpy(rble_sample_result_data+rble_smp_reslut_count,&total_dash,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,&h_step,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,&h_run,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,&h_dash,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,&total_distance,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;

    memcpy(rble_sample_result_data+rble_smp_reslut_count,&h_distance,RBLE_RESULT_LONG_SIZE);
    rble_smp_reslut_count+=RBLE_RESULT_LONG_SIZE;

    ad_nvms_write(nvms_rble_result_storage_handle, rble_result_data_addr_offset, rble_sample_result_data,rble_smp_reslut_count);
    
}



void detect_new_step_v2(float acc_x2,float acc_y2,float acc_z,unsigned short fifo_id)
{
    if(step_env.acc_value_mode.acc_x2_old== 0){
        step_env.acc_value_mode.acc_x2_old=acc_x2;
    }else{
        step_env.detect_peak_mode=ACC_PEAK_X2;
        step_env.min_acc_value=acc_x_abs_min_normal;
        if(detect_peak(acc_x2,step_env.acc_value_mode.acc_x2_old)){
            step_env.eff_time_of_last_peak=step_env.eff_time_of_this_peak;
            step_env.time_of_now=inv_get_tick_count()-(fifo_id)*FIFO_OFFSET_TICK;

            if((step_env.time_of_now - step_env.eff_time_of_last_peak)*TICK_TO_MS >=400
                && step_env.peak_wave.acc_x2>=step_env.min_acc_value){

                step_env.eff_time_of_this_peak=step_env.time_of_now;
                //step +1;
                //current_step++;
                //step_env.total_step++;
                step_env.flag=1;
                printf("wzb x2 step\r\n");
                step_env.type=HORIZONTAL;
                
                if((step_env.eff_time_of_this_peak-step_env.eff_time_of_last_peak)*TICK_TO_MS <=550){
                    step_env.mode=DASH;
                }else if((step_env.eff_time_of_this_peak-step_env.eff_time_of_last_peak)*TICK_TO_MS <=720){
                    step_env.mode=RUN;
                }else{
                    step_env.mode=WALK;
                }
               
                //write_to_flash(current_step,current_run_count,current_dash_count);
            }
             
        }
     step_env.acc_value_mode.acc_x2_old=acc_x2;  
    }

#if 1
    if(step_env.acc_value_mode.acc_y2_old== 0){
        step_env.acc_value_mode.acc_y2_old=acc_y2;
    }else{
        step_env.detect_peak_mode=ACC_PEAK_Y2;
         step_env.min_acc_value=acc_y_abs_min_normal;
        if(detect_peak(acc_y2,step_env.acc_value_mode.acc_y2_old)){           
            step_env.eff_time_of_last_peak=step_env.eff_time_of_this_peak;
            step_env.time_of_now=inv_get_tick_count()-(fifo_id)*FIFO_OFFSET_TICK;

            if((step_env.time_of_now - step_env.eff_time_of_last_peak)*TICK_TO_MS >=400
                && step_env.peak_wave.acc_y2>=step_env.min_acc_value){

                step_env.eff_time_of_this_peak=step_env.time_of_now;
                printf("wzb y2 step\r\n");
                //step +1;
                //current_step++;
                //step_env.total_step++;
                step_env.flag=1;
                step_env.type=VERTICAL;
                
                if((step_env.eff_time_of_this_peak-step_env.eff_time_of_last_peak)*TICK_TO_MS <=550){
                    step_env.mode=DASH;
                }else if((step_env.eff_time_of_this_peak-step_env.eff_time_of_last_peak)*TICK_TO_MS <=720){
                    step_env.mode=RUN;
                }else{
                    step_env.mode=WALK;
                }           
                  
                //write_to_flash(current_step,current_run_count,current_dash_count);
            }
                
        }
    step_env.acc_value_mode.acc_y2_old=acc_y2;
    }
#endif

    // statistical data
    if(step_env.flag==1){
        //step_env.total_step++;
        step_env.flag=0;
        printf("wzb step=%d\r\n",step_env.total_step);
        
        if(step_env.type == VERTICAL){
            step_env.total_step+=2;
            step_env.frequency=2*1000/(step_env.eff_time_of_this_peak-step_env.eff_time_of_last_peak);
            switch(step_env.mode){
                case WALK:
                    
                    if(step_env.frequency <2){
                        step_env.stride=0.64f; //default slow step length
                    }
                    if(step_env.frequency >=2 && step_env.frequency <2.8){
                        step_env.stride=0.2802*step_env.frequency+0.09376;
                    }else{
                        step_env.stride=0.878f;//default fast step length
                    }                   
                    break;    
               case RUN:
                    step_env.total_run++;
                    if(step_env.frequency<3){
                        step_env.stride=0.75f;
                    }

                    if(step_env.frequency >=3 && step_env.frequency <3.4){
                        step_env.stride=0.9222*step_env.frequency-1.981;
                    }else{
                        step_env.stride=1.15f;
                    }
                    break;
               case DASH:
                    //default:
                    step_env.total_dash++;
                    step_env.stride=1.35f;
                    break;
              default:
                break;
            }

            step_env.distance +=step_env.stride*2;
        }
        else if(step_env.type == HORIZONTAL){
            step_env.total_step++;
            step_env.h_step++;
            step_env.frequency=1000/(step_env.eff_time_of_this_peak-step_env.eff_time_of_last_peak);
            switch(step_env.mode){
                case WALK:
                    if(step_env.frequency <2){
                        step_env.stride=0.64f; //default slow step length
                    }
                    if(step_env.frequency >=2 && step_env.frequency <2.8){
                        step_env.stride=0.2802*step_env.frequency+0.09376;
                    }else{
                        step_env.stride=0.878f;//default fast step length
                    }                   
                    break;    
               case RUN:
                    step_env.total_run++;
                    step_env.h_run++;
                    if(step_env.frequency<3){
                        step_env.stride=0.75f;
                    }

                    if(step_env.frequency >=3 && step_env.frequency <3.4){
                        step_env.stride=0.9222*step_env.frequency-1.981;
                    }else{
                        step_env.stride=1.15f;
                    }
                    break;
               case DASH:
                    //default:
                    step_env.total_dash++;
                    step_env.h_dash++;
                    step_env.stride=1.35f;
                    break;
              default:
                break;
            }

            step_env.distance +=step_env.stride;
            step_env.h_distance +=step_env.stride;
        }
        else if(step_env.type == JUMP){
            step_env.jump++;
        }


        write_to_flash_v2(step_env.total_step,step_env.total_run,step_env.total_dash,step_env.h_step,step_env.h_run,step_env.h_dash,step_env.distance,step_env.h_distance);

    }
   
}


void init_step_env()
{
    step_env.flag=0;
    step_env.frequency=0;
    
    step_env.acc_value_mode.acc_x2_old=0;
    step_env.acc_value_mode.acc_y2_old=0;
    step_env.acc_value_mode.acc_z2_old=0;
    step_env.acc_value_mode.acc_x_old=0;
    step_env.acc_value_mode.acc_y_old=0;
    step_env.acc_value_mode.acc_z_old=0;
    
    step_env.continue_up_count.acc_x=0;
    step_env.continue_up_count.acc_y=0;
    step_env.continue_up_count.acc_z=0;
    step_env.continue_up_count.acc_x2=0;
    step_env.continue_up_count.acc_y2=0;
    step_env.continue_up_count.acc_z2=0;
    step_env.continue_up_former_count.acc_x=0;
    step_env.continue_up_former_count.acc_y=0;
    step_env.continue_up_former_count.acc_z=0;
    step_env.continue_up_former_count.acc_x2=0;
    step_env.continue_up_former_count.acc_y2=0;
    step_env.continue_up_former_count.acc_z2=0;

    step_env.direction_is_up.acc_x=0;
    step_env.direction_is_up.acc_y=0;
    step_env.direction_is_up.acc_z=0;
    step_env.direction_is_up.acc_x2=0;
    step_env.direction_is_up.acc_y2=0;
    step_env.direction_is_up.acc_z2=0;
    
    step_env.last_status_is_up.acc_x=0;
    step_env.last_status_is_up.acc_y=0;
    step_env.last_status_is_up.acc_z=0;
    step_env.last_status_is_up.acc_x2=0;
    step_env.last_status_is_up.acc_y2=0;
    step_env.last_status_is_up.acc_z2=0;

    step_env.type=0xff;
    step_env.mode=0xff;

    step_env.detect_peak_mode=0xff;

    step_env.distance=0;
    step_env.stride=0;
    step_env.h_distance=0;

    step_env.peak_wave.acc_x=0;
    step_env.peak_wave.acc_y=0;
    step_env.peak_wave.acc_z=0;
    step_env.peak_wave.acc_x2=0;
    step_env.peak_wave.acc_y2=0;
    step_env.peak_wave.acc_z2=0;
    step_env.valley_wave.acc_x=0;
    step_env.valley_wave.acc_y=0;
    step_env.valley_wave.acc_z=0;
    step_env.valley_wave.acc_x2=0;
    step_env.valley_wave.acc_y2=0;
    step_env.valley_wave.acc_z2=0;
    

    step_env.total_step=0;
    step_env.total_run=0;
    step_env.total_dash=0;

    step_env.h_step=0;
    step_env.h_run=0;
    step_env.h_dash=0;

    step_env.jump=0;

    step_env.min_acc_value=15.0f;
    step_env.max_acc_value=2000.0f;

    step_env.eff_time_of_last_peak=0;
    step_env.eff_time_of_this_peak=0;
    step_env.time_of_now=0;
    
    
}

#endif

