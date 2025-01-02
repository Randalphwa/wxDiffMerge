// MyBusyInfo.cpp
// A class to manage marking the whole application busy
// in a safe, re-entrant way in the presence of background
// threads.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

MyBusyInfo::MyBusyInfo(void)
	: m_nrBusyWindows(0),
	  m_pBusyInfo(NULL),
	  m_pDisabler(NULL)
{
}

MyBusyInfo::~MyBusyInfo(void)
{
	if (m_pBusyInfo)
		delete m_pBusyInfo;
	if (m_pDisabler)
		delete m_pDisabler;
}

//////////////////////////////////////////////////////////////////

void MyBusyInfo::Increment(void)
{
	wxCriticalSectionLocker lock(m_cs);
	m_nrBusyWindows++;

	if (m_nrBusyWindows > 1)
	{
		wxASSERT_MSG( (m_pBusyInfo), _T("Coding Error") );
		wxASSERT_MSG( (m_pDisabler), _T("Coding Error") );
	}
	else if (m_nrBusyWindows == 1)
	{
		wxASSERT_MSG( (!m_pBusyInfo), _T("Coding Error") );
		wxASSERT_MSG( (!m_pDisabler), _T("Coding Error") );
		m_pBusyInfo = new wxBusyInfo("Please wait...");
		m_pDisabler = new wxWindowDisabler(true);
	}
	else
	{
		wxASSERT_MSG( (0), _T("Coding Error") );
	}
}

void MyBusyInfo::Decrement(void)
{
	wxCriticalSectionLocker lock(m_cs);
	m_nrBusyWindows--;

	if (m_nrBusyWindows > 0)
	{
		wxASSERT_MSG( (m_pBusyInfo), _T("Coding Error") );
		wxASSERT_MSG( (m_pDisabler), _T("Coding Error") );
	}
	else if (m_nrBusyWindows == 0)
	{
		wxASSERT_MSG( (m_pBusyInfo), _T("Coding Error") );
		wxASSERT_MSG( (m_pDisabler), _T("Coding Error") );

		delete m_pDisabler;
		m_pDisabler = NULL;
		delete m_pBusyInfo;
		m_pBusyInfo = NULL;
	}
	else
	{
		wxASSERT_MSG( (0), _T("Coding Error") );
	}
}
