// util_conditional_nonblank_text_validator.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

ConditionalNonBlankTextValidator::ConditionalNonBlankTextValidator(wxString * pStr, const wxString & strErrorMsg)
	: wxValidator(),
	  m_pStr(pStr),
	  m_strErrorMsg(strErrorMsg)
{
}

ConditionalNonBlankTextValidator::ConditionalNonBlankTextValidator(const ConditionalNonBlankTextValidator & val)
	: wxValidator()
{
	Copy(val);
}

ConditionalNonBlankTextValidator::~ConditionalNonBlankTextValidator(void)
{
}

//////////////////////////////////////////////////////////////////

wxObject * ConditionalNonBlankTextValidator::Clone(void) const
{
	return new ConditionalNonBlankTextValidator(*this);
}
	
bool ConditionalNonBlankTextValidator::Copy(const ConditionalNonBlankTextValidator & val)
{
	wxValidator::Copy(val);

	m_pStr = val.m_pStr;
	m_strErrorMsg = val.m_strErrorMsg;
	
	return true;
}

bool ConditionalNonBlankTextValidator::TransferToWindow(void)
{
	// always transfer data, even if disabled.

	if (!m_validatorWindow)
		return false;

	if (m_pStr)
	{
		wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
		control->SetValue(* m_pStr);
	}

//	wxLogTrace(TRACE_UTIL_DUMP, _T("ConditionalNonBlankTextValidator::TransferToWindow: [%p][%s]"), m_pStr, m_pStr->wc_str());
	
	return true;
}
	
bool ConditionalNonBlankTextValidator::TransferFromWindow(void)
{
	// always transfer data, even if disabled.

	if (!m_validatorWindow)
		return false;

	if (m_pStr)
	{
		wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
		*m_pStr = control->GetValue();
	}

//	wxLogTrace(TRACE_UTIL_DUMP, _T("ConditionalNonBlankTextValidator::TransferFromWindow: [%p][%s]"), m_pStr, m_pStr->wc_str());

	return true;
}

bool ConditionalNonBlankTextValidator::Validate(wxWindow * pParent)
{
	if (!m_validatorWindow)
		return false;

	wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;

	// only validate if enabled.  allow blank fields if disabled.

	bool bEnabled = control->IsEnabled();
	if (!bEnabled)
		return true;

	wxString val( control->GetValue() );

	val.Trim(false);	// ltrim()
	val.Trim(true);		// rtrim()
	
//	wxLogTrace(TRACE_UTIL_DUMP, _T("ConditionalNonBlankTextValidator::Validate: [%p][%s]"), m_pStr, val.wc_str());

	if (val.Length() > 0)
		return true;
	
	// the string did not pass.  warp focus to the field, complain, and prevent
	// the parent dialog from going away.

	m_validatorWindow->SetFocus();
	
	wxMessageDialog dlg(pParent, m_strErrorMsg, _("Invalid Value!"), wxOK|wxICON_ERROR);
	dlg.ShowModal();

	return false;
}
