// fs_fs_table.cpp
// a container of fs_fs.
// we maintain a central table of all fs_fs (fileset (2-way/3-way) docs)
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <rs.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

fs_fs_table::fs_fs_table(void)
	: m_reftab( _T("fs_fs_table"), true)
{
}

fs_fs_table::~fs_fs_table(void)
{
	// map keys are pointers that we allocated, so we're responsible for
	// deleteing them before the map gets destroyed.
	//
	// but this table should be empty before we get destroyed since all
	// windows should have been closed before the app cleans up globals,
	// so we set 'bRequireEmptyOnDestroy' in util_reftab.
}

//////////////////////////////////////////////////////////////////

fs_fs * fs_fs_table::create(const wxString & path0, const wxString & path1, const cl_args * pArgs)
{
	// create (allocate) a new fs_fs for this set of files.
	// we set the files, but defer the actual loading of the
	// piecetables until the later.

	poi_item * pPoi0 = gpPoiItemTable->addItem(path0);
	poi_item * pPoi1 = gpPoiItemTable->addItem(path1);

	fs_fs * pFsFs = find(pPoi0,pPoi1);
	if (!pFsFs)
	{
		pFsFs = new fs_fs(pArgs);
		pFsFs->setFiles(pPoi0,pPoi1);
	}
	
	m_reftab.addRef(pFsFs);

	return pFsFs;
}

fs_fs * fs_fs_table::create(const wxString & path0, const wxString & path1, const wxString & path2, const cl_args * pArgs)
{
	// create (allocate) a new fs_fs for this set of files.
	// we set the files, but defer the actual loading of the
	// piecetables until the later.
	
	poi_item * pPoi0 = gpPoiItemTable->addItem(path0);
	poi_item * pPoi1 = gpPoiItemTable->addItem(path1);
	poi_item * pPoi2 = gpPoiItemTable->addItem(path2);

	fs_fs * pFsFs = find(pPoi0,pPoi1,pPoi2);
	if (!pFsFs)
	{
		pFsFs = new fs_fs(pArgs);
		pFsFs->setFiles(pPoi0,pPoi1,pPoi2);
	}
	
	m_reftab.addRef(pFsFs);

	return pFsFs;
}

//////////////////////////////////////////////////////////////////

struct _find_param
{
	poi_item *		pPoi[__NR_TOP_PANELS__];
};

