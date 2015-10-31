/************************************************************************************/
/* NessieInterfaces.h
 *
 * Last Modified: 00/08/30             Created: 00/08/30
 *
 * Project    : Nessie Proposal: NOEKEON
 *
 * Authors    : Joan Daemen, Michael Peeters, Vincent Rijmen, Gilles Van Assche
 *
 * Written by : Michael Peeters
 *
 * References : [NESSIE] see http://cryptonessie.org/ for information about 
 *                       interface conventions and definition of portable C.
 *
 * Description: Declarations of interfaces for a block cipher 
 *              following the NESSIE convention
/************************************************************************************/

#ifndef NESSIEINTERFACES__
#define NESSIEINTERFACES__

/* As specified in [NESSIE] struct NESSIEstruct shall be defined in the file Nessie.h */
#include "Nessie.h"

void NESSIEkeysetup(const unsigned char * const key, 
                    struct NESSIEstruct * const structpointer);

void NESSIEencrypt(const struct NESSIEstruct * const structpointer, 
                   const unsigned char * const plaintext,
                   unsigned char * const ciphertext);

void NESSIEdecrypt(const struct NESSIEstruct * const structpointer,
                   const unsigned char * const ciphertext,
                   unsigned char * const plaintext);

#endif   /* NESSIEINTERFACES__ */

