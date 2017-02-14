/*
 $License:
    Copyright (C) 2011 InvenSense Corporation, All Rights Reserved.
    See included License.txt for License information.
 $
 */
/*******************************************************************************
 *
 * $Id: mlmath.c 5629 2011-06-11 03:13:08Z mcaramello $
 *
 *******************************************************************************/


#include <math.h>

#include "../../qfplib/qfplib.h"
#include "../../qfplib/qfpio.h"

#define RBLE_FLOAT_MATH_QFBLIB


double ml_asin(double x)
{
   	return asin(x);
}

double ml_atan(double x)
{
    return atan(x);
}

double ml_atan2(double x, double y)
{

#if defined(RBLE_FLOAT_MATH_QFBLIB)
	return qfp_fatan2(x, y);
#else
    return atan2(x, y);
#endif

}

double ml_log(double x)
{
    return log(x);
}

double ml_sqrt(double x)
{
#if defined(RBLE_FLOAT_MATH_QFBLIB)
	return qfp_fsqrt(x);
#else
    return sqrt(x);
#endif
}

double ml_ceil(double x)
{
    return ceil(x);
}

double ml_floor(double x)
{
    return floor(x);
}

double ml_cos(double x)
{
#if defined(RBLE_FLOAT_MATH_QFBLIB)
    return qfp_fcos(x);
#else
    return cos(x);
#endif
}

double ml_sin(double x)
{
	#if defined(RBLE_FLOAT_MATH_QFBLIB)
		return qfp_fsin(x);
	#else
    	return sin(x);
	#endif
}
 
double ml_acos(double x)
{

   return acos(x);

}

double ml_pow(double x, double y)
{
    return pow(x, y);
}
