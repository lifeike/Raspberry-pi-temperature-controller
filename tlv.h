/********************************************************************************
 *      Copyright:  (C) 2020 LuXiaoyang<920916829@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  tlv.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(30/07/20)
 *         Author:  LuXiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "30/07/20 07:15:43"
 *                 
 ********************************************************************************/
#ifndef  _TLV_H_
#define  _TLV_H_


#include <string.h>
#include "crc.h"
#include "temp_time_get.h"

#define TLV_BUFSIZE      128
#define HEAD             0x01
#define ID_S             0x02
#define TIME_S           0x03
#define TEMP_S           0x04
#define tlv_buf_init(p)  { memset((p)->buf, 0, sizeof((p)->buf)); (p)->size=TLV_BUFSIZE; (p)->len=18; }

typedef struct tlv_buf_s
{
    char          buf[TLV_BUFSIZE]; /*  tlv buffer */
    int           size;     /*  tlv buffer size    */
    int           len;      /*  tlv packet length  */
} tlv_buf_t;



int Tlv_Packet(char *id,time_type *cur_t,int temp_integer,int temp_decimal,tlv_buf_t *tlv);


#endif   /* ----- #ifndef _TLV_H_  ----- */

