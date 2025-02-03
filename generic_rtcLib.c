/*******************************************************************************
 Copyright 2021 (C), ShenZhen LiYiShan Information Technology Co., Ltd.
 FileName:       generic_rtcLib.c
 Author:         sz.cbwang
 Date:           2018-09-26
 Description:    RTC 用户层接口。
 Version:        1.0
 Mail:           sz.cbwang@lyshinfo.com        <Any question, pls contact me>
 Function List:
  
 History:
 <author>        <time>        <version >        <desc>
  1. sz.cbwang    2018-09-26    1.0               Created.

*******************************************************************************/

/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include <../src/hwif/h/util/vxbRtcLib.h>
#include "hwif/vxbus/vxBus.h"
#include "vxBusLib.h"
#include "hwif/vxbus/vxbI2cLib.h"




/* defines */

#define RTC_DEBUG




/* typedefs */

#ifdef RTC_DEBUG
STATUS (*pDevShow) (VXB_DEVICE_ID pDev, int verbose);
STATUS (*pRtcFuncGet) (VXB_DEVICE_ID pInst, 
                       struct vxbRtcFunctionality ** ppRtcFunc);
#endif




/* locals */

VXB_DEVICE_ID                rtcDevId;  /* 驱动层直接调用的方法示例 */
VXB_DEVICE_ID                i2cDevId;  /* 驱动层直接调用的方法示例 */ 
struct vxbRtcFunctionality * pRtcFunc;  /* 驱动层直接调用的方法示例 */




/* globals */




/* declares */




/* functions */

/*******************************************************************************
* 函数名称: sysRtcInput
* 功能说明: 从命令行获取用户输入的字符。
* 调用函数: ...
* 被调函数: ...
* 输入参数: prompt : 输入提示符；
*           min    : 最小值；
*           max    : 最大值。
* 输出参数: rtVal  : 指向输入的字符转换成整数后返回的地址。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

void sysRtcInput (char * prompt, /* input prompt */
                  int  * rtVal,  /* store the value */
                  int    min,    /* min value */
                  int    max     /* max value */
                 )
{
    char inputBuf [10];
    int  value;
    
    memset (inputBuf, 0, sizeof(inputBuf));
   
    do
    {
        printf ("%s", prompt);
        fgets (inputBuf, sizeof (inputBuf), stdin);
        value = strtoul ((char *) inputBuf, NULL, sizeof (inputBuf));

        /* The input will be exit if the value more than 255 or '.' */

        if (value < min || value > max)
        {
            continue;
        }
        else
        {
            * rtVal = value;
            return;
        }
    } while (1);
}


/*******************************************************************************
* 函数名称: sysRtcSet
* 功能说明: 从命令行设置系统当前的日期和时间。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: 各个参数的值域如下所示：
*        struct tm (in time.h - POSIX time header)
*        {
*        int tm_sec;     seconds after the minute     - [0, 59]
*        int tm_min;     minutes after the hour       - [0, 59]
*        int tm_hour;    hours after midnight         - [0, 23]
*        int tm_mday;    day of the month             - [1, 31]
*        int tm_mon;     months since January         - [0, 11]
*        int tm_year;    years since 1900             
*        int tm_wday;    days since Sunday            - [0, 6]
*        int tm_yday;    days since January 1         - [0, 365]
*        int tm_isdst;   Daylight Saving Time flag
*        };
*******************************************************************************/

STATUS sysRtcSet (void)
{
    struct tm       rtcTime;
    struct timespec ts;
    int             ret = ERROR;
    
    bzero ((char *)&rtcTime, sizeof (struct tm));

    /* Prompt for Year */

    sysRtcInput ("Enter the year: ", &rtcTime.tm_year, 100, 200);

    /* Prompt for Month */

    sysRtcInput ("Enter the month: ", &rtcTime.tm_mon, 0, 11);

    /* Prompt for Day of the Month */

    sysRtcInput ("Enter the day of the month: ", &rtcTime.tm_mday, 1, 31);

    /* Prompt for 24-hour clock */

    sysRtcInput ("Enter the hour(0-23): ", &rtcTime.tm_hour, 0, 23);

    /* Prompt for Minute */

    sysRtcInput ("Enter the minute: ", &rtcTime.tm_min, 0, 59);

    /* Prompt for Second */

    sysRtcInput ("Enter the second: ", &rtcTime.tm_sec, 0, 59);

#if 0    
    return (vxbRtcSet(&rtcTime));
#endif

    ret = vxbRtcSet(&rtcTime);
    if (ret == ERROR)
    {
        return ERROR; 
    }
      
    /* 将 RTC 的时间转换成秒 */
    
    ts.tv_sec = mktime (&rtcTime);
    ts.tv_nsec = 0;
    
    ret = clock_settime (CLOCK_REALTIME, &ts);
    
    if (ret == ERROR)
    {
        printf ("clock_settime return ERROR \r\n");
    }
    
    return ret;
}


/*******************************************************************************
* 函数名称: sysRtcGet
* 功能说明: 显示系统当前的日期和时间。
* 调用函数: ...
* 被调函数: ...
* 输入参数: level : 0 简单显示；1 则全显示。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: 显示日期和时间示例如下：
*           Wed SEP 14 11:23:00 2011
*******************************************************************************/

