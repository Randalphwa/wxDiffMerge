// fd_softmatch__ruleset_mode_compare.cpp
// ruleset-mode soft-match routine for folder windows.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

static util_error _try_import_buffer(util_encoding enc,
									 byte * pRawBuffer, off_t lenBuffer,
									 wxChar ** ppWideBuffer, size_t * pLenWideBuffer)
{
	util_error ue;
	wxMBConv * pConv = NULL;
	wxChar * pWideBuffer = NULL;

	ue = util_encoding_does_buffer_have_nulls(enc, pRawBuffer, lenBuffer);
	if (ue.isOK())
	{
		// see all of the discussion in fim_ptable::_import_conv() on the various quirks.
		util_logToString uLog(&ue.refExtraInfo());

		// see discussion about avoiding asserts in util_enc on wxFONTENCODING_SYSTEM and _DEFAULT.
		util_encoding encNormalized = ((enc == wxFONTENCODING_DEFAULT) ? wxFONTENCODING_SYSTEM : enc);
		pConv = util_encoding_create_conv(encNormalized);

		size_t lenNeeded = pConv->MB2WC(NULL,(const char *)pRawBuffer,0);
		if ((ue.getExtraInfo().Length() > 0)  ||  (lenNeeded==0)  ||  (lenNeeded==(size_t)-1))
		{
			ue.set(util_error::UE_CANNOT_IMPORT_CONV);
		}
		else
		{
			pWideBuffer = (wxChar *)calloc((lenNeeded+1),sizeof(wxChar));
			size_t lenUsed = pConv->MB2WC(pWideBuffer,(const char *)pRawBuffer,(lenNeeded+1));
			wxASSERT_MSG( (lenUsed==lenNeeded), _T("Coding Error") );

			*ppWideBuffer = pWideBuffer;
			*pLenWideBuffer = lenUsed;

			pWideBuffer = NULL;
		}
	}

	DELETEP( pConv );
	FREEP( pWideBuffer );
	return ue;
}

static util_error _load_file(const poi_item * pPoiItem, const rs_ruleset * pRS,
							 long lenFileLimitMb,
							 wxChar ** ppWideBuffer, size_t * pLenWideBuffer,
							 util_encoding * pEncReturned, bool * pbHadBOM)
{
	// try to load the given file into a buffer, determine the character
	// encoding, convert the content to Unicode, and return an allocated
	// buffer.

	util_error ue;

	wxFile file;

	byte * pRawBuffer = NULL;
	off_t lenBOM = 0;
	off_t lenFile;

	util_encoding enc[3] = { wxFONTENCODING_DEFAULT, wxFONTENCODING_DEFAULT, wxFONTENCODING_DEFAULT };
	int nrEnc = 0;

	// don't use wxFile::Open() because of call to wxLogSysError().

	{
		util_logToString uLog(&ue.refExtraInfo());
		int fd = util_file_open(ue, pPoiItem->getFullPath(),wxFile::read);
		if (fd == -1)
			goto Failed;
		file.Attach(fd);
		
		lenFile = file.Length();
		if (lenFile == 0)
		{
			*pEncReturned = wxFONTENCODING_DEFAULT;		// doesn't matter for zero-length file.
			*pbHadBOM = false;
			*ppWideBuffer = NULL;
			*pLenWideBuffer = 0;
			goto Success;
		}

		// BUGBUG the wxWidgets character encoding routines are broken in that they
		// don't have any way of safely doing a conversion in chunks.  the routines
		// APIs assume that the entire input buffer can be converted.  they don't
		// have any way of indicating that a multi-byte (say, utf8) input buffer
		// was partially converted to wchar/Unicode and had a remainder (for incomplete
		// sequences at the end of the buffer).  ICU has this.
		//
		// Therefore, we read the entire file into a buffer and let them convert it
		// in one large piece.  I can't really fault the wxWidgets folks, because they
		// are just calling MultiByteToWideChar() and mbstowcs() and they don't have
		// the extra args either.
		//
		// BUT this does cause us to tax memory, so we put in the _FILE_LIMIT_MB stuff
		// to prevent us from trying large files.

		if ((lenFileLimitMb > 0) && ((long)(lenFile/(1024*1024)) > lenFileLimitMb))
		{
			ue.set(util_error::UE_LIMIT_EXCEEDED);
			goto Failed;
		}

		pRawBuffer = (byte *)calloc((lenFile+10),sizeof(byte));
		if (!pRawBuffer)
		{
			ue.set(util_error::UE_CANNOT_ALLOC);
			goto Failed;
		}

		if (file.Read(pRawBuffer,lenFile) == -1)
		{
			ue.set(util_error::UE_CANNOT_READ_FILE);
			goto Failed;
		}
	}
	
	// deal with Byte-Order-Mark

	if (pRS->getSniffEncodingBOM())
	{
		off_t lenMaxBOM = MyMin(lenFile,4);
		lenBOM = util_encoding_sniff_bom(pRawBuffer,lenMaxBOM,&enc[0]);	// returns -1 when no BOM
		if (lenBOM < 0)
			lenBOM = 0;
		else
			nrEnc = 1;
	}

	if (lenBOM == 0)
	{
		switch (pRS->getEncodingStyle())
		{
		case RS_ENCODING_STYLE_NAMED1:
		case RS_ENCODING_STYLE_NAMED2:
		case RS_ENCODING_STYLE_NAMED3:
			nrEnc = pRS->getNamedEncodingArray( NrElements(enc), enc );
			break;

		case RS_ENCODING_STYLE_LOCAL:
			enc[0] = wxFONTENCODING_DEFAULT;
			nrEnc = 1;
			break;

//		case RS_ENCODING_STYLE_ASK:
//		case RS_ENCODING_STYLE_ASK_EACH:
		default:
			ue.set(util_error::UE_CANNOT_AUTOMATICALLY_DETERMINE_CHARENC);
			goto Failed;
		}
	}

	if (lenFile == lenBOM)
	{
		*pEncReturned = enc[0];
		*pbHadBOM = (lenBOM > 0);
		*ppWideBuffer = NULL;
		*pLenWideBuffer = 0;
		goto Success;
	}

	int k;
	for (k=0; k<nrEnc; k++)
	{
		ue.clear();
		ue = _try_import_buffer(enc[k], pRawBuffer+lenBOM, lenFile-lenBOM,
								ppWideBuffer, pLenWideBuffer);
		if (ue.isOK())
		{
			*pEncReturned = enc[k];
			*pbHadBOM = (lenBOM > 0);
			goto Success;
		}
	}

	// none of the candidate encodings worked.
	goto Failed;

Success:
	FREEP(pRawBuffer);
	ue.clear();		// probably not needed
	return ue;

Failed:
	FREEP(pRawBuffer);
	return ue;
}

