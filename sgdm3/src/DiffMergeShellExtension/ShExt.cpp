// ShExt.cpp : Implementation of CShExt

#include "stdafx.h"
#include "ShExt.h"

//////////////////////////////////////////////////////////////////
// With the new WIX installer (3.3.1 and beyond), we put copy of
// the pathname to the EXE in the Registry.

#define MY_HKLM_INSTALLER_KEY	_T("SOFTWARE\\SourceGear\\Common\\DiffMerge\\Installer")

//////////////////////////////////////////////////////////////////
// see http://msdn2.microsoft.com/en-us/library/aa969384.aspx
// see http://www.codeproject.com/shell/shellextguide1.asp

//////////////////////////////////////////////////////////////////
// There is a bug in Shell32.dll that causes menu items that we add
// to appear multiple times when we create a popup sub-menu.  This was
// described in item:12654 and in http://support.sourcegear.com/viewtopic.php?p=36658#36658
//
// A workaround was discussed in:  http://support.microsoft.com/kb/214477
// we have to use InsertMenuItem() for the popup, rather than the simpler
// InsertMenu() or AppendMenu(), so that we can supply an (unused) ID for
// the popup (so that when Explorer munges the menus, it knows that the
// popup belongs to us).

//////////////////////////////////////////////////////////////////
// we need to use the registry to communicate preference settings
// with diffmerge.  the keys/paths need to match what wxWidgets uses
// when wxConfig does its thing.

#define HKCU_REGISTRY_directory			_T("Software\\SourceGear\\SourceGear DiffMerge\\ShellExtension")

// we get hooked into / unhooked from Windows Explorer via RegSvr32.exe.
// but the user should be able to turn us on/off from the options dialog.
// that is, if they turn us off, we'll still be in the system, but won't
// populate the context menu.

#define REGISTRY_enabled			_T("Enabled")

// diffmerge command line arguments can be tweaked.  to whatever we find
// in the registry, we append the file/folder names.  that is, the
// value in the registry ***DOES NOT*** have the '%s %s %s' on the end.
// we use the '/shex' arg to force sgdm3 to go thru the open interlude
// dialog even when there are a complete set of files (so that they can
// check the ordering).

#define REGISTRY_clargs				_T("CLARGS")
#define REGISTRY_clargs_default		_T("/nosplash /shex")

//////////////////////////////////////////////////////////////////

extern HINSTANCE g_hInstanceDll;

//////////////////////////////////////////////////////////////////

CShExt::CShExt()
{
#if 0
	MessageBeep(MB_ICONASTERISK);
#endif
#if 0
	static int cnt = 0;
	TCHAR bufDebug[4000];
	wsprintf(bufDebug,_T("%5d: CShExt::CShExt()\n"),cnt++);
	OutputDebugString(bufDebug);
#endif
	m_hBmpIcon = LoadBitmap(g_hInstanceDll, MAKEINTRESOURCE(IDB_BITMAP1));

	_loadRegistry();

	m_bHaveExePath = _getExePath();
}

CShExt::~CShExt(void)
{
	if (m_hBmpIcon)
		DeleteObject(m_hBmpIcon);
}

//////////////////////////////////////////////////////////////////
// IShellExtInit
//////////////////////////////////////////////////////////////////

STDMETHODIMP CShExt::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
#if 0
	MessageBeep(MB_ICONASTERISK);
#endif
#if 0
	static int cnt = 0;
	TCHAR bufDebug[4000];
	wsprintf(bufDebug,_T("%5d: CShExt::Initialize()\n"),cnt++);
	OutputDebugString(bufDebug);
#endif
	if (m_dwFeatureEnabled == 0)	// if feature turned off, just be quiet.
		return E_INVALIDARG;

	if (!m_bHaveExePath)
		return E_INVALIDARG;

	FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };
	HDROP	  hDrop;

	// Look for CF_HDROP data in the data object.
	if (!pDataObj || FAILED(pDataObj->GetData(&fmt, &stg)))
		return E_INVALIDARG;

	// Get a pointer to the actual data.
	hDrop = (HDROP)GlobalLock(stg.hGlobal);
	if (!hDrop)
		return E_INVALIDARG;	// TODO should we call ReleaseStgMedium() first?

	HRESULT hr = _getFiles(hDrop);

	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);

	return hr;
}

//////////////////////////////////////////////////////////////////
// IContextMenu
//////////////////////////////////////////////////////////////////

