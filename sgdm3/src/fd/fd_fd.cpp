// fd_fd.cpp
// a folder diff
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// WXBUG: wxDir::Open() and wxDir::GetFirst() are somewhat broken.
// WXBUG:
// WXBUG: wxDir::Open() is somewhat broken (it only allocs the data
// WXBUG: structure used to hold the file handle or whatever); the dir
// WXBUG: doesn't actually get opened until GetFirst() is called.
// WXBUG: on platforms where wxDir::Open() actually does something,
// WXBUG: it pushes a null-log before calling wxDir::Open(), so if an
// WXBUG: error occurs (like no-read-access), we don't get the error
// WXBUG: message.  (we can probably call ::wxSysErrorCode() to see
// WXBUG: what happened, but we'll have to work to get a message that
// WXBUG: looks the same.)
//
// WXBUG the mac version of wxDirTraverser has it's own problems.
// WXBUG
// WXBUG [] it asserts (in debug mode) if it gets an error reading
// WXBUG    the directory.
// WXBUG
// WXBUG [] GetFirst() fails on empty directories -- even if wxDIR_DOTDOT
// WXBUG    is given.  yes, there's code in dirmac.cpp to handle these,
// WXBUG    but the routine fails before we get to it....  [so we can't
// WXBUG    tell if a directory is empty or if we don't have access to
// WXBUG    read it.]  it also means we can't use the _have_dir_access()
// WXBUG    that WIN and LINUX use.
//
// WORKAROUND: i'm going to side-step all this weirdness and do an
// WORKAROUND: explicit test to see if we have access *BEFORE* letting
// WORKAROUND: the traverser enter a directory or sub-directory.  if
// WORKAROUND: we don't have access, we'll tell the traverser to just
// WORKAROUND: ignore the directory.  see _have_dir_access().
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// WXBUG On MAC, a filename with a ":" in it gets translated into
// WXBUG a "/" by the pathname normalization routines.  this causes
// WXBUG us problems because we can't address the file.
//
// TODO decide if we care and if we want to fix.
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// WXBUG On MAC, there is still a problem where we can't traverse
// WXBUG thru /dev.
//
// TODO decide if we care and if we want to fix.
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__) || defined(__WXGTK__)
static util_error _have_dir_access(const wxString & dirname)
{
	//////////////////////////////////////////////////////////////////
	// workaround for problems in wxDir::Open() and the wxDirTraverser
	//
	// verify that we can read the directory -- by trying to do it
	// directly -- this is a little more expensive than a simple
	// access() or stat() call, but it does let wxWindows take care
	// of the unicode-to-pathname conversions, platform-specifics,
	// and let's us get a uniform error message.
	//
	// also, in Win32, _access() is documented "...as only determining
	// if a directory exists -- all directories have RW access..."
	//
	// also, on Linux, access() is documented as possibly not working
	// correctly on NFS file systems with UID mapping enabled.
	//////////////////////////////////////////////////////////////////

	// divert wxWindows system error log to our buffer before
	// creating wxDir, since it emits error messages rather than
	// returning errors codes.
	//
	// create wxDir object and (allegedly) call opendir() on it.
	// then actually try to read first entry in directory.
	// if this fails, then IsOpened() lied to us or we don't
	// have read access.  [since we included wxDIR_DOTDOT, we
	// don't have to worry about the directory being empty.]

	util_error err;
	bool bOK;

	{	// push temporary active log target using local scoping

		util_logToString uLog(&err.refExtraInfo());
		wxDir d(dirname);
		wxString f;
		bOK = ( (d.IsOpened()) && (d.GetFirst(&f,wxEmptyString,wxDIR_DEFAULT|wxDIR_DOTDOT)) );

	}	// pop temporary active log target
	
	if (bOK)
		return err;
	
	//////////////////////////////////////////////////////////////////
	// WXBUG some platforms spew error messages to the active
	// WXBUG log device when either of the directory operations
	// WXBUG fail.  other platforms, just silently return false.
	// WXBUG (sigh)
	//
	// so, if no system error message was generated, let's stuff
	// the folder pathname into our extra info.
	//
	// TODO should we also get perror()-type message ??
	//////////////////////////////////////////////////////////////////

	if (err.getExtraInfo().Length() == 0)
		err.set(util_error::UE_CANNOT_OPEN_FOLDER,dirname);
	else
		err.set(util_error::UE_CANNOT_OPEN_FOLDER);

	return err;
}
#elif defined(__WXMAC__)
static util_error _have_dir_access(const wxString & dirname)
{
	//////////////////////////////////////////////////////////////////
	// workaround for problems in wxDir::Open() and the wxDirTraverser
	//////////////////////////////////////////////////////////////////

	util_error err;

	int r = wxAccess(dirname, R_OK|X_OK);

	if (r == -1)
		err.set(util_error::UE_CANNOT_OPEN_FOLDER,dirname);

	return err;
}
#endif

