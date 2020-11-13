/*********************************************************************************
 *      Copyright:  (C) 2020 Xiao yang IoT System Studio
 *                  All rights reserved.
 *
 *       Filename:  temprature_get.c
 *    Description:  This file is get temperature from RPi
 *                 
 *        Version:  1.0.0(03/30/2020)
 *         Author:  Lu Xiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "03/30/2020 10:15:30 AM"
 *                 
 ********************************************************************************/
#ifndef  _TEMP_TIME_GET_H_
#define  _TEMP_TIME_GET_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

/* This function is give you the temperature and return the address of RPi_id */
typedef struct TIME
{
    int                       year;
    int                       month;
    int                       day;
    int                       hour;
    int                       minute;
    int                       second;
}time_type;

int  temperature_get(int *temp_integer,int *temp_decimal);
void time_get(time_type *cur_t);





#endif   /*  ----- #ifndef _TEMP_TIME_GET_H_  -----  */
