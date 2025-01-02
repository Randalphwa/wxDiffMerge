// de_de__insert_omitted.cpp -- diff engine -- routines related to
// inserting lines omitted from the diff computation back into the
// sync list before we build the display list.
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

void de_de::_insert_omitted_lines_into_sync_list(long kSync)
{
	//////////////////////////////////////////////////////////////////
	// if the current ruleset has settings to mark lines as OMITTED,
	// then run thru the sync list we just constructed and split
	// nodes that span an omitted line.  we also may have to insert
	// them between nodes.
	//
	// we insert special "omitted" sync nodes.
	//
	// note: because sync nodes only refer to lines via an index into
	// note: m_vecLineCmp, these special "omitted" sync nodes will not
	// note: have an index (because omitted lines are not in the vector).
	// note: so we have a sync node sub-class to handle this case and
	// note: keep us (me) from getting confused.
	//
	// we assume that the sync list had been loaded and that it represents
	// contiguous coverage of all non-omitted lines.

	if (_totalOmitted(kSync) == 0)
		return;

	// with regards to omitted-lines adjacent to changed lines:
	// consider the following case:
	//
	//    T0       T1
	//    --       --
	//    xyz      xyz
	//    ajb      a
	//    .        AN-OMITTED-LINE
	//    .        kb
	//    zyx      zyx
	//
	// we have a 1-line vs 2-line change (with an omitted line in the
	// middle of it).
	//
	// case 1: when the detail-level is set to by-char (causing the
	//         multi-line-intra-line code to be used), the 'a' matches
	//         the 'a' and the 'b' matches the 'b'.  and the 'j' and 'k'
	//         are changes (marked partial).  if the user clicks on the
	//         'kb' line, we should highlight the entire change -- all
	//         the way back to the line containing the 'a' -- as if the
	//         omitted line weren't there -- because that is what we used
	//         when we did the complex intra-line analysis.  and so, the
	//         omitted line needs to be included in the patch -- visually.
	//
	// case 2: when the detail-level is set to line-only, it's kind of
	//         arbitrary whether the user sees 2 changes and an omitted
	//         line or if they see one change that includes an omitted line.
	//
	// case 3: when detail-level is by-char and we have a simple intra-line
	//         case (say that 'ajb' in T0 was a void instead), it is again
	//         kind of arbitrary whether the omitted line is highlighted as
	//         part of one patch or whether the user sees 2 patches.
	//
	// it is important to include the omitted line in the first case
	// *BECAUSE* the analyzed content crosses multiple lines and there are
	// portions of it BEFORE and AFTER the omitted line.  but the other cases
	// are arbitrary.
	//
	// let's go ahead and do it for all cases so that the user sees it
	// somewhat consistently.  THIS MAY CAUSE PROBLEMS FOR THE USER, if the
	// omitted text is something that they don't want to apply. 
	//
	// NOTE: ML_MEMBER will only be set when a change came thru the complex
	// NOTE: multi-line-intra-line code -- which will NOT happen when the 
	// NOTE: detail-level is set to line-only *OR* when we have a simple
	// NOTE: case and were able to use _do_simple_intralineN_sync().

//	wxLogTrace(TRACE_DE_DUMP,_T("================================================================"));
//	wxLogTrace(TRACE_DE_DUMP,_T("INSERT_OMITTED:"));
//	m_sync_list[kSync].dump(10);
	
	bool bMerge = (getLayout(kSync, PANEL_T2  ) != NULL);	// 3-way merge (as opposed to 2-way diff)
	int nrPanels = ((bMerge) ? 3 : 2);

	long lenOmitted[__NR_TOP_PANELS__];
	de_line * pDeLinesOmitted[__NR_TOP_PANELS__];
	bool bHaveOmitted;

	TVector_LineCmp * pVecLineCmp[__NR_TOP_PANELS__];
	fl_slot slot[__NR_TOP_PANELS__];
	long lineNrLast[__NR_TOP_PANELS__];
	const fl_line * pFlLineLast[__NR_TOP_PANELS__];

	for (int kPanel=PANEL_T0; (kPanel<nrPanels); kPanel++)
	{
		pVecLineCmp[kPanel] = _lookupVecLineCmp(kSync,(PanelIndex)kPanel);
		slot[kPanel]        = (* _lookupSlot(kSync,(PanelIndex)kPanel));
		lineNrLast[kPanel]  = -1;
		pFlLineLast[kPanel] = NULL;
	}

	de_sync * pDeSync;

	for (pDeSync=m_sync_list[kSync].getHead(); (pDeSync && !pDeSync->isEND()); pDeSync=pDeSync->getNext())
	{
		// see if there were omitted lines immediately prior to the start of this sync node.

		memset(lenOmitted,     0,sizeof(lenOmitted));
		memset(pDeLinesOmitted,0,sizeof(pDeLinesOmitted));
		bHaveOmitted = false;

		if (pDeSync->isMARK())
		{
			// marks are a little special, the de_lines in the mark-sync-node refer to either the marked-line or the first
			// non-omitted lines following it; we need the actual de_lines referenced by the de_mark.

			for (int kPanel=PANEL_T0; (kPanel<nrPanels); kPanel++)
			{
				de_line * pDeLine = pDeSync->getMark()->getDeLine((PanelIndex)kPanel);
				const fl_line * pFlLine = pDeLine->getFlLine();

				long skip = pFlLine->getLineNr() - lineNrLast[kPanel] - 1;
				if (skip > 0)										// an omitted line immediately prior to the start of this sync node
				{
					// for marks, we cause lines leading up to --but not including-- the marked line
					// to be accounted for.  the marked line will be handled by the sync-node *after*
					// this one -- so that the marked line appears below the mark in the display.

					const fl_line * pFlLinePrev = pFlLine->getPrev();
					wxASSERT_MSG( (pFlLinePrev), _T("Coding Error") );
					wxASSERT_MSG( (pFlLinePrev->getSlotValue(slot[kPanel])->isOmitted()), _T("Coding Error") );

					pFlLineLast[kPanel] = pFlLinePrev;
					lineNrLast[kPanel] = pFlLinePrev->getLineNr();

					for (long s=1; (s<skip); s++)
					{
						pFlLinePrev = pFlLinePrev->getPrev();
						wxASSERT_MSG( (pFlLinePrev), _T("Coding Error") );
						wxASSERT_MSG( (pFlLinePrev->getSlotValue(slot[kPanel])->isOmitted()), _T("Coding Error") );
					}

					pDeLinesOmitted[kPanel] = pFlLinePrev->getSlotValue(slot[kPanel]);
					lenOmitted[kPanel] = skip;
					bHaveOmitted = true;
				}
			}
		}
		else
		{
			for (int kPanel=PANEL_T0; (kPanel<nrPanels); kPanel++)
			{
				if (pDeSync->getLen((PanelIndex)kPanel) > 0)
				{
					long ndx = pDeSync->getNdx((PanelIndex)kPanel);
					wxASSERT_MSG( (ndx != -1), _T("Coding Error") );
				
					de_line * pDeLine = (*(pVecLineCmp[kPanel]))[ndx];	// the first line referenced by this sync node on this panel
					const fl_line * pFlLine = pDeLine->getFlLine();

					long skip = pFlLine->getLineNr() - lineNrLast[kPanel] - 1;
					if (skip > 0)										// an omitted line immediately prior to the start of this sync node
					{
						const fl_line * pFlLinePrev = pFlLine->getPrev();
						wxASSERT_MSG( (pFlLinePrev), _T("Coding Error") );
						wxASSERT_MSG( (pFlLinePrev->getSlotValue(slot[kPanel])->isOmitted()), _T("Coding Error") );

						for (long s=1; (s<skip); s++)
						{
							pFlLinePrev = pFlLinePrev->getPrev();
							wxASSERT_MSG( (pFlLinePrev), _T("Coding Error") );
							wxASSERT_MSG( (pFlLinePrev->getSlotValue(slot[kPanel])->isOmitted()), _T("Coding Error") );
						}

						pDeLinesOmitted[kPanel] = pFlLinePrev->getSlotValue(slot[kPanel]);
						lenOmitted[kPanel] = skip;
						bHaveOmitted = true;

						wxASSERT_MSG( (pFlLinePrev->getPrev() == pFlLineLast[kPanel]), _T("Coding Error") );
					}

					lineNrLast[kPanel] = pFlLine->getLineNr();
					pFlLineLast[kPanel] = pFlLine;
				}
			}
		}
		
		if (bHaveOmitted)
		{
			// see if we have a ML_Member on either side of the place where we are about
			// to insert our new omitted node.

			bool bMLMember = (   pDeSync->isMLMember()
							  || pDeSync->isAnyChangeType()
							  || (   (pDeSync->getPrev())
								  && (   pDeSync->getPrev()->isMLMember()
									  || pDeSync->getPrev()->isAnyChangeType())));
			
			de_sync * pDeSyncNew;

			if (bMerge)
				pDeSyncNew = new de_sync_line_omitted(PANEL_T1,pDeLinesOmitted[PANEL_T1],lenOmitted[PANEL_T1],
													  PANEL_T0,pDeLinesOmitted[PANEL_T0],lenOmitted[PANEL_T0],
													  PANEL_T2,pDeLinesOmitted[PANEL_T2],lenOmitted[PANEL_T2],
													  bMLMember);
			else
				pDeSyncNew = new de_sync_line_omitted(PANEL_T1,pDeLinesOmitted[PANEL_T1],lenOmitted[PANEL_T1],
													  PANEL_T0,pDeLinesOmitted[PANEL_T0],lenOmitted[PANEL_T0],
													  bMLMember);

			m_sync_list[kSync]._insert_before(pDeSync,pDeSyncNew);
		}

		if (pDeSync->isMARK())
			continue;

		// see if there were any omitted lines between the individual rows within this sync node.
		// remember, each panel within this sync node may have a different length -- representing
		// the lines in a change.

		long lenMax = pDeSync->getMaxLen();
		wxASSERT_MSG( (lenMax > 0), _T("Coding Error") );	// harmless, but should not happen
		if (lenMax == 0)
			continue;

		long lineNrThisRow[__NR_TOP_PANELS__];
		const fl_line * pFlLineThisRow[__NR_TOP_PANELS__];
		bool bSplit = false;

		for (long kRow=1; (!bSplit && (kRow<lenMax)); kRow++)
		{
			for (int kPanel=PANEL_T0; (kPanel<nrPanels); kPanel++)
			{
				if (kRow < pDeSync->getLen((PanelIndex)kPanel))
				{
					long ndx = pDeSync->getNdx((PanelIndex)kPanel);
					wxASSERT_MSG( (ndx != -1), _T("Coding Error") );

					de_line * pDeLine = (*(pVecLineCmp[kPanel]))[ndx+kRow];	// the kth line referenced by this sync node on this panel
					const fl_line * pFlLine = pDeLine->getFlLine();

					long skip = pFlLine->getLineNr() - lineNrLast[kPanel] - 1;
					if (skip > 0)	// an omitted line immediately prior to the start of this line, split this node here
						bSplit = true;		// must let (for kPanel...) finish so that lastNrLine[] gets updated properly.

					lineNrThisRow[kPanel] = pFlLine->getLineNr();
					pFlLineThisRow[kPanel] = pFlLine;
				}
				else
				{
					lineNrThisRow[kPanel] = lineNrLast[kPanel];
					pFlLineThisRow[kPanel] = pFlLineLast[kPanel];
				}
			}
			
			// split this node here and let the main loop start over with the second half.
			// that is, the intra-line omission will now be caught by the top half of the
			// loop as happening immediately prior to the next node.
			//
			// if we split, we keep the lineNr's from the previous row, so that the gap/skip
			// will show up at the top of the loop.
			// if we're not splitting, remember the lineNr's of this row.

			if (bSplit)
				m_sync_list[kSync].split(pDeSync,kRow);
			else
				for (int kPanel=PANEL_T0; (kPanel<nrPanels); kPanel++)
				{
					lineNrLast[kPanel] = lineNrThisRow[kPanel];
					pFlLineLast[kPanel] = pFlLineThisRow[kPanel];
				}
		}
	}
	wxASSERT_MSG( (pDeSync), _T("Coding Error") );	// should have found EOF node rather than the end of the list.

	// we have handled inserting omitted-sync-nodes before and between existing
	// sync-nodes in the list (proper).  pDeSync now points to the EOF sync-node.
	// 
	// see if there are any omitted lines between the last (non-EOF) sync-node
	// and the end of the documents that need to be taken care of.

	memset(lenOmitted,     0,sizeof(lenOmitted));
	memset(pDeLinesOmitted,0,sizeof(pDeLinesOmitted));
	bHaveOmitted = false;
		
	if (m_sync_list[kSync].getHead()->isEND())
	{
		// screw case: the sync list is empty -- it just contains the EOF marker --
		// the documents are empty or we omitted the entire contents of the documents.

		for (int kPanel=PANEL_T0; (kPanel<nrPanels); kPanel++)
		{
			lenOmitted[kPanel] = getLayout(kSync,(PanelIndex)kPanel)->getFormattedLineNrs();
			if (lenOmitted[kPanel])
			{
				pDeLinesOmitted[kPanel] = getLayout(kSync,(PanelIndex)kPanel)->getFirstLine()->getSlotValue(slot[kPanel]);
				wxASSERT_MSG( (pDeLinesOmitted[kPanel]->isOmitted()), _T("Coding Error") );
				bHaveOmitted = true;
			}
		}
	}
	else
	{
		// we have a non-empty sync list. so pFlLineLast[] contains pointers to the
		// last fl_line referenced in each panel.

		for (int kPanel=PANEL_T0; (kPanel<nrPanels); kPanel++)
		{
			const fl_line * pFlLineFinal = getLayout(kSync,(PanelIndex)kPanel)->getLastLine();
			if (pFlLineFinal)
			{
				if (pFlLineLast[kPanel])
					lenOmitted[kPanel] = pFlLineFinal->getLineNr() - pFlLineLast[kPanel]->getLineNr();
				else
					lenOmitted[kPanel] = pFlLineFinal->getLineNr() + 1;

				if (lenOmitted[kPanel])
				{
					const fl_line * pFlLineFirst;
					if (pFlLineLast[kPanel])
						pFlLineFirst = pFlLineLast[kPanel]->getNext();
					else
						pFlLineFirst = getLayout(kSync,(PanelIndex)kPanel)->getFirstLine();
					wxASSERT_MSG( (pFlLineFirst), _T("Coding Error") );

					pDeLinesOmitted[kPanel] = pFlLineFirst->getSlotValue(slot[kPanel]);
					wxASSERT_MSG( (pDeLinesOmitted[kPanel]->isOmitted()), _T("Coding Error") );
					bHaveOmitted = true;
				}
			}
		}
	}
	
	if (bHaveOmitted)
	{
		// see if we have a ML_Member on either side of the place where we are about
		// to insert our new omitted node.

		bool bMLMember = (   pDeSync->isMLMember()
						  || pDeSync->isAnyChangeType()
						  || (   (pDeSync->getPrev())
							  && (   pDeSync->getPrev()->isMLMember()
								  || pDeSync->getPrev()->isAnyChangeType())));

		de_sync * pDeSyncNew;

		if (bMerge)
			pDeSyncNew = new de_sync_line_omitted(PANEL_T1,pDeLinesOmitted[PANEL_T1],lenOmitted[PANEL_T1],
												  PANEL_T0,pDeLinesOmitted[PANEL_T0],lenOmitted[PANEL_T0],
												  PANEL_T2,pDeLinesOmitted[PANEL_T2],lenOmitted[PANEL_T2],
												  bMLMember);
		else
			pDeSyncNew = new de_sync_line_omitted(PANEL_T1,pDeLinesOmitted[PANEL_T1],lenOmitted[PANEL_T1],
												  PANEL_T0,pDeLinesOmitted[PANEL_T0],lenOmitted[PANEL_T0],
												  bMLMember);

		m_sync_list[kSync]._insert_before(pDeSync,pDeSyncNew);
	}

//	wxLogTrace(TRACE_DE_DUMP,_T("================================================================"));
//	wxLogTrace(TRACE_DE_DUMP,_T("AFTER_INSERT_OMITTED:"));
//	m_sync_list[kSync].dump(30);
//	wxLogTrace(TRACE_DE_DUMP,_T("================================================================"));
}
	

