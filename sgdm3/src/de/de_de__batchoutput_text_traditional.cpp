// de_de__batchoutput_text_traditional.cpp -- diff engine -- parts of de_de related
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

wxString _traditional_diff__format_change_header(const struct _unified_diff__line_nr_mapping & lnm)
{
	// compute the gnu-style change header

	wxString strHdr;
	if ((lnm.len[PANEL_T0] > 0) && (lnm.len[PANEL_T1] > 0))
	{
		// we have a change

		if (lnm.len[PANEL_T0] > 1)
			strHdr += wxString::Format(_T("%ld,%ld"),lnm.lineNrFirst[PANEL_T0],lnm.lineNrLast[PANEL_T0]);
		else
			strHdr += wxString::Format(_T("%ld"),lnm.lineNrFirst[PANEL_T0]);
		strHdr += _T("c");
		if (lnm.len[PANEL_T1] > 1)
			strHdr += wxString::Format(_T("%ld,%ld"),lnm.lineNrFirst[PANEL_T1],lnm.lineNrLast[PANEL_T1]);
		else
			strHdr += wxString::Format(_T("%ld"),lnm.lineNrFirst[PANEL_T1]);
	}
	else if (lnm.len[PANEL_T0] > 0)
	{
		// we have a delete

		if (lnm.len[PANEL_T0] > 1)
			strHdr += wxString::Format(_T("%ld,%ld"),lnm.lineNrFirst[PANEL_T0],lnm.lineNrLast[PANEL_T0]);
		else
			strHdr += wxString::Format(_T("%ld"),lnm.lineNrFirst[PANEL_T0]);
		strHdr += _T("d");
		strHdr += wxString::Format(_T("%ld"),lnm.lineNrFirst[PANEL_T1]);
	}
	else if (lnm.len[PANEL_T1] > 0)
	{
		// we have an add

		strHdr += wxString::Format(_T("%ld"),lnm.lineNrFirst[PANEL_T0]);
		strHdr += _T("a");
		if (lnm.len[PANEL_T1] > 1)
			strHdr += wxString::Format(_T("%ld,%ld"),lnm.lineNrFirst[PANEL_T1],lnm.lineNrLast[PANEL_T1]);
		else
			strHdr += wxString::Format(_T("%ld"),lnm.lineNrFirst[PANEL_T1]);
	}
	else
	{
		wxASSERT_MSG( (0), _T("Coding Error") );
	}

	return strHdr;
}

void de_de::batchoutput_text_traditional_diff2(long kSync, wxString & strOut, bool * pbHadChanges)
{
	// generate gnu-diff-like output for a 2-way diff into
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

	wxString strPrefix[2] = { _T("< "), _T("> ") };

	strOut.Empty();

	de_display_ops dopsOld = m_dops[kSync];

	// normal/traditional diffs only show the changes and without
	// any context.
	// force DIF mode and rebuild the display list if necessary.
	// we pick up the ignore-unimportant.
	// We don't bother with the hide-omitted, since they won't
	// show up in a diff-only diff anyway.
	de_display_ops dopsNew = DE_DOP_DIF;
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
	
	// use the DIF-ONLY display list to display the changes.
	// we get row values for the display list -- these are
	// not line numbers.

	long kRowStart = 0;
	while (kRowStart < nrRows)
	{
		long kRowEnd = -1;
		if (!unified_diff__compute_row_end_of_chunk(pDis, nrRows, kRowStart, &kRowEnd))
			break;

		struct _unified_diff__line_nr_mapping lnm;
		unified_diff__map_rows_to_line_numbers(this, pDis, kSync, kRowStart, kRowEnd, &lnm);

		// compute the gnu-style change header

		wxString strHdr = _traditional_diff__format_change_header(lnm);
		strHdr += _T("\n");

		// compute the body of each side of the change

		wxString strLines[2];

		for (long kRow=kRowStart; (kRow<kRowEnd); kRow++)
		{
			const de_row & rDeRowK = (*pDis)[kRow];

			for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
			{
				const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
				if (pDeLine)
				{
					const fl_line * pFlLine = pDeLine->getFlLine();

					wxString strLine, strUnused;
					pFlLine->buildStringsFromRuns(true,&strLine,&strUnused);

					strLines[kPanel] += wxString::Format(_T("%s%s"),strPrefix[kPanel].wc_str(),strLine.wc_str());
				}
			}
		}

		// TODO think about how to note no final NL.

		// now put it all together into output buffer

		strOut += strHdr;
		strOut += strLines[PANEL_T0];
		if ((lnm.len[PANEL_T0] > 0) && (lnm.len[PANEL_T1] > 0))
			strOut += _T("---\n");
		strOut += strLines[PANEL_T1];

		// set up for start of next chunk

		kRowStart = kRowEnd;
	}

	setDisplayOps(kSync,dopsOld);
	run();
}

