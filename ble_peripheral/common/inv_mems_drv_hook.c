//#include <Windows.h>
//#include <conio.h>

#include <stdio.h>
#include <string.h>

//#include "driver/inv_mems_hw_config.h"
//#include "invn/common/invn_types.h"

#include "../driver/inv_mems_hw_config.h"
#include "../invn/common/invn_types.h"

//#include "i2c.h"
//#include "board-st_discovery.h"
#include "osal.h"
#include "platform_devices.h"

#include <ad_i2c.h>
#include "../i2c_driver.h"

extern 	i2c_device i2c_gsensor_dev;



#if defined(RBLE_DMA_I2C_INTERFACE)

//zhong


int inv_serial_interface_write_hook(uint16_t reg, uint32_t length, uint8_t *data)
{
	i2c_event_t rble_i2c_event;

    int result=0;
	i2c_device dev;
	int res=0;
	int length_tmp=length+1;
	 uint16_t data_tmp[length_tmp];
	 int i=0;
	 bool result_dma=false;


	data_tmp[0]=reg;
	
	for(i=1;i<length_tmp;i++)
	{
		data_tmp[i]=data[i-1];
	}
	

	result_dma=app_write_i2c(ICM20648_ACC, data_tmp, length_tmp,&rble_i2c_event);


	if(result_dma)
	{
		result=0;
	}
	else
	{
		result=rble_i2c_event.error;
	}

    return result;
}


int inv_serial_interface_read_hook(uint16_t reg, uint32_t length, uint8_t *data)
{
	i2c_event_t rble_i2c_event;
	bool result_dma=false;

	int status=0;
	i2c_device dev;
	int res=0;
	uint8_t reg_tmp=(uint8_t)reg;

	result_dma= app_transact_i2c(ICM20648_ACC, reg_tmp, data,length,&rble_i2c_event, 0);

	if(result_dma)
	{
		status=0;
	}
	else
	{
		status=rble_i2c_event.error;
	}

	return status;
}

#else
int inv_serial_interface_write_hook(uint16_t reg, uint32_t length, uint8_t *data)
{
    int result=0;
	i2c_device dev;
	int res=0;
	int length_tmp=length+1;
	 uint8_t data_tmp[length_tmp];
	 int i=0;

	dev = ad_i2c_open(ICM20648_ACC);
	///dev=i2c_gsensor_dev;

	data_tmp[0]=(uint8_t)reg;
	
	for(i=1;i<length_tmp;i++)
	{
		data_tmp[i]=data[i-1];
	}
	
	result = ad_i2c_write(dev, data_tmp, length_tmp);

    //result = Sensors_I2C_WriteRegister(IVORY_I2C_ADDRESS, (unsigned char)reg, (unsigned short)length, data);
	ad_i2c_close(dev);

    return result;
}


int inv_serial_interface_read_hook(uint16_t reg, uint32_t length, uint8_t *data)
{
	int status=0;
	i2c_device dev;
	int res=0;
	uint8_t reg_tmp=(uint8_t)reg;

	dev = ad_i2c_open(ICM20648_ACC);
	////dev=i2c_gsensor_dev;
	
	//sensor_read_register(dev, 0x00, data, length);
	//status= Sensors_I2C_ReadRegister(IVORY_I2C_ADDRESS, (unsigned char)reg, (unsigned short)length, data);
	status = ad_i2c_transact(dev, &reg_tmp, 1, data, length);

	ad_i2c_close(dev);

	return status;
}

/**
 *  @brief  Sleep function.
**/

void inv_sleep(unsigned long mSecs)
{
	OS_TICK_TIME xNextWakeTime;
	TickType_t tick_tmp=0;

	xNextWakeTime = OS_GET_TICK_COUNT();

	tick_tmp=OS_MS_2_TICKS(mSecs);

	if(0==tick_tmp)
	{
		tick_tmp=1;
	}
	//mdelay(mSecs);
	vTaskDelayUntil( &xNextWakeTime, tick_tmp );

	
}

void inv_sleep_100us(unsigned long nHowMany100MicroSecondsToSleep)
{
    (void)nHowMany100MicroSecondsToSleep;
   // mdelay(1);
   inv_sleep(1);
}

/**
 *  @brief  get system's internal tick count.
 *          Used for time reference.
 *  @return current tick count.
**/
long inv_get_tick_count(void)
{
    unsigned long count;

    //get_tick_count(&count);
	count=(unsigned long)(OS_GET_TICK_COUNT());

    return (long)count;
}

