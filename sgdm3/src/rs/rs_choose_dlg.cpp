// rs_choose_dlg.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

#define RS_CHOOSE_DLG_RULESET_TITLE		_("Choose Ruleset")
#define RS_CHOOSE_DLG_RULESET_MESSAGE	_("Available Rulesets")
#define RS_CHOOSE_DLG_RULESET_DEFAULT	_("Use Default Ruleset")
#define RS_CHOOSE_DLG_CHARSET_TITLE		_("Choose Character Encoding")
#define RS_CHOOSE_DLG_CHARSET_TITLE_s	_("Choose Character Encoding for %s")
#define RS_CHOOSE_DLG_CHARSET_MESSAGE	_("Available Character Encodings")
#define RS_CHOOSE_DLG_CHARSET_DEFAULT	_("Use System Default")

#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(rs_choose_dlg,wxDialog)
	EVT_LISTBOX_DCLICK(ID_LISTBOX, rs_choose_dlg::onListBoxDClickEvent_listbox)
	//EVT_LISTBOX(ID_LISTBOX, rs_choose_dlg::onListBoxEvent_listbox)
	EVT_BUTTON(ID_USE_DEFAULT,rs_choose_dlg::onButtonEvent_use_default)
	EVT_BUTTON(wxID_OK,       rs_choose_dlg::OnOK)
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////

