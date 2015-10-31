
#define LED_DDR     (DDRB)
#define LED_PORT    (PORTB)
#define LED         (1)

#define LED_Init()      LED_OUTPUT()
#define LED_OUTPUT()    (LED_DDR |= (1<<LED))
#define LED_HIGH()      (LED_PORT |= (1<<LED))
#define LED_LOW()       (LED_PORT &= ~(1<<LED))
#define LED_TOGGLE()    (LED_PORT ^= (1<<LED))
