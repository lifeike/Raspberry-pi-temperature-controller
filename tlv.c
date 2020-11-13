/*********************************************************************************
 *      Copyright:  (C) 2020 LuXiaoyang<920916829@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  tlv.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(30/07/20)
 *         Author:  LuXiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "30/07/20 07:20:32"
 *                 
 ********************************************************************************/
#include "tlv.h"



/*******************************************************************************
 * 函数名：Tlv_Packet
 * 功能：对结构体中的buf进行tlv封包，封包内容包括CRC校验和
 * 参数：id - 用户id
 *       cur_t - time_type 类型的结构体指针，指向了一个包含了年月日时分秒的结构体
         temp_integer，temp_decimal - 温度的整数部分和小数部分
         tlv - tlv_buf_t 类型的结构体指针
 * 返回值：返回数组大小
 
 * TLV buf:
 * +----+-----+----+---------------+----+-----+-----+-----+----+------+------+----+----------------+---------------+
 * |    |     |    |               |    |     |     |     |    |      |      |    |                |               |
 * |HEAD|Lenth|0x02|3 bytes for sn |0x03| Year|Month| Day |Hour|Minute|Second|0x04|2 bytes for temp|2 bytes for CRC|
 * |    |     |    |               |    |     |     |     |    |      |      |    |                |               |
   +----+-----+----+---------------+----+-----+-----+-----+----+------+------+----+----------------+---------------+
   A total of 18 bytes. 

 ******************************************************************************/
int Tlv_Packet(char *id,time_type *cur_t,int temp_integer,int temp_decimal,tlv_buf_t *tlv)
{
    unsigned short crc16 = 0;
    tlv_buf_init(tlv);

    tlv->buf[0] = HEAD;
    tlv->buf[1] = 11;
    tlv->buf[2] = ID_S;
    memcpy(&(tlv->buf[3]),id,3);
    tlv->buf[6] = TIME_S;
    tlv->buf[7] = cur_t->year;
    tlv->buf[8] = cur_t->month;
    tlv->buf[9] = cur_t->day;
    tlv->buf[10] = cur_t->hour;
    tlv->buf[11] = cur_t->minute;
    tlv->buf[12] = cur_t->second;
    tlv->buf[13] = TEMP_S;
    tlv->buf[14] = temp_integer;
    tlv->buf[15] = temp_decimal;

    crc16=crc_itu_t(MAGIC_CRC,tlv->buf,16);
    ushort_to_bytes(&(tlv->buf[16]),crc16);

    return tlv->len;
}