//////////////////////////////////////////////////////////////////
// _fd_fd_loadFolder -- a private wxDirTraverser for loadFolders()

class _fd_fd_loadFolder : public wxDirTraverser
{
public:
	_fd_fd_loadFolder(fd_fd * pFdFd, int k, const wxString & strRoot,
					  const fd_filter * pFdFilter,
					  const fd_softmatch * pFdSoftmatch,
					  bool bUsePreviousSoftmatchResult)
		: m_pFdFd(pFdFd),
		  m_k(k),
		  m_strRoot(strRoot),
		  m_pFdFilter(pFdFilter),
		  m_pFdSoftmatch(pFdSoftmatch),
		  m_bUsePreviousSoftmatchResult(bUsePreviousSoftmatchResult)
		{};

	//////////////////////////////////////////////////////////////////
	// WXBUG: some parts of the wxDir docs refer to an OnOpenError() other
	// WXBUG: parts do not.  there is a call to it in the source, but since
	// WXBUG: (on MSW, at least) wxDir::Open() is broken, this never gets
	// WXBUG: called.
	//
	//	virtual wxDirTraverseResult OnOpenError(const wxString & filename)
	//		{
	//			wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnOpenError: [%s]"), filename.wc_str());
	//			return wxDIR_IGNORE;
	//		};
	//////////////////////////////////////////////////////////////////

	virtual wxDirTraverseResult OnFile(const wxString & filename)
		{
//			wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnFile: [%s]"), filename.wc_str());

			if (m_pFdFilter->isFilteredFullFilename(filename))
			{
//				wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnFile: FILTERED FULLNAME [%s]"), filename.wc_str());
				return wxDIR_CONTINUE;
			}

			if (m_pFdFilter->isFilteredSuffix(filename))
			{
//				wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnFile: FILTERED SUFFIX [%s]"), filename.wc_str());
				return wxDIR_CONTINUE;
			}

			// extract pathname relative to our fd root[k] (so treewalk in both folders will be comparable).
			// 
			// Originally the code to get the relative part used
			// wxFileName::MakeRelativeTo(), but on Windows this
			// will expand .lnk targets and we don't want the
			// target (which might be a printer or server or a
			// network thingy).
			//
			// So just use the fact that all of the pathnames
			// we see here should be children of the root of the
			// traversal.
			wxString strTail;
			if (!filename.StartsWith(m_strRoot, &strTail))
			{
				// This should not happen.  I want to assert here,
				// but I don't have a way to do it.  Just use the
				// entire pathname for now.  This will cause the
				// row to look weird in the folder window, but we
				// won't crash.
				strTail = filename;
			}
//			wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnFile: [%s][%s]"),
//					   filename.wc_str(),
//					   strTail.wc_str());
				
			// use absolute path (as received) for the POI.
			poi_item * pPoiItem = gpPoiItemTable->addItem(filename);

#if defined(__WXMSW__)
			if (pPoiItem->isLnk())
				strTail += "^";
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
			if (pPoiItem->isSymlink())		// like /bin/ls -F
				strTail += "@";
#endif

			// use relative path as key in fd_table.
			fd_item * pFdItem = m_pFdFd->m_table.addItem(m_k,&strTail);

			// populate kth half of fd_item pair.
			pFdItem->setPoiItem(m_k,pPoiItem);
			
			return wxDIR_CONTINUE;
		};

