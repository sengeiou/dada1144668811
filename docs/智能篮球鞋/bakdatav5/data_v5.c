#if 1
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "data_v5.h"

#define WEIBIN_PARAMETER

static void reset_jump_state();
static void reset_x_step_env();
static void reset_y_step_env();

static void enable_temp_step();
static void enable_x_step();
static void enable_y_step();
static void copy_x_step_to_temp();
static void copy_y_step_to_temp();
static void clear_temp_step();

static void enable_jump();
static void enable_temp_jump();
static void clear_temp_jump();
static void copy_jump_to_temp();
static void copy_jump_to_temp_step();
//static step_env_t step_env;

float jump_time_height[100][2] = { 0 };
int jump_row = 0;
int jump_col = 0;

#define FILTER
///initialization parameter
#ifdef WEIBIN_PARAMETER

#ifdef FILTER
float acc_x_abs_min_normal = 5.50f;//25.2f;
float acc_y_abs_min_normal = 5.5f;//15.0f;//6.5f;//15.0f;//25.2f;
float acc_x_abs_min_run = 15.0f;
float acc_y_abs_min_run = 15.0f;
float acc_x_abs_min_dash = 28.0f;
float acc_y_abs_min_dash = 28.0f;
float acc_z_abs_min_jump = 40.0f;//15.0f;//40.0f;
int acc_y_min_interval_step = 500;//ms

int acc_y_min_interval_normal = 1260;//ms
int acc_y_min_interval_run = 720;//ms
int acc_y_min_interval_dash = 600;//ms

int acc_x_min_interval_normal = 1260;//ms
int acc_x_min_interval_run = 850;//ms
int acc_x_min_interval_dash = 600;//ms
#else
float acc_x_abs_min_normal = 15.0f;//25.2f;
float acc_y_abs_min_normal = 12.5f;//15.0f;//6.5f;//15.0f;//25.2f;
float acc_x_abs_min_run = 30.4f;
float acc_y_abs_min_run = 30.4f;
float acc_x_abs_min_dash = 49.0f;
float acc_y_abs_min_dash = 49.0f;
float acc_z_abs_min_jump = 40.0f;//15.0f;//40.0f;
int acc_y_min_interval_step = 500;//ms

int acc_y_min_interval_normal = 1260;//ms
int acc_y_min_interval_run = 720;//ms
int acc_y_min_interval_dash = 600;//ms

int acc_x_min_interval_normal = 1260;//ms
int acc_x_min_interval_run = 850;//ms
int acc_x_min_interval_dash = 600;//ms
#endif
#else
float acc_x_abs_min_normal = 15.0f;//28.2f;
float acc_y_abs_min_normal = 15.0f;//28.2f;
float acc_x_abs_min_run = 37.4f;
float acc_y_abs_min_run = 37.4f;
float acc_z_abs_min_jump = 40.0f;//15.0f;//40.0f;
#endif


