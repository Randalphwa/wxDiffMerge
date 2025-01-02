// MessageOutputString.h
//////////////////////////////////////////////////////////////////

#ifndef H_MESSAGEOUTPUTSTRING_H
#define H_MESSAGEOUTPUTSTRING_H

//////////////////////////////////////////////////////////////////

class MessageOutputString : public wxMessageOutput
{
public:
	MessageOutputString() { };

	virtual void	Printf(const wxChar * format, ...) WX_ATTRIBUTE_PRINTF_2;
	wxString		getMsg(void) const	{ return msg; };

	virtual void	Output(const wxString & str);

private:
	wxString		msg;
};

//////////////////////////////////////////////////////////////////

#endif//H_MESSAGEOUTPUTSTRING_H
