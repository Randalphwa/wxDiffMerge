// util_shell_lnk.cpp
// wrapper for dealing with Windows Shortcuts (.lnk) files.
//
// See also util_file__is_shell_lnk()
//
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb774942%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb773321%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb759800%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/cc144090%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb762194%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb762114%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb759820%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb776885%28v=vs.85%29.aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb776891%28v=vs.85%29.aspx
//
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

#if defined(__WXMSW__)
#include <shlobj.h>
#include <shlwapi.h>

//////////////////////////////////////////////////////////////////

util_shell_lnk::util_shell_lnk(void)
{
	memset(this, 0, sizeof(util_shell_lnk));
}

util_shell_lnk::~util_shell_lnk(void)
{
	PIDLIST_ABSOLUTE pidl = (PIDLIST_ABSOLUTE)pVoid_pidl;

	DELETEP( pStrArguments );
	DELETEP( pStrDisplayName );
	DELETEP( pStrDescription );
	DELETEP( pStrIDListPath );
	DELETEP( pStrTargetPath );
	DELETEP( pStrWorkingDirectory );
	DELETEP( pIcon );

	if (pidl)
		CoTaskMemFree(pidl);
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
wxString debug_hex_dump_pidl(PIDLIST_ABSOLUTE pidl)
{
	wxString strResult;
	
	int k=0;
	unsigned short len;
	unsigned char * pc = (unsigned char *)pidl;

	len = *((unsigned short *)pc);
	while (len > 0)
	{
		int j;
		wxString strLine;
		strLine += wxString::Format(_T("\tid[%d]: [len %04x] ["), k, len);
		for (j=2; j<len; j++)
			strLine += wxString::Format(_T(" %02x"), pc[j]);
		strLine += wxString::Format(_T("]\n"));

		strResult += strLine;

		pc += len;
		len = *((unsigned short *)pc);
		k++;
	}

	return strResult;
}
#endif

//////////////////////////////////////////////////////////////////
		
static bool get_display_name_child(wxString * pStr,
								   PIDLIST_ABSOLUTE pidl_in)
{
	IShellFolder * psfParent = NULL;
	LPCITEMIDLIST pidlRelative = NULL;
	STRRET strret;
	wxChar * psz = NULL;
	HRESULT hres;

	hres = SHBindToParent(pidl_in, IID_IShellFolder, (void**)&psfParent, &pidlRelative);
	if (SUCCEEDED(hres))
	{
		psfParent->GetDisplayNameOf(pidlRelative, SHGDN_NORMAL, &strret);
		psfParent->Release();

		hres = StrRetToStr(&strret, pidlRelative, &psz);
		if (SUCCEEDED(hres))
		{
//#ifdef DEBUG
//			wxLogTrace(TRACE_LNK, _T("get_display_name_child: '%s' + '%s'"),
//					   pStr->wc_str(), psz);
//#endif
			pStr->Append(psz);
			CoTaskMemFree(psz);
			return true;
		}
	}

	return false;
}

static bool get_display_name_recurse(wxString * pStr,
									 PIDLIST_ABSOLUTE pidl_in)
{
//#ifdef DEBUG
//	wxLogTrace(TRACE_LNK, _T("get_display_name_recurse:\n%s"),
//			   debug_hex_dump_pidl(pidl_in));
//#endif

	if (ILGetNext(pidl_in))
	{
		// we have id[1] in <id[0]/id[1]..../id[n]>
		//
		// recurse to build display name for <id[0]/.../id[n-1]>
		// and append name for the terminal child <id[n]>.

		PIDLIST_ABSOLUTE pidl_copy = ILClone(pidl_in);
		ILRemoveLastID(pidl_copy);
		if (!get_display_name_recurse(pStr, pidl_copy))
			return false;

		pStr->Append(L"\\");

		if (!get_display_name_child(pStr, pidl_in))
			return false;
	}
	else
	{
		// we have <id[0]>

		if (!get_display_name_child(pStr, pidl_in))
			return false;
	}

	return true;
}
		
void util_shell_lnk::get_display_name(void)
{
	PIDLIST_ABSOLUTE pidl = (PIDLIST_ABSOLUTE)pVoid_pidl;

	wxString * pStr = new wxString();

	if (get_display_name_recurse(pStr, pidl))
		pStrDisplayName = pStr;
	else
		delete pStr;
}

//////////////////////////////////////////////////////////////////

/*static*/util_error util_shell_lnk::ctor(const wxString & pathname,
										  util_shell_lnk ** ppLnk)
{
	util_shell_lnk * pLnk = new util_shell_lnk();
	PIDLIST_ABSOLUTE pidl = NULL;
	
	util_error err;
	HRESULT hres;
	IShellLink * psl = NULL;
	IPersistFile * ppf = NULL;
	wxChar buf[MAX_PATH + 64*1024]; // paranoid
	WIN32_FIND_DATA data;
	
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (!SUCCEEDED(hres))
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (1) Cannot get shortcut info from '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
	if (!SUCCEEDED(hres))
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (2) Cannot get shortcut info from '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	hres = ppf->Load(pathname.wc_str(), 0);
	ppf->Release();
	if (!SUCCEEDED(hres))
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (3) Cannot get shortcut info from '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	//////////////////////////////////////////////////////////////////

	hres = psl->GetArguments(buf, NrElements(buf));
	if (hres == S_OK)
	{
		if (buf[0])
			pLnk->pStrArguments = new wxString(buf);
	}
	else
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (4) GetArguments() on shortcut '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	//////////////////////////////////////////////////////////////////

	hres = psl->GetDescription(buf, NrElements(buf));
	if (hres == S_OK)
	{
		if (buf[0])
			pLnk->pStrDescription = new wxString(buf);
	}
	else
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (5) GetDescription() on shortcut '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	//////////////////////////////////////////////////////////////////

	hres = psl->GetIDList(&pidl);
	pLnk->pVoid_pidl = (void *)pidl;

	if (hres == S_OK)
	{
		if (SHGetPathFromIDList(pidl, buf))
		{
			pLnk->pStrIDListPath = new wxString(buf);
		}
		else
		{
			// not part of the filesystem.
			// probably a printer or something.
			// compute synthetic display path.

			pLnk->get_display_name();
		}
	}
	else if (hres == S_FALSE)
	{
		// routine worked, but no PIDL available.
	}
	else
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (6) GetIDList() on shortcut '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	//////////////////////////////////////////////////////////////////

	hres = psl->GetPath(buf, NrElements(buf), &data, SLGP_RAWPATH);
	if (hres == S_OK)
	{
		if (buf[0])
			pLnk->pStrTargetPath = new wxString(buf);
	}
	else if (hres == S_FALSE)
	{
		// routine worked, but no path available.
		// this happens for links to printers, for example.
	}
	else
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (7) GetPath() on shortcut '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	//////////////////////////////////////////////////////////////////

	hres = psl->GetWorkingDirectory(buf, NrElements(buf));
	if (hres == S_OK)
	{
		if (buf[0])
			pLnk->pStrWorkingDirectory = new wxString(buf);
	}
	else
	{
		err.set( util_error::UE_OLE,
				 wxString::Format(_T("0x%08x; (8) GetWorkingDirectory() on shortcut '%s'."),
								  hres, pathname.wc_str()));
		goto fail;
	}

	//////////////////////////////////////////////////////////////////

	{
		SHFILEINFO shfi = {0};
		SHGetFileInfo((LPCTSTR)pidl, 0, &shfi, sizeof(shfi),
					  SHGFI_PIDL | SHGFI_ICON);
		wxLogTrace(TRACE_LNK,_T("GetIcon: [ndx %d][hIcon %p"),
				   shfi.iIcon, (void *)shfi.hIcon);

		pLnk->pIcon = new wxIcon();
		pLnk->pIcon->CreateFromHICON( shfi.hIcon );
	}

	//////////////////////////////////////////////////////////////////

#if 0 && defined(DEBUG)
	wxLogTrace(TRACE_LNK,
			   _T("%s"),
			   pLnk->debug_dump( pathname.wc_str() ));
#endif

	*ppLnk = pLnk;
	pLnk = NULL;

fail:
	if (psl)
		psl->Release();
	if (pLnk)
		delete pLnk;
	return err;
}

//////////////////////////////////////////////////////////////////

#if 0 // don't need this since we do raw binary compare now
static bool test_str(const wxString * ps1, const wxString * ps2)
{
	if (ps1)
		if (ps2)
			return ps1->IsSameAs(*ps2);
		else
			return false;
	else
		if (ps2)
			return false;
		else
			return true;
}

static bool compare_pidl(const PIDLIST_ABSOLUTE pidl_1,
						 const PIDLIST_ABSOLUTE pidl_2)
{
	unsigned char * pc_1 = (unsigned char *)pidl_1;
	unsigned char * pc_2 = (unsigned char *)pidl_2;

	while (1)
	{
		unsigned short len_1 = *((unsigned short *)pc_1);
		unsigned short len_2 = *((unsigned short *)pc_2);

		if (len_1 != len_2)
			return false;

		if (len_1 == 0)
			return true;
	
		for (int j=2; j<len_1; j++)
			if (pc_1[j] != pc_2[j])
				return false;

		pc_1 += len_1;
		pc_2 += len_2;
	}
}

static bool test_pidl(const PIDLIST_ABSOLUTE pidl_1,
					  const PIDLIST_ABSOLUTE pidl_2)
{
	if (pidl_1)
		if (pidl_2)
			return compare_pidl(pidl_1, pidl_2);
		else
			return false;
	else
		if (pidl_2)
			return false;
		else
			return true;
}

bool util_shell_lnk::equal(const util_shell_lnk * pLnkOther) const
{
	if (!test_str(this->pStrArguments, pLnkOther->pStrArguments))
		return false;
	if (!test_str(this->pStrDescription, pLnkOther->pStrDescription))
		return false;
	// TODO add icon path and index
	if (!test_str(this->pStrIDListPath, pLnkOther->pStrIDListPath))
		return false;
	if (!test_str(this->pStrTargetPath, pLnkOther->pStrTargetPath))
		return false;
	if (!test_str(this->pStrWorkingDirectory, pLnkOther->pStrWorkingDirectory))
		return false;

	if (!test_pidl((const PIDLIST_ABSOLUTE)this->pVoid_pidl,
				   (const PIDLIST_ABSOLUTE)pLnkOther->pVoid_pidl))
		return false;

	return true;
}
#endif//0

//////////////////////////////////////////////////////////////////

#if defined(DEBUG)
wxString util_shell_lnk::debug_dump(const wxString & strLabel) const
{

#define SorN(s) ((s) ? s->wc_str() : _T("(null)"))

	return wxString::Format(
		(L"DerefShellLink: '%s' ==>\n"
		 L"\t{\n"
		 L"\tArgs: '%s'\n"
		 L"\tDesc: '%s'\n"
		 L"\tDisp: '%s'\n"
		 L"\tPIDL: '%s'\n"
		 L"\tPath: '%s'\n"
		 L"\tWD..: '%s'\n"
		 L"\t}"),
		strLabel.wc_str(),
		SorN(this->pStrArguments),
		SorN(this->pStrDescription),
		SorN(this->pStrDisplayName),
		SorN(this->pStrIDListPath),
		SorN(this->pStrTargetPath),
		SorN(this->pStrWorkingDirectory));
}
#endif

//////////////////////////////////////////////////////////////////
#endif//__WXMSW__
