
#include "uif.h"
#include "usi.h"
#include "uib.h"
#include "crd.h"
#include "opt.h"
#include "print.h"
#include "version.h"

/* public variable */
uint8_t UIF_state = START;
uint8_t UIF_credIndex = 0;
uint8_t UIF_optionsIndex = 0;
uint8_t UIF_userInputIndex = 0;

const char uif_initStr[] PROGMEM = MEMTYPE_VERSION_STR;

void UIF_Init(void)
{
    printStr((void*)uif_initStr, FLASH);
    return;
}

void UIF_Task(void)
{
      if((UIB_buttonChanged == 1u) && (UIB_buttonPressed != NOT_PRESSED))
      {
            switch(UIF_state)
            {
                case START:
                    UIF_state = USER_INPUT;
                    deleteStr();
                    USI_Init();
                    break;
                case OPTIONS:
                    OPT_fsm(UIB_buttonPressed);
                    break;
                case USER_INPUT:
                    USI_fsm(UIB_buttonPressed);
                    break;
                default: //CREDENTIALS
                    CRD_fsm(UIB_buttonPressed);
                    break;
            }
      }

    return;
}

void UIF_increment(uint8_t* val, uint8_t max)
{
    *val = ((*val)+1) % max;
}


void UIF_decrement(uint8_t* val, uint8_t max)
{
    if((*val) == 0){*val = (max-1);}
    else {*val = (*val)-1;}
}