static int detect_peak(float new_value, float old_value)
{
	if (step_env.detect_peak_mode == ACC_PEAK_X2) {
		step_env.last_status_is_up.acc_x2 = step_env.direction_is_up.acc_x2;
		if (new_value > old_value) {
			step_env.direction_is_up.acc_x2 = 1;
			step_env.continue_up_count.acc_x2++;
		}
		else {
			step_env.continue_up_former_count.acc_x2 = step_env.continue_up_count.acc_x2;
			step_env.continue_up_count.acc_x2 = 0;
			step_env.direction_is_up.acc_x2 = 0;
		}
		//printf("wzb detect peak old v=%d,new v=%d\r\n",(int)old_value,(int)new_value);
		if ((step_env.direction_is_up.acc_x2 == 0) && (step_env.last_status_is_up.acc_x2 == 1) && (step_env.continue_up_former_count.acc_x2 >= 1/*2*/ && (old_value >= step_env.min_acc_value))) {
			step_env.peak_wave.acc_x2 = old_value;
			//printf("wzb detect peak 11\r\n");
			return 1;
		}
		else if ((step_env.last_status_is_up.acc_x2 == 0) && (step_env.direction_is_up.acc_x2 == 1)) {
			step_env.valley_wave.acc_x2 = old_value;
			return 0;
		}
		else {
			return 0;
		}
	}
	else if (step_env.detect_peak_mode == ACC_PEAK_Y2) {
		step_env.last_status_is_up.acc_y2 = step_env.direction_is_up.acc_y2;
		if (new_value > old_value) {
			step_env.direction_is_up.acc_y2 = 1;
			step_env.continue_up_count.acc_y2++;
		}
		else {
			step_env.continue_up_former_count.acc_y2 = step_env.continue_up_count.acc_y2;
			step_env.continue_up_count.acc_y2 = 0;
			step_env.direction_is_up.acc_y2 = 0;
		}
		//printf("wzb detect peak y old v=%f,new v=%f\r\n",old_value,new_value);
		if ((step_env.direction_is_up.acc_y2 == 0) && (step_env.last_status_is_up.acc_y2 == 1) && (step_env.continue_up_former_count.acc_y2 >= 1/*2*/ && (old_value >= step_env.min_acc_value))) {
			step_env.peak_wave.acc_y2 = old_value;
			// printf("wzb detect peak y 11\r\n");
			return 1;
		}
		else if ((step_env.last_status_is_up.acc_y2 == 0) && (step_env.direction_is_up.acc_y2 == 1)) {
			step_env.valley_wave.acc_y2 = old_value;
			return 0;
		}
		else {
			return 0;
		}

	}
	else if (step_env.detect_peak_mode == ACC_PEAK_Z) {
		step_env.last_status_is_up.acc_z = step_env.direction_is_up.acc_z;
		if (new_value > old_value) {
			step_env.direction_is_up.acc_z = 1;
			step_env.continue_up_count.acc_z++;
		}
		else {
			step_env.continue_up_former_count.acc_z = step_env.continue_up_count.acc_z;
			step_env.continue_up_count.acc_z = 0;
			step_env.direction_is_up.acc_z = 0;
		}
		//printf("wzb detect peak z old v=%d,new v=%d\r\n", (int)old_value, (int)new_value);
		if ((step_env.direction_is_up.acc_z == 0) && (step_env.last_status_is_up.acc_z == 1) && (step_env.continue_up_former_count.acc_z >= 1/*2*/ && (old_value >= step_env.min_acc_value))) {
			step_env.peak_wave.acc_z = old_value;
			//printf("wzb detect peak z 11\r\n");
			return 1;
		}
		else if ((step_env.last_status_is_up.acc_z == 0) && (step_env.direction_is_up.acc_z == 1)) {
			step_env.valley_wave.acc_z = old_value;
			return 0;
		}
		else {
			return 0;
		}

	}
}

static long inv_get_tick_count(int line)
{
	long tick = line*LINE_TO_TICK;
	//printf("line=%d,tick=%d\n", line, tick);
	return tick;
}

static float data_abs(float value)
{
	return value > 0 ? value : -value;
}