#if 0

void de_de::_insert_omitted_lines_into_sync_list(long kSync)
{
	if (_totalOmitted(kSync) == 0)
		return;

	de_sync * pDeSync = m_sync_list[kSync].getHead();
	do
	{
		pDeSync =__insert_omitted_lines_into_sync_list(kSync,pDeSync);
		
	} while (pDeSync != m_sync_list[kSync].getTail());
}

de_sync * de_de::__insert_omitted_lines_into_sync_list(long kSync, de_sync * pDeSyncHead)
{
	//////////////////////////////////////////////////////////////////
	// if the current ruleset has settings to mark lines as OMITTED,
	// then run thru the sync list we just constructed and split
	// nodes that span an omitted line.  insert a special "omitted"
	// sync node.
	//
	// note: because sync nodes only refer to lines via an index into
	// note: m_vecLineCmp, these special "omitted" sync nodes will not
	// note: have an index (because omitted lines are not in the vector).
	// note: so we have a sync node sub-class to handle this case and
	// note: keep us (me) from getting confused.


	// we assume that the sync list had been loaded and that it represents
	// contiguous coverage of all non-omitted lines.

	de_sync *		pDeSync,    * pDeSyncNew;
	de_line *		pDeLine,    * pDeLinePrev;
	const fl_line * pFlLine[3], * pFlLinePrev, * pFlLineNext;
	bool bHaveOmitted;
	long k, kRow, kPanel, lenSync, ndx;

	// WARNING: we use a layer of indirect subscripting here to let
	// WARNING: us reference different sets of panels.  All of the
	// WARNING: arrays of size [3] should be indexed with k.  All of
	// WARNING: the sync-list,layout,etc data structures should be
	// WARNING: indexed with kPanel == panels[k].
	// WARNING: note that the center panel (either _T1 or _EDIT) is
	// WARNING: is in [0].

	bool bMerge = (getLayout(kSync, PANEL_T2  ) != NULL);	// 3-way merge (as opposed to 2-way diff)

	PanelIndex panels[__NR_TOP_PANELS__];
	long nrPanels, lenOmitted[__NR_TOP_PANELS__];
	de_line * pDeLinesOmitted[__NR_TOP_PANELS__];

	panels[0]  = ((kSync==SYNC_VIEW) ? PANEL_T1 : PANEL_EDIT);
	panels[1]  = PANEL_T0;
	panels[2]  = PANEL_T2;

	nrPanels   = ((bMerge) ? 3 : 2);

//	wxASSERT_MSG( (pDeSyncTail->isEND() || pDeSyncTail->isMARK()), _T("Coding Error") );

	// for each node in the sync list:
	//    for each (possibly unbalanced) row within it:
	//       for each panel:
	//          if there is one or more omitted lines before the line on this row:
	//             remember first omitted line and the number of omitted lines for this panel.
	//       if any omitted lines were found for this row:
	//          split current sync node (if necessary).
	//          insert new "omitted" node between halves (or prior to unsplit node).

	//////////////////////////////////////////////////////////////////
	// TODO the above is not quite correct when omitted lines are adjacent
	// TODO to non-equal lines where we're showing a gap in one of the
	// TODO panels.  [think: an insert]
	// TODO
	// TODO the problem is that the correct answer is kind of ambiguous.
	// TODO and it's not clear how often it will happen and i don't want
	// TODO to mess with it right now.
	// TODO
	// TODO for example, suppose our RuleSet is set to omit lines consisting
	// TODO of "<omit>", then for the source files:
	// TODO 
	// TODO T0        T1
	// TODO ======    ======
	// TODO AAA       AAA
	// TODO <omit>    <omit>
	// TODO bbb       CCC
	// TODO CCC       <eof>
	// TODO <eof>
	// TODO
	// TODO then the following algorithm will produce:
	// TODO
	// TODO T0        T1
	// TODO ======    ======
	// TODO AAA       AAA
	// TODO <omit>    <gap>
	// TODO bbb       <gap>
	// TODO <gap>     <omit>
	// TODO CCC       CCC
	// TODO <eof>     <eof>
	// TODO
	// TODO in this case, the result looks wrong -- the 2 omitted lines
	// TODO should be lined up.  but when <omit> and "bbb" are switched,
	// TODO the result looks right (with both omitted lines lined up).
	// TODO 
	// TODO the problem is a little more obvious when there multiple
	// TODO consecutive omitted lines on one side and the same number
	// TODO are distributed before and after the "bbb" and/or within
	// TODO "bbb".
	// TODO 
	// TODO this problem really stands out when treat blank lines as
	// TODO omitted -- it's almost like we need to do all the complicated
	// TODO sync/gap-match-up code in de_sync for this.  [just say no.]
	// TODO 
	// TODO SINCE THIS FEATURE IS INITIALLY INTENDED AS WAY TO OMIT
	// TODO HARD PAGE BREAKS AND PAGE HEADERS [that is, a \f and n header
	// TODO lines], and the user is likely to turn on hide-omitted,
	// TODO I'm going to ignore this problem for now.
	//////////////////////////////////////////////////////////////////

	memset(pFlLine,0,sizeof(pFlLine));

	pDeSync = pDeSyncHead;
	while (pDeSync && !pDeSync->isEND())
	{
		kRow = 0;
	TopOfLoop:
		lenSync = pDeSync->getMaxLen();
		for (/*kRow*/; (kRow<lenSync); kRow++)
		{
			memset(lenOmitted,     0,sizeof(lenOmitted));
			memset(pDeLinesOmitted,0,sizeof(pDeLinesOmitted));
			bHaveOmitted = false;
			
			for (k=0; k<nrPanels; k++)
			{
				kPanel = panels[k];
				ndx = pDeSync->getNdx((PanelIndex)kPanel);
				if ((ndx != -1)  && (kRow < pDeSync->getLen((PanelIndex)kPanel)))
				{
					TVector_LineCmp * pVecLineCmp = _lookupVecLineCmp(kSync,(PanelIndex)kPanel);
					pDeLine     = (*pVecLineCmp)[ndx+kRow];
					pFlLine[k]  = pDeLine->getFlLine();
					if (!pDeLine->hasMark(kSync))
					{
						pFlLinePrev = pFlLine[k]->getPrev();
						pDeLinePrev = ((pFlLinePrev) ? pFlLinePrev->getSlotValue( *_lookupSlot(kSync,(PanelIndex)kPanel) ) : NULL);
						while (pDeLinePrev && pDeLinePrev->isOmitted())
						{
							bHaveOmitted = true;
							lenOmitted[k]++;
							pDeLinesOmitted[k] = pDeLinePrev;
							if (pDeLinePrev->hasMark(kSync))
								break;
							pFlLinePrev = pFlLinePrev->getPrev();
							pDeLinePrev = ((pFlLinePrev) ? pFlLinePrev->getSlotValue( *_lookupSlot(kSync,(PanelIndex)kPanel) ) : NULL);
						}
					}
				}
			}
			if (bHaveOmitted)
			{
				if (bMerge)
					pDeSyncNew = new de_sync_line_omitted(panels[0],pDeLinesOmitted[0],lenOmitted[0],
														  panels[1],pDeLinesOmitted[1],lenOmitted[1],
														  panels[2],pDeLinesOmitted[2],lenOmitted[2]);
				else
					pDeSyncNew = new de_sync_line_omitted(panels[0],pDeLinesOmitted[0],lenOmitted[0],
														  panels[1],pDeLinesOmitted[1],lenOmitted[1]);

				if (kRow == 0)
				{
					// if omission was prior to first row, no split needed.
					// insert new "omitted" sync node before current node and
					// continue scanning this node.
					m_sync_list[kSync]._insert_before(pDeSync,pDeSyncNew);
				}
				else			// in middle of node, must split.
				{
					// split current at given row, insert new "omitted" sync node before the 2nd half
					// and continue scanning with 2nd half.  since pDeSync changed, lenSync & kRow
					// are now invalid.  so let's restart the main loop again starting with 2nd half
					// and start with row 1 so we don't duplicate work [the omission is *still* before
					// row 0 in the 2nd half].

					m_sync_list[kSync].split(pDeSync,kRow);
					pDeSync = pDeSync->getNext();
					m_sync_list[kSync]._insert_before(pDeSync,pDeSyncNew);
					kRow = 1;
					goto TopOfLoop;
				}
			}
		}
		pDeSync = pDeSync->getNext();
		if (pDeSync->isMARK())
			break;
	}

	// we have handled inserting omitted-sync-nodes before and between existing
	// sync-nodes in the list (proper).  pDeSync now points to a TAIL sync-node
	// which is either the EOF or the next MARK.
	// 
	// see if there are any omitted lines between the last sync-node and the tail
	// sync-node that need to be taken care of.

	if (pDeSync == pDeSyncHead)
	{
		// if the sync-list was empty, we need to get the first lines in
		// each panel (since we just initialized them with memset()).  [this is kind
		// of a screw-case where we omit the entire content of both/all files.]

		wxASSERT_MSG( (pDeSyncHead == m_sync_list[kSync].getHead()), _T("Coding Error") );

		for (k=0; k<nrPanels; k++)
		{
			kPanel = panels[k];
			pFlLine[k] = getLayout(kSync, (PanelIndex)kPanel)->getFirstLine();
		}
	}
	else
	{
		// if we have a non-empty list (head != tail), then pFlLine[] contains
		// pointers to the last fl-line referenced in each panel.  so, we need to advance
		// them to the next line.

		for (k=0; k<nrPanels; k++)
			if (pFlLine[k])
				pFlLine[k] = pFlLine[k]->getNext();
	}
	
	memset(lenOmitted,     0,sizeof(lenOmitted));
	memset(pDeLinesOmitted,0,sizeof(pDeLinesOmitted));
	bHaveOmitted = false;
			
	for (k=0; k<nrPanels; k++)
	{
		if (pFlLine[k])
		{
			// get the first de_line and compute the number of lines

			kPanel = panels[k];
			pDeLinesOmitted[k] = pFlLine[k]->getSlotValue( *_lookupSlot(kSync,(PanelIndex)kPanel) );
			pFlLineNext        = pFlLine[k];
			while (pFlLineNext)
			{
				de_line * pDeLine = pFlLineNext->getSlotValue( *_lookupSlot(kSync,(PanelIndex)kPanel) );
				if (pDeLine->hasMark(kSync))
					break;
				
				bHaveOmitted = true;

				lenOmitted[k]++;
				wxASSERT_MSG( (pDeLine->isOmitted()), _T("Coding Error!") );
				pFlLineNext = pFlLineNext->getNext();
			}
		}
	}

	if (bHaveOmitted)
	{
		if (bMerge)
			pDeSyncNew = new de_sync_line_omitted(panels[0],pDeLinesOmitted[0],lenOmitted[0],
												  panels[1],pDeLinesOmitted[1],lenOmitted[1],
												  panels[2],pDeLinesOmitted[2],lenOmitted[2]);
		else
			pDeSyncNew = new de_sync_line_omitted(panels[0],pDeLinesOmitted[0],lenOmitted[0],
												  panels[1],pDeLinesOmitted[1],lenOmitted[1]);
		m_sync_list[kSync]._insert_before(pDeSync,pDeSyncNew);

#if 0
#ifdef DEBUG
		wxLogTrace(TRACE_DE_DUMP,_T("End of InsertOmitted: [%p][%p][%p]"),
				   pFlLine[0],pFlLine[1],pFlLine[2]);
		for (int kk=0; kk<3; kk++)
			if (pFlLine[kk])
				pFlLine[kk]->dump(10);
		pDeSyncNew->dump(20);
#endif
#endif
	}

	// return the end of the partition -- where we should start next time.

	return pDeSync;
}
#endif
