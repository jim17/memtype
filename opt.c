
#include "opt.h"
#include "crd.h"
#include "uif.h"
#include "uib.h"
#include "fls.h"
#include "led.h"
#include "print.h"
#include <avr/pgmspace.h>

#define PRINTOptFlash(x)    printStr((void*)pgm_read_word(&x), FLASH)
/*-----------------------------START MENU CONFIGURATION */
#define MAX_OPT_SIZE    (sizeof(opt_menuFp)/sizeof(fp_t))

// Local Function prototype
static void opt_previous(void);
static void opt_next(void);
static void opt_apply(void);
static void opt_lock(void);
static void opt_printUser(void);
static void opt_printPass(void);

/*Function pointer to loop inside options Menu */
typedef void (*fp_t)(void);

fp_t const opt_menuFp[] PROGMEM ={
    opt_printPass,opt_printUser,
};

/* Initialize function pointer to NULL */
fp_t fptr = 0;

/*-------------------------- END OF MENU CONFIGURATION */

/* Options Start Message */
const char opt_startStr[] PROGMEM = OPT_START_STR;

/* Options Finite state machine [Start] */
void OPT_fsmStart(void){
    UIF_state = OPTIONS;
    UIF_optionsIndex = 0;
    deleteStr();
    printStr((void*)opt_startStr, FLASH);
    LedBlinkBoth();
}

/* Options Finite state machine */
void OPT_fsm(uint8_t button){

    switch(button) {
    case LEFT:
        opt_lock();
        break;
    case UP:
        opt_previous();
        break;
    case RIGHT:
        CRD_fsmStart();
        break;
    case DOWN:
        opt_next();
        break;
    default:
        return;
    }
}

static void opt_previous(void){
    // select previous credential
    deleteStr();
    UIF_increment(&UIF_optionsIndex, MAX_OPT_SIZE);
    opt_apply();
}

static void opt_next(void){
    UIF_decrement(&UIF_optionsIndex, MAX_OPT_SIZE);
    opt_apply();
}

static void opt_apply(void){
    fptr = (fp_t)pgm_read_word(&(opt_menuFp[UIF_optionsIndex]));
    fptr();
}


static void opt_lock(void){
    deleteStr();
    UIF_Init();
    LedOff();
}

static void opt_printUser(void){
    deleteStr();
    crd_printDetail(CRD_USER, CRD_USER+1);
    LedBlinkGreen();
}
static void opt_printPass(void){
    deleteStr();
    crd_printDetail(CRD_PASS, CRD_PASS+1);
    LedBlinkRed();
}
