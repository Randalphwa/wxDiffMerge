// de_de.cpp -- diff engine
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>

//////////////////////////////////////////////////////////////////

#ifdef DEBUGUTILPERF
static const wxChar * sszKey_run			= L"de_de::run";
static const wxChar * sszKey_run_sync2		= L"de_de::_run_sync2";
static const wxChar * sszKey_run_sync3		= L"de_de::_run_sync3";
static const wxChar * sszKey_run_algorithm	= L"de_de::_run_algorithm";
#endif

//////////////////////////////////////////////////////////////////

static void s_cb_rs_changed(void * pThis, const util_cbl_arg & /*arg*/)		{ ((de_de *)pThis)->cb_rs_changed(); }

static void s_cb_fl_changed_T0(void * pThis, const util_cbl_arg & arg) { ((de_de *)pThis)->cb_fl_changed_T0T2(PANEL_T0,arg); }
static void s_cb_fl_changed_T2(void * pThis, const util_cbl_arg & arg) { ((de_de *)pThis)->cb_fl_changed_T0T2(PANEL_T2,arg); }

static void s_cb_fl_changed_T1(void * pThis, const util_cbl_arg & arg) { ((de_de *)pThis)->cb_fl_changed_T1ED(SYNC_VIEW, PANEL_T1,  arg); }
static void s_cb_fl_changed_ED(void * pThis, const util_cbl_arg & arg) { ((de_de *)pThis)->cb_fl_changed_T1ED(SYNC_EDIT,PANEL_EDIT,arg); }

static void s_cb_intraline_threshold_changed(void * pThis, const util_cbl_arg & /*arg*/)	{ ((de_de *)pThis)->cb_threshold_changed(); }
static void s_cb_interline_threshold_changed(void * pThis, const util_cbl_arg & /*arg*/)	{ ((de_de *)pThis)->cb_threshold_changed(); }

static void s_cb_detail_level_changed(void * pThis, const util_cbl_arg & /*arg*/)			{ ((de_de *)pThis)->cb_detail_level_changed(); }
static void s_cb_multiline_detail_changed(void * pThis, const util_cbl_arg & /*arg*/)		{ ((de_de *)pThis)->cb_multiline_detail_changed(); }

//////////////////////////////////////////////////////////////////

// TODO consider addRef()'ing the PTables for the Layouts that we use,
// TODO since Layouts are now shared and owned by the Piecetable.

//////////////////////////////////////////////////////////////////

de_line * de_de::getDeLineFromFlLine(long kSync, PanelIndex kPanel, const fl_line * pFlLine)
{
	de_line * pDeLine = pFlLine->getSlotValue( *_lookupSlot(kSync,kPanel) );

	return pDeLine;
}

fl_slot * de_de::_lookupSlot(long kSync, PanelIndex kPanel)
{
	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error") );
	
	if (kPanel == PANEL_T1)
		return &m_slot[kSync][PANEL_T1];
	else
		return &m_slot[SYNC_VIEW][kPanel];
}

TVector_LineCmp * de_de::_lookupVecLineCmp(long kSync, PanelIndex kPanel)
{
	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error") );
	
	if (kPanel == PANEL_T1)
		return &m_vecLineCmp[kSync][PANEL_T1];
	else
		return &m_vecLineCmp[SYNC_VIEW][kPanel];
}

void de_de::_install_fl_cb(long kSync, PanelIndex kPanel, void (*pfn)(void *, const util_cbl_arg &))
{
	fl_fl * pFlFl = getLayout(kSync,kPanel);
	if (pFlFl)
	{
		pFlFl->addChangeCB(pfn,this);
		*_lookupSlot(kSync,kPanel) = pFlFl->claimSlot(this);
	}
	else
		*_lookupSlot(kSync,kPanel) = -1;
}

void de_de::_uninstall_fl_cb(long kSync, PanelIndex kPanel, void (*pfn)(void *, const util_cbl_arg &))
{
	fl_fl * pFlFl = getLayout(kSync,kPanel);
	if (!pFlFl)
		return;
	
	pFlFl->delChangeCB(pfn,this);
	pFlFl->releaseSlot(this,*_lookupSlot(kSync,kPanel));

	for (fl_line * pFlLine=pFlFl->getFirstLine(); pFlLine; pFlLine=pFlLine->getNext())
	{
		de_line * pDeLine = pFlLine->getSlotValue(*_lookupSlot(kSync,kPanel));
		if (pDeLine)
		{
			pFlLine->setSlotValue(*_lookupSlot(kSync,kPanel),NULL);
			delete pDeLine;
		}
	}
}

//////////////////////////////////////////////////////////////////

de_de::de_de(fs_fs * pFsFs,
			 de_display_ops dopsView, de_display_ops dopsEdit,
			 de_detail_level detailLevel)
	: m_chgs(DE_CHG__CHG_MASK),
	  m_detailLevel(detailLevel),
	  m_bNeedRun(true),
	  m_bCacheRSChanged(true),
	  m_nestedCalls(0),
	  m_pFsFs(pFsFs),
	  m_bCloneMarks(false)
{
#ifdef DEBUG
	m_sync_list[SYNC_VIEW].m_bIsLine = true;
	m_sync_list[SYNC_EDIT].m_bIsLine = true;
#endif

	m_bMarksChanged[SYNC_VIEW] = true;
	m_bMarksChanged[SYNC_EDIT] = true;

	m_listMarks[SYNC_VIEW].push_back( new de_mark(SYNC_VIEW) );	// seed list with trivial mark for whole document

	m_bSyncValid[SYNC_VIEW] = false;
	m_bSyncValid[SYNC_EDIT] = false;

	m_bDisplayValid[SYNC_VIEW] = false;
	m_bDisplayValid[SYNC_EDIT] = false;

	m_dops[SYNC_VIEW] = dopsView;
	m_dops[SYNC_EDIT] = dopsEdit;

	memset(m_bVecValid,0,sizeof(m_bVecValid));
	memset(m_sumOmitted,0,sizeof(m_sumOmitted));

	m_bPatchHighlight[SYNC_VIEW] = false;
	m_bPatchHighlight[SYNC_EDIT] = false;

	_install_fl_cb(SYNC_VIEW, PANEL_T0,   s_cb_fl_changed_T0);
	_install_fl_cb(SYNC_VIEW, PANEL_T1,   s_cb_fl_changed_T1);
	_install_fl_cb(SYNC_VIEW, PANEL_T2,   s_cb_fl_changed_T2);
	_install_fl_cb(SYNC_EDIT, PANEL_EDIT, s_cb_fl_changed_ED);

	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD, s_cb_intraline_threshold_changed, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_LINE_SMOOTHING_THRESHOLD,      s_cb_interline_threshold_changed, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_DETAIL_LEVEL,                  s_cb_detail_level_changed, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LEVEL,        s_cb_multiline_detail_changed, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LIMIT,        s_cb_multiline_detail_changed, this);
	
	// our caller must call run() now.  so that we can
	// return immediately without allocating a lot of stuff.
	// run();

	// ask the FS_FS to tell us whenever the Ruleset changes

	m_pFsFs->addRsChangeCB(s_cb_rs_changed,this);
}

de_de::~de_de(void)
{
	for (long kSync=0; kSync<__NR_SYNCS__; kSync++)
	{
		for (TList_DeMarkIterator it=m_listMarks[kSync].begin(); it != m_listMarks[kSync].end(); it++)
		{
			de_mark * pDeMark = (*it);
			delete pDeMark;
		}
	}

	m_pFsFs->delRsChangeCB(s_cb_rs_changed,this);

	_uninstall_fl_cb(SYNC_VIEW, PANEL_T0,   s_cb_fl_changed_T0);
	_uninstall_fl_cb(SYNC_VIEW, PANEL_T1,   s_cb_fl_changed_T1);
	_uninstall_fl_cb(SYNC_VIEW, PANEL_T2,   s_cb_fl_changed_T2);
	_uninstall_fl_cb(SYNC_EDIT, PANEL_EDIT, s_cb_fl_changed_ED);

	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD, s_cb_intraline_threshold_changed, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_LINE_SMOOTHING_THRESHOLD,      s_cb_interline_threshold_changed, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_DETAIL_LEVEL,                  s_cb_detail_level_changed, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LEVEL,        s_cb_multiline_detail_changed, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LIMIT,        s_cb_multiline_detail_changed, this);

	// TODO verify that m_cblChange is empty or properly dealt with.
}

//////////////////////////////////////////////////////////////////

fim_ptable * de_de::getPTable(long kSync, PanelIndex kPanel) const
{
	return m_pFsFs->getPTable(kSync,kPanel);
}

fl_fl * de_de::getLayout(long kSync, PanelIndex kPanel) const
{
	fim_ptable * pPTable = getPTable(kSync,kPanel);
	return ((pPTable) ? pPTable->getLayout() : NULL);
}

long de_de::_getSumOmitted(long kSync, PanelIndex kPanel) const
{
	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error") );

	if (kPanel == PANEL_T1)
		return m_sumOmitted[kSync][PANEL_T1];
	else
		return m_sumOmitted[SYNC_VIEW][kPanel];
}

long de_de::_incrSumOmitted(long kSync, PanelIndex kPanel)
{
	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error") );

	if (kPanel == PANEL_T1)
		return ++m_sumOmitted[kSync][PANEL_T1];
	else
		return ++m_sumOmitted[SYNC_VIEW][kPanel];
}

void de_de::_setSumOmitted(long kSync, PanelIndex kPanel, long value)
{
	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error") );

	if (kPanel == PANEL_T1)
		m_sumOmitted[kSync][PANEL_T1] = value;
	else
		m_sumOmitted[SYNC_VIEW][kPanel] = value;
}

long de_de::_totalOmitted(long kSync) const
{
	long total=0;

	for (int k=0; k<__NR_TOP_PANELS__; k++)
		total += _getSumOmitted(kSync,(PanelIndex)k);

	return total;
}

//////////////////////////////////////////////////////////////////

