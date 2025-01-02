// util_background_thread_helper.h
// A class to help manage a background thread and report progess
// to the GUI.
//
// Parts of this class runs in the *MAIN* thread and other parts
// run in the *BACKGROUND* thread.
// 
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_BACKGROUND_THREAD_HELPER_H
#define H_UTIL_BACKGROUND_THREAD_HELPER_H

//////////////////////////////////////////////////////////////////

class util_background_thread_helper : public wxThreadHelper
{
public:
	//
	//      (c)        (r)       (ctx)       (x)
	//  V -------> C -------> S -------> R --------+----------> F(OK)
	//             |          |          |         |
	//             |          |          |         +----------> F(ERROR)
	//         (e) |      (k) |      (k) |
	//             |          |          |
	//             |          V          V
	//             |         KRQ        KRQ
	//             |          |          |
	//             |    (ctx) |      (x) |
	//             |      (x) |          V
	//             |          |      F(ABORTED)
	//             V          V
	//         F(NOTRUN)  F(NOTRUN)
	//
	typedef enum _MyThreadState { MTS__VOID              = 0x0000,
								  MTS__CREATED           = 0x0001,
								  MTS__STARTED           = 0x0002,
								  MTS__RUNNING           = 0x0003,
								  MTS__KILL_REQUESTED    = 0x0004,
								  MTS__TERMINAL__MASK    = 0x1000,
								  MTS__FINISHED_OK       = MTS__TERMINAL__MASK + 0x0001,
								  MTS__FINISHED_ERROR    = MTS__TERMINAL__MASK + 0x0002,
								  MTS__FINISHED_ABORTED  = MTS__TERMINAL__MASK + 0x0003,
								  MTS__FINISHED_NOT_RUN  = MTS__TERMINAL__MASK + 0x0004,
								  MTS__FINISHED_INVALID  = MTS__TERMINAL__MASK + 0x0005,
	} MyThreadState;

	typedef enum _MyThreadExitCode { MTEC__OK = 0,
									 MTEC__ERROR = 1,
									 MTEC__ABORT = 2
	} MyThreadExitCode;		// cast this to wxThread::ExitCode
		
public:
	util_background_thread_helper();
	virtual ~util_background_thread_helper(void);

	util_error create_and_run(void);		// called by the GUI thread

	// set*() are called by the background thread to
	// report progress.  THEY DO NOT UPDATE THE GUI.
	// They only set fields that the GUI thread can
	// see.
	void setProgressMessage(const wxString & strMsg);	// called by BG thread to report progress
	void setProgress(int numerator, int denominator);	// called by BG thread to report progress

	MyThreadState setThreadState(MyThreadState mts);					// called by BG or GUI thread
	MyThreadState getThreadState(int * pcProgress = NULL,				// called by BG or GUI thread
								 int * pNumerator = NULL,
								 int * pDenominator = NULL,
								 wxString * pStrMsg = NULL,
								 util_error * pue = NULL);

	MyThreadState setThreadResult(MyThreadState mts, const util_error & ue);		// called by BG or GUI thread

protected:
	virtual wxThread::ExitCode	DoWork(void) = 0;

private:
	virtual wxThread::ExitCode	Entry();	// BG thread entry point

protected:
	wxCriticalSection			m_cs;
	MyThreadState				m_mts;
	int							m_cProgress;
	int							m_numerator;
	int							m_denominator;
	wxString					m_strMsg;
	util_error					m_ueResult;
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_BACKGROUND_THREAD_HELPER_H