STDMETHODIMP CShExt::QueryContextMenu(HMENU hMenu, UINT uMenuIndex,
									  UINT uidFirstCmd, UINT uidLastCmd,
									  UINT uFlags)
{
#if 0
	MessageBeep(MB_ICONASTERISK);
#endif
#if 0
	{
		static int cnt = 0;
		TCHAR bufDebug[4000];
		wsprintf(bufDebug,
				 _T("%5d: CShExt::QueryContextMenu(0x%08lx,%d,%d,%d,%x)\n"),
				 cnt++,
				 hMenu,uMenuIndex,uidFirstCmd,uidLastCmd,uFlags);
		MessageBox(NULL,bufDebug,_T("QueryContextMenu at top"),MB_ICONINFORMATION);
		//OutputDebugString(bufDebug);
	}
#endif

	// If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
	if (uFlags & CMF_DEFAULTONLY)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	// remember (first_uid,count) of what we put in menu.

	m_uidFirstCmd = uidFirstCmd;
	m_nrCmds = 0;

	InsertMenu(hMenu, uMenuIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

	// WARNING: the following must match exactly the menu decoding code.
	// WARNING: that is m_nrCmds becomes the menu id.

	// If gray-menu-item, just return without
	// putting anything in the menu.  Less clutter
	// for the user.  I'm going to expand the set
	// of reasons for graying (originally it was
	// when more than 2 items selected).  Eventually
	// I'd like to have it so that if they click on
	// one or two JPEG's (or some other binary files)
	// we don't even offer them a chance to diff them.

	if (m_bGrayMenuItem)
	{
		MENUITEMINFO mii;
		memset(&mii,0,sizeof(mii));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE | MIIM_CHECKMARKS;
		mii.fType = MFT_STRING;
		mii.wID = uidFirstCmd + m_nrCmds++;
		mii.dwTypeData = _T("DiffMerge");
		mii.cch = (UINT)wcslen(mii.dwTypeData);
		mii.fState = MFS_GRAYED;
		mii.hbmpChecked = m_hBmpIcon;
		mii.hbmpUnchecked = m_hBmpIcon;

		InsertMenuItem(hMenu, uMenuIndex++, true, &mii);
	}
	else if (m_bUseComparePairMenu)
	{
		// the compare-pair menu contains:
		//
		// [>] DiffMerge >
		//      [] Compare #1 with #2
		//      [] Compare #2 with #1
		// 
		// build pull-right pop up sub menu

		TCHAR buf[MAX_PATH*3];

		HMENU hMenuPopup = CreatePopupMenu();
		UINT uMenuPopupIndex = 0;

		swprintf_s(buf,NrElements(buf),_T("Compare \"%s\" with \"%s\""), m_szFilename[0],m_szFilename[1]);
		InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++,buf);		// 0

		swprintf_s(buf,NrElements(buf),_T("Compare \"%s\" with \"%s\""), m_szFilename[1],m_szFilename[0]);
		InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++,buf);		// 1
		
		// hang pop up menu off of main context menu

		MENUITEMINFO mii;
		memset(&mii,0,sizeof(mii));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_SUBMENU | MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE | MIIM_CHECKMARKS;
		mii.fType = MFT_STRING;
		mii.wID = uidFirstCmd + m_nrCmds++;
		mii.hSubMenu = hMenuPopup;
		mii.dwTypeData = _T("&DiffMerge");
		mii.cch = (UINT)wcslen(mii.dwTypeData);
		mii.fState = MFS_ENABLED;
		mii.hbmpChecked = m_hBmpIcon;
		mii.hbmpUnchecked = m_hBmpIcon;

		InsertMenuItem(hMenu, uMenuIndex++, true, &mii);
		// J5173: DestroyMenu(hMenuPopup);
	}
	else
	{
		// the advanced menu contains:
		//
		// [>] DiffMerge >
		//      [] Open with DiffMerge
		//      --------------------------
		//      [] Remember File
		//      [] Clear List
		//      --------------------------
		//      [] Compare %s with >
		//          [] mru #1
		//          [] mru ...
		//
		// build pull-right pop up sub menu

#if 0
	MessageBeep(MB_ICONASTERISK);
#endif

		HMENU hMenuPopup = CreatePopupMenu();
		UINT uMenuPopupIndex = 0;
		
		InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++,
				   _T("&Open with DiffMerge"));			// 0

		InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

		if (m_bMruUseFiles)
		{
			InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++, _T("&Remember File"));	// 1

			if (m_nrMRU_Files > 0)
			{
				TCHAR buf[MAX_PATH*3];

				// when we have at least one item in the list, add command to
				// clear the list.

				InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++, _T("Clear File List"));	// 2
				InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

				// create sub-popup with the mru list.

				HMENU hMenuPopupList_1 = CreatePopupMenu();
				UINT uMenuPopupListIndex_1 = 0;

				for (int kMRU=0; kMRU<(int)m_nrMRU_Files; kMRU++)
				{
					size_t lenBuf = wcslen(m_szMRU_Files[kMRU]) + 100;
					TCHAR * pbuf = (TCHAR *)calloc(lenBuf,sizeof(TCHAR));
					swprintf_s(pbuf,lenBuf,_T("&%d %s"),kMRU+1,m_szMRU_Files[kMRU]);
					InsertMenu(hMenuPopupList_1, uMenuPopupListIndex_1++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++, pbuf);	// 3 .. 3+n
					free(pbuf);
				}

				// insert mru sub-popup list into popup list.

				swprintf_s(buf,NrElements(buf),_T("Compare \"%s\" with"),m_szFilename[0]);

				MENUITEMINFO mii_1;
				memset(&mii_1,0,sizeof(mii_1));
				mii_1.cbSize = sizeof(MENUITEMINFO);
				mii_1.fMask = MIIM_SUBMENU | MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
				mii_1.fType = MFT_STRING;
				mii_1.wID = uidFirstCmd + m_nrCmds++;
				mii_1.hSubMenu = hMenuPopupList_1;
				mii_1.dwTypeData = buf;
				mii_1.cch = (UINT)wcslen(mii_1.dwTypeData);
				mii_1.fState = MFS_ENABLED;

				InsertMenuItem(hMenuPopup, uMenuPopupIndex++, true, &mii_1);
				// J5173: DestroyMenu(hMenuPopupList_1);
			}
		}
		else
		{
			InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++, _T("&Remember Folder"));		// 1

			if (m_nrMRU_Folders > 0)
			{
				TCHAR buf[MAX_PATH*3];

				// when we have at least one item in the list, add command to
				// clear the list.

				InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++, _T("Clear Folder List"));	// 2
				InsertMenu(hMenuPopup, uMenuPopupIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

				// create sub-popup with the mru list.

				HMENU hMenuPopupList_1 = CreatePopupMenu();
				UINT uMenuPopupListIndex_1 = 0;

				for (int kMRU=0; kMRU<(int)m_nrMRU_Folders; kMRU++)
				{
					size_t lenBuf = wcslen(m_szMRU_Folders[kMRU]) + 100;
					TCHAR * pbuf = (TCHAR *)calloc(lenBuf,sizeof(TCHAR));
					swprintf_s(pbuf,lenBuf,_T("&%d %s"),kMRU+1,m_szMRU_Folders[kMRU]);
					InsertMenu(hMenuPopupList_1, uMenuPopupListIndex_1++, MF_BYPOSITION, uidFirstCmd + m_nrCmds++, pbuf);		// 3 .. 3+n
					free(pbuf);
				}

				// insert mru sub-popup list into popup list.

				swprintf_s(buf,NrElements(buf),_T("Compare \"%s\" with"),m_szFilename[0]);

				MENUITEMINFO mii_1;
				memset(&mii_1,0,sizeof(mii_1));
				mii_1.cbSize = sizeof(MENUITEMINFO);
				mii_1.fMask = MIIM_SUBMENU | MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
				mii_1.fType = MFT_STRING;
				mii_1.wID = uidFirstCmd + m_nrCmds++;
				mii_1.hSubMenu = hMenuPopupList_1;
				mii_1.dwTypeData = buf;
				mii_1.cch = (UINT)wcslen(mii_1.dwTypeData);
				mii_1.fState = MFS_ENABLED;

				InsertMenuItem(hMenuPopup, uMenuPopupIndex++, true, &mii_1);
				// J5173: DestroyMenu(hMenuPopupList_1);
			}
		}

		// hang pop up menu off of main context menu

		MENUITEMINFO mii;
		memset(&mii,0,sizeof(mii));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_SUBMENU | MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_STATE | MIIM_CHECKMARKS;
		mii.fType = MFT_STRING;
		mii.wID = uidFirstCmd + m_nrCmds++;
		mii.hSubMenu = hMenuPopup;
		mii.dwTypeData = _T("&DiffMerge");
		mii.cch = (UINT)wcslen(mii.dwTypeData);
		mii.fState = MFS_ENABLED;
		mii.hbmpChecked = m_hBmpIcon;
		mii.hbmpUnchecked = m_hBmpIcon;

		InsertMenuItem(hMenu, uMenuIndex++, true, &mii);
		// J5173: DestroyMenu(hMenuPopup);
	}
	
