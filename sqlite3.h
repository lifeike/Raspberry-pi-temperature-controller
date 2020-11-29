/********************************************************************************
 *       Filename:  sqlite3.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(30/07/20)
 *      ChangeLog:  1, Release initial version on "30/07/20 07:06:53"
 *                 
 ********************************************************************************/
#ifndef  _SQLITE3_H_
#define  _SQLITE3_H_


#include <stdio.h>
#include <sqlite3.h>
#include <syslog.h>
#include "tlv.h"

#define sqlite_name      "./temperature.db"


sqlite3 *Create_Get_Table(char *table_name);


int Save_Tlv_To_Database(sqlite3 *db,tlv_buf_t *tlv);




int Get_One_TlvPacket_From_Database(sqlite3 *db,tlv_buf_t *tlv);




int Delete_One_TlvPacket_From_Database(sqlite3 *db);







#endif   /*  ----- #ifndef _SQLITE3_H_  ----- */