void de_de::enableEditing(bool bEditBufferIsNew)
{
	// PANEL_EDIT wasn't present when we were constructed,
	// so we need to do the things that we did in the
	// constructor as if it was.
	
	wxASSERT_MSG( (*_lookupSlot(SYNC_EDIT,PANEL_EDIT) == -1), _T("Coding Error") );
	wxASSERT_MSG( (getLayout(SYNC_EDIT,PANEL_EDIT)), _T("Coding Error") );

	_install_fl_cb(SYNC_EDIT,PANEL_EDIT, s_cb_fl_changed_ED);

	wxASSERT_MSG( (*_lookupSlot(SYNC_EDIT,PANEL_EDIT) != -1), _T("Coding Error") );

	// bEditBufferIsNew tells us if the piece-table representing
	// the edit-buffer was specifically created when editing was
	// enabled in this window.  that is, the edit-buffer and the
	// reference-document should still be identical (one having
	// been just cloned from the other).  if this is the case,
	// then we can also clone all of the MARKS (manual alignments)
	// that the user placed in the SYNC_VIEW page.  if the edit-buffer
	// already existed (it must be being edited in another window
	// where both windows have the same T1's), it may have been changed
	// by the user. so there's no guarantee that a particular line
	// in VIEW can be easily located on the EDIT page.  so we don't
	// clone the MARKS.
	//
	// WARNING: it is not sufficient to just test the dirty-bit
	// WARNING: (in pt_stat) on the edit-buffer, since they could
	// WARNING: have saved the buffer (in a /result file) after
	// WARNING: they changed it and before they opened this window.
	//
	// BUT, having a T1 shared by multiple windows is not a very
	// common circumstance, so the fact that we don't clone some
	// marks is not that big of a deal.

	if ((m_listMarks[SYNC_VIEW].size() == 1) || (!bEditBufferIsNew))
	{
		// the view page's mark list has no additional marks -- only the
		// trivial mark representing the whole document.  so we can go
		// ahead and create one for the EDIT page now.

		wxASSERT_MSG( ((*(m_listMarks[SYNC_VIEW].begin()))->isTrivial()), _T("Coding Error") );

		m_listMarks[SYNC_EDIT].push_back( new de_mark(SYNC_EDIT) );	// seed list with trivial mark for whole document
	}
	else
	{
		// the view page has additional marks and this is a fresh edit-buffer..
		//
		// when the edit page is created, we should clone the mark-list
		// from the view page (so that any manual-alignments that the
		// user already made are preserved).  BUT, the de_lines for the
		// SYNC_EDIT,PANEL_EDIT are not created until the first time
		// call to _build_vector(SYNC_EDIT,...) in run().  also we don't
		// know what state the edit-buffer's piecetable and layout are
		// in presently.
		//
		// so we need to queue a request to clone the mark list the
		// next time that run() is called.

		m_bCloneMarks = true;
	}

	m_bNeedRun = true;
}

//////////////////////////////////////////////////////////////////

void de_de::cb_fl_changed_T0T2(PanelIndex kPanel, const util_cbl_arg & arg)
{
	// the layout of the document in either T0 or T2 has changed.
	// we are using this document as PANEL_T0 or PANEL_T2 on BOTH
	// the SYNC_VIEW and SYNC_EDIT sync's (when editing is enabled).

	//wxLogTrace(TRACE_DE_DUMP, _T("de_de::cb_fl_changed_T0T2 [kPanel %d][arg %p %lx] [m_chgs on input %lx]"),kPanel,arg.m_p,arg.m_l, m_chgs);

	// note: arg.m_p contains the first line affected during this change.
	// note: arg.m_l contains the FL_INV_ or FL_MOD_ code for this change.

	fl_line * pFlLine = (fl_line *)arg.m_p;
	unsigned long flFlag = (unsigned long)arg.m_l;
	
	// something in this layout changed/moved/etc.
	// invalidate cached info as necessary and/or
	// queue diff-engine to re-run.
	
	// TODO later, try to detect if this change actually changes the
	// TODO layout in a significant manner and only invalidate then.
	// TODO for example, if this line is part of a 0-way-equal, then
	// TODO after it changes, it'll probably still be different.  we
	// TODO might be able to detect this and avoid everything in _run()
	// TODO if we can show that the output would be the same as what
	// TODO we have currently.
	// TODO
	// TODO for now, just invalidate everything for this panel.
	//
	// TODO if arg.m_l is a set-prop we may not
	// TODO need to issue a de change.

	if (m_bVecValid[SYNC_VIEW][kPanel])		// currently valid, so we need to invalidate it
	{
		m_bVecValid[SYNC_VIEW][kPanel] = false;
		m_bNeedRun = true;
	}
	//we don't need this because we share the T0 and T2 vectors on both SYNC_ pages.
	//if (m_bVecValid[SYNC_EDIT][kPanel])		// currently valid, so we need to invalidate it
	//{
	//	m_bVecValid[SYNC_EDIT][kPanel] = false;
	//	m_bNeedRun = true;
	//}

	m_chgs |= DE_CHG_VIEW_CHG|DE_CHG_EDIT_CHG;
	//wxLogTrace(TRACE_DE_DUMP, _T("de_de::cb_fl_changed_T0T2 [kPanel %d][arg %p %lx] [m_chgs set to %lx]"),kPanel,arg.m_p,arg.m_l,m_chgs);

	if (pFlLine && FL_MOD_IS_SET(flFlag,FL_MOD_DEL_LINE))
	{
		// since de_line is shared between sync-view and sync-edit, we only
		// need to delete it once -- BUT marks are not shared, so we need to
		// check for marks on both pages before deleting the line.
		_deal_with_mark_before_delete_line(SYNC_VIEW,kPanel,pFlLine);
		_deal_with_mark_before_delete_line(SYNC_EDIT,kPanel,pFlLine);
		_delete_line(SYNC_VIEW,kPanel,pFlLine);
	}
	
	// propagate a message to all watchers to let windows/views
	// force a repaint.

#if 0
	m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
#endif
}

void de_de::cb_fl_changed_T1ED(long kSync, PanelIndex kPanel, const util_cbl_arg & arg)
{
	// the layout of the document in either SYNC_VIEW,PANEL_T1 or SYNC_EDIT,PANEL_EDIT
	// has changed.

	//wxLogTrace(TRACE_DE_DUMP, _T("de_de::cb_fl_changed_T1ED [kSync %d][arg %p %lx] [m_chgs on input %lx]"),kSync,arg.m_p,arg.m_l, m_chgs);

	// note: arg.m_p contains the first line affected during this change.
	// note: arg.m_l contains the FL_INV_ or FL_MOD_ code for this change.

	fl_line * pFlLine = (fl_line *)arg.m_p;
	unsigned long flFlag = (unsigned long)arg.m_l;
	
	// something in this layout changed/moved/etc.
	// invalidate cached info as necessary and/or
	// queue diff-engine to re-run.
	
	// TODO later, try to detect if this change actually changes the
	// TODO layout in a significant manner and only invalidate then.
	// TODO for example, if this line is part of a 0-way-equal, then
	// TODO after it changes, it'll probably still be different.  we
	// TODO might be able to detect this and avoid everything in _run()
	// TODO if we can show that the output would be the same as what
	// TODO we have currently.
	// TODO
	// TODO for now, just invalidate everything for this panel.
	//
	// TODO if arg.m_l is a set-prop we may not
	// TODO need to issue a de change.

	if (m_bVecValid[kSync][kPanel])		// currently valid, so we need to invalidate it
	{
		m_bVecValid[kSync][kPanel] = false;
		m_bNeedRun = true;
	}

	m_chgs |= ((kSync==SYNC_VIEW) ? DE_CHG_VIEW_CHG : DE_CHG_EDIT_CHG);
	//wxLogTrace(TRACE_DE_DUMP, _T("de_de::cb_fl_changed_T1ED [kSync %d][arg %p %lx] [m_chgs set to %lx]"),kSync,arg.m_p,arg.m_l,m_chgs);

	if (pFlLine && FL_MOD_IS_SET(flFlag,FL_MOD_DEL_LINE))
	{
		_deal_with_mark_before_delete_line(kSync,kPanel,pFlLine);
		_delete_line(kSync,kPanel,pFlLine);
	}
	
	// propagate a message to all watchers to let windows/views
	// force a repaint.

#if 0
	m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
#endif
}

/**
 * TODO 2013/09/09 Should we take an extra arg to force a run() ?
 * TODO            I ran into this when doing some of the html export
 * TODO            and explicitly needing to force a run because I
 * TODO            was calling getStats() before seeing if I needed
 * TODO            to call getDisplayList().  Alternatively, one could
 * TODO            argue that the various getStats...() routines should
 * TODO            do a run if needed.
 *
 */
void de_de::setDisplayOps(long kSync, de_display_ops dops)
{
	if (kSync == -1)		// bogus or not-yet-initialized notebook
		return;

	if (dops == m_dops[kSync])
		return;
	
	m_dops[kSync] = dops;

	m_bDisplayValid[kSync] = false;
	if (kSync == SYNC_VIEW)
		m_chgs |= DE_CHG_VIEW_CHG;
	else
		m_chgs |= DE_CHG_EDIT_CHG;

	m_bNeedRun = true;
	
	// propagate a message to all watchers to let windows/views
	// force a repaint.

	m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
}

void de_de::cb_rs_changed(void)
{
	//wxLogTrace(TRACE_DE_DUMP, _T("de_de::cb_rs_changed: ruleset changed."));

	// if ruleset we're using changes, invalidate and signal.
	// we have to invalidate the line vectors -- because the
	// ruleset controls their construction.  (these imply that
	// the algorithm gets re-run and the display lists re-built.)

	m_bVecValid[SYNC_VIEW ][PANEL_T0] = false;
	m_bVecValid[SYNC_VIEW ][PANEL_T2] = false;

	m_bVecValid[SYNC_VIEW ][PANEL_T1  ] = false;
	m_bVecValid[SYNC_EDIT][PANEL_EDIT] = false;

	bool bEdit = (getLayout(SYNC_EDIT,PANEL_EDIT) != NULL);	// have edit page (as opposed to simple viewer)

	m_chgs = DE_CHG_VIEW_CHG;
	if (bEdit)
		m_chgs |= DE_CHG_EDIT_CHG;
	
	m_bNeedRun = true;
	m_bCacheRSChanged = true;
	
	// propagate a message to all watchers to let windows/views
	// force a repaint.

	m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
}

//////////////////////////////////////////////////////////////////

void de_de::_force_resync(void)
{
	bool bEdit = (getLayout(SYNC_EDIT,PANEL_EDIT) != NULL);	// have edit page (as opposed to simple viewer)

	m_bSyncValid[SYNC_VIEW] = false;
	if (bEdit)
		m_bSyncValid[SYNC_EDIT] = false;

	m_chgs = DE_CHG_VIEW_CHG;
	if (bEdit)
		m_chgs |= DE_CHG_EDIT_CHG;
	
	m_bNeedRun = true;

	// propagate a message to all watchers to let windows/views
	// force a repaint.

	m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
}

void de_de::cb_threshold_changed(void)
{
	// either the intra-line or inter-line smoothing threshold changed
	// we shouldn't have to rebuild the vecLineCmp's or rerun the CSSL's.
	// all we should have to do is re-sync.

	_force_resync();
}

void de_de::cb_detail_level_changed(void)
{
	// the global detail-level changed in the options dialog.
	// this is still a single global field, but i'm going to
	// cache the current value in de_de (because the batchmode
	// CLI stuff needs it in the class when calling run()).
	// eventually i'd like to make this a per-window toolbar
	// button (like pilcrow) but not today.

	m_detailLevel = (de_detail_level)gpGlobalProps->getLong(GlobalProps::GPL_FILE_DETAIL_LEVEL);

	// the diff-engine detail-level changed, re-diff.
	// we shouldn't have to rebuild the vecLineCmp's or rerun the CSSL's.
	// all we should have to do is re-sync.

	_force_resync();
}

void de_de::cb_multiline_detail_changed(void)
{
	// the diff-engine multiline-detail-level or multiline-detail-limit changed, re-diff.
	// we shouldn't have to rebuild the vecLineCmp's or rerun the CSSL's.
	// all we should have to do is re-sync.

	_force_resync();
}

//////////////////////////////////////////////////////////////////

