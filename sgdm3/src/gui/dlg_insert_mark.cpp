// dlg_insert_mark.cpp
// modeless dialog to help user insert a manual-alignment mark.
//////////////////////////////////////////////////////////////////
// NOTE: we assume that the frame's notebook page (sync-view or sync-edit)
// NOTE: CANNOT be changed while we are up.  that is, we assume that
// NOTE: ViewFile::onEvent_NotebookChanging() vetos the event when we are
// NOTE: up.  (we could allow it, but we'd need to repopulate the listbox
// NOTE: and re-calibrate the spin boxes.)
//
// TODO if the user edits PANEL_EDIT while we are up and causes the line
// TODO line number of a mark to change, the contents of our listbox will
// TODO be stale.  (and the range on our spin boxes will need to be updated)
// TODO leave this for now.

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <de.h>
#include <fl.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#define DLG_TITLE			_("Manual Alignment Markers")
#define DLG_HELP_TITLE		_("Manual Alignment Marker Help")

#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(dlg_insert_mark,wxDialog)
	EVT_BUTTON(wxID_APPLY,			dlg_insert_mark::OnApply)
	EVT_BUTTON(wxID_CANCEL,			dlg_insert_mark::OnCancel)
	EVT_BUTTON(ID_SHOW_HELP,		dlg_insert_mark::onButton_ShowHelp)
	EVT_BUTTON(ID_BUTTON_DELETE,	dlg_insert_mark::onButton_Delete)
	EVT_BUTTON(ID_BUTTON_DELETE_ALL,dlg_insert_mark::onButton_DeleteAll)
	EVT_LISTBOX(ID_LISTBOX,			dlg_insert_mark::onListBoxSelect)
//	EVT_LISTBOX(ID_LISTBOX,			dlg_insert_mark::onListBoxDClick)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(dlg_insert_mark_help,wxDialog)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

