// fl_fl.cpp -- a file layout
//
// note: the layout (the formatting into lines & runs) happens
// note: immediately (in response the changes in the piecetables).
// note: line-number,column coordinate assignment is deferred until format()
// note: is called -- generally, immediately prior to painting or
// note: running the diff-engine.
// 
// part of the reason for the postponing line-number,col assignment
// is that multiple independent changes can happen (think
// auto-merge) before a paint comes in.
//
// note: there will be exactly ONE layout for a piecetable.
// note: all windows/views that reference a particular document
// note: will all share the same layout.
// 
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fl.h>

//////////////////////////////////////////////////////////////////

static void s_cb_frag_change(void * pThis, const util_cbl_arg & arg)
{
	fl_fl * pFlFl			= (fl_fl *)pThis;
	const fim_frag * pFrag	= (const fim_frag *)arg.m_p;
	fr_op fop				= (fr_op)arg.m_l;

	pFlFl->_cb_frag_change(pFrag,fop);
}

//////////////////////////////////////////////////////////////////
// static callbacks to let fl_fl know when global props that we
// are interested in change.

#if 0
static void _cb_long(void * pThis, const util_cbl_arg & arg)
{
	GlobalProps::EnumGPL id = (GlobalProps::EnumGPL)arg.m_l;

	fl_fl * pFlFl = (fl_fl *)pThis;
	pFlFl->_gp_cb_long(id);
}
#endif

#if 0
static void _cb_string(void * pThis, const util_cbl_arg & arg)
{
	GlobalProps::EnumGPS id = (GlobalProps::EnumGPS)arg.m_l;

	fl_fl * pFlFl = (fl_fl *)pThis;
	pFlFl->_gp_cb_string(id);
}
#endif

//////////////////////////////////////////////////////////////////

fl_fl::fl_fl(fim_ptable * pPTable)
	: m_pPTable(pPTable),
	  m_listRun(NULL,NULL),
	  m_pLineHead(NULL), m_pLineTail(NULL),
	  m_totalLineNrs(0), m_totalCols(0),
	  m_invUnion(0)
{
	wxASSERT_MSG( (m_pPTable), _T("Coding Error") );

	// NOTE: because we are created/owned by the piecetable
	// NOTE: (we are the single/shared layout for the document)
	// NOTE: we hold a PTable pointer -- but we ***DO NOT***
	// NOTE: hold a reference.  the piecetable will delete us
	// NOTE: when it gets deleted.

	// sign up for notifications when the document changes

	m_pPTable->addChangeCB(s_cb_frag_change,this);
	
	// populate layout based upon contents of document.

	_do_initial_layout();
}

fl_fl::~fl_fl(void)
{
#if 1
#ifdef DEBUG
	// make sure all slots have been released
	for (TVec_SlotsIterator it=m_vecSlots.begin(); (it < m_vecSlots.end()); it++)
	{
		de_de * pDeDe = (*it);
		wxASSERT_MSG( (pDeDe==NULL), _T("Coding Error: slot not released before fl_fl deleted.") );
	}
#endif
#endif

	m_pPTable->delChangeCB(s_cb_frag_change,this);

	DELETE_LIST(fl_line,m_pLineHead);
}

//////////////////////////////////////////////////////////////////

fl_run * fl_fl::_get_first_run_in_frag(const fim_frag * pFrag)
{
	TMap_FragToRunListIterator it = m_mapFrag.find(pFrag);

	wxASSERT_MSG( (it!=m_mapFrag.end()), _T("Coding Error") );

	return (it->second).getHead();
}

fl_run * fl_fl::_get_last_run_in_frag(const fim_frag * pFrag)
{
	TMap_FragToRunListIterator it = m_mapFrag.find(pFrag);

	wxASSERT_MSG( (it!=m_mapFrag.end()), _T("Coding Error") );

	return (it->second).getTail();
}

//////////////////////////////////////////////////////////////////

void fl_fl::_delete_line(fl_line * pLine)
{
	// actually delete a pLine from the line list

	if (!pLine)
		return;

	// when we delete this line, the line-number value of the next
	// line should change.  we just invalidate the coords of
	// the next line -- we don't actually compute the new values
	// for it (and all of the subsequent lines in the file).
	// since we're in the middle of a change we may have stuff
	// in an inconsistent state -- and we may be deleting more
	// than one line -- so we just mark it invalid and go on.
	// after we have completely processed this piecetable change,
	// we'll force all the coordinates to be updated.

	if (pLine->m_next)
		_invalidateLineNrs(pLine->m_next);

	if ((m_pLineHead == pLine) && (m_pLineTail == pLine))
	{
		m_pLineHead = NULL;
		m_pLineTail = NULL;
	}
	else if (m_pLineHead == pLine)
	{
		m_pLineHead = pLine->m_next;
		m_pLineHead->m_prev = NULL;
	}
	else if (m_pLineTail == pLine)
	{
		m_pLineTail = pLine->m_prev;
		m_pLineTail->m_next = NULL;
	}
	else
	{
		fl_line * pLineNext = pLine->m_next;
		fl_line * pLinePrev = pLine->m_prev;

		pLinePrev->m_next = pLineNext;
		pLineNext->m_prev = pLinePrev;
	}

	// tell any watchers that this line has been deleted.
	// this is considered a "low-level" broadcast to inform
	// of a physical change in our line list -- so that
	// watchers can make similar change if necessary.  we
	// *DO NOT* include any FL_INV_ status in this message.
	// we assume that a "high-level" broadcast will be
	// made by our caller when it has completely processed
	// the FOP.
	//
	// WARNING: we leave the next/prev pointers in the line
	// WARNING: so that the caller can tell where the line
	// WARNING: was.  BUT THIS LINE HAS ALREADY BEEN REMOVED
	// WARNING: FROM THE LIST.

	m_cblChange.callAll( util_cbl_arg(pLine, FL_MOD_DEL_LINE) );

	pLine->m_next = NULL;
	pLine->m_prev = NULL;
	
	delete pLine;
}

