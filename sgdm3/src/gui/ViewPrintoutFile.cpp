// ViewPrintoutFile.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <fl.h>
#include <de.h>
#include <rs.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

ViewPrintoutFileDiff::ViewPrintoutFileDiff(ViewFileDiff * pView, ViewPrintoutFileData * pData)
	: ViewPrintoutFile( _("File Comparison"), pView, pData)
{
}

ViewPrintoutFileMerge::ViewPrintoutFileMerge(ViewFileMerge * pView, ViewPrintoutFileData * pData)
	: ViewPrintoutFile( _("File Comparison"), pView, pData)
{
}

//////////////////////////////////////////////////////////////////

ViewPrintoutFile::ViewPrintoutFile(const wxString & strTitle, ViewFile * pViewFile, ViewPrintoutFileData * pData)
	: ViewPrintout(strTitle),
	  m_pView(pViewFile),
	  m_cRowsPerPage(0),
	  m_cPages(0)
{
	pData = NULL;

	m_kSync = pViewFile->getCurrentNBSync();
}

ViewPrintoutFile::~ViewPrintoutFile(void)
{
}

//////////////////////////////////////////////////////////////////

void ViewPrintoutFile::OnPreparePrinting(void)
{
	// called ONCE before anything happens so that we can
	// calculate the number of pages, for example.
	
	//wxLogTrace(wxTRACE_Messages, _T("OnPreparePrinting:"));

	wxDC * dc = GetDC();

	// calibrate coordinate system (technically, this is DC specific
	// and can change (if they use the preview zoom)), but we only
	// need it to measure the height of the page header/footer (also
	// DC specific), but relative.  and then compute the number of
	// detail lines that will be visible on each full page (which
	// should be constant (because of the scaling of the font).
	
	initializeCoordinateSystem(dc);

	wxFont * pFont;
	allocateFonts(GlobalProps::GPS_PRINTER_FILE_FONT,&pFont);
	
	dc->SetFont(*pFont);

	// we compute the body area here -- into local variables -- and
	// use them to compute the rows-per-page and number-of-pages.
	// these are global and must be constant throughout the printing
	// or preview (and independent of zoom factor).
	//
	// as we print each page we will re-compute the body area so that
	// we get the pixels right -- but we do not recompute the rows-per-page
	// -- this helps avoid oddities and round-off errors with fonts.

	int yCoordBodyTop, yCoordBodyBottom;
	int xCoordBodyLeft, xCoordLineNrRight;

	drawPageHeaderFooter(dc,1,0,
						 _("File Comparison"),
						 _T("X"),
						 m_pView->getDE()->getStats2Msg(m_kSync),
						 true,
						 &yCoordBodyTop,&yCoordBodyBottom,
						 &xCoordBodyLeft,&xCoordLineNrRight);

	wxCoord wText,hText;
	dc->GetTextExtent( _T("X"),&wText,&hText);

	m_cRowsPerPage = (yCoordBodyBottom - yCoordBodyTop) / hText;
	m_cPages       = ((int)m_pView->getDE()->getDisplayList(m_kSync)->size() + m_cRowsPerPage - 1) / m_cRowsPerPage;

	//wxLogTrace(wxTRACE_Messages,_T("cRowsPerPage: [%d] cPages: [%d]"),m_cRowsPerPage,m_cPages);

	// for now we do n-pages across

	m_cPages *= m_pView->getNrTopPanels();

	m_bShowLineNumbers = m_pView->getShowLineNumbers();
	for (int kk=0; kk<m_pView->getNrTopPanels(); kk++)
		m_cDigitsLineNumbers[kk] = m_pView->getPanel(m_kSync,(PanelIndex)kk)->getDigitsLineNr();

	delete pFont;
}

