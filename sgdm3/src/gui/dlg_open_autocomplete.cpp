// dlg_open_autocomplete.cpp -- a file/folder open dialog that gives 2/3 pathnames
// as text fields and associated browse buttons.
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

#define M 7
#define FIX 0
#define VAR 1
#define X_TEXT 500

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(dlg_open_autocomplete,wxDialog)
	EVT_BUTTON(wxID_OK,					dlg_open_autocomplete::OnOK)
	EVT_FILEPICKER_CHANGED(wxID_ANY,	dlg_open_autocomplete::OnFilePickerChanged)
	EVT_DIRPICKER_CHANGED(wxID_ANY,		dlg_open_autocomplete::OnDirPickerChanged)
	EVT_BUTTON(ID_SWAP,					dlg_open_autocomplete::onEventButtonSwap)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

class dlg_open_autocomplete__DnDFile : public wxFileDropTarget
{
public:
	dlg_open_autocomplete__DnDFile(dlg_open_autocomplete * pOwner) : m_pOwner(pOwner)
		{
		};

	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString & astrPathnames)
		{
			//wxLogTrace(wxTRACE_Messages,_T("dlg_open_autocomplete__DnDFile:OnDropFiles:"));
			//for (int k=0; k<(int)astrPathnames.GetCount(); k++)
			//	wxLogTrace(wxTRACE_Messages,_T("\t[%d]: %s"),k,astrPathnames[k].wc_str());

			if (astrPathnames.GetCount() > 0)
			{
				int ndx = m_pOwner->dnd_hit_test(x,y);
				if (ndx >= 0)
					m_pOwner->OnDropFiles(ndx,astrPathnames[0]);
			}
			
			return true;
		};

	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
		{
			// the user is dragging a filename object over us.
			// hit-test for the 2 or 3 textctrl's and allow the drop.
			// if not over any of them, reject the drop.

			int ndx = m_pOwner->dnd_hit_test(x,y);

			//wxLogTrace(wxTRACE_Messages,
			//		   _T("dlg_open_autocomplete__DnDFile:OnDragOver: [%d,%d] [def %d] [hit %d]"),
			//		   x,y, def, ndx);

			if (ndx >= 0)
				return def;			// mouse over the nth-th textctrl
			else
				return wxDragNone;	// mouse not over any of the textctrl
		};

private:
	dlg_open_autocomplete *		m_pOwner;
};

//////////////////////////////////////////////////////////////////