static bool _apply_find_cb(void * /*pVoid_This*/, void * pVoid_FindParam, void * pVoid_Object_Candidate)
{
	fs_fs * pFsFs   = (fs_fs *)pVoid_Object_Candidate;
	struct _find_param * pFindParam = (struct _find_param *)pVoid_FindParam;
	
	if (pFindParam->pPoi[PANEL_T0] != pFsFs->getPoi(SYNC_VIEW,PANEL_T0))		return false;
	if (pFindParam->pPoi[PANEL_T1] != pFsFs->getPoi(SYNC_VIEW,PANEL_T1))		return false;
	if (pFindParam->pPoi[PANEL_T2] != pFsFs->getPoi(SYNC_VIEW,PANEL_T2))		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////

fs_fs * fs_fs_table::find(const wxString & path0, const wxString & path1)
{
	// find first fs_fs that has these parameters.

	return find(gpPoiItemTable->addItem(path0),
				gpPoiItemTable->addItem(path1));
}

fs_fs * fs_fs_table::find(const wxString & path0, const wxString & path1, const wxString & path2)
{
	// find first fs_fs that has these parameters.

	return find(gpPoiItemTable->addItem(path0),
				gpPoiItemTable->addItem(path1),
				gpPoiItemTable->addItem(path2));
}

//////////////////////////////////////////////////////////////////

fs_fs * fs_fs_table::find(poi_item * pPoi0, poi_item * pPoi1)
{
	// find first fs_fs that has these parameters.

	struct _find_param findParam;
	findParam.pPoi[0]    = pPoi0;
	findParam.pPoi[1]    = pPoi1;
	findParam.pPoi[2]    = NULL;
	
	fs_fs * pFsFs = (fs_fs *)m_reftab.apply(_apply_find_cb,this,&findParam);

	return pFsFs;
}

fs_fs * fs_fs_table::find(poi_item * pPoi0, poi_item * pPoi1, poi_item * pPoi2)
{
	// find first fs_fs that has these parameters.

	struct _find_param findParam;
	findParam.pPoi[0]    = pPoi0;
	findParam.pPoi[1]    = pPoi1;
	findParam.pPoi[2]    = pPoi2;
	
	fs_fs * pFsFs = (fs_fs *)m_reftab.apply(_apply_find_cb,this,&findParam);

	return pFsFs;
}

//////////////////////////////////////////////////////////////////
// applyNewRuleSetTable()
// 
// the caller is about to install a new ruleset-table.
// we must do a "changeRuleset" to each fs_fs in our
// table.
//
// the problem we have is how to map from a ruleset in
// the current table to one in the new table.  since the
// user can rename a ruleset, delete a ruleset, etc, this
// is not straight-forward.
//
// this routine must match the essence of rs_ruleset_table::findBestRuleSet()
// but with additional guessing to try to minimize the number of times we
// hit the user with a dialog.
//
//////////////////////////////////////////////////////////////////

static const rs_ruleset * _apply_new_ruleset_cb_lookup(const rs_ruleset_table * pRST_New, fs_fs * pFsFs)
{
	const rs_ruleset * pRS_NewDefault		= pRST_New->getDefaultRuleSet();

	const rs_ruleset * pRS_Current			= pFsFs->getRuleSet();

	//////////////////////////////////////////////////////////////////
	// if enable-custom-rulesets is turned off, we must use the default.

	bool bEnableCustomRulesets = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS);
	if (!bEnableCustomRulesets)
		return pRS_NewDefault;

	//////////////////////////////////////////////////////////////////
	// so, custom rulesets are enabled.
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// try to match by ID -- this is a hidden field.  there should only
	// be one ruleset with any given ID in a rule-set-table.  this should
	// match whenever the corresponding ruleset (created using the copy
	// ctor) is still present in the new table REGARDLESS WHAT THEY MAY
	// HAVE DONE TO THE COPY.

	const rs_ruleset * pRS_NewByID = pRST_New->findByID(pRS_Current->getID());
	if (pRS_NewByID)
		return pRS_NewByID;

	//////////////////////////////////////////////////////////////////
	// so the ruleset corresponding to ours is not in the new table.
	// this would imply that the user deleted it.  so we need to try
	// all the tricks we would have if we were freshly loading them
	// from disk.
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// if auto-match enabled, use the new ruleset-table try to find the
	// best match -- we want to try to do as much as possible without
	// bothering the user.

	bool bAutoMatch = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH);
	if (bAutoMatch)
	{
		//////////////////////////////////////////////////////////////////
		// see if the new table has a ruleset with the same name
		// as what we are currently using.  this would trigger if
		// they had deleted and recreated a ruleset with the same
		// name.
	
		const rs_ruleset * pRS_NewByName = pRST_New->findByName(pRS_Current->getName());
		if (pRS_NewByName)
		{
			// we have a name-based-match.
			// 
			// NOTE: this is possibly bogus -- if the user created a new rulest
			// NOTE: with the same name as a previous one, there is no guarantee
			// NOTE: that this is in any way related to the original.
			// NOTE:
			// NOTE: but we're going for user-observable name-consistency.

			return pRS_NewByName;
		}

		//////////////////////////////////////////////////////////////////
		// let the file-suffix matcher do its magic.

		bool bRulesetRequireCompleteMatch = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH);
		bool bRulesetIgnoreSuffixCase = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE);

		const rs_ruleset * pRS_NewGuess = pRST_New->do_automatic_best_guess(bRulesetRequireCompleteMatch,
																			bRulesetIgnoreSuffixCase,
																			pFsFs->getNrTops(),pFsFs->getPoiRefTable());
		if (pRS_NewGuess)
			return pRS_NewGuess;

		//////////////////////////////////////////////////////////////////
		// when auto-match is on and we can't find a match, we look at the
		// ask-if-no-match flag.  if it is on, we need to ask the user.
		// if it is off, we just use the default.

		bool bAskIfNoMatch = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ASK_IF_NO_MATCH);
		if (!bAskIfNoMatch)
			return pRS_NewDefault;
	}
	
	//////////////////////////////////////////////////////////////////
	// so (   (automatic matches are off)
	//     or (    (we couldn't find a match)
	//         and (we can't assume default)))
	//
	// so we need to ask for help.
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// time to punt and ask them for help.  let's raise one of
	// the windows that is using this fs_fs and then raise the
	// choose-ruleset dialog on top of it. (sigh)

	int bMerge = (pFsFs->getNrTops() == 3);
	gui_frame * pFrame = ((bMerge) ? gpFrameFactory->findFileMergeFrame(pFsFs) : gpFrameFactory->findFileDiffFrame(pFsFs));
	if (!pFrame)
	{
		// there is no open window referencing this fs-fs.
		// this would imply a reference count problem on the fs-fs.

		wxASSERT_MSG( (0), _T("Coding Error!") );
		return pRS_NewDefault;
	}
		
	gpFrameFactory->raiseFrame(pFrame);		// warning: this may or may not work on all platforms (due to window mangler)
		
	const rs_ruleset * pRS_NewChoice = pRST_New->do_choose_ruleset_dialog(pFrame,0,NULL);
	if (pRS_NewChoice)
		return pRS_NewChoice;

	//////////////////////////////////////////////////////////////////
	// if the user hit cancel, we just use the default ruleset.
	
	return pRS_NewDefault;
}

static bool _apply_new_ruleset_cb(void * pVoid1, void * /*pVoid2*/, void * pVoid3)
{
	const rs_ruleset_table * pRST_New		= (const rs_ruleset_table *)pVoid1;
	fs_fs * pFsFs							= (fs_fs *)pVoid3;

	const rs_ruleset * pRS_New		= _apply_new_ruleset_cb_lookup(pRST_New,pFsFs);
	
	pFsFs->changeRuleset(pRS_New);

	return false;	// keep iterating.
}

void fs_fs_table::applyNewRulesetTable(const rs_ruleset_table * pRST_New)
{
	m_reftab.apply(_apply_new_ruleset_cb,(void *)pRST_New);
}

//////////////////////////////////////////////////////////////////	

struct _rebind_param
{
	poi_item *		pPoiOld;
	poi_item *		pPoiNew;
};

static bool _apply_rebind_cb(void * /*pVoid_This*/, void * pVoid_RebindParam, void * pVoid_Object_Candidate)
{
	fs_fs * pFsFS = (fs_fs *)pVoid_Object_Candidate;
	struct _rebind_param * pRebindParam = (struct _rebind_param *)pVoid_RebindParam;

	pFsFS->rebindPoi(pRebindParam->pPoiOld, pRebindParam->pPoiNew);
	return false;	// keep iterating.
}
			
void fs_fs_table::rebindPoi(poi_item * pPoiOld, poi_item * pPoiNew)
{
	struct _rebind_param rebindParam;
	rebindParam.pPoiOld = pPoiOld;
	rebindParam.pPoiNew = pPoiNew;

	m_reftab.apply(_apply_rebind_cb,this,&rebindParam);
}

