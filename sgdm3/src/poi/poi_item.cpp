// poi_item.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>

//////////////////////////////////////////////////////////////////

poi_item::poi_item(void)
	: m_pPTable(NULL),
	  m_pPoiEditBuffer(NULL),
	  m_pPoiEditSrc(NULL),
	  m_saveCount(0)
{
}

poi_item::~poi_item(void)
{
//	wxLogTrace(TRACE_POI_DUMP, _T("poi_item[%p]:~poi_item:"), this);
}

//////////////////////////////////////////////////////////////////

util_error poi_item::setFileName(const wxFileName & f)
{
	m_filename = f;

//	wxLogTrace(TRACE_POI_DUMP,
//			   _T("poi_item::poi_item: [this %p][%s]"),
//			   this,
//			   m_filename.GetFullPath().wc_str());

	// normalize the filename.
	// wrap the log handler so that we catch all error messages.

	util_error err;

	util_logToString uLog(&err.refExtraInfo());

	// we do not fold case when normalizing pathname on win32
	// because we want the actual case when printing pathnames.

	int flagsNorm = wxPATH_NORM_ALL;	// the default (everything except case)
#if defined(__WXMSW__)
	flagsNorm &= ~wxPATH_NORM_ENV_VARS;	// turn off envvar expansion (see item:13183)
	flagsNorm &= ~wxPATH_NORM_SHORTCUT; // turn off .lnk expansion (see W6778)
#endif

	if (!m_filename.Normalize(flagsNorm))
		err.set(util_error::UE_INVALID_PATHNAME);

	return err;
}

//////////////////////////////////////////////////////////////////

void poi_item::setPTable(fim_ptable * pPTable)
{
	// allow set-when-null, clear-when-set, or clear-when-null only.
	// don't allow set-when-set.
	wxASSERT_MSG( (!m_pPTable || !pPTable), _T("Coding Error: poi_item::setPTable: ptable already set.") );

	// we don't own this pointer

	m_pPTable = pPTable;
}

//////////////////////////////////////////////////////////////////

void poi_item::setPoiEditBuffer(poi_item * pPoi)
{
	// call this on the POI of a document in a T1 VIEW window to associate
	// a editable POI (tempfile, piecetable).  normally, one should
	// call this after cloning T1 when enabling editing in a window.
	//
	// BUT the caller should see if this T1 POI already has one and
	// if so, just reference it.  this will give multiple windows
	// editing into one buffer -- which is better than having 2 different
	// edit buffers on a document.

	// allow set-when-null, clear-when-set, or clear-when-null only.
	// don't allow set-when-set.
	wxASSERT_MSG( (!m_pPoiEditBuffer || !pPoi), _T("Coding Error: poi_item::setPoiEditBuffer: already set.") );

	m_pPoiEditBuffer = pPoi;
}

void poi_item::setPoiEditSrc(poi_item * pPoi)
{
	// call this on the POI of a _BOTTOM window to associate it
	// back with the document of the corresponding T1 VIEW.
	// normally, one should call this after cloning T1.

	// allow set-when-null, clear-when-set, or clear-when-null only.
	// don't allow set-when-set.
	wxASSERT_MSG( (!m_pPoiEditSrc || !pPoi), _T("Coding Error: poi_item::setPoiEditSrc: already set.") );

	m_pPoiEditSrc = pPoi;
}

//////////////////////////////////////////////////////////////////

util_error poi_item::getFileSize(wxULongLong * pULL) const
{
	util_error err;

	util_logToString uLog(&err.refExtraInfo());
#if defined(__WXMSW__)
	*pULL = m_filename.GetSize();
#else
	// default wxFileName::GetSize() on Linux/Mac calls
	// wxStat and therefore follows symlinks.
	wxStructStat st;
	if (wxLstat( m_filename.GetFullPath().wc_str(), &st) != 0)
		err.set(util_error::UE_CANNOT_STAT_FILE);
	*pULL = wxULongLong(st.st_size);
#endif

	return err;
}

wxString poi_item::getFileSizeAsString(void) const
{
	wxULongLong ull(0);
	util_error ue = getFileSize(&ull);
	if (ue.isErr())
		return _T("?");

	wxULongLong_t uv = ull.GetValue();

	wxString str;
	str.Printf( ("%" wxLongLongFmtSpec "d"), uv );

	return str;
}