	virtual wxDirTraverseResult OnDir(const wxString & dirname)
		{
//			wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnDir: [%s]"), dirname.wc_str());

			if (m_pFdFilter->isFilteredSubdir(dirname))
			{
//				wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnDir: FILTERED [%s]"), dirname.wc_str());
				return wxDIR_IGNORE;
			}

			// extract pathname relative to our fd root[k] (so treewalk in
			// both folders will be comparable).
			//
			// this leaves the trailing '/' on the relative path.  this causes
			// a f1/file_or_dir and f2/file_or_dir/ to appear as 2 different
			// singleton line items -- rather than as a single MISMATCH.

			wxString strSep( wxFileName::GetPathSeparator() );
			wxString strPath(dirname);
			if (!strPath.EndsWith(strSep))
				strPath += strSep;
			wxString strTail;
			if (!strPath.StartsWith(m_strRoot, &strTail))
			{
				// This should not happen.  I want to assert here,
				// but I don't have a way to do it.  Just use the
				// entire pathname for now.  This will cause the
				// row to look weird in the folder window, but we
				// won't crash.
				strTail = dirname;
			}
//			wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnDir: [%s][%s][%s]"),
//					   dirname.wc_str(),
//					   strPath.wc_str(),
//					   strTail.wc_str());

			// use absolute path (with trailing slash) for the POI.
			poi_item * pPoiItem = gpPoiItemTable->addItem(strPath);

			// use relative path (with trailing slash) as key in fd_table.
			fd_item * pFdItem = m_pFdFd->m_table.addItem(m_k,&strTail);

			// populate kth half of fd_item pair.
			pFdItem->setPoiItem(m_k,pPoiItem);

			util_error err;

			//////////////////////////////////////////////////////////////////
			// workaround for problems in wxDir::Open() and wxDirTraverser
			//
			// check if we have access to read/scan this directory.  if not,
			// we remember the error with the item and skip this directory.
			//////////////////////////////////////////////////////////////////

			err = _have_dir_access(dirname);
			if (err.isErr())
			{
//				wxLogTrace(TRACE_FD_DUMP, _T("DirTraverse::OnDir: NO ACCESS [%s][%d]"), strPath.wc_str(), err.getErr());
				pFdItem->setError(err);
				return wxDIR_IGNORE;
			}

			return wxDIR_CONTINUE;
		};

private:
	fd_fd *		m_pFdFd;
	int			m_k;
	wxString	m_strRoot;

	const fd_filter * m_pFdFilter;
	const fd_softmatch * m_pFdSoftmatch;
	bool m_bUsePreviousSoftmatchResult;
	
};

//////////////////////////////////////////////////////////////////

