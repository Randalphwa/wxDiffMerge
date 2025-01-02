// util_background_thread_helper.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

util_background_thread_helper::util_background_thread_helper()
	: m_mts(MTS__VOID),
	  m_cProgress(0),
	  m_numerator(-1),
	  m_denominator(-1)
{
	wxCriticalSectionLocker lock(m_cs);
//	wxLogTrace(wxTRACE_Messages, _T("bg:ctor: %p %d"), this, m_mts);
}

util_background_thread_helper::~util_background_thread_helper(void)
{
	wxCriticalSectionLocker lock(m_cs);
//	wxLogTrace(wxTRACE_Messages, _T("bg:dtor: %p %d"), this, m_mts);

//	switch (m_mts)
//	{
//	case MTS__VOID:
//	case MTS__CREATED:
//		break;
//
//	case MTS__RUNNING:
//		// should we assert
//		break;
//	case MTS__RUNNING_KILL_REQUESTED:
//		// should we assert
//		break;
//
//	case MTS__FINISHED_OK:
//	case MTS__FINISHED_ERROR:
//	case MTS__FINISHED_ABORTED:
//	case MTS__FINISHED_NOT_RUN:
//		break;
//	}
}

//////////////////////////////////////////////////////////////////

util_error util_background_thread_helper::create_and_run(void)
{
	wxCriticalSectionLocker lock(m_cs);
	m_ueResult.clear();

	if (m_mts != MTS__VOID)
	{
		m_ueResult.set(util_error::UE_CANNOT_CREATE_RUN_THREAD, _T("Already started"));
		return m_ueResult;
	}

	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
	{
		m_ueResult.set(util_error::UE_CANNOT_CREATE_RUN_THREAD, _T("CreateThread failed"));
		return m_ueResult;
	}

	m_mts = MTS__CREATED;

	if (GetThread()->Run() != wxTHREAD_NO_ERROR)
	{
		m_mts = MTS__FINISHED_NOT_RUN;
		m_ueResult.set(util_error::UE_CANNOT_CREATE_RUN_THREAD, _T("RunThread failed"));
		return m_ueResult;
	}

	m_mts = MTS__STARTED;

	return m_ueResult;
}

//////////////////////////////////////////////////////////////////

void util_background_thread_helper::setProgressMessage(const wxString & strMsg)
{
	wxCriticalSectionLocker lock(m_cs);
	m_strMsg = strMsg;
	m_cProgress++;
}

void util_background_thread_helper::setProgress(int numerator, int denominator)
{
	wxCriticalSectionLocker lock(m_cs);
	m_numerator = numerator;
	m_denominator = denominator;
	m_cProgress++;
}

//////////////////////////////////////////////////////////////////

util_background_thread_helper::MyThreadState
util_background_thread_helper::setThreadState(util_background_thread_helper::MyThreadState mts)
{
	wxCriticalSectionLocker lock(m_cs);
//	wxLogTrace(wxTRACE_Messages, _T("bg:state: %p [%d --> %d]"), this, m_mts, mts);

	MyThreadState old_mts = m_mts;
	m_mts = mts;

	return old_mts;
}

util_background_thread_helper::MyThreadState
util_background_thread_helper::setThreadResult(MyThreadState mts,
											   const util_error & ue)
{
	wxCriticalSectionLocker lock(m_cs);
//	wxLogTrace(wxTRACE_Messages, _T("bg:result: %p [%d --> %d][err %d]"), this, m_mts, mts, ue.getErr());

	MyThreadState old_mts = m_mts;
	m_mts = mts;
	m_ueResult.set( ue.getErr(), ue.getExtraInfo() );

	return old_mts;
}

util_background_thread_helper::MyThreadState
util_background_thread_helper::getThreadState(int * pcProgress,
											  int * pNumerator,
											  int * pDenominator,
											  wxString * pStrMsg,
											  util_error *pUE)
{
	wxCriticalSectionLocker lock(m_cs);
	if (pcProgress)
		*pcProgress = m_cProgress;
	if (pNumerator)
		*pNumerator = m_numerator;
	if (pDenominator)
		*pDenominator = m_denominator;
	if (pStrMsg)
	{
		// This is basically a copy and could probably use
		// a plain operator=(), but I want to ensure that
		// we are not returning a reference to the m_cs-protected
		// string field.
		pStrMsg->Empty();
		pStrMsg->Append(m_strMsg);
	}
	if (pUE)
	{
		pUE->set( m_ueResult.getErr(), m_ueResult.getExtraInfo() );
	}
	return m_mts;
}

//////////////////////////////////////////////////////////////////

wxThread::ExitCode util_background_thread_helper::Entry()
{
	wxThread::ExitCode ec = 0;
	bool bDoRun = false;

	{
		wxCriticalSectionLocker lock(m_cs);
//		wxLogTrace(wxTRACE_Messages, _T("bg:entry:begin: %p %d --"), this, m_mts);
		if (m_mts == MTS__STARTED)
		{
			m_mts = MTS__RUNNING;
			bDoRun = true;
		}
		else if (m_mts == MTS__KILL_REQUESTED)
		{
			// GUI thread started us and immediately killed us
			// before we ever got a chance to start.  Not sure
			// this could happen (in practice), but don't freak.
			bDoRun = false;
			m_mts = MTS__FINISHED_NOT_RUN;
		}
		else
		{
			wxASSERT_MSG( (0), _T("Coding Error") );
			bDoRun = false;
			m_mts = MTS__FINISHED_INVALID;
		}
	}

	// release the lock before we actually any work.

	if (bDoRun)
	{
		ec = DoWork();
	}

#if defined(DEBUG)
	// Verify that the worker routine set a terminal state when it finished.
	{
		wxCriticalSectionLocker lock(m_cs);
//		wxLogTrace(wxTRACE_Messages, _T("bg:entry:end: %p %d --"), this, m_mts);
		wxASSERT_MSG( ((m_mts & MTS__TERMINAL__MASK) != 0), _T("Coding Error") );
	}
#endif

	return ec;
}
