// de_de__batchoutput_text_unified.cpp -- diff engine -- parts of de_de related
// to generating batch output (such as gnu-diff-like output).
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

static wxString _format_unified_header_string(const wxChar * szPrefix, poi_item * pPoiItem, wxString * pStrLabel=NULL)
{
	// if pStrLabel is given, use it rather than the pathname in the header.

	wxDateTime dtm;
	pPoiItem->getDateTimeModified(&dtm);
	wxString strDTM = util_string__format_unified_date_from_dtm(dtm);

	wxString str;

	str += szPrefix;
	str += _T(" ");
	if (pStrLabel)
		str += *pStrLabel;
	else
		str += pPoiItem->getFullPath();
	str += _T("\t");
	str += strDTM;
	str += _T("\n");

	return str;
}

bool unified_diff__compute_row_end_of_chunk(TVector_Display * pDis,
											long nrRows,
											long kRowStart,
											long * pkRowEnd)
{
	// find a 'chunk' in the display list.  that is a change with the context
	// around it.  these are delimited by GAPs (the bit on the row to indicate
	// that we need to draw a horizontal line above it).

	const de_row & rDeRowStart = (*pDis)[kRowStart];
	if (rDeRowStart.isEOF())
		return false;
			
	wxASSERT_MSG( ((rDeRowStart.haveGap()) || (kRowStart == 0)), _T("Coding Error") );

	long kRowEnd = kRowStart + 1;

	while (kRowEnd < nrRows)
	{
		const de_row & rDeRowEnd = (*pDis)[kRowEnd];
		if (rDeRowEnd.isEOF() || rDeRowEnd.haveGap())
			break;
		kRowEnd++;
	}

	wxLogTrace(wxTRACE_Messages,
			   _T("bud: chunk [rows %ld .. %ld]"),kRowStart,kRowEnd);

	*pkRowEnd = kRowEnd;
	return true;
}

void unified_diff__map_rows_to_line_numbers(de_de * pDeDe, TVector_Display * pDis, long kSync,
											long kRowStart, long kRowEnd,
											struct _unified_diff__line_nr_mapping * p)
{
	// map the row numbers (in the display list) to line numbers (in the documents).
	// according to gnu-diff-format, we'll want to display %d or %d,%d when we have
	// content and we want these to be 1-based numbers.

	p->bVoid[0] = true;
	p->bVoid[1] = true;
	p->lineNrFirst[0] = 1;
	p->lineNrFirst[1] = 1;
	p->lineNrLast[0] = 1;
	p->lineNrLast[1] = 1;
	p->len[0] = 0;
	p->len[1] = 0;
	
	for (long kRow=kRowStart; (kRow<kRowEnd); kRow++)
	{
		const de_row & rDeRowK = (*pDis)[kRow];

		for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
		{
			const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
			if (pDeLine)
			{
				if (p->bVoid[kPanel])
				{
					p->lineNrFirst[kPanel] = pDeLine->getFlLine()->getLineNr() + 1;
					p->bVoid[kPanel] = false;
				}
				p->lineNrLast[kPanel] = pDeLine->getFlLine()->getLineNr() + 1;
			}
		}
	}

	// when we are completely void on a side of the change, we need to get
	// the line number of the line immediately prior to the start of the change.
	// this is also 1-based, but refers to the spot before the change.

