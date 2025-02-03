/*******************************************************************************
/*******************************************************************************
 Copyright (C), 2010-2021  ShenZhen LiYiShan Information Technology Co.,Ltd.
                           www.lyshinfo.com 
 FileName:       test_udev.c
 Author:         sz.cbwang
 Date:           2013-03-13
 Description:    U盘测试应用程序源代码。
 Version:        1.0
 Mail:           sz.cbwang@lyshinfo.com          <Any question, pls contact me>
 Function List:
  1. int UpScan (char *UP_NameRet)
  2. int UpScan_IsIn (char *UP_Name)
  3. int CopyFileOne_To_Up (char *UpDrv, char *FileName)
  4. void CopyFile_To_Up (void)
  5. void MainTask (void)

 History:
 <author>        <time>        <version >        <desc>
  1. sz.cbwang    2013-03-13    1.0               初始版本。
*******************************************************************************/

#include <iosLib.h>
#include <dirent.h>
#include <stat.h>

char UP_Name[256];

#define DEFAULT_DEV_NAME    "/bd"
#define NULL_DEV_NAME       ""





/*******************************************************************************
* 函数名称: UpScan
* 功能说明: 扫描 U 盘是否有插入，并获取当前的设备名。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: UP_NameRet: U盘设备名。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int UpScan (char *UP_NameRet)
{
    int     i;
    int     ret;
    char    Str[64];
    DEV_HDR *Dev;
    char    Name[64];
    char    *pNameTail;

    /* BSP设置时只有两个 */
    
    for (i = 0; i < 10; i++)
    {
        sprintf (Name, "%s%d", DEFAULT_DEV_NAME, i);
        
        Dev = iosDevFind (Name, &pNameTail);
        if (Dev == NULL)
        {
          
        }
        else
        {
            /* 是否有U盘插入---只处理一个U盘 */
            
            if (strcmp (Dev->name, Name)==0)
            {
                strcpy (UP_NameRet, Name);
                
                return OK;
            }
        }
     }

     strcpy (UP_NameRet, NULL_DEV_NAME);
     
     return ERROR;
}


/*******************************************************************************
* 函数名称: UpScan_IsIn
* 功能说明: 扫描 U 盘是否有挂载到系统。
* 调用函数: ...
* 被调函数: ...
* 输入参数: UP_Name  : U盘设备名。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int UpScan_IsIn (char *UP_Name)
{
    int     ret;
    DEV_HDR *Dev;
    char    *pNameTail;

    Dev = iosDevFind (UP_Name, &pNameTail);
    if (Dev != NULL)
    {
        if (strcmp (Dev->name, UP_Name)==0)
        
        return OK;
    }
    
    return ERROR;
}


/*******************************************************************************
* 函数名称: CopyFileOne_To_Up
* 功能说明: 拷贝单个文件到U盘指定的目录下。
* 调用函数: ...
* 被调函数: ...
* 输入参数: UpDrv   : U盘设备名。
*           FileName: 待拷贝的文件名。
* 输出参数: N/A。
* 函数返回: 成功返回 OK，否则返回 ERROR。
* 其它说明: N/A。
*******************************************************************************/

int CopyFileOne_To_Up (char *UpDrv, char *FileName)
{
    int    i, RetIn;
    DIR    *pDir;
    char   DirName[256];
    char   Log_Dir_AllFile[256];
    STATUS Ret;
    int    lockKey;

    /* 是否有相应的目录 */

    strcpy (DirName, UpDrv);
    strcat (DirName, "/Log/");
    pDir = opendir (DirName);
    if (pDir == NULL)
    {
        mkdir (DirName);
    }
    else
    {
        closedir (pDir);
    }

    sprintf (Log_Dir_AllFile, "/ata0a/Log/%s", FileName);
    sprintf (DirName, "%s/Log/%s", UpDrv, FileName);
    printf ("copy %s to %s\n", Log_Dir_AllFile, DirName);
    Ret = copy (Log_Dir_AllFile, DirName);
    if (Ret == ERROR)
    {
        printf ("拷贝文件%s出错\n可能是U盘满或文件出错\n");
        return ERROR;
    }

    return OK;
}


/*******************************************************************************
* 函数名称: CopyFile_To_Up
* 功能说明: 从指定的源盘路径下拷贝所有文件到U盘相应的目录下。
* 调用函数: ...
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

void CopyFile_To_Up (void)
{
    int            ret;
    int            i, k;
    DIR           *pDir;
    char           path_name[256];
    struct dirent *FileName;
    struct stat    FileStat;
    char           CopyMode = 0;   /* 0提问，1：全部覆盖 */
    short          temp;

    ret = UpScan (UP_Name);
    if (ret != OK) 
    {
        return;
    }

    strcpy(path_name, "/ata0a");
    strcat(path_name, "/Log/");
    pDir = opendir (path_name);
    if (pDir == NULL)
    {
        printf ("%s dir error\n", path_name);
        mkdir (path_name);
        return;
    }

    printf ("开始新的拷贝 %s To %s\n",path_name,UP_Name);
    
    while ((FileName = readdir(pDir)) != NULL)
    {
        taskDelay (10);
        ret = UpScan_IsIn (UP_Name);
        if (ret != OK)
        { 
            closedir (pDir);
            printf ("U盘退出,不能拷贝!请拔出U盘重新插入再拷贝\n");
            return;
        }
        
        strcpy ((char *)path_name, "/ata0a");
        strcat ((char *)path_name, "/Log/");
        strcat ((char *)path_name, FileName->d_name);
        
        if (stat ((char *)path_name, &FileStat) == ERROR)
        {
            continue;
        }
        
        temp = (FileStat.st_mode) & S_IFMT;
        if (temp != S_IFDIR)  /* Filter file folder */
        {
            if ((!strstr ((char*)(FileName), ".Log"))
                 && (!strstr ((char*)(FileName), ".log"))
                 && (!strstr ((char*)(FileName), ".mht"))) 
            {
                continue;     /* Filter log type file */
            }
            
            if (strcmp ((char*)(FileName), "AUTO-SAVE.Log") == 0) 
            {
                continue;
            }
            
            if (strcmp ((char*)(FileName), "LogList.lst") == 0) 
            {
                continue;
            }
                
            ret = CopyFileOne_To_Up (UP_Name, (char*)(FileName));
            if (ret != OK)
            {
                printf ("Error copy \n");
                closedir (pDir);
                
                return;
            }
        }
    }

    closedir (pDir);
}


/*******************************************************************************
* 函数名称: MainTask
* 功能说明: 拷贝文件到U盘任务实现。
* 调用函数: taskDelay, CopyFile_To_Up
* 被调函数: ...
* 输入参数: N/A。
* 输出参数: N/A。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

void MainTask (void)
{
    for (;;)
    {
        taskDelay (sysClkRateGet() * 1);
        CopyFile_To_Up();
    }
}

