/* USI - USER INPUT state machine */
#include "usi.h"
#include "uif.h"
#include "uib.h"
#include "crd.h"
#include "ucp.h"
#include "print.h"

/** Global Data */
// PIN: 0000 default HASH
const uint8_t LOCK_HASH[16] EEMEM = {
    0xd4,0x4f,0xb2,0x7a,0x58,0xb4,0x27,0x4a,0x21,0xe6,0x8f,0x39,0x69,0x74,0x23,0x54
};

/** Local Data */
static char userText[16];
static char userPin[4];
static uint8_t userTextIndex;
static const char USI_keys[] PROGMEM = {'0','1','2','3','4','5','6','7','8','9'};
static const char PIN_str[] PROGMEM = "PIN: ";
static const char LOCKED_str[] PROGMEM = "PIN ERR";
static const uint8_t LOCK_HASH_PGM[16] PROGMEM = {
    0xd4,0x4f,0xb2,0x7a,0x58,0xb4,0x27,0x4a,0x21,0xe6,0x8f,0x39,0x69,0x74,0x23,0x54
};

/** Private Function declaration */
static void usi_print(void);
static void usi_previous(void);
static void usi_next(void);
static uint8_t usi_pinCheck(char pin[4]);

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
        print_deleteStr();
        USI_Init();
        break;
    case UP:
        print_deleteStr();
        usi_next();
        break;
    case RIGHT:
        print_deleteStr();
        if(userTextIndex == (sizeof(userPin)-1))     // real array elements (\0)
        {
            /* Device Unlocked */
            if(usi_pinCheck(userPin) == 1)
            {
                CRD_fsmStart();
            }
            /* Device Locked */
            else
            {
                printStr((void*)LOCKED_str,FLASH);
                userTextIndex = 0;
                UIF_userInputIndex = 0;
                userPin[0] = '0';
                memcpy_P((void*)userText, (void*)PIN_str, sizeof(PIN_str));
            }
        }
        else
        {
            userText[sizeof(PIN_str)-1+userTextIndex] = '*';
            userTextIndex++;
            usi_print();
        }
        break;
    case DOWN:
        print_deleteStr();
        usi_previous();
        break;
    default:
        break;
    }
}

static void usi_print(void){
    userPin[userTextIndex] = pgm_read_byte(&USI_keys[UIF_userInputIndex]);
    userText[sizeof(PIN_str)-1+userTextIndex] = userPin[userTextIndex];
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

static uint8_t usi_pinCheck(char pin[4]){
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
