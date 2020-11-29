/*********************************************************************************
 *       Filename:  socket_server_thread.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/17/2020)
 *      ChangeLog:  1, Release initial version on "04/17/2020 10:10:57 PM"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <ctype.h>
#include <sqlite3.h>
#include <libgen.h>
#include "crc.h"

#define sqlite3_name "project.db"
#define HEAD         0x01
#define len          11
#define ID_S         0x02
#define TIME_S       0x03
#define TEMP_S       0x04

void print_helpmsg(char *progname)
{
    printf("%s help massage:\n",progname);
    printf("-p(--port) The port you will listen\n");
    printf("-h(--help) Print help massage\n");
}


typedef void *(work_type) (void *thread_arg);
void *thread_work(void *ctx);
int thread_start(pthread_t *id,work_type *work_ptr,void *thread_arg);
sqlite3 *Create_Get_Table(char *sqlite_name);


int socket_init(int port);

int main(int argc, char *argv[])
{
    int                    clifd;
    int                    retval;
    int                    sockfd = -1;
    int                    listen_port = 0;
    pthread_t              tid;
    int                    ch;
    char                  *progname = NULL;
    sqlite3               *db;
    char                  *errmsg;
    struct sockaddr_in     cliaddr;
    socklen_t              addrlen = sizeof(cliaddr);


    struct option          opts[] = {
        {"port",required_argument,NULL,'p'},
        {"help",no_argument,NULL,'h'},
        {NULL,0,NULL,0}
    };

    progname = basename(argv[0]);

    while((ch = getopt_long(argc,argv,"p:h",opts,NULL)) != -1)
    {
        switch(ch)
        {
            case 'p':
                listen_port = atoi(optarg);
                break;

            case 'h':
                print_helpmsg(progname);
                break;
        }
    }



    if(!listen_port)
    {
        print_helpmsg(progname);
        return -1;
    }

    sockfd = socket_init(listen_port);
    if(sockfd < 0)
    {
        printf("Socket init failure\n");
        return -1;
    }

    db = Create_Get_Table(sqlite3_name);
    if(!db)
    {
        printf("Sqlite failure:%s\n",strerror(errno));
        return -1;
    }


    while(1)
    {
        sleep(1);
        printf("Wait client connect\n");
        clifd = accept(sockfd,NULL,NULL);
        if(clifd < 0)
        {
            printf("Accept new client failure:%s\n",strerror(errno));
            continue;
        }

        printf("Accept new client[%d]!\n",clifd);

        thread_start(&tid,thread_work,(void *)clifd);
    }

    close(sockfd);

    return 0;
}

void *thread_work(void *ctx)
{
    sqlite3               *db;
    int                    i;
    char                   ACK_buf[10] = "1";
    char                   NAK_buf[10] = "0";
    int                    retval;
    int                    clifd;
    char                   buf[56];
    long int               size;
    int                    year = 0;
    int                    month = 0;
    int                    day = 0;
    int                    hour = 0;
    int                    minute = 0;
    int                    second = 0;
    char                   id[10]={0};
    char                   time[128] = {0};
    char                   temp[10] = {0};
    unsigned short         new_crc16 = 0;
    unsigned short         crc16 = 0;
    char                   sql[128] = {0};
    char                  *errmsg;

    if(!ctx)
    {
        printf("Invalid argument\n");
        pthread_exit(NULL);
    }

    clifd = (int)ctx;
    db = Create_Get_Table(sqlite3_name);
    if(!db)
    {
        printf("Sqlite failure:%s\n",strerror(errno));
        close(clifd);
        pthread_exit(NULL);
    }


    printf("Child thread start communicate with client....\n");

    for( ; ; )
    {

recv_msg:
        bzero(buf,sizeof(buf));
        retval = read(clifd,buf,sizeof(buf));
        if(retval <= 0)
        {
            printf("Recieve massage faliure:%s\n",strerror(errno));
            close(clifd);
            pthread_exit(NULL);
        }

        size = sizeof(buf);

check_size:
        if(size < 18)
        {
            retval = write(clifd,NAK_buf,sizeof(NAK_buf));
            if(retval <= 0 )
            {
                printf("Write to client failure:%s\n",strerror(errno));
                close(clifd);
                pthread_exit(NULL);
            }

            goto recv_msg;
        }

        for(i = 0; i < size; i++)
        {
            if(buf[i] == HEAD)
            {
                printf("HEAD right\n");
                if(buf[i+1] == len)
                {
                    printf("lenth right\n");
                    if(buf[i+2] == ID_S)
                    {
                        printf("ID_S right\n");
                        memcpy(id,&buf[i+3],3);
                        if(buf[6] == TIME_S)
                        {
                            year = buf[i+7];
                            year += 1900;
                            month = buf[i+8];
                            day = buf[i+9];
                            hour = buf[i+10];
                            minute = buf[i+11];
                            second = buf[i+12];
                            sprintf(time,"%d年%d月%d日%d时%d分%d秒\n",year,month,day,hour,minute,second);
                            if(buf[13] == TEMP_S)
                            {
                                sprintf(temp,"%d.%d",buf[14],buf[15]);
                                crc16 = bytes_to_ushort((unsigned char *)&buf[i+16],2);
                                new_crc16 = crc_itu_t(MAGIC_CRC,buf,16);

                                if(crc16 == new_crc16)
                                {
                                    printf("CRC check success\n");
                                    printf("The data will be stored in the database!\n");
                                    retval = write(clifd,ACK_buf,sizeof(ACK_buf));
                                    if(retval <= 0)
                                    {
                                        printf("Write to client failure:%s\n",strerror(errno));
                                        close(clifd);
                                        pthread_exit(NULL);
                                    }
                                    else 
                                    {
                                        sprintf(sql,"insert into data values('%s','%s','%s');",id,time,temp);
                                        retval = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
                                        if(retval != SQLITE_OK)
                                        {
                                            printf("Can not use database:%s\n",errmsg);
                                            close(clifd);
                                            pthread_exit(NULL);
                                        }
                                        else 
                                        {
                                            printf("Inset into table ok!\n");
                                            goto recv_msg;
                                        }

                                    }
                                }
                                else
                                {
                                    printf("CRC check failure!\n");
                                    retval = write(clifd,NAK_buf,sizeof(NAK_buf));
                                    if(retval <= 0)
                                    {
                                        printf("write to client failure:%s\n",strerror(errno));
                                        close(clifd);
                                        pthread_exit(NULL);
                                    }
                                    else 
                                    {
                                        goto recv_msg;
                                    }

                                }
                            }
                            else 
                            {
                                memmove(buf,&buf[i+14],size - i - 14);
                                size = size - i - 14;
                                goto check_size;
                            }

                        }
                        else 
                        {
                            memmove(buf,&buf[i+7],size - i - 7);
                            size = size - i - 7;
                            goto check_size;
                        }
                    }
                    else 
                    {
                        memmove(buf,&buf[i+3],size - i - 3);
                        size = size - i - 3;
                        goto check_size;
                    }
                } //check lenth
                else 
                {
                    memmove(buf,&buf[i+2],size - i -2);
                    size = size - i - 2;
                    goto check_size;
                } 
            } //check head
            else 
            {
                memmove(buf,&buf[i+1],size - i - 1);
                size = size -i - 1;
                goto check_size;
            } 
        } //for()

    }
}

int socket_init(int port)
{
    int                        on = 1;
    int                        retval = -1;
    int                        sockfd = -1;
    struct sockaddr_in         servaddr;
    socklen_t                  addrlen = sizeof(servaddr);

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        printf("Socket failure\n");
        return retval;
    }

    printf("sockfd ok\n");
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    memset(&servaddr,0,sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sockfd,(struct sockaddr *)&servaddr,addrlen);
    if(retval < 0)
    {
        printf("Bind failure:%s\n",strerror(errno));
        return -1;
    }

    listen(sockfd,13);

    return sockfd;
}

sqlite3 *Create_Get_Table(char *sqlite_name)
{
    char              *errmsg;
    int                rv = -1;
    char              *sql[128] = {0};
    sqlite3           *db;

    if(sqlite3_open(sqlite_name,&db) != SQLITE_OK)
    {
        printf("Open failure\n");
        return NULL;
    }

    rv = sqlite3_exec(db,"create table data (id char,time char,temp char);",NULL,NULL,&errmsg);

    if((rv != SQLITE_OK) && (rv != 1))
    {
        printf("Create or get table failure:%s\n",errmsg);
        return NULL;
    }

    if(rv == 1)
    {
        printf("The table already exist,so get it\n");
    }

    else if(rv == SQLITE_OK)
    {
        printf("Create a new table!\n");
    }

    return db;
}

int thread_start(pthread_t *id,work_type *work_ptr,void *thread_arg)
{
    int                retval = -1;
    pthread_attr_t     thread_attr;

    if( pthread_attr_init(&thread_attr) )
    {
        printf("pthread_attr_init() failure: %s\n", strerror(errno));
        goto CleanUp;
    }

    if( pthread_attr_setstacksize(&thread_attr, 120*1024) )
    {
        printf("pthread_attr_setstacksize() failure: %s\n", strerror(errno));
        goto CleanUp;
    }

    if( pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) )
    {
        printf("pthread_attr_setdetachstate() failure: %s\n", strerror(errno));
        goto CleanUp;
    }
     /* Create the thread */
    if( pthread_create(id, &thread_attr, work_ptr, thread_arg) )
    {
        printf("Create thread failure: %s\n", strerror(errno));
        goto CleanUp;
    }

    retval = 0;
 
CleanUp:
 
 /* Destroy the attributes of thread */
 pthread_attr_destroy(&thread_attr);
 return retval;
}