dlg_insert_mark::dlg_insert_mark(gui_frame * pFrame, de_mark * pDeMarkInitial)
	: m_pFrame(pFrame)
{
	m_nrFiles = (m_pFrame->isFileMerge() ? 3 : 2);
	m_pViewFile = static_cast<ViewFile *>(m_pFrame->getView());

	_preload_validated_fields();
	
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	Create(pFrame,-1,DLG_TITLE,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxSizerTry = new wxStaticBoxSizer( new wxStaticBox(this,-1, _("Insert Alignment Marker")), wxVERTICAL);
		{
			wxBoxSizer * hSizerLineNrs = new wxBoxSizer(wxHORIZONTAL);
			{
				hSizerLineNrs->Add( new wxStaticText(this,-1, _("&Line Numbers:")), FIX,wxALIGN_CENTER_VERTICAL|wxRIGHT, M);

				for (int kPanel=0; kPanel<m_nrFiles; kPanel++)
				{
					m_pSpin[kPanel] = new wxSpinCtrl(this,ID_SPIN_LINE_NR_T0,wxEmptyString,wxDefaultPosition,wxDefaultSize,
													 wxSP_ARROW_KEYS,1,_get_spin_limit(kPanel),m_iSpin[kPanel]);	// spin ctrl does not use generic validator
					hSizerLineNrs->Add( m_pSpin[kPanel], FIX,wxALIGN_CENTER_VERTICAL|wxRIGHT, M);
					m_pSpin[kPanel]->SetValue(m_iSpin[kPanel]);		// initial value in ctor does not work
				}

				hSizerLineNrs->AddStretchSpacer(VAR);

				m_pButtonInsert = new wxButton(this,wxID_APPLY,_("&Insert"));
				hSizerLineNrs->Add( m_pButtonInsert, FIX, wxLEFT, M);
			}
			staticBoxSizerTry->Add(hSizerLineNrs, FIX, wxGROW|wxALL,M);
		}
		vSizerTop->Add(staticBoxSizerTry, FIX, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * staticBoxSizerList = new wxStaticBoxSizer( new wxStaticBox(this,-1, _("Current Alignment Markers")), wxVERTICAL);
		{
			wxBoxSizer * hSizerList = new wxBoxSizer(wxHORIZONTAL);
			{
				wxString * array = NULL;
				int cStrings = _allocate_array_of_mark_descriptions(&array);

				m_pListBox = new wxListBox(this,ID_LISTBOX,wxDefaultPosition,wxDefaultSize,
										   cStrings,array,wxLB_SINGLE);
				_free_array_of_mark_descriptions(array);

				hSizerList->Add( m_pListBox, VAR, wxGROW, 0);

				wxBoxSizer * vSizerListButtons = new wxBoxSizer(wxVERTICAL);
				{
					m_pButtonDelete = new wxButton(this,ID_BUTTON_DELETE, _("&Delete"));
					vSizerListButtons->Add( m_pButtonDelete, FIX, wxGROW, 0);

					m_pButtonDeleteAll = new wxButton(this,ID_BUTTON_DELETE_ALL, _("Delete &All"));
					vSizerListButtons->Add( m_pButtonDeleteAll, FIX, wxGROW|wxTOP, M);
				}
				hSizerList->Add(vSizerListButtons, FIX, wxLEFT, M*2);
			}
			staticBoxSizerList->Add(hSizerList, VAR, wxGROW|wxALL, M);
		}
		vSizerTop->Add(staticBoxSizerList, VAR, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		// put horizontal line and set of ok/cancel buttons across the bottom of the dialog

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxTOP|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			m_pButtonHelp = new wxButton(this,ID_SHOW_HELP,_("&Help..."));
			hSizerButtons->Add( m_pButtonHelp, FIX, 0, 0);
			hSizerButtons->AddStretchSpacer(VAR);
			m_pButtonClose = new wxButton(this,wxID_CANCEL,_("&Close"));	// using wxID_CANCEL lets ESC key close dialog
			hSizerButtons->Add(m_pButtonClose, FIX,0,0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);
	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);
	Centre(wxBOTH);

	m_pButtonInsert->SetDefault();	// make INSERT the default button

	if (pDeMarkInitial)
	{
		// if an initial mark given, select it in the listbox
		// and make DELETE the default button.

		int kSync = m_pViewFile->getSync();
		de_de * pDeDe = m_pViewFile->getDE();

		// mark[0] is the trivial mark which must not be shown

		int nrMarks = pDeDe->getNrMarks(kSync);
		int kMarkInitial = -1;
		for (int kMark=1; kMark<nrMarks; kMark++)
		{
			const de_mark * pMark = pDeDe->getNthMark(kSync,kMark);
			if (pMark == pDeMarkInitial)
			{
				kMarkInitial = kMark;
				break;
			}
		}
		if (kMarkInitial != -1)
		{
			m_pListBox->SetFocus();
			m_pListBox->SetSelection(kMarkInitial-1);	// control is 0-based
			m_pButtonDelete->SetDefault();
		}
	}

	_enable_fields();
}

dlg_insert_mark::~dlg_insert_mark(void)
{
	if (m_pFrame)
		m_pFrame->clearInsertMarkDialog();

	//wxLogTrace(wxTRACE_Messages,_T("dlg_insert_mark::DTOR"));
}

//////////////////////////////////////////////////////////////////

void dlg_insert_mark::_preload_validated_fields(void)
{
	for (int kPanel=0; kPanel<m_nrFiles; kPanel++)
		m_iSpin[kPanel] = _get_current_line(kPanel);
}

int dlg_insert_mark::_get_spin_limit(int kPanel)
{
	// return the number of formatted lines in the document.
	// this is not the size of the display-list.

	if (kPanel == -1)
		kPanel = PANEL_T1;
	
	// WARNING: we return the data for the currently active page (sync-edit or sync-view).

	int kSync = m_pViewFile->getSync();
	fl_fl * pFlFl = m_pViewFile->getDE()->getLayout(kSync,(PanelIndex)kPanel);

	return pFlFl->getFormattedLineNrs();
}

int dlg_insert_mark::_get_current_line(int kPanel)
{
	// return the LINE NUMBER in the requested document where the caret is.
	// this is NOT the display-list-row-number.
	//
	// WARNING: we return the data for the currently active page (sync-edit or sync-view).

	int kSync = m_pViewFile->getSync();
	ViewFilePanel * pViewFilePanel = m_pViewFile->getPanel(kSync,(PanelIndex)kPanel);
	if (!pViewFilePanel)
		return 1;

	long row = pViewFilePanel->getCaretOrSelectionRow(false);
	if (row == -1)		// bogus caret
		return 1;		// return line number 1 (not row 1)

	const fl_line * pFlLine = m_pViewFile->getDE()->getFlLineFromDisplayListRow(kSync,(PanelIndex)kPanel,row);
	if (!pFlLine)		// void line on this row -- should not happen
		return 1;		// but return line number 1 if it does.

	return pFlLine->getLineNr() + 1; // convert from 0-based to 1-based
}

void dlg_insert_mark::_enable_fields(void)
{
	bool bHaveListBoxSelection = (m_pListBox->GetSelection() != wxNOT_FOUND);

	m_pButtonDelete->Enable(bHaveListBoxSelection);

	bool bListBoxEmpty = (m_pListBox->IsEmpty());

	m_pButtonDeleteAll->Enable( !bListBoxEmpty );
}

//////////////////////////////////////////////////////////////////

void dlg_insert_mark::OnCancel(wxCommandEvent & /*e*/)
{
	// either CLOSE or title-bar [x] clicked (or ESC pressed)

	// end modeless dialog
	SetReturnCode(wxID_CANCEL);		// used to be wxDialog::OnCancel()
	Hide();

	Destroy();					// let wxWindows safely close/destroy the dialog.
}

void dlg_insert_mark::OnApply(wxCommandEvent & /*e*/)
{
	// insert a mark.

	if (Validate())					// used to be wxDialog::OnApply();
		TransferDataFromWindow();	// get stuff from from fields.

	long alLineNr[__NR_TOP_PANELS__];
	for (int kPanel=0; kPanel<m_nrFiles; kPanel++)
	{
		m_iSpin[kPanel] = m_pSpin[kPanel]->GetValue();	// spin button does not use validators, so we have to do Transfer ourself
		alLineNr[kPanel] = m_iSpin[kPanel] - 1;			// convert from user-visible-1-based to internal-0-based
	}
	
#ifdef DEBUG
//	if (m_nrFiles == 3)
//		wxLogTrace(wxTRACE_Messages,_T("OnApply: lines [%d,%d,%d]"),m_iSpin[PANEL_T0],m_iSpin[PANEL_T1],m_iSpin[PANEL_T2]);
//	else
//		wxLogTrace(wxTRACE_Messages,_T("OnApply: lines [%d,%d]"),m_iSpin[PANEL_T0],m_iSpin[PANEL_T1]);
#endif

	// WARNING: we operation on the currently active page (sync-edit or sync-view).

	int kSync = m_pViewFile->getSync();

	PanelIndex errorPanel;
	de_mark * pDeMarkPreview = NULL;
	util_error ue = m_pViewFile->createMark(kSync, DE_MARK_USER, m_nrFiles, alLineNr, &pDeMarkPreview, &errorPanel);

	if (ue.isErr())
	{
		m_pSpin[errorPanel]->SetFocus();		// use errorPanel to put focus on the offending spin ctrl
		
		wxMessageDialog dlg(this,ue.getMBMessage(),_("Error"),wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}

	// repopulate the listbox with the (now) current set of marks

	wxString * array = NULL;
	int cStrings = _allocate_array_of_mark_descriptions(&array);
	m_pListBox->Set(cStrings,array);
	_free_array_of_mark_descriptions(array);
	_enable_fields();
	
	// warp scroll to make new mark visible.
	m_pViewFile->warpScrollCentered(kSync, m_pViewFile->getDE()->getMarkRowNr(pDeMarkPreview));

	m_pButtonClose->SetFocus();
}

void dlg_insert_mark::onListBoxSelect(wxCommandEvent & /*e*/)
{
	// current selection has changed -- enable {delete} button if necessary
	//
	// wxBUG: we do not get called when the user clears the selection, only when making a selection.

	// if they selected a valid mark, warp scroll and make selected mark visible.
	
	int index = m_pListBox->GetSelection();
	if (index != wxNOT_FOUND)
	{
		int kSync = m_pViewFile->getSync();
		de_de * pDeDe = m_pViewFile->getDE();

		de_mark * pDeMark = pDeDe->getNthMark(kSync, index+1);
		if (pDeMark)
			m_pViewFile->warpScrollCentered(kSync, m_pViewFile->getDE()->getMarkRowNr(pDeMark));
	}

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

int dlg_insert_mark::_allocate_array_of_mark_descriptions(wxString ** array)
{
	// WARNING: we return the data for the currently active page (sync-edit or sync-view).

	int kSync = m_pViewFile->getSync();
	de_de * pDeDe = m_pViewFile->getDE();

	// mark[0] is the trivial mark which must not be shown

	int nrMarks = pDeDe->getNrMarks(kSync);
	if (nrMarks <= 1)
	{
		*array = NULL;
		return 0;
	}

	*array = new wxString[nrMarks-1];
	
	for (int kMark=1; kMark<nrMarks; kMark++)
	{
		const de_mark * pMark = pDeDe->getNthMark(kSync,kMark);
		if (m_nrFiles == 3)
			(*array)[kMark-1] = wxString::Format(_T("Line %d / Line %d / Line %d"),
												 pMark->getDeLine(PANEL_T0)->getFlLine()->getLineNr()+1,
												 pMark->getDeLine(PANEL_T1)->getFlLine()->getLineNr()+1,
												 pMark->getDeLine(PANEL_T2)->getFlLine()->getLineNr()+1);
		else
			(*array)[kMark-1] = wxString::Format(_T("Line %d / Line %d"),
												 pMark->getDeLine(PANEL_T0)->getFlLine()->getLineNr()+1,
												 pMark->getDeLine(PANEL_T1)->getFlLine()->getLineNr()+1);
	}

	return nrMarks-1;
}

void dlg_insert_mark::_free_array_of_mark_descriptions(wxString * array)
{
	delete [] array;
}

//////////////////////////////////////////////////////////////////

void dlg_insert_mark::onButton_ShowHelp(wxCommandEvent & /*e*/)
{
	dlg_insert_mark_help dlg(this);
	dlg.ShowModal();
}

void dlg_insert_mark::onButton_Delete(wxCommandEvent & /*e*/)
{
	int index = m_pListBox->GetSelection();
	if (index == wxNOT_FOUND)		// should not happen because button should not be enabled
		return;
	
	// WARNING: we return the data for the currently active page (sync-edit or sync-view).

	int kSync = m_pViewFile->getSync();
	de_de * pDeDe = m_pViewFile->getDE();

	de_mark * pDeMark = pDeDe->getNthMark(kSync, index+1);
	if (!pDeMark)
		return;

	m_pViewFile->deleteMark(kSync,pDeMark);

	// repopulate the listbox with the (now) current set of marks

	wxString * array = NULL;
	int cStrings = _allocate_array_of_mark_descriptions(&array);
	m_pListBox->Set(cStrings,array);
	_free_array_of_mark_descriptions(array);
	_enable_fields();
}

void dlg_insert_mark::onButton_DeleteAll(wxCommandEvent & /*e*/)
{
	// WARNING: we return the data for the currently active page (sync-edit or sync-view).

	int kSync = m_pViewFile->getSync();

	m_pViewFile->deleteAllMark(kSync);

	// repopulate the listbox with the (now) current set of marks

	wxString * array = NULL;
	int cStrings = _allocate_array_of_mark_descriptions(&array);
	m_pListBox->Set(cStrings,array);
	_free_array_of_mark_descriptions(array);
	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void dlg_insert_mark::externalUpdateLineNr(PanelIndex kPanel, const fl_line * pFlLine)
{
	// somebody (outside of the dialog) wants to update the
	// spin box containing one of the line numbers.

	m_iSpin[(int)kPanel] = pFlLine->getLineNr() + 1;	// convert from 0-based to 1-based
	m_pSpin[kPanel]->SetValue(m_iSpin[kPanel]);
}

//////////////////////////////////////////////////////////////////

dlg_insert_mark_help::dlg_insert_mark_help(wxWindow * pParent)
{
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	Create(pParent,-1,DLG_TITLE,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticText * pHelp = new wxStaticText(this,-1,
												wxGetTranslation( L"This dialog lets you manually specify the alignment of specific lines\n"
																  L"within each file.  This can be useful if you have re-ordered large blocks\n"
																  L"of text to let you see the changes within the blocks.\n"
																  L"\n"
																  L"You may create many such alignments throughout the files, but they must not\n"
																  L"overlap.  Also, a single line cannot be part of more than one alignment."
																  // TODO include note about for best results select EQ lines
																  // TODO add note about being able to click on lines in the files, since we're modeless.
													));
		vSizerTop->Add( pHelp, FIX, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		// put horizontal line and set of ok/cancel buttons across the bottom of the dialog

//		vSizerTop->AddStretchSpacer(VAR);
		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( new wxButton(this,wxID_CANCEL,_("&Close")), FIX, wxALL, M);	// using wxID_CANCEL lets ESC key close dialog
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);

	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);
	Centre(wxBOTH);
}