void detect_new_step_v2(float acc_x, float acc_y, float acc_z, int line)
{

	float acc_x2, acc_y2;
	acc_x2 = data_abs(acc_x);
	acc_y2 = data_abs(acc_y);
	step_env.time_of_now = inv_get_tick_count(line);
	if (((step_env.time_of_now - step_env.temp_step.time) >= 450)) {
		if (step_env.temp_step.flag == 1) {
			printf("wzb time interval >>>step   line=%d\n", line);
			enable_temp_step();
			clear_temp_step();
		}
	}
	if (step_env.acc_value_mode.acc_x2_old == 0) {
		step_env.acc_value_mode.acc_x2_old = acc_x2;
	}
	else {
		step_env.detect_peak_mode = ACC_PEAK_X2;
		step_env.min_acc_value = acc_x_abs_min_normal;
		if (detect_peak(acc_x2, step_env.acc_value_mode.acc_x2_old)) {

			if (step_env.time_of_now - MAX(step_env.last_step.time, get_temp_step_time(HORIZONTAL)) > 110) {
				if (step_env.x_step.flag == 0) {
					step_env.x_step.flag = 1;
					printf("x flag=1,line=%d\n", line);
					step_env.x_step.fir_peak_time = step_env.time_of_now;
					step_env.x_step.fir_peak_value = step_env.acc_value_mode.acc_x_old;
				}
				else if (step_env.x_step.flag == 1) {
					if (((step_env.time_of_now - step_env.x_step.fir_peak_time) > 60) && ((step_env.time_of_now - step_env.x_step.fir_peak_time) < 420)) {
						printf("x_step.fir_peak_value=%f,now acc_x=%f\n", step_env.x_step.fir_peak_value, step_env.acc_value_mode.acc_x_old);
						if (step_env.x_step.fir_peak_value < 0 && step_env.acc_value_mode.acc_x_old>0) {
							step_env.x_step.flag = 2;
							step_env.x_step.sec_peak_time = step_env.time_of_now;
							step_env.x_step.sec_peak_value = step_env.acc_value_mode.acc_x_old;
							step_env.x_step.ori = LEFT;
							printf("x flag=2,line=%d\n", line);
						}
						if (step_env.x_step.fir_peak_value > 0 && step_env.acc_value_mode.acc_x_old < 0) {
							step_env.x_step.flag = 2;
							step_env.x_step.sec_peak_time = step_env.time_of_now;
							step_env.x_step.sec_peak_value = step_env.acc_value_mode.acc_x_old;
							step_env.x_step.ori = RIGHT;
							printf("x flag=2,line=%d\n", line);
						}

					}
					else if ((step_env.time_of_now - step_env.x_step.fir_peak_time) >= 420) {
						reset_x_step_env();
					}
				}
				if (line == 476) {
					printf("x 476\n");
				}
				if (step_env.x_step.flag == 2) {
					step_env.x_step.value = (data_abs(step_env.x_step.fir_peak_value) + data_abs(step_env.x_step.sec_peak_value)) / 2;
					long step_div = (step_env.x_step.sec_peak_time - MAX(step_env.last_step.time, step_env.temp_step.time))*TICK_TO_MS;
					if (step_div<acc_x_min_interval_dash && step_env.x_step.value>acc_x_abs_min_dash) {
						step_env.x_step.mode = DASH;
					}
					else if (step_div<acc_x_min_interval_run && step_env.x_step.value>acc_x_abs_min_run) {
						step_env.x_step.mode = RUN;
					}
					else {
						step_env.x_step.mode = WALK;
					}

					printf("xx temp flag=%d,temp type=%d\n", step_env.temp_step.flag, step_env.temp_step.type);
					if (step_env.temp_step.flag == 0) {
						copy_x_step_to_temp();
						reset_x_step_env();
					}
					else if (step_env.temp_step.flag == 1) {
						if (step_env.temp_step.type == HORIZONTAL) {
							enable_temp_step();
							copy_x_step_to_temp();
							reset_x_step_env();
						}
						else if (step_env.temp_step.type == VERTICAL) {
							if (step_env.x_step.sec_peak_time - step_env.temp_step.time < 190) {
								if (step_env.temp_step.value < step_env.x_step.value) {
									//enable_x_step();
									//clear_temp_step();
									copy_x_step_to_temp();
									reset_x_step_env();
								}
								else {
									//enable_temp_step();
									//clear_temp_step();
									reset_x_step_env();
								}
							}
							else {
								enable_temp_step();
								copy_x_step_to_temp();
								reset_x_step_env();
							}
						}
						else if (step_env.temp_step.type == JUMP) {
							if (step_env.x_step.sec_peak_time - step_env.temp_step.time < 190) {
								if (step_env.temp_step.value < step_env.x_step.value) {
									copy_x_step_to_temp();
									reset_x_step_env();
								}
								else {
									reset_x_step_env();
								}
							}
							else {
								enable_temp_step();
								copy_x_step_to_temp();
								reset_x_step_env();
							}
						}
					}
				}
			}
		}
		step_env.acc_value_mode.acc_x2_old = acc_x2;
		step_env.acc_value_mode.acc_x_old = acc_x;
	}


	if (step_env.acc_value_mode.acc_y2_old == 0) {
		step_env.acc_value_mode.acc_y2_old = acc_y2;
	}
	else {
		step_env.detect_peak_mode = ACC_PEAK_Y2;
		step_env.min_acc_value = acc_y_abs_min_normal;
		if (detect_peak(acc_y2, step_env.acc_value_mode.acc_y2_old)) {
			//step_env.time_of_now = inv_get_tick_count(line);
			if (step_env.time_of_now - MAX(step_env.last_step.time, get_temp_step_time(VERTICAL)) > 110) {
				if (step_env.y_step.flag == 0) {
					step_env.y_step.flag = 1;
					step_env.y_step.fir_peak_time = step_env.time_of_now;
					step_env.y_step.fir_peak_value = step_env.acc_value_mode.acc_y_old;
					printf("y flag=1 line=%d\n", line);
				}
				else if (step_env.y_step.flag == 1) {
					if (((step_env.time_of_now - step_env.y_step.fir_peak_time) > 60) && ((step_env.time_of_now - step_env.y_step.fir_peak_time) < 500)) {
						printf("y_step.fir_peak_value=%f,now acc_y=%f\n", step_env.y_step.fir_peak_value, step_env.acc_value_mode.acc_y_old);
						if (step_env.y_step.fir_peak_value < 0 && step_env.acc_value_mode.acc_y_old>0) {
							if ((step_env.time_of_now - step_env.y_step.fir_peak_time) >= (step_env.last_step.sec_peak_time - step_env.last_step.fir_peak_time)) {
								printf("reset y ,line=%d\n", line);
								//reset_y_step_env();
								step_env.y_step.flag == 1;
								step_env.y_step.fir_peak_time = step_env.time_of_now;
								step_env.y_step.fir_peak_value = step_env.acc_value_mode.acc_y_old;
								printf("reset y flag=1 line=%d\n", line);
							}
							else {
								step_env.y_step.flag = 2;
								step_env.y_step.sec_peak_time = step_env.time_of_now;
								step_env.y_step.sec_peak_value = step_env.acc_value_mode.acc_y_old;
								step_env.y_step.ori = BACKWARD;
								printf("y flag=2 line=%d\n", line);
							}
						}
						if (step_env.y_step.fir_peak_value > 0 && step_env.acc_value_mode.acc_y_old < 0) {
							step_env.y_step.flag = 2;
							step_env.y_step.sec_peak_time = step_env.time_of_now;
							step_env.y_step.sec_peak_value = step_env.acc_value_mode.acc_y_old;
							step_env.y_step.ori = FORWARD;
							printf("y flag=2 line=%d\n", line);
						}

					}
					else if ((step_env.time_of_now - step_env.y_step.fir_peak_time) >= 500) {
						if (step_env.acc_value_mode.acc_y_old > 0) {
							step_env.y_step.flag = 1;
							step_env.y_step.fir_peak_time = step_env.time_of_now;
							step_env.y_step.fir_peak_value = step_env.acc_value_mode.acc_y_old;
						}
						else {
							reset_y_step_env();
						}
					}
				}

				if (step_env.y_step.flag == 2) {
					step_env.y_step.value = (data_abs(step_env.y_step.fir_peak_value) + data_abs(step_env.y_step.sec_peak_value)) / 2;
					long step_div = (step_env.y_step.sec_peak_time - MAX(step_env.last_step.time, step_env.temp_step.time))*TICK_TO_MS;
					if (step_div<acc_y_min_interval_dash && step_env.y_step.value>acc_y_abs_min_dash) {
						step_env.y_step.mode = DASH;
					}
					else if (step_div<acc_y_min_interval_run && step_env.y_step.value>acc_y_abs_min_run) {
						step_env.y_step.mode = RUN;
					}
					else {
						step_env.y_step.mode = WALK;
					}
					printf("yy temp flag=%d\n", step_env.temp_step.flag);
					if (step_env.temp_step.flag == 0) {
						copy_y_step_to_temp();
						reset_y_step_env();
					}
					else if (step_env.temp_step.flag == 1) {
						printf("y temp type=%d\n", step_env.temp_step.type);
						if (step_env.temp_step.type == VERTICAL) {
							enable_temp_step();
							copy_y_step_to_temp();
							reset_y_step_env();
						}
						else if (step_env.temp_step.type == HORIZONTAL) {
							printf("y step_env.y_step.sec_peak_time - step_env.temp_step.time=%d\n", step_env.y_step.sec_peak_time - step_env.temp_step.time);
							printf("y temp_step.value=%f,y_step.value=%f\n", step_env.temp_step.value, step_env.y_step.value);
							if (step_env.y_step.sec_peak_time - step_env.temp_step.time < 190) {
								if (step_env.temp_step.value <= step_env.y_step.value) {
									//clear_temp_step();
									//enable_y_step();
									copy_y_step_to_temp();
									reset_y_step_env();
								}
								else {
									//enable_temp_step();
									//clear_temp_step();
									reset_y_step_env();
								}
							}
							else {
								enable_temp_step();
								copy_y_step_to_temp();
								reset_y_step_env();
							}
						}
						else if (step_env.temp_step.type == JUMP) {
							if (step_env.y_step.sec_peak_time - step_env.temp_step.time < 190) {
								if (step_env.temp_step.value <= step_env.y_step.value) {
									copy_y_step_to_temp();
									reset_y_step_env();
								}
								else {
									reset_y_step_env();
								}
							}
							else {
								enable_temp_step();
								copy_y_step_to_temp();
								reset_y_step_env();
							}
						}
					}
				}
			}
		}
		step_env.acc_value_mode.acc_y2_old = acc_y2;
		step_env.acc_value_mode.acc_y_old = acc_y;
	}



	if (step_env.acc_value_mode.acc_z_old == 0) {
		step_env.acc_value_mode.acc_z_old = acc_z;
	}
	else {
		step_env.detect_peak_mode = ACC_PEAK_Z;
		step_env.min_acc_value = acc_z_abs_min_jump;
		if (detect_peak(acc_z, step_env.acc_value_mode.acc_z_old)) {
			//step_env.time_of_now = inv_get_tick_count(line);		
			if (step_env.time_of_now - MAX(step_env.last_step.time, get_temp_step_time(JUMP)) > 110) {
				if (step_env.jump.flag == 0) {
					step_env.jump.flag = 1;
					step_env.jump.fir_peak_time = step_env.time_of_now;
					step_env.jump.fir_peak_value = step_env.acc_value_mode.acc_z_old;
					printf("z jump.flag=1; line=%d\n", line);
				}
				else if (step_env.jump.flag == 1) {
					if (((step_env.time_of_now - step_env.jump.fir_peak_time) > 180) && ((step_env.time_of_now - step_env.jump.fir_peak_time) < 450)) {
						step_env.jump.flag = 2;
						step_env.jump.sec_peak_time = step_env.time_of_now;
						step_env.jump.sec_peak_value = step_env.acc_value_mode.acc_z_old;
						printf("z jump.flag=2;line=%d\n", line);
					}
					else if ((step_env.time_of_now - step_env.jump.fir_peak_time) >= 450) {
						step_env.jump.flag = 1;
						step_env.jump.fir_peak_time = step_env.time_of_now;
						step_env.jump.fir_peak_value = step_env.acc_value_mode.acc_z_old;
					}
				}


				if (step_env.jump.flag == 2) {
					step_env.jump.value = (step_env.jump.fir_peak_value + step_env.jump.sec_peak_value) / 2 -25;
					step_env.jump.time = step_env.jump.sec_peak_time = step_env.time_of_now;
					step_env.jump.air_time = (step_env.jump.sec_peak_time - step_env.jump.fir_peak_time - 20)*TICK_TO_MS;

					if (step_env.temp_step.flag == 0) {
						copy_jump_to_temp_step();
						reset_jump_state();
					}
					else if (step_env.temp_step.flag == 1) {
						if (step_env.temp_step.type == JUMP) {
							enable_temp_step();
							copy_jump_to_temp_step();
							reset_jump_state();
						}
						else if (step_env.temp_step.type == HORIZONTAL || step_env.temp_step.type == VERTICAL) {
							if (step_env.jump.sec_peak_time - step_env.temp_step.time < 190) {

								if (step_env.jump.value > step_env.temp_step.value) {
									copy_jump_to_temp_step();
									reset_jump_state();
								}
								else {
									reset_jump_state();
								}
							}
							else {
								enable_temp_step();
								copy_jump_to_temp_step();
								reset_jump_state();
							}
						}
					}

				}
			}
		}
		step_env.acc_value_mode.acc_z_old = acc_z;
	}



	// statistical data
	if (step_env.flag == 1) {
		//step_env.total_step++;
		step_env.flag = 0;
		printf("wzb step/jump detect ok,line=%d,type=%d,step time=%d\n", line, step_env.type, step_env.time);

		if (step_env.type == VERTICAL) {
			step_env.total_step += 2;
			if ((step_env.time - step_env.last_step.time) == 0) {
				step_env.frequency = 1;
			}
			else {
				step_env.frequency = 2 * 1000 / (step_env.time - step_env.last_step.time);
			}
			switch (step_env.mode) {
			case WALK:
#ifdef WEIBIN_PARAMETER
				step_env.stride = 0.714f;
#else
				if (step_env.frequency < 2) {
					step_env.stride = 0.64f; //default slow step length
				}
				if (step_env.frequency >= 2 && step_env.frequency < 2.8) {
					step_env.stride = 0.2802*step_env.frequency + 0.09376;
				}
				else {
					step_env.stride = 0.878f;//default fast step length
				}
#endif
				break;
			case RUN:
				step_env.total_run += 2;
#ifdef WEIBIN_PARAMETER
				step_env.stride = 0.882f;
#else
				if (step_env.frequency < 3) {
					step_env.stride = 0.75f;
				}

				if (step_env.frequency >= 3 && step_env.frequency < 3.4) {
					step_env.stride = 0.9222*step_env.frequency - 1.981;
				}
				else {
					step_env.stride = 1.15f;
				}
#endif
				break;
			case DASH:
				//default:
				step_env.total_dash += 2;
				step_env.stride = 1.35f;
				break;
			default:
				break;
			}

			step_env.distance += step_env.stride * 2;
		}
		else if (step_env.type == HORIZONTAL) {
			step_env.total_step++;
			step_env.h_step++;
			if ((step_env.time - step_env.last_step.time) == 0) {
				step_env.frequency = 1;
			}
			else {
				step_env.frequency = 1000 / (step_env.time - step_env.last_step.time);
			}
			switch (step_env.mode) {
			case WALK:
#ifdef WEIBIN_PARAMETER
				step_env.stride = 0.882f;
#else
				if (step_env.frequency < 2) {
					step_env.stride = 0.64f; //default slow step length
				}
				if (step_env.frequency >= 2 && step_env.frequency < 2.8) {
					step_env.stride = 0.2802*step_env.frequency + 0.09376;
				}
				else {
					step_env.stride = 0.878f;//default fast step length
				}
#endif
				break;
			case RUN:
				step_env.total_run++;
				step_env.h_run++;
#ifdef WEIBIN_PARAMETER
				step_env.stride = 0.91f;
#else
				if (step_env.frequency < 3) {
					step_env.stride = 0.75f;
				}

				if (step_env.frequency >= 3 && step_env.frequency < 3.4) {
					step_env.stride = 0.9222*step_env.frequency - 1.981;
				}
				else {
					step_env.stride = 1.15f;
				}
#endif
				break;
			case DASH:
				//default:
				step_env.total_dash++;
				step_env.h_dash++;
				step_env.stride = 1.10f;
				break;
			default:
				break;
			}

			step_env.distance += step_env.stride;
			step_env.h_distance += step_env.stride;
		}
		else if (step_env.type == JUMP) {

			step_env.jump_count++;
			printf("step_env jump count++\n");
			step_env.jump_height = DATA_JUMP_HEIGHT(step_env.jump_air_time / 1000 / 2);

			jump_time_height[jump_row][0] = step_env.jump_air_time;
			jump_time_height[jump_row][1] = step_env.jump_height;
			jump_row++;

		}

		printf("wzb total step=%d,total run=%d,total_dash=%d\n", step_env.total_step, step_env.total_run, step_env.total_dash);
		printf("wzb h step=%d,h run=%d,h dash=%d\n", step_env.h_step, step_env.h_run, step_env.h_dash);
		printf("wzb jump=%d\n", step_env.jump_count);

		step_env.last_step.type = step_env.type;
		step_env.last_step.mode = step_env.mode;
		step_env.last_step.time = step_env.time;
		step_env.last_step.length = step_env.stride;
		step_env.last_step.ori = step_env.ori;



	}



}

