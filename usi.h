#ifndef _USI_H_
#define _USI_H_

#include <avr/eeprom.h>
#include <stdint.h>

// prototype definition
void USI_fsm(uint8_t button);
void USI_Init(void);

// HASH
extern const uint8_t LOCK_HASH[16] EEMEM;
#endif