#if 0
	{
		static int cnt = 0;
		TCHAR bufDebug[4000];
		wsprintf(bufDebug,
				 _T("%5d: CShExt::QueryContextMenu(0x%08lx,%d,%d,%d,%x) [nrCmds %d]\n"),
				 cnt++,
				 hMenu,uMenuIndex,uidFirstCmd,uidLastCmd,uFlags,
				 m_nrCmds);
		MessageBox(NULL,bufDebug,_T("QueryContextMenu at bottom"),MB_ICONINFORMATION);
		//OutputDebugString(bufDebug);
	}
#endif

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, m_nrCmds);
}

STDMETHODIMP CShExt::GetCommandString(UINT_PTR uidCmdOffset, UINT uFlags, UINT * pwReserved,
									  LPSTR pszName, UINT cchMax)
{
#if 0
	TCHAR bufDebug[4000];
	wsprintf(bufDebug,_T("CShExt::GetCommandString(%d / %d,%d)\n"),
			 uidCmdOffset,m_nrCmds,
			 uFlags);
	OutputDebugString(bufDebug);
#endif

	if (uidCmdOffset >= m_nrCmds)
		return E_INVALIDARG;

	// If Explorer is asking for a help string, copy our string into the
	// supplied buffer.

	if (uFlags & GCS_HELPTEXT)
	{
		// WARNING: the following must match exactly the menu construction code.
		// WARNING: that is the lpVerb value is the menu id.

		if (m_bGrayMenuItem)
		{
			_getCommandString(uFlags,_T(" "),pszName,cchMax);
			return S_OK;
		}
		else if (m_bUseComparePairMenu)
		{
			switch (uidCmdOffset)
			{
			case 0:
			case 1:		// compare now
				_getCommandString(uFlags,_T("Compare the Selected Items with SourceGear DiffMerge"),pszName,cchMax);
				return S_OK;
			
			default:
				_getCommandString(uFlags,_T(" "),pszName,cchMax);
				return S_OK;
			}
		}
		else	// advanced menu
		{
			switch (uidCmdOffset)
			{
			case 0:		// 1 file/folder open (always present)
				_getCommandString(uFlags,_T("Open the Selected Item with SourceGear DiffMerge"),pszName,cchMax);
				return S_OK;

			case 1:		// remember this (always present)
				_getCommandString(uFlags,_T("Remember this Item for a Future Compare"),pszName,cchMax);
				return S_OK;

			case 2:		// clear the list (conditionally present)
				if (m_bMruUseFiles)
				{
					if (m_nrMRU_Files > 0)
						_getCommandString(uFlags,_T("Clear the List of Remembered Files"),pszName,cchMax);
					else
						_getCommandString(uFlags,_T(" "),pszName,cchMax);
				}
				else
				{
					if (m_nrMRU_Folders > 0)
						_getCommandString(uFlags,_T("Clear the List of Remembered Folders"),pszName,cchMax);
					else
						_getCommandString(uFlags,_T(" "),pszName,cchMax);
				}
				return S_OK;

			default:	// mru item = cmd - 3  (conditionally present)
				{
					UINT_PTR kMRU = uidCmdOffset - 3;
					if (m_bMruUseFiles)
					{
						if (kMRU < m_nrMRU_Files)
							_getCommandString(uFlags,_T("Compare the Selected and Remembered Files"),pszName,cchMax);
						else
							_getCommandString(uFlags,_T(" "),pszName,cchMax);
					}
					else
					{
						if (kMRU < m_nrMRU_Folders)
							_getCommandString(uFlags,_T("Compare the Selected and Remembered Folders"),pszName,cchMax);
						else
							_getCommandString(uFlags,_T(" "),pszName,cchMax);
					}
					return S_OK;
				}
			}
		}
	}

	return E_INVALIDARG;
}

