/* USI - USER INPUT state machine */
#include "usi.h"
#include "uif.h"
#include "uib.h"
#include "crd.h"
#include "ucp.h"
#include "print.h"

static char userText[32];
static uint8_t userTextIndex = 0;
const char USI_keys[] PROGMEM = {
    '0','1','2','3','4','5','6','7','8','9'
};                                                                        //,'A','B','C','D','E','F'};
//"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
const char LOCK[] PROGMEM = "0000";
const char PIN_str[] PROGMEM = "PIN: ";
const char LOCKED_str[] PROGMEM = "PIN ERR";
// PIN: 0000 default HASH
uint8_t LOCK_HASH[16] EEMEM = {
    0xd4,0x4f,0xb2,0x7a,0x58,0xb4,0x27,0x4a,0x21,0xe6,0x8f,0x39,0x69,0x74,0x23,0x54
};
uint8_t USI_pinCheck(char pin[4]);

static void usi_print(void){
    userText[sizeof(PIN_str)-1+userTextIndex] = pgm_read_byte(&USI_keys[UIF_userInputIndex]);
    userText[sizeof(PIN_str)-1+userTextIndex+1] = 0;
    printStr(userText,RAM);
}

static void usi_previous(void){
    UIF_decrement(&UIF_userInputIndex, sizeof(USI_keys));
    usi_print();
}

static void usi_next(void){
    UIF_increment(&UIF_userInputIndex, sizeof(USI_keys));
    usi_print();
}

void USI_Init(void){
    userTextIndex = 0;
    UIF_userInputIndex = 0;
    memcpy_P((void*)userText, (void*)PIN_str, sizeof(PIN_str));
    usi_print();
    UCP_Lock();
}

// user input finitestate machine
void USI_fsm(uint8_t button){
    switch(button) {
    case LEFT:
        deleteStr();
        USI_Init();
        break;
    case UP:
        deleteStr();
        usi_next();
        break;
    case RIGHT:
        deleteStr();
        if(userTextIndex == (sizeof(LOCK)-2))     // real array elements (\0)
        {
            /* Device Unlocked */
            //if(strcmp_P(userText+sizeof(PIN_str)-1, LOCK) == 0)
            if(USI_pinCheck(userText+sizeof(PIN_str)-1) == 1)
            {
                CRD_fsmStart();
            }
            /* Device Locked */
            else
            {
                printStr((void*)LOCKED_str,FLASH);
                userTextIndex = 0;
                UIF_userInputIndex = 0;
                memcpy_P((void*)userText, (void*)PIN_str, sizeof(PIN_str));
                userText[sizeof(PIN_str)-1+userTextIndex] = pgm_read_byte(&USI_keys[UIF_userInputIndex]);
                userText[sizeof(PIN_str)-1+userTextIndex+1] = 0;
            }
        }
        else
        {
            userTextIndex++;
            usi_print();
        }
        break;
    case DOWN:
        deleteStr();
        usi_previous();
        break;
    default:
        break;
    }
}

uint8_t USI_pinCheck(char pin[4]){
    uint8_t i;

    for(i=0; i<16; i++) {
        cipher.key[i] = pin[(i%4)];
        cipher.plain[i] = pin[(i%4)];
    }
    noekeon_encrypt();

    for(i=0; i<16; i++)
    {
        if(eeprom_read_byte(LOCK_HASH+i) != cipher.plain[i]) {
            break;
        }
    }
    return i==16;
}
