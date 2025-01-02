// util_enc.cpp -- deal with some character encoding issues.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

util_enc::util_enc(void)
{
	// use the static parts of wxFontMapper to get a list
	// (with nice, user-friendly names) and let the user
	// pick one.

	int k;
	int nr = (int)wxFontMapper::GetSupportedEncodingsCount();

	//////////////////////////////////////////////////////////////////
	// there is some subscript confusion in the wxFontMapper's list of
	// supported character encodings.  (perhaps, it's just me; perhaps not.)
	// 
	// GetSupportedEncodingsCount() returns the number of NAMED encodings
	// (ISO-8859-1 thru EUC-JP) -- it does not include the system default.
	// 
	// GetEncoding[0..n) returns the wxFontEncoding for them (a number from
	// [1..n]).
	// 
	// GetEncodingDescription[0..n] maps a wxFontEncoding to a string and
	// *DOES* include the system default (as 0).
	//
	// so we build an array[0..n] of strings to present in a dialog and
	// an array[0..n] of the corresponding wxFontEncodings that can be used
	// to map the selection back into an actual encoding.  the dialog doesn't
	// have to worry about this confusion and we don't require the wxWidgets
	// wxFONTENCODING_ vars to be contiguous.
	//
	//////////////////////////////////////////////////////////////////
	// i just found another bit of confusion.  if you call wxCSConv() ctor and
	// pass wxFONTENCODING_DEFAULT, it asserts (and substitutes) _SYSTEM.
	// but if we try to seed it with _SYSTEM here, the name that we get is
	// "Unknown encoding (-1)".
	//
	// so we do all our with with _DEFAULT (so that we get nice messages)
	// and then substitute _SYSTEM when we get ready to load the file.
	// (see fim_ptable::_import_conv())
	//////////////////////////////////////////////////////////////////

	m_nr         = nr + 1;
	m_arrayNames = new wxString[m_nr];
	m_arrayEnc   = (wxFontEncoding *)calloc(m_nr,sizeof(wxFontEncoding));

	m_arrayEnc[0] = wxFONTENCODING_DEFAULT;
	for (k=1; k<m_nr; k++)
		m_arrayEnc[k] = wxFontMapper::GetEncoding(k-1);

	for (k=0; k<m_nr; k++)
		m_arrayNames[k] = wxFontMapper::GetEncodingDescription(m_arrayEnc[k]);
}

util_enc::~util_enc(void)
{
	delete [] m_arrayNames;
	if (m_arrayEnc) free(m_arrayEnc);
}

//////////////////////////////////////////////////////////////////

int util_enc::findEnc(wxFontEncoding enc) const
{
	// find enc in m_arrayEnc; return index.
	// if not found, return 0 (the place where we put _DEFAULT)

	for (int k=0; k<m_nr; k++)
		if (enc == m_arrayEnc[k])
			return k;

	return 0;
}

int util_enc::findEnc(const wxString & str) const
{
	// find str in m_arrayNames; return index.
	// if not found, return 0 (the place where we put _DEFAULT)

	for (int k=0; k<m_nr; k++)
		if (str == m_arrayNames[k])
			return k;

	return 0;
}

//////////////////////////////////////////////////////////////////

int util_encoding_sniff_bom(const byte * pBufRaw, size_t lenBuf, util_encoding * pEnc)
{
	// sniff buffer for Unicode BOM and return the implied encoding
	// and the length of the BOM.  the input buffer length may be
	// larger than what's needed to represent the BOM.
	//
	// return -1 if no BOM.

	if (lenBuf < 2)
		return -1;

	// see http://www.unicode.org/faq/utf_bom.html#BOM

	const byte * p = pBufRaw;

	if (lenBuf >= 4)
	{
		if ((p[0]==0x00) && (p[1]==0x00) && (p[2]==0xfe) && (p[3]==0xff))		// 00 00 FE FF
		{
			*pEnc = wxFONTENCODING_UTF32BE;
			//*ppConv = new wxMBConvUTF32BE;
			return 4;
		}
		if ((p[0]==0xff) && (p[1]==0xfe) && (p[2]==0x00) && (p[3]==0x00))		// FF FE 00 00 
		{
			*pEnc = wxFONTENCODING_UTF32LE;
			//*ppConv = new wxMBConvUTF32LE;
			return 4;
		}
	}

	if (lenBuf >= 3)
	{
		if ((p[0]==0xef) && (p[1]==0xbb) && (p[2]==0xbf))						// EF BB BF 
		{
			*pEnc = wxFONTENCODING_UTF8;
			//*ppConv = new wxMBConvUTF8;
			return 3;
		}
	}

	if (lenBuf >= 2)
	{
		if ((p[0]==0xfe) && (p[1]==0xff))										// FE FF
		{
			*pEnc = wxFONTENCODING_UTF16BE;
			//*ppConv = new wxMBConvUTF16BE;
			return 2;
		}

		if ((p[0]==0xff) && (p[1]==0xfe))										// FF FE
		{
			*pEnc = wxFONTENCODING_UTF16LE;
			//*ppConv = new wxMBConvUTF16LE;
			return 2;
		}
	}

	return -1;
}

