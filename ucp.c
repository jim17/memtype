#include "ucp.h"
#include "usi.h"
#include "version.h"
#include "crd.h"
#include "fls.h"
#include "hid_defines.h"

#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>

/* public variables */
uint8_t UCP_state;
ucp_pkt_t customReport;

volatile uint8_t tempBuff[SPM_PAGESIZE];
volatile uint16_t flashWriteAddr;
volatile uint8_t buffIndex;
volatile uint8_t usbWrite = 0;
volatile uint8_t usbWriteAccepted = 0;

/* INFO Data, the first byte is for cmd */
typedef struct
{
    uint8_t versionMajor;
    uint8_t versionMinor;
    uint8_t versionPatch;
    uint16_t credSize;
    uint16_t dummy;
    
} info_t;
        
const info_t INFO_data PROGMEM = 
{
    .versionMajor = MEMTYPE_VERSION_MAJOR,
    .versionMinor = MEMTYPE_VERSION_MINOR,
    .versionPatch = MEMTYPE_VERSION_PATCH,
    .credSize = MAX_CRED_FLASH_SIZE,
    .dummy = 0
};

//READ COMMAND variables
uint16_t readOffset=0;
uint16_t readEnd=0;
uint8_t  readType = 0; // 0 - Flash, 1 - RAM

void UCP_Init(void)
{
    UCP_state = DEVICE_LOCKED;
    return; //nothing to do yet ;)
}

void UCP_Task(void)
{
    switch(UCP_state)
    {

        case WRITE_EEPROM:
        case IDLE:
        case DEVICE_LOCKED:
            break;
        case READING_CMD:
            UCP_state++;
            break;
        case READING:
            if(readOffset < readEnd)
            {
                    if(readType==0) { memcpy_P((void*)&customReport.buf, (void*)(credentials+readOffset), 8); }
                    else if (readType==1) { memcpy((void*)&customReport.buf, (void*)(readOffset), 8); }
                    else /*if(readType==2)*/ { eeprom_read_block((void*)&customReport.buf, (void*)(readOffset), 8); }
                    readOffset += 8;
                    if(readOffset >= readEnd)
                    {
                        UCP_state = IDLE;
                    }
            }
            else
            {
                UCP_state = IDLE;
                customReport.buf[0] = UCP_CMD_ERROR;
                customReport.buf[1] = UCP_ERR_PROTOCOL;
            }
            break;
        case WRITTING:
            if(usbWriteAccepted == 1)
            {
                usbWrite = 1;
                usbWriteAccepted = 0;
            }
            break;
        default: //should not arrive here,Ignore
            break;

    }
}

/* Decoding a received message */
void UCP_Decode(uint8_t *data, uint8_t len)
{
    /* Always return the same command data except if error occour */
    memcpy((void*)customReport.buf, (void*)data, sizeof(customReport.buf));
    
    //Validate packet
    if(len != 8){ //UCP packets are 8 byte size
        customReport.buf[0]=UCP_CMD_ERROR;
        customReport.buf[1]=UCP_ERR_SIZE;
    }
    else if (UCP_state == DEVICE_LOCKED)
    {
        customReport.buf[0]=UCP_CMD_ERROR;
        customReport.buf[1]=UCP_ERR_LOCKED;
    }
    else if (UCP_state == WRITTING)
    {
        if( (usbWrite == 0) && (readOffset < readEnd))
        {
            usbWriteAccepted = 1; // flag indicating a write must be done
        }        
        else
        {
            UCP_state = IDLE;
            customReport.buf[0] = UCP_CMD_ERROR;
            customReport.buf[1] = UCP_ERR_PROTOCOL;
            usbWriteAccepted = 0;
            usbWrite = 0;
        }
    }
    else if (UCP_state == WRITE_EEPROM)
    {
        if(readOffset < readEnd)
        {
            eeprom_update_block ((void*)customReport.buf, (void*)readOffset, 8);
            readOffset += 8;
            /* end of WRITE */
            if( readOffset >= readEnd )
            {
                UCP_state = IDLE;
            }
        }
        else
        {
            UCP_state = IDLE;
            customReport.buf[0] = UCP_CMD_ERROR;
            customReport.buf[1] = UCP_ERR_PROTOCOL;
        }
    }
    else
    {
        
        switch(data[0])
        {
            case UCP_CMD_RESET:
                break;
            case UCP_CMD_READ:
                if(UCP_state==IDLE){//TO START READING MUST BE IDLE
                    UCP_state = READING_CMD;
                    readOffset = *(uint16_t*)&data[1];
                    readEnd = readOffset + *(uint16_t*)&data[3];
                    readType = *(uint8_t*)&data[5];
                }else{ //PROTOCOL ERROR
                    UCP_state = IDLE;
                    customReport.buf[0] = UCP_CMD_ERROR;
                    customReport.buf[1] = UCP_ERR_PROTOCOL;
                }
                break;
            case UCP_CMD_WRITE:
                if(UCP_state==IDLE){//TO START READING MUST BE IDLE
                    UCP_state = WRITTING;
                    readOffset = *(uint16_t*)&data[1];
                    readEnd = readOffset + *(uint16_t*)&data[3];
                    
                    buffIndex = 0;
                    flashWriteAddr = (readOffset+(uint16_t)credentials);

                }else{ //PROTOCOL ERROR
                    UCP_state = IDLE;
                    customReport.buf[0] = UCP_CMD_ERROR;
                    customReport.buf[1] = UCP_ERR_PROTOCOL;
                }
                break;
            case UCP_CMD_SET_PIN:
                UCP_state = WRITE_EEPROM;
                readOffset = (uint16_t)LOCK_HASH;
                readEnd = (uint16_t)LOCK_HASH + 16;
                break;
            case UCP_CMD_READ_PIN:
                UCP_state = READING_CMD;
                readOffset = (uint16_t)LOCK_HASH;
                readEnd = readOffset + 16;
                readType = 2; /* Eeprom */
                break;
            case UCP_CMD_KEYBOARD:
                UCP_state = WRITE_EEPROM;
                readOffset = (uint16_t)keyboardLUT_ES;
                readEnd = (uint16_t)keyboardLUT_ES+KEYBOARD_SIZE;
                break;
            case UCP_CMD_DATA:
                break;
            case UCP_CMD_INFO:
                memcpy_P((void*)&customReport.buf[1], (void*)&INFO_data, sizeof(INFO_data));
                break;
            default:
                customReport.buf[0] = UCP_CMD_ERROR;
                customReport.buf[1] = UCP_ERR_CMD;
        }
    }
     
}

void UCP_WriteTask(void)
{
    uint8_t i;
    
    if(usbWrite == 1)
    {
        for(i=0; i < 8; i++)
        {
            tempBuff[buffIndex++] = customReport.buf[i]; // fill the temp buffer
            readOffset++;

            // if it's aligned or end, write to avoid waste of flash cycles
            if( ((readOffset%SPM_PAGESIZE) == 0) || (readOffset >= readEnd) )
            {
                FLS_write((uint8_t*)tempBuff, flashWriteAddr, buffIndex, 0);
                flashWriteAddr += (buffIndex);
                buffIndex = 0; // resetBufferIndex
                
                /* end of WRITE */
                if( readOffset >= readEnd )
                {
                    UCP_state = IDLE;
                    /* Load Credentials Again */
                    CRD_Init();
                    break;
                }
            }
        }
        usbWrite = 0;
    }
}
