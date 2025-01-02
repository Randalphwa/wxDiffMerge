// ViewFilePanel.cpp
// Window for viewing/editing an individual text file -- one of
// the panels in a file-diff/-merge -- may be one of top or bottom
// panels.
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

BEGIN_EVENT_TABLE(ViewFilePanel, wxWindow)
	EVT_PAINT(ViewFilePanel::OnPaint)
	EVT_SIZE(ViewFilePanel::onSize)
	EVT_ERASE_BACKGROUND(ViewFilePanel::onEraseBackground)

	EVT_SET_FOCUS(ViewFilePanel::onSetFocusEvent)
	EVT_KILL_FOCUS(ViewFilePanel::onKillFocusEvent)

	EVT_KEY_DOWN(ViewFilePanel::onKeyDownEvent)
	EVT_CHAR(ViewFilePanel::onCharEvent)

	EVT_LEFT_DOWN(ViewFilePanel::onMouseLeftDown)
	EVT_LEFT_DCLICK(ViewFilePanel::onMouseLeftDClick)
	EVT_LEFT_UP(ViewFilePanel::onMouseLeftUp)

	EVT_RIGHT_DOWN(ViewFilePanel::onMouseRightDown)

	EVT_MOUSEWHEEL(ViewFilePanel::onMouseEventWheel)

	EVT_MIDDLE_DOWN(ViewFilePanel::onMouseMiddleDown)

	EVT_MOTION(ViewFilePanel::onMouseMotion)
	EVT_ENTER_WINDOW(ViewFilePanel::onMouseEnterWindow)
	EVT_LEAVE_WINDOW(ViewFilePanel::onMouseLeaveWindow)

#define ID_TIMER_MY_MOUSE 1000	// distinct from View::ID_TIMER_*
	EVT_TIMER(ID_TIMER_MY_MOUSE, ViewFilePanel::onTimerEvent_MyMouse)

	EVT_MENU(CTX_DELETE,			ViewFilePanel::_onMenuEvent_CTX_DELETE)
	EVT_MENU(CTX_INSERT_L,			ViewFilePanel::_onMenuEvent_CTX_INSERT_L)
	EVT_MENU(CTX_INSERT_BEFORE_L,	ViewFilePanel::_onMenuEvent_CTX_INSERT_BEFORE_L)
	EVT_MENU(CTX_INSERT_AFTER_L,	ViewFilePanel::_onMenuEvent_CTX_INSERT_AFTER_L)
	EVT_MENU(CTX_REPLACE_L,			ViewFilePanel::_onMenuEvent_CTX_REPLACE_L)
	EVT_MENU(CTX_INSERT_R,			ViewFilePanel::_onMenuEvent_CTX_INSERT_R)
	EVT_MENU(CTX_INSERT_BEFORE_R,	ViewFilePanel::_onMenuEvent_CTX_INSERT_BEFORE_R)
	EVT_MENU(CTX_INSERT_AFTER_R,	ViewFilePanel::_onMenuEvent_CTX_INSERT_AFTER_R)
	EVT_MENU(CTX_REPLACE_R,			ViewFilePanel::_onMenuEvent_CTX_REPLACE_R)
	EVT_MENU(CTX_DELETE_MARK,		ViewFilePanel::_onMenuEvent_CTX_DELETE_MARK)
	EVT_MENU(CTX_CUT,				ViewFilePanel::_onMenuEvent_CTX_CUT)
	EVT_MENU(CTX_COPY,				ViewFilePanel::_onMenuEvent_CTX_COPY)
	EVT_MENU(CTX_PASTE,				ViewFilePanel::_onMenuEvent_CTX_PASTE)
	EVT_MENU(CTX_SELECT_ALL,		ViewFilePanel::_onMenuEvent_CTX_SELECT_ALL)
	EVT_MENU(CTX_NEXT_CHANGE,		ViewFilePanel::_onMenuEvent_CTX_NEXT_CHANGE)
	EVT_MENU(CTX_PREV_CHANGE,		ViewFilePanel::_onMenuEvent_CTX_PREV_CHANGE)
	EVT_MENU(CTX_NEXT_CONFLICT,		ViewFilePanel::_onMenuEvent_CTX_NEXT_CONFLICT)
	EVT_MENU(CTX_PREV_CONFLICT,		ViewFilePanel::_onMenuEvent_CTX_PREV_CONFLICT)
	EVT_MENU(CTX_SELECT_PATCH,		ViewFilePanel::_onMenuEvent_CTX_SELECT_PATCH)
	EVT_MENU(CTX_DIALOG_MARK,		ViewFilePanel::_onMenuEvent_CTX_DIALOG_MARK)

END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

static void s_cb_font_change(void * pThis, const util_cbl_arg & /*arg*/)	{ ((ViewFilePanel *)pThis)->cb_font_change(); }
static void s_cb_fl_changed(void * pThis, const util_cbl_arg & arg)			{ ((ViewFilePanel *)pThis)->cb_fl_changed(arg); }
static void s_cb_de_changed(void * pThis, const util_cbl_arg & arg)			{ ((ViewFilePanel *)pThis)->cb_de_changed(arg); }

