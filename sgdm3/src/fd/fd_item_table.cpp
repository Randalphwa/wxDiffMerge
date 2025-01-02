// fd_item_table.cpp
// a table of fd_items -- directory tree in a folder diff window.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>

//////////////////////////////////////////////////////////////////

fd_item_table::fd_item_table(void)
{
	memset(m_stats,0,sizeof(m_stats));
}

fd_item_table::~fd_item_table(void)
{
//	wxLogTrace(TRACE_FD_DUMP, _T("fd_item_table[%p]::~fd_item_table:"),this);

	// map value has pointers rather than references, so we're responsible
	// for deleteing the values before the map gets destroyed..

	for (TMapIterator it = m_map.begin(); (it != m_map.end()); it++)
	{
		fd_item * pFdItem = it->second;
		delete pFdItem;
	}
}

//////////////////////////////////////////////////////////////////

fd_item * fd_item_table::addItem(int kItem, const wxString * pRelativePathname)
{
	// add item to the table.  we preserve the case of the relative pathnames
	// for display purposes in the listctrl.  but fold case (on win32) so that
	// they sort and match up properly.  the case-folded pathname is used as
	// the key to the map.
	//
	// wxBUG: on win32, the case-folding doesn't seem to fold accented chars
	// wxBUG: like A-ring into a-ring.  is this a bug or a locale issue or what?

	fd_item * pFdItem = findItem(*pRelativePathname);
	if (pFdItem)
	{
		pFdItem->setRelativePathname(kItem,pRelativePathname);
		return pFdItem;
	}
	
	if (kItem == 0)
		pFdItem = new fd_item(this,pRelativePathname,NULL);
	else
		pFdItem = new fd_item(this,NULL,pRelativePathname);

	if (m_bIgnoreMatchupCase)
	{
		wxString strLower(*pRelativePathname);
		strLower.MakeLower();
		m_map.insert( TMapValue(strLower,pFdItem) );
	}
	else
	{
		m_map.insert( TMapValue(*pRelativePathname,pFdItem) );
	}

	// pFdItem ctor should set unknown and not have
	// called _setStatus() nor _updateStatus() so this
	// item has not been counted yet in m_stats[].
	// we do not call pFdItem->getStatus() now because
	// it will try to compute it and (if we're in loadFolders())
	// we don't need the answer yet (until both trees have
	// been walked).
	wxASSERT_MSG( (pFdItem->m_status == fd_item::FD_ITEM_STATUS_UNKNOWN), _T("Coding Error") );
	m_stats[ fd_item::FD_ITEM_STATUS_UNKNOWN ]++; // we assume fd_item constructor did not do an _updateStats()

	return pFdItem;
}

fd_item_table::TMapConstIterator fd_item_table::findIter(const wxString & rRelativePathname) const
{
	TMapConstIterator it;

	if (m_bIgnoreMatchupCase)
	{
		wxString strLower(rRelativePathname);
		strLower.MakeLower();
		it = m_map.find(strLower);
	}
	else
	{
		it = m_map.find(rRelativePathname);
	}

	return it;
}

fd_item * fd_item_table::findItem(const wxString & rRelativePathname) const
{
	TMapConstIterator it = findIter(rRelativePathname);

	if (it == m_map.end())
		return NULL;

	return it->second;
}

//////////////////////////////////////////////////////////////////

void fd_item_table::markAllStale(void)
{
	for (TMapConstIterator it=m_map.begin(); (it != m_map.end()); it++)
	{
		fd_item * pFdItem = it->second;
		pFdItem->setStale();
	}
}

void fd_item_table::deleteAllStale(void)
{
#if 0 && defined(DEBUG)
	print_stats(L"deleteAllStale starting");
#endif

	TMapIterator it=m_map.begin();
	while (it != m_map.end())
	{
		fd_item * pFdItem = it->second;
		if (pFdItem->deleteStale())				// if both halves stale, we can delete from our table.
		{
			m_stats[ pFdItem->getStatus() ]--;
			delete pFdItem;

			TMapIterator itNext = it; itNext++;	// iterator class doesn't have '+1' or 'next' operator.
			m_map.erase(it);
			it = itNext;
		}
		else
		{
			it++;
		}
	}

#if 0 && defined(DEBUG)
	print_stats(L"deleteAllStale ending");
#endif

}

//////////////////////////////////////////////////////////////////

void fd_item_table::computeAllStatus(const fd_filter * pFdFilter,
									 const fd_quickmatch * pFdQuickmatch,
									 const fd_softmatch * pFdSoftmatch,
									 const rs_ruleset_table * pRsRuleSetTable,
									 bool bUsePreviousSoftmatchResult) const
{
#if 0 && defined(DEBUG)
	print_stats(L"computeAllStatus starting");
#endif

	// compute status on all items in the table.

	// we assume that m_stats will be updated (via _updateStats())
	// by fd_item as each item changes, so we do not need to
	// rebuild it here.

	unsigned int k = 0;
	for (TMapConstIterator it=m_map.begin(); (it != m_map.end()); it++)
	{
		fd_item * pFdItem = it->second;
		pFdItem->computeStatus(pFdFilter, pFdQuickmatch, pFdSoftmatch, pRsRuleSetTable, bUsePreviousSoftmatchResult);

		m_pFdFd->updateProgress( ++k, (int)m_map.size() );
	}

#if 0 && defined(DEBUG)
	print_stats(L"computeAllStatus ending");
#endif

}