dlg_open_autocomplete::dlg_open_autocomplete(wxWindow * pParent, ToolBarType tbt, const cl_args * pArgs)
	: m_pParent(pParent),
	  m_tbt(tbt)
{
	m_pFilePickerCtrl[0] = NULL;
	m_pFilePickerCtrl[1] = NULL;
	m_pFilePickerCtrl[2] = NULL;
	m_pDirPickerCtrl[0] = NULL;
	m_pDirPickerCtrl[1] = NULL;
	
	int ndxPathToFocus = 0;

	m_bSwapped = false;

	wxString strDlgTitle;

	switch (m_tbt)
	{
	default:	// shouldn't happen
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;

	case TBT_FOLDER:
		strDlgTitle     = _("Select Folders to Compare");
		m_strCaption[0] = _("&Left Folder");
		m_strCaption[1] = _("&Right Folder");
		m_strBrowse[0]  = _("Select Left Folder");
		m_strBrowse[1]  = _("Select Right Folder");
		if (pArgs)
		{
			if (pArgs->nrParams > 0)	{ m_strPath[0] = pArgs->pathname[0]; ndxPathToFocus = 1; }
			if (pArgs->nrParams > 1)	{ m_strPath[1] = pArgs->pathname[1]; ndxPathToFocus = 0; }
		}
		else
		{
			m_strPath[0]    = gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FOLDER_SEED_L);
			m_strPath[1]    = gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FOLDER_SEED_R);
		}
		break;
		
	case TBT_DIFF:
		strDlgTitle   = _("Select Files to Compare");
		m_strCaption[0] = _("&Left File (for Older or Original Version)");
		m_strCaption[1] = _("&Right File (for Newer or Modified Version)");
		m_strBrowse[0]  = _("Select Left File (for Older or Original Version)");
		m_strBrowse[1]  = _("Select Right File (for Newer or Modified Version)");
		if (pArgs)
		{
			if (pArgs->nrParams > 0)	{ m_strPath[0] = pArgs->pathname[0]; ndxPathToFocus = 1; }
			if (pArgs->nrParams > 1)	{ m_strPath[1] = pArgs->pathname[1]; ndxPathToFocus = 0; }
		}
		else
		{
			m_strPath[0]    = gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_DIFF_SEED_0);
			m_strPath[1]    = gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_DIFF_SEED_1);
		}
		break;

	case TBT_MERGE:
		strDlgTitle   = _("Select Files to Merge");
		m_strCaption[0] = _("&Left File (for First Modified or Branched Version)");
		m_strCaption[1] = _("&Center File (for Common Ancestor Version)");
		m_strCaption[2] = _("&Right File (for Second Modified or Branched Version)");
		m_strBrowse[0]  = _("Select Left File (for First Modified or Branched Version)");
		m_strBrowse[1]  = _("Select Center File (for Common Ancestor Version)");
		m_strBrowse[2]  = _("Select Right File (for Second Modified or Branched Version)");
		if (pArgs)
		{
			if (pArgs->nrParams > 0)	{ m_strPath[0] = pArgs->pathname[0]; ndxPathToFocus = 1; }
			if (pArgs->nrParams > 1)	{ m_strPath[1] = pArgs->pathname[1]; ndxPathToFocus = 2; }
			if (pArgs->nrParams > 2)	{ m_strPath[2] = pArgs->pathname[2]; ndxPathToFocus = 0; }
		}
		else
		{
			m_strPath[0]    = gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_MERGE_SEED_0);
			m_strPath[1]    = gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_MERGE_SEED_1);
			m_strPath[2]    = gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_MERGE_SEED_2);
		}
		break;
	}

#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	Create(m_pParent,-1,strDlgTitle,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * bs0 = new wxStaticBoxSizer(new wxStaticBox(this,wxID_ANY,m_strCaption[0]),wxHORIZONTAL);
		{
			if (m_tbt == TBT_FOLDER)
			{
				m_pDirPickerCtrl[0] = new wxDirPickerCtrl(this, ID_PICKER_0,
														  m_strPath[0],
														  m_strBrowse[0],
														  wxDefaultPosition, wxSize(X_TEXT,-1),
														  wxDIRP_USE_TEXTCTRL|wxDIRP_SMALL);
				bs0->Add( m_pDirPickerCtrl[0], VAR, wxALIGN_CENTER_VERTICAL|wxGROW|wxALL, M);
			}
			else
			{
				m_pFilePickerCtrl[0] = new wxFilePickerCtrl(this, ID_PICKER_0,
															m_strPath[0],
															m_strBrowse[0],
															wxFileSelectorDefaultWildcardStr,
															wxDefaultPosition, wxSize(X_TEXT,-1),
															wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
				bs0->Add( m_pFilePickerCtrl[0], VAR, wxALIGN_CENTER_VERTICAL|wxGROW|wxALL, M);
			}
		}
		vSizerTop->Add(bs0,FIX,wxGROW|wxALL,M);

		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * bs1 = new wxStaticBoxSizer(new wxStaticBox(this,wxID_ANY,m_strCaption[1]),wxHORIZONTAL);
		{
			if (m_tbt == TBT_FOLDER)
			{
				m_pDirPickerCtrl[1] = new wxDirPickerCtrl(this, ID_PICKER_1,
														  m_strPath[1],
														  m_strBrowse[1],
														  wxDefaultPosition, wxSize(X_TEXT,-1),
														  wxDIRP_USE_TEXTCTRL|wxDIRP_SMALL);
				bs1->Add( m_pDirPickerCtrl[1], VAR, wxALIGN_CENTER_VERTICAL|wxGROW|wxALL, M);
			}
			else
			{
				m_pFilePickerCtrl[1] = new wxFilePickerCtrl(this, ID_PICKER_1,
															m_strPath[1],
															m_strBrowse[1],
															wxFileSelectorDefaultWildcardStr,
															wxDefaultPosition, wxSize(X_TEXT,-1),
															wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
				bs1->Add( m_pFilePickerCtrl[1], VAR, wxALIGN_CENTER_VERTICAL|wxGROW|wxALL, M);
			}
		}
		vSizerTop->Add(bs1,FIX,wxGROW|wxALL,M);

		//////////////////////////////////////////////////////////////////

		if (m_tbt == TBT_MERGE)
		{
			wxStaticBoxSizer * bs2 = new wxStaticBoxSizer(new wxStaticBox(this,wxID_ANY,m_strCaption[2]),wxHORIZONTAL);
			{
				m_pFilePickerCtrl[2] = new wxFilePickerCtrl(this, ID_PICKER_2,
															m_strPath[2],
															m_strBrowse[2],
															wxFileSelectorDefaultWildcardStr,
															wxDefaultPosition, wxSize(X_TEXT,-1),
															wxFLP_USE_TEXTCTRL|wxFLP_SMALL);
				bs2->Add( m_pFilePickerCtrl[2], VAR, wxALIGN_CENTER_VERTICAL|wxGROW|wxALL, M);
			}
			vSizerTop->Add(bs2,FIX,wxGROW|wxALL,M);
		}
		else
		{
			m_pFilePickerCtrl[2] = NULL;
		}

		//////////////////////////////////////////////////////////////////

		// put horizontal line and close buttons across the bottom of the dialog

		vSizerTop->AddStretchSpacer(VAR);
		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL),
						FIX, wxGROW|wxTOP|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			if ((m_tbt == TBT_FOLDER) || (m_tbt == TBT_DIFF))
			{
				m_pButtonSwap = new wxButton(this,ID_SWAP,_("Swap"));
				hSizerButtons->Add( m_pButtonSwap,FIX,wxALIGN_CENTER_VERTICAL,0);
			}

			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( CreateButtonSizer(wxOK | wxCANCEL), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);
	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);
	Centre(wxBOTH);

	_enable_fields();

	SetDropTarget(new dlg_open_autocomplete__DnDFile(this));
}

