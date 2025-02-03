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

#include "vxWorks.h"




/* defines */

#define post_log            printf 
#define WATCHDOG_RESET()
#define START_ADDR          0x20000000
#define END_ADDR            0x30000000

#define DDR_DIAG_DEBUG
/*#define CLEAR_LINE          printf ("\r                                                    \r");*/
#define CLEAR_LINE

/*
#define QUIT_CMD_CHECK \
        {if (tstc ()) { \
            if (getc () == 0x1b) {return 0;}\
        }} 
*/
#define QUIT_CMD_CHECK \
        {\
            if (getc () == 0x1b) {return 0;}\
        } 



/* typedefs */

typedef unsigned long ulong;




/* globals */




/* locals */

/*
* This is 64 bit wide test patterns. Note that they reside in ROM
* (which presumably works) and the tests write them to RAM which may
* not work.
*
* The "otherpattern" is written to drive the data bus to values other
* than the test pattern. This is for detecting floating bus lines.
*
*/
const static unsigned long long pattern [] = {
    0xaaaaaaaaaaaaaaaaULL,
    0xccccccccccccccccULL,
    0xf0f0f0f0f0f0f0f0ULL,
    0xff00ff00ff00ff00ULL,
    0xffff0000ffff0000ULL,
    0xffffffff00000000ULL,
    0x00000000ffffffffULL,
    0x0000ffff0000ffffULL,
    0x00ff00ff00ff00ffULL,
    0x0f0f0f0f0f0f0f0fULL,
    0x3333333333333333ULL,
    0x5555555555555555ULL
};

static const unsigned long long otherpattern = 0x0123456789abcdefULL;




/* externs */




/* functions */

/*******************************************************************************
* 函数名称: show_ddr_config
* 功能说明: 显示 DDR 的相关配置参数，未实现。
* 调用函数: ...
* 被调函数: ...
* 输入参数: 无。
* 输出参数: 无。
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

static void show_ddr_config (void)
{
    return ;
}


/*******************************************************************************
* 函数名称: move64
* 功能说明: 进行 64 位数据的拷贝。
* 调用函数: ...
* 被调函数: ...
* 输入参数: src  : 源数据
* 输出参数: dest : 目标数据
* 函数返回: N/A。
* 其它说明: N/A。
*******************************************************************************/

static void move64 (unsigned long long * src, unsigned long long * dest)
{
    *dest = *src;
}


/*******************************************************************************
* 函数名称: memory_post_dataline
* 功能说明: 数据线检测
* 调用函数: ...
* 被调函数: ...
* 输入参数: pmem : 指向 64 位的内存地址
* 输出参数: 无。
* 函数返回: 成功返回 0，否则返回 -1。
* 其它说明: N/A。
*******************************************************************************/

static int memory_post_dataline (unsigned long long * pmem)
{
    unsigned long long temp64 = 0;
    int                num_patterns = sizeof (pattern)/ sizeof (pattern[0]);
    int                i;
    unsigned int       hi;
    unsigned int       lo;
    unsigned int       pathi;
    unsigned int       patlo;
    int                ret = 0;
  
    
    for (i = 0; i < num_patterns; i++)
    {
        move64 ((unsigned long long *)&(pattern[i]), pmem++);
        
        /*
         * Put a different pattern on the data lines: otherwise they
         * may float long enough to read back what we wrote.
         */
        
        /* 预防floating buses错误 */
        
        move64 ((unsigned long long *) &otherpattern, pmem--);
        move64 (pmem, &temp64);
        
#ifdef INJECT_DATA_ERRORS
        temp64 ^= 0x00008000;
#endif
        
        if (temp64 != pattern [i])
        {
            pathi = (pattern [i] >> 32) & 0xffffffff;
            patlo = pattern [i] & 0xffffffff;
            
            hi = (temp64 >> 32) & 0xffffffff;
            lo = temp64 & 0xffffffff;
            
            post_log ("Memory (date line) error at %08llx, "
                      "wrote %08x%08x, read %08x%08x !\r\n",
                      pmem, pathi, patlo, hi, lo);
            ret = -1;
        }
    }
    
    return ret;
}


/*******************************************************************************
* 函数名称: memory_post_addrline
* 功能说明: 地址线检测 
* 调用函数: ...
* 被调函数: ...
* 输入参数: testaddr: 指向待测试的地址；
*           base    : 指向内存基地址；
*           size    ：内存大小。
* 输出参数: 
* 函数返回: 成功返回 0，否则返回 -1。
* 其它说明: N/A。
*******************************************************************************/

