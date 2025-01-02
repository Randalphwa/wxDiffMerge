// dlg_auto_merge_result_summary.cpp -- simple modal dialog to show
// the results of auto-merge.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#define DLG_TITLE			_("Auto-Merge Results")

#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
#	define MY_BORDER_STYLE		0
#elif defined(__WXGTK__)
#	define MY_BORDER_STYLE		wxSUNKEN_BORDER
#elif defined(__WXMAC__)
#	define MY_BORDER_STYLE		0
#endif

// WXBUG i'd like to turn on wxLC_HRULES and wxLC_VRULES, but they
// WXBUG cause display dirt above the first row, when scrolling, and
// WXBUG just don't work on the mac.

#if defined(__WXMSW__)
// vrules cause display dirt above line 0.
#	define MY_RULES_STYLE		0
#elif defined(__WXGTK__)
// hrules don't always get drawn when vscrolling.
// vrules look stupid without hrules.
#	define MY_RULES_STYLE		0
#elif defined(__WXMAC__)
// rules just don't work on mac.
#	define MY_RULES_STYLE		0
#endif

//////////////////////////////////////////////////////////////////

dlg_auto_merge_result_summary::dlg_auto_merge_result_summary(wxWindow * pParent, const fim_patchset * pPatchSet)
	: m_pPatchSet(pPatchSet)
{
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pParent,-1,DLG_TITLE,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		vSizerTop->Add( new wxStaticText(this,-1,_("Auto-Merge Performed the Following Actions:")),
						FIX, wxGROW|wxALL, M);

		m_pListCtrl = new wxListCtrl(this,-1,wxDefaultPosition,wxSize(500,150),
									 wxLC_REPORT|wxLC_SINGLE_SEL|MY_BORDER_STYLE|MY_RULES_STYLE);
		_define_columns();
		long nrUnresolvedConflicts = _populate_list();
		vSizerTop->Add( m_pListCtrl, VAR, wxGROW|wxALL, M);

		if (nrUnresolvedConflicts > 0)
		{
			// add a warning box to the dialog explaining that there were
			// unresolved conflicts and that they are responsible for them.

			wxString strErrorFormat = wxGetTranslation(L"There were %ld conflicts that could not be automatically merged.\n"
													   L"These must be manually resolved."
				);
			wxString strError = wxString::Format(strErrorFormat,nrUnresolvedConflicts);
			
			wxStaticText * pStaticText = new wxStaticText(this,wxID_ANY,
														  strError,
														  wxDefaultPosition,wxDefaultSize,
														  wxSIMPLE_BORDER|wxALIGN_CENTER);
			wxFont font(pStaticText->GetFont());
			font.SetWeight(wxFONTWEIGHT_BOLD);
			pStaticText->SetFont(font);
			pStaticText->SetBackgroundColour(wxColour(0xff,0xff,0xd8));
			pStaticText->SetForegroundColour(*wxRED);
			vSizerTop->Add( pStaticText, FIX, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, M);
		}
		
		// put horizontal line and set of ok button across the bottom of the dialog

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( CreateButtonSizer(wxOK), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);

	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);
	Centre(wxBOTH);
}

//////////////////////////////////////////////////////////////////
// NOTE: since we have hidden the task-list for this release, we don't
// NOTE: need the conflicts column.

#define COL_DESCRIPTION			0
#define COL_CHANGES				1
//#define COL_CONFLICTS			2

void dlg_auto_merge_result_summary::_define_columns(void)
{
	wxListItem liCol;

	liCol.SetText(_("Action"));
	liCol.SetAlign(wxLIST_FORMAT_LEFT);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_DESCRIPTION,liCol);

	liCol.SetText(_("Changes"));
	liCol.SetAlign(wxLIST_FORMAT_RIGHT);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_CHANGES,liCol);

//	liCol.SetText(_("Conflicts"));
//	liCol.SetAlign(wxLIST_FORMAT_RIGHT);
//	liCol.SetMask(wxLIST_MASK_TEXT);
//	m_pListCtrl->InsertColumn(COL_CONFLICTS,liCol);

	m_pListCtrl->SetBackgroundColour(*wxWHITE);
	m_pListCtrl->SetTextColour(*wxBLACK);
}

long dlg_auto_merge_result_summary::_populate_list(void)
{
	long nrItems = m_pPatchSet->getNrPatches();
	wxASSERT_MSG( (nrItems > 0), _T("Coding Error") );

	long sums[2][POP__LAST__];
	memset(sums,0,sizeof(sums));

	for (long kItem=0; (kItem<nrItems); kItem++)
	{
		fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kItem);
		fim_patch_op opCurrent = pFimPatch->getPatchOpCurrent();
		fim_patch_op opOriginal = pFimPatch->getPatchOpOriginal();
		int bConflict = (opOriginal == POP_CONFLICT) ? 1 : 0;

		sums[bConflict][ (int)opCurrent-(int)POP__FIRST__ ]++;
	}

	long ndx = 0;

#define SUM(c,p)	(sums[(c)][(int)(p) - (int)POP__FIRST__])
#if 0	// since we have hidden the task-list for this release, we don't need the conflicts column
#define INSERT_ITEM(str,pop)													\
	Statement(	long ndxInList = m_pListCtrl->InsertItem(ndx++,(str));			\
				m_pListCtrl->SetItem(ndxInList,COL_CHANGES,						\
									 wxString::Format(_T("%ld"),SUM(0,pop)));	\
				m_pListCtrl->SetItem(ndxInList,COL_CONFLICTS,					\
									 wxString::Format(_T("%ld"),SUM(1,pop)));	);
#else
#define INSERT_ITEM(str,pop)													\
	Statement(	long ndxInList = m_pListCtrl->InsertItem(ndx++,(str));			\
				m_pListCtrl->SetItem(ndxInList,COL_CHANGES,						\
									 wxString::Format(_T("%ld"),SUM(0,pop)));	);
#endif	

	INSERT_ITEM( _("Deleted from Center"),			POP_DELETE);
	INSERT_ITEM( _("Inserted from Left"),			POP_INSERT_L);
	INSERT_ITEM( _("Replaced Center with Left"),	POP_REPLACE_L);
	INSERT_ITEM( _("Inserted from Right"),			POP_INSERT_R);
	INSERT_ITEM( _("Replaced Center with Right"),	POP_REPLACE_R);
// since we have hidden the task-list, we don't need the this either
//	INSERT_ITEM( _("Completely Ignored"),			POP_IGNORE);

	m_pListCtrl->SetColumnWidth(COL_DESCRIPTION,wxLIST_AUTOSIZE);
	m_pListCtrl->SetColumnWidth(COL_CHANGES,    100);
//	m_pListCtrl->SetColumnWidth(COL_CONFLICTS,  100);

	// return the number of unresolved conflicts

	return SUM(1,POP_CONFLICT);
}