STDMETHODIMP CShExt::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo)
{
	if (!pCmdInfo)
		return E_INVALIDARG;

	//BOOL bExtended = (pCmdInfo->cbSize == sizeof(CMINVOKECOMMANDINFOEX));
	//BOOL bUnicode = (bExtended && (pCmdInfo->fMask & CMIC_MASK_UNICODE));

	// there are 2 types of commands: [1] verb strings (ansi or unicode);
	// and [2] uid offset.
	//
	// we ignore verb commands.

	if (HIWORD(pCmdInfo->lpVerb) != 0)
		return E_INVALIDARG;

	// WARNING: the following must match exactly the menu construction code.
	// WARNING: that is the lpVerb value is the menu id.

	if (m_bGrayMenuItem)
	{
	}
	else if (m_bUseComparePairMenu)
	{
		switch (LOWORD(pCmdInfo->lpVerb))
		{
		case 0:		// compare now %0 vs %1
			return _invokeDiffMerge(pCmdInfo->hwnd, m_szPathname[0], m_szPathname[1]);

		case 1:		// compare now %1 vs %0
			return _invokeDiffMerge(pCmdInfo->hwnd, m_szPathname[1], m_szPathname[0]);

		default:	// should not happen
			break;
		}
	}
	else		// advanced menu
	{
		switch (LOWORD(pCmdInfo->lpVerb))
		{
		case 0:		// 1 file/folder open
			return _invokeDiffMerge(pCmdInfo->hwnd, m_szPathname[0], NULL);

		case 1:		// remember this
			return _rememberThis(pCmdInfo->hwnd);

		case 2:		// clear list
			return _clear_list();

		default:	// mru item = verb - 3
			{
				// compare the single selected item with a single mru item.
				// the order that we put the files/folders on the command line
				// is a bit of a crapshoot.  there was a request on the forum
				// to do this by mtime (they may have only been speaking about
				// the 2 selected file case, but i'm going to try it here).

				struct __stat64 st_a, st_b;

				UINT kMRU = LOWORD(pCmdInfo->lpVerb) - 3;
				if (m_bMruUseFiles)
				{
					if (kMRU < m_nrMRU_Files)
					{
						int result_a = _wstat64(m_szMRU_Files[kMRU],&st_a);
						int result_b = _wstat64(m_szPathname[0],&st_b);
						if ((result_a != 0) || (result_b != 0))
							return E_INVALIDARG;

						if (st_a.st_mtime <= st_b.st_mtime)
							return _invokeDiffMerge(pCmdInfo->hwnd, m_szMRU_Files[kMRU], m_szPathname[0]);
						else
							return _invokeDiffMerge(pCmdInfo->hwnd, m_szPathname[0], m_szMRU_Files[kMRU]);
					}
				}
				else
				{
					if (kMRU < m_nrMRU_Folders)
					{
						int result_a = _wstat64(m_szMRU_Folders[kMRU],&st_a);
						int result_b = _wstat64(m_szPathname[0],&st_b);
						if ((result_a != 0) || (result_b != 0))
							return E_INVALIDARG;

						if (st_a.st_mtime <= st_b.st_mtime)
							return _invokeDiffMerge(pCmdInfo->hwnd, m_szMRU_Folders[kMRU], m_szPathname[0]);
						else
							return _invokeDiffMerge(pCmdInfo->hwnd, m_szPathname[0], m_szMRU_Folders[kMRU]);
					}
				}
			}
		}
	}

	return E_INVALIDARG;
}