fd_fd::fd_fd(void)
{
	m_table.setFdFd(this);

	m_pPoiItemRoot[0] = NULL;
	m_pPoiItemRoot[1] = NULL;
	m_pBGTH = NULL;
	m_pPoiSaveAsPathname = NULL;
	m_expfmt = FD_EXPORT_FORMAT__UNSET;

	m_ShowHideFlags = gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_HIDE_FLAGS); /*init*/
	if ((m_ShowHideFlags & ~FD_SHOW_HIDE_FLAGS__MASK_BITS) != 0)
	{
		// I changed how we store the various show/hide flags in the registry
		// between 4.0 and 4.1 because I made them per-window rather than global.
		// And I changed the set of bits (no more _SHOW_SHORTCUTS).
		// And I changed the default values.
		//
		// The following is an attempt to inherit the values of the settings
		// from a pre-4.1 installation.
		//
		// I set the default value of _SHOW_HID_FLAGS to -1 to say show-all.
		// This value has more bits than we actually define.  So the first
		// time the user actually toggles a bit, we set the flags to a value
		// covered by the mask bits.  So if there are non-mask bits set, we
		// know the user hasn't done anything yet, so we look to see if there
		// are pre-4.1 values.

		unsigned int old_default = FD_SHOW_HIDE_FLAGS__ERRORS;	// 4.0 was {0,0,0,0,1}

		switch (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_EQUAL)) /*init*/
		{
		default:	break;
		case 0:		old_default &= ~FD_SHOW_HIDE_FLAGS__EQUAL;	break;
		case 1:		old_default |=  FD_SHOW_HIDE_FLAGS__EQUAL;	break;
		}
		
		switch (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_EQUIVALENT)) /*init*/
		{
		default:	break;
		case 0:		old_default &= ~FD_SHOW_HIDE_FLAGS__EQUIVALENT;	break;
		case 1:		old_default |=  FD_SHOW_HIDE_FLAGS__EQUIVALENT;	break;
		}
		
		switch (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_SINGLES)) /*init*/
		{
		default:	break;
		case 0:		old_default &= ~FD_SHOW_HIDE_FLAGS__SINGLES;	break;
		case 1:		old_default |=  FD_SHOW_HIDE_FLAGS__SINGLES;	break;
		}

		switch (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_FOLDERS)) /*init*/
		{
		default:	break;
		case 0:		old_default &= ~FD_SHOW_HIDE_FLAGS__FOLDERS;	break;
		case 1:		old_default |=  FD_SHOW_HIDE_FLAGS__FOLDERS;	break;
		}

		switch (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_ERRORS)) /*init*/
		{
		default:	break;
		case 0:		old_default &= ~FD_SHOW_HIDE_FLAGS__ERRORS;	break;
		case 1:		old_default |=  FD_SHOW_HIDE_FLAGS__ERRORS;	break;
		}

		// Note that _QUICKMATCH settings were not present in 4.0 or 4.1
		// so we don't need to import/inherit them.

//		wxLogTrace(wxTRACE_Messages, _T("fd_fd:ctor(): [flags %x][flags %x]"),
//				   m_ShowHideFlags, old_default);

		// inherit the old defaults and override our new show-all default
		// for the this and future windows.

		gpGlobalProps->setLong(GlobalProps::GPL_FOLDER_SHOW_HIDE_FLAGS, old_default); /*init*/
		m_ShowHideFlags = old_default;
	}
	

}

fd_fd::~fd_fd(void)
{
//	wxLogTrace(TRACE_FD_DUMP, _T("fd_fd[%p]::~fd_fd:"),this);
}

//////////////////////////////////////////////////////////////////

void fd_fd::setFolders(poi_item * pPoi0, poi_item * pPoi1)
{
	wxASSERT_MSG( (!m_pPoiItemRoot[0] && !m_pPoiItemRoot[1]), _T("fd_fd::setFolders: already have folders") );

	m_pPoiItemRoot[0] = pPoi0;
	m_pPoiItemRoot[1] = pPoi1;
}

//////////////////////////////////////////////////////////////////

