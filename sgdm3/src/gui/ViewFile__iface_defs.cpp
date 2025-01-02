// ViewFile__iface_defs.cpp
// View onto 2-way/3-way file set.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fl.h>
#include <fd.h>
#include <fs.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////
// we tweak the various margins for the different platforms.  there aren't any hard-n-fast
// rules here -- just what i thought looked better -- with respect to the OS's window
// decorations and how the individual platform widgets look.

// the wxSplitter class sets the wxTAB_TRAVERSAL style bit,
// this causes TAB/SHIFT-TAB key events to generate focus
// events (cycling between the splitter children).  unfortunately,
// this also causes the ARROW keys to behave funky (at least on
// MSW and GTK) and also do the focus cycling thing.
//
// so we add the wxWANTS_CHARS style bit to our ViewFilePanels to
// defeat that and let us have the ARROW keys.  (see wxWANTS_CHARS
// in wx_wxwindow.html for note about wxNavigationKeyEvent.)

#if defined(__WXMSW__)

#	define MY_TOPLEVEL_TOP_MARGIN			4		// margin above notebook
#	define MY_TOPLEVEL_SIDE_MARGIN			0		// margin on outside of notebook and task-list
#	define MY_TOPLEVEL_BOTTOM_MARGIN		2		// margin below notebook or task-list
#	define MY_NB_PAGE_GLANCE_LEFT_MARGIN	2		// left margin within notebook (between glance and edge of notebook)
#	define MY_NB_PAGE_GLANCE_PANEL_GAP		4		// width of gap between glance and scrolled-window
#	define MY_NB_PAGE_PANEL_RIGHT_MARGIN	0		// right margin within notebook (between scrolled-window and edge of notebook)

#	define MY_SCROLL_WIN_STYLE	wxNO_BORDER | wxCLIP_CHILDREN
#	define MY_SUB_WIN_STYLE		wxNO_BORDER | wxCLIP_CHILDREN
#	define MY_TITLE_WIN_STYLE	wxNO_BORDER | wxCLIP_CHILDREN
#	define MY_VFP_STYLE			wxSIMPLE_BORDER | wxWANTS_CHARS
#	define MY_NB_WIN_STYLE		0
#	define MY_NB_PAGE_WIN_STYLE	0

#	define MY_FIND_HGAP 4
#	define MY_FIND_VGAP 4
#	define MY_FIND_LINE_ABOVE 1
#	define MY_FIND_BUTTON_STYLE		(wxBU_AUTODRAW)		// | wxBORDER_NONE

#elif defined(__WXGTK__)

#	define MY_TOPLEVEL_TOP_MARGIN		4
#	define MY_TOPLEVEL_SIDE_MARGIN		4
#	define MY_TOPLEVEL_BOTTOM_MARGIN	4
#	define MY_NB_PAGE_GLANCE_LEFT_MARGIN	7
#	define MY_NB_PAGE_GLANCE_PANEL_GAP		7
#	define MY_NB_PAGE_PANEL_RIGHT_MARGIN	7

#	define MY_SCROLL_WIN_STYLE	wxNO_BORDER
#	define MY_SUB_WIN_STYLE		wxNO_BORDER
#	define MY_TITLE_WIN_STYLE	wxNO_BORDER
#	define MY_VFP_STYLE			wxSIMPLE_BORDER | wxWANTS_CHARS
#	define MY_NB_WIN_STYLE		0
#	define MY_NB_PAGE_WIN_STYLE	0

#	define MY_FIND_HGAP 4
#	define MY_FIND_VGAP 4
#	define MY_FIND_LINE_ABOVE 1
#	define MY_FIND_BUTTON_STYLE		(wxBU_AUTODRAW)		// | wxBORDER_NONE

#elif defined(__WXMAC__)

#	define MY_TOPLEVEL_TOP_MARGIN		4
#	define MY_TOPLEVEL_SIDE_MARGIN		4
#	define MY_TOPLEVEL_BOTTOM_MARGIN	4
#	define MY_T_TITLE_MARGIN			4
#	define MY_NB_PAGE_GLANCE_LEFT_MARGIN	7
#	define MY_NB_PAGE_GLANCE_PANEL_GAP		7
#	define MY_NB_PAGE_PANEL_RIGHT_MARGIN	7

