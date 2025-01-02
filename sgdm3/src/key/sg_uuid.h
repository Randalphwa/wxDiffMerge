/*
Copyright 2010-2013 SourceGear, LLC
*/

#ifndef H_SG_UUID_H
#define H_SG_UUID_H

//////////////////////////////////////////////////////////////////
// Stuff for UUID's (aka GUID's).
//
// It turns out that there are 5 types of UUID's.
// See:
//     http://www.ietf.org/rfc/rfc4122.txt)
//     http://en.wikipedia.org/wiki/UUID
//     http://en.wikipedia.org/wiki/Guid
//
// Both Windows and Linux now default to version-4 -- which are just *big* random
// numbers and *DO NOT* contain any MAC address or time information.  Both platforms
// have a historical version-1 available -- which are only MAC address and time.
//
// A single UUID is 128 bits.
// This is 16 bytes as a packed binary buffer.
// This is 32 bytes when expressed as a hex string.

#define MY_SIZEOF_BINARY_UUID		16
#define MY_SIZEOF_HEX_UUID			32

//////////////////////////////////////////////////////////////////

/**
 * Generate a UUID into the given buffer.
 * The provided buffer must be "char [MY_SIZEOF_HEX_UUID+1]".
 *
 */
void SG_uuid__generate4(char * pbuf);
void SG_uuid__generate1(char * pbuf);

#endif // H_SG_UUID_H
