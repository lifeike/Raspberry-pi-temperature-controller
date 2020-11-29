/*********************************************************************************
 *       Filename:  tlv.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(30/07/20)
 *      ChangeLog:  1, Release initial version on "30/07/20 07:20:32"
 *                 
 ********************************************************************************/
#include "tlv.h"


 /******************************************************************************
 
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

