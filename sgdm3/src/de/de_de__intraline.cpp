// de_de__intraline.cpp -- diff engine -- routines related to the
// intra-line sub-diff computation.  these routines start with the
// line-oriented diff and do the intra-line thing on each line that
// was identified.
//
// we assume that:
// [] the source files have been scanned and RuleSet contexts determined.
// [] the sync list has been build and the voids already determined.
// [] we have not computed the conflict-closure.
// [] we have not computed the number of changes/conflicts
// [] we have not build the display list.
// 
// for each change node in the sync list, we want to look at each
// line individually and compute the intra-line diff.  we use it along
// with the context info to characterize the line changes.  and we
// compute the overall importance of the line.  that is, if all the
// intra-line changes are in unimportant RuleSet contexts, then we
// can say that the line-change is unimportant.
//
// we can then split sync nodes in the line-oriented change list into
// important and unimportant lines.
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
static const wxChar * sszKey_do_intraline2							= L"de_de::_do_intraline2";
static const wxChar * sszKey_do_intraline3							= L"de_de::_do_intraline3";
static const wxChar * sszKey_do_simple_intraline2_sync				= L"de_de::_do_simple_intraline2_sync";
static const wxChar * sszKey_do_simple_intraline3_sync				= L"de_de::_do_simple_intraline3_sync";
static const wxChar * sszKey_do_complex_intraline2_sync				= L"de_de::_do_complex_intraline2_sync";
static const wxChar * sszKey_do_complex_intraline2_sync__cssl		= L"de_de::_do_complex_intraline2_sync:cssl";
static const wxChar * sszKey_do_complex_intraline3_sync				= L"de_de::_do_complex_intraline3_sync";
static const wxChar * sszKey_do_complex_intraline3_sync__cssl		= L"de_de::_do_complex_intraline3_sync:cssl";
static const wxChar * sszKey_extractLinesFromMultiLineIntraLine2	= L"de_de::_extractLinesFromMultiLineIntraLine2";
static const wxChar * sszKey_extractLinesFromMultiLineIntraLine3	= L"de_de::_extractLinesFromMultiLineIntraLine3";
#endif

//////////////////////////////////////////////////////////////////

void de_de::_do_intraline2(long kSync)
{
	UTIL_PERF_START_CLOCK(sszKey_do_intraline2);

	// do intra-line diff for each sync-node in the documents in a 2-way diff.

	int multilineDetailLevel = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LEVEL);
	int multilineDetailLimit = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LIMIT);

	de_sync * pSync = m_sync_list[kSync].getHead();
	while (pSync && !pSync->isEND())
	{
		// we set pSyncNext here (before we do all of the following work)
		// to be the node following the current one -- we need to take care
		// of the analysis between this node and upto the current next one.
		// splits and changes can happen to the current node in the list as
		// we work on this node.

		de_sync * pSyncNext = pSync->getNext();

		// skip over MARKS and 2EQs

		if (!pSync->isSameKind(DE_ATTR__DIF_KIND) || pSync->isSameType(DE_ATTR_DIF_2EQ))
		{
			pSync = pSyncNext;
			continue;
		}

//		wxLogTrace(TRACE_DE_DUMP,_T("TopLevelSyncBeforeIntraLine: [%p]"),pSync);
//		pSync->dump(0);
		
		wxASSERT_MSG( (pSync->isSameType(DE_ATTR_DIF_0EQ)), _T("Coding Error") );

		// compute the intra-line results for each row within pSync.
		//
		// the "non-simple" way tries to aggregate all lines within the change
		// and do the comparison -- this helps, for example, when the change
		// consists of inserting line breaks into the argument list of a function
		// call.  this is ***VERY*** expensive but the results are worth it.  this is
		// the "multi-line-intra-line" diff.
		//
		// the "simple" way does "single-line-intra-line" diff.  we use it when
		// we can because it's faster.

		if (   (pSync->getLen(PANEL_T0)==0)											// if one side is VOID
			|| (pSync->getLen(PANEL_T1)==0)											// or if other side is VOID
			|| ( pSync->getLen(PANEL_T0)==1  &&  pSync->getLen(PANEL_T1)==1 ))		// or both only 1 line long
		{
			_do_simple_intraline2_sync(kSync,pSync);	// use simple for trivial cases
		}
		else
		{
			switch (multilineDetailLevel)
			{
			default:
			case DE_MULTILINE_DETAIL_LEVEL__NONE:		// multi-line is completely disabled
				_do_simple_intraline2_sync(kSync,pSync);
				break;

			case DE_MULTILINE_DETAIL_LEVEL__NEQS:		// multi-line set to proper NEQS only
				if (   pSync->isNonExactEQ()			//   if we have a non-exact-EQ, use simple
					|| pSync->isSmoothed())				//   if we have a EQ-but-smoothed-to-NEQ, use simple
				{
					_do_simple_intraline2_sync(kSync,pSync);
					break;
				}
				else if (   (multilineDetailLimit > 0)	//   if we have a limit on proper NEQs
						 && (   (pSync->getLen(PANEL_T0) > multilineDetailLimit)	// if exceeds limit, use simple
							 || (pSync->getLen(PANEL_T1) > multilineDetailLimit)))
				{
					wxLogTrace(TRACE_PERF,
							   _T("_do_intraline: multilineDetailLimit exceeded, clipping [lenT0 %ld][lenT1 %ld]"),
							   pSync->getLen(PANEL_T0),
							   pSync->getLen(PANEL_T1));
					_do_simple_intraline2_sync(kSync,pSync);
					break;
				}
				//FALLTHRU INTENDED

			case DE_MULTILINE_DETAIL_LEVEL__FULL:		// run full complex intraline
				pSync = _do_complex_intraline2_sync(kSync,pSync);		// current node is replaced (and may have been split)
				break;
			}
		}

		// set or clear the unimportant bit in this sync
		// node based upon the rows within it.  we may
		// have to split this node one or more times.

		while (pSync != pSyncNext)
		{
			if (pSync->isSameType(DE_ATTR_DIF_0EQ))
			{
				long lenMax = pSync->getMaxLen();
				if (lenMax > 0)
				{
					//if (pSync->getIntraLineSyncList(0)->getSumChangesIntraLine() == 0)
					//	pSync->getIntraLineSyncList(0)->dump(5);

					bool bUnimportant0 = (pSync->getIntraLineSyncList(0)->getSumImportantChangesIntraLine() == 0);
					for (long kRow=1; (kRow < lenMax); kRow++)
					{
						//if (pSync->getIntraLineSyncList(kRow)->getSumChangesIntraLine() == 0)
						//	pSync->getIntraLineSyncList(kRow)->dump(5);

						bool bUnimportantK = (pSync->getIntraLineSyncList(kRow)->getSumImportantChangesIntraLine() == 0);
						if (bUnimportantK != bUnimportant0)
						{
							m_sync_list[kSync].split(pSync,kRow);
							break;
						}
					}
				
					pSync->setUnimportant(bUnimportant0);
				}
			
			}

			pSync = pSync->getNext();
		}
	}

	UTIL_PERF_STOP_CLOCK(sszKey_do_intraline2);
}

void de_de::_do_simple_intraline2_sync(long kSync, de_sync * pSync)
{
	// do intra-line diff for each sync-node in the documents in a 2-way diff.
	
	long ndxT1  = pSync->getNdx(PANEL_T1);		// starting ndx in vecLingCmp[T1] of this sync node
	long ndxT0  = pSync->getNdx(PANEL_T0);		// starting ndx in vecLineCmp[k ] of this sync node
	long lenT1  = pSync->getLen(PANEL_T1);
	long lenT0  = pSync->getLen(PANEL_T0);
		
	long lenMax = MyMax(lenT1,lenT0);

	if (lenMax <= 0)							// shouldn't happen
		return;

	UTIL_PERF_START_CLOCK(sszKey_do_simple_intraline2_sync);

	// this line-oriented sync-node represents "lenMax" lines in the documents.
	// some lines may have content in both panels; other lines will only have
	// content in one (associated with voids).  allocate and reserve space in
	// a vector to store a sync-list for each line.
		
	pSync->createIntraLineSyncListVector(lenMax);
			
	for (long jRow=0; jRow<lenMax; jRow++)
	{
		// create an intra-line sync-list for each line in this (line-oriented) sync-node.
		// 
		// note: pLine{T1,T0} will be null when we have a void on that side.

		const de_line * pLineT1 = ((jRow < lenT1) ? (*_lookupVecLineCmp(kSync,PANEL_T1))[ndxT1 + jRow] : NULL);
		const de_line * pLineT0 = ((jRow < lenT0) ? (*_lookupVecLineCmp(kSync,PANEL_T0))[ndxT0 + jRow] : NULL);
		const rs_ruleset * pRS  = m_pFsFs->getRuleSet();

		de_sync_list * pSyncIntraLine = de_sync_list::createIntraLineSyncList2((pSync->getAttr() & DE_ATTR__TYPE_MASK),
																			   pLineT1,
																			   pLineT0,
																			   pRS);

		pSync->appendIntraLineSyncList(pSyncIntraLine);	// sync node now owns the sync list we just created.
	}

	UTIL_PERF_STOP_CLOCK(sszKey_do_simple_intraline2_sync);
}

