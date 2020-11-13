/********************************************************************************
 *      Copyright:  (C) 2020 LuXiaoyang<920916829@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  sqlite3.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(30/07/20)
 *         Author:  LuXiaoyang <920916829@qq.com>
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



/*******************************************************************************
 * 函数名：Create_Get_Table()
 * 功能：创建一个新的或者获取一个已存在的数据库与表
 * 参数：table_name - 要创建或者获取的表的表名
 * 返回值：返回该数据库的句柄
 ******************************************************************************/
sqlite3 *Create_Get_Table(char *table_name);




/*******************************************************************************
 * 函数名：Save_Tlv_To_Database
 * 功能：将传入的数据保存至数据库
 * 参数：db - 要操作的数据库的句柄，tlv - tlv_buf_t 类型的结构体指针
 * 返回值：成功返回 0 ， 失败返回 -1
 ******************************************************************************/
int Save_Tlv_To_Database(sqlite3 *db,tlv_buf_t *tlv);





/*******************************************************************************
 * 函数名：Get_One_TlvPacket_From_Database()
 * 功能：从数据库中获取第一条数据
 * 参数：db - 要操作的数据库的句柄，tl - tlv_buf_t 类型的一个结构体指针
 * 返回值：获取失败返回-1 ; 若数据库为空则返回 0 ; 获取成功返回 1
 ******************************************************************************/
int Get_One_TlvPacket_From_Database(sqlite3 *db,tlv_buf_t *tlv);




/*******************************************************************************
 * 函数名：Delete_One_TlvPacket_From_Database()
 * 功能：删除数据库中的第一条数据
 * 参数：db - 要操作的数据库的句柄
 * 返回值：成功删除返回 0 ; 删除失败返回 1 
 ******************************************************************************/
int Delete_One_TlvPacket_From_Database(sqlite3 *db);







#endif   /*  ----- #ifndef _SQLITE3_H_  ----- */