#	define MY_SCROLL_WIN_STYLE	wxNO_BORDER
#	define MY_SUB_WIN_STYLE		wxNO_BORDER
#	define MY_TITLE_WIN_STYLE	wxNO_BORDER
#	define MY_VFP_STYLE			wxSIMPLE_BORDER | wxWANTS_CHARS
#	define MY_NB_WIN_STYLE		0
#	define MY_NB_PAGE_WIN_STYLE	0

#	define MY_FIND_HGAP 4
#	define MY_FIND_VGAP 4
#	define MY_FIND_LINE_ABOVE 1
#	define MY_FIND_BUTTON_STYLE		(wxBU_AUTODRAW | wxBORDER_NONE)

#endif

#define MY_FIND_GOTO_SEP_MARGIN 8
#define MY_GOTO_MARGIN 4

//////////////////////////////////////////////////////////////////

void ViewFile::_layout_enable_edit_panel(void)
{
	wxASSERT_MSG( (m_pNotebook), _T("Coding Error") );
	wxASSERT_MSG( (m_pNotebook->GetPageCount()==1), _T("Coding Error") );
	wxASSERT_MSG( (getEditPanelPresent()), _T("Coding Error") );

	_createLayout_nb_page(m_pNotebook,SYNC_EDIT);
}

//////////////////////////////////////////////////////////////////

#define M 7
#define FIX 0
#define VAR 1

void ViewFile::createLayout(void)
{
	// create our widget layout.  this is everything between the toolbar and the status bar.
	// we have several possibilities:
	//
	// [] if this is strictly a read-only viewer, then we do not need the notebook with
	//    the SYNC_VIEW tab.  (in this case, we are a strictly a viewer).
	// [] when editing is enabled or possible, we do need the notebook.

	wxBoxSizer * pSizerThis = new wxBoxSizer(wxVERTICAL);

	{
		_create_find_panel();
		pSizerThis->Add(m_pPanelFind, FIX, wxEXPAND, 0);

		pSizerThis->AddSpacer(MY_TOPLEVEL_TOP_MARGIN);

		if (!getEditPanelPresent())
		{
			_createLayout_nb_page(this,SYNC_VIEW);
			pSizerThis->Add(m_nbPage[SYNC_VIEW].m_pNotebookPanel, VAR, wxEXPAND | wxLEFT|wxRIGHT, MY_TOPLEVEL_SIDE_MARGIN);
			m_currentNBSync = SYNC_VIEW;
		}
		else
		{
			m_pNotebook = new _my_notebook(this,this,wxNB_BOTTOM | MY_NB_WIN_STYLE);
			{
				_createLayout_nb_page(m_pNotebook,SYNC_VIEW);
				m_pNotebook->AddPage(m_nbPage[SYNC_VIEW].m_pNotebookPanel, _("Reference View (Files as Loaded)") );
				// WARNING: the first AddPage() causes a page-changed-event on Win32 and MAC,
				// WARNING: but doesn't seem to on GTK, so we need to set m_currentNBSync
				// WARNING: just like we do in onEvent_NotebookChanged().
				m_currentNBSync = SYNC_VIEW;
			}
			pSizerThis->Add(m_pNotebook, VAR, wxEXPAND | wxLEFT|wxRIGHT, MY_TOPLEVEL_SIDE_MARGIN);
		}
		
		pSizerThis->AddSpacer(MY_TOPLEVEL_BOTTOM_MARGIN);
	}

	SetSizer(pSizerThis);	// associate top-most layout with our window
	Layout();				// forcing an initial layout helps the mac

	// set initial focus

	m_nbPage[SYNC_VIEW].m_pWinPanel[PANEL_T1]->SetFocus();
}

//////////////////////////////////////////////////////////////////

#include <Resources/FindPanel/close20.xpm>
#include <Resources/FindPanel/down20.xpm>
#include <Resources/FindPanel/up20.xpm>

//////////////////////////////////////////////////////////////////