fl_line * fl_fl::_append_line(fl_line * pLineNew)
{
	// actuall append a pLine to the line list

	wxASSERT_MSG( (pLineNew && pLineNew->isUnlinked()), _T("Coding Error") );

	_invalidateLineNrs(pLineNew);		// new line, mark line-number invalid, will trickle down

	pLineNew->m_next = NULL;
	pLineNew->m_prev = m_pLineTail;

	if (m_pLineTail)
		m_pLineTail->m_next = pLineNew;

	m_pLineTail = pLineNew;

	if (!m_pLineHead)
		m_pLineHead = pLineNew;

	// tell any watchers that a new line has been inserted.
	// this is considered a "low-level" broadcast to inform
	// of a physical change in our line list -- so that
	// watchers can make similar change if necessary.  we
	// *DO NOT* include any FL_INV_ status in this message.
	// we assume that a "high-level" broadcast will be
	// made by our caller when it has completely processed
	// the FOP.

	m_cblChange.callAll( util_cbl_arg(pLineNew, FL_MOD_INS_LINE) );

	return pLineNew;
}

fl_line * fl_fl::_insert_line_after(fl_line * pLineNew, fl_line * pLineExisting)
{
	// insert pLineNew after pLineExisting in our line list.
	// if pLineExisting is null, just append.

	if (!pLineExisting)
		return _append_line(pLineNew);
	
	wxASSERT_MSG( (pLineNew && pLineNew->isUnlinked()), _T("Coding Error") );

	_invalidateLineNrs(pLineNew);		// new line, mark line-number invalid, will trickle down

	fl_line * pLineNext = pLineExisting->m_next;

	pLineNew->m_prev = pLineExisting;
	pLineExisting->m_next = pLineNew;

	pLineNew->m_next = pLineNext;
	if (pLineNext)
		pLineNext->m_prev = pLineNew;
	else
		m_pLineTail = pLineNew;

	// tell any watchers that a new line has been inserted.
	// this is considered a "low-level" broadcast to inform
	// of a physical change in our line list -- so that
	// watchers can make similar change if necessary.  we
	// *DO NOT* include any FL_INV_ status in this message.
	// we assume that a "high-level" broadcast will be
	// made by our caller when it has completely processed
	// the FOP.

	m_cblChange.callAll( util_cbl_arg(pLineNew, FL_MOD_INS_LINE) );

	return pLineNew;
}

//////////////////////////////////////////////////////////////////

fl_line * fl_fl::_fixup_lines_after_insert(fl_run_list_endpoints & ep)
{
	// fix up line assignment for a sequence of newly added runs
	// and any run immediately following it that might have been
	// affected (by EOL's in the new sequence).
	//
	// return the first line still in the document that we affected
	// (where line-number,col are invalid).

	fl_line * pLineToReturn = NULL;

	// get the runs & lines around the insertion (either may be null).

	fl_run * pRunBefore     = ep.getHead()->getPrev();
	fl_run * pRunAfter      = ep.getTail()->getNext();
	fl_line * pLineBefore   = ((pRunBefore) ? pRunBefore->getLine() : NULL);
	fl_line * pLineAfter    = ((pRunAfter ) ? pRunAfter->getLine()  : NULL);

	// allow either (but not both) to be null [since they can append, prepend, or insert in the middle of the document]
	wxASSERT_MSG( (pRunBefore || pRunAfter),	_T("Coding Error: run before and after cannot both be null.") );
	// verify line if run is present
	wxASSERT_MSG( (!pRunBefore || pLineBefore),	_T("Coding Error: line not set.") );
	wxASSERT_MSG( (!pRunAfter  || pLineAfter ),	_T("Coding Error: line not set.") );
	// for an insertion in the middle, the 2 runs should be on the same line, unless the before is an EOL.
	wxASSERT_MSG( (   !pRunBefore												// prepend
				   || !pRunAfter												// append
				   || (pLineBefore==pLineAfter)									// middle and same line
				   || (   (pLineBefore->getNext()==pLineAfter)					// middle on different lines and
					   && (   (pRunBefore->isLF())								//    prev is LF (for LF or CRLF)
						   || (pRunBefore->isCR() && !pRunAfter->isLF())))),	//    prev is mac-style CR
				  _T("Coding Error"));

	// determine the line containing of the left edge of the insertion.

	fl_run * pRun           = ep.getHead();
	fl_line * pLine         = NULL;

	if (pRunBefore)
	{
		// if insert was after a LF or after a mac-style CR, put on next line.
		// otherwise, begin in the middle of the current line.

		if (pRunBefore->isLF() || (pRunBefore->isCR() && !pRun->isLF()))
		{
			pLine = ( (pLineAfter) ? pLineAfter : _append_line( new fl_line(this,pRun) ));
			pLine->setFirstRunOnLine(pRun);
		}
		else
		{
			pLine = pLineBefore;
		}
	}
	else
	{
		// nothing to the left of the new text, so we're inserting
		// before text on right -- and are implicitly the new start
		// of the line.

		pLine = pLineAfter;
		pLine->setFirstRunOnLine(pRun);
	}

	_invalidateCols(pLine);
	pLine->updateEditOpCounter();
	pLineToReturn = pLine;

	// now start placing with pRun on pLine

	bool bInsertLine = false;
	for (/*pRun,pLine*/; (pRun != pRunAfter); pRun=pRun->getNext())
	{
		if (bInsertLine)
		{
			pLine = _insert_line_after( new fl_line(this,pRun), pLine );
			pLine->updateEditOpCounter();
			_invalidateCols(pLine);
		}
		
		pRun->setLine(pLine);

		// if this run is an EOL, the next run should start on a new line -- but we defer
		// actually creating the new line until we actually use it.

		bInsertLine = (pRun->isLF() || (pRun->isCR() && pRun->getNext() && !pRun->getNext()->isLF()));
	}

	// if we encountered line breaks in the new runs, we need to
	// move the existing runs on the current line to the last line
	// we inserted.

	if (pRunAfter)
	{
		if (bInsertLine)		// insertion ended cleanly with an EOL, so
		{
			pLine = _insert_line_after( new fl_line(this,pRunAfter), pLine );
			pLine->updateEditOpCounter();
			_invalidateCols(pLine);
		}

		if (pLine != pLineAfter)
		{
			for (pRun=pRunAfter; (pRun && pRun->getLine()==pLineAfter); pRun=pRun->getNext())
			{
				pRun->setLine(pLine);
			}
		}
	}

	return pLineToReturn;
}