util_error fd_fd::loadFolders(util_background_thread_helper * pBGTH,
							  bool bUsePreviousSoftmatchResult)
{
	wxASSERT_MSG( (m_pPoiItemRoot[0] && m_pPoiItemRoot[1]), _T("fd_fd::loadFolders: don't have 2 folders") );
//	wxLogTrace(TRACE_FD_DUMP, _T("fd_fd[%p]::loadFolders:"),this);

	UTIL_PERF_RESET_ALL();

	// Make private snap-shot of all (formerly global/singleton) classes
	// so that our thread will not be dependent upon global settings so
	// that we get a consistent scan -- and without locking issues if
	// they raise the Options Dialog while our thread is working, for
	// example.
	//
	//////////////////////////////////////////////////////////////////
	// The goal is that the background thread SHALL NOT reference gpGlobalProps.
	//////////////////////////////////////////////////////////////////

	fd_filter * pFdFilter = new fd_filter();
	fd_softmatch * pFdSoftmatch = new fd_softmatch();
	fd_quickmatch * pFdQuickmatch = new fd_quickmatch();
	rs_ruleset_table * pRsRuleSetTable = new rs_ruleset_table( *gpRsRuleSetTable );

	m_pBGTH = pBGTH;

	m_vec.clear();
	
	// remember the ignore-matchup-case value *AS OF* when we last
	// updated the table.
	m_table.rememberIgnoreMatchupCase(pFdFilter->isIgnoreMatchupCase());

	// treewalk both directories and populate our table.
	// in case this is a reload, minimize flashing and/or
	// heap thrashing, mark all items in current
	// table as "stale" before treewalk.  during line
	// item visit, flag is cleared.  afterwards, we
	// delete the ones that are still stale.

	m_table.markAllStale();

	updateProgressMessage(_T("Scanning First Directory..."));
	UTIL_PERF_START_CLOCK(_T("fd1"));
	util_error err0 = _loadFolder(0, pFdFilter, pFdSoftmatch, bUsePreviousSoftmatchResult);
	UTIL_PERF_STOP_CLOCK(_T("fd1"));

	updateProgressMessage(_T("Scanning Second Directory..."));
	UTIL_PERF_START_CLOCK(_T("fd2"));
	util_error err1 = _loadFolder(1, pFdFilter, pFdSoftmatch, bUsePreviousSoftmatchResult);
	UTIL_PERF_STOP_CLOCK(_T("fd2"));

	m_table.deleteAllStale();

	//////////////////////////////////////////////////////////////////
	// now that we have walked both trees (so we know how big things are)
	// switch to a percentage-based progress bar (rather than unbounded
	// barber pole style).
	updateProgressMessage(_T("Comparing Files..."));
	updateProgress(0, m_table.getItemCount() );

	UTIL_PERF_START_CLOCK(_T("fd3"));
	m_table.computeAllStatus(pFdFilter, pFdQuickmatch, pFdSoftmatch, pRsRuleSetTable, bUsePreviousSoftmatchResult);
	buildVec();
	UTIL_PERF_STOP_CLOCK(_T("fd3"));

	UTIL_PERF_DUMP_ALL(_T("After fd_fd:loadFolders"));

	DELETEP(pFdFilter);
	DELETEP(pFdQuickmatch);
	DELETEP(pFdSoftmatch);
	DELETEP(pRsRuleSetTable);
	m_pBGTH = NULL;

	return (err0.isErr() ? err0 : err1);
}

//////////////////////////////////////////////////////////////////

util_error fd_fd::_loadFolder(int k,
							  const fd_filter * pFdFilter,
							  const fd_softmatch * pFdSoftmatch,
							  bool bUsePreviousSoftmatchResult)
{
	// walk the filesystem for 1 of our root dirs and populate 1/2 of our table.

	if (!m_pPoiItemRoot[k]->isDir())				// complain and fail if not a directory
	{
		return util_error(util_error::UE_NOT_A_FOLDER,
						  wxString::Format( _("Pathname[%d] is not a folder. [%s]"),
											k, m_pPoiItemRoot[k]->getFullPath().wc_str()));
	}

	//////////////////////////////////////////////////////////////////
	// see WORKAROUND note above
	//////////////////////////////////////////////////////////////////

	util_error err = _have_dir_access(m_pPoiItemRoot[k]->getFullPath());
	if (err.isErr())
		return err;

	// divert wxWindows system error log to our buffer before
	// creating wxDir, since it emits error messages rather than
	// returning errors codes.
	
	util_logToString uLog(&err.refExtraInfo());

	wxString strRoot( m_pPoiItemRoot[k]->getFullPath() );

	wxDir dir( strRoot );
	
//	wxLogTrace(TRACE_FD_DUMP, _T("fd_fd::_loadFolder: BeginTraverse [%d][%s]"), k, strRoot.wc_str());

	_fd_fd_loadFolder traverser(this, k, strRoot,
								pFdFilter, pFdSoftmatch, bUsePreviousSoftmatchResult);

	// Traverse the directory but DO NOT follow symlinks.
	// (This flag only really applies to Mac/Linux, I think.)
	// 
	// WXBUG: the return value from dir.Traverse() is bogus.
	dir.Traverse(traverser, wxEmptyString, wxDIR_DEFAULT|wxDIR_NO_FOLLOW);

//	wxLogTrace(TRACE_FD_DUMP, _T("fd_fd::_loadFolder: EndTraverse   [%d][%s]"), k, strRoot.wc_str());

	return err;
}

