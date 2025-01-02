// de_de__batchoutput_html_traditional.cpp -- diff engine -- parts of de_de related
// to generating batch output (such as gnu-diff-like output) but formatted
// in HTML rather than raw text.
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

static const wchar_t * gsz_t2d_h_x = (		// part of header: path+date
	L"<tr class=\"t2d_h_x\">\n"
	L"  <td class=\"t2d_h_x_prefix\">%s</td>\n"
	L"  <td class=\"t2d_h_x_path\">%s</td>\n"
	L"  <td class=\"t2d_h_x_dtm\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_t2d_h_stats = (	// part of header: stats
	L"<tr class=\"t2d_h_stats\">\n"
	L"  <td colspan=\"4\">%s</td>\n"			// de_stats2
	L"</tr>\n"
	);

static const wchar_t * gsz_t2d_h = (			// overall header
	L"<table class=\"t2d_h\">\n"
	L"%s"			// left path/label
	L"%s"			// right path/label
	L"%s"			// stats in header
	L"</table>\n"
	);

static const wchar_t * gsz_t2d_f = (			// overall footer
	L"<table class=\"t2d_f\">\n"
	L"  <tr class=\"t2d_f_dops\">\n"
	L"    <td>%s</td>\n"						// display_ops
	L"  </tr>\n"
	L"</table>\n"
	);

static const wchar_t * gsz_t2d_x_atat = (		// a row of code in diff
	L"<tr class=\"t2d_x_atat\">\n"
	L"  <td class=\"t2d_ln t2d_x_atat_ln\">...</td>\n"			// left line number
	L"  <td class=\"t2d_ln t2d_x_atat_ln\">...</td>\n"			// right line number
	L"  <td class=\"t2d_x_atat_data\" colspan=\"2\">%s</td>\n"	// @@ data
	L"</tr>\n"
	);

static const wchar_t * gsz_t2d_x1[2] = {		// a row of code in diff
	(
		L"<tr class=\"t2d_x_del\">\n"
		L"  <td class=\"t2d_ln t2d_x_del_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"t2d_ln t2d_x_del_ln\"></td>\n"
		L"  <td class=\"t2d_char\">&lt;</td>\n"			// column for traditional '<' or '>'
		L"  <td class=\"t2d_code\">%s</td>\n"			// a line of code
		L"</tr>\n"
		),
	(
		L"<tr class=\"t2d_x_add\">\n"
		L"  <td class=\"t2d_ln t2d_x_add_ln\"></td>\n"		// left line number
		L"  <td class=\"t2d_ln t2d_x_add_ln\">%ld</td>\n"		// right line number
		L"  <td class=\"t2d_char\">&gt;</td>\n"			// column for traditional '<' or '>'
		L"  <td class=\"t2d_code\">%s</td>\n"			// a line of code
		L"</tr>\n"
		) 
};

