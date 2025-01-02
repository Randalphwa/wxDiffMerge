// fd_item.cpp
// a line-item in a folder diff window representing a pair of
// files/directories within the 2 trees.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

#define IS_TRAILING_CHAR(s,c)	(((s).GetChar((s).Length() - 1)) == (c))

//////////////////////////////////////////////////////////////////

fd_item::fd_item(fd_item_table * pFdItemTable,
				 const wxString * pRelativePathname0, const wxString * pRelativePathname1,
				 poi_item * pPoiItem0, poi_item * pPoiItem1)
	: m_pFdItemTable(pFdItemTable),
	  m_status(FD_ITEM_STATUS_UNKNOWN) // lazy eval.  do not use _setStatus() so we do not notify fd_item_table (see fd_item_table::addItem()).
{
	m_pPoiItem[0] = pPoiItem0;
	m_pPoiItem[1] = pPoiItem1;

	if (pRelativePathname0)		m_relativePathname[0] = *pRelativePathname0;
	if (pRelativePathname1)		m_relativePathname[1] = *pRelativePathname1;
		
	m_bStale[0] = false;
	m_bStale[1] = false;

	m_dtm[0] = wxInvalidDateTime;
	m_dtm[1] = wxInvalidDateTime;

}

fd_item::~fd_item(void)
{
//	wxLogTrace(TRACE_FD_DUMP, _T("fd_item[%p]::~fd_item: [%s][%s]"),
//			   this,
//			   m_relativePathname[0].wc_str(),
//			   m_relativePathname[1].wc_str());
}

//////////////////////////////////////////////////////////////////

void fd_item::setPoiItem(int kItem, poi_item * pPoiItemK)
{
	if (pPoiItemK != m_pPoiItem[kItem])
	{
		m_pPoiItem[kItem] = pPoiItemK;
		_setStatus(FD_ITEM_STATUS_UNKNOWN); // lazy eval
	}

	if (m_pPoiItem[kItem])					// we visited this half of the fd_item during the
		m_bStale[kItem] = false;			// fd_fd treewalk, so we clear the stale flag.
}

//////////////////////////////////////////////////////////////////

void fd_item::setRelativePathname(int kItem, const wxString * pRelativePathname)
{
	m_relativePathname[kItem] = *pRelativePathname;
}

//////////////////////////////////////////////////////////////////

fd_item::Status fd_item::computeStatus(const fd_filter * pFdFilter,
									   const fd_quickmatch * pFdQuickmatch,
									   const fd_softmatch * pFdSoftmatch,
									   const rs_ruleset_table * pRsRuleSetTable,
									   bool bUsePreviousSoftmatchResult)
{
//	wxLogTrace(wxTRACE_Messages,L"computeStatus starting: %d %s %s",
//			   (int)m_status,
//			   ((m_pPoiItem[0]) ? m_pPoiItem[0]->getFullPath().wc_str() : L"(null)"),
//			   ((m_pPoiItem[1]) ? m_pPoiItem[1]->getFullPath().wc_str() : L"(null)"));
			   
	_computeStatus(pFdFilter, pFdQuickmatch, pFdSoftmatch, pRsRuleSetTable, bUsePreviousSoftmatchResult);

//	wxLogTrace(wxTRACE_Messages,L"computeStatus ending: %d %s %s",
//			   (int)m_status,
//			   ((m_pPoiItem[0]) ? m_pPoiItem[0]->getFullPath().wc_str() : L"(null)"),
//			   ((m_pPoiItem[1]) ? m_pPoiItem[1]->getFullPath().wc_str() : L"(null)"));
			   
	return m_status;
}

fd_item::Status fd_item::getStatus(void)
{
	// TODO 2013/08/12 do we need this still?
	// TODO if (m_status == FD_ITEM_STATUS_UNKNOWN)
	// TODO    return computeStatus();

	return m_status;
}

//////////////////////////////////////////////////////////////////

void fd_item::_setStatus(Status newValue)
{
	// this should be the only way we change the value of m_status
	// so that we update our container's stats.

	Status oldValue = m_status;
	m_status = newValue;

	if (oldValue != newValue)
		m_pFdItemTable->_updateStats(oldValue,newValue);

	if ( (m_status==FD_ITEM_STATUS_IDENTICAL)
		 || (m_status==FD_ITEM_STATUS_DIFFERENT)
		 || (m_status==FD_ITEM_STATUS_EQUIVALENT)
		 || (m_status==FD_ITEM_STATUS_QUICKMATCH)
		 || (m_status==FD_ITEM_STATUS_SHORTCUTS_EQ)
		 || (m_status==FD_ITEM_STATUS_SHORTCUTS_NEQ)
		 // TODO 2013/07/26 We don't currently fetch the DTM for symlinks.
		)
	{
		// for _IDENTICAL, _EQUIVALENT, _QUICKMATCH, and _DIFFERENT files m_dtm[] should
		// have been set for each file.  Also for pairs of shortcuts.

		wxASSERT_MSG( (m_dtm[0].IsValid() && m_dtm[1].IsValid()), _T("Coding Error") );

		// for plain files,
		// m_strSoftQuickMatchInfo should have been set by the various compareFile* functions.
	}
	else
	{
		m_dtm[0] = wxInvalidDateTime;
		m_dtm[1] = wxInvalidDateTime;

		m_strSoftQuickMatchInfo.Clear();
	}
}