//////////////////////////////////////////////////////////////////

void fd_fd::clearVec(void)
{
	m_vec.clear();
}

void fd_fd::buildVec(void)
{
	// build our vector.
	//
	// fd_item_table contains a map<relpath, fd_item*> which lets us efficiently
	// insert/match-up partial paths from the left and right folders and
	// automatically have contents sorted.
	//
	// the only problem is that map<>'s have a normal iterator, but don't have
	// a random access operator[] (that works on an index, rather than an
	// associative thing).
	//
	// so, we define a vector<fd_item *> and populated it from the map.  this
	// lets us efficiently respond to drawing requests from ViewFolder/wxListCtrl.
	// it also lets us easily show/hide {equal,peerless} items as necessary
	// (without rewalking the filesystem); it is, in effect, our display list.

	bool bShowEqual      = ((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__EQUAL) != 0);
	bool bShowEquivalent = ((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__EQUIVALENT) != 0);
	bool bShowQuickMatch = ((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__QUICKMATCH) != 0);
	bool bShowSingles    = ((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__SINGLES) != 0);
	bool bShowFolders    = ((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__FOLDERS) != 0);
	bool bShowErrors     = ((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__ERRORS) != 0);
	
	m_vec.clear();
	m_vec.reserve( m_table.getItemCount() ); // reserve worst-case amount to minimize realloc's

	void * vit = NULL;
	fd_item * pFdItem = m_table.beginIter(&vit);
	while (pFdItem)
	{
		fd_item::Status s = pFdItem->getStatus();
		switch (s)
		{
		default:
		case fd_item::FD_ITEM_STATUS_UNKNOWN:
		case fd_item::FD_ITEM_STATUS_MISMATCH:
		case fd_item::FD_ITEM_STATUS_BOTH_NULL:
			m_vec.push_back(pFdItem);	// these should not happen -- but just incase -- include them
			break;

		case fd_item::FD_ITEM_STATUS_ERROR:
			if (bShowErrors) m_vec.push_back(pFdItem);
			break;
			
		case fd_item::FD_ITEM_STATUS_SAME_FILE:
		case fd_item::FD_ITEM_STATUS_IDENTICAL:
			if (bShowEqual) m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_EQUIVALENT:
			if (bShowEquivalent) m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_QUICKMATCH:
			if (bShowQuickMatch) m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_DIFFERENT:
			m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_FOLDERS:
			if (bShowFolders) m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_FOLDER_NULL:
		case fd_item::FD_ITEM_STATUS_NULL_FOLDER:
			if (bShowSingles) m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_FILE_NULL:
		case fd_item::FD_ITEM_STATUS_NULL_FILE:
			if (bShowSingles) m_vec.push_back(pFdItem);
			break;

#if defined(__WXMSW__)
		case fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME:
		case fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ:
			if (bShowEqual) m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ:
			m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_SHORTCUT_NULL:
		case fd_item::FD_ITEM_STATUS_NULL_SHORTCUT:
			if (bShowSingles) m_vec.push_back(pFdItem);
			break;
#endif//__WXMSW__

#if defined(__WXMAC__) || defined(__WXGTK__)
		case fd_item::FD_ITEM_STATUS_SYMLINKS_SAME:
		case fd_item::FD_ITEM_STATUS_SYMLINKS_EQ:
			if (bShowEqual) m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ:
			m_vec.push_back(pFdItem);
			break;

		case fd_item::FD_ITEM_STATUS_SYMLINK_NULL:
		case fd_item::FD_ITEM_STATUS_NULL_SYMLINK:
			if (bShowSingles) m_vec.push_back(pFdItem);
			break;
#endif

		}

		pFdItem = m_table.nextIter(vit);
	}
	m_table.endIter(vit);
}

//////////////////////////////////////////////////////////////////

