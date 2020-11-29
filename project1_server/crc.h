/********************************************************************************
 *       Filename:  crc.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(04/02/2020)
 *      ChangeLog:  1, Release initial version on "04/02/2020 04:53:41 PM"
 *                 
 ********************************************************************************/
#ifndef CRC_ITU_T_H
#define CRC_ITU_T_H

#define MAGIC_CRC           0x1E50

extern const unsigned short  crc_itu_t_table[256];

extern int ushort_to_bytes(unsigned char *bytes, unsigned short val);
extern unsigned short bytes_to_ushort(unsigned char *bytes, int len);

extern unsigned short crc_itu_t(unsigned short crc, const unsigned char *buffer, unsigned int len);

static inline unsigned short crc_itu_t_byte(unsigned short crc, const unsigned char data)
{
		return (crc << 8) ^ crc_itu_t_table[((crc >> 8) ^ data) & 0xff];
}

#endif /* TLV.CRC.H */