// Create the sometimes-visible Find Panel.
// Our caller needs to actually add it in
// the right position to the outer sizer.
void ViewFile::_create_find_panel(void)
{
	m_pPanelFind = new wxPanel(this);
	{
		wxBoxSizer * pSizer_Find_V = new wxBoxSizer(wxVERTICAL);
		{
#if MY_FIND_LINE_ABOVE
			pSizer_Find_V->Add( new wxStaticLine(m_pPanelFind), FIX, wxEXPAND, 0);
#endif

			wxBoxSizer * pSizer_Find_H = new wxBoxSizer(wxHORIZONTAL);
			{
				m_pButtonFindClose = new wxBitmapButton(m_pPanelFind, ID_FINDPANEL_CLOSE,
														wxBitmap((const char **)close20_xpm),
														wxDefaultPosition, wxDefaultSize,
														MY_FIND_BUTTON_STYLE);
				pSizer_Find_H->Add( m_pButtonFindClose, FIX,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);

				pSizer_Find_H->Add( new wxStaticText(m_pPanelFind, wxID_ANY, _T("Find:")),
									FIX,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);

				m_pTextFind = new wxTextCtrl(m_pPanelFind, ID_FINDPANEL_TEXT, _T(""),
											 wxDefaultPosition, wxDefaultSize,
											 wxTE_PROCESS_ENTER,
											 wxDefaultValidator, _T("text"));
				pSizer_Find_H->Add( m_pTextFind, VAR,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);

				m_pButtonFindNext = new wxBitmapButton(m_pPanelFind, ID_FINDPANEL_NEXT,
													   wxBitmap((const char **)down20_xpm),
													   wxDefaultPosition, wxDefaultSize,
													   MY_FIND_BUTTON_STYLE);
				pSizer_Find_H->Add( m_pButtonFindNext, FIX,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);

				m_pButtonFindPrev = new wxBitmapButton(m_pPanelFind, ID_FINDPANEL_PREV,
													   wxBitmap((const char **)up20_xpm),
													   wxDefaultPosition, wxDefaultSize,
													   MY_FIND_BUTTON_STYLE);
				pSizer_Find_H->Add( m_pButtonFindPrev, FIX,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);

				m_pCheckFindMatchCase = new wxCheckBox(m_pPanelFind, ID_FINDPANEL_MATCHCASE, _T("Match Case"));
				pSizer_Find_H->Add( m_pCheckFindMatchCase, FIX,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);

				//////////////////////////////////////////////////////////////////

				pSizer_Find_H->Add( new wxStaticLine(m_pPanelFind, wxID_ANY,
													 wxDefaultPosition, wxDefaultSize,
													 wxLI_VERTICAL),
									FIX, wxEXPAND | wxLEFT | wxRIGHT, MY_FIND_GOTO_SEP_MARGIN);

				//////////////////////////////////////////////////////////////////

				pSizer_Find_H->Add( new wxStaticText(m_pPanelFind, wxID_ANY, _T("Go To:")),
									FIX,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);

				wxIntegerValidator<unsigned int> val(&m_uiDataGoTo);
				val.SetMin(1);	// we show things 1-based
				// no upper limit on field since the user can edit the files and we don't
				// want to bother tracking their length.  we'll clip this on ENTER.
				m_pTextGoTo = new wxTextCtrl(m_pPanelFind, ID_FINDPANEL_GOTO, _T(""),
											 wxDefaultPosition, wxSize(40,-1),
											 wxTE_PROCESS_ENTER,
											 val);
				pSizer_Find_H->Add( m_pTextGoTo, FIX,
									wxALIGN_CENTER_VERTICAL |wxLEFT|wxRIGHT,
									MY_FIND_HGAP);
			}
			pSizer_Find_V->Add( pSizer_Find_H, FIX,
								wxEXPAND |wxTOP|wxBOTTOM,
								MY_FIND_VGAP);

			pSizer_Find_V->Add( new wxStaticLine(m_pPanelFind), FIX, wxEXPAND, 0);
		}
		m_pPanelFind->SetSizer(pSizer_Find_V);
	}

	m_pPanelFind->Hide();
}

