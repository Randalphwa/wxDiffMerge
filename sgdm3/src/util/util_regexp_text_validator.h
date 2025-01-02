// util_regexp_text_validator.h
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_REGEXP_TEXT_VALIDATOR_H
#define H_UTIL_REGEXP_TEXT_VALIDATOR_H

//////////////////////////////////////////////////////////////////

class RegExpTextValidator : public wxValidator
{
public:
	RegExpTextValidator(wxString * pStr, wxString strBlankErrMsg, bool * pbCanBeBlank=NULL);
	RegExpTextValidator(const RegExpTextValidator & val);
	~RegExpTextValidator(void);

	virtual wxObject *	Clone(void) const;
	
	bool				Copy(const RegExpTextValidator & val);

	virtual bool		TransferToWindow(void);
	virtual bool		TransferFromWindow(void);
	virtual bool		Validate(wxWindow * parent);

private:
	wxString *			m_pStr;
	wxString			m_strBlankErrMsg;
	bool *				m_pbCanBeBlank;
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_REGEXP_TEXT_VALIDATOR_H