//////////////////////////////////////////////////////////////////

HRESULT CShExt::_getFiles(HDROP hDrop)
{
	// fetch the pathnames (upto the number that we support).
	// if we can't get any of them, bail.
	//
	// warning: these come in a somewhat random order from the
	//          shell, so we can't tell if there is a preferred
	//          order for sending to diffmerge.  this causes 2
	//          problems: [1] we arbitrarily clip the list (if
	//          they've selected 20 or 30 files, for example);
	//          and [2] the edit panel needs to be the 2nd file.

	for (UINT k=0; k<MAX_PATHNAMES; k++)
	{
		memset(m_szPathname[k],0,MAX_PATH+1);
		memset(m_szFilename[k],0,MAX_PATH+1);
	}
	
	// Sanity check - make sure there is at least one filename.
	UINT uNumFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (uNumFiles == 0)
		return E_INVALIDARG;

	// stat the files/folders and see if we have a consistent set.

	int cFiles = 0;
	int cDirs  = 0;
	int cOther = 0;
	for (UINT k=0; k<uNumFiles; k++)
	{
		TCHAR buf[MAX_PATH+1];
		memset(buf,0,MAX_PATH+1);

		if (DragQueryFile(hDrop, k, buf, MAX_PATH) == 0)
			return E_INVALIDARG;

		struct __stat64 stat;
		int result = _wstat64(buf,&stat);	// use 64x64 version, but it doesn't really matter for our needs here
		if (result != 0)
			return E_INVALIDARG;

		if (stat.st_mode & _S_IFDIR)
			cDirs++;
		else if (stat.st_mode & _S_IFREG)
			cFiles++;
		else
			cOther++;
	}

	// we omit the menu item when we have:
	// [] more than 2 items
	// [] a mixture of files and folders
	// [] other types of items
	//
	// TODO add stuff here to optionally gray the menu when
	// TODO [] one or more binary files (user-supplied suffix list)
	// TODO [] different types of files (different suffixes)

	m_bGrayMenuItem = (   (cDirs > 2)
					   || (cFiles > 2)
					   || ( (cDirs > 0) && (cFiles > 0) )
					   || (cOther > 0) );

	// when exactly 2 files or 2 folders given, we use the compare-pair
	// menu with 'compare with...'
	// 
	// when 1 file/folder given, we use the advanced menu with choices.

	m_bUseComparePairMenu = (cDirs > 1) || (cFiles > 1);

	// we need to determine if we should use the file- or folder- mru list.

	m_bMruUseFiles = (cFiles > 0);

	// copy first few pathnames to our array of pathnames.
	// also extract the corresponding filenames.

	m_nrPathnames = MyMin(MAX_PATHNAMES,uNumFiles);
	for (UINT k=0; k<m_nrPathnames; k++)
	{
		if (DragQueryFile(hDrop, k, m_szPathname[k], MAX_PATH) == 0)
			return E_INVALIDARG;

		// extract filename from pathname.  if something
		// goofy happens, just use whole pathname.

		const TCHAR * pLastSlash = wcsrchr(m_szPathname[k],L'\\');
		if (!pLastSlash)
			pLastSlash = wcsrchr(m_szPathname[k],L'/');
		if (pLastSlash)
			wcscpy_s(m_szFilename[k],MAX_PATH,pLastSlash+1);
		else
			wcscpy_s(m_szFilename[k],MAX_PATH,m_szPathname[k]);
	}

	return S_OK;
}

