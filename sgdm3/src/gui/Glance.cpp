// Glance.cpp
// Window for drawing the files-at-a-glance.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#ifdef DEBUGUTILPERF
static const wxChar * sszKey_paint = L"glance_paint";
#endif

//////////////////////////////////////////////////////////////////
			
BEGIN_EVENT_TABLE(Glance, wxWindow)
	EVT_PAINT(Glance::OnPaint)
	EVT_SIZE(Glance::onSize)
	EVT_ERASE_BACKGROUND(Glance::onEraseBackground)
	EVT_LEFT_DOWN(Glance::onMouseLeftDown)
	EVT_LEFT_UP(Glance::onMouseLeftUp)
	EVT_MOTION(Glance::onMouseMotion)
	EVT_LEAVE_WINDOW(Glance::onMouseLeaveWindow)
//	EVT_MOUSEWHEEL(Glance::onMouseEventWheel)
//	EVT_MIDDLE_DOWN(Glance::onMouseMiddleDown)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

static void s_cb_de_changed(void * pThis, const util_cbl_arg & arg)			{ ((Glance *)pThis)->cb_de_changed(arg); }

static void _cb_long(void * pThis, const util_cbl_arg & arg)
{
	GlobalProps::EnumGPL id = (GlobalProps::EnumGPL)arg.m_l;

	Glance * pGlance = (Glance *)pThis;
	pGlance->gp_cb_long(id);
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)

#	define X_PER_PANEL				6

#elif defined(__WXGTK__)

#	define X_PER_PANEL				6

#elif defined(__WXMAC__)

#	define X_PER_PANEL				6

#endif

#define X_WIDTH_BLACK_BARS			2
#define X_WIDTH_GAP					1
#define X_WIDTH_WINDOW_BORDER		1

#define X_MARGIN					(X_WIDTH_BLACK_BARS+X_WIDTH_GAP)
#define X_EXTRA						((X_WIDTH_BLACK_BARS+X_WIDTH_GAP+X_WIDTH_WINDOW_BORDER)*2)

#define WIDTH(xPerPanel,nrPanels)		(((xPerPanel)*(nrPanels)) + X_EXTRA)
//////////////////////////////////////////////////////////////////

Glance::Glance(wxWindow * pParent, long style, ViewFile * pViewFile, long kSync)
	: wxWindow(pParent,-1,wxDefaultPosition,wxSize(WIDTH(X_PER_PANEL,pViewFile->getNrTopPanels()),-1),style),
	  m_pViewFile(pViewFile),
	  m_pDeDe(NULL),
	  m_kSync(kSync),
	  m_bWeCapturedTheMouse(false)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);		// needed for wxAutoBufferedPaintDC()
	SetBackgroundColour(*wxWHITE);
}

Glance::~Glance(void)
{
	if (m_pDeDe)
		m_pDeDe->delChangeCB(s_cb_de_changed,this);
}

//////////////////////////////////////////////////////////////////

Glance2::Glance2(wxWindow * pParent, long style, ViewFile * pViewFile, long kSync)
	: Glance(pParent,style,pViewFile,kSync)
{
	// watch for changes in the colors that we draw with.

	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,       _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,       _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,         _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,         _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_UNIMP_FG,_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_FG,      _cb_long, this);
}
	
Glance2::~Glance2(void)
{
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,       _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,       _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,         _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,         _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_UNIMP_FG,_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_FG,      _cb_long, this);
}

//////////////////////////////////////////////////////////////////

Glance3::Glance3(wxWindow * pParent, long style, ViewFile * pViewFile, long kSync)
	: Glance(pParent,style,pViewFile,kSync)
{
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,        _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,        _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,          _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,          _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_UNIMP_FG,     _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_UNIMP_FG,_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_FG,  _cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG,      _cb_long, this);
}

Glance3::~Glance3(void)
{
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,        _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,        _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,          _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,          _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_UNIMP_FG,     _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_UNIMP_FG,_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_FG,  _cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG,      _cb_long, this);
}

//////////////////////////////////////////////////////////////////

void Glance::bind_dede(de_de * pDeDeNew)
{
	// diff-engine wasn't loaded/ready when we were created, so
	// we get bound to it later when it is created/ready.

	if (m_pDeDe)	m_pDeDe->delChangeCB(s_cb_de_changed,this);

	m_pDeDe = pDeDeNew;

	if (m_pDeDe)	m_pDeDe->addChangeCB(s_cb_de_changed,this);

	Refresh(true);		// effectively InvalidateRect(NULL)
}

void Glance::cb_de_changed(const util_cbl_arg & arg)
{
	static long masks[__NR_SYNCS__] = { DE_CHG__VIEW_MASK,		// SYNC_VIEW
										DE_CHG__EDIT_MASK };	// SYNC_EDIT

	// note: arg.m_p contains nothing.
	// note: arg.m_l contains the DE_CHG_ code for this change.

	long chg = arg.m_l;
	bool bSignificantForUs = ((chg & masks[m_kSync]) != 0);

	//wxLogTrace(wxTRACE_Messages,_T("Glance:[sync %d]:de_changed [0x%lx] significant [%d]"), m_kSync,chg,bSignificantForUs);

	// something changed in the diff-engine.
	// mark this window as requiring a re-paint.

	if (bSignificantForUs)
		Refresh(true);		// effectively InvalidateRect(NULL)
}

void Glance::gp_cb_long(GlobalProps::EnumGPL /*id*/)
{
	// one of the "long" global variables that we care about has changed.
	// this means that one of the _FILE_VIEW_ colors has changed.
	// mark this window as requiring a re-paint

	Refresh(true);
}

//////////////////////////////////////////////////////////////////

void Glance::onSize(wxSizeEvent & /*e*/)
{
	// if our size changed, we must recompute the glance window
	// since it is a scaled view of the documents.
	
	Refresh(true);		// effectively InvalidateRect(NULL)
}

//////////////////////////////////////////////////////////////////

