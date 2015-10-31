#ifndef _OPT_H_
#define _OPT_H_

#include <stdint.h>

#define OPT_START_STR  "[DETAIL]"

/* Public functions */
void OPT_fsm(uint8_t button);
void OPT_fsmStart(void);

#endif /* _OPT_H_*/