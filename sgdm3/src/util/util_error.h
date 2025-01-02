// util_error.h
// define error codes.
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_ERROR_H
#define H_UTIL_ERROR_H

//////////////////////////////////////////////////////////////////

class util_error
{
public:
	typedef enum _err {
#define UE(id,msg)		UE_##id,
#include <util_error__defs.h>
		__UE__COUNT__		// must be last
#undef UE
	} err;

public:
	util_error(void)
		: m_err(UE_OK)
		{};

	util_error(err ue)
		: m_err(ue)
		{};
			
	util_error(err ue, wxString strExtraInfo)
		: m_err(ue),
		  m_strExtraInfo(strExtraInfo)
		{};

	void				set(err ue, wxString strExtraInfo);
	void				set(err ue);

	void				clear(void);

	static wxString		getMessage(err ue);

	inline err			getErr(void)		const	{ return m_err; };
	inline wxString		getMessage(void)	const	{ return util_error::getMessage(m_err); };
	inline wxString		getExtraInfo(void)	const	{ return m_strExtraInfo; };
	inline wxString &	refExtraInfo(void)			{ return m_strExtraInfo; };

	inline bool			isOK(void)			const	{ return (m_err == UE_OK); };
	inline bool			isErr(void)			const	{ return (m_err != UE_OK); };

	inline wxString		getMBMessage(void)	const	{ return getMessage() + _T("\r\n\r\n") + getExtraInfo(); };

private:
	err					m_err;
	wxString			m_strExtraInfo;		// optional info (like wxWidgets err/log msg)
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_ERROR_H
