#include "print.h"
#include "crd.h"

#define STRBUF_SIZE (5u)

typedef enum
{
    INIT = 0,
    PRESSED,
    RELEASED,
    IDLE,
    DELETE,
    DELETE_PRESSED,
}kbd_status_t;


typedef struct
{
    char *strPtr;
    uint8_t info;
    uint8_t len;
    uint8_t callback;

}strPtr_t;

strPtr_t strBuf[STRBUF_SIZE];

uint8_t actString; //Cadena actual
uint8_t cCount = 0; //Delete counter
uint8_t EOS=0; //EndOfString
uint8_t debounceCount = 0; //counter
keyboard_report_t reportBuffer;
kbd_status_t kbd_status = INIT;

/** Private Function prototype */
static void print_pressKey(char);
static void print_releaseKey(void);
static void print(char * str,uint8_t isRam, uint8_t len);

void print_pressKey(char ascii){
    //ascii to key
    char key = KEYBOARD_READ_BYTE((void*)(keyboardLUT_ES+ascii));

    //setup reportBuffer
    reportBuffer.modifier  = ((SHIFT_MASK) &key) >> 6;
    reportBuffer.modifier |= (ALTGR_MASK) &key;
    // remapping of KEY_EUROPE_2 to original value
    key = (~(SHIFT_MASK|ALTGR_MASK)) & key;
    if(key == KEY_EUROPE_2) {
        reportBuffer.keycode = KEY_EUROPE_2_ORG;
    } else {
        reportBuffer.keycode = key;
    }
}

static void print_releaseKey(void){
    //Initializing
    reportBuffer.modifier=0;
    reportBuffer.keycode=0;
}

static void print(char * str,uint8_t isRam, uint8_t len){
    uint8_t cont=0;
    uint8_t index;
    while(cont < sizeof(strBuf))
    {
        index = (actString+cont)%STRBUF_SIZE;
        if(strBuf[index].info == EMPTY)
        {
            strBuf[index].info = isRam;
            strBuf[index].strPtr = str;
            strBuf[index].len = len;
            if(len != 0) {
                strBuf[index].callback = 1;
            }
            else{
                strBuf[index].callback = 0;
            }
            break;
        }
        cont++;
    }
}

void printStr_RAM(char *str){
    print((void*)str, RAM, 0);
}

void printStr_FLASH(char *str){
    print((void*)str, FLASH, 0);
}

void print_nStr(char* str, uint8_t len){
    print((void*)str, RAM, len);
}

void printUpdate(void){
    char ascii = 0;

    if(strBuf[actString].info != EMPTY)
    {
        if(strBuf[actString].info == RAM) {
            ascii = *(strBuf[actString].strPtr); //READ CHAR
        }else if(strBuf[actString].info == FLASH) {
            ascii = pgm_read_byte(strBuf[actString].strPtr);//READ CHAR
        }
    }

    switch(kbd_status) {
    case INIT:     //Initial debouncing
        if(debounceCount < INIT_DEBOUNCE)
        {
            print_releaseKey();
            debounceCount++;

        }else{
            kbd_status=RELEASED;
            debounceCount = 0;
        }
        break;
    case PRESSED:     //We release
        print_releaseKey();
        kbd_status = RELEASED;
        if(strBuf[actString].len > 0) {
            strBuf[actString].len--;
        }
        break;
    case RELEASED:     //We can PROCESS CHAR

        if( (strBuf[actString].callback == 0) && (ascii == 0) )    //EOS
        {
            EOS=1;
            strBuf[actString].info = EMPTY;
            actString = (actString+1)%STRBUF_SIZE;
        }
        else if( (strBuf[actString].callback != 0) && (strBuf[actString].len == 0) )
        {

            strBuf[actString].info = EMPTY;
            CRD_apply();
            if(strBuf[actString].info == EMPTY)
            {
                EOS=1;
            }
        }
        else{
            strBuf[actString].strPtr++;     //Increment pointer

            //Do we have to enter IDLE state?
            if(ascii == SYNCHRONOUS_IDLE_KEY)
            {
                kbd_status = IDLE;
            }else{
                print_pressKey(ascii);
                if(EOS==0) {    //not first char
                    cCount++;    //new char to delete
                }else{      //first char of string
                    EOS=0;
                    cCount = 1;
                }
                kbd_status = PRESSED;
            }
        }
        break;
    case IDLE:
        if(debounceCount < IDLE_TIMEOUT)
        {
            print_releaseKey();
            debounceCount++;
        }else{
            kbd_status=RELEASED;
            debounceCount = 0;
            if(strBuf[actString].len > 0) {
                strBuf[actString].len--;
            }
        }
        break;

    case DELETE:
        if(cCount > 0)
        {       //DELETING
            print_pressKey(0x08);
            cCount--;
            kbd_status=DELETE_PRESSED;
        }else{     //DONE ERASING
            print_releaseKey();
            //strBuf[actString].info = EMPTY;
            kbd_status=RELEASED;
            if(EOS==0)     //if half word printing
            {
                EOS=1;    //INVALIDATE CURRENT TYPING
                strBuf[actString].info = EMPTY;
                actString = (actString+1)%STRBUF_SIZE;
            }
        }
        break;
    case DELETE_PRESSED:
        print_releaseKey();
        kbd_status=DELETE;
        break;
    }
}

void print_deleteStr(void){
    kbd_status=DELETE;
}