void rs_choose_dlg::_preload_preview_buffers(int nrPoi, poi_item * pPoiTable[])
{
#define BUF_LIMIT		1000						// fetch at most 1000 bytes

	// fetch the first BUF_LIMIT chars from each file and
	// insert into the corresponding string buffer.  since
	// we don't know what character encoding to use, convert
	// any non-ascii chars into hex.

	for (int kPoi=0; kPoi<nrPoi; kPoi++)
	{
		util_logToString uLog(&m_uePreview[kPoi].refExtraInfo());

		wxFile file(pPoiTable[kPoi]->getFullPath(),wxFile::read);
		if (!file.IsOpened())
		{
			m_uePreview[kPoi].set(util_error::UE_CANNOT_OPEN_FILE);
			m_strPreviewBuffer[kPoi] = pPoiTable[kPoi]->getFullPath() + _T("\r\n\r\n") + m_uePreview[kPoi].getMBMessage();
		}
		else
		{
			off_t lenFile = file.Length();
			if (lenFile == 0)
			{
				m_strPreviewBuffer[kPoi] = _T("");
			}
			else
			{
				lenFile = MyMin(lenFile, BUF_LIMIT);
				byte * rawBuffer = (byte *)calloc((BUF_LIMIT+1),sizeof(byte));

				if (file.Read(rawBuffer,lenFile) == -1)
				{
					m_uePreview[kPoi].set(util_error::UE_CANNOT_READ_FILE);
					m_strPreviewBuffer[kPoi] = pPoiTable[kPoi]->getFullPath() + _T("\r\n\r\n") + m_uePreview[kPoi].getMBMessage();
				}
				else
				{
					for (int kByte=0; kByte<lenFile; kByte++)
					{
						if ((rawBuffer[kByte] == 0x09) || (rawBuffer[kByte] == 0x0a) || (rawBuffer[kByte] == 0x0d))
							m_strPreviewBuffer[kPoi] += (wxChar)(rawBuffer[kByte]);
						else if ((rawBuffer[kByte] >= 0x20) && (rawBuffer[kByte] < 0x7f))
							m_strPreviewBuffer[kPoi] += (wxChar)(rawBuffer[kByte]);
						else
							m_strPreviewBuffer[kPoi] += wxString::Format(_T("\\x%02x"),((unsigned int)rawBuffer[kByte]));
					}
				}
				free(rawBuffer);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////

void rs_choose_dlg::_init(wxWindow * pParent,
						  wxString strTitle, wxString strMessage, wxString strDefault,
						  int nrPoi, poi_item * pPoiTable[],
						  int nrChoices, wxString aChoices[],
						  int kPreSelect)
{
	m_pParent = pParent;
	m_nrPoi = nrPoi;
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pParent,-1, strTitle, wxDefaultPosition, wxDefaultSize,
		   wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxSizer1 = new wxStaticBoxSizer( new wxStaticBox(this,-1,strMessage), wxVERTICAL);
		{
			m_pListBox = new wxListBox(this,ID_LISTBOX,
									   wxDefaultPosition,wxSize(400,200),
									   nrChoices,aChoices,wxLB_SINGLE|wxLB_ALWAYS_SB);
			staticBoxSizer1->Add(m_pListBox, VAR, wxGROW|wxALL, M);
		}
		vSizerTop->Add(staticBoxSizer1, VAR, wxGROW|wxALL, M);

		if (nrPoi > 0)
		{
			wxStaticBoxSizer * staticBoxSizer2 = new wxStaticBoxSizer( new wxStaticBox(this,-1,_("File Previews")), wxVERTICAL);
			{
				m_pNotebook = new wxNotebook(this,ID_NOTEBOOK,wxDefaultPosition,wxDefaultSize);
				{
					for (int kPoi=0; kPoi<nrPoi; kPoi++)
					{
						m_pPanel[kPoi] = new wxPanel(m_pNotebook,(ID_PANEL_K0+kPoi));
						{
							wxBoxSizer * vSizerPanelTop = new wxBoxSizer(wxVERTICAL);
							{
								m_pTextPreview[kPoi] = new wxTextCtrl(m_pPanel[kPoi],(ID_PREVIEW_K0+kPoi),m_strPreviewBuffer[kPoi],
																	  wxDefaultPosition,wxSize(400,100),
																	  wxTE_MULTILINE|wxTE_READONLY/*|wxHSCROLL|wxTE_DONTWRAP*/);
								vSizerPanelTop->Add( m_pTextPreview[kPoi], FIX, wxGROW|wxALL, M);
							}
							m_pPanel[kPoi]->SetSizer(vSizerPanelTop);
						}
						m_pNotebook->AddPage(m_pPanel[kPoi],pPoiTable[kPoi]->getFileName().GetFullName());
					}
				}
				staticBoxSizer2->Add(m_pNotebook,FIX, wxGROW|wxALL, M);
			}
			vSizerTop->Add(staticBoxSizer2, FIX, wxGROW|wxALL, M);
		}
		
		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);

		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->Add( new wxButton(this,ID_USE_DEFAULT,strDefault), FIX, wxALIGN_CENTER_VERTICAL|wxRIGHT, M);
			hSizerButtons->AddStretchSpacer(VAR);

//			hSizerButtons->Add( new wxButton(this,wxID_CANCEL,_("Cancel")), FIX, wxLEFT|wxRIGHT, M);
//			hSizerButtons->Add( new wxButton(this,wxID_OK,    _("OK")    ), FIX, wxLEFT|wxRIGHT, M);
			hSizerButtons->Add( CreateButtonSizer(wxOK | wxCANCEL), FIX, 0, 0);

		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);
	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);

	// if they told us to pre-select something, do it.
	// if -1, preselect the first item in the list.
	// if they gave us -2, don't pre-select anything.

	if (kPreSelect >= 0)
		m_pListBox->SetSelection(kPreSelect);
	else if (kPreSelect == -1)
		m_pListBox->SetSelection(0);

	// center dialog on parent window rather than arbitrary location
	// (screen-centered on Win32 & GTK, upper-left corner of window in MAC).
	// WXBUG: this doesn't appear to work on WIN32 -- oh well.

	Centre(wxBOTH);
}

//////////////////////////////////////////////////////////////////

void rs_choose_dlg::onListBoxDClickEvent_listbox(wxCommandEvent & /*e*/)
{
	// fake an OK on the selected item.

	EndModal(wxID_OK);
}

#if 0
void rs_choose_dlg::onListBoxEvent_listbox(wxCommandEvent & e)
{
	//////////////////////////////////////////////////////////////////
	// TODO I'd like to be able to disable the OK button when nothing
	// TODO is selected in the listbox, but it doesn't seem possible.
	// TODO The EVT_LISTBOX() event gets called when an item is selected,
	// TODO but nothing gets called when an item is deselected.
	// TODO
	// TODO UPDATE -- it turns out that on Win32 and MAC we always have
	// TODO UPDATE -- something selected.  it's only on Linux that the
	// TODO UPDATE -- can deselect the current item and have nothing
	// TODO UPDATE -- selected.
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// TODO When we are displaying the character encodings, we should
	// TODO when the selection changes, redisplay the file previews
	// TODO with the buffers converted using the now selected encoding.
	// TODO Or show the raw buffers in one page and the conversions
	// TODO in another page.
	//////////////////////////////////////////////////////////////////

	wxLogTrace(wxTRACE_Messages, _T("ListboxEvent: [IsSelection %d][GetSelection %d]"), e.IsSelection(), e.GetSelection());
}
#endif

void rs_choose_dlg::onButtonEvent_use_default(wxCommandEvent & /*e*/)
{
	EndModal(ID_USE_DEFAULT);
}

//////////////////////////////////////////////////////////////////

void rs_choose_dlg::OnOK(wxCommandEvent & /*e*/)
{
	// ensure that something is selected in the list box before we let the OK go thru.
	// i'd rather just disable the OK button, but we don't get the right events from
	// the listbox.

	int nr = m_pListBox->GetSelection();
	if (nr != wxNOT_FOUND)
	{
		if (Validate() && TransferDataFromWindow())
			EndModal(wxID_OK);
	}
	else
	{
		wxMessageDialog dlg(this,_("Please make a selection."),_("Error"),wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}
	
	return;
}

//////////////////////////////////////////////////////////////////

int rs_choose_dlg::run(void)
{
	// before we actually raise the dialog that we created,
	// see if we ran into filesystem errors loading the file
	// previews.  if so, we shouldn't bother with our dialog
	// because after the user chooses a ruleset or encoding
	// the program will then fail when it tries to load the
	// piecetables with the documents (a read error is a read
	// error).  so we give them an error messagebox now and
	// save them some clicking.
	//
	// return >=0 when user picks something from the listbox
	// return -1  when user picks use-default
	// return -2  when user picks CANCEL
	// return -3  on error

	for (int kPoi=0; kPoi < m_nrPoi; kPoi++)
	{
		if (m_uePreview[kPoi].isErr())
		{
			wxMessageDialog dlg(m_pParent,m_uePreview[kPoi].getMBMessage(),_("Error Loading File Preview"), wxOK|wxICON_ERROR);
			dlg.ShowModal();
			return -3;
		}
	}

	int result = ShowModal();

	if (result == ID_USE_DEFAULT)
		return -1;

	if (result == wxID_CANCEL)
		return -2;

	if (result == wxID_OK)
	{
		int nr = m_pListBox->GetSelection();
		if (nr != wxNOT_FOUND)
			return nr;				// normal result
		
		return -3;					// should not happen, error if it does
	}

	return -3;	// should not happen, error if it does
}

//////////////////////////////////////////////////////////////////

void rs_choose_dlg__ruleset::_init(wxWindow * pParent, int kPreSelect, int nrPoi, poi_item * pPoiTable[], const rs_ruleset_table * pRST)
{
	wxString * array;
	int cRulesets = pRST->allocateArrayOfNames(&array);
	wxASSERT_MSG( (cRulesets > 0), _T("Coding Error!") );

	_preload_preview_buffers(nrPoi,pPoiTable);

	rs_choose_dlg::_init(pParent,
						 RS_CHOOSE_DLG_RULESET_TITLE,RS_CHOOSE_DLG_RULESET_MESSAGE,RS_CHOOSE_DLG_RULESET_DEFAULT,
						 nrPoi,pPoiTable,
						 cRulesets,array,
						 kPreSelect);

	pRST->freeArrayOfNames(array);
}

rs_choose_dlg__ruleset::rs_choose_dlg__ruleset(wxWindow * pParent, int kPreSelect, int nrPoi, poi_item * pPoiTable[], const rs_ruleset_table * pRST)
{
	if (!pRST)
		pRST = gpRsRuleSetTable;
	
	_init(pParent,kPreSelect,nrPoi,pPoiTable,pRST);
}

rs_choose_dlg__ruleset::rs_choose_dlg__ruleset(wxWindow * pParent, const rs_ruleset * pRSCurrent, const rs_ruleset_table * pRST)
{
	if (!pRST)
		pRST = gpRsRuleSetTable;
	
	int index = pRST->getIndex(pRSCurrent);

	if (index == -1)		// if using the default ruleset,
		index = -2;			//   don't preselect anything.

	_init(pParent,index,0,NULL,pRST);
}

//////////////////////////////////////////////////////////////////

rs_choose_dlg__charset::rs_choose_dlg__charset(wxWindow * pParent, util_encoding encPreSelect, int nrPoi, poi_item * pPoiTable[])
{
	// present a simple modal dialog that lists all of the
	// character encodings supported on this system and let
	// the user pick one.

	int kPreSelect = m_encTable.findEnc(encPreSelect);

	_preload_preview_buffers(nrPoi,pPoiTable);

	wxString strTitle;
	if (nrPoi == 1)
		strTitle = wxString::Format(RS_CHOOSE_DLG_CHARSET_TITLE_s,pPoiTable[0]->getFileName().GetFullName().wc_str());
	else
		strTitle = RS_CHOOSE_DLG_CHARSET_TITLE;
	
	_init(pParent,
		  strTitle,RS_CHOOSE_DLG_CHARSET_MESSAGE,RS_CHOOSE_DLG_CHARSET_DEFAULT,
		  nrPoi,pPoiTable,
		  m_encTable.getCount(),m_encTable.getArrayNames(),
		  kPreSelect);
}

bool rs_choose_dlg__charset::run(util_encoding * pEnc)
{
	int result = rs_choose_dlg::run();

	if (result < -1)	// user cancel or error
		return false;

	if (result == -1)	// use default
	{
		*pEnc = (util_encoding)wxFONTENCODING_DEFAULT;
		return true;
	}
	
	wxFontEncoding fe = m_encTable.lookupEnc(result);

	wxLogTrace(wxTRACE_Messages, _T("ChooseCharacterEncoding: Selected: [result %d][fe %d][%s][%s]"),
			   result,(int)fe,
			   wxFontMapper::GetEncodingName(fe).wc_str(),
			   wxFontMapper::GetEncodingDescription(fe).wc_str());
	
	*pEnc = (util_encoding)fe;
	return true;
}
