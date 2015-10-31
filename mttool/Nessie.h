/************************************************************************************/
/* Nessie.h
 *
 * Last Modified: 00/08/30             Created: 00/08/30
 *
 * Project    : Nessie Proposal: NOEKEON
 *
 * Authors    : Joan Daemen, Michael Peeters, Vincent Rijmen, Gilles Van Assche
 *
 * Written by : originally written by [NESSIE]
 *              modifications brought by Michael Peeters
 *
 * References : [NESSIE] see http://cryptonessie.org/ for information about 
 *                       interface conventions and definition of portable C.
 *
 * Description: Macro definitions useful for portability
 *              Data structures used for NOEKEON
 *
 * Modif.     : Minor bugs in U8TO32_BIG & U8TO32_LITTLE, 
 *              data structures for Noekeon
/************************************************************************************/

#ifndef PORTABLE_C__
#define PORTABLE_C__

#include <limits.h>

/* Definition of minimum-width integer types
 * 
 * u8   -> unsigned integer type, at least 8 bits, equivalent to unsigned char
 * u16  -> unsigned integer type, at least 16 bits
 * u32  -> unsigned integer type, at least 32 bits
 *
 * s8, s16, s32  -> signed counterparts of u8, u16, u32
 *
 * Always use macro's T8(), T16() or T32() to obtain exact-width results,
 * i.e., to specify the size of the result of each expression.
 */

typedef signed char s8;
typedef unsigned char u8;

#if UINT_MAX >= 4294967295UL

typedef signed short s16;
typedef signed int s32;
typedef unsigned short u16;
typedef unsigned int u32;

#define ONE32   0xffffffffU

#else

typedef signed int s16;
typedef signed long s32;
typedef unsigned int u16;
typedef unsigned long u32;

#define ONE32   0xffffffffUL

#endif

#define ONE8    0xffU
#define ONE16   0xffffU

#define T8(x)   ((x) & ONE8)
#define T16(x)  ((x) & ONE16)
#define T32(x)  ((x) & ONE32)

/*
 * If you want 64-bit values, uncomment the following lines; this
 * reduces portability.
 */
/*
  #if ((1UL << 31) * 2UL) != 0UL
  typedef unsigned long u64;
  typedef signed long s64;
  #define ONE64   0xffffffffffffffffUL
  #else
  typedef unsigned long long u64;
  typedef signed long long s64;
  #define ONE64   0xffffffffffffffffULL
  #endif
  #define T64(x)  ((x) & ONE64)
*/
/*
 * Note: the test is used to detect native 64-bit architectures;
 * if the unsigned long is strictly greater than 32-bit, it is
 * assumed to be at least 64-bit. This will not work correctly
 * on (old) 36-bit architectures (PDP-11 for instance).
 *
 * On non-64-bit architectures, "long long" is used.
 */

/*
 * U8TO32_BIG(c) returns the 32-bit value stored in big-endian convention
 * in the unsigned char array pointed to by c.
 */
#define U8TO32_BIG(c)  (((u32)T8(*(c)) << 24) | ((u32)T8(*((c) + 1)) << 16) |\
                       ((u32)T8(*((c) + 2)) << 8) | ((u32)T8(*((c) + 3))))

/*
 * U8TO32_LITTLE(c) returns the 32-bit value stored in little-endian convention
 * in the unsigned char array pointed to by c.
 */
#define U8TO32_LITTLE(c)  (((u32)T8(*(c))) | ((u32)T8(*((c) + 1)) << 8) |\
                      (u32)T8(*((c) + 2)) << 16) | ((u32)T8(*((c) + 3)) << 24))

/*
 * U8TO32_BIG(c, v) stores the 32-bit-value v in big-endian convention
 * into the unsigned char array pointed to by c.
 */
#define U32TO8_BIG(c, v)    do { \
		u32 x = (v); \
		u8 *d = (c); \
		d[0] = T8(x >> 24); \
		d[1] = T8(x >> 16); \
		d[2] = T8(x >> 8); \
		d[3] = T8(x); \
	} while (0)

/*
 * U8TO32_LITTLE(c, v) stores the 32-bit-value v in little-endian convention
 * into the unsigned char array pointed to by c.
 */
#define U32TO8_LITTLE(c, v)    do { \
		u32 x = (v); \
		u8 *d = (c); \
		d[0] = T8(x); \
		d[1] = T8(x >> 8); \
		d[2] = T8(x >> 16); \
		d[3] = T8(x >> 24); \
	} while (0)

/*
 * ROTL32(v, n) returns the value of the 32-bit unsigned value v after
 * a rotation of n bits to the left. It might be replaced by the appropriate
 * architecture-specific macro.
 *
 * It evaluates v and n twice.
 *
 * The compiler might emit a warning if n is the constant 0. The result
 * is undefined if n is greater than 31.
 */
#define ROTL32(v, n)   (T32((v) << (n)) | ((v) >> (32 - (n))))


/************************************************************************************/
/* 
 * The following deals with Noekeon data structures
 *
/************************************************************************************/

struct NESSIEstruct {
  u32 k[4];
};

#endif   /* PORTABLE_C__ */

