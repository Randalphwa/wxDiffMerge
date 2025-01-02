// util_enc.h -- deal with some character encoding issues.
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_ENC_H
#define H_UTIL_ENC_H

//////////////////////////////////////////////////////////////////
// NOTE: we use WIDE-CHARS and UNICODE throughout this
// application, but we do have to read documents from
// the disk in whatever encoding they were saved in.
//
// as of wxWindows 2.6.1, there are several (confusing)
// sets of conversion classes.  some of these do work
// themselves, some rely on some of the others, and some
// call out to whatever is available in libc (iconv).
// 
// since it's not exactly obvious where to begin, i'll
// summarize a little here.
//
// in general, we want to use wxCSConv to convert from
// whatever on-disk encoding into UNICODE.  wxCSConv is
// the most general case -- it'll try to use the system
// (libc) libraries, if available, then try its internal
// hard-coded (UTF-n) routines, and finally try
// wxEncodingConverter as a last resort.
//
// unfortunately, wxCSConv doesn't provide any way of
// determining the set of encodings that it supports.
//
// oddly enough, wxFontMapper does provide a set of
// static methods that let us enumerate the set of
// "well known" encodings (ISO-8859-*, CPxxxx, UTF-*
// and etc) -- [i think the goal was for you to be able
// to request a font that has a variant for a specific
// encoding and then be able to use it to display data
// in that specific encoding without having to do any
// conversions.] -- and get an encoding "id".
//
// but having part of the code in wxFontMapper and part
// in wxCSConv -- just seems convoluted.  it just seems
// odd to pass around a "font-encoding" variable so that
// we the fim layer can load/convert files from disk
// into unicode.
//////////////////////////////////////////////////////////////////

#define util_encoding		wxFontEncoding

//////////////////////////////////////////////////////////////////

extern int util_encoding_sniff_bom(const byte * pBufRaw, size_t lenBuf, util_encoding * pEnc);
extern int util_encoding_create_bom(util_encoding enc, byte * pBuf);
extern wxMBConv * util_encoding_create_conv(util_encoding enc);
extern util_error util_encoding_does_buffer_have_nulls(util_encoding enc, const byte * pBufSrc, size_t lenBuf);

//////////////////////////////////////////////////////////////////

extern util_error util_encoding_export_conv(const wxString & strSrcWide,
											util_encoding enc,
											byte ** ppBufRaw, size_t * pLenRaw);

//////////////////////////////////////////////////////////////////

class util_enc
{
public:
	util_enc(void);
	~util_enc(void);

	int							findEnc(wxFontEncoding enc)		const;
	int							findEnc(const wxString & str)	const;

	inline wxFontEncoding		lookupEnc(int k)				const { return m_arrayEnc[k]; };
	inline int					getCount(void)					const { return m_nr;          };
	inline wxString *			getArrayNames(void)				const { return m_arrayNames;  };
	inline wxFontEncoding *		getArrayEnc(void)				const { return m_arrayEnc;    };

	const wxString				getName(int k)					const { return m_arrayNames[k]; };
	const wxString				getName(wxFontEncoding enc)		const { return getName(findEnc(enc)); }; 

private:
	int							m_nr;
	wxString *					m_arrayNames;
	wxFontEncoding *			m_arrayEnc;
};

//extern int		util_enc__allocateArrayOfNames(wxString ** array, wxFontEncoding ** arrayEnc);
//extern void		util_enc__freeArrayOfNames(wxString * array, wxFontEncoding * arrayEnc);
//extern int		util_enc__findEncInArray(wxFontEncoding * arrayEnc, wxFontEncoding enc);

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_ENC_H
