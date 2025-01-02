// cl_args.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <GEN_BuildNumber.h>
#include <branding.h>
#include <cl_args.h>

//////////////////////////////////////////////////////////////////

cl_args::cl_args(void)
{
	bParseErrors = false;

	bFolders     = false;

	bMerge       = false;
	bReadOnly	 = false;	// value of /ro2 only since /ro1 and /ro3 are obsolete
	bTitle[0]    = false;
	bTitle[1]    = false;
	bTitle[2]    = false;
	bResult      = false;
	bCaption     = false;

	nrParams     = 0;

	bDumpDiffs   = false;
	bDumpUnified = false;
	bDumpIgnoreUnimportant = false;		// can cause our output to non-industry-standard

#if defined(FEATURE_CLI_HTML_EXPORT)
	bDumpHtml    = false;
	bDumpHtmlIntraLine = false;
	bDumpHtmlSxS = false;
	dopsHtmlSxS = DE_DOP_ALL;
#endif

#ifdef FEATURE_SHEX
	bShEx        = false;				// --shex (force open-file/-folder interlude dialog to appear)
#endif
}

//////////////////////////////////////////////////////////////////

wxString cl_args::getFrameCaption(void) const
{
	// when we are given a caption on the command line, we build a
	// string like "%s - SourceGear DiffMerge".  we put the caption
	// first so that it's easier to find windows in the task bar.
	// this should match the format in ViewFile{Diff,Merge}::setTopLevelWindowTitle()
	// in ViewFile.cpp and ViewFolder::setTopLevelWindowTitle() in ViewFolder.cpp.
	// NOTE: this layout is different from the native version of sgdm, so
	// NOTE: the CAPTION string given to us by Vault may need to be tweaked.
 
	if (bCaption && !caption.IsEmpty())
		return wxString::Format( VER_APP_TITLE_s, caption.wc_str() );
	else
		return VER_APP_TITLE;
}