int util_encoding_create_bom(util_encoding enc, byte * pBuf)
{
	// put BOM into given buffer.  buffer should be byte[4]
	// returns length of buffer.
	//
	// DO NOT USE wxCSConv routines on this buffer because
	// the BOM should not be converted.

	switch (enc)
	{
	default:
		return -1;

	case wxFONTENCODING_UTF32BE:
		pBuf[0] = 0x00;
		pBuf[1] = 0x00;
		pBuf[2] = 0xfe;
		pBuf[3] = 0xff;
		return 4;
			
	case wxFONTENCODING_UTF32LE:
		pBuf[0] = 0xff;
		pBuf[1] = 0xfe;
		pBuf[2] = 0x00;
		pBuf[3] = 0x00;
		return 4;

	case wxFONTENCODING_UTF8:
		pBuf[0] = 0xef;
		pBuf[1] = 0xbb;
		pBuf[2] = 0xbf;
		return 3;

	case wxFONTENCODING_UTF16BE:
		pBuf[0] = 0xfe;
		pBuf[1] = 0xff;
		return 2;

	case wxFONTENCODING_UTF16LE:
		pBuf[0] = 0xff;
		pBuf[1] = 0xfe;
		return 2;
	}
}

wxMBConv * util_encoding_create_conv(util_encoding enc)
{
	// create the best wxMBConv for the named encoding.

	switch (enc)
	{
	default:						return new wxCSConv(enc);		// the general purpose one (calls iconv on some platforms)
	case wxFONTENCODING_UTF32BE:	return new wxMBConvUTF32BE;
	case wxFONTENCODING_UTF32LE:	return new wxMBConvUTF32LE;
	case wxFONTENCODING_UTF8:		return new wxMBConvUTF8;
	case wxFONTENCODING_UTF16BE:	return new wxMBConvUTF16BE;
	case wxFONTENCODING_UTF16LE:	return new wxMBConvUTF16LE;
	}
}

//////////////////////////////////////////////////////////////////

util_error util_encoding_export_conv(const wxString & strSrcWide,
									 util_encoding enc,
									 byte ** ppBufRaw, size_t * pLenRaw)
{
	// convert wide buffer given into character encoding for disk.
	// we use the deprecated WC2MB rather than FromWChar because
	// the latter is buggy and/or not completely implemented.
	//
	// caller must free *ppBufRaw.

	//////////////////////////////////////////////////////////////////
	// see discussion in util_enc on wxFONTENCODING_SYSTEM and _DEFAULT.
	// passing _DEFAULT to the wxCSConv's ctor causes an assert.  it
	// then substitutes _SYSTEM.  the following is to avoid the assert
	// message box.
	//////////////////////////////////////////////////////////////////

	util_encoding encNormalized = ((enc == wxFONTENCODING_DEFAULT) ? wxFONTENCODING_SYSTEM : enc);
	util_error ue;
	wxMBConv * pConv = NULL;

	//////////////////////////////////////////////////////////////////
	// converter setup and usage spews to the log on errors.  capture it here.
	// see notes in import_conv() w/r/t wxCSConv's ctor silently failing....
	//////////////////////////////////////////////////////////////////

	{
		util_logToString uLog(&ue.refExtraInfo());

		pConv = util_encoding_create_conv(encNormalized);		// we need to delete this

		size_t lenNeeded = pConv->WC2MB(NULL,strSrcWide.wc_str(),0);

		if (ue.getExtraInfo().Length() > 0)
		{
			wxLogTrace(TRACE_UTIL_ENC,
					   _T("util_encoding_export_conv: ERROR for [%s] failed [%s]"),
					   wxFontMapper::GetEncodingName(encNormalized).wc_str(),
					   ue.getExtraInfo().wc_str());
			ue.set(util_error::UE_CANNOT_EXPORT_CONV);
			goto Finished;
		}

		//////////////////////////////////////////////////////////////////
		// see notes in fim_ptable::_import_conv() w/r/t return codes on MB2WC/WC2MB
		// sometimes being 0 and sometimes (size_t)-1....
		//////////////////////////////////////////////////////////////////

		if ( (lenNeeded == 0) || (lenNeeded == (size_t)-1) )
		{
			wxLogTrace(TRACE_UTIL_ENC, _T("util_encoding_export_conv: ERROR [len %ld][type %s]"),
					   (long)lenNeeded,
					   wxFontMapper::GetEncodingName(encNormalized).wc_str());
			ue.set(util_error::UE_CANNOT_EXPORT_CONV, wxFontMapper::GetEncodingDescription(encNormalized).wc_str());
			goto Finished;
		}

		byte * pBufRaw = (byte *)calloc((lenNeeded+10),sizeof(byte));
		size_t lenUsed = pConv->WC2MB((char *)pBufRaw,strSrcWide.wc_str(),lenNeeded+10);

		wxASSERT_MSG( (lenUsed==lenNeeded), _T("Coding Error: conv"));

		wxLogTrace(TRACE_UTIL_ENC, _T("util_encoding_export_conv:  [len %ld][type %s]"),
				   (long)lenUsed,
				   wxFontMapper::GetEncodingName(encNormalized).wc_str());

		*ppBufRaw = pBufRaw;
		*pLenRaw = lenUsed;
	}

Finished:
	DELETEP(pConv);
	return ue;
}

