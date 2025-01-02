// util_string.cpp
// various string formatting utilities
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

// for DTM formatting
#if defined(__WXMSW__)
#	include <sys/types.h>
#	include <sys/timeb.h>
#elif defined(__WXMAC__)
#elif defined(__WXGTK__)
#endif

//////////////////////////////////////////////////////////////////

wxString util_printable_s(const wxString & s)
{
	return util_printable_cl(s.wc_str(), s.Len());
}

wxString util_printable_cl(const wxChar * pBuf, size_t len)
{
	// make printable string for stdout -- convert non-ascii chars
	// to hex (to keep from crashing libc's fputs() on Linux) when
	// doing wxLogTrace() with a "%s" on a string that has unicode
	// characters.
	//
	// NOTE: wxLog has a buffer limit of 4096 chars, so any message
	// NOTE: we send out longer than that will be truncated.

	wxString out;
	for (size_t k=0; (k<len); k++)
	{
		const wxChar c = pBuf[k];
		
		if ( ((c >= 0x20) && (c < 0x7f))  ||  (c==0x0a)  ||  (c==0x0d) )		// printable or LF or CR
			out += c;
		else if (c <= 0xffff)
			out += wxString::Format( _T("\\x%04x"), (int)c);
		else
			out += wxString::Format( _T("\\X%08x"), (int)c);
	}

	return out;
}

//////////////////////////////////////////////////////////////////

void util_print_buffer_cl(const wxChar * szTrace, const wxChar * pBuf, size_t len)
{
	// "logTrace" print the given buffer.

	if (!wxLog::IsAllowedTraceMask(szTrace))
		return;

	// hex-encode to guard against unicode spewing on stdout.

	wxString strOut = util_printable_cl(pBuf,len);

	// wxLog has a buffer limit of 4096 chars, so any message
	// we send out longer than that will be truncated.
	// 
	// chop into pieces to get around wxLog limits.

	while (strOut.Len() > 4000)
	{
		wxString sFirst(strOut.wc_str(),4000);
		wxLogTrace(szTrace, _T("[\n%s\n]"), sFirst.wc_str());
		strOut = strOut.Mid(4000);
	}

	wxLogTrace(szTrace, _T("[\n%s\n]"), strOut.wc_str());
}

//////////////////////////////////////////////////////////////////

void util_font_parse_spec(const wxString strInputSpec, long * pRequestedPointSize, long * pRequestedFamily, wxString * pRequestedFaceName)
{
	//////////////////////////////////////////////////////////////////
	// load font using font spec as used in global props.
	// we assume that this has the fields: "[<PointSize_d>]:[<Family_d>]:[<FaceName_s>]"
	//////////////////////////////////////////////////////////////////

	*pRequestedPointSize = 10;
	*pRequestedFamily    = wxFONTFAMILY_TELETYPE;
	*pRequestedFaceName  = _T("");

	long temp;
	wxString tmp = strInputSpec;

	if (tmp.BeforeFirst(':').ToLong(&temp))
		*pRequestedPointSize = temp;

	tmp = tmp.AfterFirst(':');

	if (tmp.BeforeFirst(':').ToLong(&temp))
		*pRequestedFamily = temp;

	tmp = tmp.AfterFirst(':');

	if (!tmp.IsEmpty())
		*pRequestedFaceName = tmp;
}

//////////////////////////////////////////////////////////////////

wxFont * util_font_create_normal_font(long requestedPointSize, long requestedFamily, wxString requestedFaceName)
{
	return util_font_create_font(requestedPointSize,requestedFamily,requestedFaceName,wxFONTWEIGHT_NORMAL);
}

wxFont * util_font_create_bold_font(long requestedPointSize, long requestedFamily, wxString requestedFaceName)
{
	return util_font_create_font(requestedPointSize,requestedFamily,requestedFaceName,wxFONTWEIGHT_BOLD);
}