void Glance::OnPaint(wxPaintEvent & /*e*/)
{
	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

#if defined(__WXGTK__)
	// WXBUG The GTK version crashes if the window has a negative
	// WXBUG size and we try to create a bitmap (via wxBufferedPaintDC)
	// WXBUG or sometimes if we try operate on it (dc.SetBackground).
	// WXBUG 
	// WXBUG One could ask why we're getting negative sizes on any
	// WXBUG window -- but that's a bigger question....
	// 
	// as a work-around, let's test it before try to use it.
	// even if negative, we need to create a trivial wxPaintDC
	// to satisfy the event.
	//
	// since this is fairly harmless, i'll let this code run on all
	// platforms -- rather than ifdef it.
#endif

	if ((xPixelsClientSize <= 0) || (yPixelsClientSize <= 0))
	{
		wxPaintDC dc(this);
		return;
	}

	_initHitVectors(yPixelsClientSize);
	
	wxAutoBufferedPaintDC dc(this);

	wxBrush brushWindowBG( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_WINDOW_BG) );

	dc.SetBackground(brushWindowBG);
	dc.Clear();

	if (!m_pDeDe)				// we need this incase we get a paint before
		return;					// the document is completely loaded.

	if (m_pDeDe->isRunBusy())	// HACK -- if we get an ASSERT dialog while the diff-engine is running,
		return;					// HACK -- we get a paint message -- this is bad.

	//wxLogTrace(wxTRACE_Messages,_T("Glance::OnPaint[kSync %d] calling prePaint()"),m_kSync);
	m_pViewFile->prePaint();

	// NOTE: we DO NOT reference the display list because it only
	// NOTE: has info for the lines that we are displaying.  we
	// NOTE: want the glance to be independent of the display ops.

	long cTotalRows = m_pDeDe->getTotalRows(m_kSync);
	if (cTotalRows == 0)
		return;

	// our window is yPixelsClientSize pixels tall and we have
	// cTotalRows rows of data in the documents (after voids
	// are accounted for).  since we want our glance to be
	// completely visible (and not a scrolled visual), we need
	// to draw a scaled or stretched representation.
	//
	// it'd be nice if we could draw a 1 pixel high line for
	// each row in the document, but we can't.
	//
	// so we scale it -- erring on the side of chanes/conflicts
	// -- that is, if we compress n document lines into 1 pixel,
	// we want that 1 pixel to represent a change/conflict if
	// any of the n lines did.

	UTIL_PERF_START_CLOCK(sszKey_paint);

	if (yPixelsClientSize < cTotalRows)		// if we have more rows of data than pixels, so we must
		_drawCompressed(dc);				// compress multiple rows into one pixel.
	else									// otherwise, we have more (or equal) pixels than data,
		_drawExpanded(dc);					// so we must draw multiple pixels for each row of data.

	UTIL_PERF_STOP_CLOCK(sszKey_paint);
}

//////////////////////////////////////////////////////////////////
// since we may have to map multiple document lines onto one pixel
// in the glance window, we need to remember the worst case.  we
// define these bits in a strict hierarchy so that we can just do
// a MAX on each line as we look at it.

#define X__ZERO			0x0000
#define X_EQ			0x0001
#define X_VOID			0x0002
#define X_OMITTED		0x0003
#define X_CHG_UNIMP		0x0004
#define X_CON_UNIMP		0x0005
#define X_CHG_IMP		0x0006
#define X_CON_IMP		0x0007
#define X__LIMIT		0x0008

//////////////////////////////////////////////////////////////////
// here begins the special code for 2-way glance windows

static unsigned int s_map_attr_to_bits_2(de_attr attr, bool bIgnoreUnimportantChanges, bool bHideOmittedLines)
{
	de_attr type = (attr & DE_ATTR__TYPE_MASK);

	switch (type)
	{
	default:
		wxASSERT_MSG( (0), _T("Coding Error!") );
	case DE_ATTR_MARK:		// TODO should we try to draw something special for MARKS?
	case DE_ATTR_EOF:
	case DE_ATTR_DIF_2EQ:
		return X_EQ;

	case DE_ATTR_DIF_0EQ:
		if (DE_ATTR__IS_SET(attr,DE_ATTR_UNIMPORTANT))
			if (bIgnoreUnimportantChanges)
				return X_EQ;
			else
				return X_CHG_UNIMP;
		else
			return X_CHG_IMP;

	case DE_ATTR_OMITTED:
		if (bHideOmittedLines)
			return X_EQ;
		else
			return X_OMITTED;
	}
}

//////////////////////////////////////////////////////////////////

void Glance2::_drawCompressed(wxAutoBufferedPaintDC & dc)
{
	// 2-way glance : compress multiple rows of data onto 1 pixel

	bool bIgnoreUnimportantChanges = (DE_DOP__IS_SET_IGN_UNIMPORTANT(m_pViewFile->getDisplayOps(m_kSync)));

	// the show-/hide-omitted-lines toolbar button is only enabled when
	// the display-mode is _ALL_.  we always hide omitted-lines when in
	// display-mode _DIFF_ONLY_, _DIFF_CTX_, and _EQUAL_ONLY.

	bool bHideOmittedLines         = (   !DE_DOP__IS_MODE_ALL(        m_pViewFile->getDisplayOps(m_kSync))
									  ||  DE_DOP__IS_SET_HIDE_OMITTED(m_pViewFile->getDisplayOps(m_kSync)));

	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	long cTotalRows = m_pDeDe->getTotalRows(m_kSync);

//	wxLogTrace(wxTRACE_Messages, _T("Glance:OnPaint:compressed: client_size [%d,%d] cTotalRows [%ld]"),xPixelsClientSize,yPixelsClientSize,cTotalRows);

	// ask the file view where the top windows are vertically scrolled to
	// and determine the first and last rows that are on-screen.  we want
	// to draw an indicator (vertical black line) on the glance window to
	// show the portion of the files that the file panels are displaying.
	// this is somewhat complicated because the file panels can hide lines
	// (such as when in diff-w/-context mode).

	const de_sync * pSyncStart;		long offsetInSyncStart;
	const de_sync * pSyncEndT0;		long offsetInSyncEndT0;
	const de_sync * pSyncEndT1;		long offsetInSyncEndT1;
	
	_getFirstRowVisibleForBar(&pSyncStart, &offsetInSyncStart);
	_getLastRowVisibleForBar(PANEL_T0, &pSyncEndT0, &offsetInSyncEndT0);
	if (m_pViewFile->areSplittersVertical(m_kSync))
	{
		// when vertical splitters, all panels are the same size
		// so they must display the same amount of content.
		pSyncEndT1 = pSyncEndT0;
		offsetInSyncEndT1 = offsetInSyncEndT0;
	}
	else
	{
		_getLastRowVisibleForBar(PANEL_T1,&pSyncEndT1,&offsetInSyncEndT1);
	}

	long yBarStart = 0;
	long yBarEndT0 = 0;
	long yBarEndT1 = 0;

	// create pens here so we don't need to do it on each iteration.

	wxPen * aPens[X__LIMIT];
	_getPens(aPens);
	
	dc.SetPen( *aPens[X_EQ] );

	// run thru the display list and map multiple rows onto one pixel.
	// we have the X_ bits defined such that we can OR together all of
	// the different types of rows so that the most important kind
	// pops out.

	unsigned int status[__NR_TOP_PANELS__];
		
	const de_sync_list * pSyncList = m_pDeDe->getSyncList(m_kSync);
	long yPrev = -2;
	long y     = -1;
	long row   = 0;
	long count = 0;
	for (const de_sync * pSync=pSyncList->getHead(); (pSync); pSync=pSync->getNext())
	{
		if (pSync == pSyncStart)	yBarStart = (row + offsetInSyncStart) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT0)	yBarEndT0 = (row + offsetInSyncEndT0) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT1)	yBarEndT1 = (row + offsetInSyncEndT1) * yPixelsClientSize / cTotalRows;

		long lenMax = pSync->getMaxLen();
		for (long kRow=0; kRow<lenMax; kRow++)
		{
			y = (row * yPixelsClientSize / cTotalRows);
			if (y != yPrev)					// we're beginning a new row of pixels
			{
				if (count)					// draw final result of all that OR-ing.
				{
					dc.SetPen( *aPens[status[PANEL_T0]] );
					dc.DrawLine(X_MARGIN,yPrev, X_MARGIN+X_PER_PANEL,yPrev);

					dc.SetPen( *aPens[status[PANEL_T1]] );
					dc.DrawLine(X_MARGIN+X_PER_PANEL,yPrev, X_MARGIN+2*X_PER_PANEL,yPrev);
				}

				status[PANEL_T0] = X__ZERO;
				status[PANEL_T1] = X__ZERO;
				count = 0;
				yPrev = y;

				// remember the (sync,offset) of the first row mapped to this pixel (for later
				// use by the mouse hit testing).

				m_vecHitSync[y] = pSync;
				m_vecHitOffset[y] = kRow;
			}

			// remember the largest X_ value for this row

			unsigned int s  = s_map_attr_to_bits_2(pSync->getAttr(),bIgnoreUnimportantChanges,bHideOmittedLines);
			unsigned int s0 = ((kRow < pSync->getLen(PANEL_T0)) ? s : X_VOID);
			unsigned int s1 = ((kRow < pSync->getLen(PANEL_T1)) ? s : X_VOID);
			if (s0 > status[PANEL_T0]) status[PANEL_T0] = s0;
			if (s1 > status[PANEL_T1]) status[PANEL_T1] = s1;
			count++;
			row++;
		}
	}

	m_ndxHitEnd = y+1;		// remember how much of hit-testing vectors we defined.

	if (count)
	{
		dc.SetPen( *aPens[status[PANEL_T0]] );
		dc.DrawLine(X_MARGIN,y, X_MARGIN+X_PER_PANEL,y);

		dc.SetPen( *aPens[status[PANEL_T1]] );
		dc.DrawLine(X_MARGIN+X_PER_PANEL,y, X_MARGIN+2*X_PER_PANEL,y);
	}

	_draw_bars(dc,yBarStart,yBarEndT0,yBarEndT1);

	dc.SetPen(wxNullPen);

	_freePens(aPens);
}

