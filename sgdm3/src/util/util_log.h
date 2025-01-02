// util_log.h
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_LOG_H
#define H_UTIL_LOG_H

//////////////////////////////////////////////////////////////////

class util_logToString : public wxLog
{
public:
	util_logToString(wxString * pString, bool bTrapEverything=false);
	virtual ~util_logToString(void);
	
    virtual void DoLogTextAtLevel(wxLogLevel level, const wxString& msg);

private:
	wxString *		m_pString;
	wxLog *			m_pLogPrev;
	bool			m_bTrapEverything;
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_LOG_H
