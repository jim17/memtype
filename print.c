#include "print.h"

extern volatile uint8_t USB_keycode;
extern volatile uint8_t USB_modifier;

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
 uchar info;
 uint8_t len;
 uint8_t callback;
 
}strPtr_t;

strPtr_t strBuf[5];

uchar actString; //Cadena actual
uchar cCount = 0; //Delete counter
uchar EOS=0; //EndOfString
uchar debounceCount = 0; //counter
keyboard_report_t reportBuffer;
kbd_status_t kbd_status = INIT;

extern void crd_apply(void);

void pressKeyByChar(char ascii)
{
    //Initializing
    reportBuffer.modifier=0;
    reportBuffer.keycode=0;

    //ascii to key
    char key = KEYBOARD_READ_BYTE((void*)(keyboardLUT_ES+ascii));
    
    //setup reportBuffer
    reportBuffer.modifier  = ((SHIFT_MASK) & key) >> 6;
    reportBuffer.modifier |= (ALTGR_MASK) & key;
    reportBuffer.keycode  = (~(SHIFT_MASK|ALTGR_MASK)) & key;
}

void pressKey(uint8_t key, uint8_t modifier)
{
    //Initializing
	USB_keycode = key;
	USB_modifier = modifier;
}

void releaseKeys(void)
{
   //Initializing
   reportBuffer.modifier=0;
   reportBuffer.keycode=0;
}

static void print(char * str,uchar isRam, uint8_t len){
    uchar cont=0;
    uchar index; 
    while(cont < sizeof(strBuf))
    {
        index = (actString+cont)%5;
        if(strBuf[index].info == EMPTY)
        {
            strBuf[index].info = isRam;
            strBuf[index].strPtr = str;
            strBuf[index].len = len;
            if(len != 0) strBuf[index].callback = 1;
            else strBuf[index].callback = 0;
            break;
        }
        cont++;
    }
}

void printStr_RAM(char *str)
{
    print((void*)str, RAM, 0);
}

void printStr_FLASH(char *str)
{
    print((void*)str, FLASH, 0);
}

void print_nStr(char* str, uint8_t len)
{
    print((void*)str, RAM, len);
}

void IntToChar(uchar number,uchar * tostr)
{
        tostr[0] = (number/100)+'0';
        number %= 100;
        tostr[1] = (number/10)+'0';
        tostr[2] = (number%10)+'0';
        tostr[3] = '\0'; //End of String

}

void printUpdate(void)
{
    char ascii = 0;

    if(strBuf[actString].info != EMPTY)
    {   
        if(strBuf[actString].info == RAM){
            ascii = *(strBuf[actString].strPtr); //READ CHAR
        }else if(strBuf[actString].info == FLASH){
            ascii = pgm_read_byte(strBuf[actString].strPtr);//READ CHAR
        }
    }

    switch(kbd_status){
        case INIT: //Initial debouncing
            if(debounceCount < INIT_DEBOUNCE)
            {
                 releaseKeys();
                 debounceCount++;
                 
            }else{
                kbd_status=RELEASED;
                debounceCount = 0;
            }
            break;
        case PRESSED: //We release
            releaseKeys();
            kbd_status = RELEASED;
            if(strBuf[actString].len > 0) strBuf[actString].len--;
            break;
        case RELEASED: //We can PROCESS CHAR

            if( (strBuf[actString].callback == 0) && (ascii == 0) )//EOS
            {
                EOS=1;
                strBuf[actString].info = EMPTY;
                actString = (actString+1)%5;
            }
            else if( (strBuf[actString].callback != 0) && (strBuf[actString].len == 0) )
            {

                strBuf[actString].info = EMPTY;
                crd_apply();
                if(strBuf[actString].info == EMPTY)
                {
                    EOS=1;
                }
            }
            else{
                strBuf[actString].strPtr++; //Increment pointer

                //Do we have to enter IDLE state?
                if(ascii == SYNCHRONOUS_IDLE_KEY)
                {
                    kbd_status = IDLE;
                }else{
                    pressKeyByChar(ascii);
                    if(EOS==0){ //not first char
                        cCount++;//new char to delete    
                     }else{ //first char of string
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
                 releaseKeys();
                 debounceCount++;
             }else{
                 kbd_status=RELEASED;
                 debounceCount = 0;
             }
            break;

        case DELETE:
            if(cCount > 0)
            {   //DELETING
                pressKeyByChar(0x08);
                cCount--;
                kbd_status=DELETE_PRESSED;
            }else{ //DONE ERASING
                releaseKeys();
                //strBuf[actString].info = EMPTY;
                kbd_status=RELEASED;
                if(EOS==0) //if half word printing
                {
                   EOS=1; //INVALIDATE CURRENT TYPING
                   strBuf[actString].info = EMPTY;
                   actString = (actString+1)%5;
                }
            }
            break;
        case DELETE_PRESSED:
            releaseKeys();
            kbd_status=DELETE;
            break;
    }
}

void deleteStr(void)
{
    kbd_status=DELETE;
}
void deleteNStr(uchar n)
{
    cCount = n;
    kbd_status=DELETE;
}