static int memory_post_addrline (ulong * testaddr, ulong * base, ulong size)
{
    ulong * target;
    ulong * end;
    ulong   readback;
    ulong   xor;
    int     ret = 0;
    
    end = (ulong *)((ulong)base + size);/* pointer arith! */
    xor = 0;
    
    for (xor = sizeof (ulong); xor > 0; xor <<= 1)
    {
        /* 对测试的地址的某一根地址线的值翻转 */
        
        target = (ulong *)((ulong)testaddr ^ xor);
        
        if ((target >= base) && (target < end))
        {
            /* 
             * 由于target是testaddr某一根地址线的值翻转得来
             * 故testaddr != target,下面赋值操作后
             * 应有 *testaddr != *target 
             */
            
            *testaddr = ~*target;
            readback = *target;
            
#ifdef INJECT_ADDRESS_ERRORS
            if (xor == 0x00008000)
            {
                readback = *testaddr;
            }
#endif
            /* 出现此种情况只有testaddr == target,即地址线翻转无效 */
            
            if (readback == *testaddr)
            {
                post_log ("Memory (address line) error at %08x<->%08x, "\
                          "XOR value %08x !\r\n", testaddr, target, xor);
                ret = -1;
            }
        }
    }
    
    return ret;
}


/*******************************************************************************
* 函数名称: memory_post_test1
* 功能说明: 地址线检测 
* 调用函数: ...
* 被调函数: ...
* 输入参数: start : 内存的起始地址；
*           size  : 连续内存的大小；
*           val   : 测试目标数据。
* 输出参数: 无。
* 函数返回: 成功返回 0，否则返回 -1。
* 其它说明: N/A。
*******************************************************************************/

