
/** User Input Button api */

#ifndef _UIB_H_
#define _UIB_H_

#include <stdint.h>

/* ADC INPUT BUTTON STATES */
enum
{
    NOT_PRESSED = 0,
    DOWN,
    RIGHT,
    UP,
    LEFT
} buttonStates;

/* Public variables */
extern uint8_t UIB_buttonPressed;
extern uint8_t UIB_buttonChanged;

/* Public functions */
void UIB_Init(void);
void UIB_Task(void);

#endif /* _UIB_H_*/