//////////////////////////////////////////////////////////////////

void fd_item::_computeStatus(const fd_filter * pFdFilter,
							 const fd_quickmatch * pFdQuickmatch,
							 const fd_softmatch * pFdSoftmatch,
							 const rs_ruleset_table * pRsRuleSetTable,
							 bool bUsePreviousSoftmatchResult)
{
	if (!m_pPoiItem[0] && !m_pPoiItem[1])
	{
		_setStatus(FD_ITEM_STATUS_BOTH_NULL);
		return;
	}

	POI_TYPE t0 = poi_item::getPoiType( m_pPoiItem[0] );
	POI_TYPE t1 = poi_item::getPoiType( m_pPoiItem[1] );

	if ((t0 == POI_T_FILE) && (t1 == POI_T_FILE))
	{
		// if both are plain files, actually compare them (if they
		// have changed since the last time we were here).  this may
		// return SAME,IDENTICAL,EQUIVALENT,QUICKMATCH,DIFFERENT,ERROR.
		//
		// in the case of a previous error, we want to try again.
		_computeFileDiff(pFdFilter, pFdQuickmatch, pFdSoftmatch, pRsRuleSetTable, bUsePreviousSoftmatchResult);
		return;
	}

#if defined(__WXMSW__)
	if ((t0 == POI_T_SHORTCUT) && (t1 == POI_T_SHORTCUT))
	{
		// if both are Shortcuts (.LNK), *try* to compare them.
		_computeShortcutDiff();
		return;
	}
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
	// TODO 2013/07/26 For now, I'm only going to consider symlink-vs-symlink
	// TODO            cases.  Later we might want to implicitly dereference
	// TODO            the symlinks and consider symlink-vs-file, symlink-vs-dir,
	// TODO            and etc.
	if ((t0 == POI_T_SYMLINK) && (t1 == POI_T_SYMLINK))
	{
		_computeSymlinkDiff();
		return;
	}
#endif

	if (m_err.isErr())
	{
		// if we had some kind of error on this item (while treewalking
		// or the last time we were here), we should mark it as an error
		// and preserve the error info and not keep beating on it.
		_setStatus(FD_ITEM_STATUS_ERROR);
		return;
	}

	if ((t0 >= POI_T__LIMIT) || (t1 >= POI_T__LIMIT))
	{
		m_err.set(util_error::UE_CANNOT_COMPARE_ITEMS,
				  wxString::Format(_T("[t0 %d][t1=%d]"), (int)t0, (int)t1));
		_setStatus(FD_ITEM_STATUS_ERROR);
		return;
	}

	// I'm using -1's for cases we've already handled.
	Status a01[POI_T__LIMIT][POI_T__LIMIT] =
		{
/*                 NULL                           FILE                      FOLDER                      SHORTCUT                      SYMLINK */
/*NULL*/       {   (Status)-1,                    FD_ITEM_STATUS_NULL_FILE, FD_ITEM_STATUS_NULL_FOLDER, FD_ITEM_STATUS_NULL_SHORTCUT, FD_ITEM_STATUS_NULL_SYMLINK },
/*FILE*/       {   FD_ITEM_STATUS_FILE_NULL,      (Status)-1,               FD_ITEM_STATUS_MISMATCH,    FD_ITEM_STATUS_MISMATCH,      FD_ITEM_STATUS_MISMATCH     },
/*FOLDER*/     {   FD_ITEM_STATUS_FOLDER_NULL,    FD_ITEM_STATUS_MISMATCH,  FD_ITEM_STATUS_FOLDERS,     FD_ITEM_STATUS_MISMATCH,      FD_ITEM_STATUS_MISMATCH     },
/*SHORTCUT*/   {   FD_ITEM_STATUS_SHORTCUT_NULL,  FD_ITEM_STATUS_MISMATCH,  FD_ITEM_STATUS_MISMATCH,    (Status)-1,                   FD_ITEM_STATUS_MISMATCH     },
/*SYMLINK*/    {   FD_ITEM_STATUS_SYMLINK_NULL,   FD_ITEM_STATUS_MISMATCH,  FD_ITEM_STATUS_MISMATCH,    FD_ITEM_STATUS_MISMATCH,      (Status)-1                  },
		};
	
	_setStatus( a01[t0][t1] );

	if (a01[t0][t1] == FD_ITEM_STATUS_MISMATCH)
		m_err.set(util_error::UE_CANNOT_COMPARE_ITEMS,
				  _T("Items have different types."));

	return;
}

