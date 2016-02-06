
#include "osccal.h"
#include "usbdrv.h"
#include <avr/eeprom.h>

#define OSCCAL_INVALID  (0xFF)

/* EEPROM OSCCAL value */
EEMEM const uint8_t OSCCAL_EEP_VALUE = 0xFF;

void OSCCAL_Init(void)
{
    // Load internal RC calibration value
    uint8_t calib = eeprom_read_byte(&OSCCAL_EEP_VALUE);

    if(calib != OSCCAL_INVALID){
        OSCCAL = calib;
    }
}

void OSCCAL_Start(void)
{
    uchar       step = 128;
    uchar       trialValue = 0, optimumValue;
    int         x, optimumDev;
    int         targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

    /* do a binary search: */
    do{
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();  // proportional to current real frequency
        if(x < targetValue)           // frequency still too low
            trialValue += step;
        step >>= 1;
    }while(step > 0);
    /* We have a precision of +/- 1 for optimum OSCCAL here */
    /* now do a neighborhood search for optimum value */
    optimumValue = trialValue;
    optimumDev = x; /* this is certainly far away from optimum */
    for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
        x = usbMeasureFrameLength() - targetValue;
        if(x < 0)
            x = -x;
        if(x < optimumDev){
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }
    OSCCAL = optimumValue;

    eeprom_update_block((void*)&OSCCAL, (void*)&OSCCAL_EEP_VALUE,1);
}
