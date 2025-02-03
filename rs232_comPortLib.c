/*******************************************************************************
 Copyright 2021 (C), ShenZhen LiYiShan Information Technology Co., Ltd.
 FileName:       rs232_comPortLib.c
 Author:         sz.dcma
 Date:           2012-10-02
 Description:    串口测试用例。
 Version:        1.1
 Mail:           sz.dcma@lyshinfo.com          <Any question, pls contact me>
 Function List:
  1. int comsend (int ttynum, int rate, int num);
  2. static int comrecv (int ttynum, int rate);
  3. void recvTask (int ttynum, int rate);

 History:
 <author>        <time>        <version >        <desc>
  1. sz.dcma      2012-10-02    1.0               Added。
  2. sz.dcma      2018-01-22    1.1               Cleaned the code。

*******************************************************************************/

/* includes */

#include "vxworks.h"
#include "stdio.h"
#include "taskLib.h"
#include "ioLib.h"
#include "string.h"
#include <end.h>
#include <in.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <semLib.h>
#include <sigLib.h>
#include "sioLib.h"
#include "sys/times.h"




/* defines */

#define MAX_IN_DATA           250
#define MAX_SEND_NUMS         10000000


/*
 * 配置硬件属性,说明如下:
 * IMX6 处理器不支持 NS1655X 规范,因此,数据位,停止位,奇偶校验位数等
 * 和普通的串口不兼容.
 *
 * 数据位  : 7b or 8b
 * 停止位  : 1b or 2b
 * 奇偶校验: 无, 奇, 偶 三种
 * 
 * 如下宏定义,是标准的应用程序支持的宏,注意每一个参数属性都对应自己的bit序列
 * 
 * 一般情况下, 对上述三个属性配置如下:
 * 数据位  : opts | CS7,   opts | CS8 (只能选择 CS7 或者 CS8)
 * 停止位  : opts | STOPB 对应的操作是 2b, 否则是1b
 * 奇偶校验: opts | (PARENB | PARODD), 对应的操作是 ODD 校验使能;
 *           opts | (PARENB) 对应的操作是 EVEN 校验使能;
 *           否则 ,无 
 */

#define CLOCAL                0x1   /* ignore modem status lines */
#define CREAD                 0x2   /* enable device reciever */

#define CSIZE                 0xc   /* bits 3 and 4 encode the character size */
#define CS5                   0x0   /* 5 bits */
#define CS6                   0x4   /* 6 bits */
#define CS7                   0x8   /* 7 bits */
#define CS8                   0xc   /* 8 bits */

#define HUPCL                 0x10  /* hang up on last close */
#define STOPB                 0x20  /* send two stop bits (else one) */
#define PARENB                0x40  /* parity detection enabled (else disabled) */
#define PARODD                0x80  /* odd parity  (else even) */





/* typedefs */




/* locals */

LOCAL char buf_send [] = "abcdefghijklmnopqrstuvwxyz"; 




/* globals */




/* declares */




/* functions */

/*******************************************************************************
* 函数名称: comsend
* 功能说明: 用于串口发送的程序。
* 调用函数: ...
* 被调函数: ...
* 输入参数: ttynum  : 串口号，从数字1开始。
*           rate    : 待配置的波特率参数。
*           num     : 发送固定字符串的次数。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int comsend (int ttynum, int rate, int num)
{
    int  fd;
    int  bytes_out;
    char devs [8] = {0};
    char buffer [128] = {0};
    int  numSend = 1;

    /* 输入参数安全性检查 */

    assert ((num > 0) && (ttynum > 0) && (rate > 0));

    if (num > MAX_SEND_NUMS)
    {
        printf ("Params [num] should be less than %d!\r\n", MAX_SEND_NUMS);

        return (ERROR);
    }

    /* 构造目标设备名，序号从 0 开始 */

    sprintf (devs, "/tyCo/%d", ttynum - 1);

    if ((fd = open (devs, O_RDWR, 0)) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to open device %s!\r\n", 
                (int)__FUNCTION__, (int)__LINE__, devs);

        return (ERROR);
    }

    /* 设置串口的数据模式 */
     
    if (ioctl (fd, FIOSETOPTIONS, OPT_RAW) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to set options!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);
                
        goto err;
    }

    /* 设置串口的波特率 */

    if (ioctl (fd, FIOBAUDRATE, rate) < 0)                  
    {
        printf ("\t\t [%s:%d]: Failed to set baudrate!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);
                
        goto err;
    }

#if 0
    /* 设置串口的硬件属性: 7-2-1 */

    if (ioctl (fd, SIO_HW_OPTS_SET, CS7 | STOPB | PARENB | PARODD) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to set hardware options!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);
        goto err;
    }
    else
    {
#ifdef APP_DEBUG
        printf ("Hardware options: %08x \r\n", (CS7 | STOPB | PARENB | PARODD));  
#endif
    } 
