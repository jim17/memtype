
/** User Input Button api */

#ifndef _UIF_H_
#define _UIF_H_

#include <stdint.h>

/*  UIF MENU STATES */
enum
{
    START = 0,
    CREDENTIALS,
    OPTIONS,
    USER_INPUT,
} menuStates;

/* Public vars */
extern uint8_t UIF_state;
extern uint8_t UIF_credIndex;
extern uint8_t UIF_optionsIndex;
extern uint8_t UIF_userInputIndex;

/* Public functions */
void UIF_Init(void);
void UIF_Task(void);
void UIF_increment(uint8_t* val, uint8_t max);
void UIF_decrement(uint8_t* val, uint8_t max);

#endif /* _UIF_H_*/
