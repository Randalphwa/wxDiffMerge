// util_regexp_text_validator.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

RegExpTextValidator::RegExpTextValidator(wxString * pStr, wxString strBlankErrMsg, bool * pbCanBeBlank)
	: wxValidator(),
	  m_pStr(pStr),
	  m_strBlankErrMsg(strBlankErrMsg),
	  m_pbCanBeBlank(pbCanBeBlank)
{
}

RegExpTextValidator::RegExpTextValidator(const RegExpTextValidator & val)
	: wxValidator()
{
	Copy(val);
}

RegExpTextValidator::~RegExpTextValidator(void)
{
}

//////////////////////////////////////////////////////////////////

wxObject * RegExpTextValidator::Clone(void) const
{
	return new RegExpTextValidator(*this);
}
	
bool RegExpTextValidator::Copy(const RegExpTextValidator & val)
{
	wxValidator::Copy(val);

	m_pStr = val.m_pStr;
	m_strBlankErrMsg = val.m_strBlankErrMsg;
	m_pbCanBeBlank = val.m_pbCanBeBlank;
	
	return true;
}

bool RegExpTextValidator::TransferToWindow(void)
{
	if (!m_validatorWindow)
		return false;

	if (m_pStr)
	{
		wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
		control->SetValue(* m_pStr);
	}

//	wxLogTrace(TRACE_UTIL_DUMP, _T("RegExpTextValidator::TransferToWindow: [%p][%s]"), m_pStr, m_pStr->wc_str());
	
	return true;
}
	
bool RegExpTextValidator::TransferFromWindow(void)
{
	if (!m_validatorWindow)
		return false;

	if (m_pStr)
	{
		wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
		*m_pStr = control->GetValue();
	}

//	wxLogTrace(TRACE_UTIL_DUMP, _T("RegExpTextValidator::TransferFromWindow: [%p][%s]"), m_pStr, m_pStr->wc_str());

	return true;
}

bool RegExpTextValidator::Validate(wxWindow * pParent)
{
	if (!m_validatorWindow)
		return false;

	wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
	wxString val( control->GetValue() );

//	wxLogTrace(TRACE_UTIL_DUMP, _T("RegExpTextValidator::Validate: [%p][%s]"), m_pStr, val.wc_str());

	wxString strError;

	if (val.Length() == 0)
	{
		if (!m_pbCanBeBlank || *m_pbCanBeBlank)
			return true;

		if (m_strBlankErrMsg.Length() != 0)
			strError = m_strBlankErrMsg;
		else
			strError = _("Error: Pattern cannot be blank.");
	}
	else
	{
		// Verify that the given string is a valid RegExp.
		// Compile the expression into a new wxRegEx and
		// see if it is valid.  But because this spews onto
		// the log when there are errors, we catch the error
		// in a local string.

		util_logToString uLog(&strError);

		wxRegEx re(val,wxRE_ADVANCED);
		if (re.IsValid())
			return true;
	}

	// the string did not pass.  warp focus to the field, complain, and prevent
	// the parent dialog from going away.

	m_validatorWindow->SetFocus();
	
	wxMessageDialog dlg(pParent,strError,_("Invalid Pattern!"), wxOK|wxICON_ERROR);
	dlg.ShowModal();

	return false;
}