void Glance2::_drawExpanded(wxAutoBufferedPaintDC & dc)
{
	// 2-way glance : expand rows to one or more pixels

	bool bIgnoreUnimportantChanges = (DE_DOP__IS_SET_IGN_UNIMPORTANT(m_pViewFile->getDisplayOps(m_kSync)));

	// the show-/hide-omitted-lines toolbar button is only enabled when
	// the display-mode is _ALL_.  we always hide omitted-lines when in
	// display-mode _DIFF_ONLY_, _DIFF_CTX_, and _EQUAL_ONLY.

	bool bHideOmittedLines         = (   !DE_DOP__IS_MODE_ALL(        m_pViewFile->getDisplayOps(m_kSync))
									  ||  DE_DOP__IS_SET_HIDE_OMITTED(m_pViewFile->getDisplayOps(m_kSync)));

	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	long cTotalRows = m_pDeDe->getTotalRows(m_kSync);

//	wxLogTrace(wxTRACE_Messages, _T("Glance:OnPaint:expanded: client_size [%d,%d] cTotalRows [%ld]"),xPixelsClientSize,yPixelsClientSize,cTotalRows);

	// ask the file view where the top windows are vertically scrolled to
	// and determine the first and last rows that are on-screen.  we want
	// to draw an indicator (vertical black line) on the glance window to
	// show the portion of the files that the file panels are displaying.
	// this is somewhat complicated because the file panels can hide lines
	// (such as when in diff-w/-context mode).

	const de_sync * pSyncStart;		long offsetInSyncStart;
	const de_sync * pSyncEndT0;		long offsetInSyncEndT0;
	const de_sync * pSyncEndT1;		long offsetInSyncEndT1;
	
	_getFirstRowVisibleForBar(&pSyncStart, &offsetInSyncStart);
	_getLastRowVisibleForBar(PANEL_T0, &pSyncEndT0, &offsetInSyncEndT0);
	if (m_pViewFile->areSplittersVertical(m_kSync))
	{
		// when vertical splitters, all panels are the same size
		// so they must display the same amount of content.
		pSyncEndT1 = pSyncEndT0;
		offsetInSyncEndT1 = offsetInSyncEndT0;
	}
	else
	{
		_getLastRowVisibleForBar(PANEL_T1,&pSyncEndT1,&offsetInSyncEndT1);
	}

	long yBarStart = 0;
	long yBarEndT0 = 0;
	long yBarEndT1 = 0;

	// create pens & brushes here so we don't need to do it on each iteration.

	wxPen penNone(*wxBLACK,1,wxTRANSPARENT);	// a null pen because we don't want borders on DrawRectangle()
	dc.SetPen(penNone);

	wxBrush * aBrushes[X__LIMIT];
	_getBrushes(aBrushes);

	const de_sync_list * pSyncList = m_pDeDe->getSyncList(m_kSync);
	long row   = 0;
	for (const de_sync * pSync=pSyncList->getHead(); (pSync); pSync=pSync->getNext())
	{
		long len0 = pSync->getLen(PANEL_T0);
		long len1 = pSync->getLen(PANEL_T1);
		long lenMax = MyMax(len0,len1);

		long y  =   row         * yPixelsClientSize / cTotalRows;
		long h0 = ((row + len0) * yPixelsClientSize / cTotalRows) - y;	// (row+ .. -y) to minimize roundoff problems
		long h1 = ((row + len1) * yPixelsClientSize / cTotalRows) - y;
		long hMax = MyMax(h0,h1);

		unsigned int status = s_map_attr_to_bits_2(pSync->getAttr(),bIgnoreUnimportantChanges,bHideOmittedLines);
		
		if (len0 > 0)                       { dc.SetBrush( *aBrushes[status]); dc.DrawRectangle(X_MARGIN,y,    X_PER_PANEL,h0     ); }
		if ((len0 == 0) || (len0 < len1))   { dc.SetBrush( *aBrushes[X_VOID]); dc.DrawRectangle(X_MARGIN,y+h0, X_PER_PANEL,hMax-h0); }

		if (len1 > 0)                       { dc.SetBrush( *aBrushes[status]); dc.DrawRectangle(X_MARGIN+X_PER_PANEL,y,    X_PER_PANEL,h1     ); }
		if ((len1 == 0) || (len1 < len0))   { dc.SetBrush( *aBrushes[X_VOID]); dc.DrawRectangle(X_MARGIN+X_PER_PANEL,y+h1, X_PER_PANEL,hMax-h1); }

		if (pSync == pSyncStart)	yBarStart = (row + offsetInSyncStart) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT0)	yBarEndT0 = (row + offsetInSyncEndT0) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT1)	yBarEndT1 = (row + offsetInSyncEndT1) * yPixelsClientSize / cTotalRows;

		// remember the (sync,offset) of the first row mapped to this pixel (for later
		// use by the mouse hit testing).

		for (long l=0; l<lenMax; l++)
		{
			long ya = ((row + l    ) * yPixelsClientSize / cTotalRows);
			long yb = ((row + l + 1) * yPixelsClientSize / cTotalRows);

			while (ya < yb)
			{
				m_vecHitSync[ya]   = pSync;
				m_vecHitOffset[ya] = l;
				ya++;
			}

			m_ndxHitEnd = ya;
		}
		
		row += lenMax;
	}

	_draw_bars(dc,yBarStart,yBarEndT0,yBarEndT1);

	dc.SetPen(wxNullPen);
	dc.SetBrush(wxNullBrush);

	_freeBrushes(aBrushes);
}

