// de_de__build_vecline.cpp -- diff engine -- routines related to
// building the vecLineCmp -- the vector of lines that we will compare
// when we run the diff computation.
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

bool de_de::_build_vector(long kSync, PanelIndex kPanel)
{
	// populate m_vecLineCmp[kSync][kPanel] with info from the corresponding layout/document.
	// our vector will only contain the lines that should be used in the comparison.
	// that is, our ruleset may indicate that some lines be omitted -- such as n page
	// header lines following a FF.
	//
	// along the way, we need to scan the document and assign RuleSet contexts to
	// each line and spans within the line.

	m_bVecValid[kSync][kPanel] = false;

	fl_fl * pFlFl = getLayout(kSync,kPanel);
	if (!pFlFl)
		return false;

	//UTIL_PERF_START_CLOCK(_T("de_de::_build_vector"));

	pFlFl->format();	// force layout to update format (row/col placement of all content)

	const rs_ruleset * pRS  = m_pFsFs->getRuleSet();

	rs_context_attrs attrsMatchStrip = pRS->getMatchStripAttrs();
	bool bGlobalRespectEOL   = RS_ATTRS_RespectEOL(attrsMatchStrip);
	bool bGlobalRespectWhite = RS_ATTRS_RespectWhite(attrsMatchStrip);
	bool bGlobalRespectCase  = RS_ATTRS_RespectCase(attrsMatchStrip);
	bool bGlobalTabIsWhite   = RS_ATTRS_TabIsWhite(attrsMatchStrip);

	const rs_context * pCTX = NULL;

	TVector_LineCmp	* pVecLineCmp = _lookupVecLineCmp(kSync,kPanel);
 
	pVecLineCmp->clear();
	pVecLineCmp->reserve( pFlFl->getFormattedLineNrs() );	// pre-allocate expected amount to avoid realloc's
	_setSumOmitted(kSync,kPanel,0);

	int nrLinesToSkip = 0;
	for (fl_line * pFlLine=pFlFl->getFirstLine(); (pFlLine); pFlLine=pFlLine->getNext())
	{
		// since the layout (fl_fl) and the lines (fl_line) are shared by all views
		// onto the document, we cannot have private (per-view, per-diff, per-etc)
		// data on the lines.  so we use the property-slot on the line to store any
		// private data we need (in a de_line).  [this de_line will get deleted
		// when we get the callback notification that the fl_line is being deleted.]

		de_line * pDeLine = pFlLine->getSlotValue( *_lookupSlot(kSync,kPanel) );
		if (!pDeLine)
		{
			pDeLine = new de_line(pFlLine);
			pFlLine->setSlotValue( *_lookupSlot(kSync,kPanel), pDeLine);
		}

		wxASSERT_MSG( (pDeLine->getFlLine() == pFlLine), _T("Coding Error") );

		// build a single string containing the content for this line so that
		// we can easily scan it.  then identify spans based upon the context.
		// (bCachedStringsValid will be returned TRUE if this line didn't change
		// from the last time we were called.)

		bool bCachedStringsValid;
		pCTX = pDeLine->setStringsAndComputeContext(pRS,pCTX,m_bCacheRSChanged,&bCachedStringsValid);

		//pDeLine->dump(5);
		
		// determine if line needs to be in diff comparison.  that is, see if we
		// should omit the line.

		if (nrLinesToSkip == 0)											// if not continuing a previous omit block
			nrLinesToSkip = m_pFsFs->getRuleSet()->testLOmit(pDeLine->getStrLine().wc_str());	// see if we need to start a new skip.

		if (nrLinesToSkip > 0)
		{
			// line will be excluded from the diff computation.
			// leave it out of the vector. (m_vecLineCmp)
#if 1 // TODO
			if (pDeLine->hasMark(kSync))
			{
				// TODO ack! decide what to do here.  the line is not
				// TODO going to be included in vecLineCmp, but it needs
				// TODO to be for the chunking/partitioning to work.

				de_mark * pDeMark = pDeLine->getMark(kSync);
				MY_ASSERT( (pDeMark->getDeLine(kPanel)==pDeLine) );

				//wxASSERT_MSG( (0), _T("Coding Error -- TODO") );
			}
#endif
			// since this line is not in the vector, the ndx is should be bogus.

			long ndxLineCmp = -1;
			pDeLine->setNdxLineCmp(ndxLineCmp);

			pDeLine->unsetUIDs();
			pDeLine->setOmitted(true);
			_incrSumOmitted(kSync,kPanel);
			nrLinesToSkip--;
		}
		else
		{
			// normal line to be compared.  compute its hash value (UID)
			// and insert it into our vector.
			//
			// use contents of the document as laid out on this line
			// to construct a unique hash -- unique in that two identical
			// (or equivalent) lines will hash to the same value.

			// if the match/strip context attrs say to respect the EOL's,
			// then we include the EOL chars in the UID hash (match).  if
			// not, then we don't include the EOL chars in the hash (strip).

			//////////////////////////////////////////////////////////////////
			// if the contents of the line did not change from the last run
			// of the diff-engine and the parameters of the ruleset have not
			// changed, then the hash value should not change either.  see if
			// we can reuse it.

			unsigned long				cacheUIDParamsTest  = 0x01;
			if (bGlobalRespectWhite)	cacheUIDParamsTest |= 0x02;
			if (bGlobalTabIsWhite)		cacheUIDParamsTest |= 0x04;
			if (bGlobalRespectCase)		cacheUIDParamsTest |= 0x08;
			if (bGlobalRespectEOL)		cacheUIDParamsTest |= 0x10;

			if ( (!bCachedStringsValid)  ||  (!pDeLine->getUIDValid())  ||  (cacheUIDParamsTest != pDeLine->getCachedUIDParams()))
			{
				size_t lenMax = pDeLine->getStrLine().length() + pDeLine->getStrEOL().length();

				wxString strTemp, strTempExact;
				strTemp.Alloc(lenMax);
				strTempExact.Alloc(lenMax);

				if (bGlobalRespectWhite)		// keep whitespace as is in line
					strTemp += pDeLine->getStrLine();
				else							// strip whitespace from line
				{
					if (bGlobalTabIsWhite)
					{
						for (const wxChar * p=pDeLine->getStrLine().wc_str(); (*p); p++)
							if ( (*p != 0x0020) && (*p != 0x0009) )
								strTemp += *p;
					}
					else
					{
						for (const wxChar * p=pDeLine->getStrLine().wc_str(); (*p); p++)
							if (*p != 0x0020)
								strTemp += *p;
					}
				}

				if (!bGlobalRespectCase)
					strTemp.MakeLower();

				// also compute an exact-hash for the entire line as is.

				strTempExact = pDeLine->getStrLine();

				if (bGlobalRespectEOL)
				{
					strTemp += pDeLine->getStrEOL();
					strTempExact += pDeLine->getStrEOL();
				}
				
				TSet_HashInsertResult r     = m_setHash.insert(strTemp);		// r is pair<iterator,bool>
				TSet_HashIterator     itUID = r.first;

				TSet_HashInsertResult rExact     = m_setHash.insert(strTempExact);		// r is pair<iterator,bool>
				TSet_HashIterator     itExactUID = rExact.first;

				pDeLine->setUIDs(itUID,itExactUID,cacheUIDParamsTest);
			}
			
			// remember what ndx this de_line has in vecLineCmp.
			// this lets the mark-related partitioning of the
			// vector (ala de_css_src_clipped) to run quickly. 

			long ndxLineCmp = (long)m_vecLineCmp[kSync][kPanel].size();
			pDeLine->setNdxLineCmp(ndxLineCmp);

			// insert line into vector

			pDeLine->setOmitted(false);
			m_vecLineCmp[kSync][kPanel].push_back( pDeLine );
		}
	}

	m_bVecValid[kSync][kPanel] = true;

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_build_vector"));

	return true;
}