void CShExt::_getCommandString(UINT uFlags, LPCTSTR szInput, LPSTR pszName, UINT cchMax)
{
	USES_CONVERSION;

//	OutputDebugStringW(szInput);
//	OutputDebugStringW(_T("\r\n"));

	if (uFlags & GCS_UNICODE)
		lstrcpynW((LPWSTR)pszName,T2CW(szInput),cchMax);
	else
		lstrcpynA(pszName,T2CA(szInput),cchMax);
}

void CShExt::_loadRegistry(void)
{
	m_dwFeatureEnabled = 1;		// assume enabled unless register explictly says enabled=0
	m_nrMRU_Files = 0;			// assume no files in MRU until we read some
	m_nrMRU_Folders = 0;		// assume no files in MRU until we read some

	memset(m_szCommandLineArgs,0,MAX_PATH+1);
	lstrcpynW(m_szCommandLineArgs,REGISTRY_clargs_default,MAX_PATH+1);

	CRegKey reg;
	LONG lRet;

	lRet = reg.Open(HKEY_CURRENT_USER, HKCU_REGISTRY_directory, KEY_READ);
	if (lRet != ERROR_SUCCESS)
		return;
	
	lRet = reg.QueryDWORDValue(REGISTRY_enabled, m_dwFeatureEnabled);

	ULONG lenCLArgsBuf = MAX_PATH+1;
	lRet = reg.QueryStringValue(REGISTRY_clargs, m_szCommandLineArgs, &lenCLArgsBuf);
	if (lRet != ERROR_SUCCESS)
		lstrcpynW(m_szCommandLineArgs,REGISTRY_clargs_default,MAX_PATH+1);

	// TODO stat() the pathnames that we pull from the
	// TODO registry and verify that they still exist
	// TODO on disk.  if not, either remove them from
	// TODO the mru list in memory --or-- gray the corresponding
	// TODO item in the sub-pop-up.

	for (int kMruFiles=0; kMruFiles<MAX_MRU; kMruFiles++)
	{
		ULONG lenMruBuf = MAX_PATH+1;
		memset(m_szMRU_Files[kMruFiles],0,lenMruBuf);

		TCHAR bufKey[100];
		swprintf_s(bufKey,NrElements(bufKey),_T("mru_file_%d"),kMruFiles);

		lRet = reg.QueryStringValue(bufKey,m_szMRU_Files[kMruFiles],&lenMruBuf);
		if (lRet != ERROR_SUCCESS)
			break;

		m_nrMRU_Files++;
	}

	for (int kMruFolders=0; kMruFolders<MAX_MRU; kMruFolders++)
	{
		ULONG lenMruBuf = MAX_PATH+1;
		memset(m_szMRU_Folders[kMruFolders],0,lenMruBuf);

		TCHAR bufKey[100];
		swprintf_s(bufKey,NrElements(bufKey),_T("mru_folder_%d"),kMruFolders);

		lRet = reg.QueryStringValue(bufKey,m_szMRU_Folders[kMruFolders],&lenMruBuf);
		if (lRet != ERROR_SUCCESS)
			break;

		m_nrMRU_Folders++;
	}

	reg.Close();
}

