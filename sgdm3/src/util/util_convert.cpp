//util_convert.cpp
// utility code to encode / decode data with Base16
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>

#include "util_convert.h"

//////////////////////////////////////////////////////////////////


/*
 * Convert the passed in byte array into a Base16 wxString
*/
/*static*/
wxString util_convert::ToBase16(const wxByte *pBuffer, int len)
{
	// Notes adapted from xt_tool_table::_encodeBlob()
	//
	// take the buffer and encode it in BASE16 in a string variable.
	// [yes, i can hear you groaning now.]  this solves a couple
	// of potential problems:
	// 
	// [] global props (and the wxConfig stuff) only take strings
	//    and since sgdm will need to store binary data, wxConfig
	//    won't be able to safely digest the blob (it'll try to 
	//    convert it to UTF8 before writing) **AND** the data will
	//    have lots of zeroes, so various string functions
	//    won't work.
	//    
	// [] base64 might be more kosher, but i have to dig up the
	//    routines somewhere **AND** base64 uses <>= (and maybe
	//    other chars) that XML finds useful, so we might have
	//    another quoting/escaping problem if the wxConfig layer
	//    on a platform uses XML under the hood.  [they might
	//    take care of it, but i don't want to rely on it.]
	//
	// so, we base16 it and be done with it.

	wxString strEncoded;

	if ((pBuffer != NULL) && (len > 0))
	{
		static const wxChar* szHex = wxT("0123456789abcdef");

		if (strEncoded.Alloc(2 * (size_t)len) == true)
		{
			for (int i = 0; i < len; i++)
			{
				// compute the 'hi' byte and make the first character
				strEncoded += szHex[((pBuffer[i] >> 4) & 0x0F)];

				// followed by the 'lo' byte
				strEncoded += szHex[((pBuffer[i]) & 0x0F)];
			}
		}
	}

	return strEncoded;
}


/* 
 * Convert the passed in string into a Base16 wxString
 */
/*static*/
wxString util_convert::ToBase16(const wxString &s)
{
	wxString strEncoded;

	if (s.IsEmpty() == false)
	{
		// get the data as a byte buffer after converting to UTF8.  Afterwards, base16 the resulting data buffer
		const wxScopedCharBuffer buffer = s.ToUTF8();
		const char *pData = buffer.data();
		strEncoded = ToBase16( (const wxByte*)pData, strlen(pData) );
	}
	return strEncoded;
}


/*
 * Decode a base16 string, 'strEnc', into a newly allocated buffer.
 * the 'callee' is responsible for freeing the memory of ppBuffer
 * using "delete[]" if the conversion is successful.
 */
/*static*/
bool util_convert::FromBase16(const wxString &strEnc, wxByte **ppBuffer, size_t *pBufferLen)
{
	bool bRet = false;

	if ((ppBuffer == NULL) || (pBufferLen == NULL))
		return bRet;

	// initialize the 'out' params
	*ppBuffer = NULL;
	*pBufferLen = 0;

	// need 2 or more characters to decode
	size_t nLen = strEnc.length();
	if ((nLen / 2) == 0)
	{
		if ((nLen % 2) == 0)
		{	// empty string results in an empty buffer
			*ppBuffer = new wxByte[0];
			return true;
		}
		else
		{	// illegal arg on a 1 character string
			return bRet;
		}
	}

	// adjust the length to 0.5 the size of the string (which was doubled during encoding)
	nLen /= 2;

	wxByte *pBuf = new wxByte[nLen];
	if (pBuf != NULL)
	{
		// set return value to true
		bRet = true;

		wxChar ch;
		const wxChar *pszEncoded = strEnc.wc_str();
		size_t i = 0;
		do
		{
			// set the hi-byte value
			ch = *(pszEncoded++);
			if      ( (ch >= wxT('0')) && (ch <= wxT('9')) ) { pBuf[i] = (wxByte)(( ch - wxT('0'))       << 4); }
			else if ( (ch >= wxT('a')) && (ch <= wxT('f')) ) { pBuf[i] = (wxByte)(( ch - wxT('a') + 10 ) << 4); }
			else if ( (ch >= wxT('A')) && (ch <= wxT('F')) ) { pBuf[i] = (wxByte)(( ch - wxT('A') + 10 ) << 4); }
			else { bRet = false; }

			if (bRet == true)
			{	// OR in the lo-byte value
				ch = *(pszEncoded++);
				if      ( (ch >= wxT('0')) && (ch <= wxT('9')) ) { pBuf[i] |= (wxByte)( ch - wxT('0')      ); }
				else if ( (ch >= wxT('a')) && (ch <= wxT('f')) ) { pBuf[i] |= (wxByte)( ch - wxT('a') + 10 ); }
				else if ( (ch >= wxT('A')) && (ch <= wxT('F')) ) { pBuf[i] |= (wxByte)( ch - wxT('A') + 10 ); }
				else { bRet = false; }
			}
		} while ((++i < nLen) && (bRet == true));

		if (bRet == true) { *ppBuffer = pBuf; *pBufferLen = nLen; }
		else { delete[] pBuf; }
	}

	return bRet;
}


/* Decode the strEnc string and place it in the decrypted paramter. */
/*static*/ bool util_convert::GetStringFromBase16(const wxString &strEnc, wxString &s)
{
	wxByte *p = NULL;
	size_t len;

	const bool bConvert = FromBase16(strEnc, &p, &len);
	if (bConvert == true)
	{
		if (len > 0)
		{
			// p points to a buffer array equal to the number of characters... 
			// add extra space so final buffer can be null terminated
			wxMemoryBuffer buffer(len + 2);

			// add the byte[] to the buffer so it represents the string in UTF8 format
			buffer.AppendData(p, len);

			// also, append two extra bytes of 0 (NULL) to the end for NULL terminating the string
			void* pBuffer = buffer.GetAppendBuf(2);
			memset(pBuffer, 0, 2);
			buffer.UngetAppendBuf(2);

			// convert the buffer to a wxString.
			s = wxString::FromUTF8(static_cast<wxScopedCharBuffer::CharType*>(buffer.GetData()));
		}
		else
		{	// this is an empty conversion
			s.Empty();
		}
	}

	if (p != NULL) { delete[] p; }

	return bConvert;
}
