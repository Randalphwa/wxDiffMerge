/*
Copyright 2010-2013 SourceGear, LLC
*/

//////////////////////////////////////////////////////////////////

#include <key.h>
#include "sghash__private.h"

//////////////////////////////////////////////////////////////////

extern const SGHASH_algorithm SGHASH_alg__sha1;

//////////////////////////////////////////////////////////////////

static bool _do_init(const SGHASH_algorithm * pVTable, SGHASH_handle ** ppHandle)
{
	SGHASH_handle * pHandle = (SGHASH_handle *)malloc( sizeof(SGHASH_handle) + pVTable->sizeof_AlgCtx );

	if (!pHandle)
		return false;

	pHandle->pVTable = pVTable;

	(*pHandle->pVTable->fn_hash_init)( (void *)pHandle->variable );

	*ppHandle = pHandle;
	return true;
}

#ifndef NrElements
#	define NrElements(a)	((sizeof(a))/(sizeof(a[0])))
#endif

bool SGHASH_init(SGHASH_handle ** ppHandle)
{
	if (!ppHandle)
		return false;

	return _do_init( &SGHASH_alg__sha1, ppHandle);
}

bool SGHASH_update(SGHASH_handle * pHandle, const SG_byte * pBuf, SG_uint32 lenBuf)
{
	if (!pHandle)
		return false;

	(*pHandle->pVTable->fn_hash_update)( (void *)pHandle->variable, pBuf, lenBuf);
	return true;
}

bool SGHASH_final(SGHASH_handle ** ppHandle, char * pBufResult, SG_uint32 lenBuf)
{
	SGHASH_handle * pHandle;
	bool err;

	if (!ppHandle || !*ppHandle)
		return false;

	pHandle = *ppHandle;			// we own the handle now
	*ppHandle = NULL;				// regardless of what happens

	if (!pBufResult)
	{
		err = false;
	}
	else if (lenBuf < pHandle->pVTable->strlen_Hash + 1)
	{
		err = false;
	}
	else
	{
		(*pHandle->pVTable->fn_hash_final)( (void *)pHandle->variable, pBufResult);
		assert( strlen(pBufResult) == pHandle->pVTable->strlen_Hash );
		err = true;
	}

	free(pHandle);

	return err;
}

void SGHASH_abort(SGHASH_handle ** ppHandle)
{
	SGHASH_handle * pHandle;

	if (!ppHandle || !*ppHandle)
		return;

	pHandle = *ppHandle;			// we own the handle now
	*ppHandle = NULL;				// regardless of what happens

	free(pHandle);
}

void SGHASH_quick(char * bufHash, SG_uint32 lenBufHash,
				  const SG_byte * pByteBuf, SG_uint32 lenByteBuf)
{
	SGHASH_handle * pbh = NULL;
	SGHASH_init(&pbh);
	SGHASH_update(pbh, pByteBuf, lenByteBuf);
	SGHASH_final(&pbh, bufHash, lenBufHash);
}
