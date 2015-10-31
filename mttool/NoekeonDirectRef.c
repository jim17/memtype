/************************************************************************************/
/* NoekeonDirectRef.c
 *
 * Last Modified: 00/09/26             Created: 00/08/30
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
 * Description: Reference implementation of NESSIE interfaces for the block cipher 
 *              NOEKEON in DIRECT-KEY MODE
 *
 *              This code is NOT optimised. rather clarity was favorised
 *
 *   The text vectors should be as follows:
 * 
 *                      k = 00000000 00000000 00000000 00000000
 *                      a = 00000000 00000000 00000000 00000000
 * after NESSIEencrypt, b = b1656851 699e29fa 24b70148 503d2dfc
 * after NESSIEdecrypt, a?= 00000000 00000000 00000000 00000000
 * 
 *                      k = ffffffff ffffffff ffffffff ffffffff
 *                      a = ffffffff ffffffff ffffffff ffffffff
 * after NESSIEencrypt, b = 2a78421b 87c7d092 4f26113f 1d1349b2
 * after NESSIEdecrypt, a?= ffffffff ffffffff ffffffff ffffffff
 * 
 *                      k = b1656851 699e29fa 24b70148 503d2dfc
 *                      a = 2a78421b 87c7d092 4f26113f 1d1349b2
 * after NESSIEencrypt, b = e2f687e0 7b75660f fc372233 bc47532c
 * after NESSIEdecrypt, a?= 2a78421b 87c7d092 4f26113f 1d1349b2
 *
/************************************************************************************/
 
#include "Nessie.h"

/*==================================================================================*/
/* Number of computation rounds in the block cipher
/*----------------------------------------------------------------------------------*/
#define   NROUND		16	
/*----------------------------------------------------------------------------------*/
/* Round Constants are : 80,1B,36,6C,D8,AB,4D,9A,2F,5E,BC,63,C6,97,35,6A,D4 (encrypt)
/*----------------------------------------------------------------------------------*/
#define RC1ENCRYPTSTART  T8 (0x80)
#define RC2DECRYPTSTART  T8 (0xD4) 


/*==================================================================================*/
/* Null Vector
/*----------------------------------------------------------------------------------*/
u32 NullVector[4] = {0,0,0,0};


/*==================================================================================*/
void Theta (u32 const * const k,u32 * const a)
/*----------------------------------------------------------------------------------*/
/* DIFFUSION - Linear step THETA, involution
/*==================================================================================*/
{
  u32 tmp;

  tmp  = a[0]^a[2]; 
  tmp ^= ROTL32(tmp,8)^ROTL32(tmp,24); 
  a[1]^= tmp; 
  a[3]^= tmp; 

  a[0] ^= k[0]; a[1] ^= k[1]; a[2] ^= k[2]; a[3] ^= k[3]; 

  tmp  = a[1]^a[3]; 
  tmp ^= ROTL32(tmp,8)^ROTL32(tmp,24); 
  a[0]^= tmp; 
  a[2]^= tmp; 

} /* Theta */


/*==================================================================================*/
void Pi1(u32 * const a)
/*----------------------------------------------------------------------------------*/
/* DISPERSION - Rotations Pi1
/*==================================================================================*/
{ a[1] = ROTL32 (a[1], 1); 
  a[2] = ROTL32 (a[2], 5); 
  a[3] = ROTL32 (a[3], 2); 
}  /* Pi1 */


/*==================================================================================*/
void Pi2(u32 * const a)
/*----------------------------------------------------------------------------------*/
/* DISPERSION - Rotations Pi2
/*==================================================================================*/
{ a[1] = ROTL32 (a[1], 31);
  a[2] = ROTL32 (a[2], 27); 
  a[3] = ROTL32 (a[3], 30); 
}  /* Pi2 */


/*==================================================================================*/
void Gamma(u32 * const a)
/*----------------------------------------------------------------------------------*/
/* NONLINEAR - gamma, involution
/*----------------------------------------------------------------------------------*/
/* Input of i_th s-box = (i3)(i2)(i1)(i0), with (i3) = i_th bit of a[3]
 *                                              (i2) = i_th bit of a[2]
 *                                              (i1) = i_th bit of a[1]
 *                                              (i0) = i_th bit of a[0]
 *
 * gamma = NLIN o LIN o NLIN : (i3)(i2)(i1)(i0) --> (o3)(o2)(o1)(o0)
 *
 * NLIN ((i3) = (o3) = (i3)                     NLIN is an involution
 *       (i2)   (o2)   (i2)                      i.e. evaluation order of i1 & i0
 *       (i1)   (o1)   (i1+(~i3.~i2))                 can be swapped
 *       (i0))  (o0)   (i0+(i2.i1))
 * 
 *  LIN ((i3) = (o3) = (0.i3+0.i2+0.i1+  i0)    LIN is an involution
 *       (i2)   (o2)   (  i3+  i2+  i1+  i0)    
 *       (i1)   (o1)   (0.i3+0.i2+  i1+0.i0)    
 *       (i0))  (o0)   (  i3+0.i2+0.i1+0.i0)    
 *
/*==================================================================================*/
{ u32 tmp;

  /* first non-linear step in gamma */
  a[1] ^= ~a[3] & ~a[2];
  a[0] ^=   a[2] & a[1];

  /* linear step in gamma */
  tmp   = a[3];
  a[3]  = a[0];
  a[0]  = tmp;
  a[2] ^= a[0]^a[1]^a[3];

  /* last non-linear step in gamma */
  a[1] ^= ~a[3] & ~a[2];
  a[0] ^=   a[2] & a[1];
} /* Gamma */