void de_de::run(void)
{
	m_nestedCalls++;
	if (m_nestedCalls > 1)
	{
		// TODO fix places where we get called recursively -- this should not happen.
		// TODO it happens because of calls to getDisplayList()
		// TODO
		// TODO or from calls to update the UI (glance window, toolbar buttons) while
		// TODO assert dialogs up.

		//		wxASSERT_MSG( (0), _T("Coding Error") );
		wxLogTrace(TRACE_DE_DUMP, _T("de_de::run: ........RECURSIVE CALL......."));
	}
	
#ifdef DEBUGUTILPERF
	wxLogTrace(TRACE_DE_DUMP, _T("de_de::run: starting [need %d][chgs %lx][vecValid %d %d %d %d][display %d %d][nested %d]"),
			   m_bNeedRun,m_chgs,
			   m_bVecValid[SYNC_VIEW][PANEL_T0],m_bVecValid[SYNC_VIEW][PANEL_T1],m_bVecValid[SYNC_VIEW][PANEL_T2],
			   m_bVecValid[SYNC_EDIT][PANEL_EDIT],
			   m_bDisplayValid[SYNC_VIEW],m_bDisplayValid[SYNC_EDIT],
			   m_nestedCalls);
#endif
			   
	// run the diff-engine

	if (!m_bNeedRun)	// cache our dirty state so we can be quickly called
	{					// by all the various OnPaint() routines that use us.
		m_nestedCalls--;
		return;
	}

	UTIL_PERF_START_CLOCK(sszKey_run);

	long chgRun = 0;
	
	bool bMerge = (getLayout(SYNC_VIEW,PANEL_T2  ) != NULL);	// 3-way merge (as opposed to 2-way diff)
	bool bEdit  = (getLayout(SYNC_EDIT,PANEL_EDIT) != NULL);	// have edit page (as opposed to simple viewer)

	// since SYNC_VIEW page are read-only, we can cache the results for them and
	// not have to regenerate them each time -- this should be helpful when
	// the user is interactively editing the SYNC_EDIT page.

	bool bRebuilt_T0 = false;
	bool bRebuilt_T1 = false;
	bool bRebuilt_T2 = false;
	bool bRebuilt_ED = false;

	// if the document/layout/ruleset changed, we need to rebuild our vectors.

	if (          !m_bVecValid[SYNC_VIEW][PANEL_T1])		{ _build_vector(SYNC_VIEW,PANEL_T1  ); bRebuilt_T1=true; }
	if (          !m_bVecValid[SYNC_VIEW][PANEL_T0])		{ _build_vector(SYNC_VIEW,PANEL_T0  ); bRebuilt_T0=true; }
	if (bMerge && !m_bVecValid[SYNC_VIEW][PANEL_T2])		{ _build_vector(SYNC_VIEW,PANEL_T2  ); bRebuilt_T2=true; }
	if (bEdit  && !m_bVecValid[SYNC_EDIT][PANEL_EDIT])		{ _build_vector(SYNC_EDIT,PANEL_EDIT); bRebuilt_ED=true; }

	// if we have an edit page and this is the first time that we have
	// run on it, see if enableEditing() queued up a cloning of the
	// mark list.

	if (bEdit && m_bCloneMarks)
		_cloneMarks();

	// if the mark-list has changed, report to our watchers that the
	// documents have changed.  this is somewhat of a lie, but it
	// helps ensure that the scrollbars get recalibrated.  in theory,
	// we shouldn't need to do this -- since we'll also probably set
	// DE_CHG_*_RUN later and that should cause it, but it doesn't
	// hurt anything.

	if (m_bMarksChanged[SYNC_VIEW])	chgRun |= DE_CHG_VIEW_CHG;
	if (m_bMarksChanged[SYNC_EDIT])	chgRun |= DE_CHG_EDIT_CHG;
	
	// if either vector referenced by a CSSL changed, we must re-diff that pair.
	// if the set of marks changed, we need to rebuild the CSSL to reflect the new partitioning.

	if (                   (bRebuilt_T0 || bRebuilt_T1 || m_bMarksChanged[SYNC_VIEW]))	{ _run_algorithm(PANEL_T0, SYNC_VIEW,PANEL_T1  ); }
	if (         bMerge && (bRebuilt_T1 || bRebuilt_T2 || m_bMarksChanged[SYNC_VIEW]))	{ _run_algorithm(PANEL_T2, SYNC_VIEW,PANEL_T1  ); }

	if (bEdit &&           (bRebuilt_T0 || bRebuilt_ED || m_bMarksChanged[SYNC_EDIT]))	{ _run_algorithm(PANEL_T0, SYNC_EDIT,PANEL_EDIT); }
	if (bEdit && bMerge && (bRebuilt_ED || bRebuilt_T2 || m_bMarksChanged[SYNC_EDIT]))	{ _run_algorithm(PANEL_T2, SYNC_EDIT,PANEL_EDIT); }

	// if any of the CSSL's in used in a sync_list changed or a threshold changed, we must re-sync it.

	if (!m_bSyncValid[SYNC_VIEW])
	{
		m_bPatchHighlight[SYNC_VIEW] = false;

		if (bMerge) _run_sync3(SYNC_VIEW);
		else        _run_sync2(SYNC_VIEW);
	}
	if (bEdit && !m_bSyncValid[SYNC_EDIT])
	{
		m_bPatchHighlight[SYNC_EDIT] = false;

		if (bMerge) _run_sync3(SYNC_EDIT);
		else        _run_sync2(SYNC_EDIT);
	}

	// if either sync list changed or if display ops were changed, recount changes for status bar.

	if (!m_bDisplayValid[SYNC_VIEW])
	{
		if (bMerge) m_sync_list[SYNC_VIEW].count_T1T0T2_Changes(m_dops[SYNC_VIEW], &m_stats3view);
		else        m_sync_list[SYNC_VIEW].count_T1X_Changes(   m_dops[SYNC_VIEW], &m_stats2view);
	}
	if (bEdit && !m_bDisplayValid[SYNC_EDIT])
	{
		if (bMerge) m_sync_list[SYNC_EDIT].count_T1T0T2_Changes(m_dops[SYNC_EDIT], &m_stats3edit);
		else        m_sync_list[SYNC_EDIT].count_T1X_Changes(   m_dops[SYNC_EDIT], &m_stats2edit);
	}
	
	// if either sync list changed or if display ops were changed, rebuild display list.

	if (         !m_bDisplayValid[SYNC_VIEW])	{ _build_display(SYNC_VIEW); chgRun |= DE_CHG_VIEW_RUN; }
	if (bEdit && !m_bDisplayValid[SYNC_EDIT])	{ _build_display(SYNC_EDIT); chgRun |= DE_CHG_EDIT_RUN; }

	// TODO we should assert that m_cTotalRows == m_vecDisplay[SYNC_VIEW].Length()
	// TODO but only when DOPS is _ALL and we're not hiding anything.

	// TODO we should assert that the number of changes in the stats-view is approximately
	// TODO the same as the size of m_vecDisplayIndex_Change.  Same thing for conflicts when
	// TODO a 3-way.  [but the vector is only computed when we need it -- depends on DOPS.]

	// clear change flags

	m_chgs = 0;
	m_bNeedRun = false;
	m_bCacheRSChanged = false;
	m_bMarksChanged[SYNC_VIEW] = false;
	m_bMarksChanged[SYNC_EDIT] = false;

	UTIL_PERF_STOP_CLOCK(sszKey_run);
	UTIL_PERF_DUMP_ALL(_T("After run."));

#ifdef DEBUGUTILPERF
	wxLogTrace(TRACE_DE_DUMP, _T("de_de::run: at bottom of run [chgRun %lx][vecValid %d %d %d %d][display %d %d]"),
			   chgRun,
			   m_bVecValid[SYNC_VIEW][PANEL_T0],m_bVecValid[SYNC_VIEW][PANEL_T1],m_bVecValid[SYNC_VIEW][PANEL_T2],
			   m_bVecValid[SYNC_EDIT][PANEL_EDIT],
			   m_bDisplayValid[SYNC_VIEW],m_bDisplayValid[SYNC_EDIT]);
#endif

	// broadcast message to tell views that the display lists have changed.

	if (chgRun)
		m_cblChange.callAll( util_cbl_arg(this, chgRun) );

	m_nestedCalls--;
}

//////////////////////////////////////////////////////////////////