void Glance2::_draw_bars(wxAutoBufferedPaintDC & dc, long yBarStart, long yBarEndT0, long yBarEndT1)
{
	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	if (yBarEndT0 == yBarStart)
		yBarEndT0++;
	if (yBarEndT1 == yBarStart)
		yBarEndT1++;

	if (yBarEndT0 == yBarEndT1)
	{
		dc.SetPen(*wxBLACK_PEN);
		dc.DrawLine(                  0,yBarStart,                   0,yBarEndT0);
		dc.DrawLine(                  1,yBarStart,                   1,yBarEndT0);
		dc.DrawLine(xPixelsClientSize-2,yBarStart, xPixelsClientSize-2,yBarEndT0);
		dc.DrawLine(xPixelsClientSize-1,yBarStart, xPixelsClientSize-1,yBarEndT0);
	}
	else
	{
		long yBarEndMin = MyMin(yBarEndT0,yBarEndT1);
		long yBarEndMax = MyMax(yBarEndT0,yBarEndT1);

		dc.SetPen(*wxBLACK_PEN);
		dc.DrawLine(                  0,yBarStart,                   0,yBarEndMin);
		dc.DrawLine(                  1,yBarStart,                   1,yBarEndMin);
		dc.DrawLine(xPixelsClientSize-2,yBarStart, xPixelsClientSize-2,yBarEndMin);
		dc.DrawLine(xPixelsClientSize-1,yBarStart, xPixelsClientSize-1,yBarEndMin);

		dc.SetPen(*wxGREY_PEN);
		dc.DrawLine(                  0,yBarEndMin,                   0,yBarEndMax);
		dc.DrawLine(                  1,yBarEndMin,                   1,yBarEndMax);
		dc.DrawLine(xPixelsClientSize-2,yBarEndMin, xPixelsClientSize-2,yBarEndMax);
		dc.DrawLine(xPixelsClientSize-1,yBarEndMin, xPixelsClientSize-1,yBarEndMax);
	}
}

//////////////////////////////////////////////////////////////////
// here begins the special code for 3-way glance windows

static unsigned int s_map_attr_to_bits_3(de_attr attr, bool bIgnoreUnimportantChanges, bool bHideOmittedLines)
{
	de_attr type = (attr & DE_ATTR__TYPE_MASK);

	switch (type)
	{
	default:
		wxASSERT_MSG( (0), _T("Coding Error!") );
	case DE_ATTR_MARK:		// TODO should we try to draw something special for MARKS?
	case DE_ATTR_EOF:
	case DE_ATTR_MRG_3EQ:
		return X_EQ;

	case DE_ATTR_MRG_T0T2EQ:
	case DE_ATTR_MRG_T1T2EQ:
	case DE_ATTR_MRG_T1T0EQ:
	case DE_ATTR_MRG_0EQ:
		if (DE_ATTR__IS_SET(attr,DE_ATTR_UNIMPORTANT))
			if (bIgnoreUnimportantChanges)
				return X_EQ;
			else
				if (DE_ATTR__IS_SET(attr,DE_ATTR_CONFLICT))
					return X_CON_UNIMP;
				else
					return X_CHG_UNIMP;
		else
			if (DE_ATTR__IS_SET(attr,DE_ATTR_CONFLICT))
				return X_CON_IMP;
			else
				return X_CHG_IMP;

	case DE_ATTR_OMITTED:
		if (bHideOmittedLines)
			return X_EQ;
		else
			return X_OMITTED;
	}
}

//////////////////////////////////////////////////////////////////

void Glance3::_drawCompressed(wxAutoBufferedPaintDC & dc)
{
	// 3-way glance : compress multiple rows of data onto 1 pixel

	bool bIgnoreUnimportantChanges = (DE_DOP__IS_SET_IGN_UNIMPORTANT(m_pViewFile->getDisplayOps(m_kSync)));

	// the show-/hide-omitted-lines toolbar button is only enabled when
	// the display-mode is _ALL_.  we always hide omitted-lines when in
	// display-mode _DIFF_ONLY_, _DIFF_CTX_, and _EQUAL_ONLY.

	bool bHideOmittedLines         = (   !DE_DOP__IS_MODE_ALL(        m_pViewFile->getDisplayOps(m_kSync))
									  ||  DE_DOP__IS_SET_HIDE_OMITTED(m_pViewFile->getDisplayOps(m_kSync)));

	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	long cTotalRows = m_pDeDe->getTotalRows(m_kSync);

//	wxLogTrace(wxTRACE_Messages, _T("Glance:OnPaint:compressed: client_size [%d,%d] cTotalRows [%ld]"),xPixelsClientSize,yPixelsClientSize,cTotalRows);

	// ask the file view where the top windows are vertically scrolled to
	// and determine the first and last rows that are on-screen.  we want
	// to draw an indicator (vertical black line) on the glance window to
	// show the portion of the files that the file panels are displaying.
	// this is somewhat complicated because the file panels can hide lines
	// (such as when in diff-w/-context mode).

	const de_sync * pSyncStart;		long offsetInSyncStart;
	const de_sync * pSyncEndT0;		long offsetInSyncEndT0;
	const de_sync * pSyncEndT1;		long offsetInSyncEndT1;
	const de_sync * pSyncEndT2;		long offsetInSyncEndT2;
	
	_getFirstRowVisibleForBar(&pSyncStart, &offsetInSyncStart);
	_getLastRowVisibleForBar(PANEL_T0, &pSyncEndT0, &offsetInSyncEndT0);
	if (m_pViewFile->areSplittersVertical(m_kSync))
	{
		// when vertical splitters, all panels are the same size
		// so they must display the same amount of content.
		pSyncEndT1 = pSyncEndT0;
		pSyncEndT2 = pSyncEndT0;
		offsetInSyncEndT1 = offsetInSyncEndT0;
		offsetInSyncEndT2 = offsetInSyncEndT0;
	}
	else
	{
		_getLastRowVisibleForBar(PANEL_T1,&pSyncEndT1,&offsetInSyncEndT1);
		_getLastRowVisibleForBar(PANEL_T2,&pSyncEndT2,&offsetInSyncEndT2);
	}

	long yBarStart = 0;
	long yBarEndT0 = 0;
	long yBarEndT1 = 0;
	long yBarEndT2 = 0;

	// create pens here so we don't need to do it on each iteration.

	wxPen * aPens[X__LIMIT];
	_getPens(aPens);
	
	dc.SetPen( *aPens[X_EQ] );

	// run thru the display list and map multiple rows onto one pixel.
	// we have the X_ bits defined such that we can OR together all of
	// the different types of rows so that the most important kind
	// pops out.

	unsigned int status[__NR_TOP_PANELS__];
		
	const de_sync_list * pSyncList = m_pDeDe->getSyncList(m_kSync);
	long yPrev = -2;
	long y     = -1;
	long row   = 0;
	long count = 0;
	for (const de_sync * pSync=pSyncList->getHead(); (pSync); pSync=pSync->getNext())
	{
		if (pSync == pSyncStart)	yBarStart = (row + offsetInSyncStart) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT0)	yBarEndT0 = (row + offsetInSyncEndT0) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT1)	yBarEndT1 = (row + offsetInSyncEndT1) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT2)	yBarEndT2 = (row + offsetInSyncEndT2) * yPixelsClientSize / cTotalRows;

		long lenMax = pSync->getMaxLen();
		for (long kRow=0; kRow<lenMax; kRow++)
		{
			y = (row * yPixelsClientSize / cTotalRows);
			if (y != yPrev)					// we're beginning a new row of pixels
			{
				if (count)					// draw final result of all that OR-ing.
				{
					dc.SetPen( *aPens[status[PANEL_T0]] );
					dc.DrawLine(X_MARGIN,yPrev, X_MARGIN+X_PER_PANEL,yPrev);

					dc.SetPen( *aPens[status[PANEL_T1]] );
					dc.DrawLine(X_MARGIN+X_PER_PANEL,yPrev, X_MARGIN+2*X_PER_PANEL,yPrev);

					dc.SetPen( *aPens[status[PANEL_T2]] );
					dc.DrawLine(X_MARGIN+2*X_PER_PANEL,yPrev, X_MARGIN+3*X_PER_PANEL,yPrev);
				}

				status[PANEL_T0] = X__ZERO;
				status[PANEL_T1] = X__ZERO;
				status[PANEL_T2] = X__ZERO;
				count = 0;
				yPrev = y;

				// remember the (sync,offset) of the first row mapped to this pixel (for later
				// use by the mouse hit testing).

				m_vecHitSync[y] = pSync;
				m_vecHitOffset[y] = kRow;
			}

			// remember the largest X_ value for this row

			unsigned int s  = s_map_attr_to_bits_3(pSync->getAttr(),bIgnoreUnimportantChanges,bHideOmittedLines);
			unsigned int s0 = ((kRow < pSync->getLen(PANEL_T0)) ? s : X_VOID);
			unsigned int s1 = ((kRow < pSync->getLen(PANEL_T1)) ? s : X_VOID);
			unsigned int s2 = ((kRow < pSync->getLen(PANEL_T2)) ? s : X_VOID);
			if (s0 > status[PANEL_T0]) status[PANEL_T0] = s0;
			if (s1 > status[PANEL_T1]) status[PANEL_T1] = s1;
			if (s2 > status[PANEL_T2]) status[PANEL_T2] = s2;
			count++;
			row++;
		}
	}

	m_ndxHitEnd = y+1;		// remember how much of hit-testing vectors we defined.

	if (count)
	{
		dc.SetPen( *aPens[status[PANEL_T0]] );
		dc.DrawLine(X_MARGIN,y, X_MARGIN+X_PER_PANEL,y);

		dc.SetPen( *aPens[status[PANEL_T1]] );
		dc.DrawLine(X_MARGIN+X_PER_PANEL,y, X_MARGIN+2*X_PER_PANEL,y);

		dc.SetPen( *aPens[status[PANEL_T2]] );
		dc.DrawLine(X_MARGIN+2*X_PER_PANEL,y, X_MARGIN+3*X_PER_PANEL,y);
	}

	_draw_bars(dc,yBarStart,yBarEndT0,yBarEndT1,yBarEndT2);
	
	dc.SetPen(wxNullPen);

	_freePens(aPens);
}

