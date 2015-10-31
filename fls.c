
#include "fls.h"
#include <avr/boot.h>
#include <avr/pgmspace.h>

#include <avr/interrupt.h>  /* for sei(), cli() */

// attiny85 SPM_PAGESIZE is 64 bytes
#define FLS_IS_PAGE_ALIGNED(x)  ((x%SPM_PAGESIZE) == 0u)
#define FLS_IS_WORD_ALIGNED(x)  ((x%2) == 0u)

// Local functions prototype
void fls_writePage(uint16_t byteaddr);
void fls_erasePage(uint16_t byteaddr);
void fls_loadPage(uint8_t* buff, uint16_t byteaddr);
uint8_t fls_isErased(uint16_t startaddr, uint16_t size);

void fls_writePage(uint16_t byteaddr) {
  cli();
  boot_page_write_safe(byteaddr);   // will halt CPU, no waiting required
  sei();
}

void fls_erasePage(uint16_t byteaddr)
{
    cli();
    boot_page_erase_safe(byteaddr);
    sei();
}
void fls_loadPage(uint8_t* buff, uint16_t byteaddr)
{
    uint8_t i;
    uint16_t w;
    for(i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.
        w = *buff++;
        w += (*buff++) << 8;
        boot_page_fill (byteaddr + i, w);
    }
}

// return 1 (memory erased)
// return 0 (memory not fully erased)
uint8_t fls_isErased(uint16_t startaddr, uint16_t size)
{
    uint16_t i;
    for(i=0; i<size; i++)
    {
        if(pgm_read_byte(startaddr+i) != 0xFF)
        {
            return 0;
        }
    }

    return 1;
}

void FLS_write(uint8_t* buff, uint16_t startaddr, uint16_t size,uint8_t erase)
{
    uint8_t tempBuff[SPM_PAGESIZE];
    uint8_t eraseBuffStart;
    uint8_t eraseBuffEnd;
    uint8_t totalBytesPending;
    uint16_t alignedStartAddr;

    uint16_t i ;
    uint8_t j;

    i = 0;
    while(i<size)
    {
        alignedStartAddr = (startaddr+i) & 0xFFC0; /* 5 last bytes are not usefull */
        eraseBuffStart = (startaddr+i) & ~(0xFFC0);
        totalBytesPending = (size-i);
        eraseBuffEnd = eraseBuffStart+totalBytesPending;

        if( eraseBuffEnd > SPM_PAGESIZE )
        {
            eraseBuffEnd = SPM_PAGESIZE;
        }

        memcpy_P((void*)tempBuff,(void*)alignedStartAddr, SPM_PAGESIZE);
        fls_erasePage(alignedStartAddr);

        for(j=eraseBuffStart; j<eraseBuffEnd; j++)
        {
            tempBuff[j] = (erase==1)?(0xFF):(*buff++);
        }

        fls_loadPage(tempBuff,alignedStartAddr);
        fls_writePage(alignedStartAddr);

        i+= eraseBuffEnd-eraseBuffStart;
    }
}