	for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
	{
		if (p->bVoid[kPanel])
		{
			const de_row & rDeRowFirst = (*pDis)[kRowStart];
			const de_sync * pSyncFirst = rDeRowFirst.getSync();
			wxASSERT_MSG( (rDeRowFirst.getOffsetInSync()==0), _T("Coding Error") );
			const de_sync * pSyncPrev = pSyncFirst->getPrev();
			while (pSyncPrev)
			{
				if (pSyncPrev->isOmitted())
				{
					wxLogTrace(TRACE_BATCH,
							   _T("J1202: stepping back over omitted item %p to get line number [kSync %ld][kPanel %ld][ndx %ld][len %ld]"),
							   pSyncPrev, kSync, (long)kPanel,
							   (long)pSyncPrev->getNdx((PanelIndex)kPanel),
							   (long)pSyncPrev->getLen((PanelIndex)kPanel));
					pSyncPrev = pSyncPrev->getPrev();
					continue;
				}

				long ndx = pSyncPrev->getNdx((PanelIndex)kPanel) + pSyncPrev->getLen((PanelIndex)kPanel) - 1;
				if (ndx < 0)
				{
					wxLogTrace(TRACE_BATCH,
							   _T("J1202: stepping back over void item %p to get line number [kSync %ld][kPanel %ld][ndx %ld][len %ld]"),
							   pSyncPrev, kSync, (long)kPanel,
							   (long)pSyncPrev->getNdx((PanelIndex)kPanel),
							   (long)pSyncPrev->getLen((PanelIndex)kPanel));
					pSyncPrev = pSyncPrev->getPrev();
					continue;
				}

				const de_line * pDeLinePrev = pDeDe->getDeLineFromNdx(kSync, (PanelIndex)kPanel, ndx);
				if (!pDeLinePrev)
				{
					wxLogTrace(TRACE_BATCH,
							   _T("J1202: stepping back over item %p to get line number [kSync %ld][kPanel %ld][ndx %ld][len %ld]"),
							   pSyncPrev, kSync, (long)kPanel,
							   (long)pSyncPrev->getNdx((PanelIndex)kPanel),
							   (long)pSyncPrev->getLen((PanelIndex)kPanel));
					pSyncPrev = pSyncPrev->getPrev();
					continue;
				}

				p->lineNrFirst[kPanel] = pDeLinePrev->getFlLine()->getLineNr() + 1;
				p->lineNrLast[kPanel] = pDeLinePrev->getFlLine()->getLineNr() + 1;

				wxLogTrace(TRACE_BATCH,
						   _T("J1202: using item %p to get line number [kSync %ld][kPanel %ld][ndx %ld][len %ld] [%ld..%ld]"),
						   pSyncPrev, kSync, (long)kPanel,
						   (long)pSyncPrev->getNdx((PanelIndex)kPanel),
						   (long)pSyncPrev->getLen((PanelIndex)kPanel),
						   p->lineNrFirst[kPanel],
						   p->lineNrLast[kPanel]);
				goto finished;
			}

			wxLogTrace(TRACE_BATCH,
					   _T("J1202: no previous item to get line number from [kSync %ld][kPanel %ld]"),
					   kSync, (long)kPanel);
		finished:
			;
		}
	}

	for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
	{
		p->len[kPanel] = p->lineNrLast[kPanel] - p->lineNrFirst[kPanel] + 1;
	}

	wxLogTrace(TRACE_BATCH,_T("bud: chunk [lines (%ld,%ld) (%ld,%ld)][len %ld %ld]"),
			   p->lineNrFirst[0],p->lineNrLast[0],
			   p->lineNrFirst[1],p->lineNrLast[1],
			   p->len[0],p->len[1]);
}


