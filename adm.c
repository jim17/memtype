
#include <stdint.h>
#include <avr/io.h>

/* Multiplexer Selection Register Configuration */
#define adm_cfg_R   ADMUX
#define adm_VREF    0x00 /* [7:6],[4] Voltage Reference VCC */
#define adm_ADLAR   (1 << 5) /* [5] Left Adjustment, Meaning -> ADCH [15:8] ADCL[7:6] */
#define adm_ADCCH   0x01 /* [3:0] Single Ended Channel 1 */

/* ADC Control Status Register A */
#define adm_ctrl_R0     ADCSRA
#define adm_ENABLE      (1 << 7) /* [7] ADC Enable ADC */
#define adm_START       (1 << 6) /* [6] ADC Start conversion */
#define adm_TRG         (1 << 5) /* [5] ADC Auto Trigger enable */
#define adm_VALRDY      (1 << 4) /* [4] ADC Interrupt Flag (cleared with 1) */
#define adm_INTEN       (0 << 3) /* [3] ADC Interrupt Enable */
#define adm_CONVTIME    0x07 /* [2:0] ADC Preescaler x128 */

/* ADC Control Status Register B */
#define adm_ctrl_R1     ADCSRB
#define adm_BIN         0x00 /* [7] ADC Bipolar input mode */
#define adm_IPR         0x00 /* [5] ADC Input polarity reversal */
#define adm_TRGSRC      0x00 /* [2:0] ADC Trigger Source, 000 - Free Running Mode */

/* ADC New Value Available */
#define adm_newVal()    (adm_ctrl_R0 & adm_VALRDY)

/* ADC New Value Clean */
#define adm_newValClean() {adm_ctrl_R0 |= adm_VALRDY;}

/* ADC Value */
#define adm_Val()   (ADCH)

/* Local Variable */
uint8_t adm_ADCValue;

void ADM_Init(void)
{
    DDRB &= ~(1 << 2);
    PORTB &= ~(1 << 2);
    
    adm_cfg_R = (adm_VREF | adm_ADLAR | adm_ADCCH);

    adm_ctrl_R0 = (adm_ENABLE | adm_START | adm_TRG | adm_VALRDY | adm_INTEN | adm_CONVTIME);

    adm_ctrl_R1 = (adm_BIN | adm_IPR | adm_TRGSRC);
}

void ADM_Task(void)
{
    if (adm_newVal() > 0u)
    {
        adm_newValClean();
        adm_ADCValue = adm_Val();
    }
}

