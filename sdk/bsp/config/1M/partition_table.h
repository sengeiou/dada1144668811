/*
 *****************************************************************************************
 *
 * Copyright (C) 2016. Dialog Semiconductor, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor. All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <black.orca.support@diasemi.com> and contributors.
 *
 *****************************************************************************************
 */
#if defined(CUSTOM_CONFIG_1M_DEFINE)
 //custom defined
 PARTITION2( 0x000000 , 0x01F000 , NVMS_FIRMWARE_PART		 , 0 )
 PARTITION2( 0x01F000 , 0x001000 , NVMS_PARTITION_TABLE 	 , PARTITION_FLAG_READ_ONLY )
 PARTITION2( 0x020000 , 0x002000 , NVMS_PARAM_PART			 , 0 )
 PARTITION2( 0x022000 , 0x030000 , NVMS_BIN_PART			 , 0 )
 PARTITION2( 0x052000 , 0x00D000 , NVMS_LOG_PART			 , 0 )
 PARTITION2( 0x05F000 , 0x00F000 , NVMS_GENERIC_PART		 , PARTITION_FLAG_VES )
 PARTITION2( 0x06E000 , 0x08D000 , NVMS_IMAGE_DATA_STORAGE_PART 		, 0 )
 PARTITION2( 0x0FB000 , 0x005000 , NVMS_IMAGE_RESULT_DATA_STORAGE_PART		   , 0 )
#else
//default
PARTITION2( 0x000000 , 0x07F000 , NVMS_FIRMWARE_PART        , 0 )
PARTITION2( 0x07F000 , 0x001000 , NVMS_PARTITION_TABLE      , PARTITION_FLAG_READ_ONLY )
PARTITION2( 0x080000 , 0x010000 , NVMS_PARAM_PART           , 0 )
PARTITION2( 0x090000 , 0x030000 , NVMS_BIN_PART             , 0 )
PARTITION2( 0x0C0000 , 0x020000 , NVMS_LOG_PART             , 0 )
PARTITION2( 0x0E0000 , 0x020000 , NVMS_GENERIC_PART         , PARTITION_FLAG_VES )
#endif
