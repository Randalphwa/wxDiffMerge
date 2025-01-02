// cl_args.h
//////////////////////////////////////////////////////////////////

#ifndef H_CL_ARGS_H
#define H_CL_ARGS_H

//////////////////////////////////////////////////////////////////

class cl_args
{
public:
	cl_args(void);

	wxString	getFrameCaption(void) const;

public:
	bool		bParseErrors;		// true if there was a problem with the command line

	bool		bFolders;			// true if <pathname1> and <pathname2> refer to folders
	bool		bMerge;				// --merge
#ifdef FEATURE_SHEX
	bool		bShEx;				// --shex (force open-file/-folder interlude dialog to appear)
#endif

	// TODO --ro1 and --ro3 are obsolete.  they were needed with the native
	// TODO version of SGDM to prevent editing of the top-left and -right
	// TODO panels.
	//
	// TODO --ro2 should be renamed to just --ro.

	bool		bReadOnly;			// --ro2
	bool		bTitle[3];			// --title[123]
	bool		bResult;			// --result
	bool		bCaption;			// --caption

	int			nrParams;			// how many pathnames they gave

	wxString	title[3];			// --title[123]
	wxString 	result;				// --result
	wxString 	caption;			// --caption
	wxString	pathname[3];		// <pathname[123]>

	bool		bDumpDiffs;			// true if we should dump diffs to file rather than open window
	bool		bDumpUnified;		// use "Unified" output format rather than default format
	bool		bDumpIgnoreUnimportant;	// ingore unimportant changes when dumping diff output (yields non-standard result)

#if defined(DEBUG)
// See W2355 on whether HTML Export from the command line
// should be a free feature or in any way enabled.
#define FEATURE_CLI_HTML_EXPORT
#endif

#if defined(FEATURE_CLI_HTML_EXPORT)
	bool		bDumpHtml;			// dump diff output in HTML
	bool		bDumpHtmlIntraLine;	// include intra-line diffs in html output
	bool		bDumpHtmlSxS;		// side-by-side view
	de_display_ops dopsHtmlSxS;		// amount of content [all,ctx,dif] to show in side-by-side view
#endif

	wxString	diffOutput;			// destination pathname for dumping diff output

};

//////////////////////////////////////////////////////////////////

#endif//H_CL_ARGS_H
