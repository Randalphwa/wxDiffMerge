// gui_app__cl_args.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

bool gui_app::_OnInit_cl_args(void)
{
	// let base class do its thing -- this also causes
	// OnInitCmdLine()...OnCmdLineParsed() sequence to
	// happen.
	// 
	// but first, install our custom wxMessageOutput
	// handler (that captures all output into a wxString)
	// because wxCmdLineParser sends errors to the current
	// wxMessageOutput handler rather than logging errors.
	// on win32, this causes invalid arg messages to appear
	// as 'informational (i) MB_ICONINFORMATION' message
	// boxes rather than as an 'error <x> MB_ICONERROR'
	// box.
	// on gtk, this causes invalid arg messags to appear
	// on stderr without the usual 'Error: ' prefix.
	// 
	// so, we install our custom wxMessageOutput handler
	// here and then catch OnCmdLineError and display
	// messages like we do for the errors we catch in
	// OnCmdLineParsed().  this way all error messages
	// appear consistently.
	//
	// As a side effect of this, we also need to catch
	// OnCmdLineHelp() to actually display the help.

	wxMessageOutput * moOld = wxMessageOutput::Set( new MessageOutputString() );
	bool bContinue = wxApp::OnInit();
	delete wxMessageOutput::Set(moOld);

	return bContinue;
}

//////////////////////////////////////////////////////////////////