void Glance3::_drawExpanded(wxAutoBufferedPaintDC & dc)
{
	// 3-way glance : expand rows to one or more pixels

	bool bIgnoreUnimportantChanges = (DE_DOP__IS_SET_IGN_UNIMPORTANT(m_pViewFile->getDisplayOps(m_kSync)));

	// the show-/hide-omitted-lines toolbar button is only enabled when
	// the display-mode is _ALL_.  we always hide omitted-lines when in
	// display-mode _DIFF_ONLY_, _DIFF_CTX_, and _EQUAL_ONLY.

	bool bHideOmittedLines         = (   !DE_DOP__IS_MODE_ALL(        m_pViewFile->getDisplayOps(m_kSync))
									  ||  DE_DOP__IS_SET_HIDE_OMITTED(m_pViewFile->getDisplayOps(m_kSync)));

	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	long cTotalRows = m_pDeDe->getTotalRows(m_kSync);

//	wxLogTrace(wxTRACE_Messages, _T("Glance:OnPaint:expanded: client_size [%d,%d] cTotalRows [%ld]"),xPixelsClientSize,yPixelsClientSize,cTotalRows);

	// ask the file view where the top windows are vertically scrolled to
	// and determine the first and last rows that are on-screen.  we want
	// to draw an indicator (vertical black line) on the glance window to
	// show the portion of the files that the file panels are displaying.
	// this is somewhat complicated because the file panels can hide lines
	// (such as when in diff-w/-context mode).

	const de_sync * pSyncStart;		long offsetInSyncStart;
	const de_sync * pSyncEndT0;		long offsetInSyncEndT0;
	const de_sync * pSyncEndT1;		long offsetInSyncEndT1;
	const de_sync * pSyncEndT2;		long offsetInSyncEndT2;
	
	_getFirstRowVisibleForBar(&pSyncStart, &offsetInSyncStart);
	_getLastRowVisibleForBar(PANEL_T0, &pSyncEndT0, &offsetInSyncEndT0);
	if (m_pViewFile->areSplittersVertical(m_kSync))
	{
		// when vertical splitters, all panels are the same size
		// so they must display the same amount of content.
		pSyncEndT1 = pSyncEndT0;
		pSyncEndT2 = pSyncEndT0;
		offsetInSyncEndT1 = offsetInSyncEndT0;
		offsetInSyncEndT2 = offsetInSyncEndT0;
	}
	else
	{
		_getLastRowVisibleForBar(PANEL_T1,&pSyncEndT1,&offsetInSyncEndT1);
		_getLastRowVisibleForBar(PANEL_T2,&pSyncEndT2,&offsetInSyncEndT2);
	}

	long yBarStart = 0;
	long yBarEndT0 = 0;
	long yBarEndT1 = 0;
	long yBarEndT2 = 0;

	// create pens & brushes here so we don't need to do it on each iteration.

	wxPen penNone(*wxBLACK,1,wxTRANSPARENT);	// a null pen because we don't want borders on DrawRectangle()
	dc.SetPen(penNone);

	wxBrush * aBrushes[X__LIMIT];
	_getBrushes(aBrushes);
		
	const de_sync_list * pSyncList = m_pDeDe->getSyncList(m_kSync);
	long row   = 0;
	for (const de_sync * pSync=pSyncList->getHead(); (pSync); pSync=pSync->getNext())
	{
		long len0 = pSync->getLen(PANEL_T0);
		long len1 = pSync->getLen(PANEL_T1);
		long len2 = pSync->getLen(PANEL_T2);
		long lenMax = MyMax(MyMax(len0,len1),len2);

		long y  =   row         * yPixelsClientSize / cTotalRows;
		long h0 = ((row + len0) * yPixelsClientSize / cTotalRows) - y;	// (row+ .. -y) to minimize roundoff problems
		long h1 = ((row + len1) * yPixelsClientSize / cTotalRows) - y;
		long h2 = ((row + len2) * yPixelsClientSize / cTotalRows) - y;
		long hMax = MyMax(MyMax(h0,h1),h2);

		unsigned int status = s_map_attr_to_bits_3(pSync->getAttr(),bIgnoreUnimportantChanges,bHideOmittedLines);
		
		if (len0 > 0)                       { dc.SetBrush( *aBrushes[status]); dc.DrawRectangle(X_MARGIN              ,y,    X_PER_PANEL,h0     ); }
		if ((len0 == 0) || (len0 < lenMax)) { dc.SetBrush( *aBrushes[X_VOID]); dc.DrawRectangle(X_MARGIN              ,y+h0, X_PER_PANEL,hMax-h0); }

		if (len1 > 0)                       { dc.SetBrush( *aBrushes[status]); dc.DrawRectangle(X_MARGIN+  X_PER_PANEL,y,    X_PER_PANEL,h1     ); }
		if ((len1 == 0) || (len1 < lenMax)) { dc.SetBrush( *aBrushes[X_VOID]); dc.DrawRectangle(X_MARGIN+  X_PER_PANEL,y+h1, X_PER_PANEL,hMax-h1); }

		if (len2 > 0)                       { dc.SetBrush( *aBrushes[status]); dc.DrawRectangle(X_MARGIN+2*X_PER_PANEL,y,    X_PER_PANEL,h2     ); }
		if ((len2 == 0) || (len2 < lenMax)) { dc.SetBrush( *aBrushes[X_VOID]); dc.DrawRectangle(X_MARGIN+2*X_PER_PANEL,y+h2, X_PER_PANEL,hMax-h2); }

		if (pSync == pSyncStart)	yBarStart = (row + offsetInSyncStart) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT0)	yBarEndT0 = (row + offsetInSyncEndT0) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT1)	yBarEndT1 = (row + offsetInSyncEndT1) * yPixelsClientSize / cTotalRows;
		if (pSync == pSyncEndT2)	yBarEndT2 = (row + offsetInSyncEndT2) * yPixelsClientSize / cTotalRows;

		// remember the (sync,offset) of the first row mapped to this pixel (for later
		// use by the mouse hit testing).

		for (long l=0; l<lenMax; l++)
		{
			long ya = ((row + l    ) * yPixelsClientSize / cTotalRows);
			long yb = ((row + l + 1) * yPixelsClientSize / cTotalRows);

			while (ya < yb)
			{
				m_vecHitSync[ya]   = pSync;
				m_vecHitOffset[ya] = l;
				ya++;
			}

			m_ndxHitEnd = ya;
		}
		
		row += lenMax;
	}

	_draw_bars(dc,yBarStart,yBarEndT0,yBarEndT1,yBarEndT2);

	dc.SetPen(wxNullPen);
	dc.SetBrush(wxNullBrush);

	_freeBrushes(aBrushes);
}