void de_de::_run_sync2(long kSync)
{
	// when kSync == SYNC_VIEW, we have T1-vs-T0 in m_cssl[SYNC_VIEW][T0].
	// when kSync == SYNC_EDIT, we have EDIT-vs-T0 in m_cssl[SYNC_EDIT][T0].
	// in either case, we always have vecLineCmp of T0 in SYNC_VIEW (since it's
	// the same in both cases).

	UTIL_PERF_START_CLOCK(sszKey_run_sync2);

	// perform 2-way populate of sync list. that is, fold relevant css's into one master list.

	if (m_listMarks[kSync].size() == 1)
	{
		// the mark list has no additional marks -- only the trivial
		// mark representing the whole document.  so we can avoid
		// partitioning/clipping and joining.

		de_mark * pMark = *(m_listMarks[kSync].begin());
		wxASSERT_MSG( (pMark && pMark->isTrivial()), _T("Coding Error") );

		m_sync_list[kSync].load_T1X(pMark->getCSSL(PANEL_T0));
	}
	else
	{
		// the mark list has more than one mark.  so the CSSL
		// is partitioned in sub-lists with a piece in each mark.

		m_sync_list[kSync].delete_list();

		for (TList_DeMarkIterator it=m_listMarks[kSync].begin(); (it != m_listMarks[kSync].end()); it++)
		{
			de_mark * pMark = (*it);

			// convert the EOF sync-node for the previous sub-list into a MARK

			de_sync * pSyncTail = m_sync_list[kSync].getTail();
			if (pSyncTail)
			{
				wxASSERT_MSG( (pSyncTail->isEND()), _T("Coding Error") );
				pSyncTail->changeToMark(pMark);
			}

			// now construct next section of the sync-list from the
			// CSSL in this mark -- and append it onto the sync-list
			// that we already have.

			const de_css_list * pCSSL = pMark->getCSSL(PANEL_T0);
			
			m_sync_list[kSync].load_T1X(pCSSL,false);
		}
	}

	//////////////////////////////////////////////////////////////////
	// we have just built the full line-oriented sync-list.  we should have
	// alternating EQ and NEQ sync-nodes.  now, we want to do some conditioning
	// on the list to make the intra-line stuff run faster and give better
	// results.
	//
	// this depends upon the detail-level that we're using.
	//////////////////////////////////////////////////////////////////

	int smoothingThreshold = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_LINE_SMOOTHING_THRESHOLD);

	switch (m_detailLevel)
	{
	default:
	case DE_DETAIL_LEVEL__LINE:
		//////////////////////////////////////////////////////////////////
		// line-oriented detail -- no intra-line stuff.  you also don't get any
		// of the important/unimportant marking.  you also don't get the graying
		// of the EOL markers.
		//
		// [] split the EQ nodes into ones that are truly equal and ones that
		//    contain changes that were not identified due to IGNORE- settings.
		//    we can do this quickly now that we have ExactUID's on the lines.
		//    then we can completely ignore the truly EQ lines.
		//
		// [] then, since the EQ-but-only-because-of-IGNORE-settings-nodes will
		//    be NEQ, we can do the line-smoothing to get rid of short EQ nodes
		//    between adjacent changes.
		//
		// WE DO NOT CALL join_up_neqs2() -- doing that would collapse all
		// vertical alignment within the change.  as it is, we will see changes
		// in chunks with VOIDS, and the only vertical alignment is provided
		// by the non-exact-eqs-turned-neq and the smoothed-eqs-turned-neq.
		//
		// TODO -NO- should we split multi-line NEQ sync-nodes into individual
		// TODO lines so that control-clicking on a line just gets the individual
		// TODO line rather than the chunk ??   (like we do at the end of
		// TODO de_de::_extractLinesFromMultiLineIntraLine2()).  i'm not sure
		// TODO that we should do this -- since there is not correlation between
		// TODO the lines (they're just chunks), there's not much advantage to
		// TODO being able to apply individual line changes.  i think we're
		// TODO probably better leaving the neq multi-line chunks as is -- and
		// TODO the eq-turned-neq nodes as chunks -- then control-click
		// TODO highlighting will highlight sensable chunks.
		//////////////////////////////////////////////////////////////////

		_split_up_eqs2(kSync);
		if (smoothingThreshold > 0)
			m_sync_list[kSync].applySmoothing2(smoothingThreshold);
		break;

	case DE_DETAIL_LEVEL__CHAR:
		//////////////////////////////////////////////////////////////////
		// character-level detail -- full char-by-char intra-line diffing
		// that spans multiple lines.  THIS IS VERY EXPENSIVE.
		// 
		// [] split the EQ nodes into ones that are truly equal and ones that
		//    contain changes that were not identified due to IGNORE- settings.
		//    we can do this quickly now that we have ExactUID's on the lines.
		//    then we can completely ignore the truly EQ lines in the intra-line
		//    diffing.
		//
		// [] then, since the EQ-but-only-because-of-IGNORE-settings-nodes will
		//    be NEQ, we can do the line-smoothing to get rid of short EQ nodes
		//    between adjacent changes.
		//
		// [] then, we can combine all of the adjacent NEQ's into single NEQs.
		//    this will let the intra-line diffing to see the complete context
		//    as one large chunk.  [and hopefully give better intra-line sync up
		//    because the CSS algorithm has more to work with.]
		//
		// [] run intra-line analysis on each (non-eq) sync node in the
		//    document.  this will also split (line-oriented) sync-nodes into
		//    important and unimportant.
		//////////////////////////////////////////////////////////////////

		_split_up_eqs2(kSync);
		if (smoothingThreshold > 0)
			m_sync_list[kSync].applySmoothing2(smoothingThreshold);
		_join_up_neqs2(kSync);
		_do_intraline2(kSync);
		break;
	}

	//////////////////////////////////////////////////////////////////
	// push omitted lines into the sync-list.

	_insert_omitted_lines_into_sync_list(kSync);

	m_bSyncValid[kSync] = true;
	m_bDisplayValid[kSync] = false;
	m_cTotalRows[kSync] = m_sync_list[kSync].getTotalRows2();
	
	UTIL_PERF_STOP_CLOCK(sszKey_run_sync2);
}

void de_de::_run_sync3(long kSync)
{
	//////////////////////////////////////////////////////////////////
	// fold the 2 independent CSSL's (T1 vs T0  and  T1 vs T2)
	// into one combined master (line-oriented) sync-list.
	//////////////////////////////////////////////////////////////////

	// when kSync == SYNC_VIEW, we have T1-vs-T0 and T1-vs-T2 in m_cssl[SYNC_VIEW][T0 and T2].
	// when kSync == SYNC_EDIT, we have EDIT-vs-T0 and EDIT-vs-T2 in m_cssl[SYNC_EDIT][T0 and T2].
	// in either case we always have vecLineCmp of T0 and T2 in SYNC_VIEW (since it's the same in
	// both cases).

	wxASSERT_MSG( (getLayout(SYNC_VIEW,PANEL_T2) != NULL), _T("Coding Error"));

	UTIL_PERF_START_CLOCK(sszKey_run_sync3);

	//////////////////////////////////////////////////////////////////
	// create de_css_src_lines so that we can reference T0 vs T2 later.
	// we do not run a full CSS algorithm on these, we just need the handle
	// to reference and resolve disputes when folding the T1 vs T0 and the T1 vs T2
	// CSSL's into the combined 3-way sync-list.
	//
	// dynamically set up T0 vs T2 src reference so we can compute the 3-way diff.

	de_css_src_lines srcT0T2( _lookupVecLineCmp(kSync,PANEL_T0),
							  _lookupVecLineCmp(kSync,PANEL_T2));

	//////////////////////////////////////////////////////////////////
	// when the detail-level is "by-line-only" we should let load_T1T0T2()
	// do as much as it can to vertically align the lines and distribute
	// VOIDs -- because this is the only pass we'll take at it.
	//
	// when the detail-level is "by-line-and-by-character" (multi-line-intra-line)
	// we can let load_T1T0T2() take some short-cuts -- because we are just going
	// to glob together all the adjacent changes and do the intra-line thing.

	bool bTryHardToAlign = (m_detailLevel == DE_DETAIL_LEVEL__LINE);

	if (m_listMarks[kSync].size() == 1)
	{
		// the mark list has no additional marks -- only the trivial
		// mark representing the whole document.  so we can avoid
		// partitioning/clipping and joining.

		de_mark * pMark = *(m_listMarks[kSync].begin());
		wxASSERT_MSG( (pMark && pMark->isTrivial()), _T("Coding Error") );

		m_sync_list[kSync].load_T1T0T2(pMark->getCSSL(PANEL_T0), pMark->getCSSL(PANEL_T2), &srcT0T2, true, bTryHardToAlign);
	}
	else
	{
		// the mark list has more than one mark.  so the CSSL
		// is partitioned in sub-lists with a piece in each mark.

		m_sync_list[kSync].delete_list();

		for (TList_DeMarkIterator it=m_listMarks[kSync].begin(); (it != m_listMarks[kSync].end()); it++)
		{
			de_mark * pMark = (*it);

			// convert the EOF sync-node for the previous sub-list into a MARK

			de_sync * pSyncTail = m_sync_list[kSync].getTail();
			if (pSyncTail)
			{
				wxASSERT_MSG( (pSyncTail->isEND()), _T("Coding Error") );
				pSyncTail->changeToMark(pMark);
			}

			// now construct next section of the sync-list from the
			// CSSL in this mark -- and append it onto the sync-list
			// that we already have.

			m_sync_list[kSync].load_T1T0T2(pMark->getCSSL(PANEL_T0), pMark->getCSSL(PANEL_T2), &srcT0T2, false, bTryHardToAlign);
		}
	}

	//////////////////////////////////////////////////////////////////
	// we have just built the full line-oriented sync-list on the 3
	// documents.  now, we want to do some conditioning on the list
	// to make the results "better" (and hopefully faster).
	//
	// again, this depends upon the detail-level that we're using.
	//////////////////////////////////////////////////////////////////

	int smoothingThreshold = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_LINE_SMOOTHING_THRESHOLD);

	switch (m_detailLevel)
	{
	default:
	case DE_DETAIL_LEVEL__LINE:
		//////////////////////////////////////////////////////////////////
		// line-oriented detail -- no intra-line stuff.  you also don't get any
		// of the important/unimportant marking.  you also don't get the graying
		// of the EOL markers.
		//
		// [] split the EQ nodes into ones that are truly equal and ones that
		//    contain changes that were not identified due to IGNORE- settings.
		//    we can do this quickly now that we have ExactUID's on the lines.
		//    then we can completely ignore the truly EQ lines.
		//
		// [] then, since the EQ-but-only-because-of-IGNORE-settings-nodes will
		//    be NEQ, we can do the line-smoothing to get rid of short EQ nodes
		//    between adjacent changes.
		//
		// WE DO NOT CALL join_up_neqs2() -- doing that would collapse all
		// vertical alignment within the change.  as it is, we will see changes
		// in chunks with VOIDS, and the only vertical alignment is provided
		// by the non-exact-eqs-turned-neq and the smoothed-eqs-turned-neq.
		//
		// [] we split multi-line partial-EQs when they have a zero component.
		//    this is so that control-click highlighting will isolate them.
		//    since 2 sides are equal, we do have good correlation.
		//
		// WE DO NOT split other NEQs (either 0EQs or partial-EQs without a zero
		// component) into individual lines.  this inhibits individual line
		// control-click highlighting -- but we don't know what kind (if any)
		// correlation that we have for these chunks.
		//////////////////////////////////////////////////////////////////

		_split_up_eqs3(kSync);
		if (smoothingThreshold > 0)
			m_sync_list[kSync].applySmoothing3(smoothingThreshold);
		m_sync_list[kSync].split_up_partial_eqs_with_zeros();
		break;

	case DE_DETAIL_LEVEL__CHAR:
		//////////////////////////////////////////////////////////////////
		// character-level detail -- full char-by-char intra-line diffing
		// that spans multiple lines.  THIS IS VERY EXPENSIVE.
		// 
		// [] split the EQ nodes into ones that are truly equal and ones that
		//    contain changes that were not identified due to IGNORE- settings.
		//    we can do this quickly now that we have ExactUID's on the lines.
		//    then we can completely ignore the truly EQ lines in the intra-line
		//    diffing.
		//
		// [] then, since the EQ-but-only-because-of-IGNORE-settings-nodes will
		//    be NEQ, we can do the line-smoothing to get rid of short EQ nodes
		//    between adjacent changes.
		//
		// [] then, we can combine all of the adjacent NEQ's into single NEQs.
		//    this will let the intra-line diffing to see the complete context
		//    as one large chunk.  [and hopefully give better intra-line sync up
		//    because the CSS algorithm has more to work with.]
		//
		// [] run intra-line analysis on each (non-eq) sync node in the
		//    document.  this will also split (line-oriented) sync-nodes into
		//    important and unimportant.
		//////////////////////////////////////////////////////////////////

		_split_up_eqs3(kSync);
		if (smoothingThreshold > 0)
			m_sync_list[kSync].applySmoothing3(smoothingThreshold);
		_join_up_neqs3(kSync);
		_do_intraline3(kSync);
		break;
	}

	//////////////////////////////////////////////////////////////////
	// set the CONFLICT bit on adjacent changes.

	m_sync_list[kSync].combine_T1T0T2_conflicts();

	//////////////////////////////////////////////////////////////////
	// push omitted lines into the sync-list

	_insert_omitted_lines_into_sync_list(kSync);

	m_bSyncValid[kSync] = true;
	m_bDisplayValid[kSync] = false;
	m_cTotalRows[kSync] = m_sync_list[kSync].getTotalRows3();

	UTIL_PERF_STOP_CLOCK(sszKey_run_sync3);
}