void ViewPrintoutFile::GetPageInfo(int * minPage, int * maxPage, int * pageFrom, int * pageTo)
{
	//wxLogTrace(wxTRACE_Messages, _T("GetPageInfo: [pages %d] "),m_cPages);

	*minPage = 1;
	*maxPage = m_cPages;
	*pageFrom = 1;		// what are these for ???
	*pageTo = 1;		// what are these for ???
}

//////////////////////////////////////////////////////////////////

void ViewPrintoutFile::OnBeginPrinting(void)
{
	// called once per print job
	//wxLogTrace(wxTRACE_Messages, _T("OnBeginPrinting:"));
}

void ViewPrintoutFile::OnEndPrinting(void)
{
	// called once per print job
	//wxLogTrace(wxTRACE_Messages, _T("OnEndPrinting:"));
}

//////////////////////////////////////////////////////////////////

bool ViewPrintoutFile::OnBeginDocument(int startPage, int endPage)
{
	// called once per copy
	//wxLogTrace(wxTRACE_Messages, _T("OnBeginDocument: [%d,%d]"), startPage,endPage);
	return wxPrintout::OnBeginDocument(startPage,endPage);
}

void ViewPrintoutFile::OnEndDocument(void)
{
	// called once per copy
	//wxLogTrace(wxTRACE_Messages, _T("OnEndDocument:"));
	wxPrintout::OnEndDocument();
}

//////////////////////////////////////////////////////////////////

bool ViewPrintoutFile::HasPage(int kPage)
{
	// kPage is 1-based.
	
	// wxLogTrace(wxTRACE_Messages, _T("HasPage: [%d / %d]"), kPage, m_cPages);

	return (kPage <= m_cPages);
}

//////////////////////////////////////////////////////////////////

static wxFont * _select_font(de_attr attr, wxFont * pFont, wxFont * pFontBold, wxFont * pFontBoldUnderline)
{
	switch (attr & DE_ATTR__TYPE_MASK)
	{
	default:
	case DE_ATTR_UNKNOWN:
	case DE_ATTR_EOF:
	case DE_ATTR_OMITTED:
	case DE_ATTR_DIF_2EQ:
	case DE_ATTR_MRG_3EQ:
		return pFont;

	case DE_ATTR_DIF_0EQ:
	case DE_ATTR_MRG_T0T2EQ:
	case DE_ATTR_MRG_T1T2EQ:
	case DE_ATTR_MRG_T1T0EQ:
		return pFontBold;

	case DE_ATTR_MRG_0EQ:
		return pFontBoldUnderline;
	}
}

//////////////////////////////////////////////////////////////////

