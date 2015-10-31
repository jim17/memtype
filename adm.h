
#ifndef _ADM_H_
#define _ADM_H_

#include <stdint.h>

/* ADC Max value */
#define ADC_MAX     (0xFFu)

/* Macro Like function */
#define ADM_GetAdcValue() adm_ADCValue

/* Function prototype */
void ADM_Init(void);
void ADM_Task(void);

extern uint8_t adm_ADCValue;

#endif /* _ADM_H_ */
