/*******************************************************************************
 Copyright 2021 (C), ShenZhen LiYiShan Information Technology Co., Ltd.
 FileName:       generic_flashLib.c
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
#include "hwif/vxbus/vxBus.h"
#include "vxBusLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"




/* defines */

#define K(n)               (1024*n)
#define M(n)               (1024*1024*n)
#define FLASH_TEST_BASE    0x100000




/* defines */




/* typedefs */




/* locals */

LOCAL char buff [128];




/* globals */




/* declares */

extern STATUS sysSpiFlashWrite (UINT32 addr, UINT32 len, const void * buf);
extern STATUS sysSpiFlashRead  (UINT32 addr, UINT32 len, const void * buf);




/* functions */

/*******************************************************************************
* 函数名称: ftest
* 功能说明: SPI FLASH 测试用例。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

void ftest (void)
{
    int i = 0;
    int ret = 0;
    
    srand (time(NULL));
    
    /* 1) Read flash from 0x100000, and print */
    
    printf (" 1] Read target flash area [offset 0x%x, len %d] \r\n", 
            FLASH_TEST_BASE, 128);
        
    ret = sysSpiFlashRead (FLASH_TEST_BASE, 128, buff);
    if (ret == ERROR)
    {
        printf ("sysSpiFlashRead: return ERROR \r\n");
        return ;  
    }
    
    for (i = 0; i < 128; i++)
    {
        printf ("%02x ", buff [i]);
        if ((i+1)%16 == 0)
        {
            printf ("\r\n");
        }
        
        if (i == 128)
        {
            printf ("\r\n");  
        }
    }
    
    
    /* 2) Erase flash */
    
    printf (" 2] Erase target flash area [offset 0x%x, len %d] \r\n", 
            FLASH_TEST_BASE, 4096); 
        
    ret = sysSpiFlashErase (FLASH_TEST_BASE, 4096);
    if (ret == ERROR)
    {
        printf ("sysSpiFlashErase: return ERROR \r\n");
        return ;  
    }   

    /* 3) Check flash  */
    
    printf (" 3] Check target flash area [offset 0x%x, len %d] \r\n", 
            FLASH_TEST_BASE, 128); 
    
    ret = sysSpiFlashRead (FLASH_TEST_BASE, 128, buff);
    if (ret == ERROR)
    {
        printf ("sysSpiFlashRead: return ERROR \r\n");
        return ;  
    }   
    
    for (i = 0; i < 128; i++)
    {
        printf ("%02x ", buff [i]);
        if ((i+1)%16 == 0)
        {
            printf ("\r\n");  
        }
        
        if (i == 128)
        {
            printf ("\r\n");  
        }        
    }


    /* 4) Make new data  */
    
    printf (" 4] Make template data> \r\n");

    for (i = 0; i < 128; i++)
    {
        buff [i] = rand() % 100;
    }
    
    for (i = 0; i < 128; i++)
    {
        printf ("%02x ", buff [i]);
        if ((i+1)%16 == 0)
        {
            printf ("\r\n");
        }
        
        if (i == 128)
        {
            printf ("\r\n");  
        }
    }

        
    /* 5) Write template data to flash */
    
    printf (" 5] Write target flash area [offset 0x%x, len %d] \r\n", 
            FLASH_TEST_BASE, 128); 
    
    ret = sysSpiFlashWrite (FLASH_TEST_BASE, 128, buff);
    if (ret == ERROR)
    {
        printf ("sysSpiFlashWrite: return ERROR \r\n");
        return ;  
    }
    
    memset (buff, 0, sizeof (buff));
    
    /* 6) Check flash */
    
    printf (" 6] Check target flash area [offset 0x%x, len %d] \r\n", 
            FLASH_TEST_BASE, 128); 
    
    ret = sysSpiFlashRead (FLASH_TEST_BASE, 128, buff);
    if (ret == ERROR)
    {
        printf ("sysSpiFlashRead: return ERROR \r\n");
        return ;  
    }   
    
    for (i = 0; i < 128; i++)
    {
        printf ("%02x ", buff [i]);
        if ((i+1)%16 == 0)
        {
            printf ("\r\n");  
        }
        
        if (i == 128)
        {
            printf ("\r\n");  
        }        
    }
    
    printf (" 7] test done. \r\n");
}