void Glance3::_draw_bars(wxAutoBufferedPaintDC & dc, long yBarStart, long yBarEndT0, long yBarEndT1, long yBarEndT2)
{
	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	if (yBarEndT0 == yBarStart)
		yBarEndT0++;
	if (yBarEndT1 == yBarStart)
		yBarEndT1++;
	if (yBarEndT2 == yBarStart)
		yBarEndT2++;

	if ((yBarEndT0 == yBarEndT1) && (yBarEndT0 == yBarEndT2))
	{
		dc.SetPen(*wxBLACK_PEN);
		dc.DrawLine(                  0,yBarStart,                   0,yBarEndT0);
		dc.DrawLine(                  1,yBarStart,                   1,yBarEndT0);
		dc.DrawLine(xPixelsClientSize-2,yBarStart, xPixelsClientSize-2,yBarEndT0);
		dc.DrawLine(xPixelsClientSize-1,yBarStart, xPixelsClientSize-1,yBarEndT0);
	}
	else
	{
		long yBarEndMin = MyMin(yBarEndT0,yBarEndT1);	yBarEndMin = MyMin(yBarEndMin,yBarEndT2);
		long yBarEndMax = MyMax(yBarEndT0,yBarEndT1);	yBarEndMax = MyMax(yBarEndMax,yBarEndT2);

		dc.SetPen(*wxBLACK_PEN);
		dc.DrawLine(                  0,yBarStart,                   0,yBarEndMin);
		dc.DrawLine(                  1,yBarStart,                   1,yBarEndMin);
		dc.DrawLine(xPixelsClientSize-2,yBarStart, xPixelsClientSize-2,yBarEndMin);
		dc.DrawLine(xPixelsClientSize-1,yBarStart, xPixelsClientSize-1,yBarEndMin);

		dc.SetPen(*wxGREY_PEN);
		dc.DrawLine(                  0,yBarEndMin,                   0,yBarEndMax);
		dc.DrawLine(                  1,yBarEndMin,                   1,yBarEndMax);
		dc.DrawLine(xPixelsClientSize-2,yBarEndMin, xPixelsClientSize-2,yBarEndMax);
		dc.DrawLine(xPixelsClientSize-1,yBarEndMin, xPixelsClientSize-1,yBarEndMax);
	}
}

//////////////////////////////////////////////////////////////////

void Glance::onEraseBackground(wxEraseEvent & /*e*/)
{
	// wxDC * pDC = e.GetDC();

	// Do nothing since OnPaint() completely redraws the window.
	// This prevents flashing on MSW.
}

//////////////////////////////////////////////////////////////////

void Glance::onMouseLeftDown(wxMouseEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseLeftDown: pos [%d,%d]"),e.m_x,e.m_y);

	if (!m_pDeDe  ||  m_ndxHitEnd==0)		// incase we get a click before the document is
		return;								// completely loaded or we have drawn it.

	CaptureMouse();
	m_bWeCapturedTheMouse = true;

	_warp_scroll_on_y_mouse(e.m_y);
	_clear_status_bar_from_y_mouse();
}

void Glance::onMouseLeftUp(wxMouseEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseLeftUp: pos [%d,%d]"),e.m_x,e.m_y);

	while (HasCapture())
		ReleaseMouse();
	m_bWeCapturedTheMouse = false;

	e.Skip();
}