void ViewFile::_createLayout_nb_page(wxWindow * pParent, long kSync)
{
	m_nbPage[kSync].m_pNotebookPanel = new wxPanel(pParent,kSync,wxDefaultPosition,wxDefaultSize, wxTAB_TRAVERSAL | MY_NB_PAGE_WIN_STYLE);
	{
		wxBoxSizer * pSizerPanel = new wxBoxSizer(wxHORIZONTAL);
		{
			if (MY_NB_PAGE_GLANCE_LEFT_MARGIN)
				pSizerPanel->AddSpacer(MY_NB_PAGE_GLANCE_LEFT_MARGIN);

			m_nbPage[kSync].m_pWinGlance = (  (getNrTopPanels()==2)
											? (Glance *)new Glance2(m_nbPage[kSync].m_pNotebookPanel,wxSIMPLE_BORDER,this,kSync)
											: (Glance *)new Glance3(m_nbPage[kSync].m_pNotebookPanel,wxSIMPLE_BORDER,this,kSync));
			pSizerPanel->Add(m_nbPage[kSync].m_pWinGlance, FIX, wxEXPAND | wxTOP|wxBOTTOM, M);

			if (MY_NB_PAGE_GLANCE_PANEL_GAP)
				pSizerPanel->AddSpacer(MY_NB_PAGE_GLANCE_PANEL_GAP);

			wxBoxSizer * pSizerV = new wxBoxSizer(wxVERTICAL);
			{
				m_nbPage[kSync].m_pWinScroll = new _my_scrolled_win(m_nbPage[kSync].m_pNotebookPanel,this,MY_SCROLL_WIN_STYLE, kSync);

				_create_top_panels(kSync);

				m_nbPage[kSync].m_pWinScroll->setChild(m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]);

				pSizerV->Add(m_nbPage[kSync].m_pWinScroll, VAR, wxEXPAND, 0);

				m_nbPage[kSync].m_pWinChangeStatusText = new MyStaticText(m_nbPage[kSync].m_pNotebookPanel,MY_TITLE_WIN_STYLE,_T(""),MY_STATIC_TEXT_ATTRS_ALIGN_RIGHT);
				pSizerV->Add(m_nbPage[kSync].m_pWinChangeStatusText, FIX, wxEXPAND|wxTOP, M);

			}
			pSizerPanel->Add(pSizerV, VAR, wxEXPAND | wxTOP|wxBOTTOM, M);

			if (MY_NB_PAGE_PANEL_RIGHT_MARGIN)
				pSizerPanel->AddSpacer(MY_NB_PAGE_PANEL_RIGHT_MARGIN);
		}
		m_nbPage[kSync].m_pNotebookPanel->SetSizer(pSizerPanel);
	}

	// don't add the wxPanel (the top-level widget of what we just created)
	// to the notebook yet -- there are a bunch of pointers that aren't
	// connected yet and the ADD causes a bunch of resize events.
}

//////////////////////////////////////////////////////////////////

