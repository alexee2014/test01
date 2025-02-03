/*******************************************************************************
 Copyright 2021 (C), ShenZhen LiYiShan Information Technology Co., Ltd.
 FileName:       generic_diskFsLib.c
 Author:         sz.dcma        
 Date:           2016-06-06
 Description:    通用磁盘文件系统测试简单用例源文件。
 Version:        1.0
 Mail:           sz.dcma@lyshinfo.com          <Any question, pls contact me> 
 Function List:
  1. int diskWriteFile (char *fileName, unsigned int mageByte)
  2. void taskWrFile (char *fileName, unsigned int mageByte)
                                                                         
 History:
 <author>        <time>        <version >        <desc>
  1. sz.dcma      2016-06-06    1.0               初始版本。
  2. sz.zhouhui   2017-04-09    1.1               Fixed some bugs。
                                
*******************************************************************************/


/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <sysLib.h>
#include <assert.h>
#include <tickLib.h>
#include <taskLib.h>
#include "dosFsLib.h"




/* defines */

#define WR_FILE_NAME    ("write_file.dat")
#define TEMPLATE_FILE   ("test.dat")
#define BILLION         (1000000000) /* 1000 million nanoseconds / second */

#define KB               (1024)
#define BLK_SIZE_KB(n)   (n * KB)
#define FILE_SIZE_MB(n)  (n * KB * KB)




/* typedefs */




/* locals */




/* globals */




/* declares */




/* functions */

/*******************************************************************************
* 函数名称: diskWriteFile
* 功能说明: 磁盘写文件函数。
* 调用函数: ...
* 被调函数: ...
* 输入参数: fn  : 测试文件名。
*           fs  : 文件大小，单位是 MB。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int diskWriteFile (char * fn, unsigned int fs)
{
    int     i       = 0;
    int     blkNum  = 0;
    int     blkSize = BLK_SIZE_KB(512);

    char  * pBuf = NULL;
    FILE  * fp = NULL;

    if (fn == NULL)
    {
        fn = TEMPLATE_FILE;
    }
    
    if ((fp = fopen (fn, "wb")) == NULL)
    {
        printf ("\t\t [%s:%d]: Failed to create test file!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);

        goto __err;
    }

    if ((pBuf = malloc (blkSize)) == NULL)
    {
        fclose (fp);
        
        printf ("\t\t [%s:%d]: Failed to allocate memory!\r\n", 
                (int)__FUNCTION__, (int)__LINE__);

        goto __err;
    }

    /* Fill with the pad */
    
    memset (pBuf, 0x55, blkSize);
    
    blkNum = FILE_SIZE_MB(fs) / blkSize;    
        
    for (i = 0; i < blkNum; i++)
    {
        if (fwrite (pBuf, blkSize, 1, fp) < 0)
        {
            printf ("\t\t [%s:%d]: Failed to write block [%d]\r\n", 
                   (int)__FUNCTION__, (int)__LINE__, i);
                
            goto __err;
        }
    }

    printf (">>>>>>>>>> done. <<<<<<<<<< \n");

    fclose (fp);
    free (pBuf);

    return (OK);
    
__err:

    if (fp)
    {
        fclose (fp);
    }

    if (pBuf)
    {
        free (pBuf);
    }

    return (ERROR);
}


/*******************************************************************************
* 函数名称: taskWrFile
* 功能说明: 任务级的写文件。
* 调用函数: taskSpawn()。
* 被调函数: N/A。
* 输入参数: fn  : 测试文件名。
*           fs  : 文件大小，单位是 MB。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

void taskWrFile (char * fn, unsigned int fs)
{
    /* 任务写文件 */
        
    taskSpawn ("tWrFile", 100, 0, 4000, (FUNCPTR)diskWriteFile, fn, fs, 
               0, 0, 0, 0, 0, 0, 0, 0);
}

