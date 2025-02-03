/*******************************************************************************
 Copyright (C), 2010-2021  ShenZhen LiYiShan Information Technology Co.,Ltd.
                           www.lyshinfo.com
 FileName:       generic_synctime.c
 Author:         sz.dcma      
 Date:           2012-10-02
 Description:    设置系统时间测试程序。
 Version:        1.0
 Mail:           sz.dcma@lyshinfo.com            <Any question, pls contact me>
 Function List:
  1. void dosFsTmSet (void)

 History:
 <author>        <time>        <version >        <desc>
  1. alex         2013-09-08    1.0               初始版本。
*******************************************************************************/


#include "time.h"







/*******************************************************************************
* 函数名称: dosFsTmSet
* 功能说明: DOS 文件系统时间日期设置函数。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

void dosFsTmSet (void)
{
    struct tm tm ;    /* ANSII time format */
    struct timespec tv ;  /* POSIX time */

    /* convert this to ANSI time */
    tm.tm_sec       = 1;
    tm.tm_min       = 56;
    tm.tm_hour      = 23;
    tm.tm_mday      = 23;
    tm.tm_mon       = 11 - 1;
    tm.tm_year      = 2012 - 1900;
    tm.tm_wday      = 0 ;
    tm.tm_yday      = 0 ;
    tm.tm_isdst     = 0 ;

    /* convert ANSI to POSIX */
    tv.tv_sec = mktime (&tm);
    tv.tv_nsec = 0;

    /* set system time */
    clock_settime (CLOCK_REALTIME, &tv);
}