bool ViewPrintoutFile::OnPrintPage(int kPageGiven)
{
	// kPageGiven is 1-based.

	int kPage, kPanel;
	int nrPanels = m_pView->getNrTopPanels();

	if (gpGlobalProps->getBool(GlobalProps::GPL_MISC_PRINT_ACROSS))
	{
		// do page 1a,1b[,1c],2a,2b...

		kPage  = (kPageGiven-1) / nrPanels  + 1;
		kPanel = (kPageGiven-1) % nrPanels;
	}
	else
	{
		// do page 1a,2a,...,1b,2b,...,1c,2c,...

		kPage  = (kPageGiven-1) % (m_cPages/nrPanels)  + 1;
		kPanel = (kPageGiven-1) / (m_cPages/nrPanels);
	}
	//wxLogTrace(wxTRACE_Messages, _T("OnPrintPage: [%d] --> [page %d%c]"), kPageGiven, kPage, (L"abc")[kPanel]);

	//////////////////////////////////////////////////////////////////

	wxDC * dc = GetDC();

	// we must initialize the coordinate system scale factor on each
	// page (because the DC can change (when zooming) between calls).
	// likewise, fonts are DC-scale-dependent, we we must create/free
	// them each time.
	
	initializeCoordinateSystem(dc);

	wxFont * pFont, *pFontBold, *pFontBoldUnderline;
	allocateFonts(GlobalProps::GPS_PRINTER_FILE_FONT,&pFont,&pFontBold,&pFontBoldUnderline);
	
	dc->SetFont(*pFont);

	//////////////////////////////////////////////////////////////////
	// we do use colors but we don't fill in text backgrounds.
	//////////////////////////////////////////////////////////////////

	dc->SetBackgroundMode(wxTRANSPARENT);

	wxPen penRule( wxColor(0x00,0x00,0x00), 1, wxSOLID);
	wxPen penNone( wxColor(0x00,0x00,0x00), 1, wxTRANSPARENT);

	wxBrush brushVoid(gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_VOID_FG), wxBDIAGONAL_HATCH);	// hatched brush for filling voids

	//////////////////////////////////////////////////////////////////
	
	int yCoordBodyTop, yCoordBodyBottom;
	int xCoordBodyLeft, xCoordLineNrRight;

	wxString strColumnTitle = m_pView->getTitle(kPanel);
	if (strColumnTitle.Length() == 0)
		strColumnTitle = m_pView->getDoc()->getFsFs()->getPoi(m_kSync,(PanelIndex)kPanel)->getFullPath();

	wxString strFooter;
	if (kPanel == m_pView->getNrTopPanels()-1)
		strFooter = m_pView->getDE()->getStats2Msg(m_kSync);
	else if (kPanel == 0)
		strFooter = _getModeString();

	dc->SetPen(penRule);
	dc->SetBrush(*wxTRANSPARENT_BRUSH);
	dc->SetTextForeground( wxColor(0x00,0x00,0x00) );
	drawPageHeaderFooter(dc,kPage,kPanel,
						 _("File Comparison"),
						 strColumnTitle,
						 strFooter,
						 false,
						 &yCoordBodyTop,&yCoordBodyBottom,
						 &xCoordBodyLeft,&xCoordLineNrRight);

	//////////////////////////////////////////////////////////////////

	const TVector_Display * pDis     = m_pView->getDE()->getDisplayList(m_kSync);

	const rs_ruleset * pRS           = m_pView->getDoc()->getFsFs()->getRuleSet();
	rs_context_attrs matchStripAttrs = pRS->getMatchStripAttrs();
	bool bGlobalRespectEOL           = RS_ATTRS_RespectEOL(matchStripAttrs);

	int  cTabStop                    = m_pView->getTabStop();
	bool bMerge                      = (m_pView->getNrTopPanels() == 3);

	//////////////////////////////////////////////////////////////////
	// draw cRowsPerPage detail lines within the body area.
	// because of round-off problems with the coordinate system and
	// font metrics as the zoom factor changes, we cannot space these
	// using the height reported by GetTextExtent().  rather, we need
	// to proportionally place them using this macro.