static const wchar_t * gsz_t2d_x1_gap[2] = {		// a row of code in diff
	(
		L"<tr class=\"t2d_x_del_gap\">\n"
		L"  <td class=\"t2d_ln t2d_x_del_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"t2d_ln t2d_x_del_ln\"></td>\n"
		L"  <td class=\"t2d_char\">&lt;</td>\n"			// column for traditional '<' or '>'
		L"  <td class=\"t2d_code\">%s</td>\n"			// a line of code
		L"</tr>\n"
		),
	(
		L"<tr class=\"t2d_x_add_gap\">\n"
		L"  <td class=\"t2d_ln t2d_x_add_ln\"></td>\n"		// left line number
		L"  <td class=\"t2d_ln t2d_x_add_ln\">%ld</td>\n"		// right line number
		L"  <td class=\"t2d_char\">&gt;</td>\n"	// column for traditional '<' or '>'
		L"  <td class=\"t2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		) 
};


static const wchar_t * gsz_t2d_x1_unimp_il[2] = {			// intraline highlight
	(L"<span class=\"t2d_x_unimp_il\">%s</span>"),	// DO NOT ADD TRAILING \n
	(L"<span class=\"t2d_x_unimp_il\">%s</span>")	// DO NOT ADD TRAILING \n
};
	
static const wchar_t * gsz_t2d_x1_il[2] = {			// intraline highlight
	(L"<span class=\"t2d_x_del_il\">%s</span>"),		// DO NOT ADD TRAILING \n
	(L"<span class=\"t2d_x_add_il\">%s</span>")		// DO NOT ADD TRAILING \n
};
	

#if WANT_DASHES // i don't think we need dash line in html
static const wchar_t * gsz_t2d_x_dash = (		// the '---' line between block of '<' and block of '>'
	L"<tr class=\"t2d_x_dash\">\n"
	L"  <td class=\"t2d_ln t2d_x_dash_ln\">...</td>\n"			// left line number
	L"  <td class=\"t2d_ln t2d_x_dash_ln\">...</td>\n"			// right line number
	L"  <td class=\"t2d_x_dash_data\" colspan=\"2\">---</td>\n"
	L"</tr>\n"
	);
#endif

static const wchar_t * gsz_t2d = (			// overall  diff
	L"<div class=\"t2d\">\n"
	L"  <table class=\"t2d_t\">\n"
	L"    <thead class=\"t2d_thead\">\n"
	L"      <tr>\n"
	L"        <td class=\"t2d_thead_td\" colspan=\"4\">\n"
	L"%s"									// t2d_h
	L"        </td>\n"
	L"      </tr>\n"
	L"    </thead>\n"
	L"    <tbody class=\"t2d_tbody\">\n"
	L"%s"									// array of t2d_x
	L"    </tbody>\n"
	L"    <tfoot class=\"t2d_tfoot\">\n"
	L"      <tr>\n"
	L"        <td class=\"t2d_tfoot_td\" colspan=\"4\">\n"
	L"%s"									// t2d_f
	L"        </td>\n"
	L"      </tr>\n"
	L"    </tfoot>\n"
	L"  </table>\n"
	L"</div>\n"
	);

// css-3 is in the process of defining a tab-size: property
// http://www.w3.org/TR/css3-text/#tab-size1
// which i'm going to try to use here.  so it may not work
// on all browsers.

#define TOK__CSS_TABSIZE		L"%TABSIZE%"
#define TOK__CSS_PATH_FONTSIZE	L"%PATHFONTSIZE%"
#define TOK__CSS_STAT_FONTSIZE	L"%STATFONTSIZE%"
#define TOK__CSS_CODE_FONTSIZE	L"%CODEFONTSIZE%"

static const wchar_t * gsz_t2d_css = (		// css info for t2d
	L"  .t2d_h { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-bottom: 1px solid rgb(216,216,216); } \n"
	L"  .t2d_h_x { }\n"
	L"  .t2d_h_x_prefix { width: 1px; padding-right: 8px; font-weight: bold; font-size: "  TOK__CSS_PATH_FONTSIZE  L"pt; }\n"
	L"  .t2d_h_x_path { font-weight: bold; font-size: "  TOK__CSS_PATH_FONTSIZE  L"pt; }\n"
	L"  .t2d_h_x_dtm { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"
	L"  .t2d_h_stats { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	L"  .t2d_f { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-top: 1px solid rgb(216,216,216); }\n"
	L"  .t2d_f_dops { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	L"  .t2d_ln   { width: 1px; padding-left: 8px; padding-right: 8px; text-align: right; vertical-align: text-top; }\n"
	L"  .t2d_char { width: 1px; vertical-align: text-top; }\n"
	L"  .t2d_code { vertical-align: text-top;\n"
	L"              white-space: pre-wrap;\n"
	L"              tab-size:      "  TOK__CSS_TABSIZE L";\n"
	L"              -moz-tab-size: "  TOK__CSS_TABSIZE L";\n"
	L"              -o-tab-size:   "  TOK__CSS_TABSIZE L";\n"
	L"            }\n"


	L"  .t2d_x_atat      { background-color: rgb(240,240,255);   border-top: 1px solid rgb(183,183,216); color: rgb(168,168,168); }\n"
	L"  .t2d_x_atat_ln   { background-color: rgb(216,216,255); border-right: 1px solid rgb(183,183,216); text-align: center; }\n"
	L"  .t2d_x_atat_data { }\n"

	L"  .t2d_x_add_gap  { background-color: rgb(190,233,190); border-top: 1px solid rgb(161,204,161); }\n"
	L"  .t2d_x_add      { background-color: rgb(190,233,190); }\n"
	L"  .t2d_x_add_ln   { background-color: rgb(170,216,170); border-right: 1px solid rgb(161,204,161); }\n"

	L"  .t2d_x_del_gap  { background-color: rgb(233,190,190); border-top: 1px solid rgb(204,161,161); }\n"
	L"  .t2d_x_del      { background-color: rgb(233,190,190); }\n"
	L"  .t2d_x_del_ln   { background-color: rgb(216,170,170); border-right: 1px solid rgb(204,161,161); }\n"

#if WANT_DASHES
	L"  .t2d_x_dash      { background-color: rgb(240,240,240); color: rgb(128,128,128); }\n"
	L"  .t2d_x_dash_ln   { background-color: rgb(216,216,216); border-right: 1px solid rgb(183,183,183); }\n"
	L"  .t2d_x_dash_data { }\n"
#endif

	L"  .t2d_x_del_il   { background-color: rgb(255,170,170); font-weight: bold; }\n"
	L"  .t2d_x_add_il   { background-color: rgb(170,255,170); font-weight: bold; }\n"
	L"  .t2d_x_unimp_il { color: rgb(150,150,150); }\n"

	L"  .t2d { }\n"
	L"  .t2d_t { width: 100%; border: 1px solid rgb(204,204,204); border-spacing: 0px; border-collapse: collapse; font-family: Courier New,monospace; font-size: "  TOK__CSS_CODE_FONTSIZE  L"pt; }\n"
	L"  .t2d_thead { }\n"
	L"  .t2d_thead_td { padding: 0px; }\n"
	L"  .t2d_tbody { }\n"
	L"  .t2d_tfoot { }\n"
	L"  .t2d_tfoot_td { padding: 0px; }\n"

	);

static const wchar_t * gsz_t2 = (
	L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
	L"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	L"  <head>\n"
	L"    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
	L"    <style type=\"text/css\">\n"
	L"%s"									// t2d_css
	L"    </style>\n"
	L"    <title>\n"
	L"      SourceGear DiffMerge Traditional Diff\n"
	L"    </title>\n"
	L"  </head>\n"
	L"  <body>\n"
	L"    <div>\n"
	L"%s"			// t2d containing complete  diff, headers, and footers
	L"    </div>\n"
	L"  </body>\n"
	L"</html>\n"
	);

//////////////////////////////////////////////////////////////////

wxString html_diff_escape_string(const wxString & strInput,
								 bool bGlobalRespectEOL)
{
	wxString s(strInput);

	s.Replace(_T("&"), _T("&amp;"), true);
	s.Replace(_T("<"), _T("&lt;"),  true);
	s.Replace(_T(">"), _T("&gt;"),  true);

	if (bGlobalRespectEOL)
	{
		// When we are respecting EOLs we get the CR/LF chars
		// in the main body of the line string.  Writing them
		// out unescaped can cause problems for our HTML since
		// we are using a "white-space: pre-wrap" for the content
		// and they behave like <br>'s.
		//
		// Escape them using the special unicode symbol chars
		// in the control symbols block.  They are a little
		// ugly, but do show up.  And highlight nicely when
		// intra-line is on and you have a LF vs CRLF in the files.
		//
		// This mode probably isn't often, but it the only
		// real way to see where the EOLs are messed up in a file.

		s.Replace(_T("\x000d"), _T("&#x240D;"), true);
		s.Replace(_T("\x000a"), _T("&#x240A;"), true);
	}
	else
	{
		// When we are stripping EOLs before analysis, we
		// should not get either in the content buffer.
		// I'm going to force remove them just in case because
		// we don't want to the <br> effect.
	
		s.Replace(_T("\x000d"), _T(""), true);
		s.Replace(_T("\x000a"), _T(""), true);
	}

	return s;
}

//////////////////////////////////////////////////////////////////

static wxString _t2d_h_x(const wxChar * szPrefix,
					   poi_item * pPoiItem,
					   wxString * pStrLabel=NULL)
{
	// NOTE we assume that there are no '<', '>', or '&' in the
	// given prefix and in the generated DTM strings.

	wxDateTime dtm;
	pPoiItem->getDateTimeModified(&dtm);
	wxString strDTM = util_string__format_unified_date_from_dtm(dtm, false, false);

	wxString strPath;
	if (pStrLabel)
		strPath = *pStrLabel;
	else
		strPath = pPoiItem->getFullPath();
	wxString strPathEsc = html_diff_escape_string(strPath);

	return wxString::Format( gsz_t2d_h_x,
							 szPrefix, strPathEsc.wc_str(), strDTM.wc_str() );
}

static wxString _t2d_h_stats(const de_stats2 * pStats)
{
	wxString strStats = pStats->formatStatsString();
	wxString strStatsEsc = html_diff_escape_string(strStats);

	return wxString::Format( gsz_t2d_h_stats, strStatsEsc.wc_str() );

}

static wxString _t2d_h(const de_de * pDeDe, long kSync,
					 wxString * pStrLabelA, wxString * pStrLabelB)
{
	wxString str0( _t2d_h_x( L"&lt;", pDeDe->getFsFs()->getPoi(kSync,PANEL_T0), pStrLabelA) );
	wxString str1( _t2d_h_x( L"&gt;", pDeDe->getFsFs()->getPoi(kSync,PANEL_T1), pStrLabelB) );
	wxString strS( _t2d_h_stats( pDeDe->getStats2(kSync)) );

	return wxString::Format( gsz_t2d_h,
							 str0.wc_str(), str1.wc_str(), strS.wc_str() );
}

static wxString _t2d_f(const de_de * pDeDe, long kSync, int colTabWidth)
{
	wxString strModeString = pDeDe->getDisplayModeString(kSync, colTabWidth);
	wxString strModeStringEsc = html_diff_escape_string(strModeString);

	return wxString::Format( gsz_t2d_f,
							 strModeStringEsc );
}

static wxString _t2d_x_content(const de_row & rDeRowK, PanelIndex kPanel,
							   bool bIgnoreUnimportant,
							   bool bGlobalRespectEOL)
{
	wxString strThisLineEsc;
	const de_line * pDeLine = rDeRowK.getPanelLine(kPanel);

	// caller only calls us when this side has content (!eof, !void)
	wxASSERT_MSG( (pDeLine), _T("Coding Error") );
	if (!pDeLine)
		return strThisLineEsc;

	const de_sync * pDeSync = rDeRowK.getSync();
	if (pDeSync->haveIntraLineSyncInfo())
	{
		long                 offsetInSync  = rDeRowK.getOffsetInSync();
		const de_sync_list * pDeSyncListIL = pDeSync->getIntraLineSyncList(offsetInSync);
		const de_sync *      pDeSyncIL;
					
		wxString strLine( pDeLine->getStrLine() );
		if (bGlobalRespectEOL)					// if we included EOLs in the match, then there will be
			strLine += pDeLine->getStrEOL();	// intra-line sync nodes indexing past the end of the regular line.

		for (pDeSyncIL=pDeSyncListIL->getHead(); (pDeSyncIL && !pDeSyncIL->isEND()); pDeSyncIL=pDeSyncIL->getNext())
		{
			long len = pDeSyncIL->getLen(kPanel);
			if (len > 0)
			{
				de_attr attr_il = pDeSyncIL->getAttr();
				wxString strContent(strLine.wc_str() + pDeSyncIL->getNdx((PanelIndex)kPanel), len);
				wxString strContentEsc( html_diff_escape_string(strContent, bGlobalRespectEOL) );

				const wchar_t * szTemplate = NULL;
				if (DE_ATTR__IS_TYPE(attr_il, DE_ATTR_DIF_0EQ))
				{
					if (bIgnoreUnimportant && DE_ATTR__IS_SET(attr_il, DE_ATTR_UNIMPORTANT))
						szTemplate = gsz_t2d_x1_unimp_il[kPanel];
					else
						szTemplate = gsz_t2d_x1_il[kPanel];

					wxString strThisSpan;
					strThisSpan.Printf( szTemplate, strContentEsc );
					strThisLineEsc += strThisSpan;
				}
				else
				{
					// we don't need to check for DE_ATTR_DIF_2EQ
					// since we won't get any in a diffs-only view.
					//
					// So the following probably isn't necessary,
					// but it doesn't hurt.
					//
					// Don't waste a <span> on normal content
					strThisLineEsc += strContentEsc;
				}
			}
		}
	}
	else
	{
		wxString strThisLine;
		const fl_line * pFlLine = pDeLine->getFlLine();
		pFlLine->buildStringsFromRuns(bGlobalRespectEOL, &strThisLine  /*,&strThisEol*/);
		strThisLineEsc = html_diff_escape_string(strThisLine, bGlobalRespectEOL);
	}

	// TODO 2013/09/03 think about whether to do something with this
	// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
	//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");

	return strThisLineEsc;
}

static wxString _t2d_array(de_de * pDeDe, long kSync)
{
	wxString strResult;		// to accumulate detail lines

	de_display_ops dops = pDeDe->getDisplayOps(kSync);
	bool bIgnoreUnimportant = DE_DOP__IS_SET_IGN_UNIMPORTANT(dops);
	bool bGlobalRespectEOL = RS_ATTRS_RespectEOL( pDeDe->getFsFs()->getRuleSet()->getMatchStripAttrs() );

	TVector_Display * pDis = pDeDe->getDisplayList(kSync);	// forces run() if necessary
	long nrRows = (long)pDis->size() - 1;

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
		unified_diff__map_rows_to_line_numbers(pDeDe, pDis, kSync, kRowStart, kRowEnd, &lnm);

		// compute the gnu-style change header

		wxString strAtAt = _traditional_diff__format_change_header(lnm);
		wxString strAtAtDetail;
		strAtAtDetail.Printf( gsz_t2d_x_atat, strAtAt );
		
		// compute the body of each side of the change

		wxString strLines[2];

		for (long kRow=kRowStart; (kRow<kRowEnd); kRow++)
		{
			const de_row & rDeRowK = (*pDis)[kRow];
			const de_sync * pSyncK = rDeRowK.getSync();
			wxASSERT_MSG( (pSyncK->isSameType(DE_ATTR_DIF_0EQ)), _T("Coding Error") );

			for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
			{
				const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
				if (pDeLine)
				{
					wxString strContentEsc( _t2d_x_content(rDeRowK, (PanelIndex)kPanel,
														   bIgnoreUnimportant, bGlobalRespectEOL) );

					const fl_line * pFlLine = pDeLine->getFlLine();
					long ln = (long)pFlLine->getLineNr() + 1;

					wxString strThisLineDetail;
					if (rDeRowK.haveGap())
						strThisLineDetail.Printf( gsz_t2d_x1_gap[kPanel], ln, strContentEsc );
					else
						strThisLineDetail.Printf( gsz_t2d_x1[kPanel], ln, strContentEsc );

					strLines[kPanel] += strThisLineDetail;
				}
			}
		}

		// now put it all together into output buffer

		strResult += strAtAtDetail;
		strResult += strLines[PANEL_T0];
#if WANT_DASHES
		if ((lnm.len[PANEL_T0] > 0) && (lnm.len[PANEL_T1] > 0))
			strResult += gsz_t2d_x_dash;
#endif
		strResult += strLines[PANEL_T1];

		// set up for start of next chunk

		kRowStart = kRowEnd;
	}
	
	return strResult;
}

