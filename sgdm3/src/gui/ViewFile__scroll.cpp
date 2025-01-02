// ViewFile__scroll.cpp
// scroll-bar related stuff on 2-way/3-way file set.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fl.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////
// _my_scrolled_win -- a simple wrapper around wxWindow to redirect
// events on the various scrollbars back into ViewFile without having
// to worry about the toplogy of nested child windows and splitters.

BEGIN_EVENT_TABLE(ViewFile::_my_scrolled_win, wxWindow)
	EVT_SCROLLWIN(ViewFile::_my_scrolled_win::onScrollEvent)
	EVT_SIZE (ViewFile::_my_scrolled_win::onSizeEvent)
	EVT_SET_FOCUS(ViewFile::_my_scrolled_win::onSetFocusEvent)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

void ViewFile::_my_scrolled_win::onScrollEvent(wxScrollWinEvent & e)
{
	// a wxScrollWinEvent from a basic wxWindow is different
	// from a wxScrollEvent on a wxScrollBar (control).

#define XX_DO_TRC 0
//#define XX_DO_TRC 1

#if XX_DO_TRC && defined(DEBUG)
#define XX_TRC(sz)	Statement(	wxLogTrace(wxTRACE_Messages,												\
										   _T("VF:onScroll:[page %d][orientation %c] %s [%d,%d,%d][%d]"),	\
										   (int)m_kSync,													\
										   ((orientation==wxVERTICAL) ? _T('V') : _T('H')),					\
										   (sz),															\
										   inThumb,inRange,inSize,											\
										   inTrack);														)
#else
#define XX_TRC(sz)	Statement( ; )
#endif

#if defined(__WXMSW__)
	// WXBUG Per wxWidgets bug 1224397 (that I filed), the Win32 version
	// WXBUG of class wxWindow does not update the thumb position.  [the
	// WXBUG wxScrollBar code does, but that is only used when the scrollbar
	// WXBUG is a control -- as opposed to something built into the window.
	// WXBUG
	// WXBUG so, it appears, that *WE* are responsible for updating the
	// WXBUG scrollbar thumb position based upon what we want to do with
	// WXBUG the event.

#	define XX_SSP(o,p)	SetScrollPos((o),(p))
#else
	// WXBUG The MAC and GTK versions of wxWindow have already updated
	// WXBUG the thumb position before we get called.

#	define XX_SSP(o,p)	Statement( ; )
#endif

#if XX_DO_TRC || defined(__WXMSW__)
	int orientation  = e.GetOrientation();
	WXTYPE eventType = e.GetEventType();

	int inThumb = GetScrollPos(orientation);
	int inRange = GetScrollRange(orientation);
	int inSize  = GetScrollThumb(orientation);
	if (inSize < 2)			// for very narrow/short windows, the thumb size can
		inSize = 2;			// go to 1.  this causes page{up,down} to not work.
	int inTrack = e.GetPosition();		// only defined for THUMBTRACK & THUMBRELEASE events
	
	if (eventType == wxEVT_SCROLLWIN_TOP)					{	XX_TRC(_T("top"));			XX_SSP(orientation,0);					}
	else if (eventType == wxEVT_SCROLLWIN_BOTTOM)			{	XX_TRC(_T("bottom"));		XX_SSP(orientation,inRange);			}
	else if (eventType == wxEVT_SCROLLWIN_LINEUP)			{	XX_TRC(_T("lineup"));		XX_SSP(orientation,inThumb-1);			}
	else if (eventType == wxEVT_SCROLLWIN_LINEDOWN)			{	XX_TRC(_T("linedown"));		XX_SSP(orientation,inThumb+1);			}
	else if (eventType == wxEVT_SCROLLWIN_PAGEUP)			{	XX_TRC(_T("pageup"));		XX_SSP(orientation,inThumb-inSize+1);	}
	else if (eventType == wxEVT_SCROLLWIN_PAGEDOWN)			{	XX_TRC(_T("pagedown"));		XX_SSP(orientation,inThumb+inSize-1);	}
	else if (eventType == wxEVT_SCROLLWIN_THUMBTRACK)		{	XX_TRC(_T("thumbtrack"));	XX_SSP(orientation,inTrack);			}
	else if (eventType == wxEVT_SCROLLWIN_THUMBRELEASE)		{	XX_TRC(_T("thumbrelease"));	XX_SSP(orientation,inTrack);			}