fl_line * fl_fl::_fixup_lines_after_delete(fl_run_list_endpoints & ep)
{
	// fix up line assignments after a sequence of runs was deleted.
	// we need to:
	// [] delete the lines that were wholly contained within the deleted region.
	// [] maybe update line assignments for runs immediately after deleted region.
	//
	// we assume that the sub-run list in "ep" has not yet actually
	// been removed from our main run list.
	//
	// return the first line still in the document that we affected
	// (where line-number,col are invalid).

	// get the runs & lines around the deletion (either or both may be null).

	fl_run * pRunBefore     = ep.getHead()->getPrev();
	fl_run * pRunAfter      = ep.getTail()->getNext();
	fl_line * pLineBefore   = ((pRunBefore) ? pRunBefore->getLine() : NULL);
	fl_line * pLineAfter    = ((pRunAfter ) ? pRunAfter->getLine()  : NULL);

	// if no surrounding runs document will be empty after delete.

	if (!pLineBefore && !pLineAfter)
	{
//		wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_fixup_lines_after_delete: [%p...%p] --> empty doc"),
//				   ep.getHead(), ep.getTail());

		while (m_pLineHead)
			_delete_line(m_pLineHead);

		return NULL;
	}

	// if nothing after deletion, we can do a simple r-truncate.

	if (!pLineAfter)
	{
//		wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_fixup_lines_after_delete: [%p...%p] r-truncate [%p...%p]"),
//				   ep.getHead(), ep.getTail(), m_pLineHead,m_pLineTail);

		while (m_pLineTail != pLineBefore)
			_delete_line(m_pLineTail);

		pLineBefore->updateEditOpCounter();
		_invalidateCols(pLineBefore);

		return pLineBefore;
	}

	// if nothing before deletion, we can do a simple l-truncate.
	// set first run on the line-after (incase the old first run
	// was inside deleted region).

	if (!pLineBefore)
	{
//		wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_fixup_lines_after_delete: [%p...%p] l-truncate [%p...%p]"),
//				   ep.getHead(), ep.getTail(), m_pLineHead,m_pLineTail);

		while (m_pLineHead != pLineAfter)
			_delete_line(m_pLineHead);

		m_pLineHead->setFirstRunOnLine(pRunAfter);
		m_pLineHead->updateEditOpCounter();
		_invalidateCols(m_pLineHead);
		
		return m_pLineHead;
	}

	// if before and after currently on same line, then deletion must be
	// contained within and we don't need to do anything.

	if (pLineBefore == pLineAfter)
	{
//		wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_fixup_lines_after_delete: [%p...%p] intraline [%p...%p] in [%p...%p]"),
//				   ep.getHead(), ep.getTail(), pLineBefore,pLineAfter, m_pLineHead,m_pLineTail);

		pLineBefore->updateEditOpCounter();
		_invalidateCols(pLineBefore);

		return pLineBefore;
	}

	// the delete must be in middle of document content and must begin, end,
	// and/or span a line boundary.  and it may reference first/last line
	// in the document.)

	// delete all lines between the before-line and the after-line.

	if (pLineBefore->m_next != pLineAfter)
	{
		fl_line * pLine = pLineBefore->m_next;
		while (pLine != pLineAfter)
		{
			fl_line * pLineNext = pLine->m_next;
			_delete_line(pLine);
			pLine = pLineNext;
		}

		wxASSERT_MSG( (pLineBefore->m_next==pLineAfter), _T("Coding Error") );
	}

	// now figure out how to join content before and after.

	// if before content ends in LF or mac-style CR, leave
	// content after on next line.  set the first run in case
	// first run was inside deleted region.
	
	if (pRunBefore->isLF() || (pRunBefore->isCR() && !pRunAfter->isLF()))
	{
//		wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_fixup_lines_after_delete: [%p...%p] maintaining EOL [%p...%p] in [%p...%p]"),
//				   ep.getHead(), ep.getTail(), pLineBefore,pLineAfter, m_pLineHead,m_pLineTail);
		
		pLineAfter->setFirstRunOnLine(pRunAfter);
		pLineAfter->updateEditOpCounter();
		_invalidateCols(pLineAfter);
		
		return pLineAfter;
	}

	// there's no EOL at end of content before, so there's no need
	// to keep the after content on a different line.  delete the
	// line and re-associate all runs on that line with the line before.

//	wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_fixup_lines_after_delete: [%p...%p] joining line [%p...%p] in [%p...%p]"),
//			   ep.getHead(), ep.getTail(), pLineBefore,pLineAfter, m_pLineHead,m_pLineTail);

	_delete_line(pLineAfter);
	for (fl_run * pRun=pRunAfter; (pRun && pRun->getLine()==pLineAfter); pRun=pRun->getNext())
		pRun->setLine(pLineBefore);

	pLineBefore->updateEditOpCounter();
	_invalidateCols(pLineBefore);

	return pLineBefore;
}