//////////////////////////////////////////////////////////////////
long de_de::_get_ndx_for_de_line(long kSync, PanelIndex kPanel, de_line * pDeLineThis)
{
	// if the content in the line is included in the analysis,
	// it will have an ndx -- this was set in buildVector.
	//
	// if the line is omitted, it won't have an index.  so we
	// can't use it to help us partition vecLineCmp and build
	// sub-CSSL's.  so return the ndx of the first line following
	// it -- this will let us do the partitioning and the sub-CSSL's
	// -- with the following line beginning the next partition.
	// we assume that when omitted lines are folded into the
	// sync list, it'll happen correctly.
	
	if (!pDeLineThis->isOmitted())
		return pDeLineThis->getNdxLineCmp();

	const fl_line * pFlLineThis = pDeLineThis->getFlLine();

	const fl_line * pFlLine = pFlLineThis->getNext();
	while (pFlLine)
	{
		de_line * pDeLine = pFlLine->getSlotValue( *_lookupSlot(kSync,kPanel) );
		if (!pDeLine->isOmitted())
			return pDeLine->getNdxLineCmp();

		pFlLine = pFlLine->getNext();
	}
	
	return (long)(_lookupVecLineCmp(kSync,kPanel))->size();
}

de_css_src_clipped * de_de::_create_clip_src_for_mark(de_css_src_lines * pSrc, long kSync, TList_DeMarkIterator it, PanelIndex kPanelA, PanelIndex kPanelB)
{
	// create a subset (clipped de_css_src) of pSrc for the
	// partition represented by MARK in the MARK-LIST.

	de_mark * pMark = (*it);

	long ndxThis_a, ndxThis_b;
	bool bIsFirst = (it == m_listMarks[kSync].begin());
	if (bIsFirst)
	{
		wxASSERT_MSG( (pMark->isTrivial()), _T("Coding Error") );

		// the first/trivial mark doesn't have any NDX values set.

		ndxThis_a = 0;
		ndxThis_b = 0;
	}
	else
	{
		ndxThis_a = _get_ndx_for_de_line(kSync,kPanelA,pMark->getDeLine(kPanelA));
		ndxThis_b = _get_ndx_for_de_line(kSync,kPanelB,pMark->getDeLine(kPanelB));
	}
	
	// length of this partition is defined by our starting point upto the start of the next mark.
	// if there is no next mark, we use the lengths of the document.

	it++;

	long ndxNext_a, ndxNext_b;
	bool bHaveNext = (it != m_listMarks[kSync].end());
	if (bHaveNext)
	{
		de_mark * pMarkNext = (*it);
		wxASSERT_MSG( (pMarkNext && !pMarkNext->isTrivial()), _T("Coding Error") );

		ndxNext_a = _get_ndx_for_de_line(kSync,kPanelA,pMarkNext->getDeLine(kPanelA));
		ndxNext_b = _get_ndx_for_de_line(kSync,kPanelB,pMarkNext->getDeLine(kPanelB));
	}
	else
	{
		ndxNext_a = pSrc->getLenA();
		ndxNext_b = pSrc->getLenB();
	}

	long lenThis_a = ndxNext_a - ndxThis_a;
	long lenThis_b = ndxNext_b - ndxThis_b;

	de_css_src_clipped * pSrcClipped = new de_css_src_clipped( pSrc, ndxThis_a,lenThis_a, ndxThis_b,lenThis_b );

	return pSrcClipped;
}

//////////////////////////////////////////////////////////////////

void de_de::_run_algorithm(PanelIndex kPanel_x,
						   long kSync, PanelIndex kPanel_1)
{
	UTIL_PERF_START_CLOCK(sszKey_run_algorithm);

	// run the CSS diff algorithm between T1 and the given panel.
	// _1 has the center panel (either SYNC_VIEW,PANEL_T1 or SYNC_EDIT,PANEL_EDIT)
	// _x has either T0 or T2 (and _VIEW vs _EDIT doesn't matter because the
	// vecLinCmp are shared and we only store info in _VIEW).  we do respect
	// the kSync when stroring the resulting cssl.

	TVector_LineCmp * pVecLineCmp_x = _lookupVecLineCmp(SYNC_VIEW,kPanel_x);
	TVector_LineCmp * pVecLineCmp_1 = _lookupVecLineCmp(kSync,    kPanel_1);
	
	de_css_src_lines src(pVecLineCmp_1, pVecLineCmp_x);		// build css_src handle for T1 vs Tx

	if (m_listMarks[kSync].size() == 1)
	{
		// the mark list has no additional marks -- only the trivial
		// mark representing the whole document.  so we can avoid
		// partitioning/clipping.

		de_mark * pMark = *(m_listMarks[kSync].begin());
		wxASSERT_MSG( (pMark && pMark->isTrivial()), _T("Coding Error") );

		pMark->getCSSL(kPanel_x)->runAlgorithm( &src );		// run css and store in Tx

#ifdef DEBUG
//		wxLogTrace(TRACE_DE_DUMP,_T("de_de::_run_algorithm(Panel %d, Sync %d, Panel %d) yields:"),kPanel_x,kSync,kPanel_1);
//		pMark->getCSSL(kPanel_x)->dump(10);
#endif
	}
	else
	{
		// the mark list has more than one mark.  the first is the
		// trivial mark for the beginning of the document.  the
		// subsequent ones are boundaries; they have ndxLine values
		// defining where in the vecLineCmp that they begin.  the
		// last mark in the list implicitly represents the rest of
		// the document.
		//
		// construct subset CSSL's for each partition.

		for (TList_DeMarkIterator it=m_listMarks[kSync].begin(); (it != m_listMarks[kSync].end()); it++)
		{
			de_mark * pMark = (*it);
			de_css_src_clipped * pSrcClipped = _create_clip_src_for_mark(&src,kSync,it,kPanel_1,kPanel_x);	// create sub-set for T1 vs Tx

			pMark->getCSSL(kPanel_x)->runAlgorithm(pSrcClipped);		// run CSS for this sub-set, store in Tx
			pMark->getCSSL(kPanel_x)->rel2abs(pSrcClipped->getOffsetA(),pSrcClipped->getOffsetB());		// convert absolute coords from than partition-relative

#ifdef DEBUG
//		wxLogTrace(TRACE_DE_DUMP,_T("de_de::_run_algorithm(Panel %d, Sync %d, Panel %d) by mark yields:"),kPanel_x,kSync,kPanel_1);
//		pMark->getCSSL(kPanel_x)->dump(10);
#endif

			delete pSrcClipped;
		}
	}

	m_bSyncValid[kSync] = false;

	UTIL_PERF_STOP_CLOCK(sszKey_run_algorithm);
}

//////////////////////////////////////////////////////////////////

de_css_src_string::de_css_src_string(const de_line * pDeLineA, const de_line * pDeLineB, bool bMatchEOLs)
	: m_pDeLineA(pDeLineA), m_pDeLineB(pDeLineB),
	  m_bMatchEOLs(bMatchEOLs),
	  m_szA(NULL),          m_szB(NULL),
	  m_szAeol(NULL),       m_szBeol(NULL),
	  m_lenA(0),            m_lenB(0),
	  m_lenAeol(0),         m_lenBeol(0)
{
	//////////////////////////////////////////////////////////////////
	// THIS IS USED FOR INTRA-LINE DIFFS
	//////////////////////////////////////////////////////////////////
	// 
	// cache a pointer to the string buffer and lengths of each line.
	// decide how to handle EOL chars:
	//
	// if EOL's are globally respected, we virtually include the EOL
	// with the content body.

	if (pDeLineA)
	{
		m_szA = pDeLineA->m_strLine.wc_str();
		m_lenA = (long)pDeLineA->m_strLine.Len();

		if (   (bMatchEOLs)
//			&& (   (pDeLineA->m_pCTXEOL == NULL)
//				|| (RS_ATTRS_RespectEOL(pDeLineA->m_pCTXEOL->getContextAttrs())))
			)
		{
			m_szAeol = pDeLineA->m_strEOL.wc_str();
			m_lenAeol = (long)pDeLineA->m_strEOL.Len();
		}
	}

	if (pDeLineB)
	{
		m_szB = pDeLineB->m_strLine.wc_str();
		m_lenB = (long)pDeLineB->m_strLine.Len();

		if (   (bMatchEOLs)
//			&& (   (pDeLineB->m_pCTXEOL == NULL)
//				|| (RS_ATTRS_RespectEOL(pDeLineB->m_pCTXEOL->getContextAttrs())))
			)
		{
			m_szBeol = pDeLineB->m_strEOL.wc_str();
			m_lenBeol = (long)pDeLineB->m_strEOL.Len();
		}
	}
}

de_css_src_simple_strings::de_css_src_simple_strings(const wxString & strA, const wxString & strB)
{
	// WARNING: we borrow storage from caller's string

	m_szA = strA.wc_str();
	m_lenA = (long)strA.Len();

	m_szB = strB.wc_str();
	m_lenB = (long)strB.Len();
}

//////////////////////////////////////////////////////////////////

void de_de::_cloneMarks(void)
{
	// clone all of the marks from the VIEW into the EDIT page.
	// 
	// WARNING: this only works when the document in PANEL_T1 (the
	// WARNING: reference document) and the document in PANEL_EDIT
	// WARNING: (the edit-buffer document) are identical -- because
	// WARNING: there is no correspondence between them once editing
	// WARNING: has started.

	wxASSERT_MSG( (m_bCloneMarks), _T("Coding Error") );

	wxASSERT_MSG( (m_listMarks[SYNC_VIEW].size()  > 1), _T("Coding Error") );
	wxASSERT_MSG( (m_listMarks[SYNC_EDIT].size() == 0), _T("Coding Error") );

	// verify that VIEW's mark-list starts with a trivial mark and
	// create one to start EDIT's mark-list.

	TList_DeMarkIterator it = m_listMarks[SYNC_VIEW].begin();
	de_mark * pMarkView = (*it);
	wxASSERT_MSG( (pMarkView->isTrivial()), _T("Coding Error") );
	m_listMarks[SYNC_EDIT].push_back( new de_mark(SYNC_EDIT) );

	// now clone the remaining non-trivial marks

	it++;

	for (/*it*/; (it != m_listMarks[SYNC_VIEW].end()); it++)
	{
		pMarkView = (*it);
		de_line * pDeLineT0 = pMarkView->getDeLine(PANEL_T0);	// de_line for T0 is shared by both sync pages
		de_line * pDeLineT2 = pMarkView->getDeLine(PANEL_T2);	// de_line for T2 is shared by both sync pages (when present)

		// get the line in the document in PANEL_T1 in SYNC_VIEW, then try to find the same
		// spot in the document (the edit buffer).  this is a little convoluted because
		// they are in different piece-tables and hence, different layouts.  also, we can't
		// use any of the display-list-row stuff because we haven't built the display-lists
		// yet.

		const de_line * pDeLineT1 = pMarkView->getDeLine(PANEL_T1);
		const fl_line * pFlLineT1 = pDeLineT1->getFlLine();
		const fim_frag * pFragT1;
		fr_offset offsetFragT1;
		pFlLineT1->getFragAndOffsetOfColumn(0,8,&pFragT1,&offsetFragT1);
		fim_ptable * pPTableT1 = getPTable(SYNC_VIEW,PANEL_T1);
		fim_offset docPosT1 = pPTableT1->getAbsoluteOffset(pFragT1,offsetFragT1);
		fim_ptable * pPTableED = getPTable(SYNC_EDIT,PANEL_EDIT);
		fim_frag * pFragED;
		fr_offset offsetFragED;
		pPTableED->getFragAndOffset(docPosT1,&pFragED,&offsetFragED);
		fl_fl * pFlLayoutED = getLayout(SYNC_EDIT,PANEL_EDIT);
		const fl_line * pFlLineED;
		bool bFound = pFlLayoutED->getLineAndColumnOfFragAndOffset(pFragED,offsetFragED,8,&pFlLineED,NULL);
		MY_ASSERT( (bFound) );
		de_line * pDeLineED = pFlLineED->getSlotValue( *_lookupSlot(SYNC_EDIT,PANEL_EDIT) );

		// now create the new mark for the SYNC_EDIT mark-list
		// and bind the lines to this mark.
		
		de_mark * pMarkEdit = new de_mark(SYNC_EDIT,pMarkView->getType(),pDeLineT0,pDeLineED,pDeLineT2);
		pDeLineT0->setMark(SYNC_EDIT,pMarkEdit);
		pDeLineED->setMark(SYNC_EDIT,pMarkEdit);
		if (pDeLineT2) pDeLineT2->setMark(SYNC_EDIT,pMarkEdit);		// pDeLineT2 will be NULL on a 2-way

		m_listMarks[SYNC_EDIT].push_back(pMarkEdit);
	}
	
	m_bMarksChanged[SYNC_EDIT] = true;
	m_bCloneMarks = false;
}

