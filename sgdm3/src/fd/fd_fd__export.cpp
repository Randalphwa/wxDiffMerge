// fd_fd__export.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

util_error fd_fd::exportVisibleItems(wxWindow * pParent,
									 FD_EXPORT_FORMAT expfmt,
									 const wxString * pStrPath)
{
	// This is called when the user does a "View | Export..." .
	// We want to dump the contents of the window folder window
	// (using the current show/hide settings) to a file or
	// clipboard.

	// if necessary, ask the user for a pathname to write the output to.

	util_error ue;
	byte * pBuf = NULL;
	size_t len = 0;

	if (expfmt == FD_EXPORT_FORMAT__UNSET)
	{
		ue.set(util_error::UE_UNSUPPORTED, _T("Invalid argument"));
		return ue;
	}

	if (pStrPath)
	{
		if (expfmt & FD_EXPORT__TO_FILE)
		{
			m_pPoiSaveAsPathname = gpPoiItemTable->addItem(*pStrPath);
			m_expfmt = expfmt;
		}
		else
		{
			// they gave us a path and yet said to go to the clipboard.
			ue.set(util_error::UE_UNSUPPORTED, _T("Invalid argument"));
			return ue;
		}
	}
	else if (expfmt & FD_EXPORT__TO_FILE)
	{
		// no path given so ask the user for one.
		ue = _getExportPathnameFromUser(pParent, expfmt);
		if (ue.isErr())
			return ue;
	}
	
	FD_EXPORT_FORMAT expDest = (expfmt & FD_EXPORT__TO_MASK);
	FD_EXPORT_FORMAT expType = (expfmt & FD_EXPORT_FORMAT__MASK);

	// build the output in some format.  (this will be in our wide
	// character internal UNICODE format.)

	wxString strWide;
	switch (expType)
	{
	case FD_EXPORT_FORMAT__CSV:
		ue = _exportContents_csv(strWide);
		break;

	case FD_EXPORT_FORMAT__RQ:
		ue = _exportContents__rq(strWide);
		break;

	case FD_EXPORT_FORMAT__HTML:
		if (expDest != FD_EXPORT__TO_FILE)
		{
			// TODO 2013/08/29 we could support this, but i need to convert
			// TODO            our wide char buffer into a utf-8 buffer and
			// TODO            put it on the clipboard instead of using the
			// TODO            wide char clipboard routines.
			ue.set(util_error::UE_UNSUPPORTED, _T("Html to clipboard"));
			return ue;
		}
		ue = _exportContents__html(strWide);
		break;

	default:
		ue.set(util_error::UE_UNSUPPORTED);
	}
	if (ue.isErr())
		return ue;


	switch (expDest)
	{
	case FD_EXPORT__TO_FILE:
		{
			// convert the output buffer from internal UNICODE to a
			// character encoding that the user might actually find
			// useful.
			//
			// we need to free pBuf when we are done.
			//
			// TODO right now, i'm assuming UTF8.  since all of the
			// TODO pathnames originally came from the filesystem, we
			// TODO might want to use the encoding suggested by wxConvFile
			// TODO instead.  revisit this later.
			//
			// Note that we DO NOT write a BOM (whether text or html).

			util_encoding enc = wxFONTENCODING_UTF8;
	
			if (strWide.Length() > 0)
			{
				ue = util_encoding_export_conv(strWide,enc,&pBuf,&len);
				if (ue.isErr())
					goto failed;
			}

			// open the output file and write the contents.
			// divert the error log as usual.

			{
				util_logToString uLog(&ue.refExtraInfo());

				int fd = util_file_open(ue, m_pPoiSaveAsPathname->getFullPath(), wxFile::write);
				if (fd == -1)
					goto failed;

				wxFile file;
				file.Attach(fd);
		
				if (pBuf && len)
				{
					if (!file.Write((const void *)pBuf,len))
					{
						ue.set(util_error::UE_CANNOT_WRITE_FILE);
						goto failed;
					}
				}
			}
		}
		break;

	case FD_EXPORT__TO_CLIPBOARD:
		{
			if (wxTheClipboard->Open())
			{
				wxTextDataObject * pTDO = new wxTextDataObject(strWide);

				wxTheClipboard->SetData(pTDO);
				wxTheClipboard->Close();
			}
		}
		break;

	}

	FREEP(pBuf);
	return ue;

failed:
	FREEP(pBuf);
	return ue;
}

//////////////////////////////////////////////////////////////////

util_error fd_fd::_getExportPathnameFromUser(wxWindow * pParent, FD_EXPORT_FORMAT expfmt)
{
	// ask the user for a save-as pathname for our export.

	util_error ue;

	// seed the dialog with the current value of the edit pathname
	// or the result of the previous invocation of the dialog.

	wxString strDir;
	wxString strFile;

	if (m_pPoiSaveAsPathname &&
		((expfmt & FD_EXPORT_FORMAT__MASK) == (m_expfmt & FD_EXPORT_FORMAT__MASK)))
	{
		// if we have already done a save-as on this window
		// **AND WE ARE USING THE SAME EXPORT FORMAT AS LAST TIME**,
		// seed the dialog with the pathname we used last time.
		wxFileName fnSeed(m_pPoiSaveAsPathname->getFullPath());
		strDir = fnSeed.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
		strFile = fnSeed.GetFullName();
	}
	else
	{
		// otherwise, seed dialog with the directory of the last save-as.
		// we do not want the filename portion.
		wxString strSeed(gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FOLDER_SAVEAS_SEED)); /*gui*/
		wxFileName fnSeed(strSeed);
		strDir = fnSeed.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
		//strFile = fnSeed.GetFullName();
	}

	wxString strPattern;
	switch (expfmt & FD_EXPORT_FORMAT__MASK)
	{
	case FD_EXPORT_FORMAT__CSV:
		strPattern = _T("UTF-8 CSV (*.csv)|*.csv");
		break;

	case FD_EXPORT_FORMAT__RQ:
		strPattern = _T("Summary (*.txt)|*.txt");
		break;

	case FD_EXPORT_FORMAT__HTML:
		strPattern = _T("HTML (*.html)|*.html");
		break;

	default:
		ue.set(util_error::UE_UNSUPPORTED);
		return ue;
	}
		
	wxFileDialog dlg(pParent,_("Save As"),
					 strDir,strFile,
					 strPattern,
					 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK)
	{
		ue.set(util_error::UE_CANCELED);
		return ue;
	}

	wxString strDialogResult = dlg.GetPath();
	m_pPoiSaveAsPathname = gpPoiItemTable->addItem(strDialogResult);
	m_expfmt = expfmt;

	gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FOLDER_SAVEAS_SEED,strDialogResult); /*gui*/

	return ue;
}
	