#define EOL_NONE		0
#define EOL_IS_CRLF		1
#define EOL_IS_LF		2
#define EOL_IS_CR		3

static wxChar * _find_next_line(wxChar * pBuf, int * pEolType)
{
	// find the start of the next line.  we are given a pointer
	// into a large buffer.  search forward for a CRLF, LF, or CR
	// and return the address of the character after the EOL.
	//
	// if we reach the terminating NULL, return the address of it.
	//
	// also return the type of the EOL that we saw.
	//
	// store a NULL at the start of the EOL (so that upon return
	// pBuf points to a single line (and doesn't require any data
	// copies))

	wxChar * p = pBuf;

	while (*p)
	{
		if (*p == 0x000a)
		{
			*pEolType = EOL_IS_LF;
			*p = 0;
			return p+1;
		}
		
		if (*p == 0x000d)
		{
			if (p[1] == 0x000a)
			{
				*pEolType = EOL_IS_CRLF;
				*p = 0;
				return p+2;
			}
			else
			{
				*pEolType = EOL_IS_CR;
				*p = 0;
				return p+1;
			}
		}
		
		p++;
	}

	*pEolType = EOL_NONE;
	return p;
}

//////////////////////////////////////////////////////////////////

util_error fd_softmatch::compareFileRulesetMode(const poi_item * pPoiItem1,
												const poi_item * pPoiItem2,
												const rs_ruleset * pRS,
												int * pResult,
												wxString & strSoftMatchInfo) const
{
	// compare the contents of files.  we perform a *soft* match which
	// uses the various properties in the ruleset to ignore subtle/insignificant
	// differences.
	//
	// again, this is an attempt to reduce the odds of the user seeing a
	// set of files listed in the folder window as different but when they
	// double click on them they get the "files are identical" dialog.
	//
	// this version *****TRIES***** to emulate as much of the diff-engine
	// as we can *without* actually using the diff-engine.  (using the
	// full diff-engine would require us to build piecetables and layout
	// information and then run the CSS (at least in line-mode).  this
	// is a lot of stuff.  since we don't actually care *what* the differences
	// are -- just that there *are* some, we can probably do better here.
	//
	// we allow ourselves to abort the comparison if there's something
	// either not safe (huge files) or would require us to ask the user
	// a question (such as the character encoding if we can't detect it).
	// in these cases, we silently return 0 -- and let our caller pretend
	// that no ruleset could be found to apply.
	//
	//////////////////////////////////////////////////////////////////
	//
	// [] read the files line-by-line.
	//    () apply ruleset eol ignore/strip
	//    () apply ruleset LINES-to-OMIT rules.  (takes care of RCSID fields)
	//    () apply ruleset case folding and whitespace rules.
	//    () compare whatever we have left.
	//
	// set *pResult to: -1 on error, 0 when different, 2 when equivalent

	strSoftMatchInfo.Empty();

#define RESULT_ERROR	-1
#define RESULT_DIFF		0
#define RESULT_EQUIV	2

#define FAIL(r)			do { *pResult = (r); goto fail; } while (0)
#define SUCCEED(r)		do { *pResult = (r); goto succeed; } while (0)

	util_error ue;

	wxChar * pWideBuffer1 = NULL;
	wxChar * pWideBuffer2 = NULL;
	size_t lenWideBuffer1, lenWideBuffer2;

	wxChar * pBufferEnd1;
	wxChar * pBufferEnd2;
	wxChar * pLineBegin1;
	wxChar * pLineBegin2;
	wxChar * pLineNext1;
	wxChar * pLineNext2;
	wxChar * p1;
	wxChar * p2;

	rs_context_attrs attrsEquivalenceStrip = pRS->getEquivalenceAttrs();
	bool bGlobalRespectEOL   = RS_ATTRS_RespectEOL(attrsEquivalenceStrip);
	bool bGlobalRespectWhite = RS_ATTRS_RespectWhite(attrsEquivalenceStrip);
	bool bGlobalRespectCase  = RS_ATTRS_RespectCase(attrsEquivalenceStrip);
	bool bGlobalTabIsWhite   = RS_ATTRS_TabIsWhite(attrsEquivalenceStrip);

	bool bIgnoreWhite = !bGlobalRespectWhite;

	int nrLinesToSkip1 = 0;
	int nrLinesToSkip2 = 0;

	int eolType1;
	int eolType2;

	util_encoding enc1, enc2;
	bool bHadBOM1, bHadBOM2;

	// read each file into a unicode buffer.

	ue = _load_file(pPoiItem1,pRS, m_nRulesetFileLimitMb,
					&pWideBuffer1,&lenWideBuffer1,&enc1,&bHadBOM1);
	if (ue.isErr())
		FAIL(RESULT_ERROR);

	ue = _load_file(pPoiItem2,pRS, m_nRulesetFileLimitMb,
					&pWideBuffer2,&lenWideBuffer2,&enc2,&bHadBOM2);
	if (ue.isErr())
		FAIL(RESULT_ERROR);

	// process the unicode buffers line-by-line.

	pBufferEnd1 = pWideBuffer1 + lenWideBuffer1;
	pBufferEnd2 = pWideBuffer2 + lenWideBuffer2;

	pLineBegin1 = pWideBuffer1;
	pLineBegin2 = pWideBuffer2;

	while ((pLineBegin1 < pBufferEnd1) && (pLineBegin2 < pBufferEnd2))
	{
		// isolate the current line within the buffer (put a NULL where
		// the EOL is) and determine if we should look at it.  if not,
		// eat the lines-to-omit and loop until we have a line that we
		// should look at.
		//
		// W8032: we amend the skipping to check if the line immediately
		// following the omission should also be omitted.  there are 2
		// ways to do this:
		// [] skip n lines for the first match and then check the single
		//    following line.
		// [] skip n lines but look at each and reset the count if we
		//    find another match.
		// i'm going with the latter.
		//
		// these loops are safe if pLineBegin[12] is pointing
		// at pBufferEnd[12].

		pLineNext1 = _find_next_line(pLineBegin1,&eolType1);
		nrLinesToSkip1 = pRS->testLOmit(pLineBegin1);
		while (nrLinesToSkip1 > 0)
		{
			pLineBegin1 = pLineNext1;
			pLineNext1 = _find_next_line(pLineBegin1,&eolType1);
			nrLinesToSkip1--;

			int x = pRS->testLOmit(pLineBegin1);
			if (x > 0)
				nrLinesToSkip1 = x;
		}

		pLineNext2 = _find_next_line(pLineBegin2,&eolType2);
		nrLinesToSkip2 = pRS->testLOmit(pLineBegin2);
		while (nrLinesToSkip2 > 0)
		{
			pLineBegin2 = pLineNext2;
			pLineNext2 = _find_next_line(pLineBegin2,&eolType2);
			nrLinesToSkip2--;

			int x = pRS->testLOmit(pLineBegin2);
			if (x > 0)
				nrLinesToSkip2 = x;
		}

		// if we've reached the end of both buffers, we must have
		// equivalent files.
		//
		// we do not stop here if we only reach the end of one of
		// them, because the remainder of the other could be omitted
		// content (such as blank lines).

		if ((pLineBegin1 == pBufferEnd1) && (pLineBegin2 == pBufferEnd2))
			break;

		// if we are respecting EOLs (not ignoring them) and the EOLs
		// are different on these lines, we're done.

		if (bGlobalRespectEOL && (eolType1 != eolType2))
			SUCCEED(RESULT_DIFF);

		// look within this pair of lines and see if they are "equivalent"
		//
		// we scan both lines until BOTH are pointing at the NULL, we allow
		// the loop to continue when only one is NULL in case the other has
		// trailing whitespace.

		p1 = pLineBegin1;
		p2 = pLineBegin2;
		while (*p1 || *p2)
		{
			if (bIgnoreWhite)
			{
				if (bGlobalTabIsWhite)
				{
					while ((*p1==0x0020) || (*p1==0x0009))
						p1++;
					while ((*p2==0x0020) || (*p2==0x0009))
						p2++;
				}
				else
				{
					while (*p1==0x0020)
						p1++;
					while (*p2==0x0020)
						p2++;
				}
			}

			if ((*p1==0) && (*p2==0))		// at the end of both lines
				break;
			
			// at least one of them is non-null; both of them are non-white.
			
			if (*p1 != *p2)
			{
				if (bGlobalRespectCase)
					SUCCEED(RESULT_DIFF);
				if (wxTolower(*p1) != wxTolower(*p2))
					SUCCEED(RESULT_DIFF);
			}

			// the 2 characters are equal or equal when case is folded.

			if (*p1)
				p1++;
			if (*p2)
				p2++;
		}

		// the lines are equivalent, 

		pLineBegin1 = pLineNext1;
		pLineBegin2 = pLineNext2;
	}

	// we reached the end of at least one of the buffers.
	
	if ((pLineBegin1 < pBufferEnd1) || (pLineBegin2 < pBufferEnd2))
		SUCCEED(RESULT_DIFF);

	// we reached the end of both buffers.

	SUCCEED(RESULT_EQUIV);

succeed:
	if (*pResult == RESULT_DIFF)
		strSoftMatchInfo.Printf(_("Different using Ruleset: %s / "),pRS->getName().wc_str());
	else
		strSoftMatchInfo.Printf(_("Equivalent using Ruleset: %s / "),pRS->getName().wc_str());
	strSoftMatchInfo.Append(wxFontMapper::GetEncodingName(enc1));
	if (bHadBOM1)
		strSoftMatchInfo.Append(_("(BOM)"));
	if ((enc1 != enc2) || (bHadBOM1 != bHadBOM2))
	{
		strSoftMatchInfo.Append(_(" : "));
		strSoftMatchInfo.Append(wxFontMapper::GetEncodingName(enc2));
		if (bHadBOM2)
			strSoftMatchInfo.Append(_("(BOM)"));
	}

	FREEP(pWideBuffer1);
	FREEP(pWideBuffer2);
	ue.clear();
	return ue;

fail:
	// if we get *any* error, just fall back to the original exact match status so
	// that the user sees a "different" status rather than an "error" status (which
	// would prevent them from double-clicking on the pair.  we want this for the
	// cases where the file-size-limit was reached or where we couldn't automatically
	// guess the character encoding.
	//
	// we assume that we won't get access/open/read errors because the exact-match
	// code already ran and didn't have any problems.

	switch (ue.getErr())
	{
	case util_error::UE_CANNOT_IMPORT_CONV:
	case util_error::UE_NO_UNICODE_BOM:
	case util_error::UE_CONV_ODD_BUFFER_LEN:
	case util_error::UE_CONV_BUFFER_HAS_NUL:
	case util_error::UE_CANNOT_AUTOMATICALLY_DETERMINE_CHARENC:
		strSoftMatchInfo.Printf(_("Different using Exact Match (Could not auto-detect/process character encoding using Ruleset: %s)."),
								pRS->getName().wc_str());
		break;

	case util_error::UE_LIMIT_EXCEEDED:
		strSoftMatchInfo.Printf(_("Different using Exact Match (File size limit exceeded for ruleset equivalence testing)."));
		break;

	default:
		strSoftMatchInfo.Printf(_("Different using Exact Match (Error: '%s'  Using Ruleset: %s)."), ue.getMessage().wc_str(), pRS->getName().wc_str());
		break;
	}
	
	*pResult = 0;

	FREEP(pWideBuffer1);
	FREEP(pWideBuffer2);
	ue.clear();
	return ue;
}