void ViewFileDiff::_create_top_panels(long kSync)
{
	static MyStaticTextAttrs attrs[__NR_TOP_PANELS__] = { MY_STATIC_TEXT_ATTRS_ALIGN_LEFT,
														  MY_STATIC_TEXT_ATTRS_ALIGN_RIGHT,
														  0	};
	
	m_nbPage[kSync].m_pWinSplitter[SPLIT_V1] = new _my_splitter(m_nbPage[kSync].m_pWinScroll,this,SPLIT_V1,kSync,0.5f);

	wxWindow * pWin[__NR_TOP_PANELS__];
	for (int kPanel=PANEL_T0; kPanel<PANEL_T2; kPanel++)
	{
		pWin[kPanel] = new wxPanel(m_nbPage[kSync].m_pWinSplitter[SPLIT_V1],wxID_ANY,
								   wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | MY_SUB_WIN_STYLE);
		{
			wxBoxSizer * pSizerV = new wxBoxSizer(wxVERTICAL);
			{
				m_nbPage[kSync].m_pWinTitle[kPanel] = new MyStaticText(pWin[kPanel],MY_TITLE_WIN_STYLE,_T(""),attrs[kPanel]);
				pSizerV->Add(m_nbPage[kSync].m_pWinTitle[kPanel], FIX, wxEXPAND | wxLEFT|wxRIGHT, 1);

				pSizerV->AddSpacer(M);

				m_nbPage[kSync].m_pWinPanel[kPanel] = new ViewFilePanel(pWin[kPanel],MY_VFP_STYLE,this,(PanelIndex)kPanel,kSync);
				pSizerV->Add(m_nbPage[kSync].m_pWinPanel[kPanel], VAR, wxEXPAND | wxLEFT|wxRIGHT, 1);
			}

			pWin[kPanel]->SetSizer(pSizerV);
		}
	}
	m_nbPage[kSync].m_pWinTitle[PANEL_T2] = NULL;
	m_nbPage[kSync].m_pWinPanel[PANEL_T2] = NULL;

	if (gpGlobalProps->getBool((kSync == SYNC_VIEW)
							   ? GlobalProps::GPL_VIEW_FILE_DIFF_VIEW_SPLITTER_ORIENTATION
							   : GlobalProps::GPL_VIEW_FILE_DIFF_EDIT_SPLITTER_ORIENTATION))
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitVertically(pWin[PANEL_T0],pWin[PANEL_T1],0.5f);
	else
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitHorizontally(pWin[PANEL_T0],pWin[PANEL_T1],0.5f);

	m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->SetMinimumPaneSize(MY_V_SPLITTER_MIN);
}

//////////////////////////////////////////////////////////////////

void ViewFileMerge::_create_top_panels(long kSync)
{
	static MyStaticTextAttrs attrs[__NR_TOP_PANELS__] = { MY_STATIC_TEXT_ATTRS_ALIGN_LEFT,
														  MY_STATIC_TEXT_ATTRS_ALIGN_CENTER,
														  MY_STATIC_TEXT_ATTRS_ALIGN_RIGHT };
	static Split splitParent[__NR_TOP_PANELS__] = { SPLIT_V1, SPLIT_V2, SPLIT_V2 };

	m_nbPage[kSync].m_pWinSplitter[SPLIT_V1] = new _my_splitter(m_nbPage[kSync].m_pWinScroll,this,SPLIT_V1,kSync,0.33f);
	m_nbPage[kSync].m_pWinSplitter[SPLIT_V2] = new _my_splitter(m_nbPage[kSync].m_pWinSplitter[SPLIT_V1],this,SPLIT_V2,kSync,0.33f);

	wxWindow * pWin[__NR_TOP_PANELS__];
	for (int kPanel=PANEL_T0; kPanel<__NR_TOP_PANELS__; kPanel++)
	{
		pWin[kPanel] = new wxPanel(m_nbPage[kSync].m_pWinSplitter[splitParent[kPanel]],wxID_ANY,
								   wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | MY_SUB_WIN_STYLE);
		{
			wxBoxSizer * pSizerV = new wxBoxSizer(wxVERTICAL);
			{
				m_nbPage[kSync].m_pWinTitle[kPanel] = new MyStaticText(pWin[kPanel],MY_TITLE_WIN_STYLE,_T(""),attrs[kPanel]);
				pSizerV->Add(m_nbPage[kSync].m_pWinTitle[kPanel], FIX, wxEXPAND | wxLEFT|wxRIGHT, 1);

				pSizerV->AddSpacer(M);

				m_nbPage[kSync].m_pWinPanel[kPanel] = new ViewFilePanel(pWin[kPanel],MY_VFP_STYLE,this,(PanelIndex)kPanel,kSync);
				pSizerV->Add(m_nbPage[kSync].m_pWinPanel[kPanel], VAR, wxEXPAND | wxLEFT|wxRIGHT, 1);
			}

			pWin[kPanel]->SetSizer(pSizerV);
		}
	}

	// strive for 33%-33%-33% initial layout

	if (gpGlobalProps->getBool((kSync == SYNC_VIEW)
							   ? GlobalProps::GPL_VIEW_FILE_MERGE_VIEW_SPLITTER_ORIENTATION
							   : GlobalProps::GPL_VIEW_FILE_MERGE_EDIT_SPLITTER_ORIENTATION))
	{
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitVertically(pWin[PANEL_T0],m_nbPage[kSync].m_pWinSplitter[SPLIT_V2],0.33f);
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->MySplitVertically(pWin[PANEL_T1],pWin[PANEL_T2],   0.33f);
	}
	else
	{
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitHorizontally(pWin[PANEL_T0],m_nbPage[kSync].m_pWinSplitter[SPLIT_V2],0.33f);
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->MySplitHorizontally(pWin[PANEL_T1],pWin[PANEL_T2],   0.33f);
	}
	
	m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->SetMinimumPaneSize(MY_V_SPLITTER_MIN*2);	// V1 needs to be bigger so the right side won't collapse over V2
	m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->SetMinimumPaneSize(MY_V_SPLITTER_MIN);
}