static int memory_post_test1 (ulong start, ulong size,  ulong val)
{
    ulong   i = 0;
    ulong * mem = (ulong *) start;
    ulong   readback;
    int     ret = 0;
    
    for (i = 0; i < size / sizeof (ulong); i++) 
    {
        mem [i] = val;
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("1-W [0x%x:0x%08x] %10c\r", mem + i, mem [i], ' ');
#endif           
            QUIT_CMD_CHECK 
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
#endif
    
    for (i = 0; i < size / sizeof (ulong) && ret == 0; i++) 
    {
        readback = mem [i];
        if (readback != val) 
        {
            post_log ("Memory error at %08x, wrote %08x, read %08x !\r\n",
                      mem + i, val, readback);
            
            ret = -1;
            /*break;*/
        }
        
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("1-R-C [0x%x:0x%08x] %10c\r", mem + i, mem [i], ' ');
#endif     
            QUIT_CMD_CHECK
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
    CLEAR_LINE;
#endif

    return ret;
}


/*******************************************************************************
* 函数名称: memory_post_test2
* 功能说明: 地址线检测 
* 调用函数: ...
* 被调函数: ...
* 输入参数: start : 内存的起始地址；
*           size  : 连续内存的大小。
* 输出参数: 无。
* 函数返回: 成功返回 0，否则返回 -1。
* 其它说明: N/A。
*******************************************************************************/

static int memory_post_test2 (ulong start, ulong size)
{
    ulong   i = 0;
    ulong * mem = (ulong *) start;
    ulong   readback;
    int     ret = 0;
    
    for (i = 0; i < size / sizeof (ulong); i++) 
    {
        mem [i] = 1 << (i % 32);
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("2-W [0x%x:0x%08x] %10c\r", mem + i, mem [i], ' ');
#endif
            QUIT_CMD_CHECK
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
#endif
    
    for (i = 0; i < size / sizeof (ulong) && ret == 0; i++) 
    {
        readback = mem [i];
        if (readback != (1 << (i % 32))) 
        {
            post_log ("Memory error at %08x, wrote %08x, read %08x !\r\n",
                      mem + i, 1 << (i % 32), readback);
            
            ret = -1;
            /*break;*/
        }
        
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("2-R-C [0x%x:0x%08x] %10c\r", mem + i, mem [i], ' ');
#endif 
            QUIT_CMD_CHECK
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
    CLEAR_LINE;
#endif
    
    return ret;
}


/*******************************************************************************
* 函数名称: memory_post_test3
* 功能说明: 内存测试模式3。
* 调用函数: ...
* 被调函数: ...
* 输入参数: start : 内存的起始地址；
*           size  : 连续内存的大小。
* 输出参数: 无。
* 函数返回: 成功返回 0，否则返回 -1。
* 其它说明: N/A。
*******************************************************************************/

static int memory_post_test3 (ulong start, ulong size)
{
    ulong   i = 0;
    ulong * mem = (ulong *) start;
    ulong   readback;
    int     ret = 0;
    
    for (i = 0; i < size / sizeof (ulong); i++) 
    {
        mem [i] = i;
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("3-W [0x%x:0x%08x] %10c\r", mem + i, mem [i], ' ');
#endif
            QUIT_CMD_CHECK;
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
#endif
    
    for (i = 0; i < size / sizeof (ulong) && ret == 0; i++) 
    {
        readback = mem [i];
        if (readback != i) 
        {
            post_log ("Memory error at %08x, wrote %08x, read %08x !\r\n",
                      mem + i, i, readback);
            
            ret = -1;
            break;
        }
        
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("3-R-C [0x%x:0x%08x] %10c\r", mem + i, mem [i], ' ');
#endif 
            QUIT_CMD_CHECK
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
    CLEAR_LINE;
#endif
    
    return ret;
}


/*******************************************************************************
* 函数名称: memory_post_test4
* 功能说明: 内存测试模式4。
* 调用函数: ...
* 被调函数: ...
* 输入参数: start : 内存的起始地址；
*           size  : 连续内存的大小。
* 输出参数: 无。
* 函数返回: 成功返回 0，否则返回 -1。
* 其它说明: N/A。
*******************************************************************************/

static int memory_post_test4 (ulong start, ulong size)
{
    ulong   i = 0;
    ulong * mem = (ulong *) start;
    ulong   readback;
    int     ret = 0;
    
    for (i = 0; i < size / sizeof (ulong); i++) 
    {
        mem [i] = ~i;
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("4-W [0x%x:0x%08x] %10c\r", mem + i, mem [i], ' ');
#endif
            QUIT_CMD_CHECK
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
#endif
    
    for (i = 0; i < size / sizeof (ulong) && ret == 0; i++)
    {
        readback = mem [i];
        if (readback != ~i) 
        {
            post_log ("Memory error at %08x, wrote %08x, read %08x !\r\n",
                       mem + i, ~i, readback);
            
            ret = -1;
            /*break;*/
        }
        
        if (i % 1024 == 0)
        {
#ifdef DDR_DIAG_DEBUG
            CLEAR_LINE;
            printf ("4-R-C [0x%x:0x%x] %10c\r", mem + i, mem [i], ' ');
#endif 
            QUIT_CMD_CHECK 
            WATCHDOG_RESET ();
        }
    }

#ifdef DDR_DIAG_DEBUG
    printf ("Done.\r");
    CLEAR_LINE;
#endif
  
    return ret;
}


/*******************************************************************************
* 函数名称: memory_post_tests
* 功能说明: 内存测试主序列。
* 调用函数: ...
* 被调函数: ...
* 输入参数: start : 内存的起始地址；
*           size  : 连续内存的大小。
* 输出参数: 无。
* 函数返回: 成功返回 0，否则返回 -1。
* 其它说明: N/A。
*******************************************************************************/

int memory_post_tests (ulong start, ulong size)
{
    int ret = 0;
    
    if (ret == 0)
    {
        ret = memory_post_dataline ((unsigned long long *) start);
    }
    else
    {
#ifdef DDR_DIAG_DEBUG
        printf ("DDR dataline error \r\n");
#endif
        return -1;  
    }
   
    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_addrline ((ulong *) start, (ulong *) start, size);
    }
    else
    {
#ifdef DDR_DIAG_DEBUG
        printf ("DDR addrline error \r\n");
#endif
        return -1;  
    }

    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_addrline ((ulong *)(start + size - 8), 
                                    (ulong *)start, size);
    }
    else
    {
        return -1;  
    }

    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_test1 (start, size, 0x00000000);
    }
    else
    {
        return -1;  
    } 
        
    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_test1 (start, size, 0xffffffff);
    }
    else
    {
        return -1;  
    }
        
    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_test1 (start, size, 0x55555555);
    }
    else
    {
        return -1;  
    }
       
    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_test1 (start, size, 0xaaaaaaaa);
    }
    else
    {
        return -1;  
    }

#if 1         
    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_test2 (start, size);
    }
    else
    {
        return -1;  
    }
        
    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_test3 (start, size);
    }
    else
    {
        return -1;  
    }
        
    WATCHDOG_RESET ();
    
    if (ret == 0)
    {
        ret = memory_post_test4 (start, size);
    }
    else
    {
        return -1;
    }
        
    WATCHDOG_RESET ();
#endif
    
    return ret;
}

