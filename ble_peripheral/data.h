/*
 * data.h
 *
 *  Created on: 2017Äê2ÔÂ27ÈÕ
 *      Author: Administrator
 */

#ifndef DATA_H_
#define DATA_H_

void detect_new_step(float values);

#define RBLE_RESULT_DATA_PATITION_SIZE  (0x005000)
#define RBLE_RESULT_DATA_BUF_LENGTH    32
#define RBLE_RESULT_FLOAT_SIZE   4
#define RBLE_RESULT_INT_SIZE   4

#define RBLE_RESULT_DATA_LABLE_LENGTH  2
#define RBLE_RESULT_DATA_LABLE_SC "sc" //step count
#define RBLE_RESULT_DATA_LABLE_RC "rc" //run count
#define RBLE_RESULT_DATA_LABLE_DC "dc" //dash count





#endif /* DATA_H_ */
