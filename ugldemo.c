/* ugldemo.c - Wind River Media Library Graphics primitives demonstration program */

/* Copyright 1999-2006 Wind River Systems, Inc. All Rights Reserved */

/*
modification history
--------------------
01v,21jun06,rfm  Modify to support multiple displays
01u,10jan06,rfm  Fix SPR#114133: make colorTable local
01t,15mar05,jlb  Correct size of title buffer
01s,26aug04,jlb  Update for execution in RTP
01r,09may03,jlb  Correct version number display format
01q,16apr03,jlb  Improved comments and program documentation and updated 
                 version display
01p,02apr03,jlb  Correct Diab compile warning
01o,01apr03,sts  [SPR 86504] Use uglRegistryFind() safely.
01n,01aug02,rfm  Removed static declaration of windMLDemo
01m,26apr02,gav  Change banner to display actual version.
01l,25feb02,wdf  Fixed compiler warnings.
01k,22feb02,msr  Backward compatibility for input API.
01j,29jan02,rbp  Addition of support for Native Unix.
01i,28aug01,rbp  Fix for __unix__ macro.
01h,23jul01,c_s  Change NATIVE to WINDML_NATIVE (less chance of collision)
01g,16jun01,c_s  Allow argv [1] to set mode.
01f,15jun01,c_s  Add native support
01i,05nov01,gav  Fixed misnamed devIds
01h,05nov01,gav  Change to new registry
01g,05nov01,gav  Change to new registry
01f,09oct01,msr  Ported to new UGL_Q_EVENT architecture.
01e,30nov00,gav  Fixed message length to fit on small screens.
01d,21nov00,gav  Clearscreen corrected (SPR 36009).
01c,16nov00,msr  Fixed SPR #62051
01b,27oct00,rfm  Added func ptr cast to taskSpawn
01a,25oct00,rfm  Added taskSpawn
*/

/*
DESCRIPTION

 This example program demonstrates basic drawing primitives.  It performs a 
 basic test of the drawing primitives. It demonstrates how to:

\ml 
\m Initialize library
\m Identify the graphics device and input devices
\m Create fonts 
\m Determine the resolution of the display
\m Create a graphics context
\m Allocate colors
\m Create a clipping region
\m Create various bitmaps (color, monochrome, and transparent
\m Create a cursor and position the cursor based on pointer movement reports
\m Write bitmaps to the display
\m Blt color bitmaps to the display
\m Blt transparent bitmaps to the display
\m Draw lines (thin solid, thin dashed, wide solid, and wide dashed)
\m Draw filled rectangles
\m Draw rectangles filled with a pattern
\m Draw filled polygons
\m Draw polygons filled with a pattern
\m Draw text
\m Draw filled ellipses
\m Draw filled pie shapes
\m Blt with a image stretch
\m Releasing all resources and terminating the application.
\me

 To start the program in kernel mode:

 -> ugldemo <pDisplay>, <mode>

 The <pDisplay> string allows the user to specify which display to run
 ugldemo. If the display string is a number, it is interpreted as a display number.
 Any other string is interpreted as a display name. If <pDisplay> is NULL, display
 number zero is assumed.

 If the <mode> parameter is positive, no input devices are assumed to be 
 present and the demo will wait <mode> number of seconds before moving to 
 the next test. A value of zero assumes that a mouse or keyboard is present 
 and the user must press a key or mouse button to continue to the next test. 
 A negative value assumes no input devices are present and no delay between 
 tests. When the <mode> specifies a pause waiting for operator input (<mode> = 0),
 input devices are assumed to be present and entry of 'q' or 'Q' can immediately 
 terminate the program.  Any other key entered or depressing any pointer button 
 causes the program to proceed on to the next graphics primitive demonstration.
 
 To start the program in RTP mode:
 
 -> rtpSp "uglDemo.vxe \"myDisplay\", 0"
 
*/

#include <vxWorks.h>
#include <ugl/ugl.h>
#include <ugl/uglos.h>
#include <ugl/uglMsg.h>
#include <ugl/uglfont.h>
#include <ugl/uglinput.h>
#include <ugl/ugldib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UGL_STATUS windMLDemo (char * pDisplay, int mode);

/*
* The color table is where we define the colors we want
* to have available.  The format is an array of
* ARGB values paired with their allocated uglColor.  As
* of this writing, we don't need to worry about Alpha
* ("A") values unless we are using video.
*/
typedef struct _colorStruct
    {
    UGL_RGB rgbColor;
    UGL_COLOR uglColor;
    }COLOR_STRUCT;


/* Put all display specific information into a single structure */

typedef struct display_control
    {
    UGL_DEVICE_ID devId;
    UGL_GC_ID gc;
    UGL_INPUT_SERVICE_ID inputServiceId;
    UGL_REGION_ID regionId;
    UGL_FONT_ID fontDialog;
    UGL_FONT_ID fontSystem;
    UGL_FONT_ID fontFixed;
    UGL_FONT_DRIVER_ID fontDrvId;
    UGL_DDB_ID stdDdb;
    UGL_MDDB_ID patternDdb;
    UGL_CDDB_ID cursorDdb;
    UGL_TDDB_ID transDdb;
    int *randomData;
    UGL_COLOR * colorData;
    int displayHeight;
    int displayWidth;
    int mode;
    char pDisplay[256];
    COLOR_STRUCT colorTable[16];
    } DISPLAY_CONTROL;

