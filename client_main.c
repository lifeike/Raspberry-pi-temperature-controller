/*********************************************************************************
 *      Copyright:  (C) 2020 Xiao yang IoT System Studio
 *                  All rights reserved.
 *
 *       Filename:  f.c
 *    Description:  The temperature real-time reporting 
 *                 
 *        Version:  1.0.0(03/29/2020)
 *         Author:  Lu Xiaoyang <920916829@qq.com>
 *      ChangeLog:  1, Release initial version on "03/29/2020 03:10:02 PM"
 *                 
 ********************************************************************************/
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <syslog.h>
#include "sqlite3.h"
#include "socket.h"
#include "tlv.h"
#include "crc.h"
#include "temp_time_get.h"

#define T_OUT            2.5

#define ACK              1
#define NAK              2
#define send_type1       1 //表示是直接发送的数据还是从数据库中拿出来发送的数据，若从数据库中拿的数据发送三次失败后，将删除这条记录；
#define send_type2       2


int  g_stop = 0;
int  flag_save_db = 0;
char id[3] = "LXY";



void sig_handle(int signum)
{
    printf("\nProgram exit,the client stop uploading temperature to server\n");
    exit(0);
}

void sig_handle2(int signum)
{
    printf("检测到服务器断开连接\n");

}


static inline void print_helpmsg(char *progname);
int Send_TlvPacket_To_Server(int sockfd,tlv_buf_t *tlv,int time_s,int send_type,sqlite3 *db);


int main(int argc, char *argv[])
{
    int                    i,ch;
    int                    retval,rv;
    int                    connected = 0;
    int                    retry_times = 3;
    int                    sockfd,serv_port;
    int                    daemon_run = 0;
    int                    nRow,nColumn;
    int                    temp_integer,temp_decimal;
    int                    flag_save_db = 0;
    char                  *errmsg; 
    char                  *domain_name;
    char                 **dbResult;
    char                 *serv_ip = NULL;
    char                   pass_back[4] = {0};
    char                   sql[128] = {0};
    char                   cur_time[30];
    char                  *progname;
    sqlite3               *db;
    time_type              cur_t;
    tlv_buf_t              tlv;
    unsigned short         crc16 = 0;
    struct timeval         time_out = {10,0};


    struct option                 opts[] = {
            {"daemon",no_argument,NULL,'d'},
            {"ipaddr",required_argument,NULL,'i'},
            {"hostname",required_argument,NULL,'n'},
            {"port",required_argument,NULL,'p'},
            {"help",no_argument,NULL,'h'},
            {NULL,0,NULL,0}
        };     
               
    progname = basename(argv[0]);

    while((ch = getopt_long(argc,argv,"di:n:p:h",opts,NULL)) != -1)
    {          
        switch(ch)
        {      
            case 'd':
                daemon_run = 1;
                break;
               
            case 'i':
                serv_ip = optarg;
                break;
               
            case 'n':
                domain_name = optarg;
                break;                         
                                               
            case 'p':                          
                serv_port = atoi(optarg);
                break;                         
                                               
            case 'h':                          
                print_helpmsg(progname);       
                break;                         
                                               
            default:                           
                break;                         
                                               
        }                                      
    }

    if(argc < 3)
    {
        print_helpmsg(progname);
        exit(-1);
    }
                                               
    signal(SIGINT,sig_handle); 
    signal(SIGPIPE,sig_handle2);

    if(daemon_run)
    {
        syslog(LOG_NOTICE,"Program '%s' start running at background\n",progname);
        daemon(0,0);
    }

    if((db = Create_Get_Table(sqlite_name)) == NULL)
    {
        printf("Can not use sqlite3\n");
	    syslog(LOG_ERR,"Can not use sqlite3");
	    return -1;
    }


    while(!g_stop)                            
    {
get_temp:
        /* Get temperature from ds18b20 and return the sn(28-xxxxxxxx). */
        if((rv = temperature_get(&temp_integer,&temp_decimal)) < 0)
        {
            syslog(LOG_ERR,"Can not get temperature\n");
            printf("Can not get temperature\n");
            sleep(T_OUT);
            continue;
        }
        /* Get current time */
        bzero(&cur_t,sizeof(cur_t));
	    time_get(&cur_t);
        rv = Tlv_Packet(id,&cur_t,temp_integer,temp_decimal,&tlv);
        if(rv > 0)
        {
            flag_save_db = 1;
        }

        if(connected == 0)
        {
		    printf("The connection has not been established\n");
            if((sockfd = Connect_To_Server(serv_port,serv_ip,domain_name)) < 0)
            {
                goto SAVE_TLV;
            }
            else
            {   
	            printf("To establish the connection is successful\n");
                connected = 1;
            }
        }

        if(connected == 1)
        {
		    printf("Has been in a state of connection......\n");
            rv = Send_TlvPacket_To_Server(sockfd,&tlv,retry_times,send_type1,NULL);
            if(rv < 0)
            {
                printf("Send tlv msg to server faliure\n");
                connected = 0;
                close(sockfd);
                goto SAVE_TLV;
            }

            else if(rv == 0)
            {
                /* Write ok,Don't write data to the database */
                flag_save_db = 0;
            }
            
            for(i = 0; i < 5; i++)
            {
                rv = Get_One_TlvPacket_From_Database(db,&tlv);

                /* Get data failure */
                if(rv == -1)
                {
                    break;
                }
                /* No massage in the database */
                if(rv == 0)
                {
                    break;
                }
                /* Get one massage from database */
                if(rv == 1)
                {
                    retval = Send_TlvPacket_To_Server(sockfd,&tlv,retry_times,send_type2,db);
                    if(retval < 0)
                    {
                        printf("Send msg to server failure:%s\n",strerror(errno));
                        connected = 0;
                        close(sockfd);
                        break;
                    }
                    if(retval == 0)
                    {
                        printf("数据库中的第%d条数据发送成功\n",i+1);
                        if(!Delete_One_TlvPacket_From_Database(db))
                        {
                            printf("删除表中第一条数据成功\n");
                            continue;
                        }
                        printf("表中数据清理失败，请检查程序\n");
                        exit(-1);
                    }
                    
                }
            }

        }    

SAVE_TLV: 
       if(flag_save_db)
       {
           rv = Save_Tlv_To_Database(db,&tlv);
           if(rv < 0)
           {
               syslog(LOG_ERR,"Can't use database!");
               printf("Can't use database!\n");
               return -1;
           }
       }
       sleep(T_OUT);
    }

    return 0;

}