#endif

	// forward to ViewFile and let it do what it needs to.

	m_pViewFile->onScrollEvent(m_kSync,e);
}

//////////////////////////////////////////////////////////////////

void ViewFile::onScrollEvent(long kSync, wxScrollWinEvent & e)
{
	// TODO for now we just invalidate the child panel windows and let
	// TODO them repaint with the new scroll thumb position.  later we
	// TODO should try to do some bit-blit's...

	// NOTE i added calls to Update() with each of the Refresh() in order
	// NOTE to speed up what the user sees as he drags the thumb on the
	// NOTE scrollbar.  (rather than waiting for paints to happen during
	// NOTE idle time).  this fixed Win32 nicely, but had no effect on
	// NOTE gtk (no effect or maybe a little more sluggish).  it made
	// NOTE the mac version worse.

//	wxLogTrace(wxTRACE_Messages,_T("ViewFile::onScrollEvent: [bTop %d][orientation %d][thumb %d]"),
//			   bTop,e.GetOrientation(),e.GetPosition());

#if defined(__WXMSW__)
#	define RU(w)		Statement( (w)->Refresh(true); (w)->Update(); )
#else
#	define RU(w)		Statement( (w)->Refresh(true); )
#endif

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			RU( m_nbPage[kSync].m_pWinPanel[kPanel] );

	if (e.GetOrientation() == wxVERTICAL)
		RU( m_nbPage[kSync].m_pWinGlance );
}

//////////////////////////////////////////////////////////////////

void ViewFile::adjustVerticalScrollbar(long kSync, int thumbGiven)
{
	// we calibrate the scrollbars in characters (rows/cols) rather than pixels.
	// This assumes that we're using a fixed-width font.
	// 
	// if thumbGiven != -1, also warp scroll.

	if (!m_nbPage[kSync].m_pWinPanel[PANEL_T0] || !m_pDeDe)	// panel windows not yet ready, just ignore
		return;													// we just test PANEL_T0 since it should be all-or-none.

	int maxFormatted;		// the range in chars of the largest document
	int minClient;			// the size in chars (rows or cols) of document area of client window

	// since we are displaying voids around diff/merge patches,
	// all documents should have the same height (even though
	// they may each have a different number of lines).  so we
	// let the thumb position on this scrollbar reflect y-top
	// across all panels.

	maxFormatted = (int)(m_pDeDe->getDisplayList(kSync)->size());

	// when we have horizontal top-level splitters, the windows containing
	// the panels may have different heights, so we set the thumb and page
	// sizes to respect the shorter of them in order to guarantee that we
	// can always see the bottom edge.

	int rows0 = m_nbPage[kSync].m_pWinPanel[PANEL_T0]->getRowsDisplayable();
	int rows1 = m_nbPage[kSync].m_pWinPanel[PANEL_T1]->getRowsDisplayable();
	minClient = MyMin(rows0,rows1);
	if (m_nbPage[kSync].m_pWinPanel[PANEL_T2])
	{
		int rows2 = m_nbPage[kSync].m_pWinPanel[PANEL_T2]->getRowsDisplayable();
		minClient = MyMin(minClient,rows2);
	}
	
	int newThumb, newThumbSize, newRange, newPageSize;
	if (maxFormatted < minClient)
	{
		// effectively disable scrollbar

		newThumb     = 0;
		newThumbSize = maxFormatted;
		newRange     = maxFormatted;
		newPageSize  = maxFormatted;
	}
	else
	{
		int maxThumb = maxFormatted - minClient;
		if (thumbGiven != -1)
		{
			// use thumb position given.  but validate it.

			newThumb = MyMin(thumbGiven,maxThumb);
		}
		else
		{
			// fetch current thumb position, so we don't arbitrarily warp
			// scroll on a window resize.  verify that this thumb position
			// still makes sense.  if not warp a little.

			int curThumb = getScrollThumbCharPosition(kSync,wxVERTICAL);
			newThumb     = MyMin(curThumb,maxThumb);
		}
		newThumbSize = minClient;
		newRange     = maxFormatted;
		newPageSize  = minClient;
	}
	
	if (newThumb < 0)
		newThumb = 0;

#if 0 && defined(DEBUG)
	wxLogTrace(wxTRACE_Messages,_T("adjustVerticalScrollbar: [sync %ld][thumb %d][thumbSize %d][range %d]"),
			   kSync,newThumb,newThumbSize,newRange);
#endif

	m_nbPage[kSync].m_pWinScroll->SetScrollbar(wxVERTICAL,newThumb,newThumbSize,newRange,true);

	wxScrollWinEvent e(wxEVT_SCROLLWIN_THUMBRELEASE,newThumb,wxVERTICAL);

	m_nbPage[kSync].m_pWinScroll->GetEventHandler()->AddPendingEvent(e);
}


