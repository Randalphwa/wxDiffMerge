// util_convert.h
// utility code to encode / decode data with Base16
//////////////////////////////////////////////////////////////////

#ifndef _UTIL_CONVERT_H_
#define _UTIL_CONVERT_H_

//////////////////////////////////////////////////////////////////


/* util_convert is a class for static methods used to 
 * convert data or wxString items to and from 
 * Base 16
 */
class util_convert
{
public:
	static wxString ToBase16(const wxByte *pBuffer, int len);
	static wxString ToBase16(const wxString &s);

	static bool FromBase16(const wxString &strEnc, wxByte **ppBuffer, size_t *pBufferLen);
	static bool GetStringFromBase16(const wxString &strEnc, wxString &s);
};

#endif // _UTIL_CONVERT_H_