de_sync * de_de::_do_complex_intraline2_sync(long kSync, de_sync * pSync)
{
	UTIL_PERF_START_CLOCK(sszKey_do_complex_intraline2_sync);

	// do intra-line diff for a non-EQ sync-node in a 2-way diff.
	// we are given a non-EQ sync-node representing an n-line change.
	// compute the "complex" intra-line diff -- where we aggregate
	// all of the lines within the change and allow the intra-line
	// diff to cross lines.  this is, for example, helps a change
	// where they added line breaks to the arg list of a function call.

	long ndxT1  = pSync->getNdx(PANEL_T1);		// starting ndx in vecLingCmp[T1] of this sync node
	long ndxT0  = pSync->getNdx(PANEL_T0);		// starting ndx in vecLineCmp[k ] of this sync node
	long lenT1  = pSync->getLen(PANEL_T1);
	long lenT0  = pSync->getLen(PANEL_T0);
		
	wxASSERT_MSG( (MyMin(lenT1,lenT0) > 0), _T("Coding Error") );

	const rs_ruleset * pRS  = m_pFsFs->getRuleSet();
	rs_context_attrs defaultContextAttrs = pRS->getDefaultContextAttrs();
	rs_context_attrs matchStripAttrs     = pRS->getMatchStripAttrs();
	bool bMatchEOLs = RS_ATTRS_RespectEOL(matchStripAttrs);

	const de_line * pDeLineLastT1 = NULL;
	const de_line * pDeLineLastT0 = NULL;
	
	// build aggregate line for each panel -- this is the concatenation
	// of all the lines in the change.  we have to take care to deal with
	// EOL's the same way that de_css_src_string() would.  this must correspond
	// to what we do in de_sync_list::createIntraLineSyncList2().

	//UTIL_PERF_START_CLOCK(_T("de_de::_do_complex_intraline2_sync:buildStrings"));
	
	TVecLongs vecLensT0, vecLensT1;
	TVecBools vecEolsT0, vecEolsT1;
	wxString strT0, strT1;

	_do_build_multiline_string(kSync,PANEL_T0,ndxT0,lenT0,bMatchEOLs,&strT0,&pDeLineLastT0,&vecLensT0,&vecEolsT0);
	_do_build_multiline_string(kSync,PANEL_T1,ndxT1,lenT1,bMatchEOLs,&strT1,&pDeLineLastT1,&vecLensT1,&vecEolsT1);

	wxLogTrace(TRACE_PERF,
			   _T("de_de__intraline:complex_intraline2 [strlenT0 %ld][strlenT1 %ld][lenT0 %ld][lenT1 %ld][%s]"),
			   (long)strT0.Length(), (long)strT1.Length(),
			   (long)lenT0, (long)lenT1,
			   strT0.wc_str());

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_do_complex_intraline2_sync:buildStrings"));

	// run the CSS algorithm on the combined strings.  we now have a by-character
	// sync-list which just happens to span multiple lines.  this must correspond
	// to what we do in de_sync_list::createIntraLineSyncList2().

	UTIL_PERF_START_CLOCK(sszKey_do_complex_intraline2_sync__cssl);

	de_css_src_simple_strings css(strT1,strT0);
	de_css_list cssl;
	cssl.runAlgorithm(&css);

	UTIL_PERF_STOP_CLOCK(sszKey_do_complex_intraline2_sync__cssl);

	de_sync_list * pSyncListIntraLine = new de_sync_list();
	pSyncListIntraLine->load_T1X(&cssl);

	int smoothingThreshold = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD);
	if (smoothingThreshold > 0)
		pSyncListIntraLine->applySmoothing2(smoothingThreshold);
	
	// use the per-line span-context-list to apply context-info to each sync-node.
	// also, sub-divide the sync-list into nodes that don't cross contexts or lines.
	// this is a multi-line version of de_sync_list::_applyIntraLineContexts() and
	// must correspond to what we do in de_sync_list::createIntraLineSyncList2().

	pSyncListIntraLine->applyMultiLineIntraLineContexts(this,kSync,PANEL_T1,ndxT1,lenT1,vecLensT1,vecEolsT1,defaultContextAttrs);
	pSyncListIntraLine->applyMultiLineIntraLineContexts(this,kSync,PANEL_T0,ndxT0,lenT0,vecLensT0,vecEolsT0,defaultContextAttrs);

	pSyncListIntraLine->applyIntraLineImportance2(strT1,strT0, (pSync->getAttr() & DE_ATTR__TYPE_MASK), pDeLineLastT1,pDeLineLastT0, pRS);

	// now create smaller intra-line sync-lists from each line contained
	// within the multi-line-intra-line sync-list so that we can attach
	// them to the line-oriented sync node.  this may cause the line-oriented
	// sync-node to be split, if it will help with alignment and distribute
	// voids throughout the change.

	de_sync * pSyncReplacement = _extractLinesFromMultiLineIntraLine2(kSync,pSync,pSyncListIntraLine,
																	  strT1,
																	  lenT1,vecLensT1,
																	  lenT0,vecLensT0,
																	  defaultContextAttrs);

	delete pSyncListIntraLine;

	UTIL_PERF_STOP_CLOCK(sszKey_do_complex_intraline2_sync);

	UTIL_PERF_DUMP(sszKey_do_complex_intraline2_sync);

	return pSyncReplacement;
}

de_sync * de_de::_do_complex_intraline3_sync(long kSync, de_sync * pSync)
{
	UTIL_PERF_START_CLOCK(sszKey_do_complex_intraline3_sync);

	// do intra-line diff for a non-EQ sync-node in a 3-way diff.
	// we are given a non-EQ sync-node representing an n-line change.
	// compute the "complex" intra-line diff -- where we aggregate
	// all of the lines within the change and allow the intra-line
	// diff to cross lines.  this is, for example, helps a change
	// where they added line breaks to the arg list of a function call.

	long ndxT1  = pSync->getNdx(PANEL_T1);		// starting ndx in vecLingCmp[T1] of this sync node
	long ndxT0  = pSync->getNdx(PANEL_T0);		// starting ndx in vecLineCmp[T0] of this sync node
	long ndxT2  = pSync->getNdx(PANEL_T2);		// starting ndx in vecLineCmp[T2] of this sync node
	long lenT1  = pSync->getLen(PANEL_T1);
	long lenT0  = pSync->getLen(PANEL_T0);
	long lenT2  = pSync->getLen(PANEL_T2);
		
	const rs_ruleset * pRS  = m_pFsFs->getRuleSet();
	rs_context_attrs defaultContextAttrs = pRS->getDefaultContextAttrs();
	rs_context_attrs matchStripAttrs     = pRS->getMatchStripAttrs();
	bool bMatchEOLs = RS_ATTRS_RespectEOL(matchStripAttrs);

	const de_line * pDeLineLastT1 = NULL;
	const de_line * pDeLineLastT0 = NULL;
	const de_line * pDeLineLastT2 = NULL;
	
	// build aggregate line for each panel -- this is the concatenation
	// of all the lines in the change.  we have to take care to deal with
	// EOL's the same way that de_css_src_string() would.  this must correspond
	// to what we do in de_sync_list::createIntraLineSyncList3().

	TVecLongs vecLensT0, vecLensT1, vecLensT2;
	TVecBools vecEolsT0, vecEolsT1, vecEolsT2;
	wxString strT0, strT1, strT2;

	_do_build_multiline_string(kSync,PANEL_T0,ndxT0,lenT0,bMatchEOLs,&strT0,&pDeLineLastT0,&vecLensT0,&vecEolsT0);
	_do_build_multiline_string(kSync,PANEL_T1,ndxT1,lenT1,bMatchEOLs,&strT1,&pDeLineLastT1,&vecLensT1,&vecEolsT1);
	_do_build_multiline_string(kSync,PANEL_T2,ndxT2,lenT2,bMatchEOLs,&strT2,&pDeLineLastT2,&vecLensT2,&vecEolsT2);

	// run the CSS algorithm on the combined strings.  we now have a by-character
	// sync-list which just happens to span multiple lines.  this must correspond
	// to what we do in de_sync_list::createIntraLineSyncList3().

	UTIL_PERF_START_CLOCK(sszKey_do_complex_intraline3_sync__cssl);

	de_css_src_simple_strings css10(strT1,strT0);
	de_css_list cssl10;
	cssl10.runAlgorithm(&css10);

	de_css_src_simple_strings css12(strT1,strT2);
	de_css_list cssl12;
	cssl12.runAlgorithm(&css12);

	de_css_src_simple_strings css02(strT0,strT2);

	UTIL_PERF_STOP_CLOCK(sszKey_do_complex_intraline3_sync__cssl);

	de_sync_list * pSyncListIntraLine = new de_sync_list();
	pSyncListIntraLine->load_T1T0T2(&cssl10,&cssl12,&css02,true,true);

	int smoothingThreshold = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD);
	if (smoothingThreshold > 0)
		pSyncListIntraLine->applySmoothing3(smoothingThreshold);
	
	// use the per-line span-context-list to apply context-info to each sync-node.
	// also, sub-divide the sync-list into nodes that don't cross contexts or lines.
	// this is a multi-line version of de_sync_list::_applyIntraLineContexts() and
	// must correspond to what we do in de_sync_list::createIntraLineSyncList3().

	pSyncListIntraLine->applyMultiLineIntraLineContexts(this,kSync,PANEL_T1,ndxT1,lenT1,vecLensT1,vecEolsT1,defaultContextAttrs);
	pSyncListIntraLine->applyMultiLineIntraLineContexts(this,kSync,PANEL_T0,ndxT0,lenT0,vecLensT0,vecEolsT0,defaultContextAttrs);
	pSyncListIntraLine->applyMultiLineIntraLineContexts(this,kSync,PANEL_T2,ndxT2,lenT2,vecLensT2,vecEolsT2,defaultContextAttrs);

	pSyncListIntraLine->applyIntraLineImportance3(strT1,strT0,strT2,
												  (pSync->getAttr() & DE_ATTR__TYPE_MASK),
												  pDeLineLastT1,pDeLineLastT0,pDeLineLastT2,
												  pRS);

	// now create smaller intra-line sync-lists from each line contained
	// within the multi-line-intra-line sync-list so that we can attach
	// them to the line-oriented sync node.  this may cause the line-oriented
	// sync-node to be split, if it will help with alignment and distribute
	// voids throughout the change.

	de_sync * pSyncReplacement = _extractLinesFromMultiLineIntraLine3(kSync,pSync,pSyncListIntraLine,
																	  strT1,
																	  lenT1,vecLensT1,
																	  lenT0,vecLensT0,
																	  lenT2,vecLensT2,
																	  defaultContextAttrs);

	delete pSyncListIntraLine;

	UTIL_PERF_STOP_CLOCK(sszKey_do_complex_intraline3_sync);

	return pSyncReplacement;
}

