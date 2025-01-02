// MyBusyInfo.h
// A class to manage marking the whole application busy
// in a safe, re-entrant way in the presence of background
// threads.
//////////////////////////////////////////////////////////////////

#ifndef H_MYBUSYINFO_H
#define H_MYBUSYINFO_H

//////////////////////////////////////////////////////////////////

class MyBusyInfo
{
public:
	MyBusyInfo(void);
	~MyBusyInfo(void);

	void Increment(void);
	void Decrement(void);

private:
	wxCriticalSection		m_cs;
	int						m_nrBusyWindows;
	wxBusyInfo *			m_pBusyInfo;
	wxWindowDisabler *		m_pDisabler;
};




//////////////////////////////////////////////////////////////////

#endif//H_MYBUSYINFO_H
