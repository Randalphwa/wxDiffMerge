// dlg_open.cpp -- a file/folder open dialog that gives 2/3 pathnames
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

BEGIN_EVENT_TABLE(dlg_open,wxDialog)
	EVT_BUTTON(ID_BROWSE_0,				dlg_open::onEventButton_k)
	EVT_BUTTON(ID_BROWSE_1,				dlg_open::onEventButton_k)
	EVT_BUTTON(ID_BROWSE_2,				dlg_open::onEventButton_k)
	EVT_BUTTON(wxID_OK,					dlg_open::OnOK)
	EVT_TEXT(ID_TEXT_0,					dlg_open::onEventTextChanged_k)
	EVT_TEXT(ID_TEXT_1,					dlg_open::onEventTextChanged_k)
	EVT_TEXT(ID_TEXT_2,					dlg_open::onEventTextChanged_k)
	EVT_BUTTON(ID_SWAP,					dlg_open::onEventButtonSwap)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////
// enable drag-n-drop (well, DROP, that is) for pathnames into the textctrl's.

#if defined(__WXGTK__) || defined(__WXMSW__)
	// i wanted to let each text control handle its own drop
	// functionality but that doesn't work (at least with wxWidgets 2.6.3)
	// i could only get the top-level dialog window to accept DND events.
	// the wxTextCtrl is documented as having a special DropAcceptFiles()
	// that uses the old windows message mechanism (rather than the full
	// blown/general OLE mechanism), but it didn't work either.
	//
	// SO, i make the entire dialog DND aware and hit test for the individual
	// textctrl's.  we let the cursor be "no" everywhere on the dialog except
	// for within the bounds of the textctrl's.
	//
	// we only accept filename drops (like from Windows Explorer or the desktop)
	// we do not accept plain text (like dragging from a text selection) (they
	// should use cut-n-paste for this)

#	define XX_DND_WHOLE_DIALOG 1

#elif defined(__WXMAC__)
	// dnd targets work when assigned directly to textctrl's.  we create one
	// target for each textctrl and let it receive the drop and tell the dlg
	// to update the field using the same validator stuff.

#	define XX_DND_WHOLE_DIALOG 0

#endif

//////////////////////////////////////////////////////////////////

class dlg_open__DnDFile : public wxFileDropTarget
{
public:
#if XX_DND_WHOLE_DIALOG
	dlg_open__DnDFile(dlg_open * pOwner) : m_pOwner(pOwner)
#else
	dlg_open__DnDFile(dlg_open * pOwner, int ndx) : m_pOwner(pOwner), m_ndx(ndx)
#endif
		{
		};

	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString & astrPathnames)
		{
//			wxLogTrace(wxTRACE_Messages,_T("dlg_open__DnDFile:OnDropFiles:"));
//			for (int k=0; k<(int)astrPathnames.GetCount(); k++)
//				wxLogTrace(wxTRACE_Messages,_T("\t[%d]: %s"),k,astrPathnames[k].wc_str());

			if (astrPathnames.GetCount() > 0)
			{
#if XX_DND_WHOLE_DIALOG
				int ndx;

				if (m_pOwner->dnd_hit_test(x,y,&ndx))
					m_pOwner->OnDropFiles(ndx,astrPathnames[0]);
#else
				m_pOwner->OnDropFiles(m_ndx,astrPathnames[0]);
#endif
			}
			
			return true;
		};

	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
		{
//			wxLogTrace(wxTRACE_Messages,_T("dlg_open__DnDFile:OnDragOver: [%d,%d] [def %d]"),x,y,def);

#if XX_DND_WHOLE_DIALOG
			// the user is dragging a filename object over us.
			// hit-test for the 2 or 3 textctrl's and allow the drop.
			// if not over any of them, reject the drop.

			int ndx;

			if (m_pOwner->dnd_hit_test(x,y,&ndx))
				return def;			// mouse over the nth-th textctrl
			else
				return wxDragNone;	// mouse not over any of the textctrl
#else
			// when the individual textctrl's are dnd enabled, we always
			// accept it.
			return def;
#endif
		};

private:
	dlg_open *		m_pOwner;
#if XX_DND_WHOLE_DIALOG
#else
	int				m_ndx;
#endif
};

//////////////////////////////////////////////////////////////////

