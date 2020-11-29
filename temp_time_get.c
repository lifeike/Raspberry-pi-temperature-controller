/*********************************************************************************
 *       Filename:  temprature_get.c
 *    Description:  This file is get temperature from RPi
 *                 
 *        Version:  1.0.0(03/30/2020)
 *      ChangeLog:  1, Release initial version on "03/30/2020 10:15:30 AM"
 *                 
 ********************************************************************************/

#include "temp_time_get.h"

int temperature_get(int *temp_integer,int *temp_decimal)
{
    char             *chip = NULL;
    char             path[128] = "/sys/bus/w1/devices/";
    char            *ptr = NULL;
    DIR             *dirp;
    struct dirent   *direntp;
    int              fd = -1;
    int              rv = -1;
    int              found = 0;
    char             buf[1024] = {0};
    float            temp = 0;

    if(!temp_integer && !temp_decimal)
    {
        printf("Invalid input argument.\n");
        return -1;
    }

    if((dirp = opendir(path)) == 0)
    {
        printf("Opendir %s failure,please check your path.\n",path);
        return -1;
    }


    while((direntp = readdir(dirp)) != NULL)
    {
        if(strstr(direntp->d_name,"28-"))
        {
            chip = direntp->d_name;
            found = 1;
            break;
        }
    }

    closedir(dirp);

    if(!found)
    {
        printf("Can not find \"28-\" \n");
        return -1;
    }

    strncat(path,chip,sizeof(path) - strlen(path));
    strncat(path,"/w1_slave",sizeof(path) - strlen(path));

    if((fd = open(path,O_RDONLY)) < 0)
    {
        printf("Can't open %s:%s\n",path,strerror(errno));
        return -1;
    }

    if((rv = read(fd,buf,sizeof(buf))) < 0)
    {
        printf("Read failure:%s\n",strerror(errno));
        return -2;
    }

    close(fd);

    if(!(ptr = strstr(buf,"t=")))
    {
        printf("Can not find \"t=\"\n");
        return -3;
    }

    ptr += 2;

    temp = atof(ptr)/1000.0;

    *temp_integer = (int)temp;
    *temp_decimal = (int)((temp - (float)*temp_integer) * 100);

    return 1;
   
}

void time_get(time_type *cur_t)
{

    struct tm  *ptm;
    long       ts;
    ts = time(NULL);
    ptm = localtime(&ts);

    cur_t->year     =   ptm -> tm_year;
    cur_t->month    =   ptm-> tm_mon+1;
    cur_t->day      =   ptm-> tm_mday;
    cur_t->hour     =   ptm-> tm_hour;
    cur_t->minute   =   ptm-> tm_min;
    cur_t->second   =   ptm-> tm_sec;


}