wxString fd_fd::formatStatsString(void) const
{
	long cErrors = 0;
	long cEqualFiles = 0;
	long cEquivalentFiles = 0;
	long cQuickMatchFiles = 0;
	long cDiffFiles = 0;
	long cPeerlessFiles = 0;
	long cFolderPairs = 0;
	long cPeerlessFolders = 0;
#if defined(__WXMSW__)
	long cEqualShortcuts = 0;
	long cDiffShortcuts = 0;
	long cPeerlessShortcuts = 0;
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	long cEqualSymlinks = 0;
	long cDiffSymlinks = 0;
	long cPeerlessSymlinks = 0;
#endif

	cErrors            += getStats(fd_item::FD_ITEM_STATUS_UNKNOWN);		// shouldn't be needed
	cErrors            += getStats(fd_item::FD_ITEM_STATUS_ERROR);
	cErrors            += getStats(fd_item::FD_ITEM_STATUS_MISMATCH);
	cErrors            += getStats(fd_item::FD_ITEM_STATUS_BOTH_NULL);		// shouldn't be needed
					
	cEqualFiles        += getStats(fd_item::FD_ITEM_STATUS_SAME_FILE);
	cEqualFiles        += getStats(fd_item::FD_ITEM_STATUS_IDENTICAL);
	cEquivalentFiles   += getStats(fd_item::FD_ITEM_STATUS_EQUIVALENT);
	cQuickMatchFiles   += getStats(fd_item::FD_ITEM_STATUS_QUICKMATCH);
	cDiffFiles         += getStats(fd_item::FD_ITEM_STATUS_DIFFERENT);

	cFolderPairs       += getStats(fd_item::FD_ITEM_STATUS_FOLDERS);

	cPeerlessFiles     += getStats(fd_item::FD_ITEM_STATUS_FILE_NULL);
	cPeerlessFiles     += getStats(fd_item::FD_ITEM_STATUS_NULL_FILE);

	cPeerlessFolders   += getStats(fd_item::FD_ITEM_STATUS_FOLDER_NULL);
	cPeerlessFolders   += getStats(fd_item::FD_ITEM_STATUS_NULL_FOLDER);

#if defined(__WXMSW__)
	cPeerlessShortcuts += getStats(fd_item::FD_ITEM_STATUS_SHORTCUT_NULL);
	cPeerlessShortcuts += getStats(fd_item::FD_ITEM_STATUS_NULL_SHORTCUT);
	cEqualShortcuts    += getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME);
	cEqualShortcuts    += getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ);
	cDiffShortcuts     += getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ);
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
	cPeerlessSymlinks += getStats(fd_item::FD_ITEM_STATUS_SYMLINK_NULL);
	cPeerlessSymlinks += getStats(fd_item::FD_ITEM_STATUS_NULL_SYMLINK);
	cEqualSymlinks    += getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_SAME);
	cEqualSymlinks    += getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_EQ);
	cDiffSymlinks     += getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ);
#endif

	wxString msg = wxString::Format( L"Files(%ld+%ldeq+%ldqm %ldneq %ldnp)",
									 cEqualFiles, cEquivalentFiles, cQuickMatchFiles,
									 cDiffFiles, cPeerlessFiles);
	if (cFolderPairs+cPeerlessFolders > 0)
		msg += wxString::Format( L" Folders(%ldp %ldnp)",
								 cFolderPairs, cPeerlessFolders);
#if defined(__WXMSW__)
	if (cEqualShortcuts+cDiffShortcuts+cPeerlessShortcuts > 0)
		msg += wxString::Format( L" Shortcuts(%ldeq %ldneq %ldnp)",
								 cEqualShortcuts, cDiffShortcuts, cPeerlessShortcuts);
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	if (cEqualSymlinks+cDiffSymlinks+cPeerlessSymlinks > 0)
		msg += wxString::Format( L" Symlinks(%ldeq %ldneq %ldnp)",
								 cEqualSymlinks, cDiffSymlinks, cPeerlessSymlinks);
#endif

	if (cErrors > 0)
		msg += wxString::Format( L" E(%ld)", cErrors);

	return msg;
}

//////////////////////////////////////////////////////////////////

void fd_fd::updateProgressMessage(const wxString & strMsg)
{
	if (m_pBGTH)
		m_pBGTH->setProgressMessage(strMsg);
}

void fd_fd::updateProgress(int n, int d)
{
	if (m_pBGTH)
		m_pBGTH->setProgress(n,d);
}

