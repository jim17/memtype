#ifndef __PRINT_H__
#define __PRINT_H__

#include "hid_defines.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

#define SYNCHRONOUS_IDLE_KEY    (0x16u) // \x16 ASCII code
#define IDLE_TIMEOUT            (50u)   // 10ms x 250 = 500ms
#define INIT_DEBOUNCE           (50u)   // 10ms x 50 = 500ms

#define printStr(a,b)   printStr_##b(a)

void pressKeyByChar(char);
void releaseKeys(void);
void printUpdate(void);
void printStr_RAM(char *str);
void printStr_FLASH(char *str);
void print_nStr(char* str, uint8_t len);
void IntToChar(uchar,uchar*); //int,tochar
void deleteStr(void); //deletes last string
void deleteNStr(uchar); //deletes n characters

typedef struct
{
    uint8_t reportid;
    uint8_t modifier;
    uint8_t keycode;
} keyboard_report_t;

extern keyboard_report_t reportBuffer;

enum
{
    EMPTY = 0, //EMPTY, can be used
    RAM,        //STRING IN RAM
    FLASH,      //STRING IN FLASH
};

#endif /* __PRINT_H__ */
