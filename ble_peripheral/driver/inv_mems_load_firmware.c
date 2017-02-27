/*
* ________________________________________________________________________________________________________
* Copyright � 2014-2015 InvenSense Inc. Portions Copyright � 2014-2015 Movea. All rights reserved.
* This software, related documentation and any modifications thereto (collectively 揝oftware�) is subject
* to InvenSense and its licensors' intellectual property rights under U.S. and international copyright and
* other intellectual property rights laws.
* InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
* and any use, reproduction, disclosure or distribution of the Software without an express license
* agreement from InvenSense is strictly prohibited.
* ________________________________________________________________________________________________________
*/

//#include "driver/inv_mems_load_firmware.h"
//#include "driver/inv_mems_hw_config.h"
//#include "driver/inv_mems_defines.h"

#include "inv_mems_load_firmware.h"
#include "inv_mems_hw_config.h"
#include "inv_mems_defines.h"


#ifndef MEMS_20609
//#include "driver/inv_mems_base_driver.h"
#include "inv_mems_base_driver.h"

#else
#include "driver/inv_mems_base_driver_20609.h"
#endif
//#include "driver/inv_mems_transport.h"
//#include "dmp3/inv_mems_interface_mapping.h"

//#include "invn/common/invn_types.h"


#include "inv_mems_transport.h"

//test_dmp3a
#include "../dmp3a/inv_mems_interface_mapping.h"

#include "../invn/common/invn_types.h"


#if defined(RBLE_DMP_REPEAT_WRITE)

//test_r

#define RBLE_DMP_WRITE_TIMES  5
//3   7   300

#define RBLE_DMP_READ_TIMES   2
#define RBLE_DMP_UNCMP_TIMES   3
#define RBLE_DMP_SART_ADDR_RESET_TIMES   5
//3  300


#define RBLE_DMP_LOAD_START      0x90


//test_r
inv_error_t inv_mems_firmware_load(const unsigned char *data_start, unsigned short size_start, unsigned short load_addr)
{ 
    int write_size;
    int result;
    unsigned short memaddr;
    const unsigned char *data;
    unsigned short size;

	uint8_t mem_start_addr_reset=RBLE_DMP_LOAD_START;
	int result_reset=0;

	int uncmp_times=0,write_times=0,read_times=0   ,dmp_start_reset_times=0;

	int while_times=0;

	
#if (MEMS_CHIP == HW_ICM20648 || MEMS_CHIP == HW_ICM20609)
    unsigned char data_cmp[INV_MAX_SERIAL_READ+1];
#else
    unsigned char data_cmp[INV_MAX_SERIAL_READ];
#endif
    int flag = 0;
    memset (data_cmp , 0 , (INV_MAX_SERIAL_READ + 1));
    // Write DMP memory
    data = data_start;
    size = size_start;
    memaddr = load_addr;
    while (size > 0) {

		
		#if 0 //defined(RBLE_UART_DEBUG)
			printf("fir_ld while_times=%d\n",(++while_times));
		#endif
		
        write_size = min(size, INV_MAX_SERIAL_WRITE);
        if ((memaddr & 0xff) + write_size > 0x100) {
            // Moved across a bank
            write_size = (memaddr & 0xff) + write_size - 0x100;
        }

		for(uncmp_times=0;uncmp_times<RBLE_DMP_UNCMP_TIMES;uncmp_times++)
		{
			for(write_times=0;write_times<RBLE_DMP_WRITE_TIMES;write_times++)
			{
		        result = inv_write_mems(memaddr, write_size, (unsigned char *)data);
		        if (result)  
		        {
		        	
					#if 1 //defined(RBLE_UART_DEBUG)
						printf("fir_ld rt=%d,wts=%d\n",result,write_times);
					#endif


					for( dmp_start_reset_times=0;dmp_start_reset_times<RBLE_DMP_SART_ADDR_RESET_TIMES;dmp_start_reset_times++)
					{
						result_reset = inv_serial_interface_write_hook(REG_MEM_START_ADDR, 1, &mem_start_addr_reset);
						
			            if(result_reset)
			            {
							#if 1 //defined(RBLE_UART_DEBUG)
								printf("fir_ld wrest=%d\n",result_reset);
							#endif
							
						
							result_reset = 0;
			            }
						else
						{
							break;
						}
						
					}


					if(dmp_start_reset_times>(RBLE_DMP_SART_ADDR_RESET_TIMES-1))
					{
						#if 1 //defined(RBLE_UART_DEBUG)
							printf("fir_ld rt wt st\n");
						#endif

						return -1;
					}
		        }
				else
				{
					
					break;
				}
			}


			if(write_times>(RBLE_DMP_WRITE_TIMES-1))
			{
				
				#if 1 //defined(RBLE_UART_DEBUG)
					printf("fir_ld rt=%d,rtn\n",result);
				#endif
				
				return result;
			}

			result =0;


			for(read_times=0;read_times<RBLE_DMP_READ_TIMES;read_times++)
			{
				result = inv_read_mems(memaddr, write_size, data_cmp);
		        if (result)
		        {
		        	
		            flag++; // Error, DMP not written correctly
		            
					#if 1 //defined(RBLE_UART_DEBUG)
						printf("fir_ld flag=%d\n",flag);
					#endif
		        }
				
		        if (memcmp(data_cmp, data, write_size))
		        {
		        	
					#if 1 //defined(RBLE_UART_DEBUG)
						printf("fir_ld rt=%d,rts=%d\n",result,read_times);
					#endif

					for( dmp_start_reset_times=0;dmp_start_reset_times<RBLE_DMP_SART_ADDR_RESET_TIMES;dmp_start_reset_times++)
					{
						result_reset = inv_serial_interface_write_hook(REG_MEM_START_ADDR, 1, &mem_start_addr_reset);

						if(result_reset)
			            {
							#if 1 //defined(RBLE_UART_DEBUG)
								printf("fir_ld Rrst=%d\n",result_reset);
							#endif

							result_reset = 0;
						}
						else
						{
							break;
						}
					}


					if(dmp_start_reset_times>(RBLE_DMP_SART_ADDR_RESET_TIMES-1))
					{
						#if 1 //defined(RBLE_UART_DEBUG)
							printf("fir_ld rt rd st\n");
						#endif

						return -1;
					}
		            
		        }
				else
				{
					break;
				}
			}


			if(read_times<RBLE_DMP_READ_TIMES)
			{
				break;
			}

		}

		if(uncmp_times>(RBLE_DMP_UNCMP_TIMES-1))
		{
			#if 1 //defined(RBLE_UART_DEBUG)
				printf("fir_ld uncmp rtn\n");
			#endif

			return -1;
		}
		
        data += write_size;
        size -= write_size;
        memaddr += write_size;
    }
 

#if defined(WIN32)   
    if(!flag)
      inv_log("DMP Firmware was updated successfully..\r\n");
#endif

#if 1 //defined(RBLE_UART_DEBUG)
			printf("inv_mems_firmware_load success\n");
#endif


    return INV_SUCCESS;
}


