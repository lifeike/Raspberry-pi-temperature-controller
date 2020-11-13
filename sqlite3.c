/*********************************************************************************
 *      Copyright:  (C) 2020 LuXiaoyang<920916829@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  sqlite3.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(30/07/20)
 *         Author:  LuXiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "30/07/20 07:09:23"
 *                 
 ********************************************************************************/
#include "sqlite3.h"






/*******************************************************************************
 * 函数名：Create_Get_Table()
 * 功能：创建一个新的或者获取一个已存在的数据库与表
 * 参数：table_name - 要创建或者获取的表的表名
 * 返回值：返回该数据库的句柄
 ******************************************************************************/
sqlite3 *Create_Get_Table(char *table_name)
{
    char              *errmsg;
    int                rv = -1;
    char              *sql[128] = {0};
    sqlite3           *db;

    if(sqlite3_open(table_name,&db) != SQLITE_OK)
    {
        printf("Open failure\n");
        return NULL;
    }

    rv = sqlite3_exec(db,"create table upload (data char);",NULL,NULL,&errmsg);

    if((rv != SQLITE_OK) && (rv != 1))
    {
        printf("Create or get table failure:%s\n",errmsg);
        return NULL;
    }

    if(rv == 1)
    {
        printf("The table already exist,so get it\n");
    }

    else if(rv == 0)
    {
        printf("Create a new table!\n");
    }

    return db;
}


/*******************************************************************************
 * 函数名：Save_Tlv_To_Database
 * 功能：将传入的数据保存至数据库
 * 参数：db - 要操作的数据库的句柄，tlv - tlv_buf_t 类型的结构体指针
 * 返回值：成功返回 0 ， 失败返回 -1
 ******************************************************************************/
int Save_Tlv_To_Database(sqlite3 *db,tlv_buf_t *tlv)
{
    char             sql[56] = {0};
    char            *errmsg = NULL;

    sprintf(sql,"insert into upload values ('%s');",tlv->buf);
    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
    {
        syslog(LOG_ERR,"Save tlv buf to database failure:%s\n",errmsg);
        printf("Save tlv buf to database failure:%s\n",errmsg);
        return -1;
    }

    return 0;
}


/*******************************************************************************
 * 函数名：Get_One_TlvPacket_From_Database()
 * 功能：从数据库中获取第一条数据
 * 参数：db - 要操作的数据库的句柄，tl - tlv_buf_t 类型的一个结构体指针
 * 返回值：获取失败返回-1 ; 若数据库为空则返回 0 ; 获取成功返回 1
 ******************************************************************************/
int Get_One_TlvPacket_From_Database(sqlite3 *db,tlv_buf_t *tlv)
{
    char          *errmsg;
    char         **dbResult;
    int            nRow,nColumn;

    tlv_buf_init(tlv);

    if(sqlite3_get_table(db,"select * from upload",&dbResult,&nRow,&nColumn,&errmsg) != SQLITE_OK)
    {
        printf("Seek Massage from database failure\n");
        return -1;
    }

    /* Check if the database is empty */
    if(!nRow)
    {
        printf("There is no massage in database\n");
        return 0;
    }

    /* If not empty,copy the first data from database */
    memcpy(tlv->buf,dbResult[1],tlv->len);

    return 1;
}



/*******************************************************************************
 * 函数名：Delete_One_TlvPacket_From_Database()
 * 功能：删除数据库中的第一条数据
 * 参数：db - 要操作的数据库的句柄
 * 返回值：成功删除返回 0 ; 删除失败返回 1 
 ******************************************************************************/
int Delete_One_TlvPacket_From_Database(sqlite3 *db)
{
    char            *errmsg = NULL;

    if(sqlite3_exec(db,"delete from upload where data = (select data from upload limit 1);",NULL,NULL,&errmsg) != SQLITE_OK)
    {
        printf("Delete the first massage from table failure:%s\n",errmsg);
        return 1;
    }

    return 0;
}