#define YROW(kRow)	(yCoordBodyTop + ((yCoordBodyBottom-yCoordBodyTop)*(kRow)/m_cRowsPerPage))

	int kStart = (kPage-1)*m_cRowsPerPage;
	int kLimit = MyMin(kStart+m_cRowsPerPage, (int)pDis->size());

	for (int kIndex=kStart, kRow=0; kIndex<kLimit; kIndex++, kRow++)	// kIndex is index into display list; kRow is row on page
	{
		int yCoord = YROW(kRow);

		const de_row & rDeRow = (*pDis)[kIndex];
		const de_line * pDeLine = rDeRow.getPanelLine((PanelIndex)kPanel);

		if (pDeLine)
		{
			const fl_line * pFlLine = pDeLine->getFlLine();

			//////////////////////////////////////////////////////////////////
			// set overall line color and font based upon line attributes
			//////////////////////////////////////////////////////////////////

			wxColour colorTextFg;
			if (bMerge)
			{
				_attr_to_color3_fg((PanelIndex)kPanel,rDeRow.getSync()->getAttr(),colorTextFg);
//				_attr_to_color3_bg(bIgnoreUnimportantChanges,m_kPanel,rDeRow.getSync()->getAttr(),colorTextBg);
			}
			else
			{
				_attr_to_color2_fg(rDeRow.getSync()->getAttr(),colorTextFg);
//				_attr_to_color2_bg(bIgnoreUnimportantChanges,rDeRow.getSync()->getAttr(),colorTextBg);
			}
			dc->SetTextForeground(colorTextFg);

			dc->SetFont( * (_select_font( rDeRow.getSync()->getAttr(), pFont, pFontBold, pFontBoldUnderline )) );

			//////////////////////////////////////////////////////////////////
			// draw line number if appropriate
			//////////////////////////////////////////////////////////////////

			if (m_bShowLineNumbers)
			{
				int lineNr = pFlLine->getLineNr();
				wxChar bufLineNr[32];
				::wxSnprintf(bufLineNr,NrElements(bufLineNr),_T("%d"),lineNr+1);
				wxString strLineNr(bufLineNr);
				wxCoord wLineNr, hLineNr;
				dc->GetTextExtent(strLineNr,&wLineNr,&hLineNr);
				dc->DrawText(strLineNr, xCoordLineNrRight-wLineNr, yCoord);
			}

			//////////////////////////////////////////////////////////////////
			// draw content for this line
			//////////////////////////////////////////////////////////////////
			
			wxCoord xCoord = xCoordBodyLeft;
			int col = 0;

			const de_sync * pDeSyncLine = rDeRow.getSync();
			if (pDeSyncLine->haveIntraLineSyncInfo())
			{
				//////////////////////////////////////////////////////////////////
				// we have intra-line highlight info -- use it (and the intra-line
				// sync list) to draw the line.
				//////////////////////////////////////////////////////////////////

				long                 offsetInSync  = rDeRow.getOffsetInSync();
				const de_sync_list * pDeSyncListIL = pDeSyncLine->getIntraLineSyncList(offsetInSync);
				const de_sync *      pDeSyncIL;
					
				wxString strLine( pDeLine->getStrLine() );
				if (bGlobalRespectEOL)					// if we included EOLs in the match, then there will be
					strLine += pDeLine->getStrEOL();	// intra-line sync nodes indexing past the end of the regular line.

				for (pDeSyncIL=pDeSyncListIL->getHead(); (pDeSyncIL && !pDeSyncIL->isEND()); pDeSyncIL=pDeSyncIL->getNext())
				{
					long len = pDeSyncIL->getLen((PanelIndex)kPanel);
					if (len > 0)
					{
						wxColor colorLocalFg;

						if (bMerge)
							_attr_to_color3_fg((PanelIndex)kPanel,pDeSyncIL->getAttr(),colorLocalFg);
						else
							_attr_to_color2_fg(pDeSyncIL->getAttr(),colorLocalFg);

						dc->SetTextForeground(colorLocalFg);
						dc->SetFont( * (_select_font( pDeSyncIL->getAttr(), pFont, pFontBold, pFontBoldUnderline )) );

						const wxChar * sz = strLine.wc_str() + pDeSyncIL->getNdx((PanelIndex)kPanel);
						_drawString(dc, sz,len, cTabStop, xCoord,yCoord,col);
					}
				}
			}
			else
			{
				//////////////////////////////////////////////////////////////////
				// no intra-line info -- use layout to draw the line.
				//////////////////////////////////////////////////////////////////

				for (const fl_run * pFlRun=pFlLine->getFirstRunOnLine(); (pFlRun && (pFlRun->getLine()==pFlLine)); pFlRun=pFlRun->getNext())
				{
					const wxChar * sz = pFlRun->getContent();
					long len = (long)pFlRun->getLength();
						
					if (pFlRun->isLF() || pFlRun->isCR())
						;
					else if (pFlRun->isTAB())
						_drawString(dc, sz,1, cTabStop, xCoord,yCoord,col);
					else
					{
						long begin = 0;
						long k     = begin;
						while (begin < len)
						{
							while ( (k < len) && (sz[k] != 0x0020) )
								k++;
							if (k != begin)
								_drawString(dc, &sz[begin],(k-begin), cTabStop, xCoord,yCoord,col);
							begin = k;

							while ( (k < len) && (sz[k] == 0x0020) )
								k++;
							if (k != begin)
								_drawString(dc, &sz[begin],(k-begin), cTabStop, xCoord,yCoord,col);
							begin = k;
						}
					}
				}
			}
		}
		else if (rDeRow.isEOF())
		{
			// TODO do we want to draw anything for this ?
		}
		else // a void 
		{
			dc->SetPen(penNone);
			dc->SetBrush(brushVoid);
			long h = (yCoordBodyBottom-yCoordBodyTop)/m_cRowsPerPage;	// reduces from: (YROW(kRow+1)-YROW(kRow))
			if (m_bShowLineNumbers)
				dc->DrawRectangle(0,yCoord, xCoordLineNrRight,h);
			dc->DrawRectangle(xCoordBodyLeft,yCoord, m_xCoordMax-xCoordBodyLeft,h);
		}

		// we are drawing row-by-row using the display list -- not the line list
		// in the document.  this allows us to show diffs-only or diffs-with-context.
		// if the row is marked "gapped", then the diff-engine hid some content
		// immediately prior to this row.  draw a line above our text (in the
		// leading above the characters).

		if (rDeRow.haveGap())
		{
			dc->SetPen(penRule);
			dc->DrawLine(0,yCoord,m_xCoordMax,yCoord);
		}
	}

	//////////////////////////////////////////////////////////////////

	dc->SetPen(wxNullPen);
	dc->SetBrush(wxNullBrush);
	
	delete pFont;
	delete pFontBold;
	delete pFontBoldUnderline;

	return true;
}