util_error poi_item::getDateTimeModified(wxDateTime * pDTM) const
{
	// try to stat() it and get the dtMod.
	// wrap the log handler so that we catch all error messages.
	//
	// wxWidgets file io routines spew MessageBox()'s on error -- most annoying.
	// capture error messages for display later.  use local scoping to limit
	// lifetime of log/error diversion while we try to stat the file.

	util_error err;

	util_logToString uLog(&err.refExtraInfo());

// TODO 2013/09/10 see if this calls wxStat or wxLstat....
	if (!m_filename.GetTimes(NULL,pDTM,NULL))
		err.set(util_error::UE_CANNOT_STAT_FILE);
	
//	wxLogTrace(TRACE_POI_DUMP,
//			   _T("poi_item::getDateTimeModified: [poi %p][%s][ue %d] [dtm %s]."),
//			   this,
//			   m_filename.GetFullPath().wc_str(),
//			   err.getErr(),
//			   ((pDTM->IsValid()) ? pDTM->Format().wc_str() : _T("DATE INVALID")));

	return err;
}

//////////////////////////////////////////////////////////////////

util_error poi_item::compareFileExact(const poi_item * pPoiItem, int * pResult)
{
	// compare the contents of files.  we perform an *exact* match.
	// set *pResult to: -1 on error, 0 when different, 1 when equal
	//
	// wrap the log handler so that we catch all error messages.

	util_error err;
	util_logToString uLog(&err.refExtraInfo());

	// don't use wxFile::Open() because of call to wxLogSysError().

	int fdThis = util_file_open(err,m_filename.GetFullPath(),wxFile::read);
	if (fdThis == -1)
	{
		*pResult = -1;
		return err;
	}
	wxFile fThis(fdThis);	// attach fd to wxFile
	
	int fdThat = util_file_open(err,pPoiItem->m_filename.GetFullPath(),wxFile::read);
	if (fdThat == -1)
	{
		*pResult = -1;
		return err;
	}
	wxFile fThat(fdThat);	// attach fd to wxFile
	
	off_t lenThis = fThis.Length();
	if (lenThis == -1)
	{
		*pResult = -1;
		err.set(util_error::UE_CANNOT_READ_FILE);
		return err;
	}
	
	off_t lenThat = fThat.Length();
	if (lenThat == -1)
	{
		*pResult = -1;
		err.set(util_error::UE_CANNOT_READ_FILE);
		return err;
	}

	if (lenThis != lenThat)		// if lengths are different, contents
	{							// dont matter -- they are different.
		*pResult = 0;
		err.set(util_error::UE_OK);
		return err;
	}

	// rip thru both files and do a quick EXACT MATCH test.
	// we do not do any of the ignore-whitespace, -eol, -case,
	// or character encoding conversions, so a pair of files
	// that we say are different here may appear equal in a
	// window (with all of that stuff turned on).

#define BUFSIZE	(8*1024)
	byte bufThis[BUFSIZE], bufThat[BUFSIZE];

	while (1)
	{
		int nbrThis = fThis.Read(bufThis,BUFSIZE);
		if (nbrThis == -1)
		{
			*pResult = -1;
			err.set(util_error::UE_CANNOT_READ_FILE);
			return err;
		}
		
		int nbrThat = fThat.Read(bufThat,BUFSIZE);
		if (nbrThat == -1)
		{
			*pResult = -1;
			err.set(util_error::UE_CANNOT_READ_FILE);
			return err;
		}
		
		if (nbrThis != nbrThat)		// one file grew/shrunk since we stat'd it ??
		{
			*pResult = 0;
			err.set(util_error::UE_OK);
			return err;
		}
		
		if (nbrThis == 0)			// eof, quit.
		{
			*pResult = 1;
			err.set(util_error::UE_OK);
			return err;
		}

		if (memcmp(bufThis,bufThat,nbrThis) != 0)	// if this buffer is different,
		{				  							// then we found a difference and can quit.
			*pResult = 0;
			err.set(util_error::UE_OK);
			return err;
		}
	}

	/*NOTREACHED*/

	*pResult = -1;
	err.set(util_error::UE_UNSPECIFIED_ERROR);
	return err;
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
bool poi_item::isDir(void) const
{
	return m_filename.IsOk() && m_filename.DirExists();
}

bool poi_item::isFile(void) const
{
	return m_filename.IsOk() && m_filename.FileExists();
}

bool poi_item::isLnk(void) const
{
	// return TRUE if we have a shell shortcut .lnk.

	bool bIsLnk = false;

	(void)util_file__is_shell_lnk( m_filename.GetFullPath(), &bIsLnk );

#if 0 && defined(DEBUG)	// just using this to test log trace
	if (bIsLnk)
	{
		util_shell_lnk * pLnk = NULL;
		(void)get_lnk_target( &pLnk );
		delete pLnk;
	}
#endif

	return bIsLnk;
}

util_error poi_item::get_lnk_target(util_shell_lnk ** ppLnk) const
{
	return util_shell_lnk::ctor( m_filename.GetFullPath(), ppLnk );
}

POI_TYPE poi_item::getPoiType(void) const
{
	if (isFile())
	{
		if (isLnk())
			return POI_T_SHORTCUT;
		return POI_T_FILE;
	}
	else if (isDir())
		return POI_T_DIR;

	return POI_T__LIMIT;	// invalid/unknown value
}

#elif defined(__WXMAC__) || defined(__WXGTK__)

bool poi_item::isDir(void) const
{
	return (getPoiType() == POI_T_DIR);
}

bool poi_item::isFile(void) const
{
	return (getPoiType() == POI_T_FILE);
}

bool poi_item::isSymlink(void) const
{
	return (getPoiType() == POI_T_SYMLINK);
}

util_error poi_item::get_symlink_target_raw(char * pBuf, int bufLen) const
{
	util_error err;

	int result = readlink(m_filename.GetFullPath().c_str(), pBuf, bufLen);
	if (result == -1)
		err.set(util_error::UE_CANNOT_READ_SYMLINK,
				m_filename.GetFullPath());

	pBuf[result] = 0;	// readlink() does not NUL terminate the buffer

	return err;
}
	
util_error poi_item::get_symlink_target(wxString & rStrTarget) const
{
	util_error err;
	char buf[4096];

	rStrTarget.Empty();

	err = get_symlink_target_raw(buf, ((int)NrElements(buf)));
	if (err.isOK())
		rStrTarget = wxString::FromUTF8(buf);

	return err;
}

POI_TYPE poi_item::getPoiType(void) const
{
	if (!m_filename.IsOk())
		return POI_T__LIMIT;

	wxStructStat statbuf;
	int result = wxLstat(m_filename.GetFullPath().wc_str(), &statbuf);
	if (result != 0)
		return POI_T__LIMIT;	// invalid/unknown value

	if (S_ISDIR(statbuf.st_mode))
		return POI_T_DIR;
	if (S_ISREG(statbuf.st_mode))
		return POI_T_FILE;
	if (S_ISLNK(statbuf.st_mode))
		return POI_T_SYMLINK;

	return POI_T__LIMIT;
}

#else

#  error TODO

#endif

/*static*/ POI_TYPE poi_item::getPoiType(const poi_item * poi)
{
	if (poi)
		return poi->getPoiType();
	else
		return POI_T_NULL;
}

//////////////////////////////////////////////////////////////////

bool poi_item::tryToMakeWritable(void) const
{
	if (isFile())		// if pathname refers to a file and the file exists.
	{
		// try to chmod() it and give us read/write.

#if defined(__WXMSW__)

		// wxBUG: do not use wxFile::Access() on windows.  it does not return the
		// wxBUG: correct answer when passed wxFile::read_write
		// wxBUG:
		// wxBUG: this may be because of an ACL on the file that superceedes the
		// wxBUG: discretionary mode bits.  this may also cause the chmod() to
		// wxBUG: appear to work, but actually not affect the file.

		int result = _wchmod(m_filename.GetFullPath().wc_str(), _S_IREAD|_S_IWRITE);
		return (result == 0);

#elif defined(__WXMAC__) || defined(__WXGTK__)

		wxStructStat stat;
		if (wxStat(m_filename.GetFullPath().wc_str(), &stat) != 0)
			return false;

		int oldMode = stat.st_mode & 0777;
		int newMode = oldMode | 0600;	// this may strip SUID/SGID/SVTX but that's probably a good thing.

		int result = chmod(m_filename.GetFullPath().fn_str(), newMode);	// must use fn_str() since chmod() is native
		return (result == 0);
#endif
	}

	return false;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void poi_item::dump(int indent) const
{
	wxLogTrace(TRACE_POI_DUMP, _T("%*cPOI_ITEM: [%p][%s][ptable %p][saveCount %d]"),
			   indent,' ',this,
			   m_filename.GetFullPath().wc_str(),
			   m_pPTable,
			   m_saveCount
			   );
}
#endif