void de_de::batchoutput_text_unified_diff2(long kSync, wxString & strOut, bool * pbHadChanges,
										   wxString * pStrLabelA, wxString * pStrLabelB)
{
	// generate gnu-diff-like UNIFIED output for a 2-way diff into
	// the string given.  this is a wide buffer so the caller
	// will probably need to convert it to an 8bit form.
	//
	// we use LF's for the change headers and.
	// we will use the EOL's in file for each data line.
	// (this seems to be consistent with gnu diff output.)
	//
	// we assume that we have complete control of our this and
	// can tinker with it as necessary (such as setting the
	// display ops).
	//
	// NOTE: we must assume that the caller has set up the
	// NOTE: appropriate global-props environment (so that the
	// NOTE: current ruleset is sane for this type of output
	// NOTE: (no omitted lines, for example)) and that there
	// NOTE: are no manual-alignment-marks....
	// NOTE:
	// NOTE: ideally, we'll be in line-only detail-level, but
	// NOTE: that is not a requirement.

	strOut.Empty();

	de_display_ops dopsOld = m_dops[kSync];

	// unified diffs are only really defined as context diff.
	// force CTX mode and rebuild the display list if necessary.
	// we pick up the ignore-unimportant.
	// We don't bother with the hide-omitted, since they won't
	// show up in a context diff anyway.
	de_display_ops dopsNew = DE_DOP_CTX;
	bool bIgnoreUnimportant = DE_DOP__IS_SET_IGN_UNIMPORTANT(dopsOld);
	if (bIgnoreUnimportant)
		dopsNew |= DE_DOP_IGN_UNIMPORTANT;
	setDisplayOps(kSync, dopsNew);
	run();

	TVector_Display * pDis = getDisplayList(kSync);	// forces run() if necessary
	int nrRows = (int)pDis->size() - 1;

	const de_stats2 * pStats = getStats2(kSync);
	long cChanges = pStats->m_cImportantChanges;	// + pStats->m_cUnimportantChanges;

	*pbHadChanges = (cChanges > 0);
	
	// write unified header block.

	poi_item * pPoiT0 = m_pFsFs->getPoi(kSync,PANEL_T0);
	poi_item * pPoiT1 = m_pFsFs->getPoi(kSync,PANEL_T1);

	strOut += _format_unified_header_string(L"---",pPoiT0, pStrLabelA);
	strOut += _format_unified_header_string(L"+++",pPoiT1, pStrLabelB);

	wxString strPrefix[2] = { _T("-"), _T("+") };
	wxString strPrefixUnimportant[2] = { _T("u"), _T("U") };

	// for unified output, we need some lines of context.  this is exactly what we have
	// in the _CTX-mode display list.  so we can walk it directly and generate our output.
	// we cannot use the next/prev change list because it is not set up for us in this mode.
	//
	// again, note that row numbers are not line numbers.

	long kRowStart = 0;
	while (kRowStart < nrRows)
	{
		long kRowEnd = -1;
		if (!unified_diff__compute_row_end_of_chunk(pDis, nrRows, kRowStart, &kRowEnd))
			break;
			
		struct _unified_diff__line_nr_mapping lnm;
		unified_diff__map_rows_to_line_numbers(this, pDis, kSync, kRowStart, kRowEnd, &lnm);

		// write hunk header -- we always write length [startLine,nrLines "%d,%d"]
		// rather than just the start when the length is 1.

		strOut += wxString::Format(_T("@@ -%ld,%ld +%ld,%ld @@\n"),
								   lnm.lineNrFirst[0],lnm.len[0],
								   lnm.lineNrFirst[1],lnm.len[1]);

		//////////////////////////////////////////////////////////////////
		// write diffs
		//
		// because we have context, we have EQ sync nodes and NEQ sync nodes.
		// as we walk the display list rows, we output the EQ ones as soon as
		// we see them.  we accumulate the NEQ ones until we reach either an EQ
		// node or the end of the chunk -- so that we can display all of the
		// deletions and then all the additions -- rather than interleave them
		// one by one.

		wxString strLines[2];

		for (long kRow=kRowStart; (kRow<kRowEnd); kRow++)
		{
			const de_row & rDeRowK = (*pDis)[kRow];
			const de_sync * pSyncK = rDeRowK.getSync();

			bool bTreatAsUnimportant = (bIgnoreUnimportant && pSyncK->isUnimportant());

			if (pSyncK->isSameType(DE_ATTR_DIF_2EQ)
				|| pSyncK->isSameType(DE_ATTR_OMITTED)
				|| bTreatAsUnimportant)
			{
				// 2-way equal or omitted.
				// 
				// when we have an omitted (from analysis) line, we
				// have to draw these in a unified diff so that the
				// range numbers in the @@ header is well-defined.
				//
				// flush any cached lines.

				if (strLines[0].Length() > 0)
				{
					strOut += strLines[0];
					strLines[0].Empty();
				}
				if (strLines[1].Length() > 0)
				{
					strOut += strLines[1];
					strLines[1].Empty();
				}
			}

			if (pSyncK->isSameType(DE_ATTR_DIF_2EQ))
			{
				// 2-way equal.  write this line immediately.
				
				const de_line * pDeLine = rDeRowK.getPanelLine(PANEL_T0);	// either panel is fine, since they are equal
				wxASSERT_MSG( (pDeLine), _T("Coding Error") );
				const fl_line * pFlLine = pDeLine->getFlLine();

				wxString strThisLine, strThisEol;
				pFlLine->buildStringsFromRuns(false,&strThisLine,&strThisEol);

				strOut += _T(" ");		// prefix on eq lines
				strOut += strThisLine;
				strOut += strThisEol;

				if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
					strOut += _T("\n\\ No newline at end of file\n");
			}
			else if (bTreatAsUnimportant)
			{
				// an unimportant change that is adjacent to an important change.
				// normally we would exclude an isolated unimportant change (because
				// we're only showing context diffs here), but when an unimportant
				// change is adjacent to an important change, we need to draw part
				// of it as the "context" to keep the range numbers in the @@ header
				// in sync.
				//
				// the question is, should we display both sides or just pick one.
				// they've already said they don't care about it.  for example, if
				// it is an indentation change in the code, showing both sides makes
				// as much noise as just showing the '-' and '+' lines for important
				// changes.
				//
				// for now, i think i'll duplicate what i did for omitted lines.
				// this avoids the ambiguity in the display and shouldn't cause
				// too much noise since we're only talking about adjacent changes.
				//
				// TODO 2013/09/05 How should we write an unimportant change
				// TODO            within a unified text diff?  I'm going to
				// TODO            deviate from industry standard and use
				// TODO            special prefix characters for them.

				for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
				{
					const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
					if (pDeLine)
					{
						const fl_line * pFlLine = pDeLine->getFlLine();

						wxString strThisLine, strThisEol;
						pFlLine->buildStringsFromRuns(false,&strThisLine,&strThisEol);

						strOut += strPrefixUnimportant[kPanel];
						strOut += strThisLine;
						strOut += strThisEol;

						if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
							strLines[kPanel] += _T("\n\\ No newline at end of file\n");
					}
				}
			}
			else if (pSyncK->isSameType(DE_ATTR_DIF_0EQ))
			{
				// accumulate changed lines as 2 sets of "+" and "-" lines
				// in the line cache. so that we can cluster them.

				for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
				{
					const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
					if (pDeLine)
					{
						const fl_line * pFlLine = pDeLine->getFlLine();

						wxString strThisLine, strThisEol;
						pFlLine->buildStringsFromRuns(false,&strThisLine,&strThisEol);

						strLines[kPanel] += strPrefix[kPanel];
						strLines[kPanel] += strThisLine;
						strLines[kPanel] += strThisEol;

						if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
							strLines[kPanel] += _T("\n\\ No newline at end of file\n");
					}
				}
			}
			else if (pSyncK->isSameType(DE_ATTR_OMITTED))
			{
				// write this line immediately.

				wxString strThisLine[2];
				wxString strThisEol[2];
				bool bPresent[2];
					
				for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
				{
					const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
					if (pDeLine)
					{
						bPresent[kPanel] = true;
						const fl_line * pFlLine = pDeLine->getFlLine();

						pFlLine->buildStringsFromRuns(false,&strThisLine[kPanel],&strThisEol[kPanel]);

					}
					else
					{
						bPresent[kPanel] = false;
					}
				}

				if (bPresent[0] && bPresent[1] && strThisLine[0].IsSameAs(strThisLine[1]))
				{
					// if both sides have an omitted line and they are equal,
					// just write a single line in the output.
					// I'm just going to the leave column 0 as blank since they
					// are equal -- I don't have a good way to note that they
					// were omitted.

					strOut += _T(" ");		// prefix on eq lines
					strOut += strThisLine[0];
					strOut += strThisEol[0];
					// TODO 2013/09/03 think about whether to do something with this
					// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
					//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");
				}
				else
				{
					// one side only or both sides are different.
					// write '-' then '+' rows like we do for changes.
					//
					// TODO 2013/09/04 How should an omitted line be drawn
					// TODO            in a text unified diff.  This is an
					// TODO            industry format.  Ideally if the
					// TODO            user cared about using patch and
					// TODO            friends they wouldn't have defined
					// TODO            omit patterns.  So anything we do
					// TODO            here is wrong-ish.  But I have to
					// TODO            write something so that the counts
					// TODO            in the @@ header are correct.

					if (bPresent[0])
					{
						strOut += _T("x");
						strOut += strThisLine[0];
						strOut += strThisEol[0];
						// TODO 2013/09/03 think about whether to do something with this
						// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
						//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");

					}
					if (bPresent[1])
					{
						strOut += _T("X");
						strOut += strThisLine[1];
						strOut += strThisEol[1];
						// TODO 2013/09/03 think about whether to do something with this
						// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
						//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");

					}
				}
			}
			else
			{
				wxASSERT_MSG( (0), _T("Coding Error") );
			}
		}

		// reached end of chunk, see if we have any cached lines (possible when
		// no EQ context following NEQ lines.
			
		if (strLines[0].Length() > 0)
		{
			strOut += strLines[0];
			strLines[0].Empty();
		}
		if (strLines[1].Length() > 0)
		{
			strOut += strLines[1];
			strLines[1].Empty();
		}

		// set up for start of next chunk

		kRowStart = kRowEnd;
	}

	setDisplayOps(kSync,dopsOld);
	run();
}