//////////////////////////////////////////////////////////////////

void fl_fl::_map_insert(const fim_frag * pFrag, fl_run_list_endpoints & ep)
{
//	wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_map_insert: associating [%p...%p] with frag [%p]"),
//			   ep.getHead(),ep.getTail(),pFrag);

	TMap_FragToRunListIterator it = m_mapFrag.find(pFrag);
	if (it == m_mapFrag.end())
		m_mapFrag.insert( TMap_FragToRunListValue(pFrag,ep) );
	else
		it->second = ep;
}

void fl_fl::_map_delete(const fim_frag * pFrag)
{
	TMap_FragToRunListIterator it = m_mapFrag.find(pFrag);
	if (it != m_mapFrag.end())
		m_mapFrag.erase(it);
}

//////////////////////////////////////////////////////////////////

void fl_fl::_do_initial_layout(void)
{
	// build run & line lists and frag map from current contents of piecetable.

	for (const fim_frag * pFrag = m_pPTable->getFirstFrag(); (pFrag); pFrag=pFrag->getNext())
	{
		fl_run_list runList(pFrag);								// build new/owned list of runs to represent frag's content
		fl_run_list_endpoints ep(runList);						// remember end points of list for the frag map
		m_listRun.appendList(runList);							// insert new/owned list into our master list (runList now empty)
		_map_insert(pFrag,ep);									// associate frag with endpoints
	}

	// now that we have cut up content into simplified spans, assign them to display lines.

	fl_run * pRun    = m_listRun.getHead();
	if (pRun)
	{
		fl_line * pLine  = _append_line( new fl_line(this,pRun) );

		bool bInsertLine = false;
		for (/*pRun,pLine*/; (pRun); pRun=pRun->getNext())
		{
			if (bInsertLine)
				pLine = _insert_line_after( new fl_line(this,pRun), pLine );
		
			pRun->setLine(pLine);

			bInsertLine = (pRun->isLF() || (pRun->isCR() && pRun->getNext() && !pRun->getNext()->isLF()));
		}
	}
	
	// tell any watchers that document changed beginning with
	// the first line and don't bother trying shortcuts.

	_invalidateFull();

	m_cblChange.callAll( util_cbl_arg(m_pLineHead, FL_INV_FULL) );
}

//////////////////////////////////////////////////////////////////

void fl_fl::_cb_frag_change(const fim_frag * pFrag, fr_op fop)
{
	// a change has been made in the document
//	wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl:_cb_frag_change: [fl_fl %p][ptable %p][frag %p][fop %d]"), this,m_pPTable,pFrag,fop);

	switch (fop)
	{
	default:
	case FOP_INVALID:
		wxLogTrace(TRACE_FLFL_DUMP, _T("FOP: invalid"));
		return;

	case FOP_INSERT_INITIAL:	// inserting first frag into an empty document
		// this might happen if ptable and layout get setup before the document
		// is loaded.  [as opposed to creating the ptable, loading the document
		// and THEN associating a layout with it.]
		wxASSERT_MSG( (m_listRun.getHead()==NULL), _T("coding error") );
		wxASSERT_MSG( (m_pLineHead==NULL), _T("coding error") );
		wxASSERT_MSG( (m_mapFrag.size()==0), _T("coding error") );
		_do_initial_layout();
		return;

	case FOP_INSERT_BEFORE:		// a new fragment with new text has been inserted
		_fop_insert_before(pFrag);
		return;
		
	case FOP_INSERT_AFTER:		// a new fragment with new text has been inserted
		_fop_insert_after(pFrag);
		return;

	case FOP_SPLIT:				// a new fragment created by splitting off tail of previous fragment
		_fop_split(pFrag);
		return;

	case FOP_JOIN_BEFORE:		// fragment absorbed onto end of previous fragment
		_fop_join_before(pFrag);
		return;

	case FOP_JOIN_AFTER:
		_fop_join_after(pFrag);
		return;

	case FOP_DELETE_SELF:		// fragment and content deleted
		_fop_delete_self(pFrag);
		return;

	case FOP_SET_PROP:
		_fop_set_prop(pFrag);
		return;
	}
}

//////////////////////////////////////////////////////////////////

void fl_fl::_fop_insert_before(const fim_frag * pFrag)
{
	// pFrag was inserted before an existing frag.

	const fim_frag * pFragExisting = pFrag->getNext();
	wxASSERT_MSG( (pFragExisting), _T("Coding Error") );

	// hang on to the runs on both sides of the insertion point.

	fl_run * pRunExisting = _get_first_run_in_frag(pFragExisting);
	//fl_run * pRunExistingPrev = pRunExisting->getPrev();

	// build run list for this fragment and insert it into
	// the middle of master run list.

	fl_run_list runList(pFrag);								// build new/owned list of runs to represent frag's content
	fl_run_list_endpoints ep(runList);						// remember end points of list for the frag map
	m_listRun.insertListBeforeRun(runList,pRunExisting);	// insert new/owned list into master list (runList now empty)
	_map_insert(pFrag,ep);									// associate frag with endpoints

	// set display lines for the newly added runs and
	// adjust display lines for the rest of the current
	// line at (to the right of) the insertion point.

	fl_line * pLineFirstAffected = _fixup_lines_after_insert(ep);

	// tell any watchers that document changed somehow.
	// they should scan line list starting with the given line
	// and repair line-number,col values and repaint.

	m_cblChange.callAll( util_cbl_arg(pLineFirstAffected,pLineFirstAffected->getInvalidate()) );
}

