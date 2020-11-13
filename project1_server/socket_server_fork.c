/*********************************************************************************
 *      Copyright:  (C) 2020 Xiao yang IoT System Studio
 *                  All rights reserved.
 *
 *       Filename:  socket_server_fork.c
 *    Description:  This file server by fork
 *                 
 *        Version:  1.0.0(03/08/2020)
 *         Author:  Lu Xiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "03/08/2020 05:02:49 PM"
 *                 
 ********************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>        
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "crc.h"
#include <sqlite3.h>

#define sqlite_name "./temperature_get.db"
#define HEAD            0x01
#define ID_S            0x02
#define TIME_S          0x03
#define TEMP_S          0x04

int conmunicate_with_client(int fd,sqlite3 *db);
int socket_init(int listen_port);
sqlite3 *Create_Get_Table(char *table_name);
void print_usage(char *progname)
{
    printf("%s usage:\n",progname);
    printf("-p(--port) input the port you will bind\n");
    printf("-d(--daemon) the program will run at background\n");
    printf("-h(--help) print the help massage\n");
    return ;

}

int main(int argc,char **argv)
{
    char                       buf[20] = {0};
    char                      *progname;
    int                        listenfd = -1;
    int                        daemon_run = 0;
    int                        clifd = -1;
    int                        listen_port = 0;
    int                        rv = -1;
    int                        ch;
    pid_t                      pid;
    char                      *errmsg;
    sqlite3                   *db = NULL;
    struct option             opts[] = {
        {"port",required_argument,0,'p'},
        {"daemon",no_argument,0,'d'},
        {"help",no_argument,0,'h'},
        {NULL,0,NULL,0}
    };

    progname = basename(argv[0]);

    while((ch = getopt_long(argc,argv,"p:dh",opts,NULL)) != -1)
    {
        switch(ch)
        {
            case 'p':
                listen_port = atoi(optarg);
                break;

            case 'h':
                print_usage(progname);
                break;

            case 'd':
                daemon_run = 1;
                break;

            default:
                break;

        }
    }

    if(!listen_port)
    {
        print_usage(progname);
        return -1;
    }
    
    if((db = Create_Get_Table(sqlite_name)) == NULL)
    {
        printf("Can not use sqlite3\n");
        return -1;
    }
    
    if((listenfd = socket_init(listen_port)) < 0)
    {
        printf("Socket init failure\n");
        return -1;
    }
    
    printf("Create socket ok\n");

    if(daemon_run)
    {
        daemon(0,0);
                            
    }

    while(1)
    {
        if((clifd = accept(listenfd,NULL,NULL)) < 0)
        {
            perror("Can not accept client connect");
            close(listenfd);
            return -1;
        }

        printf("Accept new client\n");

        pid = fork();

        if(pid < 0)
        {
            perror("fork() failure");
            close(listenfd);
            return -1;
        }
        else if(pid > 0)
        {
            close(clifd);
            continue;
        }
        else //if(pid == 0)
        {
           close(listenfd);
           if((rv = conmunicate_with_client(clifd,db)) <= 0)
           {
                 close(clifd);
                 continue;

           }

        }

    return 0;
        
   }
}

int conmunicate_with_client(int fd,sqlite3 *db)
{
    char                   buf[128];
    int                    rv = -1;
    int                    i;
    int                   year = 0;
    int                    month = 0;
    int                    day = 0;
    int                    hour = 0;
    int                    minute = 0;
    int                    second = 0;
    int                    temp1 = 0;
    int                    temp2 = 0;
    int                    size = 0;
    char                   id[10]={0};
    char                   time[128] = {0};
    char                   temp[10] = {0};
    unsigned short         recv_crc16 = 0;
    unsigned short         crc16 = 0;
    char                   sql[128] = {0};
    char                  *errmsg;


    while(1)
    {

read_wait:
        if((rv = read(fd,buf,sizeof(buf))) <= 0)
        {
            perror("Read failure");
            close(fd);
            return rv;
        }
 
        size = strlen(buf);

        for(i = 0;i < rv;i++)
        {
            printf("0x%02x",buf[i]);
        }
        printf("\n");

check_size:  
        if(size < 18)
        {
            printf("The massage is too short,so discard\n");
            goto read_wait;
        }

        for(i = 0;i < size;i++)
        {
            if(buf[i] == HEAD)
            {
                if(buf[i+1] == 11)
                {
                    if(buf[i+2] == ID_S)
                    {
                        memcpy(id,&buf[i+3],3);
                        if(buf[i+6] == TIME_S)
                        {
                            year = buf[i+7];
                            year+=1900;
                            month = buf[i+8];
                            day = buf[i+9];
                            hour = buf[i+10];
                            minute = buf[i+11];
                            second = buf[i+12];
                            sprintf(time,"%d年%d月%d日%d时%d分%d秒\n",year,month,day,hour,minute,second);
                            if(buf[i+13] == TEMP_S)
                            {
                                sprintf(temp,"%d.%d",buf[i+14],buf[i+15]);
                                recv_crc16 = bytes_to_ushort((unsigned char*)&buf[i+16],2);
                                crc16 = crc_itu_t(MAGIC_CRC,buf,16);
                                if(recv_crc16 == crc16)
                                {
                                    printf("校验成功，数据无误，将写入数据库\n");
                                    sprintf(sql,"insert into Eventually_data values('%s','%s','%s');",id,time,temp);
                                    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
                                    {
                                        printf("insert data failure\n");
                                        memset(buf,0,sizeof(buf));
                                        goto read_wait;
                                    }
                                    
                                    else
                                    {
                                        printf("Insert into table ok\n");
                                        goto read_wait;
                                    }
                                }
                                else
                                {
                                    printf("校验失败，数据有误\n");
                                    memmove(buf,&buf[i+18],size-i-18);
                                    goto read_wait;
                                }
                            }
                            else
                            {
                                printf("TEMP_S error\n");
                                memmove(buf,&buf[i+14],size-i-14);
                                size=size-i-14;
                                goto check_size;
                            }
                        }
                        else
                        {
                            memmove(buf,&buf[i+7],size-i-7);
                            size=size-i-7;
                            goto check_size;
                        }
                    }
                    else
                    {
                        memmove(buf,&buf[i+3],size-i-3);
                        size=size-i-3;
                        goto check_size;
                    }
                }
                else
                {
                    memmove(buf,&buf[i+2],size-i-2);
                    size=size-i-2;
                    goto check_size;
                }
            }
            else
            {
                memmove(buf,&buf[i+1],size-i-1);
                size = size - i - 1;
                goto check_size;
            }
        }
    }

    return 0;
}
                                
sqlite3 *Create_Get_Table(char *table_name)
{
    char                *errmsg;
    sqlite3             *db;
    char                 sql[128] = {0};
    int                  rv = -1;

    if(sqlite3_open(table_name,&db) != SQLITE_OK)
    {   
        printf("sqlite create failure:%s\n",sqlite3_errmsg(db));
        return NULL;
    }

    rv = sqlite3_exec(db,"create table Eventually_data (id char,time char,temp char);",NULL,NULL,&errmsg);
    if((rv != SQLITE_OK) && (rv != 1))   //若表格已存在，则sqlite3_exec 返回1；
    {
        printf("Create or Get table failure:%s\n",errmsg);
        return NULL;
    }
    
    if(rv == SQLITE_OK)
    {
        printf("Create new table ok\n");
    }

    else if(rv == 1)
    {
        printf("Get table successfully. \n");
    }

    return db;
}

int socket_init(int listen_port)
{
    int                  listenfd = -1;
    struct sockaddr_in   servaddr;
    socklen_t            addrlen = sizeof(servaddr);

    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("Create socket failure");
        return -1;
    }
    printf("Create listenfd[%d] OKK\n",listenfd);

    memset(&servaddr,0,addrlen);
    servaddr.sin_port = htons(listen_port);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr =htonl(INADDR_ANY);


    if((bind(listenfd,(struct sockaddr *)&servaddr,addrlen)) < 0)
    {
        printf("Bind listenfd[%d] on port[%d] failure:%s\n",listenfd,listen_port,strerror(errno));
        return -2;
    }

    listen(listenfd,13);

    return listenfd;
}