static void _cb_long(void * pThis, const util_cbl_arg & arg)
{
	GlobalProps::EnumGPL id = (GlobalProps::EnumGPL)arg.m_l;

	ViewFilePanel * pViewFilePanel = (ViewFilePanel *)pThis;
	pViewFilePanel->gp_cb_long(id);
}

//////////////////////////////////////////////////////////////////

ViewFilePanel::ViewFilePanel(wxWindow * pParent, long style, ViewFile * pViewFile, PanelIndex kPanel, long kSync)
	: wxWindow(pParent,-1,wxDefaultPosition,wxDefaultSize,style),
	  m_pViewFile(pViewFile),
	  m_kPanel(kPanel),
	  m_kSync(kSync),
	  m_pFlFl(NULL),
	  m_pDeDe(NULL),
	  m_bWeCapturedTheMouse(false),
	  m_rowRightMouse(-1),
	  m_bRecalc(true),
	  m_rowsDisplayable(0), m_colsDisplayable(0),
	  m_pixelsPerRow(0),    m_pixelsPerCol(0),
	  m_nrDigitsLineNr(0),
	  m_xPixelLineNr(0), m_xPixelLineNrRight(0), m_xPixelIcon(0), m_xPixelBar(0), m_xPixelText(0),
	  m_colGoal(0)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);		// needed for wxAutoBufferedPaintDC()

	m_timer_MyMouse.SetOwner(this,wxID_ANY);
	SetBackgroundColour(*wxWHITE);
	gpViewFileFont->addChangeCB(s_cb_font_change,this);

	m_bShowLineNumbers = pViewFile->getShowLineNumbers();
	m_nrTopPanels = pViewFile->getNrTopPanels();

	if (m_nrTopPanels == 3)	// panel in a 3-way
	{
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,        _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_UNIMP_FG,  _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_FG,        _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,          _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_UNIMP_FG,_cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG,      _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_UNIMP_FG,     _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_FG,  _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_FG,     _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_BG,      _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,        _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_BG,          _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_BG,  _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_BG,     _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_IL_BG,   _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_IL_BG,_cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_IL_BG,   _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_EOL_UNKNOWN_FG,   _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_FG,        _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_BG,        _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CARET_FG,         _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,          _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_VOID_BG,          _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_FG,    _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_BG,    _cb_long, this);
	}
	else // panel in a 2-way
	{
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,        _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_UNIMP_FG, _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_FG,       _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,         _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_UNIMP_FG,_cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_FG,      _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,       _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_BG,         _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_BG,      _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_IL_BG,   _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_EOL_UNKNOWN_FG,  _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_FG,       _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_BG,       _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_CARET_FG,         _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,         _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_VOID_BG,         _cb_long, this);

		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_FG,    _cb_long, this);
		gpGlobalProps->addLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_BG,    _cb_long, this);
	}
	
}

ViewFilePanel::~ViewFilePanel(void)
{
	if (m_nrTopPanels == 3)	// panel in a 3-way
	{
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,        _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_UNIMP_FG, _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_FG,        _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,          _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_UNIMP_FG,_cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG,      _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_UNIMP_FG,     _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_FG,  _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_FG,     _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_BG,      _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,        _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_BG,          _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_BG,  _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_BG,     _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CONFLICT_IL_BG,   _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_IL_BG,_cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_IL_BG,   _cb_long, this);
		
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_EOL_UNKNOWN_FG,   _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_FG,        _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_BG,        _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CARET_FG,         _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,          _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_VOID_BG,          _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_FG,     _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_BG,     _cb_long, this);
	}
	else // panel in a 2-way
	{
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,        _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_UNIMP_FG, _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_FG,       _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_FG,         _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_UNIMP_FG,_cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_FG,      _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,       _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_OMIT_BG,         _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_BG,      _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_NONE_EQ_IL_BG,   _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_EOL_UNKNOWN_FG,  _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_FG,       _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_LINENR_BG,       _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_CARET_FG,         _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_VOID_FG,         _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_VOID_BG,         _cb_long, this);

		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_FG,     _cb_long, this);
		gpGlobalProps->delLongCB(GlobalProps::GPL_FILE_COLOR_SELECTION_BG,     _cb_long, this);
	}

	gpViewFileFont->delChangeCB(s_cb_font_change,this);

	if (m_pDeDe)
		m_pDeDe->delChangeCB(s_cb_de_changed,this);

	if (m_pFlFl)
		m_pFlFl->delChangeCB(s_cb_fl_changed,this);

	while (HasCapture())
		ReleaseMouse();
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::cb_font_change(void)
{
	// the font we use to draw the file panels has changed.
	// we get this via ViewFileFont ChangeCB rather than
	// from a global prop change notification -- because ViewFileFont
	// has already parsed and rendered the font from the description
	// in global props.

	m_bRecalc = true;

	// if the font changes, we should just redraw everything.
	
	Refresh(true);		// effectively InvalidateRect(NULL)
}

