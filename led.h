
#ifndef _LED_H_
#define _LED_H_

#define LED_DDR     (DDRB)
#define LED_PORT    (PORTB)
#define LED_RED     (1)
#define LED_GREEN   (0)

#define LED_Init()         LED_OUTPUT()
#define LED_OUTPUT()       (LED_DDR |= (1<<LED_RED) | (1<<LED_GREEN))
#define LED_HIGH(LED)      (LED_PORT |= (1<<LED))
#define LED_LOW(LED)       (LED_PORT &= ~(1<<LED))
#define LED_ON             LED_HIGH
#define LED_OFF            LED_LOW
#define LED_TOGGLE(LED)    (LED_PORT ^= (1<<LED))

#define LedOff()            {led = OFF;         }
#define LedRed()            {led = RED;         }
#define LedGreen()          {led = GREEN;       }
#define LedBlinkRed()       {led = BLINK_RED;   }
#define LedBlinkGreen()     {led = BLINK_GREEN; }
#define LedBlinkBoth()      {led = BLINK_BOTH;  }

/* Typedefs */
typedef enum{
    OFF,
    RED,
    GREEN,
    BLINK_RED,
    BLINK_GREEN,
    BLINK_BOTH
} ledStatus_t;

extern uint8_t led;

/* Main LED Task function */
extern void LED_Task(void);

#endif /* _LED_H_ */