//////////////////////////////////////////////////////////////////
// private classes for _extractLines...()
////////////////////////////////////////////////////////////////

class _line_assignment2
{
public:
	_line_assignment2(void)
		: m_pSyncIL(NULL),
		  m_kLineT1(0), m_kLineT0(0),
		  m_offsetT1(0), m_offsetT0(0)
		{};
	_line_assignment2(const de_sync * pSyncIL, long kLineT1, long kLineT0,
					  long offsetT1, long offsetT0)
		: m_pSyncIL(pSyncIL),
		  m_kLineT1(kLineT1), m_kLineT0(kLineT0),
		  m_offsetT1(offsetT1), m_offsetT0(offsetT0)
		{};
	_line_assignment2(const _line_assignment2 & la)
		: m_pSyncIL(la.m_pSyncIL),
		  m_kLineT1(la.m_kLineT1), m_kLineT0(la.m_kLineT0),
		  m_offsetT1(la.m_offsetT1), m_offsetT0(la.m_offsetT0)
		{};

#ifdef DEBUG
	void dump(int indent) const
		{
			wxLogTrace(TRACE_DE_DUMP,_T("%*cLA2: [%p][kLineT1 %ld][kLineT0 %ld][offsetT1 %ld][offsetT0 %ld]"),
					   indent,' ',
					   m_pSyncIL,m_kLineT1,m_kLineT0,m_offsetT1,m_offsetT0);
		};
#endif

public:
	const de_sync * m_pSyncIL;
	long m_kLineT1, m_kLineT0;		// raw line number within change block
	long m_offsetT1, m_offsetT0;	// cache relative-offset of content within line
};

class _line_assignment3
{
public:
	_line_assignment3(void)
		: m_pSyncIL(NULL),
		  m_kLineT1(0), m_kLineT0(0), m_kLineT2(0),
		  m_offsetT1(0), m_offsetT0(0), m_offsetT2(0)
		{};
	_line_assignment3(const de_sync * pSyncIL, long kLineT1, long kLineT0, long kLineT2,
					 long offsetT1, long offsetT0, long offsetT2)
		: m_pSyncIL(pSyncIL),
		  m_kLineT1(kLineT1), m_kLineT0(kLineT0), m_kLineT2(kLineT2),
		  m_offsetT1(offsetT1), m_offsetT0(offsetT0), m_offsetT2(offsetT2)
		{};
	_line_assignment3(const _line_assignment3 & la)
		: m_pSyncIL(la.m_pSyncIL),
		  m_kLineT1(la.m_kLineT1), m_kLineT0(la.m_kLineT0), m_kLineT2(la.m_kLineT2),
		  m_offsetT1(la.m_offsetT1), m_offsetT0(la.m_offsetT0), m_offsetT2(la.m_offsetT2)
		{};

#ifdef DEBUG
	void dump(int indent) const
		{
			wxLogTrace(TRACE_DE_DUMP,_T("%*cLA3: [%p][kLineT1 %ld][kLineT0 %ld][kLineT2 %ld][offsetT1 %ld][offsetT0 %ld][offsetT2 %ld]"),
					   indent,' ',
					   m_pSyncIL,m_kLineT1,m_kLineT0,m_kLineT2,m_offsetT1,m_offsetT0,m_offsetT2);
		};
#endif

public:
	const de_sync * m_pSyncIL;
	long m_kLineT1, m_kLineT0, m_kLineT2;		// raw line number within change block
	long m_offsetT1, m_offsetT0, m_offsetT2;	// cache relative-offset of content within line
};

class _line_bindings
{
public:
	_line_bindings(long kLineT1, long kLineT0, long kLineT2 = -1)
		{
			m_kLine[PANEL_T1] = kLineT1;
			m_kLine[PANEL_T0] = kLineT0;
			m_kLine[PANEL_T2] = kLineT2;
		};
		
	_line_bindings(const _line_bindings & lb)
		{
			m_kLine[PANEL_T1] = lb.m_kLine[PANEL_T1];
			m_kLine[PANEL_T0] = lb.m_kLine[PANEL_T0];
			m_kLine[PANEL_T2] = lb.m_kLine[PANEL_T2];
		};
		
	inline long getBinding(PanelIndex kPanel) const { return m_kLine[kPanel]; };

	inline void update(long kLineT1, long kLineT0, long kLineT2 = -1)
		{
			m_kLine[PANEL_T1] = kLineT1;
			m_kLine[PANEL_T0] = kLineT0;
			m_kLine[PANEL_T2] = kLineT2;
		};

#ifdef DEBUG
	void dump(int indent) const
		{
			wxLogTrace(TRACE_DE_DUMP,_T("%*cLB: [%ld][%ld][%ld]"),
					   indent,' ',
					   getBinding(PANEL_T1),getBinding(PANEL_T0),getBinding(PANEL_T2));
		};
#endif

public:
	long m_kLine[__NR_TOP_PANELS__];
};

static long s_first_nonzero(long lenMax, const std::vector<long> & vec, long kStart)
{
	// return index of first non-zero cell in vec in range [k...max).
	// if none, return k.

	for (long k=kStart; (k < lenMax); k++)
		if (vec[k] != 0)
			return k;

	return kStart;
}
	
//////////////////////////////////////////////////////////////////

