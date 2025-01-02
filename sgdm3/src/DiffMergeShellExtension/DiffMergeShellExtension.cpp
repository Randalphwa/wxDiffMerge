// DiffMergeShellExtension.cpp : Implementation of DLL Exports.
//////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "GEN_DiffMergeShellExtension.h"
#include "ShExt.h"

//////////////////////////////////////////////////////////////////

#if defined(WIN64)
#define GUID__AppId		"{ED49B78A-417D-45CF-8E5A-68F0EE3E3916}"	// In 3.3.0 was "{709C0C3F-A793-4163-A1FE-F6122C3DE78E}"
#else
#define GUID__AppId		"{0E1F6785-237D-48A3-8782-7DE558EA62E6}"	// In 3.3.0 was "{709C0C3F-A793-4163-A1FE-F6122C3DE78E}"
#endif

//////////////////////////////////////////////////////////////////

class CDiffMergeShellExtensionModule : public CAtlDllModuleT< CDiffMergeShellExtensionModule >
{
public :
	DECLARE_LIBID(LIBID_DiffMergeShellExtensionLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DIFFMERGESHELLEXTENSION, GUID__AppId )
};

CDiffMergeShellExtensionModule _AtlModule;

HINSTANCE g_hInstanceDll;

//////////////////////////////////////////////////////////////////

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//////////////////////////////////////////////////////////////////
// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	g_hInstanceDll = hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

//////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}

//////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

//////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	// TODO 2011/06/03 With the new WIX Windows Installer, letting
    // TODO            a DLL self-register is DEPRECATED.  The new
    // TODO            preferred way is to extract the list of GUIDs
    // TODO            from the DLL and build a .wxs script that
    // TODO            installs them and let the installer do the
    // TODO            work.  This allows the OS to better remove
    // TODO            or repair an installation.
    // 
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer(FALSE);
	return hr;
}

//////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer(FALSE);
	return hr;
}

//////////////////////////////////////////////////////////////////
// With the newer versions of Windows (such as Win7) someone needs
// to call SHChangeNotify(SHCNE_ASSOCCHANGED,...) after installing
// a new shell extension to get the system to flush the cache and
// rescan (rather than waiting for the next reboot).
//
// http://msdn.microsoft.com/en-us/library/cc144067%28v=vs.85%29.aspx
//
// It does not appear that WIX has a verb/routine to do this, so
// this entry point is installed as custom action in the script.
//
// This routine could be in it's own 1-function DLL rather than in
// the main DLL, but I didn't feel like creating one just for it
// (in both 32- and 64-bits).
//////////////////////////////////////////////////////////////////
// The full API for a WIX callout uses MSIHANDLE which you can use
// to call various MSI-related routines.  These require the MSI SDK
// which I don't feel like tracking down and installing (on every
// build machine) and since I'm not using any args, I don't really
// care anyway.
// 
// #include <msi.h>
// #include <msiquery.h>
//
// They also require msi.lib on the linker.

extern "C" UINT __stdcall Call_SHChangeNotify(HANDLE /*MSIHANDLE*/ /*hInstall*/)
{
#if 0
	MessageBox(NULL,_T("Hello from Call_SHChangeNotify..."),_T("Hello"),MB_OK);
#endif

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_DWORD, 0, 0);
	return 0;
}