#else

inv_error_t inv_mems_firmware_load(const unsigned char *data_start, unsigned short size_start, unsigned short load_addr)
{ 
    int write_size;
    int result;
    unsigned short memaddr;
    const unsigned char *data;
    unsigned short size;
#if (MEMS_CHIP == HW_ICM20648 || MEMS_CHIP == HW_ICM20609)
    unsigned char data_cmp[INV_MAX_SERIAL_READ+1];
#else
    unsigned char data_cmp[INV_MAX_SERIAL_READ];
#endif
    int flag = 0;
    memset (data_cmp , 0 , (INV_MAX_SERIAL_READ + 1));
    // Write DMP memory
    data = data_start;
    size = size_start;
    memaddr = load_addr;
    while (size > 0) {
        write_size = min(size, INV_MAX_SERIAL_WRITE);
        if ((memaddr & 0xff) + write_size > 0x100) {
            // Moved across a bank
            write_size = (memaddr & 0xff) + write_size - 0x100;
        }
        result = inv_write_mems(memaddr, write_size, (unsigned char *)data);
        if (result)  
            return result;
        data += write_size;
        size -= write_size;
        memaddr += write_size;
    }
    //memcmp(data_cmp, data, write_size);
    // Verify DMP memory

    data = data_start;
    size = size_start;
    memaddr = load_addr;
    while (size > 0) {
        write_size = min(size, INV_MAX_SERIAL_READ);
        if ((memaddr & 0xff) + write_size > 0x100) {
            // Moved across a bank
            write_size = (memaddr & 0xff) + write_size - 0x100;
        }
        result = inv_read_mems(memaddr, write_size, data_cmp);
        if (result)
            flag++; // Error, DMP not written correctly
        if (memcmp(data_cmp, data, write_size))
            return -1;
        data += write_size;
        size -= write_size;
        memaddr += write_size;
    }
    

#if defined(WIN32)   
    if(!flag)
      inv_log("DMP Firmware was updated successfully..\r\n");
#endif

#if defined(RBLE_UART_DEBUG)
			printf("inv_mems_firmware_load success\n");
#endif


    return INV_SUCCESS;
}
#endif