void print_result()
{
	printf("###################################################\n");
	printf("total step=%d\n", step_env.total_step);
	printf("total run=%d\n", step_env.total_run);
	printf("total dash=%d\n", step_env.total_dash);
	printf("h step=%d\n", step_env.h_step);
	printf("h run=%d\n", step_env.h_run);
	printf("h dash=%d\n", step_env.h_dash);
	printf("jump count=%d\n", step_env.jump_count);
	printf("total distance=%f\n", step_env.distance);
	printf("h distance=%f\n", step_env.h_distance);
	printf("###################################################\n");
	for (int i = 0; i < 20; i++) {
		printf("jump time=%f,height=%f\n", jump_time_height[i][0], jump_time_height[i][1]);
	}
}


static long get_temp_step_time(uint8_t type)
{
	if (step_env.temp_step.type == type) {
		return step_env.temp_step.time;
	}

	return 0;
}

static void enable_temp_step()
{
	printf("enable_temp_step,type=%d \n", step_env.temp_step.type);
	step_env.flag = 1;
	step_env.mode = step_env.temp_step.mode;
	step_env.type = step_env.temp_step.type;
	step_env.ori = step_env.temp_step.ori;
	step_env.time = step_env.temp_step.time;
	step_env.value = step_env.temp_step.value;
	step_env.jump_air_time = step_env.temp_step.air_time;
}

