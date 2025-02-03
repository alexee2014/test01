/*******************************************************************************
 Copyright 2021 (C), ShenZhen LiYiShan Information Technology Co., Ltd.
 FileName:       generic_pwmLib.c
 Author:         sz.dcma
 Date:           2018-10-11
 Description:    PWM测试源文件。
 Version:        1.0
 Mail:           sz.dcma@lyshinfo.com          <Any question, pls contact me> 
 Function List:
                                                   
 History:
 <author>        <time>        <version >        <desc>
  1. sz.dcma      2018-10-11    1.0               初始版本。                                                
*******************************************************************************/

/* includes */

#include "selectlib.h"
#include "vxWorks.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "ioLib.h"
#include "iosLib.h"
#include "memLib.h"
#include "errnoLib.h"
#include "string.h"
#include "sys/stat.h"
#include "../drv/pwm/pwmIoDrv.h"




/* defines */

#define PWM_DEBUG
#define PWM_DEV_NAME      "/pwm"




/* typedefs */




/* locals */




/* globals */

int pwmFreq = 0;
int pwmDuty = 0;




/* declares */

/*******************************************************************************
* 函数名称: pwmLoadCfgParams
* 功能说明: 加载 PWM 配置参数。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

int pwmLoadCfgParams (void)
{
    FILE *      fp = NULL;
    char *      path = NULL;
    char *      defaultPath = "/ata0a/pwmCfgParms";
    struct stat sbuf;
    
    if ((path = getenv ("PWM_CFG_PARAMS_FILE")) == NULL)
    {
        path = defaultPath;
    }
    
    if (stat (path, &sbuf) == ERROR)
    {
#ifdef PWM_DEBUG
        printf ("dbg> file \'%s\' not exist \r\n", path);
#endif      
        return ERROR; 
    }
      
    fp = fopen (path, "r");
    if (fp == NULL)
    {
#ifdef PWM_DEBUG
        printf ("dbg> open file \'%s\' failed  \r\n", path);
#endif      
        return ERROR; 
    }
    
    fscanf (fp, "%d %d", &pwmFreq, &pwmDuty);
    
#ifdef PWM_DEBUG
    printf ("dbg> freq %d, duty %d \r\n", pwmFreq, pwmDuty);
#endif
    
    fclose (fp);
    
    return OK;
}


/*******************************************************************************
* 函数名称: pwmIoDrvTest
* 功能说明: 驱动测试程序入口。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

void pwmIoDrvTest (void)
{
    int       devFd = 0;
    CMD_DESC  desc;
    
    pwmLoadCfgParams ();
    
#ifdef PWM_DEBUG
    printf ("dbg> load config params done \r\n");
#endif

    devFd = open (PWM_DEV_NAME, O_RDWR, 0);
    if (devFd == ERROR)
    {
#ifdef PWM_DEBUG
        printf ("dbg> open device \'%s\' failed  \r\n", PWM_DEV_NAME);
#endif
        return;
    }

    /* Config freq */
    
    desc.chan = 0;
    desc.val = pwmFreq;
    if (ioctl (devFd, PWM_CMD_SET_FREQ, (int)&desc) == ERROR)
    {
        close (devFd);
        return ERROR; 
    }
      
    taskDelay (10);
    
    /* Config duty ratio */
    
    desc.chan = 0;
    desc.val = pwmDuty;
    
    if (ioctl (devFd, PWM_CMD_SET_DUTY, (int)&desc) == ERROR)
    {
        close (devFd);
        return ERROR;
    }
      
    taskDelay (10);

    /* Enable PWM output */

    desc.chan = 0;
    desc.val = 0;
    ioctl (devFd, PWM_CMD_START, (int)&desc);

    /* Waiting for quit 'q' key input */

    while (1)
    {
        char iBuf [16];
        if (fgetc (stdin) == 'q')
        {
            break;
        }
    }
    
    close (devFd);
    
    return OK;
}


