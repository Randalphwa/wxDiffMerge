// util_nonblank_text_validator.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

NonBlankTextValidator::NonBlankTextValidator(wxString * pStr, const wxString & strErrorMsg)
	: wxValidator(),
	  m_pStr(pStr),
	  m_strErrorMsg(strErrorMsg)
{
}

NonBlankTextValidator::NonBlankTextValidator(const NonBlankTextValidator & val)
	: wxValidator()
{
	Copy(val);
}

NonBlankTextValidator::~NonBlankTextValidator(void)
{
}

//////////////////////////////////////////////////////////////////

wxObject * NonBlankTextValidator::Clone(void) const
{
	return new NonBlankTextValidator(*this);
}
	
bool NonBlankTextValidator::Copy(const NonBlankTextValidator & val)
{
	wxValidator::Copy(val);

	m_pStr = val.m_pStr;
	m_strErrorMsg = val.m_strErrorMsg;
	
	return true;
}

bool NonBlankTextValidator::TransferToWindow(void)
{
	if (!m_validatorWindow)
		return false;

	if (m_pStr)
	{
		wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
		control->SetValue(* m_pStr);
	}

//	wxLogTrace(TRACE_UTIL_DUMP, _T("NonBlankTextValidator::TransferToWindow: [%p][%s]"), m_pStr, m_pStr->wc_str());
	
	return true;
}
	
bool NonBlankTextValidator::TransferFromWindow(void)
{
	if (!m_validatorWindow)
		return false;

	if (m_pStr)
	{
		wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
		*m_pStr = control->GetValue();
	}

//	wxLogTrace(TRACE_UTIL_DUMP, _T("NonBlankTextValidator::TransferFromWindow: [%p][%s]"), m_pStr, m_pStr->wc_str());

	return true;
}

bool NonBlankTextValidator::Validate(wxWindow * pParent)
{
	if (!m_validatorWindow)
		return false;

	wxTextCtrl * control = (wxTextCtrl *)m_validatorWindow;
	wxString val( control->GetValue() );

	val.Trim(false);	// ltrim()
	val.Trim(true);		// rtrim()
	
//	wxLogTrace(TRACE_UTIL_DUMP, _T("NonBlankTextValidator::Validate: [%p][%s]"), m_pStr, val.wc_str());

	if (val.Length() > 0)
		return true;
	
	// the string did not pass.  warp focus to the field, complain, and prevent
	// the parent dialog from going away.

	m_validatorWindow->SetFocus();
	
	wxMessageDialog dlg(pParent, m_strErrorMsg, _("Invalid Value!"), wxOK|wxICON_ERROR);
	dlg.ShowModal();

	return false;
}