static void enable_y_step()
{
	step_env.flag = 1;
	step_env.mode = step_env.y_step.mode;
	step_env.type = VERTICAL;
	step_env.ori = step_env.y_step.ori;
	step_env.time = step_env.y_step.sec_peak_time;
	step_env.value = step_env.y_step.value;
}

static void enable_x_step()
{
	step_env.flag = 1;
	step_env.mode = step_env.x_step.mode;
	step_env.type = HORIZONTAL;
	step_env.ori = step_env.x_step.ori;
	step_env.time = step_env.x_step.sec_peak_time;
	step_env.value = step_env.x_step.value;
}

static void copy_jump_to_temp()
{
	step_env.temp_jump.air_time = step_env.jump.air_time;
	step_env.temp_jump.value = step_env.jump.value;
	step_env.temp_jump.time = step_env.jump.time;
	step_env.temp_jump.fir_peak_time = step_env.jump.fir_peak_time;
	step_env.temp_jump.sec_peak_time = step_env.jump.sec_peak_time;
	step_env.temp_jump.flag = 1;
}

static void copy_jump_to_temp_step()
{
	printf("copy_jump_to_temp_step \n");
	step_env.temp_step.air_time = step_env.jump.air_time;
	step_env.temp_step.value = step_env.jump.value;
	step_env.temp_step.time = step_env.jump.time;
	step_env.temp_step.type = JUMP;
	step_env.temp_step.flag = 1;
}

