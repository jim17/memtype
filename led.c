
#include <avr/io.h>
#include "led.h"

ledStatus_t led;

void LED_Task(void){
    static uint8_t i = 0;

    if( (i++ % 16) == 0){
        if(led == RED) {
            LED_ON(LED_RED);
            LED_OFF(LED_GREEN);
        } else if (led == GREEN){
            LED_OFF(LED_RED);
            LED_ON(LED_GREEN);
        } else if (led == BLINK_BOTH){
            LED_TOGGLE(LED_RED);
            LED_TOGGLE(LED_GREEN);
        }else if (led == BLINK_RED){
            LED_TOGGLE(LED_RED);
            LED_OFF(LED_GREEN);
        } else if(led == BLINK_GREEN){
            LED_OFF(LED_RED);
            LED_TOGGLE(LED_GREEN);
        } else{
            LED_OFF(LED_RED);
            LED_OFF(LED_GREEN);
        }
    }
}
