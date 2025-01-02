// util_log.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

util_logToString::util_logToString(wxString * pString, bool bTrapEverything)
{
	m_bTrapEverything = bTrapEverything;
	m_pString = pString;

	if (wxThread::IsMain())
		m_pLogPrev = wxLog::SetActiveTarget(this);
	else
		m_pLogPrev = wxLog::SetThreadActiveTarget(this);
}

util_logToString::~util_logToString(void)
{
	if (wxThread::IsMain())
		wxLog::SetActiveTarget(m_pLogPrev);
	else
		wxLog::SetThreadActiveTarget(m_pLogPrev);
}

//////////////////////////////////////////////////////////////////

void util_logToString::DoLogTextAtLevel(wxLogLevel level,
										const wxString & msg)
{
	switch (level)
	{
	case wxLOG_Debug:
	case wxLOG_Trace:
		if (m_bTrapEverything)
		{
			*m_pString += msg;
		}
		else
		{
			wxLog::DoLogTextAtLevel(level, msg);
		}
		break;
		
	default:
		*m_pString += msg;
		break;
	}
}