dlg_open::dlg_open(wxWindow * pParent, ToolBarType tbt, const cl_args * pArgs)
	: m_pParent(pParent),
	  m_tbt(tbt)
{
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
			m_pTextCtrl[0] = new wxTextCtrl(this,ID_TEXT_0,_T(""),wxDefaultPosition,wxSize(X_TEXT,-1),0,wxTextValidator(wxFILTER_NONE,&m_strPath[0]));
			bs0->Add( m_pTextCtrl[0],VAR,wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM,M);

			m_pButtonBrowse[0] = new wxButton(this,ID_BROWSE_0,_("Browse..."));
			bs0->Add( m_pButtonBrowse[0],FIX,wxALIGN_CENTER_VERTICAL|wxALL,M);
		}
		vSizerTop->Add(bs0,FIX,wxGROW|wxALL,M);

		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * bs1 = new wxStaticBoxSizer(new wxStaticBox(this,wxID_ANY,m_strCaption[1]),wxHORIZONTAL);
		{
			m_pTextCtrl[1] = new wxTextCtrl(this,ID_TEXT_1,_T(""),wxDefaultPosition,wxSize(X_TEXT,-1),0,wxTextValidator(wxFILTER_NONE,&m_strPath[1]));
			bs1->Add( m_pTextCtrl[1],VAR,wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM,M);

			m_pButtonBrowse[1] = new wxButton(this,ID_BROWSE_1,_("Browse..."));
			bs1->Add( m_pButtonBrowse[1],FIX,wxALIGN_CENTER_VERTICAL|wxALL,M);
		}
		vSizerTop->Add(bs1,FIX,wxGROW|wxALL,M);

		//////////////////////////////////////////////////////////////////

		if (m_tbt == TBT_MERGE)
		{
			wxStaticBoxSizer * bs2 = new wxStaticBoxSizer(new wxStaticBox(this,wxID_ANY,m_strCaption[2]),wxHORIZONTAL);
			{
				m_pTextCtrl[2] = new wxTextCtrl(this,ID_TEXT_2,_T(""),wxDefaultPosition,wxSize(X_TEXT,-1),0,wxTextValidator(wxFILTER_NONE,&m_strPath[2]));
				bs2->Add( m_pTextCtrl[2],VAR,wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM,M);

				m_pButtonBrowse[2] = new wxButton(this,ID_BROWSE_2,_("Browse..."));
				bs2->Add( m_pButtonBrowse[2],FIX,wxALIGN_CENTER_VERTICAL|wxALL,M);
			}
			vSizerTop->Add(bs2,FIX,wxGROW|wxALL,M);
		}
		else
		{
			m_pTextCtrl[2] = NULL;
			m_pButtonBrowse[2] = NULL;
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

	m_pTextCtrl[ndxPathToFocus]->SetFocus();

	_enable_fields();

#if XX_DND_WHOLE_DIALOG
	// we have one drop target for the whole dialog.  we do our own hit-testing
	// to identify the individual textctrl fields.
	SetDropTarget(new dlg_open__DnDFile(this));
#else
	// we have one drop target for *EACH* textctrl.  they individually take care
	// of the drop and notifying the dialog.
	for (int k=0; k<3; k++)
		if (m_pTextCtrl[k])
			m_pTextCtrl[k]->SetDropTarget(new dlg_open__DnDFile(this,k));
#endif//XX_DND_WHOLE_DIALOG
}

//////////////////////////////////////////////////////////////////