bool CShExt::_statExePath(void)
{
	// stat() the exe path and verify that it exists and we can execute it.

	struct __stat64 stat;
	int result = _wstat64(m_szExePath,&stat);
	if (result != 0)
		return false;

	if ((stat.st_mode & _S_IFREG) == 0)
		return false;

	// TODO check for x access to file
	// NOTE this would be bogus since Windows doesn't have an x bit
	// NOTE and only fakes it for their posix wrapper.

	return true;
}

HRESULT CShExt::_invokeDiffMerge(HWND hWnd, const TCHAR * szPathname1, const TCHAR * szPathname2)
{
	size_t lenBuf = (wcslen(m_szExePath) + 3		// +3 for surrounding quotes and trailing space
					 + wcslen(m_szCommandLineArgs)
					 + wcslen(szPathname1) + 3		// +3 for leading space and surrounding quotes
					 + ((szPathname2) ? wcslen(szPathname2) : 0) + 3
					 + 10);

	TCHAR * pbuf = (TCHAR *)calloc(lenBuf, sizeof(TCHAR));
	if (!pbuf)
		return E_INVALIDARG;

	// "$exe" $args "$path1" ["$path2"]

	wcscat_s(pbuf,lenBuf,_T("\""));
	wcscat_s(pbuf,lenBuf,m_szExePath);
	wcscat_s(pbuf,lenBuf,_T("\" "));
	
	wcscat_s(pbuf,lenBuf,m_szCommandLineArgs);

	wcscat_s(pbuf,lenBuf,_T(" \""));
	wcscat_s(pbuf,lenBuf,szPathname1);
	wcscat_s(pbuf,lenBuf,_T("\""));

	if (szPathname2)
	{
		wcscat_s(pbuf,lenBuf,_T(" \""));
		wcscat_s(pbuf,lenBuf,szPathname2);
		wcscat_s(pbuf,lenBuf,_T("\""));
	}

	//MessageBox(hWnd,pbuf,m_szExePath,MB_ICONINFORMATION);

	STARTUPINFO si;
	memset(&si,0,sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	memset(&pi,0,sizeof(pi));

	BOOL bResult = CreateProcess(m_szExePath, pbuf,		// exe, clargs
								 NULL,NULL,				// procattrs, threadattrs,
								 FALSE,					// inherit handles
								 CREATE_UNICODE_ENVIRONMENT|CREATE_DEFAULT_ERROR_MODE,
								 NULL,					// environment
								 NULL,					// initial directory
								 &si,					// startup info
								 &pi);

	free(pbuf);

	if (!bResult)
		return E_INVALIDARG;

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return S_OK;
}

bool CShExt::_getExePath_FromRegistry(void)
{
	// Try to get the EXE path from what the WIX installer put in the registry.

	memset(m_szExePath,0,MAX_PATH+1);

	CRegKey reg;
	LONG lResult = reg.Open(HKEY_LOCAL_MACHINE, MY_HKLM_INSTALLER_KEY, KEY_READ);
	if (lResult != ERROR_SUCCESS)
		return false;
	
	ULONG len = MAX_PATH+1;
	lResult = reg.QueryStringValue(_T("Location"), m_szExePath, &len);

#if 0
	if (lResult == ERROR_SUCCESS)
		MessageBox(NULL,m_szExePath,_T("Found exe path in registry"),MB_OK);
#endif

	return (lResult == ERROR_SUCCESS);
}

bool CShExt::_getExePath(void)
{
	// Try to get the EXE path from what the WIX installer put in the registry.
	// If that fails, fall back and look in the DLLs directory.

	if (_getExePath_FromRegistry())
		return true;

	memset(m_szExePath,0,MAX_PATH+1);

	// find the .EXE for DiffMerge.  it should be in the same directory as this .DLL.
	// it may be named DiffMerge.exe or sgdm.exe or sgdm3.exe depending upon which
	// version (standalone or bundled) we were shipped with.

	// get the pathname of our .DLL.

	DWORD dwLen = GetModuleFileName(g_hInstanceDll,m_szExePath,MAX_PATH+1);
	if (dwLen == 0)
		goto Failed;

	// truncate the .DLL filename

	while (dwLen > 0)
	{
		if ((m_szExePath[dwLen-1] == L'\\') || (m_szExePath[dwLen-1] == L'/'))
			break;

		m_szExePath[dwLen-1] = 0;
		dwLen--;
	}
	if (dwLen == 0)
		goto Failed;

	// try each of the choices in turn.  make sgdm.exe first because
	// the new 3.3.1 installer renames the exe to avoid collisions with
	// a diffmerge.exe that MSFT put in Common7/IDE.

	static TCHAR * aszChoices[] = { L"sgdm.exe", L"sgdm3.exe", L"DiffMerge.exe" };
	int nrChoices = NrElements(aszChoices);
	for (int k=0; k<nrChoices; k++)
	{
		wcscat_s(m_szExePath,MAX_PATH+1,aszChoices[k]);

		if (_statExePath())
			return true;

		m_szExePath[dwLen] = 0;
	}

Failed:
	m_szExePath[0] = 0;
	return false;
}

//////////////////////////////////////////////////////////////////

HRESULT CShExt::_rememberThis(HWND hWnd)
{
	//MessageBox(hWnd,_T("In rememberThis 2"),m_szExePath,MB_ICONINFORMATION);

	CRegKey reg;
	LONG lRet;

	// save the current selection as mru0 and then push
	// the others down one in the list.

	lRet = reg.Open(HKEY_CURRENT_USER, HKCU_REGISTRY_directory, KEY_READ|KEY_WRITE);
	if (lRet != ERROR_SUCCESS)
	{
		lRet = reg.Create(HKEY_CURRENT_USER, HKCU_REGISTRY_directory);
		if (lRet != ERROR_SUCCESS)
		{
			MessageBox(hWnd,_T("Could not open HKCU_REGISTRY_directory"),m_szExePath,MB_ICONINFORMATION);
			return E_INVALIDARG;	// TODO pick a better error code
		}
	}
	
	if (m_bMruUseFiles)
		lRet = reg.SetStringValue(_T("mru_file_0"),m_szPathname[0]);
	else
		lRet = reg.SetStringValue(_T("mru_folder_0"),m_szPathname[0]);
	if (lRet != ERROR_SUCCESS)
	{
		//MessageBox(hWnd,_T("Could not write mru0"),m_szExePath,MB_ICONINFORMATION);
		return E_INVALIDARG;	// TODO pick a better error code
	}

	if (m_bMruUseFiles)
	{
		int limitMRU = MyMin(MAX_MRU-1, m_nrMRU_Files);
		for (int kMRU=0; kMRU<limitMRU; kMRU++)
		{
			TCHAR bufKey[100];
			swprintf_s(bufKey,NrElements(bufKey),_T("mru_file_%d"), kMRU+1);
			lRet = reg.SetStringValue(bufKey, m_szMRU_Files[kMRU]);

			if (lRet != ERROR_SUCCESS)
			{
				//MessageBox(hWnd,_T("Could not write mru_n"),m_szExePath,MB_ICONINFORMATION);
				return E_INVALIDARG;	// TODO pick a better error code
			}
		}
	}
	else
	{
		int limitMRU = MyMin(MAX_MRU-1, m_nrMRU_Folders);
		for (int kMRU=0; kMRU<limitMRU; kMRU++)
		{
			TCHAR bufKey[100];
			swprintf_s(bufKey,NrElements(bufKey),_T("mru_folder_%d"), kMRU+1);
			lRet = reg.SetStringValue(bufKey, m_szMRU_Folders[kMRU]);

			if (lRet != ERROR_SUCCESS)
			{
				//MessageBox(hWnd,_T("Could not write mru_n"),m_szExePath,MB_ICONINFORMATION);
				return E_INVALIDARG;	// TODO pick a better error code
			}
		}
	}
	
	reg.Close();

	return S_OK;
}

HRESULT CShExt::_clear_list(void)
{
	// erase the contents of the file or folder list.

	CRegKey reg;
	LONG lRet;
	const TCHAR * szKeyPattern = ((m_bMruUseFiles) ? _T("mru_file_%d") : _T("mru_folder_%d"));

	lRet = reg.Open(HKEY_CURRENT_USER, HKCU_REGISTRY_directory, KEY_READ|KEY_WRITE);
	if (lRet != ERROR_SUCCESS)
		return E_INVALIDARG;
	
	int limitMRU = MAX_MRU-1;	// delete all keys possible, not just what we have in the array.
	for (int kMRU=0; kMRU<limitMRU; kMRU++)
	{
		TCHAR bufKey[100];
		swprintf_s(bufKey,NrElements(bufKey),szKeyPattern,kMRU);

		// delete the value -- the Win32 API terminology sucks.
		// a "value" is a "name = (string)value" thing that's
		// inside a "key" which is a directory of sorts.
		//
		// we ignore the error because it may not exist and we
		// don't care.

		(void)reg.DeleteValue(bufKey);
	}

	reg.Close();

	return S_OK;
}
