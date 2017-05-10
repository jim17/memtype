
#include "crd.h"
#include "opt.h"
#include "uif.h"
#include "uib.h"
#include "ucp.h"
#include "fls.h"
#include "print.h"

#define PRINTCrdFlash(x)    printStr((void*)x, FLASH)
#define PRINTCrdRAM(x)      printStr((void*)x, RAM)

// Local Function prototype
static void crd_print(void);
static void crd_previous(void);
static void crd_next(void);
static void crd_loadCredential(uint16_t cred);
static uint16_t crd_findEOS(uint16_t offset);
static uint8_t crd_find(uint8_t* start, uint8_t len);

cipher_t cipher;

// Credential structure
const uint8_t credentials[MAX_CRED_FLASH_SIZE] PROGMEM = {
    0x57,0x65,0x6c,0x63,0x6f,0x6d,0x65,0x00,0x3a,0x00,0x79,0x62,0x6f,0xc8,0x0f,0xeb,0x16,0x21,0x95,0xdd,0x0f,0x42,0xe5,0x33,0x9f,0xb7,0x8d,0x43,0xb4,0xaa,0xb7,0xac,0xf0,0xe3,0xc4,0x71,0xcd,0xe1,0xa2,0x4f,0x15,0x33,0x65,0x41,0x1d,0xee,0xac,0xf3,0x5c,0x00,0xd1,0x8f,0x62,0x28,0x79,0x61,0xf9,0xba,0x41,0x62,0x6f,0x75,0x74,0x00,0x72,0x00,0x16,0xdc,0x40,0xd1,0xe4,0x36,0x4d,0xa7,0x8a,0x3d,0x5c,0x19,0x94,0xe1,0x88,0xdf,0xbf,0x68,0x01,0xac,0x46,0x8c,0x4b,0x9d,0x01,0x55,0x24,0x07,0x85,0x80,0xa0,0x62,0x39,0xab,0x03,0xc8,0x84,0x64,0xfa,0xf1,0xa3,0xb7,0x74,0x7b,0xa3,0xf3,0x05,0xcb,0x52,0x6f,0x75,0x74,0x65,0x72,0x20,0x44,0x65,0x66,0x61,0x75,0x6c,0x74,0x00,0x00,0x00,0x08,0xa9,0x43,0x69,0xd2,0x61,0x54,0xe0,0x6c,0x18,0xd5,0x72,0x4b,0x0d,0xe6,0x62,0xeb,0x74,0xbb,0x43,0x3c,0x16,0x5a,0x04,0xd0,0x87,0xd0,0x35,0x36,0x66,0xca,0x72,0x00,0x00,0x00,0x00,0x00
};

/* Credentials Start Message */
const char crd_startStr[] PROGMEM = CRD_START_STR;
credentialEncryptedFlash_t credential;

extern volatile uint8_t tempBuff[SPM_PAGESIZE];
uint8_t position = 0;
uint8_t counter = 0;
credPosition_t crdStart;
credPosition_t crdStop;

/* Init CRD */
void CRD_Init(void){
    //First credential initialization
    crd_loadCredential(0);
}

/* Credentials Finite state machine [Start] */
void CRD_fsmStart(void){
    UIF_state = CREDENTIALS;
    print_deleteStr();
    crd_print();
    UCP_Unlock();
}

/* Credentials Finite state machine */
void CRD_fsm(uint8_t button){

    print_deleteStr();

    switch(button) {
    case LEFT:
        OPT_fsmStart();
        break;
    case UP:
        crd_previous();
        break;
    case RIGHT:
        CRD_printDetail(CRD_USER, CRD_END);
        break;
    case DOWN:
        crd_next();
        break;
    default:
        return;
    }
}

void CRD_apply(){
    if (counter >= crdStop) {
        return;
    }

    uint8_t size;
    uint8_t buffpos = (position % 16);
    uint8_t pending = 16-(buffpos);


    // STEP - 1 (decrypt), only decrypt if there's no more bytes pending in plain buffer
    if(buffpos == 0)
    {
        memcpy_P((void*)cipher.plain,(void*)(credential.encrypt+position), 16);
        noekeon_decrypt();
    }

    // STEP - 2 find how many characters in first decrypted block
    size = crd_find((void*)cipher.plain+buffpos, pending);

    // STEP - 3 print the credentials
    if ( (counter >= crdStart) && (size != 0) )
    {
        print_nStr((void*)cipher.plain+buffpos, size);
    }
    else
    {
        /* special case when the first character is \0 */
        print_nStr("", 1);
    }

    if (size < pending)
    {
        counter++;
        position++; // add \0
    }

    position += size;
}

/* Writes the credential */
void CRD_printDetail(uint8_t start, uint8_t stop){
    position = 0;
    counter = 0;
    crdStart = start;
    crdStop = stop;
    CRD_apply();
}

static void crd_print(void){
    PRINTCrdFlash(credential.name);
}

static void crd_previous(void){
    uint16_t currentcred = (credential.name-(uint16_t)credentials);
    uint16_t lastcred = 0;

    while(credential.next != currentcred )
    {
        lastcred = credential.next;
        crd_loadCredential(credential.next);
    }
    // select previous valid credential
    crd_loadCredential(lastcred);
    crd_print();
}

static void crd_next(void){
    // select next valid credential
    crd_loadCredential(credential.next);
    crd_print();
}

/* load a credential from flash to working credential */
static void crd_loadCredential(uint16_t cred){
    uint16_t offset = cred+(uint16_t)credentials; //counter

    credential.name = offset;
    offset = crd_findEOS(offset); //Find next EOS
    credential.next = pgm_read_word(offset);
    //offset = crd_findEOS(offset); //Find next EOS
    credential.encrypt = offset+2;
}

/* return EOS '\0' position + 1 */
static uint16_t crd_findEOS(uint16_t offset){
    char c;
    uint16_t off = offset;
    c = pgm_read_byte(off);
    while(c!='\0')
    {
        off++;
        c = pgm_read_byte(off);
    }
    off++;
    return off;
}

/* return position of first EOS, len must be lower than 255*/
static uint8_t crd_find(uint8_t* start, uint8_t len){
    uint8_t i;
    uint8_t* ptr = start;

    for(i=0; i<len; i++){
        if(ptr[i] == '\0') {
            break;
        }
    }

    return i;
}