/*******************************************************************************
 * 函数名：Send_TlvPacket_To_Server()
 * 功能：将已打包好的tlv字节流数组发送到服务器端，并等待服务器回发报文判断是否重传
 * 参数： sockfd - 用于通信的socket套接字;
 *        tlv - tlv_buf_t 类型的结构体指针;
 *        retry_times - 重传次数;
 *        send_type - 发送数据的是直接发送的还是从数据库中取出发送的;
 *        db - 要操作的数据库的句柄
 * 返回值：成功返回实际发送的字节数，失败返回-1
 ******************************************************************************/
int Send_TlvPacket_To_Server(int sockfd,tlv_buf_t *tlv,int retry_times,int send_type,sqlite3 *db)
{

    char            back_msg[10];
    int             back_v;
    int             i;
    int             retval = -1;
    fd_set          set;

    /* The TLV buf will send three times at most if resolution failure */
    for(i = 0; i < retry_times; i++)
    {
        struct timeval       time_out = {3,0};

        if(write(sockfd,tlv->buf,tlv->len) <= 0)
        {
            printf("Server connection failure\n");
            return retval;
        }



        FD_ZERO(&set);
        FD_SET(sockfd,&set);
        
        switch(select(6,&set,NULL,NULL,&time_out))
        {
            case -1:
                printf("Select failure:%s\n",strerror(errno));
                return retval;

            case 0:
                printf("Time Out!\n");
                continue;

            default:
                if(FD_ISSET(sockfd,&set))
                {
                    read(sockfd,back_msg,sizeof(back_msg));
                    back_v = atoi(back_msg);
                    if(back_v == ACK)
                    {
                        printf("ACK packet is received,This data sent successfully\n");
                        retval = 0;
                        return retval;
                    }
                    else if(back_v == NAK)
                    {
                        printf("NAK message is received,This data will send up to three times\n");
                        continue;
                    }
                }
        }//switch

    }//for

    if(send_type == send_type2)
    {
        printf("数据库中的发送三次失败,将删除本条数据\n");
        Delete_One_TlvPacket_From_Database(db);
    }
    else if(send_type == send_type1)
    {
        flag_save_db = 0;
    }

}



/*******************************************************************************
 * 函数名：print_helpmsg()
 * 功能：打印程序的帮助信息
 * 参数：progname - 程序名
 * 返回值：无
 ******************************************************************************/
static inline void print_helpmsg(char *progname)
{
    printf("*************************** %s helpmsg ****************************************\n\n",progname);
    printf("        -p(--port):       The port which server listen on.\n\n");
    printf("        -i(--ipaddr):     Server ipaddr.\n\n");
    printf("        -n(--domainname): Connect server by domain name.\n\n");
    printf("        -d(--daemon):     The program will running at background.\n\n");
    printf("        -h(--help):       print help massage.\n\n");
    printf("*******************************************************************************\n");
}