//////////////////////////////////////////////////////////////////

fd_item * fd_fd::findItemByRelativePath(const wxString & strRelativePath, long * pNdxInVec)
{
	// find the fd_item of the given relative pathname.
	// then search for it in the m_vec to get the "display list" index
	// for the listctrl.

	fd_item * pFdItem = m_table.findItem(strRelativePath);
	if (pFdItem)
	{
		long ndx = 0;
		for (TVecIterator it=m_vec.begin(); it!=m_vec.end(); it++,ndx++)
		{
			fd_item * pFdItemIT = *it;
			if (pFdItemIT == pFdItem)
			{
				*pNdxInVec = ndx;
				return pFdItem;
			}
		}
	}

	*pNdxInVec = -1;
	return NULL;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fd_fd::dump(int indent) const
{
	wxLogTrace(TRACE_FD_DUMP, _T("%*cFD_FD: [%p]"),indent,' ',this);
	for (int k=0; k<2; k++)
	{
		wxLogTrace(TRACE_FD_DUMP, _T("%*c[%d]: [%p]:[%s]"),
				   indent+5,' ',k,
				   m_pPoiItemRoot[k],
				   ((m_pPoiItemRoot[k]) ? m_pPoiItemRoot[k]->getFullPath().wc_str() : _T("")));
	}
	m_table.dump(indent+5);

	wxLogTrace(TRACE_FD_DUMP, _T("%*cFD_ITEM_VEC: [nr %ld]"),indent,' ',(long)m_vec.size());

	for (TVecConstIterator it=m_vec.begin(); it!=m_vec.end(); it++)
	{
		fd_item * pFdItem = *it;
		pFdItem->dump(indent+5);
	}
}
#endif

//////////////////////////////////////////////////////////////////

void fd_fd::setShowHideFlags(FD_SHOW_HIDE_FLAGS f)
{
	// WARNING: we only set the flags in this fd_fd.
	// WARNING: these will be used for the next scan/buildvec.
	// WARNING: the caller is responsible for scheduling/queueing
	// WARNING: the scan/buildvec.

	m_ShowHideFlags = f;
}

//////////////////////////////////////////////////////////////////

bool fd_fd::haveChanges(void) const
{
#if 0 && defined(DEBUG)
	print_stats("haveChanges");
#endif

	if (getStats(fd_item::FD_ITEM_STATUS_UNKNOWN) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_ERROR) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_MISMATCH) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_BOTH_NULL) > 0)
		return true;
	
					
//	cEqualFiles        += getStats(fd_item::FD_ITEM_STATUS_SAME_FILE);
//	cEqualFiles        += getStats(fd_item::FD_ITEM_STATUS_IDENTICAL);
//	cEquivalentFiles   += getStats(fd_item::FD_ITEM_STATUS_EQUIVALENT);
//	cQuickMatchFiles   += getStats(fd_item::FD_ITEM_STATUS_QUICKMATCH);

	if (getStats(fd_item::FD_ITEM_STATUS_DIFFERENT) > 0)
		return true;

//	cFolderPairs       += getStats(fd_item::FD_ITEM_STATUS_FOLDERS);

	if (getStats(fd_item::FD_ITEM_STATUS_FILE_NULL) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_NULL_FILE) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_FOLDER_NULL) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_NULL_FOLDER) > 0)
		return true;

#if defined(__WXMSW__)
	if (getStats(fd_item::FD_ITEM_STATUS_SHORTCUT_NULL) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_NULL_SHORTCUT) > 0)
		return true;
	
//	cEqualShortcuts    += getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME);
//	cEqualShortcuts    += getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ);

	if (getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ) > 0)
		return true;
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
	if (getStats(fd_item::FD_ITEM_STATUS_SYMLINK_NULL) > 0)
		return true;

	if (getStats(fd_item::FD_ITEM_STATUS_NULL_SYMLINK) > 0)
		return true;
	
//	cEqualSymlinks    += getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_SAME);
//	cEqualSymlinks    += getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_EQ);

	if (getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ) > 0)
		return true;
#endif

	return false;
}