static void enable_jump()
{
	step_env.flag = 1;
	step_env.type = JUMP;
	step_env.jump_air_time = step_env.jump.air_time;
	step_env.jump_time = step_env.jump.time;
}

static void enable_temp_jump()
{
	step_env.flag = 1;
	step_env.type = JUMP;
	step_env.jump_air_time = step_env.temp_jump.air_time;
	step_env.jump_time = step_env.temp_jump.time;
}

static void clear_temp_jump()
{
	step_env.temp_jump.flag = 0;
}

static void copy_y_step_to_temp()
{
	step_env.temp_step.mode = step_env.y_step.mode;
	step_env.temp_step.ori = step_env.y_step.ori;
	step_env.temp_step.fir_peak_time = step_env.y_step.fir_peak_time;
	step_env.temp_step.sec_peak_time = step_env.temp_step.time = step_env.y_step.sec_peak_time;
	step_env.temp_step.type = VERTICAL;
	step_env.temp_step.value = step_env.y_step.value;
	step_env.temp_step.flag = 1;
}

static void copy_x_step_to_temp()
{
	step_env.temp_step.mode = step_env.x_step.mode;
	step_env.temp_step.ori = step_env.x_step.ori;
	step_env.temp_step.fir_peak_time = step_env.x_step.fir_peak_time;
	step_env.temp_step.sec_peak_time = step_env.temp_step.time = step_env.x_step.sec_peak_time;
	step_env.temp_step.type = HORIZONTAL;
	step_env.temp_step.value = step_env.x_step.value;
	step_env.temp_step.flag = 1;
}

