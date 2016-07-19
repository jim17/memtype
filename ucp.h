
/** UCP - USB Communication Protocol api */

#ifndef _UCP_H_
#define _UCP_H_

#include <stdint.h>
#include "led.h"

/* PROTOCOL COMMANDS */
#define UCP_CMD_ERROR   0x00
#define UCP_CMD_RESET   0x01
#define UCP_CMD_READ    0x02
#define UCP_CMD_WRITE   0x03
#define UCP_CMD_DATA    0x04
#define UCP_CMD_INFO    0x05
#define UCP_CMD_READ_RAM     0x06
#define UCP_CMD_SET_PIN     0x07
#define UCP_CMD_READ_PIN  0x08
#define UCP_CMD_KEYBOARD 0x09

/* ERROR CODES */
#define UCP_ERR             0xF0 /* generic error */
#define UCP_ERR_PACKET      0xF1 /* invalid packet */
#define UCP_ERR_CMD         0xF2 /* command error */
#define UCP_ERR_ADDR        0xF3 /* wrong address error */
#define UCP_ERR_SIZE        0xF4 /* wrong size error */
#define UCP_ERR_PROTOCOL    0xF5 /* protocol error */
#define UCP_ERR_LOCKED      0xF6 /* device is locked */

/* UCP STATES */
enum
{
    IDLE = 0,
    READING_CMD,
    READING,
    WRITTING,
    DEVICE_LOCKED,
    WRITE_EEPROM,
} ucpStates;

/* Specific protocol */
typedef struct
{
    //uint8_t cmd[1];
   uint8_t buf[8];
} ucp_pkt_t; /* basic packet */

/* Public vars */
extern uint8_t UCP_state;
extern ucp_pkt_t customReport ;//= {.buff = "hello\r\n"};

/* Public functions */
void UCP_Init(void);
void UCP_Task(void);
void UCP_Decode(uint8_t *data, uint8_t len);
void UCP_WriteTask(void);

/* Macros */
#define UCP_Unlock() { UCP_state = IDLE; LedGreen(); }
#define UCP_Lock()   { UCP_state = DEVICE_LOCKED; LedRed(); }

#endif /* _UCP_H_ */
