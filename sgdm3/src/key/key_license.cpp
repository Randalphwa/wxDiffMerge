// routines to construct and validate a license key
//////////////////////////////////////////////////////////////////

#include <key.h>
#include "sghash__private.h"
#include "sghash__sha1.h"

//////////////////////////////////////////////////////////////////

#define CHUNK 5

struct key_layout
{
	struct key_random
	{
		char m_chDomain;
		char m_dash_1;
		char m_buf_1[CHUNK];
		char m_dash_2;
		char m_buf_2[CHUNK];
		char m_dash_3;
		char m_buf_3[CHUNK];
		char m_dash_4;
	} r;
	char m_buf_check_1[CHUNK];
	char m_dash_5;
	char m_buf_check_2[CHUNK];
	char m_zero;
};

#define KEY_TEMPLATE "X-XXXXX-XXXXX-XXXXX-XXXXX-XXXXX"

//////////////////////////////////////////////////////////////////

/**
 * the generated UUID and the SHA1 hash are both hex strings.
 * let's obscure that a bit when we stuff them into a license
 * key.
 *
 */
static void _weird_map(const char * pBufIn,
					   char * pBufOut,
					   int bufLen)
{
	static const char * hex_weird  = "BCDFHKLMNPRSTVXZ";  // "0123456789abcdef";
//	static const char * hex_normal = "0123456789abcdef";

	for (int k=0; k<bufLen; k++)
	{
		char c = pBufIn[k];
		if (c == 0)
		{
			pBufOut[k] = 0;
			break;
		}

		int h = 0;
		if ((c >= '0') && (c <= '9'))
			h = c - '0';
		else if ((c >= 'a') && (c <= 'f'))
			h = c - 'a' + 10;
		else if ((c >= 'A') && (c <= 'F'))
			h = c - 'A' + 10;
		pBufOut[k] = hex_weird[h];
	}
	
}

//////////////////////////////////////////////////////////////////

/**
 * Return an allocated buffer containing a newly
 * generated license key in the requested key domain.
 *
 * We define a "key domain" as a cheap way to tell a
 * free key (given to Vault users so that we don't ever
 * nag them) from a paid key.
 *
 * We just return the generated string; our caller can
 * install/register it if desired.
 *
 * The caller must free the returned key.
 *
 */
char * sgdm_genkey(const char chDomain)
{
	struct key_layout * pKey = (struct key_layout *)calloc(1, sizeof(struct key_layout));
	
	// generate a random sequence of hex digits.
	// we use a type-4 UUID because it is easy
	// and well defined and so we don't have
	// to worry about the various random number
	// generators on all the various platforms.

	char buf_uuid[MY_SIZEOF_HEX_UUID+1];
	char buf_uuid_weird[MY_SIZEOF_HEX_UUID+1];
	memset(buf_uuid, 0, sizeof(buf_uuid));
	SG_uuid__generate4(buf_uuid);
	_weird_map(buf_uuid, buf_uuid_weird, MY_SIZEOF_HEX_UUID+1);

	// only put part of the full UUID into the key layout.
	pKey->r.m_chDomain = chDomain;
	pKey->r.m_dash_1 = '-';
	memcpy(pKey->r.m_buf_1, &buf_uuid_weird[0*CHUNK], sizeof(pKey->r.m_buf_1));
	pKey->r.m_dash_2 = '-';
	memcpy(pKey->r.m_buf_2, &buf_uuid_weird[1*CHUNK], sizeof(pKey->r.m_buf_2));
	pKey->r.m_dash_3 = '-';
	memcpy(pKey->r.m_buf_3, &buf_uuid_weird[2*CHUNK], sizeof(pKey->r.m_buf_3));
	pKey->r.m_dash_4 = '-';

	// compute a hash on the string we already have
	// in the key.  that is, we DO NOT compute the
	// hash on the binary form of the UUID or necessarily
	// the full UUID.

	char buf_sha1[SHA1_RESULTLEN*2 + 1];
	char buf_sha1_weird[SHA1_RESULTLEN*2 + 1];
	memset(buf_sha1, 0, sizeof(buf_sha1));
	SGHASH_quick(buf_sha1, sizeof(buf_sha1),
				 (SG_byte *)&pKey->r, sizeof(pKey->r));
	_weird_map(buf_sha1, buf_sha1_weird, SHA1_RESULTLEN*2+1);

	// the hash is returned as a sequence of hex digits.
	// only put part of the full SHA1 hash into the key.
	memcpy(pKey->m_buf_check_1, &buf_sha1_weird[0*CHUNK], sizeof(pKey->m_buf_check_1));
	pKey->m_dash_5 = '-';
	memcpy(pKey->m_buf_check_2, &buf_sha1_weird[1*CHUNK], sizeof(pKey->m_buf_check_2));

	pKey->m_zero = 0;

	// return the allocated key as a simple char *
	// rather then alloc and copy it.
	
	char * pszKey = (char *)pKey;

	assert( (strlen(pszKey)+1) == sizeof(struct key_layout) );

	return pszKey;
}

/**
 * Check the given candidate key and see if it is valid.
 *
 */
bool sgdm_checkkey(const char * pszKey)
{
	if ( (strlen(pszKey) + 1) != sizeof(struct key_layout) )
		return false;
	
	struct key_layout * pKey = (struct key_layout *)pszKey;

	// re-compute the hash on just the random portion.

	char buf_sha1[SHA1_RESULTLEN*2 + 1];
	char buf_sha1_weird[SHA1_RESULTLEN*2 + 1];
	memset(buf_sha1, 0, sizeof(buf_sha1));
	SGHASH_quick(buf_sha1, sizeof(buf_sha1),
				 (SG_byte *)&pKey->r, sizeof(pKey->r));
	_weird_map(buf_sha1, buf_sha1_weird, SHA1_RESULTLEN*2+1);

	// since we only put part of the hash into the key,
	// only verify a portion of computed result.

	if (strncmp(&buf_sha1_weird[0*CHUNK], pKey->m_buf_check_1, sizeof(pKey->m_buf_check_1)) != 0)
		return false;
	if (pKey->m_dash_5 != '-')
		return false;
	if (strncmp(&buf_sha1_weird[1*CHUNK], pKey->m_buf_check_2, sizeof(pKey->m_buf_check_2)) != 0)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////
// the following are only used to size the dialog fields.

int sgdm_maxkeylen(void)
{
	return (int)(sizeof(key_layout) - 1);
}

const char * sgdm_keytemplate(void)
{
	return KEY_TEMPLATE;
}