//////////////////////////////////////////////////////////////////

void fd_item_table::_updateStats(fd_item::Status oldValue, fd_item::Status newValue)
{
	if (oldValue == newValue)
		return;

	m_stats[oldValue]--;
	m_stats[newValue]++;
}

//////////////////////////////////////////////////////////////////

fd_item * fd_item_table::beginIter(void ** ppvoid) const
{
	wxASSERT_MSG( ppvoid, _T("Coding Error: fd_item_table::beginIter: ppvoid NULL"));

	TMapConstIterator * pit = new TMapConstIterator; // caller must call endIter()
	*ppvoid = (void *)pit;
	
	*pit = m_map.begin();
	if (*pit == m_map.end())
		return NULL;

	fd_item * pFdItem = (*pit)->second;
	(*pit)++;

	return pFdItem;
}

fd_item * fd_item_table::nextIter(void * pvoid) const
{
	wxASSERT_MSG( pvoid, _T("Coding Error: fd_item_table::nextIter: pvoid NULL"));

	TMapConstIterator * pit = (TMapConstIterator *) pvoid;
	if (*pit == m_map.end())
		return NULL;

	fd_item * pFdItem = (*pit)->second;
	(*pit)++;

	return pFdItem;
}

void fd_item_table::endIter(void * pvoid) const
{
	wxASSERT_MSG( pvoid, _T("Coding Error: fd_item_table::endIter: pvoid NULL"));

	TMapConstIterator * pit = (TMapConstIterator *) pvoid;
	delete pit;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fd_item_table::dump(int indent) const
{
	wxLogTrace(TRACE_FD_DUMP, _T("%*cFD_ITEM_TABLE: [%p] [nr %ld]"),indent,' ',this,(long)m_map.size());
	dump_stats(indent+5);
	int k = 0;
	for (TMapConstIterator it=m_map.begin(); (it != m_map.end()); k++, it++)
	{
		wxString rel = it->first;
		const fd_item * pFdItem = it->second;

		wxLogTrace(TRACE_FD_DUMP, _T("%*c[%d] [key %s]:"), indent,' ', k, rel.wc_str());	// print version of relative path used for key (casefolded on win32)
		pFdItem->dump(indent+5);
	}
}

void fd_item_table::dump_stats(int indent) const
{
	for (int k=0; k<fd_item::__FD_ITEM_STATUS__COUNT__; k++)
		wxLogTrace(TRACE_FD_DUMP, _T("%*cSTATS[%d]: stats[%s] = %ld"),
				   indent,' ',k,
				   fd_item::getStatusText((fd_item::Status)k),
				   m_stats[k]);
}

void fd_item_table::print_stats(const wxString & strMsg) const
{
	wxLogTrace(wxTRACE_Messages,
			   (L"PrintStats:"
				L" (%ld %ld %ld %ld)"
				L" (%ld %ld %ld %ld %ld)"
				L" (%ld)"
				L" (%ld %ld)(%ld %ld)"
#if defined(__WXMSW__)
				L" (%ld %ld %ld %ld %ld)"
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
				L" (%ld %ld %ld %ld %ld)"
#endif
				L" %s"),
			   getStats(fd_item::FD_ITEM_STATUS_UNKNOWN),
			   getStats(fd_item::FD_ITEM_STATUS_ERROR),
			   getStats(fd_item::FD_ITEM_STATUS_MISMATCH),
			   getStats(fd_item::FD_ITEM_STATUS_BOTH_NULL),

			   getStats(fd_item::FD_ITEM_STATUS_SAME_FILE),
			   getStats(fd_item::FD_ITEM_STATUS_IDENTICAL),
			   getStats(fd_item::FD_ITEM_STATUS_EQUIVALENT),
			   getStats(fd_item::FD_ITEM_STATUS_QUICKMATCH),
			   getStats(fd_item::FD_ITEM_STATUS_DIFFERENT),

			   getStats(fd_item::FD_ITEM_STATUS_FOLDERS),

			   getStats(fd_item::FD_ITEM_STATUS_FILE_NULL),
			   getStats(fd_item::FD_ITEM_STATUS_NULL_FILE),
			   getStats(fd_item::FD_ITEM_STATUS_FOLDER_NULL),
			   getStats(fd_item::FD_ITEM_STATUS_NULL_FOLDER),

#if defined(__WXMSW__)
			   getStats(fd_item::FD_ITEM_STATUS_SHORTCUT_NULL),
			   getStats(fd_item::FD_ITEM_STATUS_NULL_SHORTCUT),
			   getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME),
			   getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ),
			   getStats(fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ),
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
			   getStats(fd_item::FD_ITEM_STATUS_SYMLINK_NULL),
			   getStats(fd_item::FD_ITEM_STATUS_NULL_SYMLINK),
			   getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_SAME),
			   getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_EQ),
			   getStats(fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ),
#endif
			   strMsg.wc_str());
}
#endif
