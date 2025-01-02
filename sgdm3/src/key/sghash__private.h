/*
Copyright 2010-2013 SourceGear, LLC
*/

//////////////////////////////////////////////////////////////////

#ifndef H_SGHASH__PRIVATE_H
#define H_SGHASH__PRIVATE_H

//////////////////////////////////////////////////////////////////

#if defined(WINDOWS) || defined(_WINDOWS)

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define u_int8_t  SG_byte
#define u_int32_t SG_uint32
#define u_int64_t SG_uint64
#define caddr_t   void *

#define bzero(p,len)			memset((p),0,(len))
#define bcopy(psrc,pdest,len)	memcpy((pdest),(psrc),(len))

#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN    4321
#define BYTE_ORDER    LITTLE_ENDIAN

#pragma warning( disable : 4244 )	// MSVC complains about possible loss of data when mixing size_t and u_int8_t

#pragma warning( disable : 4267 )	// VS2010 64bit complains about possible loss of data when converting size_t to anything smaller.

#elif defined(MAC) || defined(LINUX)

#include <stdlib.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <strings.h>
#if defined(MAC)
#include <machine/endian.h>
#elif defined(LINUX)
#include <endian.h>
#endif
#include <assert.h>

#else
#error "WINDOWS, MAC, LINUX not defined."
#endif

//////////////////////////////////////////////////////////////////

typedef void FN__hash_init  (void * pAlgCtx);
typedef void FN__hash_update(void * pAlgCtx, const SG_byte * pBuf, SG_uint32 lenBuf);
typedef void FN__hash_final (void * pAlgCtx, char * ppszResult);

struct _sghash_algorithm
{
	SG_uint32                   sizeof_AlgCtx;			// working space required by algorithm
	SG_uint32                   strlen_Hash;			// strlen() of a hash result (does not include the null)

	FN__hash_init * /*const*/		fn_hash_init;
	FN__hash_update * /*const*/		fn_hash_update;
	FN__hash_final * /*const*/		fn_hash_final;
};

struct _sghash_handle
{
	const SGHASH_algorithm *	pVTable;
	SG_uint64					variable[1];			// start of AlgCtx -- a variable length array of bytes
};

//////////////////////////////////////////////////////////////////

#endif//H_SGHASH__PRIVATE_H
