// SupportDlg.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <rs.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <xt.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#define DLG_TITLE			_("Support Information")

#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(SupportDlg,wxDialog)
	EVT_BUTTON(ID_BUTTON_COPY_ALL,		SupportDlg::onEventButton_CopyAll)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

SupportDlg::SupportDlg(wxWindow * pWindowParent,
					   gui_frame * pGuiFrameActive)
	: m_pWindowParent(pWindowParent),
	  m_pGuiFrameActive(pGuiFrameActive)
{
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxString strMessage = _createMessage();

	Create(pWindowParent,-1,DLG_TITLE,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		m_pTextCtrl = new wxTextCtrl(this,ID_TEXT,strMessage,
									 wxDefaultPosition,wxSize(800,600),
									 wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxTE_RICH2|wxTE_DONTWRAP);
		wxFont font(10,wxFONTFAMILY_MODERN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T(""),wxFONTENCODING_SYSTEM);
		m_pTextCtrl->SetFont(font);
		vSizerTop->Add(m_pTextCtrl, VAR, wxGROW|wxALL, M);
		
		// put horizontal line and set of ok button across the bottom of the dialog

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->Add( new wxButton(this,ID_BUTTON_COPY_ALL,_("&Copy All")), FIX, wxALIGN_CENTER_VERTICAL, 0);
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( CreateButtonSizer(wxOK), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);

	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);
	Centre(wxBOTH);

	m_pTextCtrl->SetInsertionPoint(0);
	m_pTextCtrl->ShowPosition(0);

}

//////////////////////////////////////////////////////////////////

void SupportDlg::onEventButton_CopyAll(wxCommandEvent & /*e*/)
{
	// get entire contents of control -- will have \n on all platforms.

	wxString strBuf = m_pTextCtrl->GetValue();

	// see wxBUG notes in ViewFilePanel__selection.cpp:copyToClipboard()

	if (wxTheClipboard->Open())
	{
		wxTextDataObject * pTDO = new wxTextDataObject(strBuf);

		wxTheClipboard->SetData(pTDO);
		wxTheClipboard->Close();
	}
}

//////////////////////////////////////////////////////////////////

wxString SupportDlg::_createMessage(void)
{
	wxString strMessage;

	//////////////////////////////////////////////////////////////////
	// Get Program version info
	//////////////////////////////////////////////////////////////////

	wxString strAppTitle = VER_APP_TITLE;
	wxString strBuildVersion = AboutBox::build_version_string();

#if defined(__WXMSW__)
	wxString strPlatform = _T("WXMSW");
#elif defined(__WXGTK__)
	wxString strPlatform = _T("WXGTK");
#elif defined(__WXMAC__)
	wxString strPlatform = _T("WXMAC");
#endif

#ifndef wxVERSION_NUM_DOT_STRING_T	// a 2.8.0 feature -- we do it the hard way for 2.6.x -- build L"2.6.x"
#  define _s_(n)		#n
#  if defined(__WXMSW__) || defined(__WXGTK__)
#	 define _s(n)		_s_( n )
#	 define _st_(n)		_T( _s(n) )
#	 define _st(n)		_st_( n )
#	 define _m(x,y,z)	_st(x) _T(".") _st(y) _T(".") _st(z)
#  else	// __WXMAC__
#    define _m(x,y,z)	_s_(x) L"." _s_(y) L"." _s_(z)
#  endif
#  define wxVERSION_NUM_DOT_STRING_T	_m(wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER)
#endif

	wxString strWxWidgetsVersion = wxVERSION_NUM_DOT_STRING_T;

	strMessage += wxString::Format(_T("%s : %s : %s {%s}\n\n"),
								   strAppTitle.wc_str(),strBuildVersion.wc_str(),
								   strPlatform.wc_str(), strWxWidgetsVersion.wc_str());

	//////////////////////////////////////////////////////////////////
	// Add new style B, P, V fields from the new-revision-check code.

	strMessage += wxString::Format(_T("[B %s][P %s][V %s]\n\n"),
								   util_my_branch(),
								   util_my_package(),
								   util_my_version());

	//////////////////////////////////////////////////////////////////
	
	strMessage += wxString::Format(_T("Machine: ll=%ld l=%ld i=%ld v*=%ld s=%ld c=%ld\n\n"),
								   (long)sizeof(long long),
								   (long)sizeof(long),
								   (long)sizeof(int),
								   (long)sizeof(void*),
								   (long)sizeof(size_t),
								   (long)sizeof(wxChar));

	//////////////////////////////////////////////////////////////////
	// list compiled features
	//////////////////////////////////////////////////////////////////

	strMessage += _T("Compiled features:\n");
	strMessage += COMPILED_FEATURE_LIST;
	strMessage += _T("\n");

	//////////////////////////////////////////////////////////////////
	// get command line arguments
	//////////////////////////////////////////////////////////////////

	strMessage += wxGetApp().formatArgvForSupportMessage();

	//////////////////////////////////////////////////////////////////
	// get toolbar/menu settings for the frame that they launched us from.
	//////////////////////////////////////////////////////////////////

	if (m_pGuiFrameActive)
	{
		strMessage += _T("Active Window Properties:\n");
		strMessage += m_pGuiFrameActive->dumpSupportInfo( _T("\t") );
	}

	//////////////////////////////////////////////////////////////////
	// current rulesets
	//////////////////////////////////////////////////////////////////

	strMessage += gpRsRuleSetTable->dumpSupportInfoRST();
	
	//////////////////////////////////////////////////////////////////
	// current external tool configuration
	//////////////////////////////////////////////////////////////////

	strMessage += gpXtToolTable->dumpSupportInfo();

	//////////////////////////////////////////////////////////////////
	// get the value of all global props
	//////////////////////////////////////////////////////////////////

	strMessage += gpGlobalProps->dumpSupportInfo();

	//////////////////////////////////////////////////////////////////
	// for completeness, dump all the frame windows (so we can see if
	// there are any file sharing quirks) (this will duplicate the info
	// for the active window, but i don't care)
	//////////////////////////////////////////////////////////////////

	strMessage += gpFrameFactory->dumpSupportInfoFF();

	//////////////////////////////////////////////////////////////////

	return strMessage;
}