wxString de_de::batchoutput_html_traditional_diff2__div(long kSync,
														bool * pbHadChanges,
														int colTabWidth,
														wxString * pStrLabelA,
														wxString * pStrLabelB)
{
	wxString strDiv;

	de_display_ops dopsOld = m_dops[kSync];

	// force diff-only mode and rebuild the display list if necessary.
	// we pick up the ignore-unimportant.
	// We don't bother with the hide-omitted, since they won't
	// show up in a diff-only diff anyway.
	de_display_ops dopsNew = DE_DOP_DIF;
	if (DE_DOP__IS_SET_IGN_UNIMPORTANT(dopsOld))
		dopsNew |= DE_DOP_IGN_UNIMPORTANT;
	setDisplayOps(kSync, dopsNew);
	run();
	
	const de_stats2 * pStats = getStats2(kSync);
	long cChanges = pStats->m_cImportantChanges;	// + pStats->m_cUnimportantChanges;

	*pbHadChanges = (cChanges > 0);
	
	wxString str_t2d_h( _t2d_h(this, kSync, pStrLabelA, pStrLabelB) );
	wxString str_t2d_f( _t2d_f(this, kSync, colTabWidth) );
	wxString str_t2d_array( _t2d_array(this, kSync) );

	strDiv = wxString::Format( gsz_t2d,
							   str_t2d_h.wc_str(),
							   str_t2d_array.wc_str(),
							   str_t2d_f.wc_str() );

	wxLogTrace(wxTRACE_Messages, _T("t2d:div: [%s]\n"), strDiv.wc_str());

	// Restore users display options.  Really only needed if interactive.
	setDisplayOps(kSync, dopsOld);
	run();

	return strDiv;
}