//////////////////////////////////////////////////////////////////

void fd_item::_computeFileDiff(const fd_filter * /*pFdFilter*/,
							   const fd_quickmatch * pFdQuickmatch,
							   const fd_softmatch * pFdSoftmatch,
							   const rs_ruleset_table * pRsRuleSetTable,
							   bool bUsePreviousSoftmatchResult)
{
	// compute whether or not the two plain files are equal.

	wxASSERT_MSG( (m_pPoiItem[0] && m_pPoiItem[1]), _T("fd_item::_computeFileDiff: Coding Error.") );

	// if we both point to the same file, they are by definition equal.

	if (m_pPoiItem[0] == m_pPoiItem[1])
	{
		_setStatus(FD_ITEM_STATUS_SAME_FILE);
		return;
	}

	// get the last date-time-modified on both files.
	// if we can't even stat the files, give up now.

	wxDateTime dtm[2];
	for (int k=0; k<2; k++)
	{
		m_err = m_pPoiItem[k]->getDateTimeModified(&dtm[k]);
		if (m_err.isErr())
		{
			_setStatus(FD_ITEM_STATUS_ERROR);
			return;
		}
		wxASSERT_MSG( (dtm[k].IsValid()), _T("Coding Error") );
	}

	// Should the quick-match code decide on the state of this file pair.
	bool bIsQuickMatchCandidate = (pFdQuickmatch
								   && pFdQuickmatch->isQuickMatchEnabled()
								   && pFdQuickmatch->isQuickSuffix(m_pPoiItem[0]->getFullPath()));

	// if we have valid previous date-time-modified values for both
	// files, see if the on-disk dtm values are the same. if so, our
	// previous answer ***MAY*** still be valid; that is, the files haven't
	// changed since the last time we compared them.

	bool bCachedDTMsValid = (m_dtm[0].IsValid()
							 && m_dtm[1].IsValid()
							 && (dtm[0]==m_dtm[0])
							 && (dtm[1]==m_dtm[1]));
	if (bCachedDTMsValid)
	{
		if (m_status == FD_ITEM_STATUS_IDENTICAL)
		{
			// previous run did full exact match.
			// see if we can still use that result.

			if (bIsQuickMatchCandidate)
			{
				// this may look strange, but if the previous run
				// did a full exact match, but now they only want
				// approximations based upon file size, we should
				// throwaway the previous result.
				bCachedDTMsValid = false;
			}
			else
			{
				return;
			}
		}
	}
	
	int result = -1;

	// If quick-match is enabled and this file is a candidate,
	// try a quick-match **BEFORE** we try for an exact-match
	// or soft-match.  User wanted this to say "i'll accept
	// the risk that my 2 multi-gigabyte .mov files are identical
	// if they have the same size, but just don't waste my time
	// reading them."  W0118, B9592.

	if (bIsQuickMatchCandidate)
	{
		m_err = pFdQuickmatch->compareQuick(m_pPoiItem[0], m_pPoiItem[1],
											&result,
											m_strSoftQuickMatchInfo);
		if (m_err.isErr() || (result == -1))
		{
			_setStatus(FD_ITEM_STATUS_ERROR);
			return;
		}
		else
		{
			wxASSERT_MSG( ((result == 3 /*QUICK*/) || (result == 0 /*DIFF*/)), _T("Coding Error") );
			goto done;
		}
	}
	else
	{
		// Quick-match does not apply to this item.
		// Force a re-compute if we have a quick-match
		// result from the previous pass.
		if (m_status == FD_ITEM_STATUS_QUICKMATCH)
		{
			bCachedDTMsValid = false;
		}
	}
	
	// Time for an EXACT-MATCH comparison.

	if (bCachedDTMsValid)
	{
		// if the cached DTMs were valid, we can skip the exact match because
		// we already know they didn't match because we tested for it before
		// that big assert earlier.
	}
	else
	{
		// the following returns:
		//    -1 on error,
		//     0 when different,
		//     1 when equal.
		m_err = m_pPoiItem[0]->compareFileExact(m_pPoiItem[1],&result);
		if (m_err.isErr() || (result == -1))
		{
			_setStatus(FD_ITEM_STATUS_ERROR);
			return;
		}
		if (result == 1)
		{
			m_strSoftQuickMatchInfo = _("Identical using Exact Match");
			goto done;
		}
		else
		{
			wxASSERT_MSG( (result == 0), _T("Coding Error.") );
			// For now we just say it is different.
			// If the optional soft-match is turned on, we may overrule this.
			m_strSoftQuickMatchInfo = _("Different using Exact Match");
		}
	}
	
	wxASSERT_MSG( ((result == -1) || (result == 0)), _T("Coding Error") );

	// Use soft-match if enabled and necessary.
	
	if (bCachedDTMsValid && (bUsePreviousSoftmatchResult)
		&& ((m_status==FD_ITEM_STATUS_EQUIVALENT) || (m_status==FD_ITEM_STATUS_DIFFERENT)))
	{
		return;
	}
		
	if (pFdSoftmatch->isSimpleMode())
	{
		// both files (m_pPoiItem[0] and [1]) should have the same
		// file name, right.  so, we only need to lookup the suffix once.

		if (pFdSoftmatch->isSimpleModeSuffix(m_pPoiItem[0]->getFullPath()))
		{
			// the following returns: -1 on error, 0 when different, 2 when equivalent.
			m_err = pFdSoftmatch->compareFileSimpleMode(m_pPoiItem[0],m_pPoiItem[1],
														&result,
														m_strSoftQuickMatchInfo);
			if (m_err.isErr() || (result == -1))
			{
				_setStatus(FD_ITEM_STATUS_ERROR);
				return;
			}
		}
	}
	else if (pFdSoftmatch->isRulesetMode())
	{
		// both files (m_pPoiItem[0] and [1]) should have the same
		// file name, right.  so, we only need to lookup the suffix once.
		// this will silently substitute the default ruleset if appropriate.

		const rs_ruleset * pRS = pFdSoftmatch->findRuleset(pRsRuleSetTable, m_pPoiItem[0]);
		if (pRS)
		{
			// the following returns: -1 on error, 0 when different, 2 when equivalent.
			// we don't expect to get an -1 errors any more.
			m_err = pFdSoftmatch->compareFileRulesetMode(m_pPoiItem[0],m_pPoiItem[1],
														 pRS,&result,
														 m_strSoftQuickMatchInfo);
			if (m_err.isErr() || (result == -1))
			{
				_setStatus(FD_ITEM_STATUS_ERROR);
				return;
			}
		}
	}

done:
	m_dtm[0] = dtm[0];
	m_dtm[1] = dtm[1];

	switch (result)
	{
	case 3:
		_setStatus(FD_ITEM_STATUS_QUICKMATCH);
		break;

	case 2:
		_setStatus(FD_ITEM_STATUS_EQUIVALENT);
		break;

	case 1:
		_setStatus(FD_ITEM_STATUS_IDENTICAL);
		break;

	case 0:
	default:
		_setStatus(FD_ITEM_STATUS_DIFFERENT);
		break;
	}

	return;
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMAC__) || defined(__WXGTK__)
void fd_item::_computeSymlinkDiff(void)
{
	char buf0[4096];
	char buf1[4096];

	wxASSERT_MSG( (m_pPoiItem[0] && m_pPoiItem[1]),
				  _T("fd_item::_computeSymlinkDiff: Coding Error.") );

	// if we both point to the same symlink,
	// they are by definition equal.

	if (m_pPoiItem[0] == m_pPoiItem[1])
	{
		_setStatus(FD_ITEM_STATUS_SYMLINKS_SAME);
		return;
	}

	m_err = m_pPoiItem[0]->get_symlink_target_raw(buf0, (int)NrElements(buf0));
	if (m_err.isErr())
	{
		_setStatus(FD_ITEM_STATUS_ERROR);
		return;
	}

	m_err = m_pPoiItem[1]->get_symlink_target_raw(buf1, (int)NrElements(buf1));
	if (m_err.isErr())
	{
		_setStatus(FD_ITEM_STATUS_ERROR);
		return;
	}

	// TODO 2013/07/26 Note that we don't need to get the DTMs.
	// TODO            We could, but I don't think it is worth
	// TODO            the bother.
	// 
	// TODO 2013/07/26 For now I'm going to do an exact match
	// TODO            on the symlink targets and not try to
	// TODO            interpret them (either absolutely or
	// TODO            relatively).  That is, I'm only reporting
	// TODO            on the equality of the target string
	// TODO            *AND NOT* on the equality of the target
	// TODO            filesystem object.

	if (strcmp(buf0, buf1) == 0)
	{
		_setStatus(FD_ITEM_STATUS_SYMLINKS_EQ);
		return;
	}
	else
	{
		_setStatus(FD_ITEM_STATUS_SYMLINKS_NEQ);
		return;
	}
}
#endif