#endif
    
    
    ioctl (fd, FIOFLUSH, 0);

    /*printf ("The send string is \"%s\" \r\n", buf_send);*/

    FOREVER
    {
        memset (buffer, 0, sizeof(buffer));
        
        sprintf (buffer, "[%s(%d)]", buf_send, numSend);

        if ((bytes_out = write (fd, buffer, strlen(buffer))) < 0)
        {
            printf ("\t\t [%s:%d]: Error when write data to com port!\r\n", 
                   (int)__FUNCTION__, (int)__LINE__);
                
            goto err;
        }
        
        /*printf ("\r%d", numSend);*/
        printf ("The send string \"%s\" ... [%8d]        \r", buf_send, numSend);
        
        numSend++;

        if (numSend > num)
        {
            printf ("\r\n");
            break;
        }

        taskDelay (1);
    }

    close (fd);
    
    return (OK);
    
err:

    /* 关闭串口 */

    if (fd > 0)
    {
        close (fd);
    }

    return (ERROR);
}     


int gtotalBytes [5] = {0,};

/*******************************************************************************
* 函数名称: comrecv
* 功能说明: 用于串口接收的程序。
* 调用函数: ...
* 被调函数: ...
* 输入参数: ttynum  : 串口号，从数字1开始。
*           rate    : 待配置的波特率参数。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int comrecv (int ttynum, int rate)
{
    int            fd1;  /* 用于接收数据写文件的描述符 */
    int            fd2;  /* 用于打开的串口设备的描述符 */
    int            nBytes;
    char           buf [40];
    char           devs [8];
    int            width;
    struct timeval timeout;
    fd_set         readFds;
    char *         pBuf = NULL;
    int            totalBytes = 0;
    int            rc = 0;

    /* 设置等待时间 */
    
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    /* 输入参数安全性检查 */

    assert ((ttynum > 0) && (rate > 0) && (rate > 0));

    if ((pBuf = malloc (MAX_IN_DATA)) == NULL)
    {
        return (ERROR);
    }
    
    /* 构造目标设备名 */

    sprintf (devs, "/tyCo/%d", ttynum - 1);

    if ((fd2 = open (devs, O_RDWR, 0)) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to open device %s!\r\n", 
               (int)__FUNCTION__, (int)__LINE__, devs); 
        goto err;
    }

    /* 设置串口的数据模式 */
     
    if (ioctl (fd2, FIOSETOPTIONS, OPT_RAW) < 0)            
    {
        printf ("Failed to set options! \n");
        goto err;
    }

    /* 设置串口的硬件属性: 7-2-1 */

#if 0
    if (ioctl (fd2, SIO_HW_OPTS_SET, CS7 | STOPB | PARENB | PARODD) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to set hardware options!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);
        goto err;
    }
    else
    {
#ifdef APP_DEBUG
        printf ("Hardware options: %08x \r\n", (CS7 | STOPB | PARENB | PARODD));  
#endif
    }
#endif
    
    
    /* 设置串口的波特率 */

    if (ioctl (fd2, FIOBAUDRATE, rate) < 0)                  
    {
        printf ("\t\t [%s:%d]: Failed to set baudrate!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);
        goto err;
    }

    FOREVER
    {
        /* 清除等待设备集合中的读屏蔽位 */
        
        FD_ZERO(&readFds);

        /* 初始化屏蔽位为等待串口状态 */
        
        FD_SET(fd2, &readFds);
        
        width = fd2 + 1;
        
        /* 阻塞, 等待串口设备变为就绪状态 */
        
        rc = select (width, &readFds, NULL, NULL, &timeout);
        if (rc == 0)
        {
            continue;  /* Currently, it has no valid fd when timeout */
        }
        else if (rc < 0)
        {
            goto err;
        }
        else
        {
            /* TODO: the valid fd */
        }

        /* Check if the fd is ready for data? */
        
        if (FD_ISSET(fd2, &readFds))
        {
            if ((nBytes = read (fd2, pBuf, MAX_IN_DATA)) == ERROR)
            {
                printf ("\t\t [%s:%d]: Error when read device descriptor!\r\n", 
                       (int)__FUNCTION__, (int)__LINE__);
                goto err;  
            }
            else if (nBytes == 0)
            {
                continue;
            }
            else
            {
                /* TODO: */
            }
            
            totalBytes += nBytes;
            
            printf ("Received data bytes ......[%8d]         \r", totalBytes);
        }
        else
        {
            /* TODO: */
            
            printf ("\t\t [%s:%d]: Com device fd is not set!\r\n", 
                   (int)__FUNCTION__, (int)__LINE__); 
        } 
    }
    
err:

    if (fd2 > 0)
    {
        close (fd2);
    }

    if (pBuf)
    {
        free (pBuf);
    }

    return (OK);
}


/*******************************************************************************
* 函数名称: recvTask
* 功能说明: 用于串口接收的程序。
* 调用函数:
* 被调函数:
* 输入参数: ttynum  : 串口号，从数字1开始。
*           rate    : 待配置的波特率参数。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

void recvTask (int ttynum, int rate)
{
    int taskID;

    taskID = taskNameToId ("recvTask");

    if (taskID > 0)
    {
        taskDelete (taskID);
    }

    /* 创建接收任务 */

    taskSpawn ("recvTask", 200, 0, 4000, (FUNCPTR) comrecv, ttynum, rate, 
           0, 0, 0, 0, 0, 0, 0, 0);
}