void ViewFile::adjustHorizontalScrollbar(long kSync, int thumbGiven)
{
	// we calibrate the scrollbars in characters (rows/cols) rather than pixels.
	// This assumes that we're using a fixed-width font.
	// 
	// if thumbGiven != -1, also warp scroll.

	if (!m_nbPage[kSync].m_pWinPanel[PANEL_T0] || !m_pDeDe)	// panel windows not yet ready, just ignore
		return;													// we just test PANEL_T0 since it should be all-or-none.

	int maxFormatted;		// the range in chars of the largest document
	int minClient;			// the size in chars (rows or cols) of document area of client window

	// we always keep the left edges of the panels in sync.
	// and the scroll maximum is based upon the panel with the longest line.

	maxFormatted = 0;
	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
	{
		fl_fl * pFlFl = getLayout(kSync,(PanelIndex)kPanel);
		if (pFlFl)
		{
			int w = pFlFl->getFormattedCols();
			maxFormatted = MyMax(maxFormatted,w);
		}
	}
			
	// because of the top-level vsplitters, the windows containing
	// the panels may have different widths, so we set
	// the thumb & page sizes to respect the shorter of the two,
	// in order to guarantee that we can always see the right edge.
	//
	// NOTE this has the quirky behavior that a page-up/down acts
	// NOTE as a full page-up/down on the shorter window and a
	// NOTE partial one on the taller window.  this feels a little
	// NOTE weird.

	int cols0 = m_nbPage[kSync].m_pWinPanel[PANEL_T0]->getColsDisplayable();
	int cols1 = m_nbPage[kSync].m_pWinPanel[PANEL_T1]->getColsDisplayable();
	minClient = MyMin(cols0,cols1);
	if (m_nbPage[kSync].m_pWinPanel[PANEL_T2])
	{
		int cols2 = m_nbPage[kSync].m_pWinPanel[PANEL_T2]->getColsDisplayable();
		minClient = MyMin(minClient,cols2);
	}
	
	int newThumb, newThumbSize, newRange, newPageSize;
	if (maxFormatted < minClient)
	{
		// effectively disable scrollbar

		newThumb     = 0;
		newThumbSize = maxFormatted;
		newRange     = maxFormatted;
		newPageSize  = maxFormatted;
	}
	else
	{
		int maxThumb = maxFormatted - minClient;
		if (thumbGiven != -1)
		{
			// use thumb position given.  but validate it.

			newThumb = MyMin(thumbGiven,maxThumb);
		}
		else
		{
			// fetch current thumb position, so we don't arbitrarily warp
			// scroll on a window resize.  verify that this thumb position
			// still makes sense.  if not warp a little.

			int curThumb = getScrollThumbCharPosition(kSync,wxHORIZONTAL);
			newThumb     = MyMin(curThumb,maxThumb);
		}
		newThumbSize = minClient;
		newRange     = maxFormatted;
		newPageSize  = minClient;
	}
	
	if (newThumb < 0)
		newThumb = 0;
	
	m_nbPage[kSync].m_pWinScroll->SetScrollbar(wxHORIZONTAL,newThumb,newThumbSize,newRange,true);

	wxScrollWinEvent e(wxEVT_SCROLLWIN_THUMBRELEASE,newThumb,wxHORIZONTAL);

	m_nbPage[kSync].m_pWinScroll->GetEventHandler()->AddPendingEvent(e);
}