de_sync * de_de::_extractLinesFromMultiLineIntraLine2(long kSync,
													  de_sync * pSyncLine,
													  de_sync_list * pSyncListIntraLine,
													  const wxString & strT1,
													  long lenVecT1, const std::vector<long> & vecLineLensT1,
													  long lenVecT0, const std::vector<long> & vecLineLensT0,
													  rs_context_attrs defaultContextAttrs)
{
#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: strT1 [%s]"),strT1.wc_str());
	for (int k=0; k<lenVecT1; k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t\t[%d]: T1 line-end [%d]"),k,vecLineLensT1[k]);
	for (int k=0; k<lenVecT0; k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t\t[%d]: T0 line-end [%d]"),k,vecLineLensT0[k]);
	pSyncLine->dump(0);
	pSyncListIntraLine->dump(20);
#endif
#endif

	UTIL_PERF_START_CLOCK(sszKey_extractLinesFromMultiLineIntraLine2);

	// break the given multi-line intra-line sync-list into individual lines
	// and stuff into pSyncLine's vector of intra-line sync-lists.
	// we may have to split this (line-oriented) sync-node so that we can
	// represent VOIDS before/between/after individual lines within this
	// node -- and better align EQ content.
	//
	// we also split this (line-oriented) sync-node into individual lines
	// (so the vector of intra-line sync-lists will only have 1 item).
	// this allows us to get individual-line control-context-clicking for free.
	//
	// note that because the offsets in individual intra-line nodes are
	// relative to the concatenated string, we need to compute line-relative
	// offsets, so we can't just cut this list apart.
	//
	// we also have the problem that a node that refers to content on
	// different lines will need to be present in different per-line lists
	// -- like EQ nodes with 1/2 in 1 line and the other half in another line.
	// we give these a special _ATTR_PARTIAL so that we don't freak out later
	// when the len(panel_1)!=len(panel_0) for a 2EQ node....
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// first, we compute the line assignments -- this is a raw cutting
	// of the given list into individual lines within each panel.  this
	// is like putting back in the line-breaks -- but it does not deal
	// with aligning the lines (inserting VOIDS) relative to the other
	// panel.
	//////////////////////////////////////////////////////////////////

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:listLA"));

	typedef std::list<_line_assignment2> ListLineAssignments;
	typedef ListLineAssignments::iterator ListLineAssignmentsIterator;
	
	ListLineAssignments listLA;

	long offsetLineT1 = 0;
	long offsetLineT0 = 0;

	long kLineT1 = s_first_nonzero(lenVecT1,vecLineLensT1,0);
	long kLineT0 = s_first_nonzero(lenVecT0,vecLineLensT0,0);

	for (const de_sync * pSyncIL=pSyncListIntraLine->getHead(); (pSyncIL && !pSyncIL->isEND()); pSyncIL=pSyncIL->getNext())
	{
		long lineEndT1 = offsetLineT1 + ((kLineT1 < lenVecT1) ? vecLineLensT1[kLineT1] : 0);
		long lineEndT0 = offsetLineT0 + ((kLineT0 < lenVecT0) ? vecLineLensT0[kLineT0] : 0);

		long offsetT1 = (long)pSyncIL->getNdx(PANEL_T1);	long lenT1 = (long)pSyncIL->getLen(PANEL_T1);
		long offsetT0 = (long)pSyncIL->getNdx(PANEL_T0);	long lenT0 = (long)pSyncIL->getLen(PANEL_T0);

		// because of the way nodes are split based upon the intra-line spans, we shouldn't
		// get any nodes which cross multiple lines.

		wxASSERT_MSG( (offsetT1+lenT1 <= lineEndT1), _T("Coding Error") );
		wxASSERT_MSG( (offsetT0+lenT0 <= lineEndT0), _T("Coding Error") );

		listLA.push_back( _line_assignment2(pSyncIL,kLineT1,kLineT0,
											offsetT1-offsetLineT1, offsetT0-offsetLineT0) );
		
		// if the end of this node is at the end of the line (and there is
		// a next line), we want to advance to the next line.

		if ((offsetT1+lenT1 == lineEndT1)  &&  (kLineT1+1 < lenVecT1))
		{
			offsetLineT1 = lineEndT1;
			kLineT1 = s_first_nonzero(lenVecT1,vecLineLensT1,kLineT1+1);
		}
		if ((offsetT0+lenT0 == lineEndT0)  &&  (kLineT0+1 < lenVecT0))
		{
			offsetLineT0 = lineEndT0;
			kLineT0 = s_first_nonzero(lenVecT0,vecLineLensT0,kLineT0+1);
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:listLA"));

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len LA %d]"),(long)listLA.size());
	for (ListLineAssignmentsIterator it=listLA.begin(); (it != listLA.end()); it++)
	{
		const _line_assignment2 & la = (*it);
		la.dump(10);
	}
#endif
#endif

	//////////////////////////////////////////////////////////////////
	// next, compute the pair-wise alignment of the lines.  we do this
	// by trying to line up the longest NON-WHITE EQ nodes on a set of
	// lines.  we create a "binding" for each match-up.
	//////////////////////////////////////////////////////////////////

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecLB"));

	typedef std::vector<_line_bindings> VecLineBindings;

	VecLineBindings vecLB;

	long kLinePrevBindingT1 = -1;	// kLine of T1 in previous binding
	long kLinePrevBindingT0 = -1;	// kLine of T0 in previous binding
	long lenOfPrevBinding = 0;		// length of previous binding
	
	for (ListLineAssignmentsIterator it=listLA.begin(); (it != listLA.end()); it++)
	{
		const _line_assignment2 & la = (*it);
		if (la.m_pSyncIL->isSameType(DE_ATTR_DIF_2EQ))
		{
			const wxChar * sz = strT1.wc_str() + la.m_pSyncIL->getNdx(PANEL_T1);
			if ((*sz != 0x00020)  &&  (*sz != 0x0009))
			{
				if ((kLinePrevBindingT1 != la.m_kLineT1) && (kLinePrevBindingT0 != la.m_kLineT0))
				{
					// start a new binding for this pair of lines.
					//wxLogTrace(TRACE_DE_DUMP,_T("LB:pushing(%ld,%ld)"),la.m_kLineT1,la.m_kLineT0);
					vecLB.push_back( _line_bindings(la.m_kLineT1,la.m_kLineT0) );
					kLinePrevBindingT1 = la.m_kLineT1;
					kLinePrevBindingT0 = la.m_kLineT0;
					lenOfPrevBinding = la.m_pSyncIL->getLen(PANEL_T1);
				}
				else if (   (   (la.m_kLineT1 == kLinePrevBindingT1)
							 || (la.m_kLineT0 == kLinePrevBindingT0))
						 && (la.m_pSyncIL->getLen(PANEL_T1) > lenOfPrevBinding))
				{
					// a longer/better match for the last binding, update it.
					//wxLogTrace(TRACE_DE_DUMP,_T("LB:updating from (%ld,%ld) to (%ld,%ld)"),
					//		   vecLB[vecLB.size()-1].getBinding(PANEL_T1),vecLB[vecLB.size()-1].getBinding(PANEL_T0),
					//		   la.m_kLineT1,la.m_kLineT0);
					vecLB[vecLB.size()-1].update(la.m_kLineT1,la.m_kLineT0);
					kLinePrevBindingT1 = la.m_kLineT1;
					kLinePrevBindingT0 = la.m_kLineT0;
					lenOfPrevBinding = la.m_pSyncIL->getLen(PANEL_T1);
				}
			}
		}
	}

	// push an {end(),end()} type reference to make sure we get the stuff
	// following the last EQ node in subsequent loops.
	vecLB.push_back( _line_bindings(pSyncLine->getLen(PANEL_T1),pSyncLine->getLen(PANEL_T0)) );

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecLB"));

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len LB %ld]"),(long)vecLB.size());
	for (int k=0; k<(int)vecLB.size(); k++)
	{
		const _line_bindings & lb = vecLB[k];
		lb.dump(10);
	}
#endif
#endif
	
	//////////////////////////////////////////////////////////////////
	// build a pair of index-maps to map from raw-line-number to aligned
	// line number.  and build a vector of line-oriented sync-nodes to
	// cut up the given sync-node.  initialize each line with an empty
	// intra-line sync-list.
	//////////////////////////////////////////////////////////////////

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecSyncs"));

	typedef std::vector<de_sync *> VecSyncs;
	VecSyncs vecSyncs;

	typedef std::vector<long> VecLineMap;
	VecLineMap vecLineMapT1, vecLineMapT0;
	
	long kRow = 0;
	long kRowMapT1 = 0;
	long kRowMapT0 = 0;

	long nrBindings = (long)vecLB.size();
	for (long kBinding=0; (kBinding<nrBindings); kBinding++)
	{
		const _line_bindings & lb = vecLB[kBinding];
		while ((kRowMapT1 < lb.getBinding(PANEL_T1)) || (kRowMapT0 < lb.getBinding(PANEL_T0)))
		{
			vecSyncs.push_back( new de_sync(PANEL_T1,pSyncLine->getNdx(PANEL_T1)+kRowMapT1,((kRowMapT1 < lb.getBinding(PANEL_T1)) ? 1 : 0),
											PANEL_T0,pSyncLine->getNdx(PANEL_T0)+kRowMapT0,((kRowMapT0 < lb.getBinding(PANEL_T0)) ? 1 : 0),
											pSyncLine->getAttr() | DE_ATTR_ML_MEMBER) );
			vecSyncs[kRow]->appendIntraLineSyncList( new de_sync_list() );
						
			if (kRowMapT1 < lb.getBinding(PANEL_T1))
			{
				vecLineMapT1.push_back(kRow);
				kRowMapT1++;
			}
			if (kRowMapT0 < lb.getBinding(PANEL_T0))
			{
				vecLineMapT0.push_back(kRow);
				kRowMapT0++;
			}
			kRow++;
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecSyncs"));

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len VecLineMapT1 %d]"),(long)vecLineMapT1.size());
	for (int k=0; k<(int)vecLineMapT1.size(); k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t[%d]"),vecLineMapT1[k]);
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len VecLineMapT0 %d]"),(long)vecLineMapT0.size());
	for (int k=0; k<(int)vecLineMapT0.size(); k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t[%d]"),vecLineMapT0[k]);
#endif
#endif

	//////////////////////////////////////////////////////////////////
	// now, walk the multi-line-intra-line sync-list (using the listLA
	// that we built) and build single-line intra-line lists in each of
	// the line-oriented sync-nodes that we created in the vecSyncs.

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:apply"));

	for (ListLineAssignmentsIterator it=listLA.begin(); (it != listLA.end()); it++)
	{
		const _line_assignment2 & la = (*it);
		const de_sync * pSync = la.m_pSyncIL;

		long lenT1 = (long)pSync->getLen(PANEL_T1);
		long lenT0 = (long)pSync->getLen(PANEL_T0);

		wxASSERT_MSG( (la.m_kLineT1 < (long)vecLineMapT1.size()), _T("Coding Error") );
		wxASSERT_MSG( (la.m_kLineT0 < (long)vecLineMapT0.size()), _T("Coding Error") );
		
		long kRowT1 = vecLineMapT1[la.m_kLineT1];
		long kRowT0 = vecLineMapT0[la.m_kLineT0];

		if (kRowT1 == kRowT0)
		{
			// a span completely contained within the current line on both sides.
			// both sides happen to be on the same line, so we can use a normal sync-node.

			de_sync * pSyncRelative = new de_sync(PANEL_T1, la.m_offsetT1, lenT1,
												  PANEL_T0, la.m_offsetT0, lenT0,
												  pSync->getAttr());
			vecSyncs[kRowT1]->getIntraLineSyncList(0)->_append(pSyncRelative);
		}
		else
		{
			// both sides happen to be on different lines, so we create partial sync-nodes.
			// we allow for either side to be VOID.
				
			if (lenT1 > 0)
			{
				de_sync_list * p = vecSyncs[kRowT1]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset = (pt ? pt->getNdx(PANEL_T0)+pt->getLen(PANEL_T0) : 0);
				
				de_sync * pSyncRelativeT1 = new de_sync(PANEL_T1, la.m_offsetT1, lenT1,
														PANEL_T0, offset, 0,
														pSync->getAttr() | ((lenT0 > 0) ? DE_ATTR_PARTIAL : 0));
				p->_append(pSyncRelativeT1);
			}
			if (lenT0 > 0)
			{
				de_sync_list * p = vecSyncs[kRowT0]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset = (pt ? pt->getNdx(PANEL_T1)+pt->getLen(PANEL_T1) : 0);

				de_sync * pSyncRelativeT0 = new de_sync(PANEL_T1, offset, 0,
														PANEL_T0, la.m_offsetT0, lenT0,
														pSync->getAttr() | ((lenT1 > 0) ? DE_ATTR_PARTIAL : 0));
				p->_append(pSyncRelativeT0);
			}
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:apply"));
	
	//////////////////////////////////////////////////////////////////
	// since each line was advanced at its own rate, we couldn't add
	// the END-sync-nodes to the individual intra-line lists as we were
	// building the lists.  so one more pass.
	//////////////////////////////////////////////////////////////////

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [rows in result %ld]"),(long)vecSyncs.size());
#endif
#endif

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:addEOLs"));

	long nrRows = (long)vecSyncs.size();
	for (long kRow2=0; (kRow2<nrRows); kRow2++)
	{
		de_sync_list * pSyncListIntraLineK = vecSyncs[kRow2]->getIntraLineSyncList(0);
		de_sync * pSyncLast = pSyncListIntraLineK->getTail();

		if (pSyncLast)
		{
			// we put something on this line.  count the changes and their importance.

			pSyncListIntraLineK->m_ChangeKindIntraLine = DE_ATTR_DIF_2EQ;
			for (de_sync * pSyncJ=pSyncListIntraLineK->getHead(); pSyncJ; pSyncJ=pSyncJ->getNext())
				if (pSyncJ->isSameType(DE_ATTR_DIF_0EQ) || pSyncJ->isPartial())
				{
					pSyncListIntraLineK->m_ChangeKindIntraLine &= DE_ATTR_DIF_0EQ;
					pSyncListIntraLineK->m_cChangesIntraLine++;
					if (!pSyncJ->isUnimportant())
						pSyncListIntraLineK->m_cImportantChangesIntraLine++;
				}
			pSyncListIntraLineK->_append( new de_sync(PANEL_T1, pSyncLast->getNdx(PANEL_T1)+pSyncLast->getLen(PANEL_T1), 0,
													  PANEL_T0, pSyncLast->getNdx(PANEL_T0)+pSyncLast->getLen(PANEL_T0), 0,
													  DE_ATTR_EOF));
			
		}
		else
		{
			// we didn't put anything on this line.  this should be a blank line
			// (when EOL chars are excluded from the analysis) opposite a VOID.

			const de_line * pDeLineT1 = NULL;
			if (vecSyncs[kRow2]->getLen(PANEL_T1) == 1)
				pDeLineT1 = getDeLineFromNdx(kSync,PANEL_T1,vecSyncs[kRow2]->getNdx(PANEL_T1));
			const de_line * pDeLineT0 = NULL;
			if (vecSyncs[kRow2]->getLen(PANEL_T0) == 1)
				pDeLineT0 = getDeLineFromNdx(kSync,PANEL_T0,vecSyncs[kRow2]->getNdx(PANEL_T0));

			wxASSERT_MSG( (pDeLineT1 || pDeLineT0), _T("Coding Error") );		// both cannot be VOID.

			pSyncListIntraLineK->_append( new de_sync(PANEL_T1, 0,0,
													  PANEL_T0, 0,0,
													  DE_ATTR_EOF));
			if (!pDeLineT1 || !pDeLineT0)
				pSyncListIntraLineK->m_cChangesIntraLine++;

			pSyncListIntraLineK->m_ChangeKindIntraLine = DE_ATTR_DIF_0EQ;

			// the importance of the line is determined by the context of the EOL.

			const rs_context * pCtxEol = ((pDeLineT1) ? pDeLineT1->getContextEOL() : pDeLineT0->getContextEOL());
			rs_context_attrs rsAttrs = ((pCtxEol) ? pCtxEol->getContextAttrs() : defaultContextAttrs);
			if (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs))	// if overall-is-important and blank-lines-are-important
				pSyncListIntraLineK->m_cImportantChangesIntraLine++;
		}
#if 0
#ifdef DEBUG
		vecSyncs[kRow]->dump(10);
		vecSyncs[kRow]->getIntraLineSyncList(0)->dump(30);
#endif
#endif
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:addEOLs"));

	//////////////////////////////////////////////////////////////////
	// now, get destructive -- replace the given line-oriented sync-node
	// (representing multiple lines) with a series of single-line sync-nodes.
	//////////////////////////////////////////////////////////////////

	nrRows = (long)vecSyncs.size();
	for (long kRow2=nrRows-1; (kRow2>=0); kRow2--)
		m_sync_list[kSync]._insert_after(pSyncLine,vecSyncs[kRow2]);

	m_sync_list[kSync]._delete_sync(pSyncLine);

	// return first line so that caller can update loop variables.

	UTIL_PERF_STOP_CLOCK(sszKey_extractLinesFromMultiLineIntraLine2);

	return vecSyncs[0];
}

de_sync * de_de::_extractLinesFromMultiLineIntraLine3(long kSync,
													  de_sync * pSyncLine,
													  de_sync_list * pSyncListIntraLine,
													  const wxString & strT1,
													  long lenVecT1, const std::vector<long> & vecLineLensT1,
													  long lenVecT0, const std::vector<long> & vecLineLensT0,
													  long lenVecT2, const std::vector<long> & vecLineLensT2,
													  rs_context_attrs defaultContextAttrs)
{
#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: strT1 [%s]"),strT1.wc_str());
	for (int k=0; k<lenVecT1; k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t\t[%d]: T1 line-end [%d]"),k,vecLineLensT1[k]);
	for (int k=0; k<lenVecT0; k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t\t[%d]: T0 line-end [%d]"),k,vecLineLensT0[k]);
	for (int k=0; k<lenVecT2; k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t\t[%d]: T2 line-end [%d]"),k,vecLineLensT2[k]);
	pSyncLine->dump(0);
	pSyncListIntraLine->dump(20);
#endif
#endif

	UTIL_PERF_START_CLOCK(sszKey_extractLinesFromMultiLineIntraLine3);

	// break the given multi-line intra-line sync-list into individual lines
	// and stuff into pSyncLine's vector of intra-line sync-lists.
	// we may have to split this (line-oriented) sync-node so that we can
	// represent VOIDS before/between/after individual lines within this
	// node -- and better align EQ content.
	//
	// we also split this (line-oriented) sync-node into individual lines
	// (so the vector of intra-line sync-lists will only have 1 item).
	// this allows us to get individual-line control-context-clicking for free.
	//
	// note that because the offsets in individual intra-line nodes are
	// relative to the concatenated string, we need to compute line-relative
	// offsets, so we can't just cut this list apart.
	//
	// we also have the problem that a node that refers to content on
	// different lines will need to be present in different per-line lists
	// -- like EQ nodes with 1/2 in 1 line and the other half in another line.
	// we give these a special _ATTR_PARTIAL so that we don't freak out later
	// when the len(panel_a)!=len(panel_b) for a 3EQ or partial-EQ node....
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// first, we compute the line assignments -- this is a raw cutting
	// of the given list into individual lines within each panel.  this
	// is like putting back in the line-breaks -- but it does not deal
	// with aligning the lines (inserting VOIDS) relative to the other
	// panel.
	//////////////////////////////////////////////////////////////////

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:listLA"));

	typedef std::list<_line_assignment3> ListLineAssignments;
	typedef ListLineAssignments::iterator ListLineAssignmentsIterator;
	
	ListLineAssignments listLA;

	long offsetLineT1 = 0;
	long offsetLineT0 = 0;
	long offsetLineT2 = 0;

	long kLineT1 = s_first_nonzero(lenVecT1,vecLineLensT1,0);
	long kLineT0 = s_first_nonzero(lenVecT0,vecLineLensT0,0);
	long kLineT2 = s_first_nonzero(lenVecT2,vecLineLensT2,0);

	for (const de_sync * pSyncIL=pSyncListIntraLine->getHead(); (pSyncIL && !pSyncIL->isEND()); pSyncIL=pSyncIL->getNext())
	{
		long lineEndT1 = offsetLineT1 + ((kLineT1 < lenVecT1) ? vecLineLensT1[kLineT1] : 0);
		long lineEndT0 = offsetLineT0 + ((kLineT0 < lenVecT0) ? vecLineLensT0[kLineT0] : 0);
		long lineEndT2 = offsetLineT2 + ((kLineT2 < lenVecT2) ? vecLineLensT2[kLineT2] : 0);

		long offsetT1 = (long)pSyncIL->getNdx(PANEL_T1);	long lenT1 = (long)pSyncIL->getLen(PANEL_T1);
		long offsetT0 = (long)pSyncIL->getNdx(PANEL_T0);	long lenT0 = (long)pSyncIL->getLen(PANEL_T0);
		long offsetT2 = (long)pSyncIL->getNdx(PANEL_T2);	long lenT2 = (long)pSyncIL->getLen(PANEL_T2);

		// because of the way nodes are split based upon the intra-line spans, we shouldn't
		// get any nodes which cross multiple lines.

		wxASSERT_MSG( (offsetT1+lenT1 <= lineEndT1), _T("Coding Error") );
		wxASSERT_MSG( (offsetT0+lenT0 <= lineEndT0), _T("Coding Error") );
		wxASSERT_MSG( (offsetT2+lenT2 <= lineEndT2), _T("Coding Error") );

		listLA.push_back( _line_assignment3(pSyncIL,kLineT1,kLineT0,kLineT2,
											offsetT1-offsetLineT1, offsetT0-offsetLineT0, offsetT2-offsetLineT2) );
		
		// if the end of this node is at the end of the line (and there is
		// a next line), we want to advance to the next line.

		if ((offsetT1+lenT1 == lineEndT1)  &&  (kLineT1+1 < lenVecT1))
		{
			offsetLineT1 = lineEndT1;
			kLineT1 = s_first_nonzero(lenVecT1,vecLineLensT1,kLineT1+1);
		}
		if ((offsetT0+lenT0 == lineEndT0)  &&  (kLineT0+1 < lenVecT0))
		{
			offsetLineT0 = lineEndT0;
			kLineT0 = s_first_nonzero(lenVecT0,vecLineLensT0,kLineT0+1);
		}
		if ((offsetT2+lenT2 == lineEndT2)  &&  (kLineT2+1 < lenVecT2))
		{
			offsetLineT2 = lineEndT2;
			kLineT2 = s_first_nonzero(lenVecT2,vecLineLensT2,kLineT2+1);
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:listLA"));

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len LA %d]"),(long)listLA.size());
	for (ListLineAssignmentsIterator it=listLA.begin(); (it != listLA.end()); it++)
	{
		const _line_assignment3 & la = (*it);
		la.dump(10);
	}
#endif
#endif

	//////////////////////////////////////////////////////////////////
	// next, compute the vertical alignment of the lines.  we do this
	// by trying to line up the longest NON-WHITE EQ nodes on a set of
	// lines.  we create a "binding" for each match-up.
	//
	// for now, we just concentrate on 3EQ nodes and get them vertically
	// aligned.
	//
	// TODO deal with sub-alignment of partial-EQ nodes.  especially
	// TODO partial-EQs-opposite-a-VOID.
	//////////////////////////////////////////////////////////////////

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecLB"));

	typedef std::vector<_line_bindings> VecLineBindings;

	VecLineBindings vecLB;

	long kLinePrevBindingT1 = -1;	// kLine of T1 in previous binding
	long kLinePrevBindingT0 = -1;	// kLine of T0 in previous binding
	long kLinePrevBindingT2 = -1;	// kLine of T2 in previous binding
	long lenOfPrevBinding = 0;		// length of previous binding
	
	for (ListLineAssignmentsIterator it=listLA.begin(); (it != listLA.end()); it++)
	{
		const _line_assignment3 & la = (*it);
		if (la.m_pSyncIL->isSameType(DE_ATTR_MRG_3EQ))
		{
			const wxChar * sz = strT1.wc_str() + la.m_pSyncIL->getNdx(PANEL_T1);
			if ((*sz != 0x00020)  &&  (*sz != 0x0009))
			{
				if (   (kLinePrevBindingT1 != la.m_kLineT1)
					&& (kLinePrevBindingT0 != la.m_kLineT0)
					&& (kLinePrevBindingT2 != la.m_kLineT2))
				{
					// start a new binding for this set of lines.
					//wxLogTrace(TRACE_DE_DUMP,_T("LB:pushing(%ld,%ld,%ld)"),la.m_kLineT1,la.m_kLineT0,la.m_kLineT2);
					vecLB.push_back( _line_bindings(la.m_kLineT1,la.m_kLineT0,la.m_kLineT2) );
					kLinePrevBindingT1 = la.m_kLineT1;
					kLinePrevBindingT0 = la.m_kLineT0;
					kLinePrevBindingT2 = la.m_kLineT2;
					lenOfPrevBinding = la.m_pSyncIL->getLen(PANEL_T1);
				}
				else if (   (   (la.m_kLineT1 == kLinePrevBindingT1)
							 || (la.m_kLineT0 == kLinePrevBindingT0)
							 || (la.m_kLineT2 == kLinePrevBindingT2))
						 && (la.m_pSyncIL->getLen(PANEL_T1) > lenOfPrevBinding))
				{
					// a longer/better match for the last binding, update it.
					//wxLogTrace(TRACE_DE_DUMP,_T("LB:updating from (%ld,%ld,%ld) to (%ld,%ld,%ld)"),
					//		   vecLB[vecLB.size()-1].getBinding(PANEL_T1),vecLB[vecLB.size()-1].getBinding(PANEL_T0),vecLB[vecLB.size()-1].getBinding(PANEL_T2),
					//		   la.m_kLineT1,la.m_kLineT0,la.m_kLineT2);
					vecLB[vecLB.size()-1].update(la.m_kLineT1,la.m_kLineT0,la.m_kLineT2);
					kLinePrevBindingT1 = la.m_kLineT1;
					kLinePrevBindingT0 = la.m_kLineT0;
					kLinePrevBindingT2 = la.m_kLineT2;
					lenOfPrevBinding = la.m_pSyncIL->getLen(PANEL_T1);
				}
			}
		}
	}

	// push an {end(),end(),end()} type reference to make sure we get the stuff
	// following the last EQ node in subsequent loops.
	vecLB.push_back( _line_bindings(pSyncLine->getLen(PANEL_T1),pSyncLine->getLen(PANEL_T0),pSyncLine->getLen(PANEL_T2)) );

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecLB"));

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len LB %ld]"),(long)vecLB.size());
	for (int k=0; k<(int)vecLB.size(); k++)
	{
		const _line_bindings & lb = vecLB[k];
		lb.dump(10);
	}
#endif
#endif
	
	//////////////////////////////////////////////////////////////////
	// build a set of index-maps to map from raw-line-number to aligned
	// line number.  and build a vector of line-oriented sync-nodes to
	// cut up the given sync-node.  initialize each line with an empty
	// intra-line sync-list.
	//////////////////////////////////////////////////////////////////

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecSyncs"));

	typedef std::vector<de_sync *> VecSyncs;
	VecSyncs vecSyncs;

	typedef std::vector<long> VecLineMap;
	VecLineMap vecLineMapT1, vecLineMapT0, vecLineMapT2;
	
	long kRow = 0;
	long kRowMapT1 = 0;
	long kRowMapT0 = 0;
	long kRowMapT2 = 0;

	long nrBindings = (long)vecLB.size();
	for (long kBinding=0; (kBinding<nrBindings); kBinding++)
	{
		const _line_bindings & lb = vecLB[kBinding];
		while (   (kRowMapT1 < lb.getBinding(PANEL_T1))
			   || (kRowMapT0 < lb.getBinding(PANEL_T0))
			   || (kRowMapT2 < lb.getBinding(PANEL_T2)))
		{
			vecSyncs.push_back( new de_sync(PANEL_T1,pSyncLine->getNdx(PANEL_T1)+kRowMapT1,((kRowMapT1 < lb.getBinding(PANEL_T1)) ? 1 : 0),
											PANEL_T0,pSyncLine->getNdx(PANEL_T0)+kRowMapT0,((kRowMapT0 < lb.getBinding(PANEL_T0)) ? 1 : 0),
											PANEL_T2,pSyncLine->getNdx(PANEL_T2)+kRowMapT2,((kRowMapT2 < lb.getBinding(PANEL_T2)) ? 1 : 0),
											pSyncLine->getAttr() | DE_ATTR_ML_MEMBER) );
			vecSyncs[kRow]->appendIntraLineSyncList( new de_sync_list() );
			vecSyncs[kRow]->getIntraLineSyncList(0)->m_ChangeKindIntraLine = pSyncLine->getAttr();
						
			if (kRowMapT1 < lb.getBinding(PANEL_T1))
			{
				vecLineMapT1.push_back(kRow);
				kRowMapT1++;
			}
			if (kRowMapT0 < lb.getBinding(PANEL_T0))
			{
				vecLineMapT0.push_back(kRow);
				kRowMapT0++;
			}
			if (kRowMapT2 < lb.getBinding(PANEL_T2))
			{
				vecLineMapT2.push_back(kRow);
				kRowMapT2++;
			}
			kRow++;
		}
	}

	// if a side is empty, we won't have anything in the vector
	// for that side (and we shouldn't).  but this causes vec[0]
	// to assert when we try to populate the rows from la.m_kLine*.

	if (kRowMapT1 == 0)	vecLineMapT1.push_back(-1);
	if (kRowMapT0 == 0)	vecLineMapT0.push_back(-1);
	if (kRowMapT2 == 0)	vecLineMapT2.push_back(-1);
	
	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:vecSyncs"));

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len VecLineMapT1 %ld]"),(long)vecLineMapT1.size());
	for (int k=0; k<(int)vecLineMapT1.size(); k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t[%ld]"),vecLineMapT1[k]);
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len VecLineMapT0 %ld]"),(long)vecLineMapT0.size());
	for (int k=0; k<(int)vecLineMapT0.size(); k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t[%ld]"),vecLineMapT0[k]);
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [len VecLineMapT2 %ld]"),(long)vecLineMapT2.size());
	for (int k=0; k<(int)vecLineMapT2.size(); k++)
		wxLogTrace(TRACE_DE_DUMP,_T("\t[%ld]"),vecLineMapT2[k]);
#endif
#endif

	//////////////////////////////////////////////////////////////////
	// now, walk the multi-line-intra-line sync-list (using the listLA
	// that we built) and build single-line intra-line lists in each of
	// the line-oriented sync-nodes that we created in the vecSyncs.

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:apply"));

#define APPEND(_row_,_psrel_)													\
	Statement(	wxASSERT_MSG( ((_row_) != -1), _T("Coding Error") );			\
				de_sync * pSrow = vecSyncs[(_row_)];							\
				de_sync_list * pSLil  = pSrow->getIntraLineSyncList(0);			\
				pSLil->_append((_psrel_));										\
				de_attr attr = (_psrel_)->getAttr();							\
				pSLil->m_ChangeKindIntraLine &= (attr & DE_ATTR__TYPE_MASK);	\
				if (!DE_ATTR__IS_TYPE(attr,DE_ATTR_MRG_3EQ))					\
				{																\
					pSLil->m_cChangesIntraLine++;								\
					if (!DE_ATTR__IS_SET(attr,DE_ATTR_UNIMPORTANT))				\
						pSLil->m_cImportantChangesIntraLine++;					\
				}																)

	for (ListLineAssignmentsIterator it=listLA.begin(); (it != listLA.end()); it++)
	{
		const _line_assignment3 & la = (*it);
		const de_sync * pSync = la.m_pSyncIL;

		long lenT1 = (long)pSync->getLen(PANEL_T1);
		long lenT0 = (long)pSync->getLen(PANEL_T0);
		long lenT2 = (long)pSync->getLen(PANEL_T2);

		wxASSERT_MSG( (la.m_kLineT1 < (long)vecLineMapT1.size()), _T("Coding Error") );
		wxASSERT_MSG( (la.m_kLineT0 < (long)vecLineMapT0.size()), _T("Coding Error") );
		wxASSERT_MSG( (la.m_kLineT2 < (long)vecLineMapT2.size()), _T("Coding Error") );

		long kRowT1 = vecLineMapT1[la.m_kLineT1];
		long kRowT0 = vecLineMapT0[la.m_kLineT0];
		long kRowT2 = vecLineMapT2[la.m_kLineT2];

		if ((kRowT1 == kRowT0) && (kRowT1 == kRowT2))
		{
			// a span completely contained within the current line on all panels.
			// since everything is on the same line, we can use a normal sync-node.

			de_sync * pSyncRelative102 = new de_sync(PANEL_T1, la.m_offsetT1, lenT1,
													 PANEL_T0, la.m_offsetT0, lenT0,
													 PANEL_T2, la.m_offsetT2, lenT2,
													 pSync->getAttr());
			APPEND(kRowT1,pSyncRelative102);
		}
		else if (kRowT1 == kRowT0)
		{
			// t1 and t0 are on the same line, t2 is elsewhere.  so create partial nodes.

			bool bHave10 = ((lenT1 > 0) || (lenT0 > 0));
			bool bHave2  = (lenT2 > 0);

			if (bHave10)
			{
				de_sync_list * p = vecSyncs[kRowT1]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset2 = (pt ? pt->getNdx(PANEL_T2)+pt->getLen(PANEL_T2) : 0);
				
				de_sync * pSyncRelativeT10 = new de_sync(PANEL_T1, la.m_offsetT1, lenT1,
														 PANEL_T0, la.m_offsetT0, lenT0,
														 PANEL_T2, offset2, 0,
														 pSync->getAttr() | ((bHave2) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT1,pSyncRelativeT10);
			}
			if (bHave2)
			{
				de_sync_list * p = vecSyncs[kRowT2]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset1 = (pt ? pt->getNdx(PANEL_T1)+pt->getLen(PANEL_T1) : 0);
				long offset0 = (pt ? pt->getNdx(PANEL_T0)+pt->getLen(PANEL_T0) : 0);

				de_sync * pSyncRelativeT2 = new de_sync(PANEL_T1, offset1, 0,
														PANEL_T0, offset0, 0,
														PANEL_T2, la.m_offsetT2, lenT2,
														pSync->getAttr() | ((bHave10) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT2,pSyncRelativeT2);
			}
		}
		else if (kRowT1 == kRowT2)
		{
			// t1 and t2 are on the same line, t0 is elsewhere.  so create partial nodes.

			bool bHave12 = ((lenT1 > 0) || (lenT2 > 0));
			bool bHave0  = (lenT0 > 0);

			if (bHave12)
			{
				de_sync_list * p = vecSyncs[kRowT1]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset0 = (pt ? pt->getNdx(PANEL_T0)+pt->getLen(PANEL_T0) : 0);
				
				de_sync * pSyncRelativeT12 = new de_sync(PANEL_T1, la.m_offsetT1, lenT1,
														 PANEL_T0, offset0, 0,
														 PANEL_T2, la.m_offsetT2, lenT2,
														 pSync->getAttr() | ((bHave0) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT1,pSyncRelativeT12);
			}
			if (bHave0)
			{
				de_sync_list * p = vecSyncs[kRowT0]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset1 = (pt ? pt->getNdx(PANEL_T1)+pt->getLen(PANEL_T1) : 0);
				long offset2 = (pt ? pt->getNdx(PANEL_T2)+pt->getLen(PANEL_T2) : 0);

				de_sync * pSyncRelativeT0 = new de_sync(PANEL_T1, offset1, 0,
														PANEL_T0, la.m_offsetT0, lenT0,
														PANEL_T2, offset2, 0,
														pSync->getAttr() | ((bHave12) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT0,pSyncRelativeT0);
			}
		}
		else if (kRowT0 == kRowT2)
		{
			// t0 and t2 are on the same line, t1 is elsewhere.  so create partial nodes.

			bool bHave02 = ((lenT0 > 0) || (lenT2 > 0));
			bool bHave1  = (lenT1 > 0);

			if (bHave02)
			{
				de_sync_list * p = vecSyncs[kRowT0]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset1 = (pt ? pt->getNdx(PANEL_T1)+pt->getLen(PANEL_T1) : 0);
				
				de_sync * pSyncRelativeT02 = new de_sync(PANEL_T1, offset1, 0,
														 PANEL_T0, la.m_offsetT0, lenT0,
														 PANEL_T2, la.m_offsetT2, lenT2,
														 pSync->getAttr() | ((bHave1) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT0,pSyncRelativeT02);
			}
			if (bHave1)
			{
				de_sync_list * p = vecSyncs[kRowT1]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset0 = (pt ? pt->getNdx(PANEL_T0)+pt->getLen(PANEL_T0) : 0);
				long offset2 = (pt ? pt->getNdx(PANEL_T2)+pt->getLen(PANEL_T2) : 0);

				de_sync * pSyncRelativeT1 = new de_sync(PANEL_T1, la.m_offsetT1, lenT1,
														PANEL_T0, offset0, 0,
														PANEL_T2, offset2, 0,
														pSync->getAttr() | ((bHave02) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT1,pSyncRelativeT1);
			}
		}
		else
		{
			// all sides happen to be on different lines, so we create partial sync-nodes.
			// we allow for any side to be VOID.
				
			if (lenT1 > 0)
			{
				de_sync_list * p = vecSyncs[kRowT1]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset0 = (pt ? pt->getNdx(PANEL_T0)+pt->getLen(PANEL_T0) : 0);
				long offset2 = (pt ? pt->getNdx(PANEL_T2)+pt->getLen(PANEL_T2) : 0);

				bool bHave02 = ((lenT0 > 0) || (lenT2 > 0));

				de_sync * pSyncRelativeT1 = new de_sync(PANEL_T1, la.m_offsetT1, lenT1,
														PANEL_T0, offset0, 0,
														PANEL_T2, offset2, 0,
														pSync->getAttr() | ((bHave02) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT1,pSyncRelativeT1);
			}
			if (lenT0 > 0)
			{
				de_sync_list * p = vecSyncs[kRowT0]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset1 = (pt ? pt->getNdx(PANEL_T1)+pt->getLen(PANEL_T1) : 0);
				long offset2 = (pt ? pt->getNdx(PANEL_T2)+pt->getLen(PANEL_T2) : 0);

				bool bHave12 = ((lenT1 > 0) || (lenT2 > 0));

				de_sync * pSyncRelativeT0 = new de_sync(PANEL_T1, offset1, 0,
														PANEL_T0, la.m_offsetT0, lenT0,
														PANEL_T2, offset2, 0,
														pSync->getAttr() | ((bHave12) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT0,pSyncRelativeT0);
			}
			if (lenT2 > 0)
			{
				de_sync_list * p = vecSyncs[kRowT2]->getIntraLineSyncList(0);
				de_sync * pt = p->getTail();
				long offset1 = (pt ? pt->getNdx(PANEL_T1)+pt->getLen(PANEL_T1) : 0);
				long offset0 = (pt ? pt->getNdx(PANEL_T0)+pt->getLen(PANEL_T0) : 0);

				bool bHave10 = ((lenT1 > 0) || (lenT0 > 0));

				de_sync * pSyncRelativeT2 = new de_sync(PANEL_T1, offset1, 0,
														PANEL_T0, offset0, 0,
														PANEL_T2, la.m_offsetT2, lenT2,
														pSync->getAttr() | ((bHave10) ? DE_ATTR_PARTIAL : 0));
				APPEND(kRowT2,pSyncRelativeT2);
			}
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:apply"));
	
	//////////////////////////////////////////////////////////////////
	// since each line was advanced at its own rate, we couldn't add
	// the END-sync-nodes to the individual intra-line lists as we were
	// building the lists.  so one more pass.
	//////////////////////////////////////////////////////////////////

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("extractLines: [rows in result %ld]"),(long)vecSyncs.size());
#endif
#endif

	//UTIL_PERF_START_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:addEOLs"));

	long nrRows = (long)vecSyncs.size();
	for (long kRow2=0; (kRow2<nrRows); kRow2++)
	{
		de_sync_list * pSyncListIntraLineK = vecSyncs[kRow2]->getIntraLineSyncList(0);
		de_sync * pSyncLast = pSyncListIntraLineK->getTail();

		if (pSyncLast)		// we put something on this line
		{
			pSyncListIntraLineK->_append( new de_sync(PANEL_T1, pSyncLast->getNdx(PANEL_T1)+pSyncLast->getLen(PANEL_T1), 0,
													  PANEL_T0, pSyncLast->getNdx(PANEL_T0)+pSyncLast->getLen(PANEL_T0), 0,
													  PANEL_T2, pSyncLast->getNdx(PANEL_T2)+pSyncLast->getLen(PANEL_T2), 0,
													  DE_ATTR_EOF));

			pSyncListIntraLineK->normalize_ChangeKindIntraLine3();
		}
		else
		{
			// we didn't put anything on this line.  this should be a blank line
			// (when EOL chars are excluded from the analysis) opposite a VOID.
			// we have either 1 blank line opposite 2 voids or 2 blank lines opposite 1 void.

			const de_line * pDeLineT1 = ((vecSyncs[kRow2]->getLen(PANEL_T1) == 1) ? getDeLineFromNdx(kSync,PANEL_T1,vecSyncs[kRow2]->getNdx(PANEL_T1)) : NULL);
			const de_line * pDeLineT0 = ((vecSyncs[kRow2]->getLen(PANEL_T0) == 1) ? getDeLineFromNdx(kSync,PANEL_T0,vecSyncs[kRow2]->getNdx(PANEL_T0)) : NULL);
			const de_line * pDeLineT2 = ((vecSyncs[kRow2]->getLen(PANEL_T2) == 1) ? getDeLineFromNdx(kSync,PANEL_T2,vecSyncs[kRow2]->getNdx(PANEL_T2)) : NULL);

			wxASSERT_MSG( ( pDeLineT1 ||  pDeLineT0 ||  pDeLineT2), _T("Coding Error") );		// at least one cannot be VOID.

			pSyncListIntraLineK->_append( new de_sync(PANEL_T1, 0,0,
													  PANEL_T0, 0,0,
													  PANEL_T2, 0,0,
													  DE_ATTR_EOF));

			int flags = 0x0;
			if (pDeLineT1)	flags |= 0x4;
			if (pDeLineT0)	flags |= 0x2;
			if (pDeLineT2)	flags |= 0x1;
			wxASSERT_MSG( (flags != 0), _T("Coding Error") );

			static de_attr sa[8] = { DE_ATTR_MRG_3EQ,			// 000: all 3 VOID -- cannot happen
									 DE_ATTR_MRG_T1T0EQ,		// 001: T1,T0 VOID and T2 blank
									 DE_ATTR_MRG_T1T2EQ,		// 010: T1,T2 VOID and T0 blank
									 DE_ATTR_MRG_T0T2EQ,		// 011: T1 VOID and T0,T2 blank
									 DE_ATTR_MRG_T0T2EQ,		// 100: T0,T2 VOID and T1 blank
									 DE_ATTR_MRG_T1T2EQ,		// 101: T0 VOID and T1,T2 blank
									 DE_ATTR_MRG_T1T0EQ,		// 110: T2 VOID and T1,T0 blank
									 DE_ATTR_MRG_3EQ    };		// 111: all 3 blank

			de_attr attrIL = sa[flags];

			pSyncListIntraLineK->m_ChangeKindIntraLine = de_attr_union_attr(pSyncListIntraLineK->m_ChangeKindIntraLine,attrIL);

			pSyncListIntraLineK->m_cChangesIntraLine++;

			// the importance of the line is determined by the context of the EOL.

			bool bImportant = false;
			if (pDeLineT1)
			{
				rs_context_attrs rsAttrs = ( (pDeLineT1->getContextEOL())
											 ? pDeLineT1->getContextEOL()->getContextAttrs()
											 : defaultContextAttrs);
				bImportant |= (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs));
			}
			if (!bImportant && pDeLineT0)
			{
				rs_context_attrs rsAttrs = ( (pDeLineT0->getContextEOL())
											 ? pDeLineT0->getContextEOL()->getContextAttrs()
											 : defaultContextAttrs);
				bImportant |= (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs));
			}
			if (!bImportant && pDeLineT2)
			{
				rs_context_attrs rsAttrs = ( (pDeLineT2->getContextEOL())
											 ? pDeLineT2->getContextEOL()->getContextAttrs()
											 : defaultContextAttrs);
				bImportant |= (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs));
			}
			if (bImportant)
				pSyncListIntraLineK->m_cImportantChangesIntraLine++;
		}
		
#if 0
#ifdef DEBUG
		vecSyncs[kRow]->dump(10);
		vecSyncs[kRow]->getIntraLineSyncList(0)->dump(30);
#endif
#endif
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_extractLinesFromMultiLineIntraLine2:addEOLs"));

	//////////////////////////////////////////////////////////////////
	// now, get destructive -- replace the given line-oriented sync-node
	// (representing multiple lines) with a series of single-line sync-nodes.
	//////////////////////////////////////////////////////////////////

	nrRows = (long)vecSyncs.size();
	for (long kRow2=nrRows-1; (kRow2>=0); kRow2--)
		m_sync_list[kSync]._insert_after(pSyncLine,vecSyncs[kRow2]);

	m_sync_list[kSync]._delete_sync(pSyncLine);

	// return first line so that caller can update loop variables.

	UTIL_PERF_STOP_CLOCK(sszKey_extractLinesFromMultiLineIntraLine3);

	return vecSyncs[0];
}

//////////////////////////////////////////////////////////////////

void de_de::_do_intraline3(long kSync)
{
	UTIL_PERF_START_CLOCK(sszKey_do_intraline3);

	// do intra-line diff for each sync-node in the documents in a 3-way diff.

	int multilineDetailLevel = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LEVEL);
	int multilineDetailLimit = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LIMIT);

	de_sync * pSync = m_sync_list[kSync].getHead();
	while (pSync && !pSync->isEND())
	{
		// we set pSyncNext here (before we do all of the following work)
		// to be the node following the current one -- we need to take care
		// of the analysis between this node and upto the current next one.
		// splits and changes can happen to the current node in the list as
		// we work on this node.

		de_sync * pSyncNext = pSync->getNext();

		// skip over MARKS and 3EQs

		if (!pSync->isSameKind(DE_ATTR__MRG_KIND) || pSync->isSameType(DE_ATTR_MRG_3EQ))
		{
			pSync = pSyncNext;
			continue;
		}

		// compute the intra-line results for each row within pSync.
		//
		// the "non-simple" way tries to aggregate all lines within the change
		// and do the comparison -- this helps, for example, when the change
		// consists of inserting line breaks into the argument list of a function
		// call.  this is ***VERY*** expensive but the results are worth it.  this is
		// the "multi-line-intra-line" diff.
		//
		// the "simple" way does "single-line-intra-line" diff.  we use it when
		// we can because it's faster.

		int nrZero = 0;
		int nrOne  = 0;
		for (int kPanel=0; kPanel<3; kPanel++)
		{
			long len = pSync->getLen((PanelIndex)kPanel);
			if (len == 0)
				nrZero++;
			else if (len == 1)
				nrOne++;
		}
		if ((nrZero >= 2) || (nrOne == 3) || (nrZero+nrOne == 3))
		{
			_do_simple_intraline3_sync(kSync,pSync);		// use simple for trivial cases
		}
		else
		{
			switch (multilineDetailLevel)
			{
			default:
			case DE_MULTILINE_DETAIL_LEVEL__NONE:			// multi-line is completely disabled
				_do_simple_intraline3_sync(kSync,pSync);
				break;

			case DE_MULTILINE_DETAIL_LEVEL__NEQS:			// multi-line set to proper NEQS only
				if (   pSync->isNonExactEQ()
					|| pSync->isSmoothed())
				{
					_do_simple_intraline3_sync(kSync,pSync);
					break;
				}
				else if (   (multilineDetailLimit > 0)
						 && (   (pSync->getLen(PANEL_T0) > multilineDetailLimit)
							 || (pSync->getLen(PANEL_T1) > multilineDetailLimit)
							 || (pSync->getLen(PANEL_T2) > multilineDetailLimit)))
				{
					_do_simple_intraline3_sync(kSync,pSync);
					break;
				}
				//FALLTHRU INTENDED

			case DE_MULTILINE_DETAIL_LEVEL__FULL:			// run full complex intraline
				pSync = _do_complex_intraline3_sync(kSync,pSync);		// current node is replaced
				break;
			}
		}

		// set or clear the unimportant bit in this sync
		// node based upon the rows within it.  we may
		// have to split this node one or more times.
			
		while (pSync != pSyncNext)
		{
			if (!pSync->isSameType(DE_ATTR_MRG_3EQ))
			{
				long lenMax = pSync->getMaxLen();
				wxASSERT_MSG( (lenMax > 0), _T("Coding Error") );
				if (lenMax > 0)
				{
					bool bUnimportant0 = (pSync->getIntraLineSyncList(0)->getSumImportantChangesIntraLine() == 0);
					de_attr attr0 = (pSync->getIntraLineSyncList(0)->getSumChangeKindIntraLine() & DE_ATTR__TYPE_MASK);

					for (long kRow=1; (kRow < lenMax); kRow++)
					{
						bool bUnimportantK = (pSync->getIntraLineSyncList(kRow)->getSumImportantChangesIntraLine() == 0);
						de_attr attrK = (pSync->getIntraLineSyncList(kRow)->getSumChangeKindIntraLine() & DE_ATTR__TYPE_MASK);

						if ((bUnimportantK != bUnimportant0)  ||  (attrK != attr0))
						{
							m_sync_list[kSync].split(pSync,kRow);
							break;
						}
					}

					pSync->updateAttrType(attr0);
					pSync->setUnimportant(bUnimportant0);
				}
			}
			
			pSync = pSync->getNext();
		}
	}

	UTIL_PERF_STOP_CLOCK(sszKey_do_intraline3);
}

void de_de::_do_simple_intraline3_sync(long kSync, de_sync * pSync)
{
	// do intra-line diff for each sync-node in the documents in a 3-way diff.
	
	long ndxT1  = pSync->getNdx(PANEL_T1);		// starting ndx in vecLingCmp[T1] of this sync node
	long ndxT0  = pSync->getNdx(PANEL_T0);		// starting ndx in vecLingCmp[T0] of this sync node
	long ndxT2  = pSync->getNdx(PANEL_T2);		// starting ndx in vecLingCmp[T2] of this sync node

	long lenT1  = pSync->getLen(PANEL_T1);
	long lenT0  = pSync->getLen(PANEL_T0);
	long lenT2  = pSync->getLen(PANEL_T2);
		
	long lenMax = MyMax(lenT1,MyMax(lenT0,lenT2));

	if (lenMax <= 0)							// shouldn't happen
		return;

	UTIL_PERF_START_CLOCK(sszKey_do_simple_intraline3_sync);

	// this line-oriented sync-node represents "lenMax" lines in the documents.
	// some lines may have content in both panels; other lines will only have
	// content in one (associated with voids).  allocate and reserve space in
	// a vector to store a sync-list for each line.
		
	pSync->createIntraLineSyncListVector(lenMax);
			
	for (long jRow=0; jRow<lenMax; jRow++)
	{
		// create an intra-line sync-list for each line in this (line-oriented) sync-node.
		// 
		// note: pLineTx will be null when we have a void on that side.

		const de_line * pLineT1 = ((jRow < lenT1) ? (*_lookupVecLineCmp(kSync,PANEL_T1))[ndxT1 + jRow] : NULL);
		const de_line * pLineT0 = ((jRow < lenT0) ? (*_lookupVecLineCmp(kSync,PANEL_T0))[ndxT0 + jRow] : NULL);
		const de_line * pLineT2 = ((jRow < lenT2) ? (*_lookupVecLineCmp(kSync,PANEL_T2))[ndxT2 + jRow] : NULL);

		const rs_ruleset * pRS  = m_pFsFs->getRuleSet();

		de_sync_list * pSyncListIntraLine = de_sync_list::createIntraLineSyncList3((pSync->getAttr() & DE_ATTR__TYPE_MASK),
																				   pLineT1,pLineT0,pLineT2,
																				   pRS);

		pSync->appendIntraLineSyncList(pSyncListIntraLine);	// sync node now owns the sync list we just created.
	}

	UTIL_PERF_STOP_CLOCK(sszKey_do_simple_intraline3_sync);
}

//////////////////////////////////////////////////////////////////

void de_de::_do_build_multiline_string(long kSync,PanelIndex kPanel,
									   long ndxStart, long nrLines,
									   bool bMatchEOLs,
									   wxString * pStr,
									   const de_line ** ppDeLineLast,
									   TVecLongs * pVecLens,
									   TVecBools * pVecEols)
{
	// build multi-line string for multi-line-intra-line analysis.
	// populate pStr, *ppDeLineLast, pVecLens, pVecEols.

	*ppDeLineLast = NULL;
	pVecLens->clear();
	pVecLens->reserve(nrLines);
	pVecEols->clear();
	pVecEols->reserve(nrLines);

	for (long jRow=0; jRow<nrLines; jRow++)
	{
		const de_line * pLine = getDeLineFromNdx(kSync,kPanel,ndxStart+jRow);

		if (pLine)
			*ppDeLineLast = pLine;

		long len = 0;
		bool bIncludedEOL = false;

		if (pLine)
		{
			(*pStr) += pLine->getStrLine();
			len = (long)pLine->getStrLine().Len();
			if (bMatchEOLs)			// see de_css_src_string::de_css_src_string() 
			{
				(*pStr) += pLine->getStrEOL();
				len += (long)pLine->getStrEOL().Len();
				bIncludedEOL = true;
			}
		}
		pVecLens->push_back(len);
		pVecEols->push_back(bIncludedEOL);
	}
}