//////////////////////////////////////////////////////////////////

void dlg_open_autocomplete::OnDirPickerChanged(wxFileDirPickerEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("OnDirPickerChanged"));

	_enable_fields();
}

void dlg_open_autocomplete::OnFilePickerChanged(wxFileDirPickerEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("OnFilePickerChanged"));

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

wxString dlg_open_autocomplete::_fetch_k(int k)
{
	if (m_tbt == TBT_FOLDER)
		return m_pDirPickerCtrl[k]->GetPath();
	else
		return m_pFilePickerCtrl[k]->GetPath();
}

void dlg_open_autocomplete::_set_k(int k, const wxString & str)
{
	if (m_tbt == TBT_FOLDER)
		m_pDirPickerCtrl[k]->SetPath(str);
	else
		m_pFilePickerCtrl[k]->SetPath(str);
}

void dlg_open_autocomplete::OnOK(wxCommandEvent & /*e*/)
{
	m_strPath[0] = _fetch_k(0);
	m_strPath[1] = _fetch_k(1);
	if (m_tbt == TBT_MERGE)
		m_strPath[2] = _fetch_k(2);
		
	EndModal(wxID_OK);
	
	//wxLogTrace(wxTRACE_Messages,_T("dlg_open_autocomplete:OnOK:\n\t[%s]\n\t[%s]\n\t[%s]"),
	//		   util_printable_s(m_strPath[0]).wc_str(),
	//		   util_printable_s(m_strPath[1]).wc_str(),
	//		   util_printable_s(m_strPath[2]).wc_str());

	switch (m_tbt)
	{
	default:	// shouldn't happen
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;

	case TBT_FOLDER:
		gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FOLDER_SEED_L,m_strPath[0]);
		gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FOLDER_SEED_R,m_strPath[1]);
		break;
		
	case TBT_DIFF:
		gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_DIFF_SEED_0,m_strPath[0]);
		gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_DIFF_SEED_1,m_strPath[1]);
		break;

	case TBT_MERGE:
		gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_MERGE_SEED_0,m_strPath[0]);
		gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_MERGE_SEED_1,m_strPath[1]);
		gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FILE_MERGE_SEED_2,m_strPath[2]);
		break;
	}
}