/*******************************************************************************
* 函数名称: comrecv
* 功能说明: 用于串口接收的程序。
* 调用函数: ...
* 被调函数: ...
* 输入参数: ttynum  : 串口号，从数字1开始。
*           rate    : 待配置的波特率参数。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int comrecv2 (int ttynum, int rate)
{
    int            fd1;  /* 用于接收数据写文件的描述符 */
    int            fd2;  /* 用于打开的串口设备的描述符 */
    int            nBytes;
    char           buf [40];
    char           devs [8];
    int            width;
    struct timeval timeout;
    fd_set         readFds;
    char *         pBuf = NULL;
    int            totalBytes = 0;
    int            rc = 0;
    FILE *         fp = 0;
    char           fileName [16];
    
    /* 设置等待时间 */
    
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    /* 输入参数安全性检查 */

    assert ((ttynum > 0) && (rate > 0) && (rate > 0));

    if ((pBuf = malloc (MAX_IN_DATA)) == NULL)
    {
        return (ERROR);
    }
    
    /* 构造目标设备名 */

    sprintf (devs, "/tyCo/%d", ttynum - 1);

    if ((fd2 = open (devs, O_RDWR, 0)) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to open device %s!\r\n", 
               (int)__FUNCTION__, (int)__LINE__, devs); 
        goto err;
    }

    /* 设置串口的数据模式 */
     
    if (ioctl (fd2, FIOSETOPTIONS, OPT_RAW) < 0)            
    {
        printf ("Failed to set options! \n");
        goto err;
    }

    /* 设置串口的硬件属性: 7-2-1 */

    if (ioctl (fd2, SIO_HW_OPTS_SET, CS8) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to set hardware options!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);
        goto err;
    }
    else
    {
#ifdef APP_DEBUG
        printf ("Hardware options: %08x \r\n", (CS8));  
#endif
    }

    /* 设置串口的波特率 */

    if (ioctl (fd2, FIOBAUDRATE, rate) < 0)                  
    {
        printf ("\t\t [%s:%d]: Failed to set baudrate!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);
        goto err;
    }
    
    sprintf (fileName, "port%d.txt", ttynum);
    fp = fopen (fileName, "w+");
    if (fp == NULL)
    {
        printf ("Failed to open file \r\n");
        goto err; 
    }
    
    FOREVER
    {
        if ((nBytes = read (fd2, pBuf, MAX_IN_DATA)) == ERROR)
        {
            printf ("\t\t [%s:%d]: Error when read device descriptor!\r\n", 
                   (int)__FUNCTION__, (int)__LINE__);
            goto err;  
        }
        else if (nBytes == 0)
        {
            continue;
        }
        else
        {
            /* TODO: */
        }
        
        gtotalBytes [ttynum - 1] += nBytes;
        
        write (fd2, pBuf, nBytes);
        
        /*printf ("Received data bytes ......[%8d]         \r", totalBytes);*/
        
        if (fwrite (pBuf, 1, nBytes, fp) != nBytes)
        {
            printf ("Write file %s failed \r\n", fileName); 
        }
        
        fflush (fp);
    }
    
err:

    if (fd2 > 0)
    {
        close (fd2);
    }

    if (pBuf)
    {
        free (pBuf);
    }

    if (fp)
    {
        fclose (fp);
    }
    
    return (OK);
}


/*******************************************************************************
* 函数名称: recvTask
* 功能说明: 用于串口接收的程序。
* 调用函数:
* 被调函数:
* 输入参数: ttynum  : 串口号，从数字1开始。
*           rate    : 待配置的波特率参数。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

void recvTask2 (int ttynum, int rate)
{
    int  taskID;
    char tskName [16];
    
    sprintf (tskName, "recv%d", ttynum);

    taskID = taskNameToId (tskName);

    if (taskID > 0)
    {
        taskDelete (taskID);
    }

    /* 创建接收任务 */

    taskSpawn (tskName, 90, 0, 0x100000, (FUNCPTR) comrecv2, ttynum, rate, 
               0, 0, 0, 0, 0, 0, 0, 0);
}



void showRecvInfo (void)
{
    printf ("---------- Show received data ---------- \r\n"
            "COM1        COM2        COM3        COM4\r\n"
            "%-10d  %-10d  %-10d  %-10d\r\n",
            gtotalBytes [0],
            gtotalBytes [1],
            gtotalBytes [3],
            gtotalBytes [4]); 
}

/*
void showRecvInfo2 (void)
{
    int ax [4] = {45550987, 45550987, 45550987, 45550987};
    printf ("---------- Show received data ---------- \r\n"
            "COM1        COM2        COM2        COM2\r\n"
            "%-10d  %-10d  %-10d  %-10d\r\n",
            ax [0],
            ax [1],
            ax [2],
            ax [3]); 
}
*/