void Glance::_warp_scroll_on_y_mouse(long yClick)
{
	// we want to warp scroll the documents to the corresponding line.
	// this is a problem because we show the entire profile of the
	// source documents and may have compressed/expanded rows into
	// pixels.  but even if the pixels and rows were 1-to-1, we have
	// problems because the file view's display-ops may not be set to
	// display everything (DE_DOP_ALL and ~DE_DOP_HIDE_OMITTED).  in
	// these cases, we're only showing a subset of the files (like
	// diffs-with-context).  so there MAY NOT ACTUALLY BE a line in
	// the display list that corresponds with the pixel location that
	// the user clicked.

	// use the hit-testing vectors to map the y pixel into the
	// (sync node, offset) of the first row in the total document
	// that was drawn at that pixel.

	if (yClick >= m_ndxHitEnd)		// y value past end of what we draw.
		return;						// this should not happen. (don't use vec.size())
	
	if (yClick < 0)					// y value above top of window
		return;

	const de_sync * pSyncHit = m_vecHitSync[yClick];
	if (!pSyncHit)					// this should not happen.
		return;
	long offsetInSyncHit     = m_vecHitOffset[yClick];

	const de_sync_list * pSyncList = m_pDeDe->getSyncList(m_kSync);
	const TVector_Display * pDis   = m_pDeDe->getDisplayList(m_kSync);

	// the display-list is a vector of rows.  each row has a link to
	// the sync node it is in.  these sync nodes are members of the
	// sync-list that we walked when drawing our window.
	//
	// we need to find the sync-node we hit somewhere in the display
	// list (or the last visible sync-node prior to it).
	//
	// first the easy case, directly see if (any part of) the hit
	// node is visible (a sync-node can be partially visible (like
	// when showing context around a change (and the non-visible
	// portion can be in the middle of a node (like a large _EQ node
	// between 2 changes))).

	long rowMax = (long)pDis->size();
	long row;
	bool bFoundVisible = false;
	for (row=0; row<rowMax; row++)
	{
		const de_row &  rDeRow   = (*pDis)[row];
		const de_sync * pSyncRow = rDeRow.getSync();
		if (pSyncRow == pSyncHit)
		{
			long offsetInSyncRow = rDeRow.getOffsetInSync();

			if (offsetInSyncHit <= offsetInSyncRow)
			{
				// hit was on this row of the sync-node or this
				// is the first visible row of this node past
				// the hit-point.  this is as good as it gets.
				// warp to this row (centered).

//				wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseLeftDown:Warp1: [row %d][lines %d,%d]"),
//						   row,
//						   ((rDeRow.getPanelLine(PANEL_T0)) ? rDeRow.getPanelLine(PANEL_T0)->getFlLine()->getRow()+1 : -1),
//						   ((rDeRow.getPanelLine(PANEL_T1)) ? rDeRow.getPanelLine(PANEL_T1)->getFlLine()->getRow()+1 : -1));
				
				m_pViewFile->warpScrollCentered(m_kSync,row);
				return;
			}

			// we are in the right node, but need to look forward
			// for the line we hit.  continue scan of display list
			// for the line we hit or the first one past it.
			bFoundVisible = true;
		}
		else if (bFoundVisible)
		{
			// we walked past the last visible row of the matching
			// sync-node.  so our hit-point is in the non-visible
			// tail portion of the previous sync-node.  warp to
			// this row (centered).

//			wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseLeftDown:Warp2: [row %d][lines %d,%d]"),
//					   row,
//					   ((rDeRow.getPanelLine(PANEL_T0)) ? rDeRow.getPanelLine(PANEL_T0)->getFlLine()->getRow()+1 : -1),
//					   ((rDeRow.getPanelLine(PANEL_T1)) ? rDeRow.getPanelLine(PANEL_T1)->getFlLine()->getRow()+1 : -1));

			m_pViewFile->warpScrollCentered(m_kSync,row);
			return;
		}
	}

	// so, no portion of the sync-node that we hit is visible.
	// so we walk the sync-list, looking for the sync-node that we
	// got from the pixel hit-test while simultaneously walking the
	// display-list vector.  this will give us the display-list
	// row-number of last visible row of last visible sync-node
	// prior to the mouse click.

	row = 0;
	for (const de_sync * pSyncScan=pSyncList->getHead(); (pSyncScan && (pSyncScan != pSyncHit)); pSyncScan=pSyncScan->getNext())
	{
		const de_sync * pSyncRow = (*pDis)[row].getSync();

		// if this node is visible, count the number of rows that
		// are visible (a sync-node can be partially visible (like
		// when showing context around a change)).

		while ((pSyncRow == pSyncScan) && (row < rowMax))
		{
			row++;

			pSyncRow = (*pDis)[row].getSync();
		}
	}

//	wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseLeftDown:Warp3: [row %d][lines %d,%d]"),
//			   row,
//			   (((*pDis)[row].getPanelLine(PANEL_T0)) ? (*pDis)[row].getPanelLine(PANEL_T0)->getFlLine()->getRow()+1 : -1),
//			   (((*pDis)[row].getPanelLine(PANEL_T1)) ? (*pDis)[row].getPanelLine(PANEL_T1)->getFlLine()->getRow()+1 : -1));

	m_pViewFile->warpScrollCentered(m_kSync,row);
	return;
}

//////////////////////////////////////////////////////////////////

void Glance::onMouseMotion(wxMouseEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("Glance:onMouse: pos [%d,%d][HasCapture %d][WeCaptured %d]"),e.m_x,e.m_y,HasCapture(),m_bWeCapturedTheMouse);

	if (!m_pDeDe  ||  m_ndxHitEnd==0)		// incase we get a click before the document is
		return;								// completely loaded or we have drawn it.

	if (m_pDeDe->isRunBusy())				// screw case: don't let a simple mouse motion over the window to cause de_de::run()
		return;								// to run recursively when an assert/messagebox dialog raised during run().

	if (HasCapture() && m_bWeCapturedTheMouse)
		_warp_scroll_on_y_mouse(e.m_y);
	else
		_show_line_numbers_in_status_bar_from_y_mouse(e.m_y);
}

void Glance::_show_line_numbers_in_status_bar_from_y_mouse(long yClick)
{
	// use the hit-testing vectors to map the y pixel into the
	// (sync node, offset) of the first row in the total document
	// that was drawn at that pixel.

	if ((yClick < 0) || (yClick >= m_ndxHitEnd))	// bogus motion event or y value past end of what we drew.
		return;										// this should not happen.
	
	const de_sync * pSyncHit = m_vecHitSync[yClick];
	if (!pSyncHit)					// this should not happen.
		return;
	long offsetInSyncHit     = m_vecHitOffset[yClick];

//	wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseMotion:SyncHit: [offset %d]"), offsetInSyncHit);
//	pSyncHit->dump(5);

	// get the document line numbers for the document in each panel.

	wxString strMsg( _("Lines:") );
	for (long kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
	{
		long len = pSyncHit->getLen((PanelIndex)kPanel);
		if (len == -1)
		{
			// panel not present in sync node, ignore
		}
		else if (offsetInSyncHit < len)
		{
			// a line present in the source file.
			//
			// if it is an OMITTED node, we need to get the Layout
			// info differently, because the line didn't go thru the
			// diff-engine.

			const fl_line * pFlLine;
			if (pSyncHit->isSameType(DE_ATTR_OMITTED))
			{
				const de_sync_line_omitted * pSyncHitOmitted = static_cast<const de_sync_line_omitted *>(pSyncHit);
				const de_line * pDeLine = pSyncHitOmitted->getFirstLine((PanelIndex)kPanel);
				pFlLine = pDeLine->getFlLine();
				for (int kLine=0; kLine<offsetInSyncHit; kLine++)
					pFlLine = pFlLine->getNext();
			}
			else
			{
				long ndx = pSyncHit->getNdx((PanelIndex)kPanel);
				const de_line * pDeLine = m_pDeDe->getDeLineFromNdx(m_kSync,(PanelIndex)kPanel,ndx+offsetInSyncHit);
				pFlLine = pDeLine->getFlLine();
			}
			int lineNr = pFlLine->getLineNr();

			strMsg += wxString::Format( _T(" %d"), lineNr+1);
		}
		else /* len==0 or offset>len */
		{
			// a void

			strMsg += _T(" *");
		}
	}

//	wxLogTrace(wxTRACE_Messages, _T("Glance:onMouse: msg [%s]"), strMsg.wc_str());

	// Tooltips are too slow (and tend to be stale), so stuff
	// line number info into the status bar.

	m_pViewFile->getFrame()->SetStatusText(strMsg);
}

//////////////////////////////////////////////////////////////////

void Glance::onMouseLeaveWindow(wxMouseEvent & /*e*/)
{
	_clear_status_bar_from_y_mouse();
}

void Glance::_clear_status_bar_from_y_mouse(void)
{
	wxString strBlank;

	// Tooltips are too slow (and tend to be stale), so stuff
	// line number info into the status bar.

	m_pViewFile->getFrame()->SetStatusText(strBlank);
}

//////////////////////////////////////////////////////////////////

#if 0
void Glance::onMouseEventWheel(wxMouseEvent & e)
{
	wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseEventWheel:"));

	e.Skip();
}

void Glance::onMouseMiddleDown(wxMouseEvent & e)
{
	wxLogTrace(wxTRACE_Messages, _T("Glance:onMouseMiddleDown:"));

	e.Skip();
}
#endif

//////////////////////////////////////////////////////////////////

void Glance::_initHitVectors(int yPixelsClientSize)
{
	// for speed while drawing, we preallocate and initialize the
	// vectors so that we can safely use assign values randomly as we
	// compute them and without having to worry about using push_back().

	int yMax = MyMax(yPixelsClientSize,(int)m_vecHitSync.size());

	m_vecHitSync.resize(yMax,NULL);
	m_vecHitOffset.resize(yMax,0);

	m_ndxHitEnd = 0;	// we're only using this much of the vectors
}

//////////////////////////////////////////////////////////////////

void Glance2::_getPens(wxPen ** paPens)
{
	paPens[X__ZERO    ] = NULL;
	paPens[X_EQ       ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG),        1,wxSOLID );
	paPens[X_VOID     ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_VOID_FG),          1,wxSOLID );
	paPens[X_OMITTED  ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_OMIT_FG),          1,wxSOLID );
	paPens[X_CHG_UNIMP] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_UNIMP_FG), 1,wxSOLID );
	paPens[X_CON_UNIMP] = NULL;
	paPens[X_CHG_IMP  ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_FG),       1,wxSOLID );
	paPens[X_CON_IMP  ] = NULL;
}