//////////////////////////////////////////////////////////////////

void de_de::_deal_with_mark_before_delete_line(long kSync, PanelIndex kPanel,fl_line * pFlLine)
{
	de_line * pDeLine = pFlLine->getSlotValue( *_lookupSlot(kSync,kPanel));
	if (!pDeLine)
		return;
	
	if (pDeLine->hasMark(kSync))
	{
		de_mark * pDeMark = pDeLine->getMark(kSync);

		// we are deleting a line that has a mark on it.  what should we
		// do with the mark?  we can either push it forward to the next
		// line -or- just delete the mark.
		//
		// what i'd like to do is push the mark forward to the next line
		// unless that line also has a mark.  if it does, we just delete
		// our mark.  this should let them edit near the mark without
		// a lot of strange artifacts.

		// WARNING: we rely on the fact that fl_fl::_delete_line() has not
		// WARNING: cleared the next/prev link fields in the fl_line that
		// WARNING: it is in the process of deleting.

		bool bDeleteMark = false;

		fl_line * pFlLineNext = pFlLine->getNext();
		if (!pFlLineNext)
			bDeleteMark = true;
		else
		{
			de_line * pDeLineNext = pFlLineNext->getSlotValue( *_lookupSlot(kSync,kPanel));
			if (!pDeLineNext)
				bDeleteMark = true;
			else
			{
				if (pDeLineNext->hasMark(kSync))
					bDeleteMark = true;
				else
				{
					//wxLogTrace(TRACE_DE_DUMP, _T("_delete_line: line deleted; transferring mark to next line"));
						
					pDeLine->setMark(kSync,NULL);
					pDeLineNext->setMark(kSync,pDeMark);
					pDeMark->setDeLine(kPanel,pDeLineNext);
					m_bMarksChanged[kSync] = true;
				}
			}
		}

		if (bDeleteMark)
		{
			//wxLogTrace(TRACE_DE_DUMP, _T("_delete_line: line deleted; deleting mark with it"));
			deleteMark(kSync,pDeMark);
		}
	}
}

void de_de::_delete_line(long kSync, PanelIndex kPanel,fl_line * pFlLine)
{
	de_line * pDeLine = pFlLine->getSlotValue( *_lookupSlot(kSync,kPanel));
	if (!pDeLine)
		return;

	pFlLine->setSlotValue( *_lookupSlot(kSync,kPanel),NULL);
		
	delete pDeLine;
}

//////////////////////////////////////////////////////////////////

bool de_de::isMark(long kSync, long yRowClick, de_mark ** ppDeMark)
{
	if (ppDeMark)
		*ppDeMark = NULL;

	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	if (yRowClick >= nrRows)		// on or past the special EOF row
		return false;

	// get the sync-node of the line they clicked on

	const de_row & rDeRow = (*pDis)[yRowClick];
	const de_sync * pSync = rDeRow.getSync();

	bool bResult = pSync->isMARK();
	if (bResult && ppDeMark)
		*ppDeMark = pSync->getMark();

	return bResult;
}

void de_de::deleteMark(long kSync, long yRowClick)
{
	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	if (yRowClick >= nrRows)		// on or past the special EOF row
		return;

	// get the sync-node of the line they clicked on

	const de_row & rDeRow = (*pDis)[yRowClick];
	const de_sync * pSync = rDeRow.getSync();

	de_mark * pDeMark = pSync->getMark();

	if (!pDeMark)						// silently fail if not a mark
		return;

	deleteMark(kSync,pDeMark);
}