void gui_app::OnInitCmdLine(wxCmdLineParser & parser)
{
	// let base class add '-h' and any other generic options.

	wxApp::OnInitCmdLine(parser);

	// we define define a hybrid of short and long forms.
	// if we only define {...,"c","caption",...} we get support
	// for -c or --caption (on windows, this is /c or --caption)
	// we DO NOT get -caption (on windows, /caption) unless we
	// also define a {...,"caption",NULL,...}.  so, we define
	// both.  but this has the side-effect of making the auto-
	// generated usage a little strange.
	//
	// NOTE: we now generate our own usage message.  so if you
	// NOTE: add a command line option below, also update the
	// NOTE: usage message at the bottom of the file.
	//
	// NOTE: we add both lower- and upper-case versions of each
	// NOTE: option.  this is to help integration with third-party
	// NOTE: SCC apps that want to mess with the case of the command
	// NOTE: line args before passing to us.  see item:12414.

	static const wxCmdLineEntryDesc aCLED[] = {

// we don't need our own help switch -- wxWidgets provides one
//	{ wxCMD_LINE_SWITCH, _("h"),		_("help"),		_("Show help"),												wxCMD_LINE_VAL_NONE,	wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_SWITCH, _("help"),		NULL,			NULL,														wxCMD_LINE_VAL_NONE,	wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_SWITCH, _("H"),		_("HELP"),		_("Show help"),												wxCMD_LINE_VAL_NONE,	wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_SWITCH, _("HELP"),		NULL,			NULL,														wxCMD_LINE_VAL_NONE,	wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_SWITCH, _("?"),		NULL,			NULL,														wxCMD_LINE_VAL_NONE,	wxCMD_LINE_OPTION_HELP },

	{ wxCMD_LINE_OPTION, _("caption"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("CAPTION"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("c"),		_("caption"),	_("Set descriptive caption for application title bar."),	wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("C"),		_("CAPTION"),	_("Set descriptive caption for application title bar."),	wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },

	{ wxCMD_LINE_OPTION, _("diff"),		NULL,			_("Dump diff output to file"),								wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("d"),		_("diff"),		_("Dump diff output to file"),								wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("DIFF"),		NULL,			_("Dump diff output to file"),								wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("D"),		_("DIFF"),		_("Dump diff output to file"),								wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },

	{ wxCMD_LINE_SWITCH, _("unified"),	NULL,			_("Format output in UNIFIED format."),						wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("u"),		_("unified"),	_("Format output in UNIFIED format."),						wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("UNIFIED"),	NULL,			_("Format output in UNIFIED format."),						wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("U"),		_("UNIFIED"),	_("Format output in UNIFIED format."),						wxCMD_LINE_VAL_NONE },

	{ wxCMD_LINE_SWITCH, _("ignore_unimportant"),	NULL,						_("Ignore unimportant changes when dumping to file."),		wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("i"),					_("ignore_unimportant"),	_("Ignore unimportant changes when dumping to file."),		wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("IGNORE_UNIMPORTANT"),	NULL,						_("Ignore unimportant changes when dumping to file."),		wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("I"),					_("IGNORE_UNIMPORTANT"),	_("Ignore unimportant changes when dumping to file."),		wxCMD_LINE_VAL_NONE },

#if defined(FEATURE_CLI_HTML_EXPORT)
	{ wxCMD_LINE_SWITCH, _("html"),			_("html"),			_("Format output in HTML format."),							wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("HTML"),			_("HTML"),			_("Format output in HTML format."),							wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("intraline"),	NULL,				_("Include intra-line changes in HTML format."),			wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("il"),			_("intraline"),		_("Include intra-line changes in HTML format."),			wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("INTRALINE"),	NULL,				_("Include intra-line changes in HTML format."),			wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("IL"),			_("INTRALINE"),		_("Include intra-line changes in HTML format."),			wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_OPTION, _("side_by_side"),	NULL,				_("Side-by-Side HTML format."),								wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("sxs"),			_("side_by_side"),	_("Side-by-Side HTML format."),								wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("SIDE_BY_SIDE"),	NULL,				_("Side-by-Side HTML format."),								wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("SXS"),			_("SIDE_BY_SIDE"),	_("Side-by-Side HTML format."),								wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
#endif

	{ wxCMD_LINE_SWITCH, _("merge"),	NULL,			NULL,														wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("MERGE"),	NULL,			NULL,														wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("m"),		_("merge"),		_("Run 'Merge to Center Panel' after loading files."),		wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("M"),		_("MERGE"),		_("Run 'Merge to Center Panel' after loading files."),		wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("nosplash"),_("nosplash"),	_("No splash screen [DEPRECATED]."),						wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("NOSPLASH"),_("NOSPLASH"),	_("No splash screen [DEPRECATED]."),						wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_OPTION, _("result"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("RESULT"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("r"),		_("result"),	_("Alternate pathname for saving 3-way merge result."),		wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("R"),		_("RESULT"),	_("Alternate pathname for saving 3-way merge result."),		wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_SWITCH, _("ro1"),		_("ro1"),		_("Treat file 1 as Read-Only [DEPRECATED]."),				wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("RO1"),		_("RO1"),		_("Treat file 1 as Read-Only [DEPRECATED]."),				wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("ro2"),		_("ro2"),		_("Treat file 2 as Read-Only."),							wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("RO2"),		_("RO2"),		_("Treat file 2 as Read-Only."),							wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("ro3"),		_("ro3"),		_("Treat file 3 as Read-Only [DEPRECATED]."),				wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("RO3"),		_("RO3"),		_("Treat file 3 as Read-Only [DEPRECATED]."),				wxCMD_LINE_VAL_NONE },
#ifdef FEATURE_SHEX
	{ wxCMD_LINE_SWITCH, _("shex"),	_("shex"),		_("Shell/Explorer flag; forces Open File/Folder Dialog to appear."),	wxCMD_LINE_VAL_NONE },
	{ wxCMD_LINE_SWITCH, _("SHEX"),	_("SHEX"),		_("Shell/Explorer flag; forces Open File/Folder Dialog to appear."),	wxCMD_LINE_VAL_NONE },
#endif
	{ wxCMD_LINE_OPTION, _("title1"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("TITLE1"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("t1"),		_("title1"),	_("Set display title for panel 1."),						wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("T1"),		_("TITLE1"),	_("Set display title for panel 1."),						wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("title2"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("TITLE2"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("t2"),		_("title2"),	_("Set display title for panel 2."),						wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("T2"),		_("TITLE2"),	_("Set display title for panel 2."),						wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("title3"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("TITLE3"),	NULL,			NULL,														wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("t3"),		_("title3"),	_("Set display title for panel 3."),						wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_OPTION, _("T3"),		_("TITLE3"),	_("Set display title for panel 3."),						wxCMD_LINE_VAL_STRING,	wxCMD_LINE_NEEDS_SEPARATOR },
	{ wxCMD_LINE_PARAM, NULL,			NULL,			_("<pathname1>"),											wxCMD_LINE_VAL_STRING,	wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_PARAM, NULL,			NULL,			_("<pathname2>"),											wxCMD_LINE_VAL_STRING,	wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_PARAM, NULL,			NULL,			_("<pathname3>"),											wxCMD_LINE_VAL_STRING,	wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_NONE }};

	parser.SetDesc(aCLED);		// append our table onto builtin one

//	wxString logo = ( VER_APP_TITLE _("\n")
//					  VER_COPYRIGHT _("\n") );
//	parser.SetLogo(logo);
}

//////////////////////////////////////////////////////////////////

bool gui_app::OnCmdLineParsed(wxCmdLineParser & parser)
{
	// we only get called if the command line was "lexically" valid.

	// let base class handle any args that it added, like '-h'

	if (!wxApp::OnCmdLineParsed(parser)  ||  m_cl_args.bParseErrors)
	{
		// NOTE: as of 3.3, we return true (but skip a lot of stuff)
		// so that we can get to OnRun() and properly return an
		// exit status.
		//
		// the command parser should have already stuffed a message
		// into the error msg queue.

		m_cl_args.bParseErrors = true;
		return true;
	}

	// note: we have to do the ( Found("c") || Found("caption") ) trick because
	// note: we have 2 entries for them.  [if they give both and give an argument
	// note: with each, we'll take one of them at random.]
	// note:
	// note: we also have to do both upper and lower case.

	m_cl_args.bMerge       = (    parser.Found(_T("m"))   ||  parser.Found(_T("merge"))
							   || parser.Found(_T("M"))   ||  parser.Found(_T("MERGE")) );

	m_cl_args.bReadOnly    = (    parser.Found(_T("ro2"))
							   || parser.Found(_T("RO2")) );
	m_cl_args.bTitle[0]    = (    parser.Found(_T("t1"),&m_cl_args.title[0])  ||  parser.Found(_T("title1"),&m_cl_args.title[0])
							   || parser.Found(_T("T1"),&m_cl_args.title[0])  ||  parser.Found(_T("TITLE1"),&m_cl_args.title[0]) );
	m_cl_args.bTitle[1]    = (    parser.Found(_T("t2"),&m_cl_args.title[1])  ||  parser.Found(_T("title2"),&m_cl_args.title[1])
							   || parser.Found(_T("T2"),&m_cl_args.title[1])  ||  parser.Found(_T("TITLE2"),&m_cl_args.title[1]) );
	m_cl_args.bTitle[2]    = (    parser.Found(_T("t3"),&m_cl_args.title[2])  ||  parser.Found(_T("title3"),&m_cl_args.title[2])
							   || parser.Found(_T("T3"),&m_cl_args.title[2])  ||  parser.Found(_T("TITLE3"),&m_cl_args.title[2]) );
	m_cl_args.bResult      = (    parser.Found(_T("r"),&m_cl_args.result)     ||  parser.Found(_T("result"),&m_cl_args.result)
							   || parser.Found(_T("R"),&m_cl_args.result)     ||  parser.Found(_T("RESULT"),&m_cl_args.result) );
	m_cl_args.bCaption     = (    parser.Found(_T("c"),&m_cl_args.caption)    ||  parser.Found(_T("caption"),&m_cl_args.caption)
							   || parser.Found(_T("C"),&m_cl_args.caption)    ||  parser.Found(_T("CAPTION"),&m_cl_args.caption) );

	m_cl_args.bDumpDiffs   = (    parser.Found(_T("d"),&m_cl_args.diffOutput) ||  parser.Found(_T("diff"),&m_cl_args.diffOutput)
							   || parser.Found(_T("D"),&m_cl_args.diffOutput) ||  parser.Found(_T("DIFF"),&m_cl_args.diffOutput) );
	m_cl_args.bDumpUnified = (    parser.Found(_T("u")) ||  parser.Found(_T("unified"))
							   || parser.Found(_T("U")) ||  parser.Found(_T("UNIFIED")) );
	m_cl_args.bDumpIgnoreUnimportant = (   parser.Found(_T("i"))  ||  parser.Found(_T("ignore_unimportant"))
										|| parser.Found(_T("I"))  ||  parser.Found(_T("IGNORE_UNIMPORTANT")));


#if defined(FEATURE_CLI_HTML_EXPORT)
	m_cl_args.bDumpHtml =    (    parser.Found(_T("html")) || parser.Found(_T("HTML")) );
	m_cl_args.bDumpHtmlIntraLine = (   parser.Found(_T("il"))  ||  parser.Found(_T("intraline"))
									|| parser.Found(_T("IL"))  ||  parser.Found(_T("INTRALINE")) );
	wxString strSxS;
	m_cl_args.bDumpHtmlSxS = (   parser.Found(_T("sxs"), &strSxS)  ||  parser.Found(_T("side_by_side"), &strSxS)
							  || parser.Found(_T("SXS"), &strSxS)  ||  parser.Found(_T("SIDE_BY_SIDE"), &strSxS));
	m_cl_args.dopsHtmlSxS = DE_DOP_ALL;
	if (m_cl_args.bDumpHtmlSxS)
	{
		// technically -sxs=* and -u conflict, but i'm not going to complain about it.

		wxASSERT_MSG( (strSxS.Length() > 0), _T("Coding Error") );
		switch (strSxS.wc_str()[0])
		{
		case L'c':
		case L'C':
			m_cl_args.dopsHtmlSxS = DE_DOP_CTX;
			break;

		case L'd':
		case L'D':
			m_cl_args.dopsHtmlSxS = DE_DOP_DIF;
			break;

		default:
			// assume all rather than error
			break;
		}
	}
#endif // FEATURE_CLI_HTML_EXPORT


#ifdef FEATURE_SHEX
	m_cl_args.bShEx        = (    parser.Found(_T("shex"))
							   || parser.Found(_T("SHEX")) );
#endif

	m_cl_args.nrParams     = (int)(parser.GetParamCount());
	for (int k=0; k<m_cl_args.nrParams; k++)
		m_cl_args.pathname[k] = parser.GetParam(k);

	// apply some higher-level checks.

	bool bWarning = false;
	wxString msgError;
	wxString msgWarning;

	// we allow 1 pathname -- happens when someone drags a file onto our desktop icon
	// or when the shell extension launches us.

	for (int j=m_cl_args.nrParams; j<3; j++)
	{
		if (m_cl_args.bTitle[j])		// warn about -t3 if only 2 pathnames given
		{
			bWarning = true;
			msgWarning += wxString::Format( _("Ignored Option: '--title%d' requires at least %d pathnames.  "), j+1, j+1 );
			m_cl_args.bTitle[j] = false;
		}
	}
	
	if (m_cl_args.bReadOnly && (m_cl_args.nrParams < 2))	// warn about -ro2 if not enough pathnames given
	{
		bWarning = true;
		msgWarning += _("Ignored Option: '-ro2' requires at least 2 pathnames.");
		m_cl_args.bReadOnly = false;
	}

	int nrFiles = 0;
	int nrFolders = 0;

	for (int n=0; n<m_cl_args.nrParams; n++)
	{
#if defined(__WXMSW__)
		// If m_cl_args.pathname[n] contains a SHORTCUT .lnk
		// replace it with the pathname of the target.
		// Complain if it refers to printer or something that
		// doesn't expand to a pathname.
		//
		// We DO NOT try to create a LNK window (such as the
		// "show shortcut info" dialog), since it is (I'm assuming)
		// expected that we evaulate/dereference the target
		// like Windows Explorer would when you click on it.
		deref_if_shortcut(n, msgError);
#endif

		if (wxFileName::DirExists(m_cl_args.pathname[n]))
		{
			nrFolders++;
		}
		else if (wxFileName::FileExists(m_cl_args.pathname[n]))
		{
			nrFiles++;
		}
#if defined(__WXMAC__) || defined(__WXGTK__)
		else if (m_cl_args.pathname[n].Cmp(_T("/dev/null")) == 0)
		{
			// help GIT use us.  GIT gives /dev/null on command line when diffing
			// the first version of a file with the (non-existant) previous version.
			// see item:13303.
			// /dev/null doesn't pass wxFileName::FileExists() because it is a
			// character-special file.
			nrFiles++;
		}
#endif
		else
		{
			m_cl_args.bParseErrors = true;
			msgError += wxString::Format( _("File (%s) not found.  "), m_cl_args.pathname[n].wc_str());
		}
	}

	if (nrFiles+nrFolders > 0)
	{
		if ( (nrFiles > 0)  &&  (nrFolders > 0) )
		{
			m_cl_args.bParseErrors = true;
			msgError += _("Pathnames must be all files or all folders.  ");
		}
		else if (nrFiles == m_cl_args.nrParams)
		{
			m_cl_args.bFolders = false;
		}
		else if (nrFolders == m_cl_args.nrParams)
		{
			m_cl_args.bFolders = true;
			if (m_cl_args.nrParams == 3)
			{
				m_cl_args.bParseErrors = true;
				msgError += _("3 pathname option only applies to files, not folders.");
			}
		}
	}

	if ( m_cl_args.bMerge && (m_cl_args.nrParams != 3) )
	{
		bWarning = true;
		msgWarning += _("Ignored Option: '--merge' only applies to 3 file merges.  ");
		m_cl_args.bMerge = false;
	}

	if ( m_cl_args.bResult && (m_cl_args.nrParams != 3) )
	{
		bWarning = true;
		msgWarning += _("Ignored Option: '--result' only applies to 3 file merges.  ");
		m_cl_args.bResult = false;
	}

	if (m_cl_args.bReadOnly)
	{
		if (nrFolders > 0)
		{
#if 0
			// BUG #10988 complained that when Vault is launched on a set of Folders, it
			// passes --ro2.  this causes us to raise a command line warning
			// message box.  technically, this is is not our problem, Vault shouldn't be
			// doing that.  but oh well, turn off the warning for now.
			bWarning = true;
			msgWarning += _("Ignored Option: '--ro2' only applies to files, not folders.");
#endif
			m_cl_args.bReadOnly = false;
		}
		else if (nrFiles == 0)
		{
			bWarning = true;
			msgWarning += _("Ignored Option: '--ro2' only applies when are files given.");
			m_cl_args.bReadOnly = false;
		}
	}

	if (m_cl_args.bDumpDiffs)
	{

		// TODO allow dump-diffs to take 3 files.

		if ((nrFiles == 2) || (nrFolders == 2))
		{
		}
		else
		{
			m_cl_args.bParseErrors = true;
			msgError += wxString::Format( _("Option '--diff' requires 2 files or 2 folders."));
		}

		// for safety, require that the dump output pathname be different than the
		// input pathnames.

		for (int n=0; n<m_cl_args.nrParams; n++)
		{
			if (m_cl_args.diffOutput.CmpNoCase(m_cl_args.pathname[n]) == 0)
			{
				m_cl_args.bParseErrors = true;
				msgError += wxString::Format( _("Pathname given to '--diff' must not match input files."));
			}
		}

		// dump-diffs implies that we are not going to open a window -- that is, that
		// we are running batch (probably from a test suite), so let's not block on
		// modal message box dialog (at least on windows).  dump errors/warnings to
		// the debug log and go on -- this is not optimal, it should go to stderr,
		// but on windows we don't have one....

		if (m_cl_args.bParseErrors)
		{
			//wxLogTrace(wxTRACE_Messages,_T("Error: %s"),msgError.wc_str());
			return true;
		}
		if (bWarning)
		{
			//wxLogTrace(wxTRACE_Messages,_T("Warning: %s"),msgWarning.wc_str());
		}
		
		return true;
	}

	
	if (m_cl_args.bParseErrors)
	{
		wxLogError(_T("%s"),msgError.wc_str());
		return true;			// we want to exit, but let this finish so we can set exit status.
	}
	
	if (bWarning)
	{
		wxLogWarning(_T("%s"),msgWarning.wc_str());
	}
	
	return true;				// let app start if no errors or only warnings
}

//////////////////////////////////////////////////////////////////

bool gui_app::OnCmdLineError(wxCmdLineParser & /*parser*/)
{
	// this gets called after wxCmdLineParser has generated
	// the detailed error message.  it is a chance for the app
	// to also output or not-output usage info following the
	// message.
	//
	// because we installed our custom wxMessageOutput, we
	// must actually emit the wxCmdLineParser's error message
	// here.  (see OnInit())

	MessageOutputString * mo = static_cast<MessageOutputString *>(wxMessageOutput::Get());
	mo->Output( _T("\n") );
	mo->Output( formatArgvForSupportMessage() );
	wxLogError(_T("%s"),mo->getMsg().wc_str());
	
	//wxLogTrace(wxTRACE_Messages, _T("OnCmdLineError: %s"),
	//		   mo->getMsg().wc_str());

	// the wxApp base class always prints usage info.
	// we override default behavior to not show usage
	// unless explictly asked for (with --help).

	// NOTE: as of 3.3, we allow the command line parser to
	// to complete so that we can return a proper exit status
	// in OnRun().

	m_cl_args.bParseErrors = true;
	return true;
}

//////////////////////////////////////////////////////////////////

bool gui_app::OnCmdLineHelp(wxCmdLineParser & /*parser*/)
{
	// this gets called after wxCmdLineParser has detected
	// that a '-h' was given.

	//////////////////////////////////////////////////////////////////
	// because we installed our custom wxMessageOutput, we
	// must actually emit the usage infomation.  (see OnInit())
	//	parser.Usage();
	//	MessageOutputString * mo = static_cast<MessageOutputString *>(wxMessageOutput::Get());
	//	wxLogMessage(mo->getMsg());
	//////////////////////////////////////////////////////////////////

	wxString strMyUsage = _makeUsageString();
	wxLogMessage(_T("%s"),strMyUsage.wc_str());

	return false;				// always exit after providing help
}

wxString gui_app::_makeUsageString(void) const
{
	wxString strUsage;
	
	//////////////////////////////////////////////////////////////////
	// |SourceGear DiffMerge (<version>)
	// |Copyright...
	// |
	// |Synopsis:
	// |    DiffMerge [<options>+] [<path1> <path2> [<path3>]]
	// |
	//////////////////////////////////////////////////////////////////

	strUsage += VER_APP_TITLE;
	strUsage += _T(" - ");
	strUsage += AboutBox::build_version_string();
	strUsage += _T("\n");

	strUsage += VER_COPYRIGHT _T("\n");
	
	strUsage += _T("\n");

	strUsage += _T("Synopsis:\n");
	strUsage += _T("    ");
	strUsage += wxFileName(getExePathnameString()).GetName();
	strUsage += _T(" [options] [path1 [path2 [path3]]]\n");

	strUsage += _T("\n");

	//////////////////////////////////////////////////////////////////
	// |Options:
	// |        /short, /long, --long
	// |                discussions....
	// |
	// |        /short, /long, --long
	// |                discussions....
	// |

	strUsage += _("Options:\n");
	strUsage += _("    /h, /help, --help\n");
	strUsage += _("    /c=STRING, /caption=STRING, --caption=STRING\n");
	strUsage += _("    /m, /merge, --merge\n");
	strUsage += _("    /r=PATH, /result=PATH, --result=PATH\n");
	strUsage += _("    /ro2, --ro2\n");

#ifdef FEATURE_SHEX
	strUsage += _("    /shex, --shex \n");
#endif

	strUsage += _("    /t1=STRING, /title1=STRING, --title1=STRING\n");
	strUsage += _("    /t2=STRING, /title2=STRING, --title2=STRING\n");
	strUsage += _("    /t3=STRING, /title3=STRING, --title3=STRING\n");

//	strUsage += _("    /ro1, --ro1\n");
//	strUsage += _("    /ro3, --ro3\n");
//	strUsage += _("    /nosplash, --nosplash\n");

	strUsage += _("\n");
	strUsage += _("Batch Options:\n");

	strUsage += _("    /d=PATH, /diff=PATH, --diff=PATH\n");
	strUsage += _("    /u, /unified, --unified\n");
	strUsage += _("    /i, /ignore_unimportant, --ignore_unimportant\n");

#if defined(FEATURE_CLI_HTML_EXPORT)
	strUsage += _("    /html, --html\n");
	strUsage += _("    /il, /intraline, --intraline\n");
	strUsage += _("    /sxs=[ACD], /side_by_side=[ACD], --side_by_side=[ACD]\n");
#endif

	//////////////////////////////////////////////////////////////////

#if defined(__WXMAC__) || defined(__WXGTK__)
	// mac and linux only use '-' for command line switches.
	// we built the string showing win32 style.
	strUsage.Replace(_T("/"), _T("-"), true);
#endif

	return strUsage;
}

//////////////////////////////////////////////////////////////////

wxString gui_app::formatArgvForSupportMessage(void)
{
	wxString strMessage;
	
	strMessage += wxString::Format(_T("Command Line Arguments:\n"));
	for (int k=0; k<argc; k++)
		strMessage += wxString::Format(_T("[%d]: %s\n"), k, argv[k]);
	strMessage += _T("\n");

	return strMessage;
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
void gui_app::deref_if_shortcut(int n, wxString & msgError)
{
	util_error err;
	bool bIsLnk = false;
	util_shell_lnk * pLnk = NULL;

	err = util_file__is_shell_lnk(m_cl_args.pathname[n], &bIsLnk);
	if (err.isErr())
	{
		m_cl_args.bParseErrors = true;
		msgError += wxString::Format( _("%s: %s  "), err.getMessage(), err.getExtraInfo());
		goto done;
	}

	if (!bIsLnk)
		goto done;

	err = util_shell_lnk::ctor( m_cl_args.pathname[n], &pLnk);
	if (err.isErr())
	{
		m_cl_args.bParseErrors = true;
		msgError += wxString::Format( _("%s: %s  "), err.getMessage(), err.getExtraInfo());
		goto done;
	}

	if (!pLnk->pStrTargetPath)
	{
		m_cl_args.bParseErrors = true;
		msgError += wxString::Format( _("Shortcut '%s' does not refer to a filesystem object.  "),
									  m_cl_args.pathname[n]);
		goto done;
	}

	// replace the given LNK pathname with the target
	// and continue parsing the command line.

	m_cl_args.pathname[n] = *pLnk->pStrTargetPath;

done:
	DELETEP(pLnk);
}
#endif//__WXMSW__