/* 
* This is the data for a user defined fill pattern that
* can be used with various drawing primitives.  This data
* will be used to create a monochrome DIB (MDIB), which
* will be used in turn to create a monochrome "bitmap".
* It is the bitmap that is used by the drawing primitives.
*
* Solid fills do not need to have a pattern defined, just
* pass an UGL_NULL value in the pattern parameter to
* revert (it is the default) to solid fills.
*/

struct
    {
    int width;
    int height;
    unsigned char data[32];
    } patternData =
    {
    16, 16,
        { 
        0xFF, 0xFF, 
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x01,
        0xFF, 0xFF, 
        0x01, 0x00,
        0x01, 0x00,
        0x01, 0x00,
        0x01, 0x00,
        0x01, 0x00,
        0x01, 0x00,
        0x01, 0x00
        }
    };

/* Label the colors we defined */

#define BLACK           (0)
#define BLUE            (1)
#define GREEN           (2)
#define CYAN            (3)
#define RED             (4)
#define MAGENTA         (5)
#define BROWN           (6)
#define LIGHTGRAY       (7)
#define DARKGRAY        (8)
#define LIGHTBLUE       (9)
#define LIGHTGREEN      (10)
#define LIGHTCYAN       (11)
#define LIGHTRED        (12)
#define LIGHTMAGENTA    (13)
#define YELLOW          (14)
#define WHITE           (15)
#define TRANS           (255)
#define INVERT          (254)

/* Definition of a smiley face cursor */

UGL_ARGB cursorClut[] =
    {
    UGL_MAKE_RGB(0, 0, 0),
    UGL_MAKE_RGB(255, 255, 84),
    };

UGL_UINT8 cursorData[] =
    {
#define B 0,
#define Y 1,
#define T 255,
#define I 254,
    T T T T T T T T T T T T Y Y Y Y Y Y Y Y T T T T T T T T T T T T
    T T T T T T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T T T T T T
    T T T T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T T T T
    T T T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T T T
    T T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T T
    T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T
    T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T
    T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T
    T T Y Y Y Y Y Y Y T T T Y Y Y Y Y Y Y Y I I I Y Y Y Y Y Y Y T T
    T Y Y Y Y Y Y Y T T T T T Y Y Y Y Y Y I I I I I Y Y Y Y Y Y Y T
    T Y Y Y Y Y Y Y T T T T T Y Y Y Y Y Y I I I I I Y Y Y Y Y Y Y T
    T Y Y Y Y Y Y Y T T T T T Y Y Y Y Y Y I I I I I Y Y Y Y Y Y Y T
    Y Y Y Y Y Y Y Y Y T T T Y Y Y Y Y Y Y Y I I I Y Y Y Y Y Y Y Y Y
    Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
    Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
    Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
    Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
    Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
    Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
    Y Y Y Y Y B B Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y B B Y Y Y Y Y
    T Y Y Y Y B B Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y B B Y Y Y Y T
    T Y Y Y Y B B B Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y B B B Y Y Y Y T
    T Y Y Y Y Y B B Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y B B Y Y Y Y Y T
    T T Y Y Y Y B B B Y Y Y Y Y Y Y Y Y Y Y Y Y Y B B B Y Y Y Y T T
    T T Y Y Y Y Y B B B B Y Y Y Y Y Y Y Y Y Y B B B B Y Y Y Y Y T T
    T T T Y Y Y Y Y B B B B B B Y Y Y Y Y B B B B B Y Y Y Y Y T T T
    T T T T Y Y Y Y Y Y B B B B B B B B B B B B Y Y Y Y Y Y T T T T
    T T T T T Y Y Y Y Y Y Y Y B B B B B B Y Y Y Y Y Y Y Y T T T T T
    T T T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T T T
    T T T T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T T T T
    T T T T T T T T T Y Y Y Y Y Y Y Y Y Y Y Y Y Y T T T T T T T T T
    T T T T T T T T T T T T Y Y Y Y Y Y Y Y T T T T T T T T T T T T
#undef B
#undef Y
#undef T
#undef I
    };

UGL_UINT8 transparentData[] =
    {
#define O BLACK,
#define _ BLACK,
#define I YELLOW,
    O O O O O O O O O O O O I I I I I I I I O O O O O O O O O O O O
    O O O O O O O O O I I I I I I I I I I I I I I O O O O O O O O O
    O O O O O O O I I I I I I I I I I I I I I I I I I O O O O O O O
    O O O O O O I I I I I I I I I I I I I I I I I I I I O O O O O O
    O O O O O I I I I I I I I I I I I I I I I I I I I I I O O O O O
    O O O O I I I I I I I I I I I I I I I I I I I I I I I I O O O O
    O O O I I I I I I I I I I I I I I I I I I I I I I I I I I O O O
    O O I I I I I I I I I I I I I I I I I I I I I I I I I I I I O O
    O O I I I I I I I _ _ _ I I I I I I I I _ _ _ I I I I I I I O O
    O I I I I I I I _ _ _ _ _ I I I I I I _ _ _ _ _ I I I I I I I O
    O I I I I I I I _ _ _ _ _ I I I I I I _ _ _ _ _ I I I I I I I O
    O I I I I I I I _ _ _ _ _ I I I I I I _ _ _ _ _ I I I I I I I O
    I I I I I I I I I _ _ _ I I I I I I I I _ _ _ I I I I I I I I I
    I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I
    I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I
    I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I
    I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I
    I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I
    I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I
    I I I I I _ _ I I I I I I I I I I I I I I I I I I _ _ I I I I I
    O I I I I _ _ I I I I I I I I I I I I I I I I I I _ _ I I I I O
    O I I I I _ _ _ I I I I I I I I I I I I I I I I _ _ _ I I I I O
    O I I I I I _ _ I I I I I I I I I I I I I I I I _ _ I I I I I O
    O O I I I I _ _ _ I I I I I I I I I I I I I I _ _ _ I I I I O O
    O O I I I I I _ _ _ _ I I I I I I I I I I _ _ _ _ I I I I I O O
    O O O I I I I I _ _ _ _ _ _ I I I I I _ _ _ _ _ I I I I I O O O
    O O O O I I I I I I _ _ _ _ _ _ _ _ _ _ _ _ I I I I I I O O O O
    O O O O O I I I I I I I I _ _ _ _ _ _ I I I I I I I I O O O O O
    O O O O O O I I I I I I I I I I I I I I I I I I I I O O O O O O
    O O O O O O O I I I I I I I I I I I I I I I I I I O O O O O O O
    O O O O O O O O O I I I I I I I I I I I I I I O O O O O O O O O
    O O O O O O O O O O O O I I I I I I I I O O O O O O O O O O O O
#undef _
#undef O
#undef I
    };

