/*
Copyright 2010-2013 SourceGear, LLC
*/

//////////////////////////////////////////////////////////////////

#ifndef H_SGHASH_H
#define H_SGHASH_H

//////////////////////////////////////////////////////////////////

typedef struct _sghash_algorithm SGHASH_algorithm;
typedef struct _sghash_handle    SGHASH_handle;

typedef unsigned char			SG_byte;

#if defined(WINDOWS) || defined(_WINDOWS)

typedef signed char				SG_int8;
typedef unsigned char			SG_uint8;

typedef signed short			SG_int16;
typedef unsigned short			SG_uint16;

typedef int						SG_int32;
typedef unsigned int			SG_uint32;

typedef __int64					SG_int64;
typedef unsigned __int64		SG_uint64;

#elif defined(MAC) || defined(LINUX)

#include <stdint.h>

typedef int8_t					SG_int8;
typedef uint8_t					SG_uint8;

typedef int16_t					SG_int16;
typedef uint16_t				SG_uint16;

typedef int32_t					SG_int32;
typedef uint32_t				SG_uint32;

typedef int64_t					SG_int64;
typedef uint64_t				SG_uint64;

#else
#error "WINDOWS, MAC, LINUX not defined."
#endif

//////////////////////////////////////////////////////////////////

/**
 * Begin a hash computation using the named hash-method.
 * If no hash-method is provided, we use the default hash-method.
 *
 * A hash handled is ALLOCATED and RETURNED.  You own this.  Call SGHASH_final() or
 * SGHASH_abort() to free it.
 *
 */
bool SGHASH_init(SGHASH_handle ** ppHandle);

/**
 * Add content to the hash.
 */
bool SGHASH_update(SGHASH_handle * pHandle, const SG_byte * pBuf, SG_uint32 lenBuf);

/**
 * Finish the hash computation and fill the provided buffer
 * with a hex digit string representation of the hash result.  The provided
 * buffer must at least (strlen-hashes + 1).
 *
 * The hash handle is freed and your pointer is nulled.
 */
bool SGHASH_final(SGHASH_handle ** ppHandle, char * pBufResult, SG_uint32  lenBuf);

/**
 * Abandon a hash computation and free the hash handle.  Your pointer
 * is nulled.
 */
void SGHASH_abort(SGHASH_handle ** ppHandle);

/**
 * a Quick 1-step hash wrapper
 * 
 */
void SGHASH_quick(char * bufHash, SG_uint32 lenBufHash,
				  const SG_byte * pByteBuf, SG_uint32 lenByteBuf);

//////////////////////////////////////////////////////////////////

#endif//H_SGHASH_H