/*static*/ wxString de_de::batchoutput_html_traditional_diff2__css(int colTabWidth)
{
	wxString strCSS(gsz_t2d_css);

	strCSS.Replace( TOK__CSS_TABSIZE,       wxString::Format("%d", colTabWidth), true);

	// TODO 2013/09/03 Think about using a global prop for the font sizes.
	strCSS.Replace( TOK__CSS_PATH_FONTSIZE, wxString::Format("%d", 10), true);
	strCSS.Replace( TOK__CSS_STAT_FONTSIZE, wxString::Format("%d", 8), true);
	strCSS.Replace( TOK__CSS_CODE_FONTSIZE, wxString::Format("%d", 10), true);

	wxLogTrace(wxTRACE_Messages, _T("t2d:css: [%s]\n"), strCSS.wc_str());

	return strCSS;
}

void de_de::batchoutput_html_traditional_diff2(long kSync,
										   wxString & strOut,
										   bool * pbHadChanges,
										   int colTabWidth,
										   wxString * pStrLabelA,
										   wxString * pStrLabelB)
{
	wxString str_css( batchoutput_html_traditional_diff2__css(colTabWidth) );
	wxString str_t2d( batchoutput_html_traditional_diff2__div(kSync, pbHadChanges, colTabWidth,
															  pStrLabelA, pStrLabelB) );
	
	strOut.Printf( gsz_t2, str_css, str_t2d );

}