STATUS sysRtcGet (int level)
{
    struct tm rtcTime;
    char   outputBuffer[500]; /* output buffer for returned string */

    if (vxbRtcGet(&rtcTime) != OK)
        return ERROR;

    if (level == 0)
    {
        asctime_r (&rtcTime, outputBuffer);
        printf("%s", outputBuffer);
    }
    else
    {
        strftime (outputBuffer, 500, 
                  "year:         %Y\r\n"\
                  "month:        %B[%m] \r\n"\
                  "day of month: %d \n"\
                  "hour(24h):    %H \r\n"\
                  "minute:       %M \r\n"\
                  "second:       %S \r\n"\
                  "weekday:      %A[%w]\r\n"\
                  "day of year:  %j \r\n"\
                  "week number:  %W \r\n"\
                  "time zone:    %Z\r\n"
                  "date: %x  time: %X\r\n",                
                  &rtcTime);
        printf("%s", outputBuffer);
    }

    return OK;
}


/*******************************************************************************
* 函数名称: usrSyncFsTime
* 功能说明: 用户层接口函数，用于将RTC的时间同步到文件系统。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: OK or ERROR。
* 其它说明: N/A。
*******************************************************************************/

int usrSyncFsTime (void)
{
    int             ret = ERROR;
    struct timespec ts;
    struct tm       rtcTime;
    
    if (vxbRtcGet (&rtcTime) != OK)
    {
        return ERROR;
    }
    
    /* 将 RTC 的时间转换成秒 */
    
    ts.tv_sec = mktime (&rtcTime);
    ts.tv_nsec = 0;
    
    ret = clock_settime (CLOCK_REALTIME, &ts);
    
    if (ret == ERROR)
    {
        printf ("clock_settime return ERROR \r\n");
    }
    
    return ret;
}


/*******************************************************************************
* 函数名称: rtcProbe
* 功能说明: 
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: OK or ERROR。
* 其它说明: N/A。
*******************************************************************************/

void i2cProbe (void)
{
    i2cDevId = vxbInstByNameFind ("fslI2c", 0);
    if (i2cDevId == NULL)
    {
        printf ("Error: can not find the i2c device id \r\n");
        return;
    }
    
    printf ("i2cDevId %p \r\n", i2cDevId);   
    
    pDevShow = vxbDevMethodGet (i2cDevId, DEVMETHOD_CALL (busDevShow));
    if (pDevShow)
    {
        (pDevShow) (i2cDevId, 1000);  
    } 
    
    return ;
}


/*******************************************************************************
* 函数名称: rtcProbe
* 功能说明: 
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: OK or ERROR。
* 其它说明: N/A。
*******************************************************************************/

void rtcProbe (void)
{
    rtcDevId = vxbInstByNameFind ("rtc_pcf8564", 0);
    if (rtcDevId == NULL)
    {
        printf ("Error: can not find the rtc device id \r\n");
        return; 
    }

    printf ("i2cDevId %p \r\n", i2cDevId);   
    
    pDevShow = vxbDevMethodGet (rtcDevId, DEVMETHOD_CALL (busDevShow));
    if (pDevShow)
    {
        (pDevShow) (rtcDevId, 1000);  
    } 
    
    pRtcFuncGet = vxbDevMethodGet (rtcDevId, DEVMETHOD_CALL (vxbRtcFuncGet));
    if (pRtcFuncGet)
    {
        (pRtcFuncGet) (rtcDevId, &pRtcFunc);  
    }
    
    printf ("\r\n Done. \r\n");
    
    return ;
}


/*******************************************************************************
* 函数名称: sysRtcSet2
* 功能说明: 用户层接口函数，设置系统时间。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: OK or ERROR。
* 其它说明: N/A。
*******************************************************************************/

STATUS sysRtcSet2 (void)
{
    struct tm rtcTime;

    bzero ((char *)&rtcTime, sizeof(struct tm));

    /* Prompt for Year */

    sysRtcInput ("Enter the year: ", &rtcTime.tm_year, 100, 200);

    /* Prompt for Month */

    sysRtcInput ("Enter the month: ", &rtcTime.tm_mon, 0, 11);

    /* Prompt for Day of the Month */

    sysRtcInput ("Enter the day of the month: ", &rtcTime.tm_mday, 1, 31);

    /* Prompt for 24-hour clock */

    sysRtcInput ("Enter the hour(0-23): ", &rtcTime.tm_hour, 0, 23);

    /* Prompt for Minute */

    sysRtcInput ("Enter the minute: ", &rtcTime.tm_min, 0, 59);

    /* Prompt for Second */

    sysRtcInput ("Enter the second: ", &rtcTime.tm_sec, 0, 59);

    return (pRtcFunc->rtcSet(rtcDevId, &rtcTime));
}


/*******************************************************************************
* 函数名称: sysRtcGet2
* 功能说明: 显示系统当前的日期和时间。
* 调用函数: ...
* 被调函数: ...
* 输入参数: level : 0 简单显示；1 则全显示。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: 显示日期和时间示例如下：
*           Wed SEP 14 11:23:00 2011
*******************************************************************************/

STATUS sysRtcGet2 (int level)
{
    struct tm rtcTime;
    char      outputBuffer [500]; /* output buffer for returned string */

    if (pRtcFunc->rtcGet(rtcDevId, &rtcTime) != OK)
    {
        return ERROR;
    }
    
    if (level == 0)
    {
        asctime_r (&rtcTime, outputBuffer);
        printf("%s", outputBuffer);
    }
    else
    {
        strftime (outputBuffer, 500, 
                  "year:         %Y\r\n"\
                  "month:        %B[%m] \r\n"\
                  "day of month: %d \n"\
                  "hour(24h):    %H \r\n"\
                  "minute:       %M \r\n"\
                  "second:       %S \r\n"\
                  "weekday:      %A[%w]\r\n"\
                  "day of year:  %j \r\n"\
                  "week number:  %W \r\n"\
                  "time zone:    %Z\r\n"
                  "date: %x  time: %X\r\n",                
                  &rtcTime);
        printf("%s", outputBuffer);
    }

    return OK;
}