UGL_UINT8 transparentMask[] =
    {
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x7F, 0xFE, 0x00,
    0x01, 0xFF, 0xFF, 0x80,
    0x03, 0xFF, 0xFF, 0xC0,
    0x07, 0xFF, 0xFF, 0xE0,
    0x0F, 0xFF, 0xFF, 0xF0,
    0x1F, 0xFF, 0xFF, 0xF8,
    0x3F, 0xFF, 0xFF, 0xFC,
    0x3F, 0xFF, 0xFF, 0xFC,
    0x7F, 0xFF, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0x7F, 0xFF, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFF, 0xFE,
    0x3F, 0xFF, 0xFF, 0xFC,
    0x3F, 0xFF, 0xFF, 0xFC,
    0x1F, 0xFF, 0xFF, 0xF8,
    0x0F, 0xFF, 0xFF, 0xF0,
    0x07, 0xFF, 0xFF, 0xE0,
    0x03, 0xFF, 0xFF, 0xC0,
    0x01, 0xFF, 0xFF, 0x80,
    0x00, 0x7F, 0xFE, 0x00,
    0x00, 0x0F, 0xF0, 0x00
    };

/**************************************************************************
*
* flushQ - flush the input message queue and update cursor position
*
* This routine reads from the input message queue until it is empty.  If the
* message type was a pointer position update, the cursor is repositioned on
* the display.
*
* RETURNS: 
*
* ERRNO: N/A
*
* SEE ALSO:  
*
* NOMANUAL
*
*/
static void flushQ 
    (
    DISPLAY_CONTROL * pDisplayControl
    )
    {
    UGL_MSG msg;
    UGL_STATUS status;

    do
        {
        status = uglInputMsgGet (pDisplayControl->inputServiceId, &msg, UGL_NO_WAIT);

        if ((status == UGL_STATUS_OK) && (msg.type == MSG_POINTER))
            {
            uglCursorMove(pDisplayControl->devId, 
                          msg.data.pointer.position.x, 
                          msg.data.pointer.position.y); 
            }
        } while (status != UGL_STATUS_Q_EMPTY);
    }

/**************************************************************************
*
* pauseDemo - wait for input from keyboard or pointer
*
* RETURNS:  -1 when a "q" or "Q" entered; otherwise 0
*
* ERRNO: N/A
*
* SEE ALSO:  
*
* NOMANUAL
*
*/
static int pauseDemo
    (
    DISPLAY_CONTROL * pDisplayControl   /* Display control */
    )
    {
    static UGL_CHAR * message = "Press 'q' to quit or any other";
    static UGL_CHAR * message2= "key (or mouse button) to continue.";
    int textWidth, textHeight;
    UGL_MSG msg;
    UGL_STATUS status;
    int retVal = 0;

    if (pDisplayControl->mode == 0 && pDisplayControl->inputServiceId != UGL_NULL)
        {
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[BLACK].uglColor);
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[LIGHTRED].uglColor);
        uglFontSet(pDisplayControl->gc, pDisplayControl->fontSystem);

        uglTextSizeGet(pDisplayControl->fontSystem, &textWidth, &textHeight, -1, message);

        uglTextDraw(pDisplayControl->gc, (pDisplayControl->displayWidth - textWidth) / 2,
                    (pDisplayControl->displayHeight - textHeight) / 2  - textHeight, -1, message);

        uglTextSizeGet(pDisplayControl->fontSystem, &textWidth, &textHeight,
                       -1, message2);

        uglTextDraw(pDisplayControl->gc, (pDisplayControl->displayWidth - textWidth) / 2,
                    (pDisplayControl->displayHeight - textHeight) / 2, -1, message2);

        flushQ(pDisplayControl);

        UGL_FOREVER
            {
            status = uglInputMsgGet (pDisplayControl->inputServiceId, &msg, UGL_WAIT_FOREVER);

            if (msg.type == MSG_KEYBOARD)
                {
                if (msg.data.keyboard.modifiers & UGL_KBD_KEYDOWN) 
                    {
                    if (msg.data.keyboard.key == 'q' ||
                        msg.data.keyboard.key == 'Q')
                        retVal = -1;
                    break;
                    }
                }
            else if (msg.type == MSG_POINTER)
                {
                uglCursorMove (pDisplayControl->devId, msg.data.pointer.position.x,
                               msg.data.pointer.position.y);
                if ((msg.data.pointer.buttonChange & 
                     msg.data.pointer.buttonState) != 0)
                    break;
                }
            }
        }
    else if (pDisplayControl->mode > 0)
        {
        uglOSTaskDelay (pDisplayControl->mode * 1000);
        }

    return(retVal);
    }