//////////////////////////////////////////////////////////////////

void dlg_open_autocomplete::onEventButtonSwap(wxCommandEvent & /*e*/)
{
	// swap[0,1] the contents of the first 2 text fields.
	
	m_strPath[0] = _fetch_k(0);
	m_strPath[1] = _fetch_k(1);

	m_bSwapped = !m_bSwapped;		// remember how many times we swapped.

	_set_k(0, m_strPath[1]);
	_set_k(1, m_strPath[0]);

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void dlg_open_autocomplete::_enable_fields(void)
{
	int k;
	wxString str;

	switch (m_tbt)
	{
	default:
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;

	case TBT_FOLDER:
		for (k=0; k<2; k++)
		{
			str = m_pDirPickerCtrl[k]->GetPath();
			if (str.Length() == 0)
				goto disable;
			if (!wxFileName::DirExists(str))
				goto disable;
		}
		goto enable;

	case TBT_DIFF:
	case TBT_MERGE:
		for (k=0; k<3; k++)
		{
			if (m_pFilePickerCtrl[k])
			{
				str = m_pFilePickerCtrl[k]->GetPath();
				if (str.Length() == 0)
					goto disable;
				if (!wxFileName::FileExists(str))
					goto disable;
#if defined(__WXMSW__)
				bool bIsLnk = false;
				(void)util_file__is_shell_lnk(str, &bIsLnk);
				if (bIsLnk)
					goto disable;
#endif
			}
		}
		goto enable;
	}

enable:
	FindWindow(wxID_OK)->Enable( true );
	return;

disable:
	FindWindow(wxID_OK)->Enable( false );
	return;
}

//////////////////////////////////////////////////////////////////

int dlg_open_autocomplete::dnd_hit_test(wxCoord xMouse, wxCoord yMouse)
{
	// when the dnd is associated with the entire dialog, we need to
	// hit-test over the 2/3 textctrl's to see if the mouse is over
	// one of them.
	//
	// we return true and the index if we are.

	wxCoord xMouseScreen = xMouse;
	wxCoord yMouseScreen = yMouse;
	this->ClientToScreen(&xMouseScreen, &yMouseScreen);

	for (int k=0; k<3; k++)
	{
		wxTextCtrl * pTextCtrl_k = NULL;
		if (m_pFilePickerCtrl[k])
			pTextCtrl_k = m_pFilePickerCtrl[k]->GetTextCtrl();
		else if ((k < 2) && (m_pDirPickerCtrl[k]))
			pTextCtrl_k = m_pDirPickerCtrl[k]->GetTextCtrl();
		
		if (pTextCtrl_k)
		{
			wxCoord xMouseRelative = xMouseScreen;
			wxCoord yMouseRelative = yMouseScreen;
			pTextCtrl_k->ScreenToClient(&xMouseRelative, &yMouseRelative);

			int x,y;
			pTextCtrl_k->GetPosition(&x,&y);
			int w,h;
			pTextCtrl_k->GetSize(&w,&h);

			if ((xMouseRelative > x) && (yMouseRelative > y) && (xMouseRelative < x+w) && (yMouseRelative < y+h))
				return k;
		}
	}

	return -1;
}

void dlg_open_autocomplete::OnDropFiles(int ndx, const wxString & strPathname)
{
	// the user just dropped a filename object over the ndx-th textctrl.
	// the drop needs to do everything just like when the browse... button returns.

	//wxLogTrace(wxTRACE_Messages,_T("dlg_open_autocomplete__DnDFile:OnDropFiles: [ndx %d]"), ndx);

	if (m_pFilePickerCtrl[ndx])
		m_pFilePickerCtrl[ndx]->SetPath(strPathname);
	else if ((ndx < 2) && (m_pDirPickerCtrl[ndx]))
		m_pDirPickerCtrl[ndx]->SetPath(strPathname);

	_enable_fields();
}