/*==================================================================================*/
void Round (u32 const * const k,u32 * const a,u8 const RC1,u8 const RC2)
/*----------------------------------------------------------------------------------*/
/* The round function, common to both encryption and decryption
/* - Round constants is added to the rightmost byte of the leftmost 32-bit word (=a0)
/*==================================================================================*/
{ 
  a[0] ^= RC1;
  Theta(k,a); 
  a[0] ^= RC2;
  Pi1(a); 
  Gamma(a); 
  Pi2(a); 
}  /* Round */

/*==================================================================================*/
void RCShiftRegFwd (u8 * const RC)
/*----------------------------------------------------------------------------------*/
/* The shift register that computes round constants - Forward Shift
/*==================================================================================*/
{ 

  if ((*RC)&0x80) (*RC)=((*RC)<<1) ^ 0x1B; else (*RC)<<=1;
  
} /* RCShiftRegFwd */

/*==================================================================================*/
void RCShiftRegBwd (u8 * const RC)
/*----------------------------------------------------------------------------------*/
/* The shift register that computes round constants - Backward Shift
/*==================================================================================*/
{ 

  if ((*RC)&0x01) (*RC)=((*RC)>>1) ^ 0x8D; else (*RC)>>=1;
  
} /* RCShiftRegBwd */

/*==================================================================================*/
void CommonLoop (u32 const * const k,u32 * const a, u8 RC1, u8 RC2)
/*----------------------------------------------------------------------------------*/
/* loop - several round functions, ended by theta
/*==================================================================================*/
{ 
  unsigned i;

  for(i=0 ; i<NROUND ; i++) {
    Round(k,a,RC1,RC2); 
    RCShiftRegFwd(&RC1);
    RCShiftRegBwd(&RC2);
  }
  a[0]^=RC1;
  Theta(k,a); 
  a[0]^=RC2;

} /* CommonLoop */


/*==================================================================================*/
void NESSIEencrypt(const struct NESSIEstruct * const structpointer, 
                   const unsigned char * const plaintext,
                   unsigned char * const ciphertext)
/*==================================================================================*/
{ u32 const *k=structpointer->k;
  u32 state[4];
  

  state[0]=U8TO32_BIG(plaintext   );
  state[1]=U8TO32_BIG(plaintext+4 );
  state[2]=U8TO32_BIG(plaintext+8 );
  state[3]=U8TO32_BIG(plaintext+12);

  CommonLoop (k,state,RC1ENCRYPTSTART,0);
  
  U32TO8_BIG(ciphertext   , state[0]);
  U32TO8_BIG(ciphertext+4 , state[1]);
  U32TO8_BIG(ciphertext+8 , state[2]);
  U32TO8_BIG(ciphertext+12, state[3]);
} /* NESSIEencrypt */

/*==================================================================================*/
void NESSIEdecrypt(const struct NESSIEstruct * const structpointer,
                   const unsigned char * const ciphertext,
                   unsigned char * const plaintext)
/*==================================================================================*/
{ u32 const *kencrypt=structpointer->k;
  u32 k[4],state[4];

  state[0]=U8TO32_BIG(ciphertext   );
  state[1]=U8TO32_BIG(ciphertext+4 );
  state[2]=U8TO32_BIG(ciphertext+8 );
  state[3]=U8TO32_BIG(ciphertext+12);

  k[0]=kencrypt[0];
  k[1]=kencrypt[1];
  k[2]=kencrypt[2];
  k[3]=kencrypt[3];
  Theta(NullVector,k);

  CommonLoop (k,state,0,RC2DECRYPTSTART);

  U32TO8_BIG(plaintext   , state[0]);
  U32TO8_BIG(plaintext+4 , state[1]);
  U32TO8_BIG(plaintext+8 , state[2]);
  U32TO8_BIG(plaintext+12, state[3]);
} /* NESSIEdecrypt */


/*==================================================================================*/
void NESSIEkeysetup(const unsigned char * const key, 
                    struct NESSIEstruct * const structpointer)
/*----------------------------------------------------------------------------------*/
/* PRE:
 * 128-bit key value in byte array key [16 bytes]
 *
 * key: [00] [01] [02] [03] [04] [05] [06] [07] [08] [09] [10] [11] [12] [13] [14] [15]
 *      ----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----
 *
 * POST:
 * key value written in 32-bit word array k in NESSIEstruct
 *      -------------------+-------------------+-------------------+-------------------
 *              k[0]                k[1]                k[2]                k[3]
/*==================================================================================*/
{ u32 *k=structpointer->k;
  
  k[0]=U8TO32_BIG(key   );
  k[1]=U8TO32_BIG(key+4 );
  k[2]=U8TO32_BIG(key+8 );
  k[3]=U8TO32_BIG(key+12);

} /* NESSIEkeysetup */








