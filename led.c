
#include <avr/io.h>
#include "led.h"
#include "ucp.h"

void LED_Task(void){
    if(UCP_state == DEVICE_LOCKED) {
        LED_ON(LED_RED);
        LED_OFF(LED_GREEN);
    }else{
        LED_OFF(LED_RED);
        LED_ON(LED_GREEN);
    }
}