wxFont * util_font_create_font(long requestedPointSize, long requestedFamily, wxString requestedFaceName, wxFontWeight requestedFontWeight)
{
	// WE RETURN A POINTER TO A FONT THAT THE CALLER OWNS AND MUST FREE EVENTUALLY.
	//
	// TODO see if we need to do something special for unicode -- there are man pages
	// TODO describing font-encoding and omit unicode from discussion.  wxFONTENCODING_UNICODE
	// TODO is documented as only used for character conversions.  so, it's not real clear
	// TODO what we should use here.
	//
	// WXBUG fonts are a little quirky.  wxMODERN is listed as fixed pitch, but
	// WXBUG when we request using just it and no facename, wxFont::IsFixedWidth()
	// WXBUG reports false on all three platforms, but it is.
	// WXBUG
	// WXBUG it turns out that IsFixedWidth() is defined as:
	// WXBUG	wxFontBase::IsFixedWidth() { return (GetFamily()==wxFONTFAMILY_TELETYPE); }
	// WXBUG so, let's forget about that...
	// WXBUG
	// WXBUG and when we ask for "Courier New", we get a wxSWISS font...
	// WXBUG
	// WXBUG and the wxFontDialog lets you seed it with a wxFont (via wxFontData)
	// WXBUG but it needs help, especially on the mac.
	// WXBUG

	wxFont * pFont = new wxFont(requestedPointSize,requestedFamily,
								wxFONTSTYLE_NORMAL,requestedFontWeight,
//								false,
								(requestedFontWeight == wxFONTWEIGHT_BOLD),	// also underline -- TODO remove this 
								requestedFaceName,wxFONTENCODING_SYSTEM);

//	wxLogTrace(wxTRACE_Messages,_T("util_font_create_font: [requested %d:%d:%s] yields [size %d][family %s][face %s][weight %d][enc %d][fixed %s] native [%s]"),
//			   requestedPointSize,requestedFamily,requestedFaceName.wc_str(),
//			   pFont->GetPointSize(),
//			   pFont->GetFamilyString().wc_str(),
//			   pFont->GetFaceName().wc_str(),
//			   pFont->GetWeight(),
//			   pFont->GetEncoding(),
//			   pFont->IsFixedWidth() ? _T("yes") : _T("no"),
//			   pFont->GetNativeFontInfoDesc().wc_str());

#if 1	// TODO we can probably ifdef this for GTK only
	//////////////////////////////////////////////////////////////////
	// wxBUG on GTK some fonts are broken and don't have complete metrics
	// wxBUG (or something) because GetCharHeight() can return 0.  (see
	// wxBUG font "Hershey-Script-Simplex", normal, 16pt.)
	// wxBUG
	// wxBUG this causes us lots of problems (because we need to divide by it).
	// wxBUG
	// wxBUG so, let's test the font before we blindly accept it.

	wxCoord xPixelsPerCol, yPixelsPerRow;
	{
		wxBitmap bitmap(1,1);
		wxMemoryDC dcMem;
		dcMem.SelectObject(bitmap);
		dcMem.SetFont(*pFont);

		xPixelsPerCol = dcMem.GetCharWidth();
		yPixelsPerRow = dcMem.GetCharHeight();
	}
	
//	wxLogTrace(wxTRACE_Messages,_T("util_font_create_font: pixels per [row %d][col %d]"),
//			   yPixelsPerRow,xPixelsPerCol);

	if ((xPixelsPerCol==0) || (yPixelsPerRow==0))
	{
//		wxLogTrace(wxTRACE_Messages,_T("util_font_create_font: bogus font metrics!!!"));

		delete pFont;
		return NULL;
	}
#endif

	return pFont;
}

//////////////////////////////////////////////////////////////////

wxFont * util_font_create_normal_font_from_spec(const wxString strInputSpec)
{
	long requestedPointSize;
	long requestedFamily;
	wxString requestedFaceName;

	util_font_parse_spec(strInputSpec,&requestedPointSize,&requestedFamily,&requestedFaceName);
	
	wxFont * pFont = util_font_create_normal_font(requestedPointSize,requestedFamily,requestedFaceName);

	return pFont;
}