//////////////////////////////////////////////////////////////////

void ViewFile::_my_notebook::onKeyDownEvent(wxKeyEvent & e)
{
#if 0 && defined(DEBUG)
	wxLogTrace(wxTRACE_Messages, _T("_my_notebook::onKeyDownEvent: [%p][keycode %d][cmd %d][meta %d][alt %d][control %d][shift %d]"),
			   this,
			   e.GetKeyCode(),
			   e.CmdDown(),
			   e.MetaDown(),
			   e.AltDown(),
			   e.ControlDown(),
			   e.ShiftDown());
#endif

	if (e.GetKeyCode() == WXK_ESCAPE)
	{
		m_pViewFile->getFrame()->postEscapeCloseCommand();
		return;
	}
				
	e.Skip();
}

//////////////////////////////////////////////////////////////////

void ViewFileDiff::setSplittersVertical(long kSync, bool bVertical)
{
	// change orientation of the splitters.

	// update global props with the new value so that the next new
	// window will be created with this mode.

	gpGlobalProps->setLong(((kSync == SYNC_VIEW)
							? GlobalProps::GPL_VIEW_FILE_DIFF_VIEW_SPLITTER_ORIENTATION
							: GlobalProps::GPL_VIEW_FILE_DIFF_EDIT_SPLITTER_ORIENTATION),
						   bVertical);

	if (areSplittersVertical(kSync) == bVertical)	// no change required.
		return;

	wxWindow * pWin0 = m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->GetWindow1();
	wxWindow * pWin1 = m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->GetWindow2();
	
	m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->Unsplit(NULL);

	if (bVertical)
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitVertically(pWin0,pWin1,0.5f);
	else
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitHorizontally(pWin0,pWin1,0.5f);

	adjustVerticalScrollbar(kSync);
	adjustHorizontalScrollbar(kSync);
}

void ViewFileMerge::setSplittersVertical(long kSync, bool bVertical)
{
	// change orientation of the splitters on this view.

	gpGlobalProps->setLong(((kSync == SYNC_VIEW)
							? GlobalProps::GPL_VIEW_FILE_MERGE_VIEW_SPLITTER_ORIENTATION
							: GlobalProps::GPL_VIEW_FILE_MERGE_EDIT_SPLITTER_ORIENTATION),
						   bVertical);

	if (areSplittersVertical(kSync) == bVertical)	// no change required.
		return;

	wxWindow * pWin0 = m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->GetWindow1();
	wxWindow * pWin1 = m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->GetWindow1();
	wxWindow * pWin2 = m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->GetWindow2();
	
	m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->Unsplit(NULL);
	m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->Unsplit(NULL);

	if (bVertical)
	{
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitVertically(pWin0,m_nbPage[kSync].m_pWinSplitter[SPLIT_V2],0.33f);
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->MySplitVertically(pWin1,pWin2,0.33f);
	}
	else
	{
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->MySplitHorizontally(pWin0,m_nbPage[kSync].m_pWinSplitter[SPLIT_V2],0.33f);
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->MySplitHorizontally(pWin1,pWin2,0.33f);
	}

	adjustVerticalScrollbar(kSync);
	adjustHorizontalScrollbar(kSync);
}