//////////////////////////////////////////////////////////////////

void ViewPrintoutFile::_drawString(wxDC * dc,
								   const wxChar * sz, long len,
								   int cTabStop,
								   wxCoord & xCoord, wxCoord yCoord, int & col)
{
	static const wxChar * gszBlanks = _T("                ");

	wxASSERT_MSG( (len > 0), _T("Coding Error!") );

	wxString strTemp;

	switch (*sz)
	{
	case 0x000a:
	case 0x000d:
		return;
		
	case 0x0009:
	case 0x0020:
		{
			for (long k=0; k<len; k++)
			{
				if (sz[k] == 0x0009)
				{
					long pad = cTabStop - (col % cTabStop);
					strTemp += wxString(gszBlanks,pad);
					col += pad;
				}
				else /* is 0x0020 */
				{
					strTemp += _T(' ');
					col++;
				}
			}
		}
		break;

	default:
		strTemp += wxString(sz,len);
		col += len;
		break;
	}

	dc->DrawText(strTemp,xCoord,yCoord);
	wxCoord wText,hText;
	dc->GetTextExtent(strTemp,&wText,&hText);
	xCoord += wText;
}

//////////////////////////////////////////////////////////////////

void ViewPrintoutFile::drawPageHeaderFooter(wxDC * dc, int kPage, int kPanel,
											wxString strPageTitle,
											wxString strCol1,
											wxString strFooter,
											bool bJustMeasure,
											int * pyCoordBodyTop, int * pyCoordBodyBottom,
											int * pxCoordBodyLeft, int * pxCoordLineNrRight)
{
	//////////////////////////////////////////////////////////////////
	// draw page header and footer and decoration lines
	//
	// +------------------------------------------+
	// |<app_name>     <report_title>    <page_nr>|
	// |                                          |
	// |------------------------------------------|
	// | <...file_k>                              |
	// |---+--------------------------------------|
	// |   |                                      |
	// |   |                                      |
	// |   |                                      |
	// |   |                                      |
	// |---+--------------------------------------|
	// |                                   <stats>|
	// +------------------------------------------+
	//
	// i wanted to place the stats on the very bottom of the page, but
	// at large zoom factors, it runs off the bottom of the page.  this
	// looks like round-off problems between the coordinate system and
	// the font metrics.  so instead, we do 1.5 times the font height.
	// then we place the line at 2 times.
	//////////////////////////////////////////////////////////////////

	static const wxChar * s_szLetters = L"abc";

	wxString strLeft	= VER_APP_TITLE;
	wxString strCenter	= strPageTitle;
	wxString strRight	= wxString::Format( _("Page: %d%c / %d"), kPage, s_szLetters[kPanel], m_cPages / m_pView->getNrTopPanels());

	wxCoord wTextCenter,hText;
	dc->GetTextExtent(strCenter,&wTextCenter,&hText);

	int yCoordRowTitles     = 0;
	int yCoordRowTopLine1   = hText*3;
	int yCoordRowColTitles  = hText*3 + hText/2;
	int yCoordRowTopLine2   = hText*5;
	int yCoordRowBodyTop    = hText*5 + hText/2;

	int yCoordRowStats      = m_yCoordMax - (hText*3/2);
	int yCoordRowBottomLine = m_yCoordMax - hText*2;
	int yCoordRowBodyBottom = yCoordRowBottomLine;

	int wCoordLineNr        = 0;
	int xCoordDivider       = 0;
	int xCoordBodyLeft      = 0;
	int xCoordLineNrRight   = 0;
	
	if (m_bShowLineNumbers)
	{
		wxCoord wTextX;
		dc->GetTextExtent(_T("X"),&wTextX,&hText);

		wCoordLineNr        = wTextX * m_cDigitsLineNumbers[kPanel];
		xCoordLineNrRight   = wCoordLineNr      + wTextX/2;
		xCoordDivider       = xCoordLineNrRight + wTextX/2;
		xCoordBodyLeft      = xCoordDivider     + wTextX/2;
	}

	if (!bJustMeasure)
	{
		dc->DrawText(strLeft,0,yCoordRowTitles);
		dc->DrawText(strCenter,(m_xCoordMax-wTextCenter)/2,yCoordRowTitles);
		wxCoord wTextRight;
		dc->GetTextExtent(strRight,&wTextRight,&hText);
		dc->DrawText(strRight,(m_xCoordMax-wTextRight),yCoordRowTitles);

		wxString strFile1 = _elide(strCol1, m_xCoordMax, dc);
		dc->DrawText(strFile1,0,yCoordRowColTitles);

		if (strFooter.Length() > 0)
		{
			wxCoord wTextFooter;
			dc->GetTextExtent(strFooter,&wTextFooter,&hText);
			dc->DrawText(strFooter,(m_xCoordMax-wTextFooter), yCoordRowStats);
		}

		dc->DrawLine(0,yCoordRowTopLine1,             m_xCoordMax,yCoordRowTopLine1);
		dc->DrawLine(0,yCoordRowTopLine2,             m_xCoordMax,yCoordRowTopLine2);
		dc->DrawLine(0,yCoordRowBottomLine,           m_xCoordMax,yCoordRowBottomLine);

		dc->DrawLine(xCoordDivider,yCoordRowTopLine2, xCoordDivider,yCoordRowBottomLine);
	}

	*pyCoordBodyTop     = yCoordRowBodyTop;
	*pyCoordBodyBottom  = yCoordRowBodyBottom;
	*pxCoordBodyLeft    = xCoordBodyLeft;
	*pxCoordLineNrRight = xCoordLineNrRight;
}

//////////////////////////////////////////////////////////////////

wxString ViewPrintoutFile::_getModeString(void)
{
	return m_pView->getDE()->getDisplayModeString(m_kSync,
												  m_pView->getTabStop());
}

