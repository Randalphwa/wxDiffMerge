// ShExt.h : Declaration of the CShExt
//////////////////////////////////////////////////////////////////

#pragma once
#include "resource.h"       // main symbols

#include "GEN_DiffMergeShellExtension.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

//////////////////////////////////////////////////////////////////
// allow upto 2 pathnames (we don't currently support 3-way merge because
// we receive files from Explorer in a random order making this context
// menu interface too complicated).

#define MAX_PATHNAMES		2

#ifndef MyMax
#	define MyMax(a,b)		(((a)>(b)) ? (a) : (b))
#	define MyMin(a,b)		(((a)<(b)) ? (a) : (b))
#endif

#ifndef NrElements
#	define NrElements(a)	((sizeof(a))/(sizeof(a[0])))
#endif

// remember upto 5 pathnames in the MRU list

#define MAX_MRU				5

//////////////////////////////////////////////////////////////////
// CShExt

class ATL_NO_VTABLE CShExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CShExt, &CLSID_ShExt>,
	public IShellExtInit,
    public IContextMenu
{
public:
	CShExt();
	~CShExt(void);

DECLARE_REGISTRY_RESOURCEID(IDR_SHEXT)

DECLARE_NOT_AGGREGATABLE(CShExt)

BEGIN_COM_MAP(CShExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	// IShellExtInit
	STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    // IContextMenu
    STDMETHODIMP GetCommandString(UINT_PTR, UINT, UINT*, LPSTR, UINT);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO);
    STDMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT);

protected:
	HBITMAP				m_hBmpIcon;

	DWORD				m_dwFeatureEnabled;
	TCHAR				m_szExePath[MAX_PATH+1];
	TCHAR				m_szCommandLineArgs[MAX_PATH+1];	// for /x /ro2 switches

	bool				m_bGrayMenuItem;
	bool				m_bHaveExePath;
	bool				m_bUseComparePairMenu;
	bool				m_bMruUseFiles;

	UINT				m_uidFirstCmd;		// uid of first menu item we installed
	UINT				m_nrCmds;			// nr of items we put in menu

	UINT				m_nrPathnames;
	TCHAR				m_szPathname[MAX_PATHNAMES][MAX_PATH+1];
	TCHAR				m_szFilename[MAX_PATHNAMES][MAX_PATH+1];

	UINT				m_nrMRU_Files;
	TCHAR				m_szMRU_Files[MAX_MRU][MAX_PATH+1];

	UINT				m_nrMRU_Folders;
	TCHAR				m_szMRU_Folders[MAX_MRU][MAX_PATH+1];

private:
	HRESULT				_getFiles(HDROP hDrop);
	void				_getCommandString(UINT uFlags, LPCTSTR szInput, LPSTR pszName, UINT cchMax);
	void				_loadRegistry(void);
	bool				_statExePath(void);
	HRESULT				_invokeDiffMerge(HWND hWnd, const TCHAR * szPathname1, const TCHAR * szPathname2);
	bool				_getExePath_FromRegistry(void);
	bool				_getExePath(void);
	HRESULT				_rememberThis(HWND hWnd);
	HRESULT				_clear_list(void);
};

OBJECT_ENTRY_AUTO(__uuidof(ShExt), CShExt)
