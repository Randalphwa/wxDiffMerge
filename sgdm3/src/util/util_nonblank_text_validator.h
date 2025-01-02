// util_nonblank_text_validator.h
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_NONBLANK_TEXT_VALIDATOR_H
#define H_UTIL_NONBLANK_TEXT_VALIDATOR_H

//////////////////////////////////////////////////////////////////

class NonBlankTextValidator : public wxValidator
{
public:
	NonBlankTextValidator(wxString * pStr, const wxString & strErrorMsg);
	NonBlankTextValidator(const NonBlankTextValidator & val);
	~NonBlankTextValidator(void);

	virtual wxObject *	Clone(void) const;
	
	bool				Copy(const NonBlankTextValidator & val);

	virtual bool		TransferToWindow(void);
	virtual bool		TransferFromWindow(void);
	virtual bool		Validate(wxWindow * parent);

private:
	wxString *			m_pStr;
	wxString			m_strErrorMsg;
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_NONBLANK_TEXT_VALIDATOR_H
