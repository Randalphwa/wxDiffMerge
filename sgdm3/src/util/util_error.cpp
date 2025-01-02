// util_error.cpp
// a simple error code with generic message
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

void util_error::set(util_error::err ue, wxString strExtraInfo)
{
	m_err = ue;
	m_strExtraInfo = strExtraInfo;
}

void util_error::set(util_error::err ue)
{
	m_err = ue;
}

void util_error::clear(void)
{
	m_err = UE_OK;
	m_strExtraInfo.erase();
}

//////////////////////////////////////////////////////////////////

/*static*/ wxString util_error::getMessage(util_error::err ue)
{
	switch (ue)
	{
#define UE(id,msg)		case UE_##id: return msg;
#include <util_error__defs.h>
#undef UE

	default:		return _T("Unknown Error Code.");
	}
}

//////////////////////////////////////////////////////////////////

