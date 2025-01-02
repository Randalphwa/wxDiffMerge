// ViewPrintoutFolder.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////
// a macro to draw an unfilled rectangle.  dc->DrawRectangle() is
// broken on GTK -- the generated PS is always filled black even
// though we install a transparent brush.
	
#define DRECT(x0,y0,x1,y1)	Statement( dc->DrawLine(x0,y0, x0,y1); \
									   dc->DrawLine(x0,y1, x1,y1); \
									   dc->DrawLine(x1,y1, x1,y0); \
									   dc->DrawLine(x1,y0, x0,y0); )

//////////////////////////////////////////////////////////////////

ViewPrintoutFolder::ViewPrintoutFolder(ViewFolder * pView, ViewPrintoutFolderData * pData)
	: ViewPrintout( _("Folder Comparison") ),
	  m_pView(pView),
	  m_cRowsPerPage(0),
	  m_cPages(0)
{
	if (pData)
	{
		m_pFolderData = pData;
		m_pFolderData->addRef();
	}
	else
	{
		m_pFolderData = new ViewPrintoutFolderData(pView->getDoc()->getFdFd());
	}

	//wxLogTrace(wxTRACE_Messages, _T("ViewPrintoutFolder: ctor %p"), this);
}

ViewPrintoutFolder::~ViewPrintoutFolder(void)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewPrintoutFolder: dtor %p"), this);

	m_pFolderData->delRef();
	m_pFolderData = NULL;
}

//////////////////////////////////////////////////////////////////

void ViewPrintoutFolder::OnPreparePrinting(void)
{
	// called ONCE before anything happens so that we can
	// calculate the number of pages, for example.
	
	//wxLogTrace(wxTRACE_Messages, _T("OnPreparePrinting: %p"), this);

	wxDC * dc = GetDC();

	// calibrate coordinate system (technically, this is DC specific
	// and can change (if they use the preview zoom)), but we only
	// need it to measure the height of the page header/footer (also
	// DC specific), but relative.  and then compute the number of
	// detail lines that will be visible on each full page (which
	// should be constant (because of the scaling of the font).
	
	initializeCoordinateSystem(dc);

	wxFont * pFont, *pFontBold;
	allocateFonts(GlobalProps::GPS_PRINTER_FOLDER_FONT,&pFont,&pFontBold);
	
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
	
	drawPageHeaderFooter(dc,1,
						 _("Folder Comparison"),
						 _T("X"),_T("X"),
						 m_pFolderData->getStats(),
						 true,
						 &yCoordBodyTop,&yCoordBodyBottom);

	wxCoord wText,hText;
	dc->GetTextExtent( _T("X"),&wText,&hText);

	m_cRowsPerPage = (yCoordBodyBottom - yCoordBodyTop) / hText;
	m_cPages       = (m_pFolderData->getItemCount() + m_cRowsPerPage - 1) / m_cRowsPerPage;
	if (m_cPages < 1)	// wx2.9 doesn't like 0 (at least on linux)
		m_cPages = 1;

	//wxLogTrace(wxTRACE_Messages,_T("cRowsPerPage: [%d] cPages: [%d]"),m_cRowsPerPage,m_cPages);

	delete pFont;
	delete pFontBold;
}

void ViewPrintoutFolder::GetPageInfo(int * minPage, int * maxPage, int * pageFrom, int * pageTo)
{
	//wxLogTrace(wxTRACE_Messages, _T("GetPageInfo: %p [count %ld] "), this, m_pFolderData->getItemCount());

	*minPage = 1;
	*maxPage = m_cPages;
	*pageFrom = 1;		// what are these for ???
	*pageTo = 1;		// what are these for ???
}

//////////////////////////////////////////////////////////////////

void ViewPrintoutFolder::OnBeginPrinting(void)
{
	// called once per print job
	//wxLogTrace(wxTRACE_Messages, _T("OnBeginPrinting: %p"), this);
}

