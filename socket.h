/********************************************************************************
 *      Copyright:  (C) 2020 LuXiaoyang<920916829@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(30/07/20)
 *         Author:  LuXiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "30/07/20 07:39:27"
 *                 
 ********************************************************************************/
#ifndef  _SOCKET_H_
#define  _SOCKET_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "temp_time_get.h"





int Connect_To_Server(int serv_port,char *serv_ip,char  *domain_name);

#endif   /* ----- #ifndef _SOCKET_H_  ----- */

