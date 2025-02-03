/*******************************************************************************
 Copyright (C), 2010-2021  ShenZhen LiYiShan Information Technology Co.,Ltd.
                           www.lyshinfo.com 
 FileName:       test_mouse.c
 Author:         sz.cbwang
 Date:           2013-03-13
 Description:    USB 鼠标测试应用程序源代码。
 Version:        1.0
 Mail:           sz.cbwang@lyshinfo.com          <Any question, pls contact me>
 Function List:
 
 History:
 <author>        <time>        <version >        <desc>
  1. sz.cbwang    2013-03-13    1.0               初始版本。
   
*******************************************************************************/

#include <vxWorks.h>
#include "sys/times.h"


LOCAL int fd0 = -1;
LOCAL int fd1 = -1;



/*******************************************************************************
*
* mouseTest - mouse test function.
*
*
* RETURNS: ERROR if return error.
*/ 

int mouseTest (void)
{
    int  width;
    int  rc;
    int  nBytes;
    
    struct timeval timeout;
    fd_set readFds;
    unsigned char pBuf[64];

    /* 设置等待时间 */
    
    timeout.tv_sec  = 0;
    timeout.tv_usec = 2;

    /* 构造目标设备名 */

    if ((fd0 = open ("/usb2Mse/0", 0, 0)) < 0 &&
        (fd0 = open ("/usbMo/0", 0, 0)) < 0)
    {
        printf ("\t\t [%s:%d]: Failed to open USB Mouse device!\r\n", 
               (int)__FUNCTION__, (int)__LINE__); 
               
        goto err;
    }

    FOREVER
    {
        /* 清除等待设备集合中的读屏蔽位 */
        
        FD_ZERO(&readFds);

        /* 初始化屏蔽位为等待串口状态 */
        
        FD_SET(fd0, &readFds);
        
        width = fd0 + 1;
        
        /* 阻塞, 等待串口设备变为就绪状态 */
        
        rc = select (width, &readFds, NULL, NULL, &timeout);
        if (rc < 0)
        {
            goto err;
        } 

        /* Check if the fd is ready for data? */
        
        if (FD_ISSET(fd0, &readFds))
        {
            if ((nBytes = read (fd0, pBuf, 3)) == ERROR)
            {
                printf ("\t\t [%s:%d]: Error when read device descriptor!\r\n", 
                       (int)__FUNCTION__, (int)__LINE__);
            
                goto err;  
            }
            else if (nBytes == 0)
            {
              printf ("---> 2 \n");
                continue;
            }
            else
            {
                /* TODO: */
                
                printf ("Now mouse delta pos: (x, y)->(%d, %d)\n", pBuf[1], pBuf[2]);
 
                if (pBuf[0] & 4)
                printf ("M on, ");
                else
                printf ("M off, ");

                if (pBuf[0] & 1)
                printf ("L on, ");
                else
                printf ("L off, ");
                
                if (pBuf[0] & 2)
                printf ("R on, ");
                else
                printf ("R off, ");
                
                printf ("\n");
            }
        }
    }
    
err:

    if (fd0 > 0)
    {
        close (fd0);
    }
    
    return (OK);
}
