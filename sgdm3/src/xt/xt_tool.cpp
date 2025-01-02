// xt_tool.cpp
// code to manipulate the representation of a configured external tool.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <xt.h>

//////////////////////////////////////////////////////////////////

static XT_ID g_XT_ID_Next = 0;

//////////////////////////////////////////////////////////////////

xt_tool::xt_tool(void)
{
	m_ID = g_XT_ID_Next++;
	m_bEnabled2 = false;
	m_bEnabled3 = false;
}

xt_tool::~xt_tool(void)
{
}

xt_tool::xt_tool(const xt_tool & xt)		// copy constructor
{
	m_ID = xt.m_ID;

	m_strName = xt.m_strName;
	m_strSuffixes = xt.m_strSuffixes;
	m_bEnabled2 = xt.m_bEnabled2;
	m_bEnabled3 = xt.m_bEnabled3;

	m_strGui2Exe = xt.m_strGui2Exe;
	m_strGui2Args = xt.m_strGui2Args;

	m_strGui3Exe = xt.m_strGui3Exe;
	m_strGui3Args = xt.m_strGui3Args;
}

xt_tool * xt_tool::clone(void) const
{
	// clone and update some fields

	xt_tool * pxtNew = new xt_tool(*this);

	pxtNew->setName( wxString::Format( _("Copy of %s"), m_strName.wc_str()) );
	pxtNew->m_ID = g_XT_ID_Next++;

	return pxtNew;
}

bool xt_tool::isEqual(const xt_tool * pxt) const
{
	if (!pxt) return false;

	if (m_ID != pxt->m_ID) return false;

	if (m_strName != pxt->m_strName) return false;
	if (m_strSuffixes != pxt->m_strSuffixes) return false;
	if (m_bEnabled2 != pxt->m_bEnabled2) return false;
	if (m_bEnabled3 != pxt->m_bEnabled3) return false;

	if (m_strGui2Exe != pxt->m_strGui2Exe) return false;
	if (m_strGui2Args != pxt->m_strGui2Args) return false;

	if (m_strGui3Exe != pxt->m_strGui3Exe) return false;
	if (m_strGui3Args != pxt->m_strGui3Args) return false;

	return true;
}

bool xt_tool::testPathnameSuffix(const poi_item * pPoi) const
{
	// see if the suffix of the given pathname matches one of the
	// ones in our set of suffixes.  if so, this external tool could be
	// used for this document.  [provided that we match the other
	// documents in the document set.]
	//
	// return true if we have a match.

	bool bIgnoreSuffixCase = gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_IGNORE_SUFFIX_CASE);

	wxStringTokenizer tkz(m_strSuffixes);

	if (tkz.HasMoreTokens() == false)			// list of suffixes is effectively blank
		return true;

	wxFileName fn = pPoi->getFileName();

	// fetch suffix of pathname -- if it doesn't have a suffix, use
	// the base-name (filename without path).

	wxString sfx;
	if (fn.HasExt()  &&  (fn.GetExt().Length() > 0))
		sfx = fn.GetExt();
	else
		sfx = fn.GetName();
	
	while (tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();

		if (token == _T("*"))					// let .* match anything
			return true;
		
		bool bEqual;
		if (bIgnoreSuffixCase)
			bEqual = (sfx.CmpNoCase(token) == 0);
		else
			bEqual = (sfx.Cmp(token) == 0);

		if (bEqual)
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////

wxString xt_tool::dumpSupportInfo(void) const
{
	wxString str;

	//////////////////////////////////////////////////////////////////
	// name & suffix list
	//////////////////////////////////////////////////////////////////
	
	str += wxString::Format(_T("\tExternalTool: %s\n"), m_strName.wc_str());
	str += wxString::Format(_T("\t\tSuffixes: %s\n"), m_strSuffixes.wc_str());
	str += wxString::Format(_T("\t\tEnabled: [%d][%d]\n"), m_bEnabled2, m_bEnabled3);

	//////////////////////////////////////////////////////////////////
	// GUI 2-way diff exe/args
	//////////////////////////////////////////////////////////////////

	str += wxString::Format(_T("\t\tGUI Diff:\n"));
	str += wxString::Format(_T("\t\t\tExe: %s\n"), m_strGui2Exe.wc_str());
	str += wxString::Format(_T("\t\t\tArgs: %s\n"), m_strGui2Args.wc_str());

	//////////////////////////////////////////////////////////////////
	// GUI 3-way merge exe/args
	//////////////////////////////////////////////////////////////////

	str += wxString::Format(_T("\t\tGUI Merge:\n"));
	str += wxString::Format(_T("\t\t\tExe: %s\n"), m_strGui3Exe.wc_str());
	str += wxString::Format(_T("\t\t\tArgs: %s\n"), m_strGui3Args.wc_str());

	return str;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void xt_tool::dump(int indent) const
{
	wxLogTrace(TRACE_XT_DUMP, _T("%*cTOOL: [%p][%d][%s][%s][%d][%d]"),
			   indent,' ',
			   this, m_ID, m_strName.wc_str(),m_strSuffixes.wc_str(),m_bEnabled2,m_bEnabled3);

	wxLogTrace(TRACE_XT_DUMP, _T("%*cGui2Exe[%s]"),
			   indent+5,' ',
			   m_strGui2Exe.wc_str());
	wxLogTrace(TRACE_XT_DUMP, _T("%*cGui2Args[%s]"),
			   indent+5,' ',
			   m_strGui2Args.wc_str());

	wxLogTrace(TRACE_XT_DUMP, _T("%*cGui3Exe[%s]"),
			   indent+5,' ',
			   m_strGui3Exe.wc_str());
	wxLogTrace(TRACE_XT_DUMP, _T("%*cGui3Args[%s]"),
			   indent+5,' ',
			   m_strGui3Args.wc_str());
}
#endif

//////////////////////////////////////////////////////////////////
// we define a series of class-static functions to perform variable
// substitutions into a copy of the argument template.
//
// the argument template that the user gives us might look like:
//     /title1="%LEFT_TITLE%" "%LEFT_PATH"%
//
// we want to substitute the given values for these tokens.

static void _apply_token(wxString & strArgs, const wxChar * szToken, const wxChar * szValue)
{
	strArgs.Replace(szToken,szValue,true);
}

#define FN_APPLY(fn,token)	void xt_tool::fn(wxString & strArgs, const wxChar * szValue) { _apply_token(strArgs,token,szValue); }

FN_APPLY(apply_left_path,			_T("%LEFT_PATH%"));
FN_APPLY(apply_left_title,			_T("%LEFT_LABEL%"));
FN_APPLY(apply_right_path,			_T("%RIGHT_PATH%"));
FN_APPLY(apply_right_title,			_T("%RIGHT_LABEL%"));

FN_APPLY(apply_working_path,		_T("%WORKING_PATH%"));
FN_APPLY(apply_working_title,		_T("%WORKING_LABEL%"));
FN_APPLY(apply_baseline_path,		_T("%BASELINE_PATH%"));
FN_APPLY(apply_other_path,			_T("%OTHER_PATH%"));
FN_APPLY(apply_other_title,			_T("%OTHER_LABEL%"));
FN_APPLY(apply_destination_path,	_T("%DEST_PATH%"));
FN_APPLY(apply_destination_title,	_T("%DEST_LABEL%"));
