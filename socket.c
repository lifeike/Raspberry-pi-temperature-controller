/*********************************************************************************
 *      Copyright:  (C) 2020 LuXiaoyang<920916829@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(30/07/20)
 *         Author:  LuXiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "30/07/20 07:43:13"
 *                 
 ********************************************************************************/
#include "socket.h"


/*******************************************************************************
 * 函数名：Connect_To_Server()
 * 功能：创建用于连接的套接字，并通过域名或ip地址连接服务器
 * 参数：serv_port - 监听端口，serv_ip - 服务器ip地址，domain_name - 服务器域名
 * 返回值：若连接成功，则返回用于通信的套接字，若失败返回-1
 ******************************************************************************/
int Connect_To_Server(int serv_port,char *serv_ip,char  *domain_name)
{
    if(!serv_port || (!serv_ip && !domain_name))
    {
        printf("Invalid input\n");
    }

    int                      sockfd = -1;
    struct hostent          *getname;
    struct sockaddr_in       servaddr;
    socklen_t                addrlen = sizeof(servaddr);

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        printf("Socket create failure:%s\n",strerror(errno));
        return -1;
    }
    printf("Create sockfd ok[%d]\n",sockfd);

    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serv_port);

    if(serv_ip)
    {
        printf("Connect to server by ipaddr\n");
        inet_pton(AF_INET,serv_ip,&servaddr.sin_addr);
    }

    if(domain_name)
    {
        printf("Connect to server by domain name\n");
        getname = gethostbyname(domain_name);
        servaddr.sin_addr = *(struct in_addr* )getname->h_addr;
    }

    if(connect(sockfd,(struct sockaddr *)&servaddr,addrlen) < 0)
    {
        printf("Connect to server failure:%s\n",strerror(errno));
        printf("The server is not online\n");
        close(sockfd);
        return -1;
    }


    return sockfd;

}