void Glance3::_getPens(wxPen ** paPens)
{
	paPens[X__ZERO    ] = NULL;
	paPens[X_EQ       ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG),         1,wxSOLID );
	paPens[X_VOID     ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_VOID_FG),           1,wxSOLID );
	paPens[X_OMITTED  ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_OMIT_FG),           1,wxSOLID );
	paPens[X_CHG_UNIMP] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_UNIMP_FG),      1,wxSOLID );
	paPens[X_CON_UNIMP] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_UNIMP_FG), 1,wxSOLID );
	paPens[X_CHG_IMP  ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_FG),   1,wxSOLID );
	paPens[X_CON_IMP  ] = new wxPen( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG),       1,wxSOLID );
}

void Glance::_freePens(wxPen ** paPens)
{
	for (int k=X__ZERO; k<X__LIMIT; k++)
		if (paPens[k])
			delete (paPens[k]);
}

//////////////////////////////////////////////////////////////////

void Glance2::_getBrushes(wxBrush ** paBrushes)
{
	paBrushes[X__ZERO    ] = NULL;
	paBrushes[X_EQ       ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG),        wxSOLID );
	paBrushes[X_VOID     ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_VOID_FG),          wxSOLID );
	paBrushes[X_OMITTED  ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_OMIT_FG),          wxSOLID );
	paBrushes[X_CHG_UNIMP] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_UNIMP_FG), wxSOLID );
	paBrushes[X_CON_UNIMP] = NULL;
	paBrushes[X_CHG_IMP  ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_FG),       wxSOLID );
	paBrushes[X_CON_IMP  ] = NULL;
}

void Glance3::_getBrushes(wxBrush ** paBrushes)
{
	paBrushes[X__ZERO    ] = NULL;
	paBrushes[X_EQ       ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG),         wxSOLID );
	paBrushes[X_VOID     ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_VOID_FG),           wxSOLID );
	paBrushes[X_OMITTED  ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_OMIT_FG),           wxSOLID );
	paBrushes[X_CHG_UNIMP] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_UNIMP_FG),      wxSOLID );
	paBrushes[X_CON_UNIMP] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_UNIMP_FG), wxSOLID );
	paBrushes[X_CHG_IMP  ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_FG),   wxSOLID );
	paBrushes[X_CON_IMP  ] = new wxBrush( gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG),       wxSOLID );
}

void Glance::_freeBrushes(wxBrush ** paBrushes)
{
	for (int k=X__ZERO; k<X__LIMIT; k++)
		if (paBrushes[k])
			delete (paBrushes[k]);
}

//////////////////////////////////////////////////////////////////

void Glance::_getFirstRowVisibleForBar(const de_sync ** ppSync, long * pOffsetInSync) const
{
	size_t yRowThumb = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxVERTICAL);
	const TVector_Display * pDis   = m_pDeDe->getDisplayList(m_kSync);

	size_t eofRow = pDis->size() - 1;
	if (yRowThumb >= eofRow)			// if row is out of bounds, grab 
		yRowThumb = eofRow;				// row representing the EOF sync-node.

	const de_row & rDeRow = (*pDis)[yRowThumb];

	*ppSync = rDeRow.getSync();
	*pOffsetInSync = rDeRow.getOffsetInSync();
}

void Glance::_getLastRowVisibleForBar(PanelIndex kPanel, const de_sync ** ppSync, long * pOffsetInSync) const
{
	size_t yRowThumb = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxVERTICAL);
	size_t yRowEnd = yRowThumb + m_pViewFile->getPanel(m_kSync,kPanel)->getRowsDisplayable() + 1;

	const TVector_Display * pDis   = m_pDeDe->getDisplayList(m_kSync);

	size_t eofRow = pDis->size() - 1;

	// yRowEnd := thumb + page_size, so it may or may not exist in the document.
	//
	// but there is a problem with this when we are in different display-modes
	// (like show-diffs or show-with-context).  in these cases, if we just return
	// the sync-node of the EOF and there is a LARGE chunk of EQ lines below the
	// last change, it will make the "side bars" appear too long (and touch the
	// bottom of the glance bar) -- which looks a little odd and isn't consistent
	// with how we display the top of the bars.  so we may need to tweak it some.

	if (yRowEnd < eofRow)		// in-bounds and well-defined
	{
		const de_row & rDeRow = (*pDis)[yRowEnd];

		*ppSync = rDeRow.getSync();
		*pOffsetInSync = rDeRow.getOffsetInSync();
		return;
	}
	if (eofRow == 0)			// empty display, just return EOF row
	{
		const de_row & rDeRow = (*pDis)[eofRow];

		*ppSync = rDeRow.getSync();
		*pOffsetInSync = rDeRow.getOffsetInSync();
		return;
	}

	// so, we are looking at the EOF row and there is stuff in the display list.
	// back up to the last actually-displayed row (get it's (sync-node,offset))
	// and then step forward one (to the first non-displayed line -OR- the EOF).

	size_t rowPrev = eofRow - 1;
	const de_row & rDeRowPrev = (*pDis)[rowPrev];
	
	const de_sync * pSyncPrev = rDeRowPrev.getSync();
	long offsetPrev = rDeRowPrev.getOffsetInSync();

	if (offsetPrev+1 < pSyncPrev->getMaxLen())
	{
		*ppSync = pSyncPrev;
		*pOffsetInSync = offsetPrev+1;
	}
	else
	{
		*ppSync = pSyncPrev->getNext();
		*pOffsetInSync = 0;
	}
}
