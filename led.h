
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

/* Main LED Task function */
extern void LED_Task(void);
