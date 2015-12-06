
/** User Input Button api */

#ifndef _CRD_H_
#define _CRD_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#define MAX_CRED_FLASH_SIZE (512)
#define CRD_START_STR  "[CREDENTIALS]"


typedef struct
{
    uint16_t name;
    uint16_t line1;
    uint16_t hop;
    uint16_t line2;
    uint16_t submit;
    uint16_t next;

} credentialFlash_t;

typedef struct
{
    uint16_t name; // here we will store name addr
    uint16_t next; // here we will store next credential
    uint16_t encrypt;

} credentialEncryptedFlash_t;

typedef enum credPosition
{
    CRD_USER = 0,
    CRD_HOP,
    CRD_PASS,
    CRD_SUBMIT,
    CRD_END
} credPosition_t;

extern void noekeon_decrypt(void);
extern void noekeon_encrypt(void);

typedef struct
{
    uint8_t plain[16];
    uint8_t key[16];

} cipher_t;

extern cipher_t cipher;

// Credential structure
extern const uint8_t credentials[MAX_CRED_FLASH_SIZE] PROGMEM;

/* Public functions */
void CRD_fsm(uint8_t button);
void CRD_fsmStart(void);
void CRD_Init(void);
uint16_t findEOS(uint16_t offset);
void crd_printDetail(credPosition_t start, credPosition_t stop);

#endif /* _CRD_H_*/