//////////////////////////////////////////////////////////////////

void fd_item::setStale(void)
{
	// set stale flags before re-scanning directories

	m_bStale[0] = true;
	m_bStale[1] = true;

	m_err.clear();
}

bool fd_item::deleteStale(void)
{
	// after re-scanning directories, the ones still
	// marked stale are no longer present.  so we can
	// delete our handle to them.  if both are deleted,
	// our caller can delete us.

	if (m_bStale[0] && m_pPoiItem[0])
		m_pPoiItem[0] = NULL;
	
	if (m_bStale[1] && m_pPoiItem[1])
		m_pPoiItem[1] = NULL;

	bool bResult = (m_bStale[0] && m_bStale[1]);

	m_bStale[0] = false;
	m_bStale[1] = false;

	return bResult;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fd_item::dump(int indent) const
{
	wxLogTrace(TRACE_FD_DUMP, _T("%*cFD_ITEM: [%p][rel0 %s][rel1 %s][status %s][err %d]"),
			   indent,' ',this,
			   m_relativePathname[0].wc_str(),
			   m_relativePathname[1].wc_str(),
			   getStatusText(m_status),
			   m_err.getErr());
	for (int k=0; k<2; k++)
	{
		wxLogTrace(TRACE_FD_DUMP, _T("%*c[%d]: [%p]:[%s]"), indent,' ',k,m_pPoiItem[k],((m_pPoiItem[k])?m_pPoiItem[k]->getFullPath().wc_str():_T("")));
	}
}
#endif

//////////////////////////////////////////////////////////////////

bool fd_item::getSoftQuickMatchSummaryMessage(wxString & strMsg) const
{
	wxString strLine;

	switch (m_status)
	{
	default:
		return false;

	case fd_item::FD_ITEM_STATUS_IDENTICAL:
	case fd_item::FD_ITEM_STATUS_EQUIVALENT:
	case fd_item::FD_ITEM_STATUS_DIFFERENT:
		strLine.Printf(_("Left Pathname: %s\n"),m_pPoiItem[0]->getFullPath().wc_str());
		strMsg += strLine;

		strLine.Printf(_("        Timestamp: %s\n"),m_dtm[0].Format().wc_str());
		strMsg += strLine;

		// TODO add file size in bytes

		strLine.Printf(_("Right Pathname: %s\n"),m_pPoiItem[1]->getFullPath().wc_str());
		strMsg += strLine;

		strLine.Printf(_("        Timestamp: %s\n"),m_dtm[1].Format().wc_str());
		strMsg += strLine;

		// TODO add file size in bytes

		strLine.Printf(_("Status: %s\n"),getStatusText(m_status));
		strMsg += strLine;

		strLine.Printf(_("Match Result: %s\n"),m_strSoftQuickMatchInfo.wc_str());
		strMsg += strLine;

		return true;
	}
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
void fd_item::_computeShortcutDiff(void)
{
	// compute whether or not the 2 .lnk files are equal.

	wxASSERT_MSG( (m_pPoiItem[0] && m_pPoiItem[1]),
				  _T("fd_item::_computeShortcutDiff: Coding Error.") );

	// if we both point to the same file, they are by definition equal.

	if (m_pPoiItem[0] == m_pPoiItem[1])
	{
		_setStatus(FD_ITEM_STATUS_SHORTCUTS_SAME);
		return;
	}

	// get the last date-time-modified on both files.
	// if we can't even stat the files, give up now.

	wxDateTime dtm[2];
	for (int k=0; k<2; k++)
	{
		m_err = m_pPoiItem[k]->getDateTimeModified(&dtm[k]);
		if (m_err.isErr())
		{
			_setStatus(FD_ITEM_STATUS_ERROR);
			return;
		}
		wxASSERT_MSG( (dtm[k].IsValid()), _T("Coding Error") );
	}
		
	// if we have valid previous date-time-modified values for both
	// files, see if the on-disk dtm values are the same. if so, our
	// previous answer ***MAY*** still be valid; that is, the files haven't
	// changed since the last time we compared them.  if we had an exact
	// match it is still valid.

	if (m_dtm[0].IsValid()  &&  m_dtm[1].IsValid())
	{
		if (dtm[0]==m_dtm[0]  &&  dtm[1]==m_dtm[1])
		{
			_setStatus(m_status);	// not sure this is needed
			return;
		}
	}

	// treat the SHORTCUT .LNK as plain files and do a binary read
	// and compare them using EXACT-MATCH (don't bother waking up
	// COM and pulling properties just yet).

	int result = -1;

	// the following returns: -1 on error, 0 when different, 1 when equal.
	m_err = m_pPoiItem[0]->compareFileExact(m_pPoiItem[1],&result);
	if (m_err.isErr() || (result == -1))
	{
		_setStatus(FD_ITEM_STATUS_ERROR);
		return;
	}

#if 0 && defined(DEBUG)	// just using this to test log trace
	{
		util_shell_lnk * pLnk_0 = NULL;
		util_shell_lnk * pLnk_1 = NULL;
		(void)m_pPoiItem[0]->get_lnk_target(&pLnk_0);
		(void)m_pPoiItem[1]->get_lnk_target(&pLnk_1);
		wxLogTrace(TRACE_LNK, _T("Lnk pair [equal %d] [%s][%s]"),
				   (result==1),
				   m_pPoiItem[0]->getFullPath(),
				   m_pPoiItem[1]->getFullPath());
		delete pLnk_0;
		delete pLnk_1;
	}
#endif

	m_dtm[0] = dtm[0];
	m_dtm[1] = dtm[1];

	if (result == 1)
		_setStatus(FD_ITEM_STATUS_SHORTCUTS_EQ);
	else
		_setStatus(FD_ITEM_STATUS_SHORTCUTS_NEQ);

	return;
}
#endif//__WXMSW__

//////////////////////////////////////////////////////////////////

wxString fd_item::_make_clone_target_pathname(int k_src, int k_dest)
{
	fd_fd * pFdFd = m_pFdItemTable->getFdFd();
	poi_item * pPoiRoot_dest = pFdFd->getRootPoi(k_dest);

	// create full path of the destination version
	// from the destination root directory and the
	// relative pathname in the source directory.

	wxString strDest(pPoiRoot_dest->getFullPath());
#if defined(__WXMSW__)
	if ( ! IS_TRAILING_CHAR( strDest, L'\\' ) )
		strDest += L'\\';
#else
	if ( ! IS_TRAILING_CHAR( strDest, L'/' ) )
		strDest += L'/';
#endif
	strDest += m_relativePathname[k_src];

//	wxLogTrace(wxTRACE_Messages, _T("CloneTarget[%d,%d]: %s --> %s"),
//			   k_src, k_dest,
//			   m_pPoiItem[k_src]->getFullPath().wc_str(),
//			   strDest.wc_str());

	// For a copy of a peerless item, this fd_item will not
	// have a peer in the destination (duh) so we need to
	// create the map<s,p> key for it and the POI for it.
	// 
	// The value used for "s" in map<s,p> have trailing "@"
	// or "^" when they are SYMLINK or SHORTCUT.  We want to
	// preserve that for the key, but not in the actual
	// pathname on disk.

#if defined(__WXMAC__) || defined(__WXGTK__)
	if (m_pPoiItem[k_src]->getPoiType() == POI_T_SYMLINK)
	{
		wxASSERT_MSG( (IS_TRAILING_CHAR( strDest, L'@' )), _T("Coding Error") );
		strDest.Truncate( strDest.Length() - 1 );
//		wxLogTrace(wxTRACE_Messages,
//				   _T("CloneTarget: trimmed [dest %s]"),
//				   strDest);
	}
#endif
#if defined(__WXMSW__)
	if (m_pPoiItem[k_src]->getPoiType() == POI_T_SHORTCUT)
	{
		wxASSERT_MSG( (IS_TRAILING_CHAR( strDest, L'^' )), _T("Coding Error") );
		strDest.Truncate( strDest.Length() - 1 );
//		wxLogTrace(wxTRACE_Messages,
//				   _T("CloneTarget: trimmed [dest %s]"),
//				   strDest);
	}
#endif

	return strDest;
}

/**
 * Iterate thru the fd_item_table to simulate recursion
 * on the source directory.
 */
util_error fd_item::_clone_iterate(int k_src, int k_dest)
{
	util_error err;

	// we want to recurse on the current conents of
	// the source directory in "this".  Normally we
	// would start a treewalk on the filesystem.
	// But we already have an ordered map in fd_item_table.

	wxString strPrefix(m_relativePathname[k_src]);
	if (m_pFdItemTable->m_bIgnoreMatchupCase)
		strPrefix.MakeLower();

//	wxLogTrace(wxTRACE_Messages, _T("Clone: Diving on prefix: %s"), strPrefix.wc_str());

	fd_item_table::TMapConstIterator it = m_pFdItemTable->findIter(strPrefix);
	fd_item * pFdItemFound = it->second;
	wxASSERT_MSG( (pFdItemFound == this), _T("Coding Error"));
	it++;	// skip over the row for 'this'.

	while (it != m_pFdItemTable->m_map.end())
	{
		const wxString & str_j = it->first;
		if (str_j.StartsWith(strPrefix))
		{
//			wxLogTrace(wxTRACE_Messages, _T("CloneDive: %s"), str_j.wc_str());
			fd_item * pFdItem_j = it->second;
			wxASSERT_MSG( (pFdItem_j), _T("Coding Error"));
			wxASSERT_MSG( (pFdItem_j->m_pPoiItem[k_dest]==NULL), _T("Coding Error"));
			
//			pFdItem_j->m_relativePathname[k_dest] = str_j; // case may be wrong so use other side
			pFdItem_j->m_relativePathname[k_dest] = pFdItem_j->m_relativePathname[k_src];

			wxString strDest_j = pFdItem_j->_make_clone_target_pathname(k_src, k_dest);
			pFdItem_j->m_pPoiItem[k_dest] = gpPoiItemTable->addItem(strDest_j);

			switch (pFdItem_j->m_pPoiItem[k_src]->getPoiType())
			{
			default:
				err.set(util_error::UE_UNSUPPORTED, _T("Coding Error."));
				return err;

#if defined(__WXMAC__) || defined(__WXGTK__)
			case POI_T_SYMLINK:
				err = pFdItem_j->_my_clone_or_overwrite_symlink(k_src, k_dest, false);
				if (err.isErr())
					return err;
				break;
#endif

#if defined(__WXMSW__)
			case POI_T_SHORTCUT:
#endif
			case POI_T_FILE:
				err = pFdItem_j->_my_clone_or_overwrite_file(k_src, k_dest, false);
				if (err.isErr())
					return err;
				break;

			case POI_T_DIR:
				if (!wxFileName::Mkdir(strDest_j))
				{
					err.set(util_error::UE_CANNOT_MKDIR, strDest_j);
					return err;
				}
				pFdItem_j->_setStatus(FD_ITEM_STATUS_FOLDERS);
				break;
			}

			it++;
		}
		else
		{
			break;
		}
	}

	return err;
}

util_error fd_item::clone_item_on_disk_in_other_folder(bool bRecursive)
{
	util_error err;
	int k_src, k_dest;

	if ((m_pPoiItem[0]!=NULL) == (m_pPoiItem[1]!=NULL)) // both or none set
	{
		err.set(util_error::UE_UNSUPPORTED, _T("Coding Error."));
		return err;
	}

	if (m_pPoiItem[0])
	{
		k_src = 0;
		k_dest = 1;
	}
	else
	{
		k_src = 1;
		k_dest = 0;
	}

	// If they clicked on a random item deep inside a peerless
	// subtree, we need to populate the parent directories of
	// this item.  (If the item itself is a directory, we go
	// ahead and create it here.)

	wxString strRelativeDest;
	wxString strDest( m_pFdItemTable->getFdFd()->getRootPoi(k_dest)->getFullPath() );
//	wxLogTrace(wxTRACE_Messages,_T("Clone: Root: %s"),strDest.wc_str());

#if defined(__WXMSW__)
	wxStringTokenizer strTok(m_relativePathname[k_src], _T("\\"), wxTOKEN_RET_DELIMS);
#else
	wxStringTokenizer strTok(m_relativePathname[k_src], _T("/"), wxTOKEN_RET_DELIMS);
#endif
	while (strTok.HasMoreTokens())
	{
		wxString str_j = strTok.GetNextToken();
		strRelativeDest += str_j;
		strDest += str_j;

//		wxLogTrace(wxTRACE_Messages,_T("Clone: Prestep: %s"),strDest.wc_str());
		if (
#if defined(__WXMSW__)
			IS_TRAILING_CHAR( strDest, L'\\' )
#else
			IS_TRAILING_CHAR( strDest, L'/' )
#endif
			)
		{
			// To lookup the intermediate fd_item[j] items from the
			// fd_item_table we rely on the fact that we appended a
			// trailing slash to all folder rows (which is included
			// in the map<s,p>).
			fd_item * pFdItem_j = m_pFdItemTable->findItem(strRelativeDest);

			wxASSERT_MSG( (pFdItem_j), _T("Coding Error"));
			wxASSERT_MSG( (pFdItem_j->m_pPoiItem[k_src]), _T("Coding Error"));
			wxASSERT_MSG( (pFdItem_j->m_pPoiItem[k_src]->getPoiType()==POI_T_DIR), _T("Coding Error"));
			
			if (!pFdItem_j->m_pPoiItem[k_dest])	// destination not present on this row
			{
//				wxLogTrace(wxTRACE_Messages, _T("Clone: mkdir: %s"), strDest.wc_str());
				if (!wxFileName::Mkdir(strDest))
				{
					err.set(util_error::UE_CANNOT_MKDIR, strDest);
					return err;
				}
				pFdItem_j->m_relativePathname[k_dest] = strRelativeDest;
				pFdItem_j->m_pPoiItem[k_dest] = gpPoiItemTable->addItem(strDest);
				pFdItem_j->_setStatus(FD_ITEM_STATUS_FOLDERS);
			}
		}
	}
	
	if (!m_pPoiItem[k_dest])
	{
//		wxLogTrace(wxTRACE_Messages,
//				   _T("Clone: Post loop: computed [dest %s][relative %s]"),
//				   strDest.wc_str(), strRelativeDest.wc_str());

		// for the map<s,p> key, use the same value as observed
		// on the other side (with any trailing "@", "^", or "/"
		// hinting).
		m_relativePathname[k_dest] = strRelativeDest;

		// disregard the computed strDest now and
		// let the common routine compute the full/absolute
		// pathname of the destination so that we normalize
		// how we handle the for-display-purposes-only trailing
		// characters.

		strDest = _make_clone_target_pathname(k_src, k_dest);
		m_pPoiItem[k_dest] = gpPoiItemTable->addItem(strDest);
	}

	// actually clone the item.

	switch (m_pPoiItem[k_src]->getPoiType())
	{
	default:
		err.set(util_error::UE_UNSUPPORTED, _T("Coding Error."));
		return err;

#if defined(__WXMAC__) || defined(__WXGTK__)
	case POI_T_SYMLINK:
		err = _my_clone_or_overwrite_symlink(k_src, k_dest, false);
		return err;
#endif

#if defined(__WXMSW__)
	case POI_T_SHORTCUT:
#endif
	case POI_T_FILE:
		err = _my_clone_or_overwrite_file(k_src, k_dest, false);
		return err;
		
	case POI_T_DIR:
		// parent loop already created the directory for the current row.
		if (bRecursive)
			err = _clone_iterate(k_src, k_dest);
		return err;
	}

	// WARNING: we created various files/directories in the
	// WARNING: filesystem.  we updated the various fd_items.
	// WARNING: Our caller needs to rebuild the display list
	// WARNING: (buildvec) before the user will see the updated
	// WARNING: rows.

//	return err;
}

#if defined(__WXMAC__) || defined(__WXGTK__)
/**
 * Clone or overwrite a symlink.
 */
util_error fd_item::_my_clone_or_overwrite_symlink(int k_src, int k_dest, bool bOverwrite)
{
	util_error err;
	char buf[4096];

	err = m_pPoiItem[k_src]->get_symlink_target_raw(buf, (int)NrElements(buf));
	if (err.isErr())
		return err;

	wxString strDest(m_pPoiItem[k_dest]->getFullPath());
	if (bOverwrite)
		::wxRemoveFile(strDest);

	const char * pszDest = strDest.c_str();
	int result = symlink(buf, pszDest);
	if (result == -1)
	{
		err.set(util_error::UE_CANNOT_CREATE_SYMLINK, strDest);
		return err;
	}

	_setStatus(FD_ITEM_STATUS_SYMLINKS_EQ);

	return err;
}
#endif

/**
 * Clone or overwrite a file (or shortcut).
 * We assume that the destination POI has already been set in this fd_item.
 */
util_error fd_item::_my_clone_or_overwrite_file(int k_src, int k_dest, bool bOverwrite)
{
	util_error err;

//	wxLogTrace(wxTRACE_Messages, _T("CloneOrOverwrite: %s ==> %s"),
//			   m_pPoiItem[k_src]->getFullPath().wc_str(),
//			   m_pPoiItem[k_dest]->getFullPath().wc_str());

	if (!wxCopyFile(m_pPoiItem[k_src]->getFullPath(),
					m_pPoiItem[k_dest]->getFullPath(),
					bOverwrite))
	{
		err.set(util_error::UE_CANNOT_WRITE_FILE, m_pPoiItem[k_dest]->getFullPath());
		return err;
	}

	// compute both DTMs (even though we only changed the destination)
	// because the initial load doesn't bother setting DTMs for peerless
	// items.
	err = m_pPoiItem[k_src]->getDateTimeModified(&m_dtm[k_src]);
	if (err.isErr())
		return err;
	err = m_pPoiItem[k_dest]->getDateTimeModified(&m_dtm[k_dest]);
	if (err.isErr())
		return err;

	switch (m_pPoiItem[k_src]->getPoiType())
	{
	case POI_T_FILE:
		_setStatus(FD_ITEM_STATUS_IDENTICAL);
		break;

#if defined(__WXMSW__)
	case POI_T_SHORTCUT:
		_setStatus(FD_ITEM_STATUS_SHORTCUTS_EQ);
		break;
#endif

	case POI_T_DIR:
	default:
		err.set(util_error::UE_UNSUPPORTED, _T("Coding Error."));
		break;
	}

	return err;
}

util_error fd_item::overwrite_item_on_disk(bool bReplaceLeft)
{
	util_error err;
	int k_src, k_dest;

//	wxLogTrace(wxTRACE_Messages, _T("Overwrite: %s"),
//			   ((bReplaceLeft) ? _T("Right->Left") : _T("Left->Right")));

	if (!m_pPoiItem[0] || !m_pPoiItem[1]
		|| (m_pPoiItem[0]->getPoiType() != m_pPoiItem[1]->getPoiType()))
	{
		err.set(util_error::UE_UNSUPPORTED, _T("Coding Error."));
		return err;
	}

	if (bReplaceLeft)
	{
		k_src = 1;
		k_dest = 0;
	}
	else
	{
		k_src = 0;
		k_dest = 1;
	}

	switch (m_pPoiItem[0]->getPoiType())
	{
	case POI_T_DIR:
	default:
		err.set(util_error::UE_UNSUPPORTED, _T("Coding Error."));
		return err;

#if defined(__WXMAC__) || defined(__WXGTK__)
	case POI_T_SYMLINK:
		err = _my_clone_or_overwrite_symlink(k_src, k_dest, true);
		return err;
#endif

#if defined(__WXMSW__)
	case POI_T_SHORTCUT:
#endif
	case POI_T_FILE:
		err = _my_clone_or_overwrite_file(k_src, k_dest, true);
		return err;
	}

	// WARNING: we just overwrote a file in the filesystem and
	// WARNING: updated fd_item.  the caller needs to rebuild
	// WARNING: the display list (buildvec) but they shouldn't
	// WARNING: need a full reload.

//	return err;
}