void de_de::deleteMark(long kSync, de_mark * pDeMark)
{
	for (TList_DeMarkIterator it=m_listMarks[kSync].begin(); it != m_listMarks[kSync].end(); it++)
	{
		de_mark * pDeMark_it = (*it);
		if (pDeMark == pDeMark_it)
		{
			m_listMarks[kSync].erase(it);
			delete pDeMark;

			m_bMarksChanged[kSync] = true;
			m_bNeedRun = true;

			// TODO SHOULDN"T WE DO m_chgs |= DE_CHG_{VIEW,EDIT}_CHG

			// propagate a message to all watchers to let windows/views
			// force a repaint.

			m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////

void de_de::deleteAllMark(int kSync)
{
	// delete all marks except for the first one -- that's the trivial one.

	TList_DeMarkIterator it = m_listMarks[kSync].begin();
	de_mark * pDeMark = (*it);
	wxASSERT_MSG( (pDeMark->isTrivial()), _T("Coding Error"));

	it++;
	for (TList_DeMarkIterator itK=it; (itK != m_listMarks[kSync].end()); itK++)
	{
		pDeMark = (*itK);
		delete pDeMark;
	}

	m_listMarks[kSync].erase(it, m_listMarks[kSync].end() );

	m_bMarksChanged[kSync] = true;
	m_bNeedRun = true;

	// TODO SHOULDN"T WE DO m_chgs |= DE_CHG_{VIEW,EDIT}_CHG

	// propagate a message to all watchers to let windows/views
	// force a repaint.

	m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
}

//////////////////////////////////////////////////////////////////

int de_de::getMarkRowNr(const de_mark * pDeMark)
{
	// we need to get the row-nr in the display list for this mark.
	// 
	// but what we have in the mark are pointers to the first DeLine
	// in each panel (that is, the beginning of the document subset
	// that this mark is definining).  these DeLines have a row-nr
	// into the display list, *BUT* because of voids there may be
	// a gap between one or two of the panels and the mark.  so we
	// have to find a panel without a void and then back up 1.
	//
	// we must call getDisplayList() to force the diff-engine to run
	// (because it is lazy).

	long kSync = pDeMark->getSync();
	getDisplayList(kSync);

	if (pDeMark->isTrivial())
		return 0;

	int row0 = (int)pDeMark->getDeLine(PANEL_T0)->getRowNr(kSync);
	int row1 = (int)pDeMark->getDeLine(PANEL_T1)->getRowNr(kSync);
	int rowMin = MyMin(row0,row1);

	bool bMerge = (getLayout(kSync,PANEL_T2  ) != NULL);	// 3-way merge (as opposed to 2-way diff)
	if (bMerge)
	{
		int row2 = (int)pDeMark->getDeLine(PANEL_T2)->getRowNr(kSync);
		rowMin = MyMin(rowMin,row2);
	}

	int rowMark = rowMin - 1;

#ifdef DEBUG
	const TVector_Display * pDis = getDisplayList(kSync);
	const de_row & rDeRow = (*pDis)[rowMark];
	wxASSERT_MSG( (rDeRow.isMARK()), _T("Coding Error") );
	wxASSERT_MSG( (rDeRow.getMark() == pDeMark), _T("Coding Error") );
#endif

	return rowMark;
}

//////////////////////////////////////////////////////////////////

int de_de::getNrMarks(int kSync) const
{
	// this should always return >= 1 because of trivial mark.

	return (int)m_listMarks[kSync].size();
}

de_mark * de_de::getNthMark(int kSync, int k) const
{
	wxASSERT_MSG( (k < getNrMarks(kSync)), _T("Coding Error") );
	if (k >= getNrMarks(kSync))
		return NULL;

	TList_DeMarkConstIterator it = m_listMarks[kSync].begin();
	for (int j=0; j<k; j++)
		it++;

	de_mark * pDeMark = (*it);

	return pDeMark;
}

//////////////////////////////////////////////////////////////////

de_mark * de_de::getTrivialMark(int kSync) const
{
	wxASSERT_MSG( (m_listMarks[SYNC_VIEW].size()  > 0), _T("Coding Error") );

	TList_DeMarkConstIterator it = m_listMarks[kSync].begin();
	de_mark * pDeMark = (*it);
	wxASSERT_MSG( (pDeMark->isTrivial()), _T("Coding Error"));

	return pDeMark;
}

//////////////////////////////////////////////////////////////////

const fl_line * de_de::getNthLine(int kSync, PanelIndex kPanel, long lineNr)
{
	// get the fl_line for the nth line in the document.  (not necessarily
	// the nth row in the display list.)

	fl_fl * pFlFl = getLayout(kSync,(PanelIndex)kPanel);
	long lineLimit = pFlFl->getFormattedLineNrs();

	if ( (lineNr < 0) || (lineNr >= lineLimit) )
		return NULL;

	// NOTE: fl_fl::getNthLine() requires a linear search (because it's just a list).
	// NOTE: this only gets called when the user is on the insert-mark dialog, so
	// NOTE: i'm not that concerned.  but we could go thru the display list to get
	// NOTE: a row (assuming that we're displaying everything and there aren't that
	// NOTE: many hidden omitted-lines) -- using the line-number as if it's a
	// NOTE: row-number and then search backwards or forwards to get to the actual
	// NOTE: desired line.  this should be much faster.

	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;
	const de_row & rDeRow_row = (*pDis)[ ((lineNr >= nrRows) ? nrRows : lineNr) ];
	const de_line * pDeLine_row = rDeRow_row.getPanelLine(kPanel);
	if (pDeLine_row)							// if we have a normal line of content
	{
		const fl_line * pFlLine_row = pDeLine_row->getFlLine();
		if (pFlLine_row->getLineNr() == lineNr)		// there were no hidden lines
			return pFlLine_row;						// so we found it exactly.

		if (pFlLine_row->getLineNr() > lineNr)			// there must be some hidden lines
		{											// so we over shot. so back up some.
			while (pFlLine_row && (pFlLine_row->getLineNr() > lineNr))
				pFlLine_row = pFlLine_row->getPrev();
			return pFlLine_row;
		}

		// there must be some stuff -- like marks -- in the display list
		// that caused us to under shoot.  so search forward some.

		while (pFlLine_row && (pFlLine_row->getLineNr() < lineNr))
				pFlLine_row = pFlLine_row->getNext();
		return pFlLine_row;
	}

	// display list row wasn't much help -- may be a MARK or EOF or VOID
	// so we do it the hard way -- by counting from beginning using the
	// layout directly.  this is expensive, but shouldn't happen that often.

	const fl_line * pFlLine = pFlFl->getNthLine(lineNr);
	return pFlLine;
}
	
util_error de_de::createMark(int kSync, de_mark_type markType, int nrFiles, long * alLineNr, de_mark ** ppDeMark, PanelIndex * pPanelError)
{
	// try to create a mark using the lines with the given line-numbers.
	// NOTE: these are 0-based line-numbers (kth line in the document
	// NOTE: and **NOT** display-list-row-numbers).
	//
	// if we have an error -- such as an out-of-range line-number,
	// return an error *and* the index of the panel in question
	// (so that the caller can properly put focus on the bad field
	// if necessary).

	de_line * pDeLine[__NR_TOP_PANELS__];

	for (int kPanel=PANEL_T0; kPanel<nrFiles; kPanel++)
	{
		// map the line-number into the fl_line in the document layout.
		const fl_line * pFlLine = getNthLine(kSync,(PanelIndex)kPanel,alLineNr[kPanel]);
		if (!pFlLine)
		{
			if (pPanelError) *pPanelError = (PanelIndex)kPanel;
			return util_error(util_error::UE_LINE_NR_RANGE);
		}
		wxASSERT_MSG( (pFlLine->getLineNr() == alLineNr[kPanel]), _T("Coding Error") );

		pDeLine[kPanel] = pFlLine->getSlotValue( *_lookupSlot(kSync,(PanelIndex)kPanel));
		if (pDeLine[kPanel]->hasMark(kSync))
		{
			if (pPanelError) *pPanelError = (PanelIndex)kPanel;
			return util_error(util_error::UE_LINE_ALREADY_HAS_MARK);
		}
	}

	// we can have lots of MARKS on documents, but they cannot overlap.
	// that is, we can't re-order the chunks in the files.  so we need
	// to verify that all the lines referenced by this mark are either
	// before/after all other marks or are properly between 2 marks.

	TList_DeMarkIterator itPlace = m_listMarks[kSync].end();
	TList_DeMarkIterator it=m_listMarks[kSync].begin();
	wxASSERT_MSG( ((*it)->isTrivial()), _T("Coding Error") );
	it++;	// skip over the first (trivial) mark
	
	for(/*it*/; it != m_listMarks[kSync].end(); it++)
	{
		const de_mark * pDeMark_it = (*it);

		for (int k=PANEL_T0; k<nrFiles-1; k++)		// verify T0,T1 (and optionally T1,T2) are on same side of existing mark.
		{
			bool bSideA = (alLineNr[k  ] < pDeMark_it->getDeLine((PanelIndex)(k  ))->getFlLine()->getLineNr());
			bool bSideB = (alLineNr[k+1] < pDeMark_it->getDeLine((PanelIndex)(k+1))->getFlLine()->getLineNr());
			if (bSideA != bSideB)
			{
				if (pPanelError) *pPanelError = (PanelIndex)(k+1);			// this is kind of arbitrary
				return util_error(util_error::UE_MARKS_CANNOT_OVERLAP);
			}

			// remember the iterator for the first mark beyond the lines referenced.

			if (bSideA && (itPlace==m_listMarks[kSync].end()))
				itPlace = it;
		}
	}
	
	de_mark * pDeMark = new de_mark(kSync,markType,pDeLine[PANEL_T0],pDeLine[PANEL_T1], ((nrFiles==3) ? pDeLine[PANEL_T2] : NULL));

	for (int kPanel=PANEL_T0; kPanel<nrFiles; kPanel++)
	{
		pDeLine[kPanel]->setMark(kSync,pDeMark);
	}

	// insert in sorted order rather

	if (itPlace == m_listMarks[kSync].end())
		m_listMarks[kSync].push_back(pDeMark);
	else
		m_listMarks[kSync].insert(itPlace,pDeMark);

	m_bMarksChanged[kSync] = true;
	m_bNeedRun = true;

	// TODO SHOULDN"T WE DO m_chgs |= DE_CHG_{VIEW,EDIT}_CHG and signal ???

	if (ppDeMark)
		*ppDeMark = pDeMark;
	
	return util_error(util_error::UE_OK);
}

//////////////////////////////////////////////////////////////////

void de_de::_split_up_eqs2(long kSync)
{
	// TODO refactor and move this to de_sync_list.

	// split 2EQ nodes that were only EQ because we were ignoring something
	// when we computed the hashes.  that is, split EQ nodes into ones that
	// are truly equal and ones that were only equal because of the ignore-
	// setting.

	const rs_ruleset * pRS  = m_pFsFs->getRuleSet();
	rs_context_attrs rsAttrs = pRS->getMatchStripAttrs();
	bool bHashWasExact = RS_ATTRS__MatchAllBits(rsAttrs, (RS_ATTRS_RESPECT_WHITE|RS_ATTRS_RESPECT_CASE) );

	if (bHashWasExact)
		return;

	TVector_LineCmp * pVecLineCmpT1 = (_lookupVecLineCmp(kSync,PANEL_T1));
	TVector_LineCmp * pVecLineCmpT0 = (_lookupVecLineCmp(kSync,PANEL_T0));

	for (de_sync * pSync = m_sync_list[kSync].getHead(); (pSync && !pSync->isEND()); pSync=pSync->getNext())
	{
		if (!pSync->isSameType(DE_ATTR_DIF_2EQ))
			continue;

		long len   = pSync->getLen(PANEL_T1);	// len(T1)==len(T0)

		long ndxT1 = pSync->getNdx(PANEL_T1);
		long ndxT0 = pSync->getNdx(PANEL_T0);
		
		const de_line * pDeLineT1 = (*pVecLineCmpT1)[ndxT1];
		const de_line * pDeLineT0 = (*pVecLineCmpT0)[ndxT0];

		bool bFirstExact = (pDeLineT1->getExactUID() == pDeLineT0->getExactUID());

		for (long kRow=1; (kRow < len); kRow++)
		{
			pDeLineT1 = (*pVecLineCmpT1)[ndxT1+kRow];
			pDeLineT0 = (*pVecLineCmpT0)[ndxT0+kRow];

			bool bkRowExact = (pDeLineT1->getExactUID() == pDeLineT0->getExactUID());

			if (bkRowExact != bFirstExact)
			{
				m_sync_list[kSync].split(pSync,kRow);
				break;
			}
		}

		if (!bFirstExact)
			pSync->setNonExactEQ(DE_ATTR_DIF_0EQ);
	}
}

void de_de::_split_up_eqs3(long kSync)
{
	// TODO refactor and move this to de_sync_list.

	// split 3EQ or partial-EQ nodes that were only EQ because we were ignoring
	// something when we computed the hashes.  that is, split EQ nodes into ones
	// that are truly equal and ones that were only equal because of the ignore-
	// setting.

	const rs_ruleset * pRS  = m_pFsFs->getRuleSet();
	rs_context_attrs rsAttrs = pRS->getMatchStripAttrs();
	bool bHashWasExact = RS_ATTRS__MatchAllBits(rsAttrs, (RS_ATTRS_RESPECT_WHITE|RS_ATTRS_RESPECT_CASE) );

	if (bHashWasExact)
		return;

	TVector_LineCmp * pVecLineCmpT1 = (_lookupVecLineCmp(kSync,PANEL_T1));
	TVector_LineCmp * pVecLineCmpT0 = (_lookupVecLineCmp(kSync,PANEL_T0));
	TVector_LineCmp * pVecLineCmpT2 = (_lookupVecLineCmp(kSync,PANEL_T2));

	for (de_sync * pSync = m_sync_list[kSync].getHead(); (pSync && !pSync->isEND()); pSync=pSync->getNext())
	{
		switch (pSync->getAttr() & DE_ATTR__TYPE_MASK)
		{
		default:
		//case DE_ATTR_EOF:
		//case DE_ATTR_OMITTED:
		//case DE_ATTR_MARK:
		//case DE_ATTR_MRG_0EQ:
			break;

		case DE_ATTR_MRG_3EQ:
			{
				long len   = pSync->getLen(PANEL_T1);	// len(T1)==len(T0)==len(T2)
				wxASSERT_MSG( (len>0), _T("Coding Error"));

				long ndxT1 = pSync->getNdx(PANEL_T1);
				long ndxT0 = pSync->getNdx(PANEL_T0);
				long ndxT2 = pSync->getNdx(PANEL_T2);
		
				const de_line * pDeLineT1 = (*pVecLineCmpT1)[ndxT1];
				const de_line * pDeLineT0 = (*pVecLineCmpT0)[ndxT0];
				const de_line * pDeLineT2 = (*pVecLineCmpT2)[ndxT2];

				bool bFirstExact10 = (pDeLineT1->getExactUID() == pDeLineT0->getExactUID());
				bool bFirstExact12 = (pDeLineT1->getExactUID() == pDeLineT2->getExactUID());
				bool bFirstExact02 = (pDeLineT0->getExactUID() == pDeLineT2->getExactUID());

				for (long kRow=1; (kRow < len); kRow++)
				{
					pDeLineT1 = (*pVecLineCmpT1)[ndxT1+kRow];
					pDeLineT0 = (*pVecLineCmpT0)[ndxT0+kRow];
					pDeLineT2 = (*pVecLineCmpT2)[ndxT2+kRow];

					bool bkRowExact10 = (pDeLineT1->getExactUID() == pDeLineT0->getExactUID());
					bool bkRowExact12 = (pDeLineT1->getExactUID() == pDeLineT2->getExactUID());

					if ((bkRowExact10 != bFirstExact10) || (bkRowExact12 != bFirstExact12))
					{
						m_sync_list[kSync].split(pSync,kRow);
						break;
					}
				}

				if (!bFirstExact10  ||  !bFirstExact12)
				{
					de_attr attr = DE_ATTR_MRG_0EQ;
					if (bFirstExact10)		attr |= DE_ATTR_MRG_T1T0EQ;
					if (bFirstExact12)		attr |= DE_ATTR_MRG_T1T2EQ;
					if (bFirstExact02)		attr |= DE_ATTR_MRG_T0T2EQ;
					
					pSync->setNonExactEQ(attr);
				}
			}
			break;

		case DE_ATTR_MRG_T0T2EQ:
			{
				long len   = pSync->getLen(PANEL_T0);	// len(T0)==len(T2)
				if (len > 0)
				{
					long ndxT0 = pSync->getNdx(PANEL_T0);
					long ndxT2 = pSync->getNdx(PANEL_T2);
		
					const de_line * pDeLineT0 = (*pVecLineCmpT0)[ndxT0];
					const de_line * pDeLineT2 = (*pVecLineCmpT2)[ndxT2];

					bool bFirstExact02 = (pDeLineT0->getExactUID() == pDeLineT2->getExactUID());

					for (long kRow=1; (kRow < len); kRow++)
					{
						pDeLineT0 = (*pVecLineCmpT0)[ndxT0+kRow];
						pDeLineT2 = (*pVecLineCmpT2)[ndxT2+kRow];

						bool bkRowExact02 = (pDeLineT0->getExactUID() == pDeLineT2->getExactUID());

						if (bkRowExact02 != bFirstExact02)
						{
							m_sync_list[kSync].split(pSync,kRow);
							break;
						}
					}

					if (!bFirstExact02)
					{
						de_attr newAttr = DE_ATTR_MRG_0EQ;
						if (pSync->getLen(PANEL_T1) == 0)
							pSync->setNonExactEQ(newAttr);
						else
							pSync->updateAttrType(newAttr);
					}
				}
			}
			break;

		case DE_ATTR_MRG_T1T0EQ:
			{
				long len   = pSync->getLen(PANEL_T1);	// len(T1)==len(T0)
				if (len > 0)
				{
					long ndxT1 = pSync->getNdx(PANEL_T1);
					long ndxT0 = pSync->getNdx(PANEL_T0);
		
					const de_line * pDeLineT1 = (*pVecLineCmpT1)[ndxT1];
					const de_line * pDeLineT0 = (*pVecLineCmpT0)[ndxT0];

					bool bFirstExact10 = (pDeLineT1->getExactUID() == pDeLineT0->getExactUID());

					for (long kRow=1; (kRow < len); kRow++)
					{
						pDeLineT1 = (*pVecLineCmpT1)[ndxT1+kRow];
						pDeLineT0 = (*pVecLineCmpT0)[ndxT0+kRow];

						bool bkRowExact10 = (pDeLineT1->getExactUID() == pDeLineT0->getExactUID());

						if (bkRowExact10 != bFirstExact10)
						{
							m_sync_list[kSync].split(pSync,kRow);
							break;
						}
					}

					if (!bFirstExact10)
					{
						de_attr newAttr = DE_ATTR_MRG_0EQ;
						if (pSync->getLen(PANEL_T2) == 0)
							pSync->setNonExactEQ(newAttr);
						else
							pSync->updateAttrType(newAttr);
					}
				}
			}
			break;

		case DE_ATTR_MRG_T1T2EQ:
			{
				long len   = pSync->getLen(PANEL_T1);	// len(T1)==len(T2)
				if (len > 0)
				{
					long ndxT1 = pSync->getNdx(PANEL_T1);
					long ndxT2 = pSync->getNdx(PANEL_T2);
		
					const de_line * pDeLineT1 = (*pVecLineCmpT1)[ndxT1];
					const de_line * pDeLineT2 = (*pVecLineCmpT2)[ndxT2];

					bool bFirstExact12 = (pDeLineT1->getExactUID() == pDeLineT2->getExactUID());

					for (long kRow=1; (kRow < len); kRow++)
					{
						pDeLineT1 = (*pVecLineCmpT1)[ndxT1+kRow];
						pDeLineT2 = (*pVecLineCmpT2)[ndxT2+kRow];

						bool bkRowExact12 = (pDeLineT1->getExactUID() == pDeLineT2->getExactUID());

						if (bkRowExact12 != bFirstExact12)
						{
							m_sync_list[kSync].split(pSync,kRow);
							break;
						}
					}

					if (!bFirstExact12)
					{
						de_attr newAttr = DE_ATTR_MRG_0EQ;
						if (pSync->getLen(PANEL_T0) == 0)
							pSync->setNonExactEQ(newAttr);
						else
							pSync->updateAttrType(newAttr);
					}
				}
			}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////

void de_de::_join_up_neqs2(long kSync)
{
	// join together adjacent NEQ nodes.  this is needed so that the multi-line intra-line analysis
	// can see the complete bounds of the change in one sync node.
	//
	// when the multiline-detail-level is _NONE, we don't need to do any of this because we won't be
	// doing any multiline-intraline stuff.
	//
	// when it is _NEQS, we only want to join the plain NEQs -- not the non-exact-EQs-converted-to-NEQs
	// *or* the EQs-that-were-smoothed-to-NEQs.
	//
	// when it is _FULL, we join all NEQs -- regardless of how they were created.

	int multilineDetailLevel = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LEVEL);
	switch (multilineDetailLevel)
	{
	default:	// silences the compiler
	case DE_MULTILINE_DETAIL_LEVEL__NONE:
		return;
		
	case DE_MULTILINE_DETAIL_LEVEL__NEQS:
		{
			de_sync * pSync = m_sync_list[kSync].getHead();
			while (pSync && !pSync->isEND())
			{
				if (pSync->isSameType(DE_ATTR_DIF_0EQ) && pSync->getNext()->isSameType(DE_ATTR_DIF_0EQ)
					&& !pSync->isNonExactEQ() && !pSync->isSmoothed()
					&& !pSync->getNext()->isNonExactEQ() && !pSync->getNext()->isSmoothed())
					m_sync_list[kSync].appendNextToCurrent(pSync);
				else
					pSync=pSync->getNext();
			}
		}
		return;
		
	case DE_MULTILINE_DETAIL_LEVEL__FULL:
		{
			de_sync * pSync = m_sync_list[kSync].getHead();
			while (pSync && !pSync->isEND())
			{
				if (pSync->isSameType(DE_ATTR_DIF_0EQ) && pSync->getNext()->isSameType(DE_ATTR_DIF_0EQ))
					m_sync_list[kSync].appendNextToCurrent(pSync);
				else
					pSync=pSync->getNext();
			}
		}
		return;
	}
}


void de_de::_join_up_neqs3(long kSync)
{
	// join together adjacent NEQ nodes.

	int multilineDetailLevel = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LEVEL);
	switch (multilineDetailLevel)
	{
	default:	// silences the compiler
	case DE_MULTILINE_DETAIL_LEVEL__NONE:
		return;
		
	case DE_MULTILINE_DETAIL_LEVEL__NEQS:
		{
			de_sync * pSync = m_sync_list[kSync].getHead();
			while (pSync && !pSync->isEND())
			{
				if (   pSync->isMergeKind()
					&& pSync->getNext()->isMergeKind()
					&& !pSync->isSameType(DE_ATTR_MRG_3EQ)
					&& !pSync->getNext()->isSameType(DE_ATTR_MRG_3EQ)
					&& !pSync->isNonExactEQ() && !pSync->isSmoothed()
					&& !pSync->getNext()->isNonExactEQ() && !pSync->getNext()->isSmoothed())
					m_sync_list[kSync].appendNextToCurrent(pSync);
				else
					pSync=pSync->getNext();
			}
		}
		return;

	case DE_MULTILINE_DETAIL_LEVEL__FULL:
		{
			de_sync * pSync = m_sync_list[kSync].getHead();
			while (pSync && !pSync->isEND())
			{
				if (   pSync->isMergeKind()
					&& pSync->getNext()->isMergeKind()
					&& !pSync->isSameType(DE_ATTR_MRG_3EQ)
					&& !pSync->getNext()->isSameType(DE_ATTR_MRG_3EQ))
					m_sync_list[kSync].appendNextToCurrent(pSync);
				else
					pSync=pSync->getNext();
			}
		}
		return;
	}
}

//////////////////////////////////////////////////////////////////

wxString de_de::dumpSupportInfo(const wxString & strIndent) const
{
	wxString str;
	wxString strIndent2 = strIndent + _T("\t");
	bool bMerge = (getLayout(SYNC_VIEW,PANEL_T2  ) != NULL);	// 3-way merge (as opposed to 2-way diff)
	bool bEdit  = (getLayout(SYNC_EDIT,PANEL_EDIT) != NULL);	// have edit page (as opposed to simple viewer)

	str += wxString::Format(_T("%sAnalysis Reference View:\n"), strIndent.wc_str());
	
	str += wxString::Format(_T("%sDisplayOps: 0x%08lx\n"),strIndent2.wc_str(),(unsigned long)m_dops[SYNC_VIEW]);
	str += wxString::Format(_T("%sAnalysis Stats: %s\n"),strIndent2.wc_str(),
							((bMerge) ? getStats3ViewMsg().wc_str() : getStats2ViewMsg().wc_str()));
	str += wxString::Format(_T("%sAlignment Marks: [count %d]\n"),strIndent2.wc_str(),getNrMarks(SYNC_VIEW));

	// TODO see if we want to include mark details

	if (bEdit)
	{
		str += wxString::Format(_T("%sAnalysis Editing View:\n"), strIndent.wc_str());
	
		str += wxString::Format(_T("%sDisplayOps: 0x%08lx\n"),strIndent2.wc_str(),(unsigned long)m_dops[SYNC_EDIT]);
		str += wxString::Format(_T("%sAnalysis Stats: %s\n"),strIndent2.wc_str(),
								((bMerge) ? getStats3EditMsg().wc_str() : getStats2EditMsg().wc_str()));
		str += wxString::Format(_T("%sAlignment Marks: [count %d]\n"),strIndent2.wc_str(),getNrMarks(SYNC_EDIT));

		// TODO see if we want to include mark details
	}
	
	return str;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void de_de::dump_row_vector(int indent, long kSync, de_display_ops dops) const
{
	wxLogTrace(TRACE_DE_DUMP, _T("%*cDE_ROW_VECTOR: [kSync %ld][valid %d][dops %x]"), indent,' ',kSync,m_bDisplayValid[kSync],dops);

	for (TVector_DisplayConstIterator it=m_vecDisplay[kSync].begin(); (it != m_vecDisplay[kSync].end()); it++)
		it->dump(indent+5);
}
	
void de_row::dump(int indent) const
{
	wxLogTrace(TRACE_DE_DUMP,
			   _T("%*cDE_ROW: [pSync %p][offset %4ld][gap %d][attr %04x] [T1 %d][T0 %d][T2 %d] [next %ld/%ld][prev %ld/%ld]"),
			   indent,' ',m_pSync,m_offset,m_bGap,m_pSync->getAttr(),
			   (m_pDeLines[PANEL_T1] ? m_pDeLines[PANEL_T1]->getFlLine()->getLineNr() : -1),
			   (m_pDeLines[PANEL_T0] ? m_pDeLines[PANEL_T0]->getFlLine()->getLineNr() : -1),
			   (m_pDeLines[PANEL_T2] ? m_pDeLines[PANEL_T2]->getFlLine()->getLineNr() : -1),
			   m_rowNextChange,m_rowNextConflict,
			   m_rowPrevChange,m_rowPrevConflict);
}
#endif

//////////////////////////////////////////////////////////////////

wxString de_de::getDisplayModeString(long kSync, int colTabWidth) const
{
	wxString str;

	str += wxString::Format( _("Ruleset: %s"), m_pFsFs->getRuleSet()->getName().wc_str() );
	
	de_display_ops dops = m_dops[kSync];

	switch (dops & DE_DOP__MODE_MASK)
	{
	default:			// to quiet compiler
	case DE_DOP_ALL:	str += _("  |  Mode: All");			break;
	case DE_DOP_DIF:	str += _("  |  Mode: Diff");		break;
	case DE_DOP_CTX:	str += _("  |  Mode: Context");		break;
	case DE_DOP_EQL:	str += _("  |  Mode: Equal Only");	break;
	}

	if (DE_DOP__IS_SET_IGN_UNIMPORTANT(dops))
		str += _("  |  Unimportant: Hidden");

	if (DE_DOP__IS_SET_HIDE_OMITTED(dops))
		str += _("  |  Omitted: Hidden");

	// add trailing space to this so that we don't get clipped on the paper

	str += wxString::Format( _("  |  Tab: %d "), colTabWidth);

	return str;
}