void ViewPrintoutFolder::OnEndPrinting(void)
{
	// called once per print job
	//wxLogTrace(wxTRACE_Messages, _T("OnEndPrinting: %p"), this);
}

//////////////////////////////////////////////////////////////////

bool ViewPrintoutFolder::OnBeginDocument(int startPage, int endPage)
{
	// called once per copy
	//wxLogTrace(wxTRACE_Messages, _T("OnBeginDocument: %p [%d,%d]"), this, startPage,endPage);
	return wxPrintout::OnBeginDocument(startPage,endPage);
}

void ViewPrintoutFolder::OnEndDocument(void)
{
	// called once per copy
	//wxLogTrace(wxTRACE_Messages, _T("OnEndDocument: %p"), this);
	wxPrintout::OnEndDocument();
}

//////////////////////////////////////////////////////////////////

bool ViewPrintoutFolder::HasPage(int kPage)
{
	// kPage is 1-based.
	
	// wxLogTrace(wxTRACE_Messages, _T("HasPage: %p [%d]"), this, kPage);

	return (kPage <= m_cPages);
}

//////////////////////////////////////////////////////////////////

bool ViewPrintoutFolder::OnPrintPage(int kPage)
{
	// kPage is 1-based.

	//wxLogTrace(wxTRACE_Messages, _T("OnPrintPage: %p [%d]"), this, kPage);

	wxDC * dc = GetDC();

	// we must initialize the coordinate system scale factor on each
	// page (because the DC can change (when zooming) between calls).
	// likewise, fonts are DC-scale-dependent, we we must create/free
	// them each time.
	
	initializeCoordinateSystem(dc);

	wxFont * pFont, *pFontBold;
	allocateFonts(GlobalProps::GPS_PRINTER_FOLDER_FONT,&pFont,&pFontBold);
	
	dc->SetFont(*pFont);

	//////////////////////////////////////////////////////////////////
	// for now, let's assume black & white output
	//////////////////////////////////////////////////////////////////

	dc->SetPen(*wxBLACK_PEN);
	dc->SetBackgroundMode(wxTRANSPARENT);
	dc->SetBrush(*wxTRANSPARENT_BRUSH);
	dc->SetTextForeground( wxColor(0x00,0x00,0x00) );

	//////////////////////////////////////////////////////////////////
	
	int yCoordBodyTop, yCoordBodyBottom;

	drawPageHeaderFooter(dc,kPage,
						 _("Folder Comparison"),
						 m_pView->getDoc()->getFdFd()->getRootPoi(0)->getFullPath(),
						 m_pView->getDoc()->getFdFd()->getRootPoi(1)->getFullPath(),
						 m_pFolderData->getStats(),
						 false,
						 &yCoordBodyTop,&yCoordBodyBottom);

	//////////////////////////////////////////////////////////////////
	// determine column widths

	int wImage,hImage;
	gpViewFolderImageList->GetSize(0,wImage,hImage);

	int xCoordGap = (m_xCoordPerInch / 4);

	int xCoordCol0 = 0;									// draw image at left margin
	int xCoordCol1 = wImage + xCoordGap;
	int xCoordCol2 = (m_xCoordMax + xCoordGap) / 2;

	int wCoordCol1 = ((m_xCoordMax - xCoordGap) / 2) - xCoordCol1;
	int wCoordCol2 = m_xCoordMax - xCoordCol2;

	//////////////////////////////////////////////////////////////////
	// draw cRowsPerPage detail lines within the body area.
	// because of round-off problems with the coordinate system and
	// font metrics as the zoom factor changes, we cannot space these
	// using the height reported by GetTextExtent().  rather, we need
	// to proportionally place them using this macro.

#define YROW(kRow)	(yCoordBodyTop + ((yCoordBodyBottom-yCoordBodyTop)*(kRow)/m_cRowsPerPage))

	int kStart = (kPage-1)*m_cRowsPerPage;
	int kLimit = MyMin(kStart+m_cRowsPerPage, m_pFolderData->getItemCount());

	for (int kIndex=kStart, kRow=0; kIndex<kLimit; kIndex++, kRow++)
	{
		int yCoord = YROW(kRow);

		fd_item::Status status = m_pFolderData->getItemStatus(kIndex);
		
		// WXBUG the following draw the little line-item icons on the
		// WXBUG left edge of the paper (just like in the folder window's
		// WXBUG listbox).  this works on the preview window on all platforms.
		// WXBUG it works on the printer on Win32.
		// WXBUG
		// WXBUG it *does not* work when going to the PostScript driver on
		// WXBUG Linux or the print-to-PDF on MAC.  it also doesn't work
		// WXBUG to my LaserJet on MAC (this may be using the PS driver).
		// WXBUG
		// WXBUG I'm going to leave this for now -- i think it's a PS problem.
		
		gpViewFolderImageList->Draw(status,*dc,xCoordCol0,YROW(kRow),wxIMAGELIST_DRAW_TRANSPARENT,true);
		
		switch (status)
		{
		default:
//		case FD_ITEM_STATUS_UNKNOWN:		// unknown, not yet computed
//		case FD_ITEM_STATUS_ERROR:			// error, see m_err
//		case FD_ITEM_STATUS_MISMATCH:		// file vs folder mismatch (shouldn't happen since wx appends '/' to folder entrynames)
//		case FD_ITEM_STATUS_BOTH_NULL:		// neither set (probably error)
//		case FD_ITEM_STATUS_SAME_FILE:		// they are the same (physical) file
//		case FD_ITEM_STATUS_IDENTICAL:		// files are equal
//		case FD_ITEM_STATUS_EQUIVALENT:		// files are equivalent using soft-match
//		case FD_ITEM_STATUS_QUICKMATCH:
//		case FD_ITEM_STATUS_FOLDERS:		// both are folders
//		case FD_ITEM_STATUS_FILE_NULL:		// left side only has file; right side null
//		case FD_ITEM_STATUS_NULL_FILE:		// left side null, right side has file
//		case FD_ITEM_STATUS_FOLDER_NULL:	// left side only has folder; right side null
//		case FD_ITEM_STATUS_NULL_FOLDER:	// left side null, right side has folder
#if defined(__WXMSW__)
//		case FD_ITEM_STATUS_SHORTCUT_NULL:
//		case FD_ITEM_STATUS_NULL_SHORTCUT:
//		case FD_ITEM_STATUS_SHORTCUTS_SAME:
//		case FD_ITEM_STATUS_SHORTCUTS_EQ:
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
//		case FD_ITEM_STATUS_SYMLINK_NULL:
//		case FD_ITEM_STATUS_NULL_SYMLINK:
//		case FD_ITEM_STATUS_SYMLINKS_SAME:
//		case FD_ITEM_STATUS_SYMLINKS_EQ:
#endif
			dc->SetFont(*pFont);
			break;

		case fd_item::FD_ITEM_STATUS_DIFFERENT:		// files are different
#if defined(__WXMSW__)
		case fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ:
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
		case fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ:
#endif
			dc->SetFont(*pFontBold);
			break;
		}

		dc->SetTextForeground( gpViewFolderListItemAttr->getAttr(status)->GetTextColour() );

		wxString strLeft  = m_pFolderData->getItemStrLeft(kIndex);
		wxString strRight = m_pFolderData->getItemStrRight(kIndex);
		if (m_pFolderData->getItemHaveLeft(kIndex))		dc->DrawText( _elide(strLeft, wCoordCol1,dc), xCoordCol1,yCoord);
		if (m_pFolderData->getItemHaveRight(kIndex))	dc->DrawText( _elide(strRight,wCoordCol2,dc), xCoordCol2,yCoord);
	}

	//////////////////////////////////////////////////////////////////
	
	delete pFont;
	delete pFontBold;

	return true;
}

