/* Name: main.c
 * Project: hid-custom-rq example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "led.h"            /* Led defines for debugging */

//#include "password.h"

//PROGMEM const char myPass[] = PASSWORD;

#include "print.h"
#include "adm.h"
#include "uib.h"
#include "uif.h"
#include "sch.h"
#include "crd.h"
#include "ucp.h"

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //   REPORT_ID (1)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0,                          // END_COLLECTION
          
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x02,                    //   REPORT_ID (2)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};


/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uint8_t idle_rate = 500 / 4; // see HID1_11.pdf sect 7.2.4
static uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6

uchar   usbFunctionWrite(uchar *data, uchar len)
{
    UCP_Decode(data,len); //to USB Communication Protocol
    return 1; /* return 1 if this was the last chunk */
}

uchar usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void *)data;


    if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
            return 0; // ignore request if it's not a class specific request

    // see HID1_11.pdf sect 7.2
    switch (rq->bRequest)
    {
        case USBRQ_HID_GET_IDLE:
            usbMsgPtr = &idle_rate; // send data starting from this byte
            return 1; // send 1 byte
        case USBRQ_HID_SET_IDLE:
            idle_rate = rq->wValue.bytes[1]; // read in idle rate
            return 0; // send nothing
        case USBRQ_HID_GET_PROTOCOL:
            usbMsgPtr = &protocol_version; // send data starting from this byte
            return 1; // send 1 byte
        case USBRQ_HID_SET_PROTOCOL:
            protocol_version = rq->wValue.bytes[1];
            return 0; // send nothing
        case USBRQ_HID_GET_REPORT:
            // check for report ID then send back report
            if (rq->wValue.bytes[0] == 1)
            {
                    usbMsgPtr = (void*)&reportBuffer;
                    return sizeof(reportBuffer);
            }
            else if (rq->wValue.bytes[0] == 2)
            {
                    UCP_Task(); /* send data to host */
                    usbMsgPtr = (void*)&customReport;
                    return sizeof(customReport);
            }
            else
            {
                    return 0; // no such report, send nothing
            }
        case USBRQ_HID_SET_REPORT:
            if (rq->wValue.bytes[0] == 2)
            {
                return USB_NO_MSG; /* call usbFunctionWrite() to read data from host */
            }
            return 0; // no such report, send nothing
        default: // do not understand data, ignore
            return 0; // send nothing
    }
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    SCH_Init();
    ADM_Init();
    UIB_Init();
    UIF_Init();
    LED_Init();
    CRD_Init();
    UCP_Init();

    cli(); /* Ensure usb interrupts enabled by bootloader alter disconnect of usb */
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    _delay_ms(500);
    usbDeviceConnect();

    /* USB Init must be after forcing USB reenumeration */
    usbInit();
    sei();
    wdt_enable(WDTO_1S);

    
    /* 1 - Keyboard report id
       2 - HID feature report id
     reportBuffer is only used to send keyboard data so, initialize to 1
    */
    reportBuffer.reportid = 1;
    
    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
        ADM_Task();
        SCH_Task();

        if(usbInterruptIsReady()){
            UCP_WriteTask();
            LED_TOGGLE();
            //printUpdate();
            reportBuffer.modifier = USB_modifier;
            reportBuffer.keycode = USB_keycode;
            usbSetInterrupt((void*)&reportBuffer, sizeof(reportBuffer));
        }
    }
}