//////////////////////////////////////////////////////////////////

util_error util_encoding_does_buffer_have_nulls(util_encoding enc, const byte * pBufSrc, size_t lenBuf)
{
	//////////////////////////////////////////////////////////////////
	// inspect buffer (excluding any BOM) for NULs.  these cause all
	// kinds of problems for the wxWidgets conversion routines.  there
	// are calls to strlen(), places where the input buffer length is
	// passed as -1 (meaning look NUL terminated buffer), and etc.
	// since we can't do anything about it (and NUL isn't a valid unicode
	// character), we just complain that they have a binary file.

	util_error ue;

	switch (enc)
	{
	case wxFONTENCODING_UTF32BE:	// 32bit format
	case wxFONTENCODING_UTF32LE:
		{
			if ((lenBuf % 4) != 0)
			{
				ue.set(util_error::UE_CONV_ODD_BUFFER_LEN,
					   wxString::Format(_("Length of file data must be a multiple of 4 for encoding '%s'."),
										wxFontMapper::GetEncodingDescription(enc).wc_str()));
				return ue;
			}
			
			int * p4 = (int *)pBufSrc;
			size_t len4 = lenBuf / 4;

			for (size_t k=0; k<len4; k++)
				if (p4[k] == 0)
				{
					ue.set(util_error::UE_CONV_BUFFER_HAS_NUL,
						   wxString::Format(_("File has 4 NUL bytes at byte offset %d; this is not valid for encoding '%s'."),
											(unsigned int)(k*4),
											wxFontMapper::GetEncodingDescription(enc).wc_str()));
					return ue;
				}

			return ue;
		}

	case wxFONTENCODING_UTF16BE:	// 16bit format
	case wxFONTENCODING_UTF16LE:
		{
			if ((lenBuf % 2) != 0)
			{
				ue.set(util_error::UE_CONV_ODD_BUFFER_LEN,
					   wxString::Format(_("Length of file data must be a multiple of 2 for encoding '%s'."),
										wxFontMapper::GetEncodingDescription(enc).wc_str()));
				return ue;
			}
			
			short * p2 = (short *)pBufSrc;
			size_t len2 = lenBuf / 2;

			for (size_t k=0; k<len2; k++)
				if (p2[k] == 0)
				{
					ue.set(util_error::UE_CONV_BUFFER_HAS_NUL,
						   wxString::Format(_("File has 2 NUL bytes at byte offset %d; this is not valid for encoding '%s'."),
											(unsigned int)(k*2),
											wxFontMapper::GetEncodingDescription(enc).wc_str()));
					return ue;
				}

			return ue;
		}

	default:						// all 8bit formats
		{
			for (size_t k=0; k<lenBuf; k++)
				if (pBufSrc[k] == 0)
				{
					ue.set(util_error::UE_CONV_BUFFER_HAS_NUL,
						   wxString::Format(_("File has a NUL byte at offset %d; this is not valid for encoding '%s'."),
											(unsigned int)k,
											wxFontMapper::GetEncodingDescription(enc).wc_str()));
					return ue;
				}

			return ue;
		}
	}

	//NOTREACHED
}