/**************************************************************************
*
* ClearScreen - clear the screen
*
* This routine clears the screen by drawing a black rectangle to the display.
*
* RETURNS: 
*
* ERRNO: N/A
*
* SEE ALSO:  
*
* NOMANUAL
*/
static void ClearScreen
    (
    DISPLAY_CONTROL * pDisplayControl   /* Display control */
    )
    {
    uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable [BLACK].uglColor);
    uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable [BLACK].uglColor);
    uglLineStyleSet(pDisplayControl->gc, UGL_LINE_STYLE_SOLID);
    uglLineWidthSet(pDisplayControl->gc, 1);
    uglRectangle(pDisplayControl->gc, 0, 0, pDisplayControl->displayWidth - 1, 
                 pDisplayControl->displayHeight - 1);   
    }

/**************************************************************************
*
* cleanUp - cleans up all resources prior to termination
*
* This routine releases all system resources prior to termination.
*
* RETURNS: 
*
* ERRNO: N/A
*
* SEE ALSO:  
*
* NOMANUAL
*/
static void cleanUp
    (
    DISPLAY_CONTROL * pDisplayControl   /* Display control */
    )
    {
    if (pDisplayControl->mode >= 0 && pDisplayControl->inputServiceId != UGL_NULL)
        {
        uglCursorBitmapDestroy (pDisplayControl->devId, pDisplayControl->cursorDdb);
        uglCursorDeinit (pDisplayControl->devId);
        }

    uglTransBitmapDestroy (pDisplayControl->devId, pDisplayControl->transDdb);
    uglBitmapDestroy(pDisplayControl->devId, pDisplayControl->stdDdb);
    UGL_FREE(pDisplayControl->colorData);
    uglMonoBitmapDestroy (pDisplayControl->devId, pDisplayControl->patternDdb);
    uglRegionDestroy(pDisplayControl->regionId);
    UGL_FREE (pDisplayControl->randomData);
    uglFontDestroy (pDisplayControl->fontFixed);
    uglFontDestroy (pDisplayControl->fontDialog);
    uglFontDestroy (pDisplayControl->fontSystem);
    uglGcDestroy (pDisplayControl->gc);
    uglDisplayClose(pDisplayControl->pDisplay);
    UGL_FREE(pDisplayControl);
    }

#if defined(_WRS_KERNEL)

/**************************************************************************
*
* ugldemo - start of the demo program in kernel mode
*
* This is the kernel mode entry point for the demo, which can be called
* directly from the shell.
*
* <pDisplay> is a character string to describe which display to run the 
* application. For example, "0" specifies display number 0, "myDisplay" 
* specifies the display titled "myDisplay". If <pDisplay> is NULL, display 
* zero is assumed.
*
* The <mode> when greater than 0 is the number of seconds to wait between 
* tests.  If <mode> is 0, a keyboard or pointer is assumed to be present and 
* will wait for a key press or pointer button click to move to the next tests. 
* If <mode> is less than 0, the demo will  run with no delay in between tests 
* with no input required.
*
* RETURNS: 
*
* ERRNO: N/A
*
* SEE ALSO:  
*
*/
void ugldemo 
    (
    char * pDisplay,    /* Display ID string */
    int mode            /* demo mode */
    )
    {
    uglOSTaskCreate("tWindMLDemo", (UGL_FPTR)windMLDemo, 
                    110, 0, 10000, (int)pDisplay,mode,0,0,0);
    }
#else

/**************************************************************************
*
* main - start of demo program in RTP mode
*
* Start the example program.  
*
* RETURNS: 
*
* ERRNO: N/A
*
* SEE ALSO:  
*
* NOMANUAL
*
*/
int main 
    (
    int argc, 
    char *argv []
    )
    {
    int mode = 0;
    char * pDisplay = UGL_NULL;

    if (argc > 1)
        {
        pDisplay = argv[1];
        }

    if (argc > 2) 
        {
        mode = atoi (argv [2]);
        }

    setbuf(stdout,NULL);
    setbuf(stderr,NULL);
    windMLDemo(pDisplay, mode);
    return 0;
    }
#endif

