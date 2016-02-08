
/* Task scheduler to execute tasks at a cyclic predefined time */
#include "sch.h"
#include <stdint.h>
#include <avr/io.h>
#include "led.h"
#include "uib.h"
#include "uif.h"

/* Timer 1 Config Register Configuration */
/* 16,5 Mhz /16384 = 1007 Hz, it's almost 1kHz so it's fine */
#define sch_cfg_r   TCCR1
#define sch_CTC     (0x00u) /* [7] Clear timer/counter on compare match */
#define sch_PWM     (0x00u) /* [6] Pulse width modulator enable (disabled) */
#define sch_CP      (0x00u) /* [5:4] Comparator output, (disconnected from OC1A) */
#define sch_PR      (0x0Fu) /* [3:0] CS13:10 = 0x0F */

/* Timer 1 Control Register */
#define sch_ctrl_r  GTCCR
#define sch_value   (0x00u) /* [7:0] Default value, all features disabled */

/* Timer 1 Counter Register */
#define sch_timer   TCNT1

uint8_t sch_lastTimerVal;
uint8_t sch_tick; /* incremented each 1ms */

/* Private Functions */
static void sch_Task_1ms(void){
    UIB_Task();
    UIF_Task(); /* UIF must be called in the same task as UIB task or higher period */
}

/* Public Functions */
/* Here configure the timer to be used */
void SCH_Init(void){
    sch_cfg_r = (sch_CTC) | (sch_PWM) | (sch_CP) | (sch_PR);
    sch_ctrl_r = (sch_value);
    sch_lastTimerVal = 0u;
    sch_tick = 0u; /* incremented each 1ms */
}

/* Cyclic Task where the different tasks are executed */
void SCH_Task(void){
    uint8_t sch_timer_temp = sch_timer;

    if(sch_lastTimerVal != sch_timer_temp)
    {
        sch_tick++;
        sch_lastTimerVal = sch_timer_temp;

        if( (sch_tick % 1) == 0 )
        {
            sch_Task_1ms();
        }
    }

}
