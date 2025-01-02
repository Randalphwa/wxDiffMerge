// fs_fs.cpp -- a "file set" -- a set of files -- the set of documents
// in a 2-way "file diff" or a 3-way "file merge".
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <rs.h>
#include <poi.h>
#include <fs.h>

//////////////////////////////////////////////////////////////////

fs_fs::fs_fs(const cl_args * pArgs)
	: m_bLoaded(false),
	  m_bReadOnly(false),
	  m_nrTops(0),
	  m_pRuleSet(NULL),
	  m_pPoiItemEdit(NULL),
	  m_pPTableEdit(NULL),
	  m_pPoiResult(NULL)
{
	for (int kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
	{
		m_pPoiItem[kPanel] = NULL;
		m_pPTable[kPanel]  = NULL;
	}

	if (pArgs)
	{
		// remember value of /result:<pathname> given on command-line (if we are the first window)
		
		if (pArgs->bResult)
		{
			m_pPoiResult = gpPoiItemTable->addItem(pArgs->result);
//			wxLogTrace(TRACE_FS_DUMP, _T("fs_fs::fs_fs: [ResultPathname %s]"),
//					   util_printable_s(m_pPoiResult->getFullPath()).wc_str());
		}

		// remember valud of /ro (read-only) flag if given on command-line (if we are the first window)
		//
		// TODO if read-only, we should probably mark the piece-table as read-only in case they later
		// TODO open another window that shares the same T1 file.

		m_bReadOnly = pArgs->bReadOnly;
	}
	
}

fs_fs::~fs_fs(void)
{
	//wxLogTrace(TRACE_FS_DUMP, _T("fs_fs::~fs_fs: [%p] ptables [%p][%p][%p] edit [%p]"),
	//		   this,m_pPTable[PANEL_T0],m_pPTable[PANEL_T1],m_pPTable[PANEL_T2],m_pPTableEdit);

	if (m_pPTable[PANEL_T0])	gpPTableTable->unRef(m_pPTable[PANEL_T0]);
	if (m_pPTable[PANEL_T2])	gpPTableTable->unRef(m_pPTable[PANEL_T2]);

	// if we have an edit-buffer in this window and we hold the last
	// reference to it, we want to break the binding between it and
	// the piecetable in T1.  [T1 may be open in another window and
	// not currently being edited.]

	if (m_pPTableEdit && gpPTableTable->unRef(m_pPTableEdit))
	{
		m_pPoiItemEdit->setPoiEditSrc(NULL);
		m_pPoiItem[PANEL_T1]->setPoiEditBuffer(NULL);
	}

	if (m_pPTable[PANEL_T1])	gpPTableTable->unRef(m_pPTable[PANEL_T1]);

	
	// TODO consider removing cb to RuleSet, if set.
}

//////////////////////////////////////////////////////////////////

void fs_fs::setFiles(poi_item * pPoi0, poi_item * pPoi1, poi_item * pPoi2)
{
	// associate our file set with the POI's given.

	wxASSERT_MSG( (m_bLoaded==false), _T("Coding Error: already loaded") );
	wxASSERT_MSG( (pPoi0 && pPoi1), _T("Coding Error: fs_fs::setFiles: null paths -- require at least 2 paths"));

	m_pPoiItem[PANEL_T0] = pPoi0;
	m_pPoiItem[PANEL_T1] = pPoi1;
	m_pPoiItem[PANEL_T2] = pPoi2;
	
	m_nrTops = ((pPoi2) ? 3 : 2);

	// edit panel, if present, should get cloned from one of these afterwards
	// or explicitly set from another fs_fs.
	
	// caller should call loadFiles() to get piecetables actually loaded.

	m_bLoaded = false;
}

//////////////////////////////////////////////////////////////////

static void _fancy_error_dialog(wxWindow * pParent,
								int /*nrTops*/, int kPanel,
								const wxString & strPathname,
								const util_error & ue)
{
	// instead of a simple MessageBox() with
	// "cannot import file using given character encoding.\n\n<encoding name>"
	// put up a more friendly error message.  bug:12105.

	wxString strMsg = wxString::Format( (L"File %d could not be imported.\n"
										 L"\n"
										 L"%s\n"
										 L"%s\n"
										 L"\n"
										 L"%s"),
										(kPanel+1),
										ue.getMessage().wc_str(),
										ue.getExtraInfo().wc_str(),
										strPathname.wc_str());

	wxMessageDialog dlg(pParent,strMsg,_("Error!"), wxOK|wxICON_ERROR);
	dlg.ShowModal();
}

//////////////////////////////////////////////////////////////////
// the purpose of _bom_and_nul_byte_in_file_error_dialog() and
// _nul_byte_in_file_error_dialog() is to detect when the user
// has loaded a binary file (such as a spreadsheet or a word doc)
// and prevent us from trying to display it in our text windows.
// (and to prevent wxWidgets library from artificially truncating
// them on import.)
//
// we have 2 error routines to handle this because we have 2 places
// where we test for the nul bytes during loadFiles.  in one case,
// we have sniffed the file and found a unicode BOM; in the other
// case, there is no BOM.
//
// we handle these differently since the error message needs to be
// a little different -- and the recovery is different.
//
// [] when there is a BOM, the lower layer actually scanned for
//    invalid zero code point.  (utf16 and utf32 actually can have
//    lots of NUL bytes (the ascii chars for example) and these will
//    be correctly parsed/processed by the conversion routine; rogue
//    NUL's will be caught by us.  if a rogue NUL is found in a file
//    with a BOM, we don't really want to ask them to select another
//    encoding.  so we fail.
//
// [] when there is *NO* BOM, the lower layer actually scanned for
//    a NUL byte (0x00).  this is our only real indicator that the
//    file is binary.  In this case, we could have a true binary
//    file ***OR*** we could have a utf16 or utf32 file (without BOM).
//    we want to give the user a chance to change encodings.
//
// In either case, as a convenience, we scan the files and do a
// memcmp() and see if they are identical or different and report
// that.  This should help the user, who may knowingly clicked on
// binary files, but only wants to know if they are different.

void fs_fs::_do_raw_comparison(wxString & strResultMessage) const
{
	// compare T1 and T0 (using memcmp()).
	// if T1 and T0 are identical (and we are a merge window), compare T1 and T2.
	// we eat any errors that we get doing the comparison.
	//
	// return a nicely formatted string describing the results.

	int status10, status12, status02;
	util_error errCmp10, errCmp12, errCmp02;

#define OP(s)		((s==1) ? _T("==") : ((s==0) ? _T("!=") : _T("??")))

	strResultMessage.Empty();

	if (m_nrTops == 3)
	{
		errCmp10 = m_pPoiItem[PANEL_T1]->compareFileExact(m_pPoiItem[PANEL_T0],&status10);
		errCmp12 = m_pPoiItem[PANEL_T1]->compareFileExact(m_pPoiItem[PANEL_T2],&status12);

		if ((status10==1) && (status12==1))		// if T1==T0 and T1==T2, then T0==T2
		{
			strResultMessage = _("All 3 files are identical (using raw comparison).");
			return;
		}

		errCmp02 = m_pPoiItem[PANEL_T0]->compareFileExact(m_pPoiItem[PANEL_T2],&status02);
		if ((status10==0) && (status12==0) && (status02==0))
		{
			strResultMessage = _("All 3 files are different (using raw comparison).");
			return;
		}
		
		strResultMessage = wxString::Format(wxGetTranslation(L"Raw file comparison results:\n"
															 L"        Left %s Center;\n"
															 L"        Center %s Right;\n"
															 L"        Left %s Right."),
											OP(status10), OP(status12), OP(status02));
		if ((status10==-1) || (status12==-1) || (status02==-1))
			strResultMessage += _("\nThere were errors during the raw comparison of the files.");
		return;
	}
	else
	{
		errCmp10 = m_pPoiItem[PANEL_T1]->compareFileExact(m_pPoiItem[PANEL_T0],&status10);
		switch (status10)
		{
		default:
		case -1:
			strResultMessage = _("There was an error during the raw comparison of the files.");
			return;

		case 0:
			strResultMessage = _("The files are different (using raw comparison).");
			return;

		case 1:
			strResultMessage = _("The files are identical (using raw comparison).");
			return;
		}
	}
}

void fs_fs::_bom_and_nul_byte_in_file_error_dialog(wxWindow * pParent, int kPanel) const
{
	wxASSERT_MSG( (m_errLoad.getErr() == util_error::UE_CONV_BUFFER_HAS_NUL), _T("Coding Error: wrong error") );

	// the file in m_pPoiItem[kPanel] contained a BOM and a NUL character
	// (not just a 0x00 byte, but a full zero -- a 0x00 in utf8, a 0x0000
	// in utf16, etc.  -- a 0x00 byte is a valid part of a utf16 2 byte
	// sequence, but a full zero is not a valid unicode character.)

	wxString strCmpResult;
	_do_raw_comparison(strCmpResult);
	
	const wxChar * szPanel = NULL;
	switch (m_nrTops)
	{
	case 2:
		{
			static const wxChar * aszPanels[] = { _("left"), _("right") };
			szPanel = aszPanels[kPanel];
			break;
		}
					
	default:
	case 3:
		{
			static const wxChar * aszPanels[] = { _("left"), _("center"), _("right") };
			szPanel = aszPanels[kPanel];
			break;
		}
	}

	wxString strMsg = wxString::Format(wxGetTranslation(L"%s\n"
														L"\n"
														L"The file in the %s panel:\n"
														L"    %s\n"
														L"could not be imported because it contained a NUL character.  The file did contain a valid\n"
														L"UNICODE Byte Order Mark, but zero is not a valid code point.  This may be a binary file.\n"
														L"\n"
														L"%s"),
									   m_errLoad.getExtraInfo().wc_str(),		// we want the byte-offset/encoding message
									   szPanel,
									   m_pPoiItem[kPanel]->getFullPath().wc_str(),
									   strCmpResult.wc_str());
	// TODO 2013/09/12 consider cleaning up this error message dialog
	// TODO            like I did to the one in _nul_byte_in_file_error_dialog()
	wxMessageDialog dlg(pParent,strMsg,_("Error!"), wxOK|wxICON_ERROR);
	dlg.ShowModal();
}

bool fs_fs::_nul_byte_in_file_error_dialog(wxWindow * pParent, int kPanel) const
{
	// return true if we should try a different encoding.

	wxASSERT_MSG( (m_errLoad.getErr() == util_error::UE_CONV_BUFFER_HAS_NUL), _T("Coding Error: wrong error") );

	// the file in m_pPoiItem[kPanel] contained a NUL byte.

	wxString strCmpResult;
	_do_raw_comparison(strCmpResult);

	const wxChar * szPanel = NULL;
	switch (m_nrTops)
	{
	case 2:
		{
			static const wxChar * aszPanels[] = { _("left"), _("right") };
			szPanel = aszPanels[kPanel];
			break;
		}
					
	default:
	case 3:
		{
			static const wxChar * aszPanels[] = { _("left"), _("center"), _("right") };
			szPanel = aszPanels[kPanel];
			break;
		}
	}

	wxString strMsg = wxString::Format(wxGetTranslation(L"The file in the %s panel could not be\n"
														L"imported into Unicode because it contains\n"
														L"a NUL character.\n"
														L"\n"
														L"This file may be a binary file or simply in\n"
														L"a different character encoding.\n"
														L"\n"
														L"%s\n"
														L"\n"
														L"\n"
														L"Would you like to try another character encoding?"),
									   szPanel,
									   strCmpResult.wc_str());

	wxMessageDialog dlg(pParent,strMsg,_("Cannot Import File into Unicode!"), wxYES_NO|wxYES_DEFAULT|wxICON_ERROR);
	int answer = dlg.ShowModal();

	return (answer == wxID_YES);
}

//////////////////////////////////////////////////////////////////

bool fs_fs::_try_load_one_file(wxWindow * pParent, int kPanel,
							   RS_ENCODING_STYLE encStyle,
							   int nrEnc, util_encoding aEnc[])
{
	util_encoding enc_ask = aEnc[0];
	bool bUse_enc_ask = false;

	// try to load this file.  loop until we either succeed or we get
	// an error that we can't deal with here or the user cancels.

	bool bAskAgain = false;
	do
	{
		// if they wanted us to ask them before each file is loaded, we
		// ask them (again).  if they hit cancel on the dialog, we abort.
		// if we've already been thru this loop and we could not load
		// the file using the previously selected encoding, ask for another
		// one to try.

		if ((encStyle == RS_ENCODING_STYLE_ASK_EACH) || bAskAgain)
		{
			// raise choose-encoding dialog -- but only preview this one file.

			rs_choose_dlg__charset dlg(pParent,enc_ask,1,getPoiRefTable(kPanel));
			if (!dlg.run(&enc_ask))
			{
				m_errLoad.set(util_error::UE_CANCELED);
				goto Failed;
			}
			bUse_enc_ask = true;
		}

		// actually load the file from disk into a new piecetable.  (addRef is implicit.)

		if (bUse_enc_ask)
			m_errLoad = gpPTableTable->create(m_pPoiItem[kPanel],false,enc_ask,&m_pPTable[kPanel]);
		else
			m_errLoad = gpPTableTable->create(m_pPoiItem[kPanel],false,nrEnc,aEnc,&m_pPTable[kPanel]);

		switch (m_errLoad.getErr())
		{
		case util_error::UE_OK:
			goto NextFile2;							// continue with next file

		case util_error::UE_CONV_BUFFER_HAS_NUL:	// a NUL byte was found in file -- maybe binary or utf{16,32}.
			// tell about NUL and ask if they want to try a different encoding.
			if (!_nul_byte_in_file_error_dialog(pParent,kPanel))
			{
				m_errLoad.set(util_error::UE_CANCELED);
				goto Failed;
			}
			bAskAgain = true;
			break;

		case util_error::UE_CANNOT_IMPORT_CONV:		// an iconv-related problem.
		case util_error::UE_CONV_ODD_BUFFER_LEN:
			_fancy_error_dialog(pParent,
								m_nrTops,kPanel,
								m_pPoiItem[kPanel]->getFullPath(),
								m_errLoad);
			bAskAgain = true;
			break;
				
		default:									// a non-iconv hard error (like no read access)
			_fancy_error_dialog(pParent,
								m_nrTops,kPanel,
								m_pPoiItem[kPanel]->getFullPath(),
								m_errLoad);
			goto Failed;
		}
	} while (bAskAgain);

NextFile2:
	return true;

Failed:
	return false;
}

//////////////////////////////////////////////////////////////////

util_error fs_fs::loadFiles(wxWindow * pParent)
{
	// populate piecetables for each document in the set.

	wxASSERT_MSG( (m_bLoaded==false), _T("Coding Error: already loaded") );

	int kPanel;
	int cAlreadyLoaded = 0;
	RS_ENCODING_STYLE encStyle;
	util_encoding enc[3];
	int nrEnc = 0;

	m_errLoad.clear();
	
	const rs_ruleset * pRS = gpRsRuleSetTable->findBestRuleSet(pParent,m_nrTops,getPoiRefTable(),m_pPoiResult);
	if (!pRS)
	{
		m_errLoad.set(util_error::UE_CANCELED);
		goto Failed;
	}
	
	// if a POI is already in a file set (in another window), we can just hook in
	// and share its piecetable.

	for (kPanel=0; (kPanel < m_nrTops); kPanel++)
	{
		fim_ptable * pPoiPTable = m_pPoiItem[kPanel]->getPTable();
		if (pPoiPTable)
		{
			cAlreadyLoaded++;
			
			m_pPTable[kPanel] = pPoiPTable;
			gpPTableTable->addRef(m_pPTable[kPanel]);
		}
	}

	if (cAlreadyLoaded == m_nrTops)
		goto AllLoaded;
	
	if (pRS->getSniffEncodingBOM())
	{
		// automatic detection enabled -- see if the file starts with a Unicode BOM.
		// if so, we know what to do; if not, we use the fall-back rules for this file.
		// but we defer the fall-back until we've sampled all of the files.

		for (kPanel=0; (kPanel < m_nrTops); kPanel++)
		{
			wxASSERT_MSG( (m_pPoiItem[kPanel]), _T("Coding Error!") );

			if (m_pPTable[kPanel])			// piecetable already in memory before we were called
				continue;					// continue with next file

			fim_ptable * pPoiPTable = m_pPoiItem[kPanel]->getPTable();
			if (pPoiPTable)					// piecetable loaded by previous iteration of this loop
			{								// this happens if you load a file twice in a 3-way, for example.
				cAlreadyLoaded++;

				m_pPTable[kPanel] = pPoiPTable;
				gpPTableTable->addRef(m_pPTable[kPanel]);
				continue;					// continue with next file
			}

			// load the file (sniffing for a Unicode BOM) into a new piecetable.  (addRef is implicit.)
			// if we fail, then we either had an error or there wasn't a BOM.

			util_encoding encUnused = wxFONTENCODING_DEFAULT;
			m_errLoad = gpPTableTable->create(m_pPoiItem[kPanel],true,encUnused,&m_pPTable[kPanel]);

			switch (m_errLoad.getErr())
			{
			case util_error::UE_OK:						// successful import using BOM
				cAlreadyLoaded++;
				goto NextFile1;							// continue with next file
				
			case util_error::UE_NO_UNICODE_BOM:			// no BOM in file
				goto NextFile1;							// continue with next file, we'll get this one later

			case util_error::UE_CONV_BUFFER_HAS_NUL:	// a BOM, but also an invalid NULL character/code point
				_bom_and_nul_byte_in_file_error_dialog(pParent,kPanel);
				goto Failed;

			case util_error::UE_CANNOT_IMPORT_CONV:	// a BOM, but conversion failed
			case util_error::UE_CONV_ODD_BUFFER_LEN:	// a BOM, but data not multiple of character/code point size
			default:									// a non-iconv hard error (like no read access)
				_fancy_error_dialog(pParent,
									m_nrTops,kPanel,
									m_pPoiItem[kPanel]->getFullPath(),
									m_errLoad);
				goto Failed;
			}

		NextFile1:
			;
		}
	}

	if (cAlreadyLoaded == m_nrTops)
		goto AllLoaded;

	// we use the fall-back rules
		
	encStyle = pRS->getEncodingStyle();
	switch (encStyle)
	{
	default:
		wxASSERT_MSG( (0), _T("Coding Error") );	// unknown encStyle
		enc[0] = wxFONTENCODING_DEFAULT;
		nrEnc = 1;
		break;

	case RS_ENCODING_STYLE_NAMED1:
	case RS_ENCODING_STYLE_NAMED2:
	case RS_ENCODING_STYLE_NAMED3:
		nrEnc = pRS->getNamedEncodingArray( NrElements(enc), enc );
		break;

	case RS_ENCODING_STYLE_LOCAL:
		enc[0] = wxFONTENCODING_DEFAULT;
		nrEnc = 1;
		break;

	case RS_ENCODING_STYLE_ASK_EACH:
		// don't know what to pick, but seed dialog with default.
		// wait to ask until we get into the loop for each file.
		enc[0] = wxFONTENCODING_DEFAULT;
		nrEnc = 1;
		break;
		
	case RS_ENCODING_STYLE_ASK:
		// don't know what to pick, but seed dialog with default.
		// go ahead and ask now.
		enc[0] = wxFONTENCODING_DEFAULT;
		nrEnc = 1;

		// we have preview panels for the files that we don't already
		// know about.

		poi_item * pPoiNotLoadedTable[__NR_TOP_PANELS__];
		int kNotLoaded = 0;

		for (kPanel=0; (kPanel < m_nrTops); kPanel++)
		{
			fim_ptable * pPoiPTable = m_pPoiItem[kPanel]->getPTable();
			if (!pPoiPTable)
				pPoiNotLoadedTable[kNotLoaded++] = m_pPoiItem[kPanel];
		}

		rs_choose_dlg__charset dlg(pParent,enc[0],kNotLoaded,pPoiNotLoadedTable);
		if (!dlg.run(&enc[0]))
		{
			m_errLoad.set(util_error::UE_CANCELED);
			goto Failed;
		}
		break;

	}

	// now load and create piecetables for any not already in memory.

	for (kPanel=0; (kPanel < m_nrTops); kPanel++)
	{
		wxASSERT_MSG( (m_pPoiItem[kPanel]), _T("Coding Error!") );

		if (m_pPTable[kPanel])			// piecetable already in memory before we were called
			continue;					// continue with next file

		fim_ptable * pPoiPTable = m_pPoiItem[kPanel]->getPTable();
		if (pPoiPTable)					// piecetable loaded by previous iteration of this loop
		{								// this happens if you load a file twice in a 3-way, for example.
			m_pPTable[kPanel] = pPoiPTable;
			gpPTableTable->addRef(m_pPTable[kPanel]);
			continue;					// continue with next file
		}

		if (!_try_load_one_file(pParent, kPanel, encStyle, nrEnc, enc))
			goto Failed;

	}
	
AllLoaded:
	m_bLoaded = true;

	// remember the RuleSet we chose.
	//
	// TODO consider adding a cb to let us know when something within the RuleSet is changed.

	m_pRuleSet = pRS;
	
	return m_errLoad;

Failed:
	// if we failed for any reason, we need to unref any piecetables that
	// we created or linked to.

	for (kPanel=0; (kPanel < m_nrTops); kPanel++)
	{
		if (m_pPTable[kPanel])
		{
			gpPTableTable->unRef(m_pPTable[kPanel]);
			m_pPTable[kPanel] = NULL;
		}
	}

	return m_errLoad;
}

#ifdef FEATURE_BATCHOUTPUT
util_error fs_fs::loadFiles_without_asking(void)
{
	// populate piecetables for each document in the set ***WITHOUT ASKING ANY QUESTIONS***.

	wxASSERT_MSG( (m_bLoaded==false), _T("Coding Error: already loaded") );

	int kPanel;
	int cAlreadyLoaded = 0;
	RS_ENCODING_STYLE encStyle;
	util_encoding enc[3];
	int nrEnc = 0;

	m_errLoad.clear();
	
	const rs_ruleset * pRS = gpRsRuleSetTable->findBestRuleSet_without_asking(m_nrTops,getPoiRefTable(),m_pPoiResult);
	wxASSERT_MSG( (pRS), _T("Coding Error"));
	
	// if a POI is already in a file set (in another window), we can just hook in
	// and share its piecetable.

	for (kPanel=0; (kPanel < m_nrTops); kPanel++)
	{
		fim_ptable * pPoiPTable = m_pPoiItem[kPanel]->getPTable();
		if (pPoiPTable)
		{
			cAlreadyLoaded++;
			
			m_pPTable[kPanel] = pPoiPTable;
			gpPTableTable->addRef(m_pPTable[kPanel]);
		}
	}

	if (cAlreadyLoaded == m_nrTops)
		goto AllLoaded;
	
	if (pRS->getSniffEncodingBOM())
	{
		// automatic detection enabled -- see if the file starts with a Unicode BOM.
		// if so, we know what to do; if not, we use the fall-back rules for this file.
		// but we defer the fall-back until we've sampled all of the files.

		for (kPanel=0; (kPanel < m_nrTops); kPanel++)
		{
			wxASSERT_MSG( (m_pPoiItem[kPanel]), _T("Coding Error!") );

			if (m_pPTable[kPanel])			// piecetable already in memory before we were called
				continue;

			fim_ptable * pPoiPTable = m_pPoiItem[kPanel]->getPTable();
			if (pPoiPTable)					// piecetable loaded by previous iteration of this loop
			{								// this happens if you load a file twice in a 3-way, for example.
				cAlreadyLoaded++;

				m_pPTable[kPanel] = pPoiPTable;
				gpPTableTable->addRef(m_pPTable[kPanel]);
				continue;
			}

			// load the file (sniffing for a Unicode BOM) into a new piecetable.  (addRef is implicit.)
			// if we fail, then we either had an error or there wasn't a BOM.

			util_encoding encUnused = wxFONTENCODING_DEFAULT;
			m_errLoad = gpPTableTable->create(m_pPoiItem[kPanel],true,encUnused,&m_pPTable[kPanel]);

			if (!m_errLoad.isErr())
			{
				// successful import using BOM

				cAlreadyLoaded++;
				continue;
			}

			if (m_errLoad.getErr() == util_error::UE_NO_UNICODE_BOM)		// no BOM in file
				continue;
			
			// the BOM was present and bogus or we had some non-iconv-related hard error.

			goto Failed;
		}
	}

	if (cAlreadyLoaded == m_nrTops)
		goto AllLoaded;

	// we use the fall-back rules -- but ***WITHOUT ASKING THEM***
		
	encStyle = pRS->getEncodingStyle();
	switch (encStyle)
	{
	case RS_ENCODING_STYLE_NAMED1:
	case RS_ENCODING_STYLE_NAMED2:
	case RS_ENCODING_STYLE_NAMED3:
		nrEnc = pRS->getNamedEncodingArray( NrElements(enc), enc );
		break;

	case RS_ENCODING_STYLE_LOCAL:
		enc[0] = wxFONTENCODING_DEFAULT;
		nrEnc = 1;
		break;

	case RS_ENCODING_STYLE_ASK_EACH:
	case RS_ENCODING_STYLE_ASK:
		// if the ruleset says to ask (once) for the character encoding or
		// ask (once for each file), we just supply the default encoding.
		enc[0] = wxFONTENCODING_DEFAULT;
		nrEnc = 1;
		break;

	default:
		wxASSERT_MSG( (0), _T("Coding Error") );	// unknown encStyle
		enc[0] = wxFONTENCODING_DEFAULT;
		nrEnc = 1;
		break;
	}
	
	// now load and create piecetables for any not already in memory.

	for (kPanel=0; (kPanel < m_nrTops); kPanel++)
	{
		wxASSERT_MSG( (m_pPoiItem[kPanel]), _T("Coding Error!") );

		if (m_pPTable[kPanel])			// piecetable already in memory before we were called
			continue;

		fim_ptable * pPoiPTable = m_pPoiItem[kPanel]->getPTable();
		if (pPoiPTable)					// piecetable loaded by previous iteration of this loop
		{								// this happens if you load a file twice in a 3-way, for example.
			m_pPTable[kPanel] = pPoiPTable;
			gpPTableTable->addRef(m_pPTable[kPanel]);
			continue;
		}

		// actually load the file from disk into a new piecetable.  (addRef is implicit.)

		m_errLoad = gpPTableTable->create(m_pPoiItem[kPanel],false,nrEnc,enc,&m_pPTable[kPanel]);
		if (m_errLoad.isErr())
			goto Failed;
	}
	
AllLoaded:
	m_bLoaded = true;

	// remember the RuleSet we chose.

	m_pRuleSet = pRS;
	
	return m_errLoad;

Failed:
	// if we failed for any reason, we need to unref any piecetables that
	// we created or linked to.

	for (kPanel=0; (kPanel < m_nrTops); kPanel++)
	{
		if (m_pPTable[kPanel])
		{
			gpPTableTable->unRef(m_pPTable[kPanel]);
			m_pPTable[kPanel] = NULL;
		}
	}

	return m_errLoad;
}
#endif

//////////////////////////////////////////////////////////////////

bool fs_fs::setupEditing(void)
{
	// return true if we created a new edit-buffer for editing.
	// return false if PANEL_T1 already had an edit-buffer
	// associated with it (in another window).

	// only call once per fs_fs.
	wxASSERT_MSG( (!m_pPTableEdit), _T("Coding Error") );

	// after loading all top-panel files successfully, call this if
	// you want to enable editing in the BOTTOM panel.

	wxASSERT_MSG( (m_pPoiItem[PANEL_T1] && m_pPTable[PANEL_T1]), _T("Coding Error") );

	// setFiles() uses a delayed (lazy) load technique.
	// ensure that the caller realizes this.

	wxASSERT_MSG( (m_bLoaded), _T("Coding Error") );

	wxASSERT_MSG( (!m_bReadOnly), _T("Coding Error") );

	// see if PANEL_T1 already has an editing buffer.  if so, we
	// just reference it.  if not, clone our document and create
	// a tempfile for it.

	poi_item * pPoiEdit = m_pPoiItem[PANEL_T1]->getPoiEditBuffer();
	bool bCreateEditBuffer = (pPoiEdit == NULL);

	if (bCreateEditBuffer)
	{
		// create a temp file with a recognizable prefix.
		// the format of filenames for temp files varies on all 3 platforms so we
		// adjust it a little for each.
		//
		// With W4316 we now let wxWidgets choose the directory for the temp file.
		
		wxString strPrefix;
#if   defined(__WXMSW__)
		// The system GetTempFileName() on Win32 only uses the first 3 characters
		// in the template filename.  It creates xxxUUUU.TMP where UUUU is a number
		// containing 1 to 4 digits.  (very 8.3 friendly)
		//
		// Build "___"
		strPrefix += _T("___");

#elif defined(__WXGTK__) || defined(__WXMAC__)
		// On the Linux/MAC, it takes the entire prefix upto (and NOT including the ".")
		// and uses it.  It then tacks on a random sequence of letters/numbers.
		//
		// Build "___<filename>___<extension>___"
		wxFileName fnOriginal = m_pPoiItem[PANEL_T1]->getFullPath();
		strPrefix += _T("___");
		strPrefix += fnOriginal.GetName();
		strPrefix += _T("___");
		strPrefix += fnOriginal.GetExt();
		strPrefix += _T("___");

#else
#error define platform for fs_fs.cpp temp filename template.
#endif
		
		// create a new POI with (tempfile,cloned_piecetable)

		// TODO decide what the propMask should have in it.

		m_pPoiItemEdit = gpPoiItemTable->addItem(wxFileName::CreateTempFileName(strPrefix));
		m_pPTableEdit  = gpPTableTable->createClone(m_pPTable[PANEL_T1], ((fr_prop)0x0), m_pPoiItemEdit);	// contains implicit addRef()

		// turn on auto-save on the edit buffer document.

		m_pPTableEdit->enableAutoSave();
		
		// set EditSrc link on new POI back to T1

		m_pPoiItemEdit->setPoiEditSrc(m_pPoiItem[PANEL_T1]);

		// set EditBuffer link on T1's POI to the new POI

		m_pPoiItem[PANEL_T1]->setPoiEditBuffer(m_pPoiItemEdit);

//		wxLogTrace(TRACE_FS_DUMP,_T("fs_fs::setupEditing: creating new POI for [%s][%s]"),
//				   util_printable_s(m_pPoiItem[PANEL_T1]->getFullPath()).wc_str(),
//				   util_printable_s(m_pPoiItemEdit->getFullPath()).wc_str());
	}
	else
	{
		// the document in T1 already has an edit buffer in another window.
		// just reference it

		m_pPoiItemEdit = pPoiEdit;
		m_pPTableEdit  = pPoiEdit->getPTable();
		gpPTableTable->addRef(m_pPTableEdit);

		wxASSERT_MSG( (m_pPoiItemEdit->getPoiEditSrc() == m_pPoiItem[PANEL_T1]), _T("Coding Error") );

//		wxLogTrace(TRACE_FS_DUMP,_T("fs_fs::setupEditing: sharing existing POI for [%s][%s]"),
//				   util_printable_s(m_pPoiItem[PANEL_T1]->getFullPath()).wc_str(),
//				   util_printable_s(m_pPoiItemEdit->getFullPath()).wc_str());
	}

	return bCreateEditBuffer;
}

void fs_fs::forceAutoSaveNow(void)
{
	if (m_pPTableEdit)
		m_pPTableEdit->forceAutoSaveNow();
}

static void my_remove_file(const wxString & str)
{
	// quietly try to remove the given pathname.
	// divert error log so we don't get that
	// stupid modal dialog the next idle time we
	// go idle.

#if 1 && defined(DEBUG)
	wxLogTrace(TRACE_FS_DUMP, _T("fs_fs:auto_save: removing '%s'"), str);
#endif

	util_error ue;
	util_logToString uLog(&ue.refExtraInfo());

	if (::wxRemoveFile(str))
		return;

#if 1 && defined(DEBUG)
	wxLogTrace(TRACE_FS_DUMP, _T("fs_fs:auto_save: remove failed for: '%s'"), str);
#endif
}


void fs_fs::deleteAutoSaveFile(void)
{
	if (m_pPTableEdit)
	{
		my_remove_file( m_pPoiItemEdit->getFullPath() );
	}
}

//////////////////////////////////////////////////////////////////

void fs_fs::changeRuleset(const rs_ruleset * pRS)
{
	// we initially pick a ruleset when we load the files.
	// this happens automagically.
	//
	// use this method to let the user change the ruleset
	// from the menu.

	// TODO if we ever add a cb to let us know when something
	// TODO within the ruleset changes, clear and reset it.

	m_pRuleSet = pRS;

	// notify everyone watching this fs_fs that the ruleset has changed
	
	m_cblRsChange.callAll( util_cbl_arg(this) );
}

//////////////////////////////////////////////////////////////////

bool fs_fs::reloadFiles(wxWindow * pParent, bool bForceReload)
{
	// user hit FILE|RELOAD menu item.
	//
	// if we have an edit panel, see if it is dirty.  if so, ask if they
	// want to discard their changes (if not, abort the reload).
	//
	// if any of the top panels are dirty in another frame, complain and
	// abort the reload.  (don't recurse on this as it will get nasty.)
	//
	// if all is ok, we then reload the files in the top panels.
	//
	//////////////////////////////////////////////////////////////////
	// note: since a file may be in the top panel of multiple windows, this
	// note: may cause other windows to update.
	//////////////////////////////////////////////////////////////////
	// WARNING: we assume that the caller has already checked for any
	// WARNING: panel being dirty (either in this window or in another
	// WARNING: window) and asked the user to confirm.
	//////////////////////////////////////////////////////////////////

	wxASSERT_MSG( (m_bLoaded), _T("Coding Error") );

	util_error ue;

	m_errLoad.clear();

	for (int kPanel=0; (kPanel<m_nrTops); kPanel++)
	{
		wxASSERT_MSG( (m_pPoiItem[kPanel]), _T("Coding Error!") );
		wxASSERT_MSG( (m_pPTable[kPanel]),  _T("Coding Error!") );

		// screw-case: if they load the same file into multiple panels
		// we only want to actually re-read the file once.

		bool bDuplicate = false;
		for (int jPanel=0; (jPanel<kPanel); jPanel++)
			if (m_pPTable[kPanel] == m_pPTable[jPanel])
				bDuplicate = true;
		if (bDuplicate)
			continue;

		if (!bForceReload)
		{
			// see if the disk file has changes since we loaded it.
			// if not, no need to re-read it.  likewise, if we can't
			// stat() it, don't bother trying to re-read it.

			if (!m_pPTable[kPanel]->hasChangedOnDiskSinceLoading(ue) || ue.isErr())
				continue;
		}
		
		// ask the piecetable to reload the file -- using all of the
		// previous settings.  that is, using the character encoding
		// that it used last time.  i don't think it's necessary to
		// re-do all of that ask-each or ask-on-error stuff.  so, if
		// we can't decode the file (now), give them an error message
		// and create an empty buffer (on this panel, rather than
		// reverting the whole frame to an empty window).  likewise,
		// if we get permission problems and can't re-read the file,
		// give them an error message and create an empty buffer.

		ue = m_pPTable[kPanel]->reload();

		// regardless of whether the reload() succeeds or fails, we
		// need to re-clone the document's edit-buffer, if it has one.
		// this is so that the edit-buffer always matches the initial
		// source-buffer.

		poi_item * pPoiItem_Src = m_pPoiItem[kPanel];
		poi_item * pPoiItem_Edit = pPoiItem_Src->getPoiEditBuffer();
		if (pPoiItem_Edit)
		{
			fim_ptable * pPTableEdit = pPoiItem_Edit->getPTable();
			if (pPTableEdit)
				pPTableEdit->re_clone(m_pPTable[kPanel],((fr_prop)0x0));
		}

		if (ue.isErr())
		{
			wxMessageDialog dlg(pParent,ue.getMBMessage(),_("Error!"), wxOK|wxICON_ERROR);
			dlg.ShowModal();

			m_errLoad = ue;		// remember the last error
		}
	}

	return m_errLoad.isOK();
}

//////////////////////////////////////////////////////////////////

util_error fs_fs::fileSave(wxWindow * pParent)
{
	util_error ue;

	// save the edit buffer (the document in PANEL_EDIT) into
	// either the result-pathname (given on the command line)
	// or the pathname used to load PANEL_T1.

	wxASSERT_MSG( (m_pPTableEdit), _T("Coding Error") );

	if (m_pPoiResult)				// we want to write to /result:<pathname>
	{
		if (m_pPoiResult->getPTable() == NULL)
		{
			// the file defined in result-pathname is not in-use.  so we are free
			// to do a save-as using the edit-buffer into the result-pathname.
			// this will convert the fs_fs to not have a result-pathname and a new T1.

			// TODO should we check and see if the result-pathname
			// TODO already exists and ask if they want to overwrite
			// TODO it?

			return _fileSaveAs(pParent,getPoi(SYNC_VIEW,PANEL_T1),m_pPoiResult);
		}
		else
		{
			// result-pathname is open in another window somewhere.

			wxString str(m_pPoiResult->getFullPath().wc_str());
			str += _("\n\nThis window was given /result:<pathname> on the command line.\n");
			str += _("Saving this window will write to the above pathname instead of the file loaded.\n");
			str += _("This feature is used by Vault/Fortress to locate the merge result.\n\n");
			if (m_pPoiResult->getPTable()->getPtStat() & PT_STAT_IS_DIRTY)
				str += _("This file is currently open and modified in another window.\n");
			else
				str += _("This file is currently open in another window.\n");
			str += _("\nAre you sure you want to continue?");
			wxMessageDialog dlg(pParent,str,_("Result File Open in Other Window"),wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
			int answer = dlg.ShowModal();

			if (answer == wxID_NO)
			{
				ue.set(util_error::UE_CANCELED);
				return ue;
			}

			bool bProceed = _askToProceedIfFileHasChangedOnDisk(pParent,m_pPoiResult->getPTable());
			if (!bProceed)
			{
				ue.set(util_error::UE_CANCELED);
				return ue;
			}

			// save it to disk, but don't rebind T1 (because we can't) and don't force
			// the reloading of the file. (let the window activation of the window actually
			// containing the file trigger that and give them the 'file changed on disk...'
			// message.

			ue = m_pPTableEdit->attemptSaveWithForceIfNecessary(pParent,m_pPoiResult);

			// TODO try to figure out how to create a new fs_fs with a new T1 that
			// TODO looks to the user like it would if the result-pathname hadn't
			// TODO already been open.  that is put the result-pathname's POI/PTable
			// TODO in T1 and return the new fs_fs and let the view rebind to it
			// TODO rather than 'this'.

			return ue;
		}
	}
	else								// overwrite original T1
	{
		bool bProceed = _askToProceedIfFileHasChangedOnDisk(pParent,m_pPTable[PANEL_T1]);
		if (!bProceed)
		{
			ue.set(util_error::UE_CANCELED);
			return ue;
		}
		
		ue = m_pPTableEdit->attemptSaveWithForceIfNecessary(pParent,m_pPoiItem[PANEL_T1]);
		if (ue.isErr())
			return ue;

		// reload T1 using this new file.  but we don't need to go to disk because
		// we can just clone the contents of the edit buffer and avoid all of the
		// disk io, character encoding, and etc.

		m_pPTable[PANEL_T1]->re_clone(m_pPTableEdit,((fr_prop)0x0));	// copy contents of edit buffer
		m_pPTable[PANEL_T1]->updateDateTimes();		// reset DateTimes on ptable using new file

		return ue;
	}
}

//////////////////////////////////////////////////////////////////

poi_item * fs_fs::getPoi(long kSync, PanelIndex kPanel) const
{
	// the gui layer sees us as having: poi[__NR_SYNCS__][__NR_TOP_PANELS__]
	// but T0 and T2 are the same on both notebook pages, so we don't
	// need 6 pointers/objects hanging around.

	if ((kSync==SYNC_EDIT) && (kPanel == PANEL_EDIT))
		return m_pPoiItemEdit;
	else
		return m_pPoiItem[kPanel];
}

fim_ptable * fs_fs::getPTable(long kSync, PanelIndex kPanel) const
{
	// the gui layer sees us as having: ptable[__NR_SYNCS__][__NR_TOP_PANELS__]
	// but T0 and T2 are the same on both notebook pages, so we don't
	// need 6 pointers/objects hanging around.

	if ((kSync==SYNC_EDIT) && (kPanel == PANEL_EDIT))
		return m_pPTableEdit;
	else
		return m_pPTable[kPanel];
}
//////////////////////////////////////////////////////////////////

wxString fs_fs::dumpSupportInfo(const wxString & strIndent) const
{
	wxString str;
	wxString strIndent2 = strIndent + _T("\t");
	wxString strIndent3 = strIndent2 + _T("\t");

	str += wxString::Format( _T("%sFiles Loaded:\n"), strIndent.wc_str());

#define D(label,sync,panel)		Statement( str += wxString::Format(_T("%sPanel(%s):\n"), strIndent2.wc_str(),label);	\
										   str += getPTable((sync),(panel))->dumpSupportInfo(strIndent3);			)

	D(_T("Left"), SYNC_VIEW, PANEL_T0);
	if (m_nrTops == 3)
	{
		D(_T("Center/View"), SYNC_VIEW, PANEL_T1);
		if (m_pPTableEdit)
			D(_T("Center/Edit"), SYNC_EDIT, PANEL_EDIT);
		D(_T("Right"), SYNC_VIEW, PANEL_T2);
	}
	else
	{
		D(_T("Right/View"), SYNC_VIEW, PANEL_T1);
		if (m_pPTableEdit)
			D(_T("Right/Edit"), SYNC_EDIT, PANEL_EDIT);
	}

	if (m_pPoiResult)
	{
		str += wxString::Format( _T("%sResult Path: "), strIndent.wc_str());
		str += m_pPoiResult->getFullPath().wc_str();
		str += _T("\n");
	}

	str += wxString::Format( _T("%sRuleSet: %s\n"), strIndent.wc_str(), m_pRuleSet->getName().wc_str());

	return str;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fs_fs::dump(int indent) const
{
	wxLogTrace(TRACE_FS_DUMP, _T("%*cFS_FS: [%p][bLoaded %d][nrTops %d][RuleSet %s]"),
			   indent,_T(' '),
			   this,
			   m_bLoaded,
			   m_nrTops,
			   ((m_pRuleSet) ? (m_pRuleSet->getName().wc_str()) : _T("<NULL>")));
	for (int k=0; k<__NR_TOP_PANELS__; k++)
	{
		wxLogTrace(TRACE_FS_DUMP, _T("%*c[%d]: [pt %p] [poi %p]:[%s]"),
				   indent+5,_T(' '),k,
				   m_pPTable[k],
				   m_pPoiItem[k],
				   ((m_pPoiItem[k]) ? util_printable_s(m_pPoiItem[k]->getFullPath()).wc_str() : _T("")));
	}

	if (m_pPoiItemEdit && m_pPTableEdit)
		wxLogTrace(TRACE_FS_DUMP, _T("%*c[EDIT]: [pt %p] [poi %p]:[%s]"),
				   indent+5,_T(' '),
				   m_pPTableEdit,
				   m_pPoiItemEdit,
				   ((m_pPoiItemEdit) ? util_printable_s(m_pPoiItemEdit->getFullPath()).wc_str() : _T("")));
}
#endif//DEBUG

//////////////////////////////////////////////////////////////////

util_error fs_fs::fileSaveAs(wxWindow * pParent)
{
	poi_item * pPoiNew = NULL;

	util_error ue = _getSaveAsPathname(pParent,&pPoiNew);
	if (ue.isErr())
		return ue;

	poi_item * pPoiOldT1 = getPoi(SYNC_VIEW,PANEL_T1);
	if (pPoiNew == pPoiOldT1)		// filename did not change,
		return fileSave(pParent);	// so we can do regular save.

	return _fileSaveAs(pParent,pPoiOldT1,pPoiNew);
}

util_error fs_fs::_fileSaveAs(wxWindow * pParent, poi_item * pPoiOldT1, poi_item * pPoiNew)
{
	// write the contents of the edit buffer to the newly named file

	util_error ue = m_pPTableEdit->attemptSaveWithForceIfNecessary(pParent,pPoiNew);
	if (ue.isErr())
		return ue;

	// if our fs_fs was given a /result:<pathname>, we need to override it.

	if (m_pPoiResult)
		m_pPoiResult = NULL;

	// rebind the ptable in T1 to reference the new POI.  this essentially
	// causes a rename on the ptable without causing and reload or other
	// higher-level rebindings and without changing window associations or
	// losing undo info.  the ptable should signal that it's name has changed
	// and cause all viewers to post the new name (in titles, captions, and
	// etc.)  we also need to update DateTimeModified and DateTimeAsLoaded
	// in the ptable because it reflects a different file (that we just cloned
	// instead of loading); this helps prevent false/unnecessary 'file has
	// changed on disk. {reload? | overwrite?}' message boxes.
	
	gpFsFsTable->rebindPoi(pPoiOldT1,pPoiNew);	// update all fs_fs (including 'this') that reference the ptable in our T1
	m_pPTable[PANEL_T1]->rebindPoi(pPoiNew);	// update poi<-->ptable links
	m_pPTable[PANEL_T1]->re_clone(m_pPTableEdit,((fr_prop)0x0));	// copy contents of edit buffer
	m_pPTable[PANEL_T1]->updateDateTimes();		// reset DateTimes on ptable using new file

	return ue;
}

bool fs_fs::_askToProceedIfFileHasChangedOnDisk(wxWindow * pParent, fim_ptable * pPTable) const
{
	util_error ue;

	bool bHasChangedOnDisk = pPTable->hasChangedOnDiskSinceLoading(ue);

	// if there was an error (like an access() problem while
	// stat()-ing the file or some other problem) we really
	// cannot tell if the file has changed or not.  it may
	// have even been deleted.
	//
	// so don't bother asking if they want to overwrite it.

	if (ue.isErr() || !bHasChangedOnDisk)
		return true;
	
	wxString str(pPTable->getPoiItem()->getFullPath().wc_str());
	str += _("\n\nThis file has changed on disk since it was loaded.\nAre you sure you want to overwrite it?");

	wxMessageDialog dlg(pParent,str,_("File Changed on Disk"),wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
	int answer = dlg.ShowModal();

	return (answer == wxID_YES);
}

util_error fs_fs::_getSaveAsPathname(wxWindow * pParent, poi_item ** ppPoiNew) const
{
	// ask the user for a save-as pathname.
	// loop until they give us one that is acceptable.

	util_error ue;

	*ppPoiNew = NULL;

	// if we have a /result:<pathname> warn them before starting.

	if (m_pPoiResult)
	{
		wxString strMsg(_("This window was given a /result:<pathname> argument on the command line.\n"));
		strMsg +=       _("This argument is used by Vault/Fortress to locate the merge result.\n\n");
		strMsg +=       _("This 'Save As' operation will override this.\n\n");
		strMsg +=       _("Are you sure that you want to continue?");

		wxMessageDialog dlgMsg(pParent,strMsg,_("Override /result:<pathname> option?"),wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
		int answer = dlgMsg.ShowModal();

		if (answer == wxID_NO)
		{
			ue.set(util_error::UE_CANCELED);
			return ue;
		}
	}
	
	// lookup POI of the original file -- this is in SYNC_VIEW,PANEL_T1.
	// (SYNC_EDIT,PANEL_EDIT) contains the temp file.

	poi_item * pPoiOldT1 = getPoi(SYNC_VIEW,PANEL_T1);

	wxString strSeed(pPoiOldT1->getFullPath());

	while (1)
	{
		// seed the dialog with the current value of the edit pathname
		// or the result of the previous invocation of the dialog.

		wxFileName fnSeed(strSeed);
		wxString strDir = fnSeed.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
		wxString strFile = fnSeed.GetFullName();

		wxFileDialog dlg(pParent,_("Save As"),strDir,strFile,_T("*.*"),wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() != wxID_OK)
		{
			ue.set(util_error::UE_CANCELED);
			return ue;
		}

		wxString strDialogResult = dlg.GetPath();
		poi_item * pPoiDialogResult = gpPoiItemTable->addItem(strDialogResult);

		// if the new pathname is the same as the original, we should warn them
		// because they wanted to do a SaveAs not a Save.

		if (pPoiDialogResult == pPoiOldT1)
		{
			wxString strMsg(strDialogResult);
			strMsg += _("\n\nThe new pathname is the same as the original.\n\nAre you sure you want continue?");
			
			wxMessageDialog dlgMsg(pParent,strMsg,_("Same Pathname as Original"),wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
			int answer = dlgMsg.ShowModal();

			if (answer == wxID_YES)
			{
				// they want the same pathname, caller can just do a regular save...

				*ppPoiNew = pPoiDialogResult;
				return ue;
			}
			
			strSeed = strDialogResult;
			continue;	// try again
		}

		//////////////////////////////////////////////////////////////////
		// each unique pathname maps into a unique POI.  for each POI, there can only be
		// one FIM_PTable (and FL layout).  so, to do a save-as, after we write the contents
		// of the ptable to the new pathname, we need to either rebind the current ptable to
		// the new POI for this new pathname or we need to unref the current ptable,poi and
		// essentially do a load on the new pathname and create a new ptable.
		//
		//////////////////////////////////////////////////////////////////
		// problems happen when the new pathname (and it's corresponding ptable) are open
		// in other windows or other panels in this window.
		// [] if we rebind, we'll have 2 ptables with 1 poi, this can't happen.
		// [] if we load a new poi/ptable:
		//    () our write causes the file to change and we load it into this edit buffer, but
		//       it will also cause all other windows/panels to reload the file because it changed
		//       on disk.
		//    () if the new pathname is already being edited in another window, we will implicitly
		//       share with that window (rather than whatever window we may have been sharing with
		//       previously).  this may confuse the user.
		//    () if we share with another edit window, our undo history will reflect a different
		//       window.
		//
		// because of this, we should prohibit the pathname if it is already in-use and
		// avoid the screwyness.
		//////////////////////////////////////////////////////////////////

		if (pPoiDialogResult->getPTable())	// if file in use in another window/panel
		{
			wxString strMsg(strDialogResult);
			strMsg += _("\n\nThis file is open in another window.\n\nPlease select another name.");

			wxMessageDialog dlgMsg(pParent,strMsg,_("File in Use in Another Window"),wxOK|wxCANCEL|wxICON_ERROR);
			int answer = dlgMsg.ShowModal();

			if (answer != wxID_OK)
			{
				ue.set(util_error::UE_CANCELED);
				return ue;
			}

			strSeed = strDialogResult;
			continue;	// try again
		}

		// we approve of the new pathname.

		*ppPoiNew = pPoiDialogResult;
		return ue;
	}

	/*NOTREACHED*/
	return ue;
}

void fs_fs::rebindPoi(poi_item * pPoiOld, poi_item * pPoiNew)
{
	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_pPoiItem[kPanel] == pPoiOld)
			m_pPoiItem[kPanel] = pPoiNew;

	if (m_pPoiItemEdit == pPoiOld)
		m_pPoiItemEdit = pPoiNew;
}

//////////////////////////////////////////////////////////////////

/**
 * Return strLabel{AB} with appropriate for use as column/page
 * headers for each file when exporting a diff.  Normally this
 * is just the file pathnames.
 *
 * If titles are given, include them as well.
 *
 * If the right side is dirty, indicate that with a '*'.
 *
 */
void fs_fs::makeLabelForExportUnifiedHeader(long kSync,
											wxString & strLabelA,
											wxString & strLabelB,
											const wxString & strTitleA,
											const wxString & strTitleB) const
{
	// we always want to use the pathnames from the SYNC_VIEW
	// regardless of the value of kSync because we want the
	// pathnames of the files as loaded (and the POI for
	// SYNC_EDIT has the path of the temporary edit buffer).
	wxString strPathA = getPoi(SYNC_VIEW, (PanelIndex)0)->getFullPath();
	wxString strPathB = getPoi(SYNC_VIEW, (PanelIndex)1)->getFullPath();

	if (strTitleA.Length() > 0)
		strLabelA = wxString::Format(_T("%s - %s"), strTitleA, strPathA);
	else
		strLabelA = strPathA;

	if (strTitleB.Length() > 0)
		strLabelB = wxString::Format(_T("%s - %s"), strTitleB, strPathB);
	else
		strLabelB = strPathB;

	bool bDirtyB = (   (kSync == SYNC_EDIT)
					&& (PT_STAT_TEST( getPTable(SYNC_EDIT,PANEL_EDIT)->getPtStat(),
									  PT_STAT_IS_DIRTY )));
	if (bDirtyB)
		strLabelB += _T("*");
	
}

/**
 * Convert the given buffer using the character encoding of
 * one of the files in this pair (unless we're forced to use
 * UTF-8) and then write it to disk in the destination file.
 *
 * We use this for exporting diffs, for example.
 * 
 */
util_error fs_fs::writeStringInEncodingToFile(long kSync,
											  const poi_item * pPoiDestination,
											  const wxString & strOutput,
											  bool bForceUTF8) const
{
	util_error ue;
	byte * pBuf = NULL;
	size_t len = 0;

	if (strOutput.Length() > 0)
	{
		if (bForceUTF8)
		{
			// When writing HTML, we force UTF-8 since we declared
			// that in the meta-data in the HTML header.
			ue = util_encoding_export_conv(strOutput, wxFONTENCODING_UTF8, &pBuf, &len);
			if (ue.isErr())
				return ue;
		}
		else
		{
			fim_ptable * pPTableA = getPTable(kSync, (PanelIndex)0);
			fim_ptable * pPTableB = getPTable(kSync, (PanelIndex)1);
			util_encoding encA = pPTableA->getEncoding();
			util_encoding encB = pPTableB->getEncoding();
		
			// Try to convert the diff output from unicode to the
			// character encoding of one of the input files.  since
			// we allow them to be different, we'll arbitrarily pick
			// one to try first and if that fails try the other before
			// complaining.

			ue = util_encoding_export_conv(strOutput, encA, &pBuf, &len);
			if (ue.isErr() && (encA != encB))
			{
				ue.clear();
				ue = util_encoding_export_conv(strOutput, encB, &pBuf, &len);
			}
			if (ue.isErr())
				return ue;
		}
	}

	// NOTE: From this point forward, we must free pBuf.

	// open the output file and write the contents.
	// divert the error log as usual.
	{
		util_logToString uLog(&ue.refExtraInfo());
		int fd = util_file_open(ue, pPoiDestination->getFullPath(), wxFile::write);
		if (fd == -1)
			goto failed;

		wxFile file(fd);	// this takes over ownership of the fd.

		if (pBuf && len)
		{
			if (!file.Write((const void *)pBuf,len))
			{
				ue.set(util_error::UE_CANNOT_WRITE_FILE);
				goto failed;
			}
		}
	}

failed:
	FREEP(pBuf);
	return ue;
}