//////////////////////////////////////////////////////////////////

void fl_fl::_fop_insert_after(const fim_frag * pFrag)
{
	// pFrag was inserted after an existing frag

	const fim_frag * pFragExisting = pFrag->getPrev();
	wxASSERT_MSG( (pFragExisting), _T("Coding Error") );

	// hang on to the runs on both sides of the insertion point.

	fl_run * pRunExisting = _get_last_run_in_frag(pFragExisting);
	//fl_run * pRunExistingNext = pRunExisting->getNext();

	// build run list for this fragment and insert it into
	// the middle of master run list.

	fl_run_list runList(pFrag);								// build new/owned list of runs to represent frag's content
	fl_run_list_endpoints ep(runList);						// remember end points of list for the frag map
	m_listRun.insertListAfterRun(runList,pRunExisting);		// insert new/owned list into master list (runList now empty)
	_map_insert(pFrag,ep);									// associate frag with endpoints

	// set display lines for the newly added runs and
	// adjust display lines for the rest of the current
	// line at (to the right of) the insertion point.

	fl_line * pLineFirstAffected = _fixup_lines_after_insert(ep);

	// tell any watchers that document changed somehow.
	// they should scan line list starting with the given line
	// and repair line-number,col values and repaint.

	m_cblChange.callAll( util_cbl_arg(pLineFirstAffected,pLineFirstAffected->getInvalidate()) );
}
	
//////////////////////////////////////////////////////////////////

void fl_fl::_fop_split(const fim_frag * pFrag)
{
	// pFrag was created by splitting the previous fragment and
	// giving it the tail portion of the original content.
	//
	// there is no new content here.  it was just divided between
	// these 2 fragments.
	//
	// note that both the existing fragment and the given fragment
	// have already been updated.
	//
	// we need to:
	// [] update frag map for the existing frag,
	// [] add a frag map entry for the new frag,
	// [] maybe split a run (if one spans the break point),
	// [] update (frag,offset) in the runs in the tail.

	const fim_frag * pFragExisting = pFrag->getPrev();
	wxASSERT_MSG( (pFragExisting), _T("Coding Error") );

	// get the frag map entry -- the first/last runs that refer to the existing frag.

	TMap_FragToRunListIterator it = m_mapFrag.find(pFragExisting);
	wxASSERT_MSG( (it!=m_mapFrag.end()), _T("Coding Error") );
	fl_run_list_endpoints ep(it->second);

	// find the split point.

	fim_length lenHead = pFragExisting->getFragLength();
	//fim_length lenTail = pFrag->getFragLength();
	
	fl_run * pRun = ep.getHead();
	wxASSERT_MSG( (pRun->getFrag()==pFragExisting), _T("Coding Error: frag map bogus.") );
	wxASSERT_MSG( (pRun->getFragOffset()==0), _T("Coding Error: first run in a frag should have offset 0.") );
	while ( (pRun->getFrag()==pFragExisting) && (pRun->getFragOffset()+pRun->getLength() < lenHead) )
		pRun = pRun->getNext();
	wxASSERT_MSG( (pRun->getFrag()==pFragExisting), _T("Coding Error: invalid split") );

	// since we are splitting a run, it must be on a single line.
	// mark the column values invalid on that line.  (even though
	// there isn't any new content, we need to update the column
	// values in the runs on the rest of the line.)

	fl_line * pLineFirstAffected = pRun->getLine();
	_invalidateCols(pLineFirstAffected);

	// split immediately after pRun or somewhere in the middle of pRun.
	// build a new set of end-points for each frag.
	
	fl_run_list_endpoints epPart1(ep.getHead(),pRun);
	fl_run_list_endpoints epPart2;
	if (pRun->getFragOffset()+pRun->getLength() == lenHead)		// found split at break between runs (how convienent)
	{
		wxASSERT_MSG( (pRun != ep.getTail()), _T("Coding Error: invalid split") );
		epPart2.setList(pRun->getNext(),ep.getTail());
	}
	else														// found split in middle of a run
	{
		wxASSERT_MSG( (pRun->getFragOffset() < lenHead), _T("Coding Error: missed split") );
		fl_run * pRun2 = m_listRun.splitRun(pRun,(lenHead-pRun->getFragOffset()));
		epPart2.setList(pRun2, ((ep.getTail()==pRun) ? pRun2 : ep.getTail()));
	}
	
	// re-associate runs in part2 with the new frag rather than the existing frag
	// and adjust the offset to be relative to this new frag.
	// compute the implied offset of first run after joint (this is
	// the offset+length of the last run before the joint).

	fr_offset offset = epPart1.getTail()->getFragEndOffset();
	fl_run * pRun2Stop = epPart2.getTail()->getNext();
	for (fl_run * pRun2=epPart2.getHead(); (pRun2 != pRun2Stop); pRun2=pRun2->getNext())
	{
		pRun2->setFrag(pFrag);
		wxASSERT_MSG( (((signed)pRun2->getFragOffset() - (signed)offset) >= 0), _T("Coding Error: offset wrong in split") );
		pRun2->setFragOffset( pRun2->getFragOffset() - offset );
	}

	// update frag maps

	_map_insert(pFragExisting,epPart1);
	_map_insert(pFrag,epPart2);

	// tell any watchers that document changed somehow.
	// they should scan line list starting with the given line
	// and repair line-number,col values and repaint.

	m_cblChange.callAll( util_cbl_arg(pLineFirstAffected,pLineFirstAffected->getInvalidate()) );
}