//////////////////////////////////////////////////////////////////

void ViewPrintoutFolder::drawPageHeaderFooter(wxDC * dc, int kPage,
											  wxString strPageTitle,
											  wxString strCol1, wxString strCol2,
											  wxString strFooter,
											  bool bJustMeasure,
											  int * pyCoordBodyTop, int * pyCoordBodyBottom)
{
	//////////////////////////////////////////////////////////////////
	// draw page header and footer and decoration lines
	//
	// +------------------------------------------+
	// |<app_name>     <report_title>    <page_nr>|
	// |                                          |
	// |---------------------+--------------------|
	// |<...dir_1>           |          <...dir_2>|
	// |---------------------+--------------------|
	// |                     |                    |
	// |                     |                    |
	// |                     |                    |
	// |                     |                    |
	// |                     |                    |
	// |---------------------+--------------------|
	// |                                   <stats>|
	// +------------------------------------------+
	//
	// i wanted to place the stats on the very bottom of the page, but
	// at large zoom factors, it runs off the bottom of the page.  this
	// looks like round-off problems between the coordinate system and
	// the font metrics.  so instead, we do 1.5 times the font height.
	// then we place the line at 2 times.
	//////////////////////////////////////////////////////////////////

	wxCoord wTextCenter,hText;

	wxString strLeft	= VER_APP_TITLE;
	wxString strCenter	= strPageTitle;
	wxString strRight	= wxString::Format( _("Page: %d / %d"), kPage, m_cPages);

	dc->GetTextExtent(strCenter,&wTextCenter,&hText);

	int yCoordRowTitles     = 0;
	int yCoordRowTopLine1   = hText*3;
	int yCoordRowColTitles  = hText*3 + hText/2;
	int yCoordRowTopLine2   = hText*5;
	int yCoordRowBodyTop    = hText*5 + hText/2;

	int yCoordRowStats      = m_yCoordMax - (hText*3/2);
	int yCoordRowBottomLine = m_yCoordMax - hText*2;
	int yCoordRowBodyBottom = yCoordRowBottomLine;

	if (!bJustMeasure)
	{
		dc->DrawText(strLeft,0,yCoordRowTitles);
		dc->DrawText(strCenter,(m_xCoordMax-wTextCenter)/2,yCoordRowTitles);
		wxCoord wTextRight;
		dc->GetTextExtent(strRight,&wTextRight,&hText);
		dc->DrawText(strRight,(m_xCoordMax-wTextRight),yCoordRowTitles);

		int xCoordGap = (m_xCoordPerInch / 4);

		wxString strDir1 = _elide(strCol1, (m_xCoordMax-xCoordGap)/2, dc);
		wxString strDir2 = _elide(strCol2, (m_xCoordMax-xCoordGap)/2, dc);

		dc->DrawText(strDir1,0,yCoordRowColTitles);
		wxCoord wTextDir2;
		dc->GetTextExtent(strDir2,&wTextDir2,&hText);
		dc->DrawText(strDir2,(m_xCoordMax-wTextDir2),yCoordRowColTitles);

		wxCoord wTextFooter;
		dc->GetTextExtent(strFooter,&wTextFooter,&hText);
		dc->DrawText(strFooter,(m_xCoordMax-wTextFooter), yCoordRowStats);

		dc->DrawLine(0,yCoordRowTopLine1,             m_xCoordMax,yCoordRowTopLine1);
		dc->DrawLine(0,yCoordRowTopLine2,             m_xCoordMax,yCoordRowTopLine2);
		dc->DrawLine(0,yCoordRowBottomLine,           m_xCoordMax,yCoordRowBottomLine);
		dc->DrawLine(m_xCoordMax/2,yCoordRowTopLine1, m_xCoordMax/2,yCoordRowBottomLine);
	}

	*pyCoordBodyTop = yCoordRowBodyTop;
	*pyCoordBodyBottom = yCoordRowBodyBottom;
}