void ViewFile::adjustScrollbar(long kSync, int orientation, int thumb)
{
	if (orientation == wxHORIZONTAL)
		adjustHorizontalScrollbar(kSync,thumb);
	else
		adjustVerticalScrollbar(kSync,thumb);
}

void ViewFile::warpScroll(long kSync, int orientation, int thumb)
{
	adjustScrollbar(kSync,orientation,thumb);
}

void ViewFile::warpScrollCentered(long kSync, int row)
{
	int displayable = m_nbPage[kSync].m_pWinPanel[PANEL_T0]->getRowsDisplayable();
	
	int thumb = row - ((displayable+1) / 2);
	if (thumb < 0)
		thumb = 0;
	
	adjustVerticalScrollbar(kSync,thumb);
}

//////////////////////////////////////////////////////////////////

void ViewFile::keyboardScroll_top(long kSync, int orientation)
{
	int newThumb = 0;

	adjustScrollbar(kSync,orientation,newThumb);
}

void ViewFile::keyboardScroll_bottom(long kSync, int orientation)
{
	int newThumb = getScrollRangeChar(kSync,orientation);

	adjustScrollbar(kSync,orientation,newThumb);
}

void ViewFile::keyboardScroll_lineup(long kSync, int orientation)
{
	int newThumb = getScrollThumbCharPosition(kSync,orientation) - 1;
	if (newThumb < 0)
		newThumb = 0;

	adjustScrollbar(kSync,orientation,newThumb);
}

void ViewFile::keyboardScroll_linedown(long kSync, int orientation)
{
	int newThumb = getScrollThumbCharPosition(kSync,orientation) + 1;

	adjustScrollbar(kSync,orientation,newThumb);
}

void ViewFile::keyboardScroll_pageup(long kSync, int orientation)
{
	int newThumb = getScrollThumbCharPosition(kSync,orientation) - getScrollThumbSize(kSync,orientation) + 1;
	if (newThumb < 0)
		newThumb = 0;

	adjustScrollbar(kSync,orientation,newThumb);
}

void ViewFile::keyboardScroll_pagedown(long kSync, int orientation)
{
	int newThumb = getScrollThumbCharPosition(kSync,orientation) + getScrollThumbSize(kSync,orientation) - 1;

	adjustScrollbar(kSync,orientation,newThumb);
}

//////////////////////////////////////////////////////////////////

void ViewFile::mouseScroll(long kSync, int vscroll, int hscroll)
{
	// used when mouse is dragging out a selection and the mouse
	// leaves the document area and we need to scroll a little.

	if (vscroll != 0)
	{
		int newThumb = getScrollThumbCharPosition(kSync,wxVERTICAL) + vscroll;
		adjustVerticalScrollbar(kSync,newThumb);
	}

	if (hscroll != 0)
	{
		int newThumb = getScrollThumbCharPosition(kSync,wxHORIZONTAL) + hscroll;
		adjustHorizontalScrollbar(kSync,newThumb);
	}
}

//////////////////////////////////////////////////////////////////

void ViewFile::keyboardScroll_delta(bool bNext, bool bConflict)
{
	long row = ((bNext) ? getNextChange(bConflict) : getPrevChange(bConflict));
	if (row == -1)
		return;

	long kSync = m_currentNBSync;

	m_pDeDe->setPatchHighlight(kSync,row,false);
	warpScrollCentered(kSync,row);

	if (getPanelWithFocus() != -1)
		if (getPanel(kSync,(PanelIndex)getPanelWithFocus()))
			getPanel(kSync,(PanelIndex)getPanelWithFocus())->setBogusCaret();
}

