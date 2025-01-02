/*
Copyright 2010-2013 SourceGear, LLC
*/

#include <key.h>

//////////////////////////////////////////////////////////////////

#if defined(MAC) || defined(LINUX)
#include <uuid/uuid.h>
#elif defined(WINDOWS) || defined(_WINDOWS)
#include <rpc.h>
#endif

//////////////////////////////////////////////////////////////////

typedef struct { SG_byte buf[MY_SIZEOF_BINARY_UUID]; } my_binary_uuid;

static void _format_buf(char * pbuf /*[MY_SIZEOF_HEX_UUID+1]*/,
						const my_binary_uuid * pb)
{
	static const char * hex = "0123456789abcdef";

	for (SG_uint32 k=0; k<MY_SIZEOF_BINARY_UUID; k++)
	{
		pbuf[0] = hex[ (pb->buf[k] >> 4) & 0x0f ];
		pbuf[1] = hex[ (pb->buf[k]     ) & 0x0f ];
		pbuf += 2;
	}
	*pbuf = 0;
}

#if defined(MAC) || defined(LINUX)

void SG_uuid__generate4(char * pbuf /*[MY_SIZEOF_HEX_UUID+1]*/)
{
	uuid_t u4;

	// Mac & Linux define a uuid as uuid_t and have "uuid_generate(uuid_t out)".
	// uuid_t is an array of 16 uint8's.  These are already in network-order.

	uuid_generate_random(u4);
	
	_format_buf(pbuf, (const my_binary_uuid *)u4);
}

void SG_uuid__generate1(char * pbuf /*[MY_SIZEOF_HEX_UUID+1]*/)
{
	uuid_t u1;

	// Mac & Linux define a uuid as uuid_t and have "uuid_generate(uuid_t out)".
	// uuid_t is an array of 16 uint8's.  These are already in network-order.

	uuid_generate_time(u1);
	
	_format_buf(pbuf, (const my_binary_uuid *)u1);
}

#elif defined(WINDOWS) || defined(_WINDOWS)

void _sg_gid__serialize_uuid(my_binary_uuid * pb, UUID * pu)
{
	// convert uint's inside UUID to network-order buffer.

	int k;
	SG_byte * pbuf = pb->buf;

	for (k=sizeof(pu->Data1)-1; k>=0; k--)
		*pbuf++ = (SG_byte)((pu->Data1 >> (k*8)) & 0xff);
	for (k=sizeof(pu->Data2)-1; k>=0; k--)
		*pbuf++ = (SG_byte)((pu->Data2 >> (k*8)) & 0xff);
	for (k=sizeof(pu->Data3)-1; k>=0; k--)
		*pbuf++ = (SG_byte)((pu->Data3 >> (k*8)) & 0xff);
	for (k=0; k<sizeof(pu->Data4); k++)
		*pbuf++ = pu->Data4[k];
}

void SG_uuid__generate4(char * pbuf /*[MY_SIZEOF_HEX_UUID+1]*/)
{
	// Windows defines a uuid as a UUID and have "UuidCreate(UUID * pUuid)".
	// UUID is a struct containing a uint32, 2 uint16's, and an array of bytes.
	// So there are some byte-order issues.

	UUID u4;
	my_binary_uuid BufBinary;

	UuidCreate(&u4);				// type 4 (random)

	// convert it into a uint8[16] like on linux/mac.
	_sg_gid__serialize_uuid(&BufBinary, &u4);

	// format it as hex string.
	_format_buf(pbuf, &BufBinary);
}

void SG_uuid__generate1(char * pbuf /*[MY_SIZEOF_HEX_UUID+1]*/)
{
	// Windows defines a uuid as a UUID and have "UuidCreate(UUID * pUuid)".
	// UUID is a struct containing a uint32, 2 uint16's, and an array of bytes.
	// So there are some byte-order issues.

	UUID u1;
	my_binary_uuid BufBinary;

	UuidCreateSequential(&u1);				// type 1 (mac+time)

	// convert it into a uint8[16] like on linux/mac.
	_sg_gid__serialize_uuid(&BufBinary, &u1);

	// format it as hex string.
	_format_buf(pbuf, &BufBinary);
}

#else
#error "WINDOWS, MAC, LINUX not defined."
#endif
