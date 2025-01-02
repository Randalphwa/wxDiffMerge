// fd_softmatch__simple_mode_compare.cpp
// simple-mode soft-match routine for folder windows.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fd.h>

//////////////////////////////////////////////////////////////////

util_error fd_softmatch::compareFileSimpleMode(const poi_item * pPoiItem1,
											   const poi_item * pPoiItem2,
											   int * pResult,
											   wxString & strSoftMatchInfo) const
{
	// compare the contents of files.  we perform a *soft* match which
	// ignores various things (such as EOL, whitespace, and etc).  this
	// is an attempt to reduce the odds of the user seeing a set of files
	// listed in the folder window as different but when they double click
	// on them they get the "files are identical" dialog.
	//
	// note: this is because of all of the "rounding" that happens in the
	// full diff-engine.  the usual ignorables ***and*** the hide-unimportant
	// settings.  we ***DO NOT*** deal with any of the hide-unimportant
	// stuff, so it is still possible to get a false-positive, but with this
	// we should be much closer.
	//
	// note: we read the files as raw 8-bit byte data and ***DO NOT***
	// deal with any of the unicode/character encoding stuff, so we
	// ***CANNOT*** do case folding.
	// 
	// set *pResult to: -1 on error, 0 when different, 2 when equivalent
	//
	// wrap the log handler so that we catch all error messages.

#define RESULT_ERROR	-1
#define RESULT_DIFF		0
#define RESULT_EQUIV	2

#define FAIL(r)					do { *pResult = (r); goto fail; } while (0)
#define RETURN_FAIL(r)			do { *pResult = (r); return err; } while (0)
#define RETURN_FAIL_ERR(r,e)	do { *pResult = (r); err.set(util_error::e); return err; } while (0)
#define SUCCEED(r)				do { *pResult = (r); goto succeed; } while (0)

	strSoftMatchInfo.Empty();

	// if none of the simple mode features is turned on, the
	// caller should not have called us.
	wxASSERT_MSG(  (m_bSimpleModeIgnoreEOL || m_bSimpleModeIgnoreWhitespace || m_bSimpleModeTABisWhitespace),
				   _T("Coding Error")  );

	util_error err;
	util_logToString uLog(&err.refExtraInfo());

	// don't use wxFile::Open() because of call to wxLogSysError().

	int fdThis = util_file_open(err,pPoiItem1->getFullPath(),wxFile::read);
	if (fdThis == -1)
		RETURN_FAIL(RESULT_ERROR);
	wxFile fThis(fdThis);	// attach fd to wxFile
	
	int fdThat = util_file_open(err,pPoiItem2->getFullPath(),wxFile::read);
	if (fdThat == -1)
		RETURN_FAIL(RESULT_ERROR);
	wxFile fThat(fdThat);	// attach fd to wxFile

#define BUFSIZE	(8*1024)
	byte bufThis[BUFSIZE+1], bufThat[BUFSIZE+1];
	int lenThis = 0;		// amount of data in buffer
	int lenThat = 0;
	int kThis = 0;			// index we are looking at in buffer
	int kThat = 0;

	bool bEofThis = false;
	bool bEofThat = false;

	while (1)
	{
		if (!bEofThis && (BUFSIZE-lenThis > 0))
		{
			int nbrThis = fThis.Read(&bufThis[lenThis],BUFSIZE-lenThis);
			if (nbrThis == -1)
				RETURN_FAIL_ERR(RESULT_ERROR,UE_CANNOT_READ_FILE);
			bEofThis = (nbrThis == 0);
			lenThis += nbrThis;
			bufThis[lenThis] = 0;
		}

		if (!bEofThat && (BUFSIZE-lenThat > 0))
		{
			int nbrThat = fThat.Read(&bufThat[lenThat],BUFSIZE-lenThat);
			if (nbrThat == -1)
				RETURN_FAIL_ERR(RESULT_ERROR,UE_CANNOT_READ_FILE);
			bEofThat = (nbrThat == 0);
			lenThat += nbrThat;
			bufThat[lenThat] = 0;
		}

		// if we've reached the end of both files and completely
		// processed both buffers, we're done.  we let this loop
		// continue when we run out on one side because the other
		// could be all trailing whitespace (that we want to ignore).

		if (bEofThis && (lenThis == 0) && bEofThat && (lenThat == 0))
			SUCCEED(RESULT_EQUIV);

		while (1)
		{
			while ((kThis < lenThis)
				   && (m_bSimpleModeIgnoreWhitespace
					   && ((bufThis[kThis] == 0x20)
						   || (m_bSimpleModeTABisWhitespace && (bufThis[kThis] == 0x09)))))
				kThis++;

			while ((kThat < lenThat)
				   && (m_bSimpleModeIgnoreWhitespace
					   && ((bufThat[kThat] == 0x20)
						   || (m_bSimpleModeTABisWhitespace && (bufThat[kThat] == 0x09)))))
				kThat++;

			if (bEofThis && bEofThat && ((kThis < lenThis) != (kThat < lenThat)))
				SUCCEED(RESULT_DIFF);

			if ((kThis == lenThis) || (kThat == lenThat))
				break;

			if (m_bSimpleModeIgnoreEOL)
			{
				if (bufThis[kThis] == 0x0d)
					if ((kThis+1 == lenThis) && !bEofThis)
						break;		// CR at end of buffer; get more data to see if there is an LF
				if (bufThat[kThat] == 0x0d)
					if ((kThat+1 == lenThat) && !bEofThat)
						break;

				if ((bufThis[kThis] == 0x0d) && (bufThis[kThis+1] == 0x0a))
				{
					if ((bufThat[kThat] == 0x0d) && (bufThat[kThat+1] == 0x0a))
					{	kThis += 2; kThat += 2; continue; }
					else if ((bufThat[kThat] == 0x0d) || (bufThat[kThat] == 0x0a))
					{	kThis += 2; kThat += 1; continue; }
					else
					{	SUCCEED(RESULT_DIFF); }
				}
				else if ((bufThis[kThis] == 0x0d) || (bufThis[kThis] == 0x0a))
				{
					if ((bufThat[kThat] == 0x0d) && (bufThat[kThat+1] == 0x0a))
					{	kThis += 1; kThat += 2; continue; }
					else if ((bufThat[kThat] == 0x0d) || (bufThat[kThat] == 0x0a))
					{	kThis += 1; kThat += 1; continue; }
					else
					{	SUCCEED(RESULT_DIFF); }
				}
			}

			// do not put a tolower() on this because we do not have
			// any idea what character encoding the data is in.  we
			// read raw data and *hope* that it is US-ASCII compatible
			// so that CR, LF, TAB, and SP are defined.

			if (bufThis[kThis] == bufThat[kThat])
			{	kThis++; kThat++; continue; }

			SUCCEED(RESULT_DIFF);
		}

		// we ran out of data in at least one of the buffers.
		// copy the unconsumed part to the beginning of the buffer
		// so we can get some more.

		int remainderThis = lenThis - kThis;
		if (remainderThis > 0)
			memmove(bufThis,&bufThis[kThis],remainderThis);
		kThis = 0;
		lenThis = remainderThis;

		int remainderThat = lenThat - kThat;
		if (remainderThat > 0)
			memmove(bufThat,&bufThat[kThat],remainderThat);
		kThat = 0;
		lenThat = remainderThat;
	}

	//NOTREACHED
	wxASSERT_MSG( (0), _T("Coding Error") );

succeed:
	if (*pResult == RESULT_DIFF)
		strSoftMatchInfo = _T("Different using Simple Match");
	else
		strSoftMatchInfo = _T("Equivalent using Simple Match");

	err.clear();
	return err;
}