//////////////////////////////////////////////////////////////////

void fl_fl::_fop_join_before(const fim_frag * pFrag)
{
	// the content in pFrag was absorbed (coalesced) onto the end
	// of the previous fragment.  update our runs accordingly.
	// 
	// there is no new content here.  it was just combined from
	// 2 fragments.
	//
	// note that both the existing fragment and the given fragment
	// have already been updated -- in our case, pFrag should be
	// empty -- so we can't reference anything within it.
	//
	// we need to:
	// [] delete frag map entry for given frag
	// [] update frag map for existing frag to include runs formerly in given frag
	// [] update (frag,offset) in runs that referenced given frag
	// 
	// we don't bother coalescing runs around joint (undo of splitRun() in _fop_split())
	// since we don't coalesce, we don't need to invalidate column values of the runs.

	// find the runs associated with the given frag

	TMap_FragToRunListIterator itGiven = m_mapFrag.find(pFrag);
	wxASSERT_MSG( (itGiven!=m_mapFrag.end()), _T("Coding Error") );
	fl_run_list_endpoints epGiven(itGiven->second);
	
	// use run list to find the frag that the given frag was joined with.
	// use that to find the runs associated with it.

	fl_run * pRunOtherTail = epGiven.getHead()->getPrev();
	wxASSERT_MSG( (pRunOtherTail), _T("Coding Error: invalid join_before") );
	const fim_frag * pFragOther = pRunOtherTail->getFrag();
	TMap_FragToRunListIterator itOther = m_mapFrag.find(pFragOther);
	wxASSERT_MSG( (itOther!=m_mapFrag.end()), _T("Coding Error") );
	fl_run_list_endpoints epOther(itOther->second);

	// re-associate runs in the given frag with the other frag.
	// compute the implied offset of first run after joint (this is
	// the offset+length of the last run before the joint).

	fr_offset offset = epOther.getTail()->getFragEndOffset();
	fl_run * pRunEnd = epGiven.getTail()->getNext();
	for (fl_run * pRun=epGiven.getHead(); (pRun != pRunEnd); pRun=pRun->getNext())
	{
		pRun->setFrag(pFragOther);
		pRun->setFragOffset(offset + pRun->getFragOffset());
	}

	// update frag maps

	fl_run_list_endpoints epNew(epOther.getHead(), epGiven.getTail());
	_map_insert(pFragOther,epNew);
	_map_delete(pFrag);

	// since we didn't coalesce, we don't need to send a change notification.
}

//////////////////////////////////////////////////////////////////

void fl_fl::_fop_join_after(const fim_frag * pFrag)
{
	// the content in pFrag was absorbed (coalesced) onto the beginning
	// of the next fragment.  update our runs accordingly.
	// 
	// there is no new content here.  it was just combined from
	// 2 fragments.
	//
	// note that both the existing fragment and the given fragment
	// have already been updated -- in our case, pFrag should be
	// empty -- so we can't reference anything within it.
	//
	// we need to:
	// [] delete frag map entry for given frag
	// [] update frag map for existing frag to include runs formerly in given frag
	// [] update (frag,offset) in runs that referenced given frag
	// [] update (offset) in runs in the other frag (since this was a prepend)
	// 
	// we don't bother coalescing runs around joint (undo of splitRun() in _fop_split())
	// since we don't coalesce, we don't need to invalidate column values of the runs.

	// find the runs associated with the given frag

	TMap_FragToRunListIterator itGiven = m_mapFrag.find(pFrag);
	wxASSERT_MSG( (itGiven!=m_mapFrag.end()), _T("Coding Error") );
	fl_run_list_endpoints epGiven(itGiven->second);
	
	// use run list to find the frag that the given frag was joined with.
	// use that to find the runs associated with it.

	fl_run * pRunOtherHead = epGiven.getTail()->getNext();
	wxASSERT_MSG( (pRunOtherHead), _T("Coding Error: invalid join_after") );
	const fim_frag * pFragOther = pRunOtherHead->getFrag();
	TMap_FragToRunListIterator itOther = m_mapFrag.find(pFragOther);
	wxASSERT_MSG( (itOther!=m_mapFrag.end()), _T("Coding Error") );
	fl_run_list_endpoints epOther(itOther->second);

	// re-associate runs in the given frag with the other frag.
	// offsets don't change since we are prepending.

	fl_run * pRunEnd = epGiven.getTail()->getNext();
	for (fl_run * pRun=epGiven.getHead(); (pRun != pRunEnd); pRun=pRun->getNext())
		pRun->setFrag(pFragOther);

	// update offsets in runs in the other frag
	// compute the implied offset of first run after joint (this is
	// the offset+length of the last run before the joint).

	fr_offset offset  = epGiven.getTail()->getFragEndOffset();
	fl_run * pRunEnd2 = epOther.getTail()->getNext();
	for (fl_run * pRun2=epOther.getHead(); (pRun2 != pRunEnd2); pRun2=pRun2->getNext())
		pRun2->setFragOffset(offset + pRun2->getFragOffset());

	// update frag maps

	fl_run_list_endpoints epNew(epGiven.getHead(), epOther.getTail());
	_map_insert(pFragOther,epNew);
	_map_delete(pFrag);

	// since we didn't coalesce, we don't need to send a change notification.
}

//////////////////////////////////////////////////////////////////