/**************************************************************************
*
* windMLDemo - entry point for the demo
*
* <pDisplay> is a character string to describe which display to run the 
* application. For example, "0" specifies display number 0, "myDisplay" 
* specifies the display titled "myDisplay". If <pDisplay> is NULL, display 
* zero is assumed.
*
* The <mode> when greater than 0 is the number of seconds to wait between 
* tests.  If <mode> is 0, a keyboard or pointer is assumed to be present and 
* will wait for a key press or pointer button click to move to the next tests. 
* If <mode> is less than 0, the demo will  run with no delay in between tests 
* with no input required.
*
* RETURNS: void
*
* ERRNO: N/A
*
* SEE ALSO: N/A
*
* NOMANUAL
*
*/
UGL_STATUS windMLDemo 
    (
    char * pDisplay,    /* Display ID string */
    int mode            /* demo mode */
    )
    {
    UGL_REG_DATA *pRegistryData;
    UGL_DIB transDib;
    UGL_MDIB transMdib;
    UGL_MDIB patternDib;
    UGL_CDIB  cursorDib;
    UGL_FONT_DEF systemFontDef;
    UGL_FONT_DEF dialogFontDef;
    UGL_FONT_DEF fixedFontDef;
    UGL_ORD textOrigin = UGL_FONT_TEXT_UPPER_LEFT;
    int numRandomPoints;
    int i, index, y, textpage, tmp;
    char *fontTestText = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char regionMessage[50];
    int textWidth, textHeight;
    UGL_FB_INFO fbInfo;
    UGL_RECT rect;
    UGL_UINT32 displayNumber;
    char * pDisplayName = UGL_NULL;
    DISPLAY_CONTROL * pDisplayControl;
    UGL_STATUS status;

    /* Initialize the display */

    status = uglDisplayOpen(pDisplay, &pDisplayName, &displayNumber);

    if (status == UGL_STATUS_ERROR)
        {
        printf("Display \"%s\" failed to initialize\n", pDisplay);
        return(UGL_STATUS_ERROR);
        }
    else if (status == UGL_STATUS_FINISHED)
        {
        printf("Display \"%s\" already initialized\n", pDisplayName);
        return(UGL_STATUS_ERROR);
        }

    pDisplayControl = (DISPLAY_CONTROL *)UGL_CALLOC(1, sizeof(DISPLAY_CONTROL));

    if (pDisplayControl == UGL_NULL)
        {
        uglDisplayClose(pDisplay);
        }

    pDisplayControl->mode = mode;

    if (pDisplay != UGL_NULL)
        {
        strcpy(pDisplayControl->pDisplay, pDisplay);
        }
    else
        {
        pDisplayControl->pDisplay[0] = 0;
        }

    /* Obtain display device identifier */

    pRegistryData = uglRegistryFind (UGL_DISPLAY_TYPE, &displayNumber, 0, 0);
    if (pRegistryData == UGL_NULL)
        {
        printf("Display not found. Exiting.\n");
        uglDisplayClose(pDisplay);
        UGL_FREE(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    pDisplayControl->devId = (UGL_DEVICE_ID)pRegistryData->id;

    if (pDisplayControl->mode >= 0)
        {
        /* obtain the input service identifier. */

        pRegistryData = uglRegistryFind (UGL_INPUT_SERVICE_TYPE, &displayNumber, 0, 0);
        if (pRegistryData == UGL_NULL)
            {
            printf("Input service not found. Exiting.\n");
            uglDisplayClose(pDisplay);
            UGL_FREE(pDisplayControl);
            return(UGL_STATUS_ERROR);
            }
        pDisplayControl->inputServiceId = (UGL_INPUT_SERVICE_ID)pRegistryData->id;
        }

    /* Create a graphics context */

    pDisplayControl->gc = uglGcCreate(pDisplayControl->devId);

    /* Create Fonts */

    pRegistryData = uglRegistryFind (UGL_FONT_ENGINE_TYPE, &displayNumber, 0, 0);
    if (pRegistryData == UGL_NULL)
        {
        printf("Font engine not found. Exiting.\n");
        uglDisplayClose(pDisplay);
        UGL_FREE(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }
    pDisplayControl->fontDrvId = (UGL_FONT_DRIVER_ID)pRegistryData->id;

    uglFontDriverInfo(pDisplayControl->fontDrvId, UGL_FONT_TEXT_ORIGIN, &textOrigin);

    uglFontFindString(pDisplayControl->fontDrvId, "familyName=Lucida; pixelSize = 12", &systemFontDef);

    if ((pDisplayControl->fontSystem = uglFontCreate(pDisplayControl->fontDrvId, &systemFontDef)) == UGL_NULL)
        {
        printf("Font not found. Exiting.\n");
        uglDisplayClose(pDisplay);
        UGL_FREE(pDisplayControl);
        return(UGL_STATUS_ERROR);        
        }

    uglFontFindString(pDisplayControl->fontDrvId, "familyName=Helvetica; pixelSize = 18", &dialogFontDef);

    if ((pDisplayControl->fontDialog = uglFontCreate(pDisplayControl->fontDrvId, &dialogFontDef)) == UGL_NULL)
        {
        printf("Font not found. Exiting.\n");
        uglDisplayClose(pDisplay);
        UGL_FREE(pDisplayControl);
        return(UGL_STATUS_ERROR);       
        }

    uglFontFindString(pDisplayControl->fontDrvId, "familyName=Courier; pixelSize = 12", &fixedFontDef);

    if ((pDisplayControl->fontFixed = uglFontCreate(pDisplayControl->fontDrvId, &fixedFontDef)) == UGL_NULL)
        {
        printf("Font not found. Exiting.\n");
        uglDisplayClose(pDisplay);
        UGL_FREE(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Obtain the dimensions of the display */

    uglInfo(pDisplayControl->devId, UGL_FB_INFO_REQ, &fbInfo);
    pDisplayControl->displayWidth = fbInfo.width;
    pDisplayControl->displayHeight = fbInfo.height;

    if (pDisplayControl->mode == 0)
        {
        printf("Press a pointer button or key to proceed to next graphics primitive\n");
        printf("Enter a 'q' or 'Q' to immediately terminate program\n");
        }

    /* Setup random points */

    srand(6);
    numRandomPoints = 2000;
    pDisplayControl->randomData = (int *)UGL_MALLOC(2 * numRandomPoints * sizeof(int));
    for (i = 0; i < numRandomPoints * 2; i += 2)
        {
        pDisplayControl->randomData[i] = (rand() % pDisplayControl->displayWidth);
        pDisplayControl->randomData[i + 1] = (rand() % pDisplayControl->displayHeight);
        }

    /* Initialize colors */

    pDisplayControl->colorTable[BLACK].rgbColor = UGL_MAKE_RGB(0, 0, 0);
    pDisplayControl->colorTable[BLUE].rgbColor = UGL_MAKE_RGB(0, 0, 168);
    pDisplayControl->colorTable[GREEN].rgbColor = UGL_MAKE_RGB(0, 168, 0);
    pDisplayControl->colorTable[CYAN].rgbColor = UGL_MAKE_RGB(0, 168, 168);
    pDisplayControl->colorTable[RED].rgbColor = UGL_MAKE_RGB(168, 0, 0);
    pDisplayControl->colorTable[MAGENTA].rgbColor = UGL_MAKE_RGB(168, 0, 168);
    pDisplayControl->colorTable[BROWN].rgbColor = UGL_MAKE_RGB(168, 84, 0);
    pDisplayControl->colorTable[LIGHTGRAY].rgbColor = UGL_MAKE_RGB(168, 168, 168);
    pDisplayControl->colorTable[DARKGRAY].rgbColor = UGL_MAKE_RGB(84, 84, 84);
    pDisplayControl->colorTable[LIGHTBLUE].rgbColor = UGL_MAKE_RGB(84, 84, 255);
    pDisplayControl->colorTable[LIGHTGREEN].rgbColor = UGL_MAKE_RGB(84, 255, 84);
    pDisplayControl->colorTable[LIGHTCYAN].rgbColor = UGL_MAKE_RGB(84, 255, 255);
    pDisplayControl->colorTable[LIGHTRED].rgbColor = UGL_MAKE_RGB(255, 84, 84);
    pDisplayControl->colorTable[LIGHTMAGENTA].rgbColor = UGL_MAKE_RGB(255, 84, 255);
    pDisplayControl->colorTable[YELLOW].rgbColor = UGL_MAKE_RGB(255, 255, 84);
    pDisplayControl->colorTable[WHITE].rgbColor = UGL_MAKE_RGB(255, 255, 255);

    uglColorAlloc (pDisplayControl->devId, &pDisplayControl->colorTable[BLACK].rgbColor, UGL_NULL, 
                   &pDisplayControl->colorTable[BLACK].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[BLUE].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[BLUE].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[GREEN].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[GREEN].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[CYAN].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[CYAN].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[RED].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[RED].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[MAGENTA].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[MAGENTA].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[BROWN].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[BROWN].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[LIGHTGRAY].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[LIGHTGRAY].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[DARKGRAY].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[DARKGRAY].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[LIGHTBLUE].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[LIGHTBLUE].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[LIGHTGREEN].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[LIGHTGREEN].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[LIGHTCYAN].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[LIGHTCYAN].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[LIGHTRED].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[LIGHTRED].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[LIGHTMAGENTA].rgbColor, UGL_NULL,
                  &pDisplayControl->colorTable[LIGHTMAGENTA].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[YELLOW].rgbColor,  UGL_NULL,
                  &pDisplayControl->colorTable[YELLOW].uglColor, 1);
    uglColorAlloc(pDisplayControl->devId, &pDisplayControl->colorTable[WHITE].rgbColor,  UGL_NULL,
                  &pDisplayControl->colorTable[WHITE].uglColor, 1);

    /* Create Region */

    pDisplayControl->regionId = uglRegionCreate ();
    uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[BLACK].uglColor);
    uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[LIGHTGREEN].uglColor);

    sprintf(regionMessage,"Welcome to Wind River Media Library %d.%d.%d", uglVersionMajor,uglVersionMinor,
            uglVersionPatch);

    uglTextSizeGet(pDisplayControl->fontDialog, &textWidth, &textHeight,
                   -1, regionMessage);
    uglFontSet(pDisplayControl->gc, pDisplayControl->fontDialog);
    uglTextDraw(pDisplayControl->gc, (pDisplayControl->displayWidth - textWidth) / 2,
                (pDisplayControl->displayHeight - textHeight) / 3, -1, regionMessage);

    rect.left = rect.top = 0;
    rect.right = pDisplayControl->displayWidth - 1;
    rect.bottom = pDisplayControl->displayHeight - 1;
    uglRegionRectInclude (pDisplayControl->regionId, &rect);
    rect.right = ((pDisplayControl->displayWidth - textWidth) / 2) + textWidth + 5;
    rect.bottom = ((pDisplayControl->displayHeight - textHeight) / 3) + textHeight + 5;
    rect.left = ((pDisplayControl->displayWidth - textWidth) / 2) - 6;
    rect.top = ((pDisplayControl->displayHeight - textHeight) / 3) - 6;
    uglRegionRectExclude (pDisplayControl->regionId, &rect);
    uglClipRegionSet (pDisplayControl->gc, pDisplayControl->regionId);

    /* Create the brick pattern */

    patternDib.width = patternDib.stride = patternData.width;
    patternDib.height = patternData.height;
    patternDib.pImage = patternData.data;
    pDisplayControl->patternDdb = uglMonoBitmapCreate(pDisplayControl->devId, &patternDib,
                                     UGL_DIB_INIT_DATA, 0, UGL_NULL);

    /* Create standard and transparent DDBs */

    pDisplayControl->colorData = (UGL_COLOR *)UGL_MALLOC(32 * 32 * sizeof(UGL_COLOR));

    for (i = 0; i < 32 * 32; i++) 
        {
        pDisplayControl->colorData[i] = pDisplayControl->colorTable[transparentData[i]].uglColor;
        }

    transDib.pImage = (void *)pDisplayControl->colorData;
    transDib.colorFormat = UGL_DEVICE_COLOR_32;
    transDib.clutSize = 0;
    transDib.pClut = UGL_NULL;
    transDib.imageFormat = UGL_DIRECT;
    transDib.width = transDib.height = transDib.stride = 32;

    pDisplayControl->stdDdb = uglBitmapCreate(pDisplayControl->devId, &transDib, 
                                              UGL_DIB_INIT_DATA, 0, UGL_NULL);

    transMdib.width = transMdib.stride = transMdib.height = 32;
    transMdib.pImage = transparentMask;

    pDisplayControl->transDdb = uglTransBitmapCreate(pDisplayControl->devId, &transDib, &transMdib,
                                                     UGL_DIB_INIT_DATA, 0, UGL_NULL);

    /* Create the cursor */

    if (pDisplayControl->mode >= 0 && pDisplayControl->inputServiceId != UGL_NULL)
        {
        uglCursorInit (pDisplayControl->devId, 32, 32, pDisplayControl->displayWidth / 2, 
                       pDisplayControl->displayHeight / 2);
        cursorDib.width = cursorDib.height = cursorDib.stride = 32;
        cursorDib.hotSpot.x = cursorDib.hotSpot.y = 16;
        cursorDib.pImage = cursorData;
        cursorDib.clutSize = 2;
        cursorDib.pClut = cursorClut;
        pDisplayControl->cursorDdb = uglCursorBitmapCreate(pDisplayControl->devId, &cursorDib);
        uglCursorImageSet (pDisplayControl->devId, pDisplayControl->cursorDdb);

        uglCursorOn(pDisplayControl->devId);
        }

    /* Initialization finished, drawing begins */

    ClearScreen(pDisplayControl);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* BitmapWrite test */

    uglBitmapWrite(pDisplayControl->devId, &transDib, 0,0,31,31,UGL_DISPLAY_ID,100,100);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* DDB blt */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    for (i = 0; i < 1000; i++)
        {
        uglBitmapBlt(pDisplayControl->gc, pDisplayControl->stdDdb,0,0,31,31,UGL_DEFAULT_ID,
                     pDisplayControl->randomData[i], pDisplayControl->randomData[i+1]);
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* TDDB blt */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    for (i = 0; i < 1000; i++)
        {
        uglBitmapBlt(pDisplayControl->gc, pDisplayControl->transDdb,0,0,31,31,UGL_DEFAULT_ID,
                     pDisplayControl->randomData[i],pDisplayControl->randomData[i+1]);
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Simple lines */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    index = 0;
    for (i = 0; i < numRandomPoints / 2; i++)
        {
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 16].uglColor);

        uglLine(pDisplayControl->gc, pDisplayControl->randomData[index], 
                pDisplayControl->randomData[index + 1], 
                pDisplayControl->randomData[index + 2], 
                pDisplayControl->randomData[index + 3]);
        index += 4;
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Dashed lines */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    uglLineStyleSet(pDisplayControl->gc, UGL_LINE_STYLE_DASHED);
    index = 0;
    for (i = 0; i < numRandomPoints / 2; i++)
        {
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 16].uglColor);
        uglLine(pDisplayControl->gc, pDisplayControl->randomData[index], pDisplayControl->randomData[index + 1], 
                pDisplayControl->randomData[index + 2], pDisplayControl->randomData[index + 3]);
        index += 4;
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Wide solid lines */
    
    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    index = 0;
    for (i = 0; i < numRandomPoints / 2; i++)
        {
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 15 + 1].uglColor);
        uglLineWidthSet(pDisplayControl->gc, (i % 6) + 1);
        uglLine(pDisplayControl->gc, pDisplayControl->randomData[index], pDisplayControl->randomData[index + 1], 
                pDisplayControl->randomData[index + 2], pDisplayControl->randomData[index + 3]);
        index += 4;
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Wide dashed lines */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    uglLineStyleSet(pDisplayControl->gc, UGL_LINE_STYLE_DASHED);
    index = 0;
    for (i = 0; i < numRandomPoints / 2; i++)
        {
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 15 + 1].uglColor);
        uglLineWidthSet(pDisplayControl->gc, (i % 6) + 1);
        uglLine(pDisplayControl->gc, pDisplayControl->randomData[index], pDisplayControl->randomData[index + 1], 
                pDisplayControl->randomData[index + 2], pDisplayControl->randomData[index + 3]);
        index += 4;
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Filled rectangles */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    index = 0;
    for (i = 0; i < numRandomPoints / 2; i++)
        {
        int left = min(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int right = max(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int top = min(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        int bottom = max(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 15 + 1].uglColor);
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ 15 - (i % 15)].uglColor);
        uglLineWidthSet(pDisplayControl->gc, (i % 6) + 1);
        uglRectangle(pDisplayControl->gc, left, top , right, bottom);
        index += 4;
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Rectangles filled with a pattern */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    uglFillPatternSet(pDisplayControl->gc, pDisplayControl->patternDdb);

    index = 0;
    for (i = 0; i < numRandomPoints / 15; i++)
        {
        int left = min(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int right = max(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int top = min(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        int bottom = max(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 15 + 1].uglColor);
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ 15 - (i % 15)].uglColor);
        uglLineWidthSet(pDisplayControl->gc, i % 6 + 1);
        uglRectangle(pDisplayControl->gc, left, top , right, bottom);
        index += 4;
        }

    uglFillPatternSet(pDisplayControl->gc, 0);
    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Filled polygons */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    index = 0;
    for (i = 0; i < numRandomPoints / 10; i++)
        {
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 15 + 1].uglColor);
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ 15 - (i % 15)].uglColor);
        pDisplayControl->randomData[index + 18] = pDisplayControl->randomData[index];
        pDisplayControl->randomData[index + 19] = pDisplayControl->randomData[index + 1];
        uglPolygon(pDisplayControl->gc, 10, &pDisplayControl->randomData[index]);
        index += 20;
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Polygons filled with a pattern */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    uglFillPatternSet(pDisplayControl->gc, pDisplayControl->patternDdb);

    index = 0;
    for (i = 0; i < numRandomPoints / 22; i++)
        {
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 15 + 1].uglColor);
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ 15 - (i % 15)].uglColor);
        uglLineWidthSet(pDisplayControl->gc, i % 4 + 1);
        pDisplayControl->randomData[index + 18] = pDisplayControl->randomData[index];
        pDisplayControl->randomData[index + 19] = pDisplayControl->randomData[index + 1];
        uglPolygon(pDisplayControl->gc, 10, &pDisplayControl->randomData[index]);
        index += 20;
        }

    uglFillPatternSet(pDisplayControl->gc, 0);
    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Text */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    y = 0;
    textpage = 0;
    tmp = 0;

    for (i = 0; i < 1000; i++)
        {
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 15 + 1].uglColor);
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ 15 - (i % 15)].uglColor);

        switch (i % 3)
            {
            case 0:
                uglFontSet(pDisplayControl->gc, pDisplayControl->fontSystem);
                uglTextSizeGet(pDisplayControl->fontSystem, UGL_NULL, &tmp, -1, fontTestText);
                break;

            case 1:
                uglFontSet(pDisplayControl->gc, pDisplayControl->fontDialog);
                uglTextSizeGet(pDisplayControl->fontDialog, UGL_NULL, &tmp, -1, fontTestText);
                break;

            case 2:
                uglFontSet(pDisplayControl->gc, pDisplayControl->fontFixed);
                uglTextSizeGet(pDisplayControl->fontFixed, UGL_NULL, &tmp, -1, fontTestText);
                break;
            }

        uglTextDraw(pDisplayControl->gc, 0, y, -1, fontTestText);
        y += tmp;

        if (y >= pDisplayControl->displayHeight)
            {
            y = 0;
            textpage++;
            }
        }

    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Filled Ellipses */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    index = 0;
    for (i = 0; i < numRandomPoints / 2; i++)
        {
        int left = min(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int right = max(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int top = min(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        int bottom = max(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 16].uglColor);
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ 15 - (i % 15)].uglColor);
        uglEllipse(pDisplayControl->gc, left, top, right, bottom, 0, 0, 0, 0);
        index += 4;
        }
    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Pie Shapes */

    ClearScreen(pDisplayControl);

    uglBatchStart(pDisplayControl->gc);

    index = 0;
    for (i = 0; i < numRandomPoints / 4; i++)
        {
        int left = min(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int right = max(pDisplayControl->randomData[index], pDisplayControl->randomData[index + 2]);
        int top = min(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        int bottom = max(pDisplayControl->randomData[index + 1], pDisplayControl->randomData[index + 3]);
        uglForegroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ i % 16].uglColor);
        uglBackgroundColorSet(pDisplayControl->gc, pDisplayControl->colorTable[ 15 - (i % 15)].uglColor);
        uglEllipse(pDisplayControl->gc, left, top, right, bottom, 
                   pDisplayControl->randomData[index + 4], pDisplayControl->randomData[index + 5], 
                   pDisplayControl->randomData[index + 6], pDisplayControl->randomData[index + 7]);
        index += 8;
        }
    uglBatchEnd(pDisplayControl->gc);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    /* Stretch Blits */

    ClearScreen(pDisplayControl);

    uglBitmapStretchBlt(pDisplayControl->gc,pDisplayControl->stdDdb,0,0,31,31,UGL_NULL,100,100,200,150);
   
    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    ClearScreen(pDisplayControl);

    uglBitmapStretchBlt(pDisplayControl->gc,pDisplayControl->stdDdb,0,0,31,31,UGL_NULL,100,100,150,200);
   
    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    ClearScreen(pDisplayControl);

    uglBitmapStretchBlt(pDisplayControl->gc,pDisplayControl->stdDdb,0,0,31,31,UGL_NULL,0,0, 
                        pDisplayControl->displayWidth-1, pDisplayControl->displayHeight-1);

    if(pauseDemo(pDisplayControl) < 0)
        {
        cleanUp(pDisplayControl);
        return(UGL_STATUS_ERROR);
        }

    ClearScreen(pDisplayControl);

    /* Clean Up */
    cleanUp(pDisplayControl);

    return(UGL_STATUS_OK);
    }