void dlg_open::OnOK(wxCommandEvent & /*e*/)
{
	if (Validate() && TransferDataFromWindow())		// was wxDialog::OnOK()
		EndModal(wxID_OK);
	
#if 0 && defined(DEBUG)
	wxLogTrace(wxTRACE_Messages,_T("dlg_open:OnOK:\n\t[%s]\n\t[%s]\n\t[%s]"),
			   util_printable_s(m_strPath[0]).wc_str(),
			   util_printable_s(m_strPath[1]).wc_str(),
			   util_printable_s(m_strPath[2]).wc_str());
#endif

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

void dlg_open::onEventButton_k(wxCommandEvent & e)
{
	int k = e.GetId() - ID_BROWSE_0;
	
	// raise browser (OpenFile or OpenFolder) seeded with the present value in the
	// text control.  use Validate() and TransferFromWindow() to update our text strings.

	if (Validate())					// used to be wxDialog::OnApply();
		TransferDataFromWindow();	// get stuff from from fields.

	if (m_tbt == TBT_FOLDER)
	{
		// wxWidgets defines "class wxDirDialog" and "::wxDirSelector".
		// the first does all the work.  the second is a convenience
		// wrapper.  but the wrapper doesn't return the dialog result
		// (OK, Cancel, etc) -- so it's of limited use -- we could use
		// it and check for an empty string result, but that's pretty
		// lame....
		//
		// so we do it the 'hard' way.

		// if you give an empty string as the initial directory, wxWidgets
		// varies between platforms in where the dialog starts.  on win32
		// and mac it seems somewhat reasonable -- like the last place you
		// left it or your current directory or something.  on gtk, it
		// always starts at '/' -- very annoying.
		//
		// i'm going to seed the dialog from global props so that
		// it will always come up where we left it (and persist between
		// sessions).

		// WXBUG if the seed directory is mode '000', then wxWidgets gets
		// WXBUG an error (and raises a message box) as it tries to build
		// WXBUG the tree under the seed directory.  this is annoying and
		// WXBUG confusing for the user.
		//
		// TODO we should probably check for access on the seed directory
		// TODO and if not, use the parent directory or something.

		wxFileName fn(m_strPath[k]);
		wxString strDir = fn.GetFullPath();

		wxDirDialog dlg(this,m_strBrowse[k],strDir,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER); // don't want wxDD_NEW_DIR_BUTTON
		if (dlg.ShowModal() != wxID_OK)
			return;
		
		m_strPath[k] = dlg.GetPath();
		TransferDataToWindow();
		_enable_fields();
	}
	else // TBT_DIFF or TBT_MERGE
	{
		wxFileName fn(m_strPath[k]);
		wxString strDir = fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
		wxString strFile = fn.GetFullName();

//#if wxCHECK_VERSION(2,8,0)		// wxOPEN renamed to wxFD_OPEN in wxWidgets 2.8.0
//#else
//#define wxFD_OPEN				wxOPEN
//#define wxFD_FILE_MUST_EXIST	wxFILE_MUST_EXIST
//#endif

		wxFileDialog dlg(this,m_strBrowse[k],strDir,strFile,
						 wxFileSelectorDefaultWildcardStr ,
						 wxFD_OPEN|wxFD_FILE_MUST_EXIST);
		if (dlg.ShowModal() != wxID_OK)
			return;

		m_strPath[k] = dlg.GetPath();
		TransferDataToWindow();
		_enable_fields();
	}
}

//////////////////////////////////////////////////////////////////

void dlg_open::onEventButtonSwap(wxCommandEvent & /*e*/)
{
	// swap[0,1] the contents of the first 2 text fields.
	
	m_bSwapped = !m_bSwapped;		// remember how many times we swapped.

	if (Validate())
		TransferDataFromWindow();	// capture whatever they may have typed.

	wxString strTemp = m_strPath[0];
	m_strPath[0] = m_strPath[1];
	m_strPath[1] = strTemp;

	TransferDataToWindow();			// reload all textctrls
	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void dlg_open::_enable_fields(void)
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
			str = m_pTextCtrl[k]->GetLineText(0);
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
			if (m_pTextCtrl[k])
			{
				str = m_pTextCtrl[k]->GetLineText(0);
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

void dlg_open::onEventTextChanged_k(wxCommandEvent & /*e*/)
{
	_enable_fields();
}

//////////////////////////////////////////////////////////////////

#if XX_DND_WHOLE_DIALOG
bool dlg_open::dnd_hit_test(wxCoord xMouse, wxCoord yMouse, int * pNdx)
{
	// when the dnd is associated with the entire dialog, we need to
	// hit-test over the 2/3 textctrl's to see if the mouse is over
	// one of them.
	//
	// we return true and the index if we are.

	for (int k=0; k<3; k++)
	{
		if (m_pTextCtrl[k])
		{
			int x,y;
			m_pTextCtrl[k]->GetPosition(&x,&y);
			int w,h;
			m_pTextCtrl[k]->GetSize(&w,&h);

			if ((xMouse > x) && (yMouse > y) && (xMouse < x+w) && (yMouse < y+h))
			{
				*pNdx = k;
				return true;
			}
		}
	}

	return false;
}
#endif//XX_DND_WHOLE_DIALOG

void dlg_open::OnDropFiles(int ndx, const wxString & strPathname)
{
	// the user just dropped a filename object over the ndx-th textctrl.
	// the drop needs to do everything just like when the browse... button returns.

	if (m_pTextCtrl[ndx])
	{
		if (Validate())
			TransferDataFromWindow();	// capture whatever they may have typed.
		m_strPath[ndx] = strPathname;	// update string with drop.
		TransferDataToWindow();			// reload all textctrls
		_enable_fields();
	}
}