void ViewFilePanel::gp_cb_long(GlobalProps::EnumGPL /*id*/)
{
	// one of the "long" global variables that we care about has changed.
	// this means that one of the _FILE_VIEW_ colors has changed.
	// mark this window as requiring a re-paint

	Refresh(true);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::bind_dede_flfl(de_de * pDeDeNew, fl_fl * pFlFlNew)
{
	// layout/ptable wasn't loaded/ready when we were created, so
	// we get bound to it later when it is created/ready.

	if (m_pDeDe)	m_pDeDe->delChangeCB(s_cb_de_changed,this);
	if (m_pFlFl)	m_pFlFl->delChangeCB(s_cb_fl_changed,this);

	m_pFlFl = pFlFlNew;
	m_pDeDe = pDeDeNew;

	// NOTE: we catch both fl and de change events.  it is our hope
	// NOTE: that de changes will reflect content changes that alter
	// NOTE: the de's display list (relative sync, important/unimportant,
	// NOTE: etc) -- things that will affect a lot of things.  whereas
	// NOTE: fl changes might be just a tab-stop or set-prop -- things
	// NOTE: that might change how we draw it, but not anything else.
	// NOTE: [fl changes include content changes, which should cause
	// NOTE: a similar de change, so some of these will cause an echo.]
	
	if (m_pFlFl)	m_pFlFl->addChangeCB(s_cb_fl_changed,this);
	if (m_pDeDe)	m_pDeDe->addChangeCB(s_cb_de_changed,this);

	Refresh(true);		// effectively InvalidateRect(NULL)
}

void ViewFilePanel::cb_fl_changed(const util_cbl_arg & /*arg*/)
{
	// note: arg.m_p contains the first line affected during this change.
	// note: arg.m_l contains the FL_INV_ code for this change.

	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel:[sync %ld][panel %d]:fl_changed"), m_kSync,m_kPanel);

	// something in this layout changed/moved/etc.
	// mark this panel as requiring a re-format and re-paint.

	// TODO see if we can find a way to minimize what needs repainting.
	// TODO for now, just invalidate the whole window and let it redraw.
	// TODO but don't bother trying to get too smart since diff-engine
	// TODO may also need to re-run...

	if (m_bShowLineNumbers)
	{
		// if the number of lines in the file changed and crossed a
		// power of 10, we need to recompute the number of digits in
		// the line number and reset the left margin.

		int nr = _computeDigitsRequiredForLineNumber();
		if (nr != m_nrDigitsLineNr)
			m_bRecalc = true;
	}

	// when the document changes we need to clear the selection.
	// this helps when either they edit or reload the document.

	setBogusCaret();
	
	Refresh(true);		// effectively InvalidateRect(NULL)
}

void ViewFilePanel::cb_de_changed(const util_cbl_arg & arg)
{
	static long masks[__NR_SYNCS__] = { DE_CHG__VIEW_MASK,		// SYNC_VIEW
										DE_CHG__EDIT_MASK };	// SYNC_EDIT

	// note: arg.m_p contains nothing.
	// note: arg.m_l contains the DE_CHG_ code for this change.

	long chg = arg.m_l;
	bool bSignificantForUs = ((chg & masks[m_kSync]) != 0);

	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel:[sync %ld][panel %d]:de_changed [0x%lx] significant [%d]"), m_kSync,m_kPanel,chg,bSignificantForUs);

	// something changed in the diff-engine.
	// mark this panel as requiring a re-paint.

	// TODO see if we can find a way to minimize what needs repainting.
	// TODO for now, just invalidate the whole window and let it redraw.

	if (bSignificantForUs)
		Refresh(true);		// effectively InvalidateRect(NULL)
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onSize(wxSizeEvent & /*e*/)
{
	m_bRecalc = true;

	// TODO for now, just invalidate and repaint everything
	// TODO in the window.  later we may want to try to just
	// TODO repaint newly exposed parts.

	Refresh(true);		// effectively InvalidateRect(NULL)
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onEraseBackground(wxEraseEvent & /*e*/)
{
	// wxDC * pDC = e.GetDC();

	// Do nothing since OnPaint() completely redraws the window.
	// This prevents flashing on MSW.
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onSetFocusEvent(wxFocusEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:SetFocus: [%p] [previous focus %p]"),
	//		   this, e.GetWindow());

	m_pViewFile->setPanelWithFocus((int)m_kPanel);

	e.Skip();
}

void ViewFilePanel::onKillFocusEvent(wxFocusEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:KillFocus: [%p] [focus going to %p]"),
	//		   this, e.GetWindow());

	e.Skip();
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::setShowLineNumbers(bool bOn)
{
	if (bOn != m_bShowLineNumbers)
		m_bRecalc = true;
	
	m_bShowLineNumbers = bOn;
}