void fl_fl::_fop_delete_self(const fim_frag * pFrag)
{
	// pFrag and its content was deleted from the document.
	// update our runs accordingly.
	//
	// note that pFrag has already been removed from the fragment
	// list, so there's not much we can do it it.  the content
	// length may still be intact.
	//
	// we need to:
	// [] delete frag map entry for given frag
	// [] delete the runs associated with this frag
	// [] update line list if any EOL's were deleted.

	// find the runs associated with the given frag

	TMap_FragToRunListIterator itGiven = m_mapFrag.find(pFrag);
	wxASSERT_MSG( (itGiven!=m_mapFrag.end()), _T("Coding Error") );
	fl_run_list_endpoints epGiven(itGiven->second);

	// reassign display lines to reflect deleted content

	fl_line * pLineFirstAffected = _fixup_lines_after_delete(epGiven);

	// remove frag from our frag map
	
	_map_delete(pFrag);

	// unlink the runs from our main run list and delete them

	fl_run_list * prl = m_listRun.extractSubList(epGiven);
	delete prl;

	// tell any watchers that document changed somehow.
	// they should scan line list starting with the given line
	// and repair line-number,col values and repaint.

	m_cblChange.callAll( util_cbl_arg(pLineFirstAffected,
									  ((pLineFirstAffected) ? pLineFirstAffected->getInvalidate() : FL_INV_FULL)) );
}

//////////////////////////////////////////////////////////////////

void fl_fl::_fop_set_prop(const fim_frag * pFrag)
{
	// properties on pFrag were changed.  updated runs accordingly.
	// there is no new content here.

	// find the runs associated with the given frag

	TMap_FragToRunListIterator itGiven = m_mapFrag.find(pFrag);
	wxASSERT_MSG( (itGiven!=m_mapFrag.end()), _T("Coding Error") );
	fl_run_list_endpoints epGiven(itGiven->second);

	// apply new prop to runs

	fr_prop prop = pFrag->getFragProp();
//	wxLogTrace(TRACE_FLFL_DUMP, _T("fl_fl::_fop_set_prop: [prop %d] on [%p...%p]"), prop, epGiven.getHead(),epGiven.getTail());
	fl_run * pRunEnd = epGiven.getTail()->getNext();
	for (fl_run * pRun=epGiven.getHead(); (pRun != pRunEnd); pRun=pRun->getNext())
	{
		pRun->setFragProp(prop);
		_invalidateProp(pRun->getLine()); // line needs repainting
	}

	// tell any watchers that document changed somehow.
	// they should scan line list starting with the given line
	// and repaint.

	fl_line * pLineFirstAffected = epGiven.getHead()->getLine();
	m_cblChange.callAll( util_cbl_arg(pLineFirstAffected,pLineFirstAffected->getInvalidate()) );
}

//////////////////////////////////////////////////////////////////

#if 0
void fl_fl::_gp_cb_long(GlobalProps::EnumGPL id)
{
	// one of the LONG global props has changed.

//	switch (id)
//	{
//	default:
//		return;
//	}
}
#endif

#if 0
void fl_fl::_gp_cb_string(GlobalProps::EnumGPS /*id*/)
{
	// one of the STRING global props has changed.

//	switch (id)
//	{
//	default:
//		return;
//	}
}
#endif

//////////////////////////////////////////////////////////////////

void fl_fl::format(void)
{
	// compute line-number,column values for each line and run in the document.
	// 
	// we have a per-line invalidate flag to tell us whether the
	// values on a line need to be recomputed.
	//
	// we also have m_invUnion that is a union over the per-line
	// invalidate flags.  we use this as a quick check to avoid
	// the linear search in foramt().
	//
	// for example, inserting a single 'z' into a doc will change the
	// column values for some of the runs (fl_run) on the line (at the
	// insertion point and those to the right of it).  so the line will
	// be marked with FL_INV_COL.  this will be added to the union.
	//
	// inserting an new line (CRLF or LF or CR) will cause the line-number
	// values for the newly inserted line ***and*** the line-number values
	// of all subsequent lines in the file to be invalid.  the first
	// such line will be marked with FL_INV_LINENR.  this will be added
	// to the union.
	//
	// changing the property on a run will generally cause it to change
	// color or cause an icon to appear beside the line in the left
	// margin.  we set FL_INV_PROP on the line (and m_invUnion).
	// generally this does not invalidate line-number,column coordinates.
	//
	// note that there may be more than one type of change -- doing an
	// auto merge, for example, will touch stuff all over a document and
	// then send one paint event.  so the value in m_invUnion is a
	// hint as to what we need to do, but we need to look at the
	// individual lines to see what/where/etc.
	//
	// if FL_INV_FULL is set, then we bypass any shortcuts and do
	// everything.  this is used when the doc is first loaded, for example.

	if (m_invUnion == 0)					// nothing to do
		return;

	if (m_invUnion == FL_INV_PROP)			// only props have changed, paint will redraw
	{
		m_invUnion = 0;
		return;
	}

	// NOTE 12/12/05 -- we no longer need column positions for runs
	// NOTE (since the drawing code in ViewFilePanel.cpp uses GetTextExtents()
	// NOTE rather than assuming fixed-width fonts actually are fixed-width.
	// NOTE
	// NOTE we also no longer need the tabstops setting (since the DE-based
	// NOTE drawing code dynamically uses the current tabstop setting (from
	// NOTE the toolbar) to place text).
	// NOTE
	// NOTE also, since the tabstop setting is a per-view toolbar setting, we
	// NOTE cannot have the tabstop in this (layout) layer.
	// NOTE
	// NOTE so, the only thing we still need them for is computing the width
	// NOTE of the longest line in the document so that we can calibrate the
	// NOTE	horizontal scrollbar.
	// NOTE
	// NOTE	so, for now, we assume tabs are 8 spaces and compute the layout
	// NOTE info -- this will let the scrollbar work regardless of the toolbar
	// NOTE setting.
	
	long tabStop = 8;

	bool bFull     = FL_INV_IS_SET(m_invUnion,FL_INV_FULL);
	bool bForceCol = bFull;
	
	// scan the entire doc, line-by-line, and do any fixups.
	// 
	// TODO if FL_INV_FULL is set, we must do the entire doc, but
	// TODO if FL_INV_FULL is not set, then we may be able to
	// TODO just touch up the dirty lines -- the set of lines
	// TODO broadcasted in m_cblChange.callAll() events since the
	// TODO last format() -- but this may not be worth the effort
	// TODO since a single line-number change will cascade to the rest of
	// TODO the document -- but it might let us avoid a linear
	// TODO search....  but since we need to compute the widest
	// TODO line, we might still need to scan the doc.

	int colMax   = 0;	// nr columns of widest line
	int lineNr   = 0;	// line number of current line in loop

	for (fl_line * pLine=m_pLineHead; pLine; pLine=pLine->getNext())
	{
		bool bNeedCol = bForceCol || FL_INV_IS_SET(pLine->getInvalidate(),FL_INV_COL);	// columns are on line-by-line basis, unless FL_INV_FULL is set.

		// we always set line-number values -- it's faster than computing
		// whether we need to and carrying along the current line-number value
		// when we need to switch on.

		pLine->setLineNr(lineNr);
		lineNr++;								// always advance by 1 since it is line number.

		if (bNeedCol)							// adjust column values of each run on the line
		{
			int col = 0;						// column number of left edge of run

			fl_run * pFirstRun = pLine->getFirstRunOnLine();
			for (fl_run * pRun=pFirstRun; (pRun && (pRun->getLine()==pLine)); pRun=pRun->getNext())
			{
				int nrCols = ((pRun->isTAB()) ? (tabStop - (col %tabStop)) : pRun->getLength());	// number of columns displayed
//				pRun->setCol(col,nrCols);					// set column parameters
				col += nrCols;
			}

			colMax = MyMax(colMax,col);	// remember widest line for calibrating hscroller
		}

		pLine->validate();
	}

	// remember the total width & height of the document so scroll bars can be calibrated.

	m_totalLineNrs= lineNr;
	m_totalCols   = MyMax(m_totalCols,colMax);

	// mark formatting of entire document up-to-date.

	m_invUnion    = 0;
}

