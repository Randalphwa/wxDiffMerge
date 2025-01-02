// util_conditional_nonblank_text_validator.h
// a version of the non-blank text validator that only validates
// when the control is enabled (allows a blank field when the
// control is disabled).
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_CONDITIONAL_NONBLANK_TEXT_VALIDATOR_H
#define H_UTIL_CONDITIONAL_NONBLANK_TEXT_VALIDATOR_H

//////////////////////////////////////////////////////////////////

class ConditionalNonBlankTextValidator : public wxValidator
{
public:
	ConditionalNonBlankTextValidator(wxString * pStr, const wxString & strErrorMsg);
	ConditionalNonBlankTextValidator(const ConditionalNonBlankTextValidator & val);
	~ConditionalNonBlankTextValidator(void);

	virtual wxObject *	Clone(void) const;
	
	bool				Copy(const ConditionalNonBlankTextValidator & val);

	virtual bool		TransferToWindow(void);
	virtual bool		TransferFromWindow(void);
	virtual bool		Validate(wxWindow * parent);

private:
	wxString *			m_pStr;
	wxString			m_strErrorMsg;
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_CONDITIONAL_NONBLANK_TEXT_VALIDATOR_H