wxFont * util_font_create_bold_font_from_spec(const wxString strInputSpec)
{
	long requestedPointSize;
	long requestedFamily;
	wxString requestedFaceName;

	util_font_parse_spec(strInputSpec,&requestedPointSize,&requestedFamily,&requestedFaceName);
	
	wxFont * pFont = util_font_create_bold_font(requestedPointSize,requestedFamily,requestedFaceName);

	return pFont;
}

//////////////////////////////////////////////////////////////////

wxString util_font_create_spec_from_font(const wxFont * pFont)
{
	wxString newfontdesc = wxString::Format(wxT("%d:%d:%s"),pFont->GetPointSize(),(int)pFont->GetFamily(),pFont->GetFaceName().wc_str());
	return newfontdesc;
}

//////////////////////////////////////////////////////////////////

int util_string_find_last(const wxString & strBuffer,	// buffer to search for last match
						  int limit,					// limit ndx for searching (match must be < this ndx)
						  const wxChar * szPattern)		// pattern to search for
{
	// find the position of the last occurrence of the pattern.
	// return wxNOT_FOUND if it doesn't occur at all.

	int ndx = strBuffer.Find(szPattern);

	if (ndx == wxNOT_FOUND)
		return wxNOT_FOUND;

	if (ndx >= limit)
		return wxNOT_FOUND;
	
	int prevNdx = ndx;

	while (1)
	{
		wxString strRest = strBuffer.Mid(prevNdx + 1);
		ndx = strRest.Find(szPattern);

		if (ndx == wxNOT_FOUND)
			break;

		int newNdx = prevNdx + 1 + ndx;

		if (newNdx >= limit)
			break;

		prevNdx = newNdx;
	}

	return prevNdx;	
}

//////////////////////////////////////////////////////////////////

// return a string containing the given date formatted
// appropriately for use in a Unified Diff header line.
//
// we now optionally include/exclude the nanoseconds.
// gnu-diff prints nanoseconds in their headers, so that
// is why i originally added it.
//
// likewise for the timezone.
//
wxString util_string__format_unified_date_from_dtm(const wxDateTime & dtm,
												   bool bIncludeNanoseconds,
												   bool bIncludeTZ)
{
	wxString strTz;

	if (bIncludeTZ)
	{
#if defined(__WXMSW__)

		struct _timeb tb;
		_ftime_s(&tb);
		int timezone = tb.timezone;			// minutes WEST of GMT (positive for west); negative for east of GMT
		int dstflag = tb.dstflag;

		if (dstflag)						// if daylight savings time,
			timezone -= 60;					// pretend to be one timezone east of where we are.

		if (timezone < 0)					// signs are backwards when reporting time
		{
			strTz = _T("+");
			timezone = -timezone;
		}
		else
			strTz = _T("-");

		int tzhr = (timezone / 60);
		int tzmn = (timezone % 60);

		strTz += wxString::Format(_T("%02d%02d"), tzhr, tzmn);

#elif defined(__WXMAC__) || defined(__WXGTK__)

		time_t t = time((time_t *)NULL);
		struct tm tm;
		localtime_r(&t,&tm);

		wxChar bufTz[200];
		wcsftime(bufTz,NrElements(bufTz),_T("%z"),&tm);

		strTz = bufTz;

#endif
	}

	wxString str;

	str += dtm.FormatISODate();
	str += _T(" ");
	str += dtm.FormatISOTime();
	if (bIncludeNanoseconds)
		str += wxString::Format(_T(".%09d"), dtm.GetMillisecond()*1000000);		// report in nanoseconds (sigh)
	if (bIncludeTZ)
	{
		str += _T(" ");
		str += strTz;
	}

	return str;
}

//////////////////////////////////////////////////////////////////

bool util_string__parse_hex_char(const char c,
								 unsigned int * pv)
{
	if ((c >= '0') && (c <= '9'))
	{
		*pv = (c - '0');
		return true;
	}

	if ((c >= 'a') && (c <= 'f'))
	{
		*pv = (c - 'a' + 10);
		return true;
	}

	if ((c >= 'A') && (c <= 'F'))
	{
		*pv = (c - 'A' + 10);
		return true;
	}

	*pv = '0';
	return false;
}
