/*******************************************************************************
 Copyright 2021 (C), ShenZhen LiYiShan Information Technology Co., Ltd.
 FileName:       generic_canLib.c
 Author:         sz.cbwang      
 Date:           2018-10-06
 Description:    FLASH 底层用户层 API 实现源文件。
 Version:        1.0
 Mail:           sz.cbwang@lyshinfo.com          <Any question, pls contact me> 
 Function List:

 History:
 <author>        <time>        <version >        <desc>
  1. sz.cbwang    2018-10-06    1.0               初始版本。
                                             
*******************************************************************************/


/* includes */

#include "vxWorks.h"
#include "vsbConfig.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/fcntlcom.h"
#include "../src/drv/can/flexcanIoDrv.h"




/* defines */




/* defines */




/* typedefs */

typedef struct can_frame {
    UINT32    can_id;     /* ID, ext, rtr */
    UINT8     can_dlc;    /* Data length code (0 to 8 bytes) */
    UINT8     data[8];    /* Data field */
} CAN_MSG;




/* locals */




/* globals */




/* declares */




/* functions */

/*******************************************************************************
* 函数名称: cansend
* 功能说明: FLEXCAN 手动发送CAN消息函数。
* 调用函数: ...
* 被调函数: ...
* 输入参数: canNo   : CAN 通道号，取值 0 ~ 1；
*           bitrate : 波特率参数，请参考驱动中支持的波特率表；
*           num     : 发送的示例CAN消息的次数。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: 使用前，请确保驱动已经成功加载，如未加载，请参考如下方法：
*           -> flexCANIoDrv
*           -> flexCANDevCreate "/flexcan/0"
*           -> flexCANDevCreate "/flexcan/1"
*
*******************************************************************************/

int cansend (int canNo, int bitrate, int num)
{
    int     fd = 0;
    int     params;
    int     ret = 0;
    char    devName [16];
    int     cntr = 0;
    CAN_MSG msg = {0x0123, 0x8, {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7}};
        
    if (canNo > 1 || bitrate == 0 || num > 65535)
    {
        return -1;  
    }

    sprintf (devName, "%s/%d", "/flexcan", canNo);
    fd = open (devName, O_RDWR, 0);
    if (fd < 0)
    {
        printf ("Failed to open can device \r\n");
        return -1;
    }

    params = bitrate;

    ret = ioctl (fd, FLEXCAN_CMD_SET_BITTINGS, &params);
    if (ret == ERROR)
    {
        printf ("Failed to set bitrate %d \r\n", params);
        close (fd);
        return ERROR;
    }
    
    for (;;)
    {
        if (write (fd, &msg, sizeof (CAN_MSG)) <= 0)
        {
            printf ("Failed to send data \r\n");
            close (fd);
            return -1;
        }
        
        /*taskDelay (1);*/
        
        cntr++;
        if (cntr >= num)
        {
            break;  
        }
    }

    close (fd);

    return 0;
}


/*******************************************************************************
* 函数名称: canrecv
* 功能说明: CAN 接收数据程序。
* 调用函数: ...
* 被调函数: ...
* 输入参数: canNo   : CAN 通道号，取值 0 ~ 1。
*           bitrate : 波特率参数，请参考驱动中支持的波特率表。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int canrecv (int canNo, int bitrate)
{
    int     fd = 0;
    int     params;
    int     ret = 0;
    int     totalBytes = 0;
    char    devName [16];
    CAN_MSG msg;
    
    if (canNo > 1 || bitrate == 0)
    {
        return -1;  
    }

    sprintf (devName, "%s/%d", "/flexcan", canNo);
    
    fd = open (devName, O_RDWR, 0);
    if (fd < 0)
    {
        printf ("Failed to open can device \r\n");
        return -1;
    }

    params = bitrate;

    /* Set bitrate */
    
    ret = ioctl (fd, FLEXCAN_CMD_SET_BITTINGS, &params);
    if (ret == ERROR)
    {
        printf ("Failed to set bitrate %d \r\n", params);
        close (fd);
        return ERROR;
    }
    
    /* Clear buffer */
    
    ret = ioctl (fd, FLEXCAN_CMD_FLUSH, 0);
    if (ret == ERROR)
    {
        printf ("Failed to set bitrate %d \r\n", params);
        close (fd);
        return ERROR;
    }
    
    FOREVER
    {
        if ((ret = read (fd, &msg, sizeof (CAN_MSG))) == ERROR)
        {
            printf ("\t\t [%s:%d]: Error when read device descriptor!\r\n", 
                   (int)__FUNCTION__, (int)__LINE__);
            goto err;  
        }
        
        totalBytes += msg.can_dlc;
        
        printf ("Received data bytes ......[%8d]         \r", totalBytes);  
    }
    
err:

    if (fd > 0)
    {
        close (fd);
    }

    return (OK);
}


/*******************************************************************************
* 函数名称: canRecvTask
* 功能说明: CAN接收数据任务测试用例。
* 调用函数: ...
* 被调函数: ...
* 输入参数: canNo   : CAN 通道号，取值 0 ~ 1。
*           bitrate : 波特率参数，请参考驱动中支持的波特率表。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

void canRecvTask (int canNo, int bitrate)
{
    int  taskID;
    char tskName [16];
    
    sprintf (tskName, "tCan%dRx", canNo);

    taskID = taskNameToId (tskName);

    if (taskID > 0)
    {
        taskDelete (taskID);
    }

    /* 创建接收任务 */

    taskSpawn (tskName, 110, 0, 4000, (FUNCPTR) canrecv, canNo, bitrate, 
               0, 0, 0, 0, 0, 0, 0, 0);
}


/*******************************************************************************
* 函数名称: canTxTask
* 功能说明: FLEXCAN 发送任务函数。
* 调用函数: ...
* 被调函数: ...
* 输入参数: canNo   : CAN 通道号，取值 0 ~ 1；
*           bitrate : 波特率参数，请参考驱动中支持的波特率表；
*           num     : 发送的示例CAN消息的次数。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: 使用前，请确保驱动已经成功加载，如未加载，请参考如下方法：
*           -> flexCANIoDrv
*           -> flexCANDevCreate "/flexcan/0"
*           -> flexCANDevCreate "/flexcan/1"
*
*******************************************************************************/

int canTxTask (int bitrate)
{
    int     fd = 0;
    int     params;
    int     ret = 0;
    int     tick_s = 0;
    CAN_MSG msg = {0x0123, 0x8, {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7}};

    if (bitrate == 0)
    {
        return -1;  
    }
    
    fd = open ("/flexcan/0", O_RDWR, 0);
    if (fd < 0)
    {
        printf ("Failed to open can device \r\n");
        return -1;
    }

    params = bitrate;

    ret = ioctl (fd, FLEXCAN_CMD_SET_BITTINGS, &params);
    if (ret == ERROR)
    {
        printf ("Failed to set bitrate %d \r\n", params);
        close (fd);
        return ERROR;
    }
    
    for (;;)
    {

    }

    close (fd);

    return 0;     
}