//////////////////////////////////////////////////////////////////

fl_slot fl_fl::claimSlot(de_de * pDeDe)
{
	int size = (int)m_vecSlots.size();
	int k;

	for (k=0; (k < size); k++)
		if (m_vecSlots[k]==NULL)
		{
			m_vecSlots[k] = pDeDe;
			return k;
		}
	
	m_vecSlots.push_back(pDeDe);
	return k;
}

void fl_fl::releaseSlot(de_de * pDeDe, fl_slot slot)
{
	MY_ASSERT( ((m_vecSlots.size() >= (size_t)slot) && (m_vecSlots[slot]==pDeDe)) );

	m_vecSlots[slot] = NULL;
}

//////////////////////////////////////////////////////////////////

bool fl_fl::getLineAndColumnOfFragAndOffset(const fim_frag * pFrag, fr_offset offset,
											int cColTabWidth,
											const fl_line ** ppFlLine, int * pCol) const
{
	TMap_FragToRunListConstIterator it = m_mapFrag.find(pFrag);
	if (it == m_mapFrag.end())
		return false;
	
	fl_run_list_endpoints ep(it->second);
	if (ep.isEmpty())
		return false;

	// a frag contains a list of the runs covering its content.
	// this may span more than one line.  search run list (a
	// subset of the whole document) for the run containing the
	// desired frag offset.

	const fl_run * pRunEnd = ep.getTail()->getNext();
	const fl_run * pRun;
	for (pRun=ep.getHead(); pRun!=pRunEnd; pRun=pRun->getNext())
	{
		wxASSERT_MSG( (pRun->getFrag()==pFrag), _T("Coding Error") );

		if (offset < pRun->getFragEndOffset())
		{
			wxASSERT_MSG( (offset >= pRun->getFragOffset()), _T("Coding Error"));
			goto FoundRun;
		}
	}

	// we prefer the left-edge, but if we're at the end we may have
	// to settle for a right-edge.

	if ((pRunEnd==NULL) && (offset==ep.getTail()->getFragEndOffset()))
	{
		pRun = ep.getTail();
		goto FoundRun;
	}
	
	wxASSERT_MSG( (0), _T("Coding Error") );
	return false;

FoundRun:
	const fl_line * pFlLine = pRun->getLine();
	if (ppFlLine) *ppFlLine = pFlLine;

	if (pCol)
	{
		int col = pFlLine->getColumnOfFragAndOffset(cColTabWidth,pFrag,offset);
		*pCol = col;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////

fl_line * fl_fl::getNthLine(long n)
{
	// return the nth line in the formatted document.
	// 
	// WARNING: this is expensive because we just have
	// WARNING: a list -- not a vector.

	if (FL_INV_IS_SET(m_invUnion,FL_INV_LINENR))
		format();

	long k=0;
	for (fl_line * pFlLine=m_pLineHead; (pFlLine); pFlLine=pFlLine->getNext())
	{
		if (k==n)
			return pFlLine;
		k++;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fl_fl::dump(int indent) const
{
	wxLogTrace(TRACE_FLFL_DUMP, _T("%*cFL_FL: [%p]"), indent,' ',this);

	for (fl_line * pLine=m_pLineHead; (pLine); pLine=pLine->getNext())
		pLine->dump(indent+5);
}
#endif