static void clear_temp_step()
{
	step_env.temp_step.flag = 0;
}

static void reset_x_step_env()
{
	//step_env.x_step.fir_peak_time = 0;
	//step_env.x_step.sec_peak_time = 0;
	step_env.x_step.flag = 0;
}

static void reset_y_step_env()
{
	//step_env.x_step.fir_peak_time = 0;
	//step_env.x_step.sec_peak_time = 0;
	step_env.y_step.flag = 0;
}

static void reset_jump_state()
{
	step_env.jump.flag = 0;
	//step_env.jump.sec_peak_time = 0;
	//step_env.jump.fir_peak_time = 0;
}

void init_step_env()
{
	step_env.flag = 0;
	step_env.frequency = 0;

	step_env.acc_value_mode.acc_x2_old = 0;
	step_env.acc_value_mode.acc_y2_old = 0;
	step_env.acc_value_mode.acc_z2_old = 0;
	step_env.acc_value_mode.acc_x_old = 0;
	step_env.acc_value_mode.acc_y_old = 0;
	step_env.acc_value_mode.acc_z_old = 0;

	step_env.continue_up_count.acc_x = 0;
	step_env.continue_up_count.acc_y = 0;
	step_env.continue_up_count.acc_z = 0;
	step_env.continue_up_count.acc_x2 = 0;
	step_env.continue_up_count.acc_y2 = 0;
	step_env.continue_up_count.acc_z2 = 0;
	step_env.continue_up_former_count.acc_x = 0;
	step_env.continue_up_former_count.acc_y = 0;
	step_env.continue_up_former_count.acc_z = 0;
	step_env.continue_up_former_count.acc_x2 = 0;
	step_env.continue_up_former_count.acc_y2 = 0;
	step_env.continue_up_former_count.acc_z2 = 0;

	step_env.direction_is_up.acc_x = 0;
	step_env.direction_is_up.acc_y = 0;
	step_env.direction_is_up.acc_z = 0;
	step_env.direction_is_up.acc_x2 = 0;
	step_env.direction_is_up.acc_y2 = 0;
	step_env.direction_is_up.acc_z2 = 0;

	step_env.last_status_is_up.acc_x = 0;
	step_env.last_status_is_up.acc_y = 0;
	step_env.last_status_is_up.acc_z = 0;
	step_env.last_status_is_up.acc_x2 = 0;
	step_env.last_status_is_up.acc_y2 = 0;
	step_env.last_status_is_up.acc_z2 = 0;

	step_env.type = 0xff;
	step_env.mode = 0xff;

	step_env.detect_peak_mode = 0xff;

	step_env.distance = 0;
	step_env.stride = 0;
	step_env.h_distance = 0;

	step_env.peak_wave.acc_x = 0;
	step_env.peak_wave.acc_y = 0;
	step_env.peak_wave.acc_z = 0;
	step_env.peak_wave.acc_x2 = 0;
	step_env.peak_wave.acc_y2 = 0;
	step_env.peak_wave.acc_z2 = 0;
	step_env.valley_wave.acc_x = 0;
	step_env.valley_wave.acc_y = 0;
	step_env.valley_wave.acc_z = 0;
	step_env.valley_wave.acc_x2 = 0;
	step_env.valley_wave.acc_y2 = 0;
	step_env.valley_wave.acc_z2 = 0;


	step_env.total_step = 0;
	step_env.total_run = 0;
	step_env.total_dash = 0;

	step_env.h_step = 0;
	step_env.h_run = 0;
	step_env.h_dash = 0;

	step_env.jump.count = 0;
	step_env.jump.fir_peak_time = 0;
	step_env.jump.sec_peak_time = 0;
	step_env.jump.flag = 0;
	step_env.jump.height = 0;
	step_env.jump.time = 0;

	step_env.x_step.fir_peak_time = 0;
	step_env.x_step.sec_peak_time = 0;
	step_env.x_step.flag = 0;
	step_env.x_step.ori = 0;
	step_env.x_step.fir_peak_value = 0;
	step_env.x_step.sec_peak_value = 0;


	step_env.y_step.fir_peak_time = 0;
	step_env.y_step.sec_peak_time = 0;
	step_env.y_step.flag = 0;
	step_env.y_step.ori = 0;
	step_env.y_step.fir_peak_value = 0;
	step_env.y_step.sec_peak_value = 0;



	step_env.min_acc_value = 15.0f;
	step_env.max_acc_value = 2000.0f;


	step_env.time_of_now = 0;

	step_env.last_step.length = 0;
	step_env.last_step.mode = 0xff;
	step_env.last_step.type = 0xff;
	step_env.last_step.time = 0;
	step_env.last_step.ori = 0;

}


#endif