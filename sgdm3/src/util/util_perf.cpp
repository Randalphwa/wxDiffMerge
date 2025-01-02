// util_perf.cpp -- performance timing routines
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

#ifdef DEBUGUTILPERF

util_perf_item::util_perf_item(void)
	: m_uqSum(0), m_uqCount(0), m_uqLastUse(0), m_bStarted(false)
{
}

util_perf_item::util_perf_item(const util_perf_item & upi)
	: m_uqSum(upi.m_uqSum),
	  m_uqCount(upi.m_uqCount),
	  m_uqStartingTime(upi.m_uqStartingTime),
	  m_uqLastUse(upi.m_uqLastUse),
	  m_bStarted(upi.m_bStarted)
{
}

//////////////////////////////////////////////////////////////////

void util_perf_item::startClock(void)
{
	wxASSERT_MSG( (!m_bStarted), _T("Coding Error") );

#if defined(__WXMSW__)
	QueryPerformanceCounter((LARGE_INTEGER *)&m_uqStartingTime);
#elif defined(__WXMAC__)
	m_uqStartingTime = mach_absolute_time();
#endif

	m_bStarted = true;
}

void util_perf_item::stopClock(void)
{
	wxASSERT_MSG( (m_bStarted), _T("Coding Error") );

#if defined(__WXMSW__)
	INT64 uqEnd;
	QueryPerformanceCounter((LARGE_INTEGER *)&uqEnd);
#elif defined(__WXMAC__)
	uint64_t uqEnd = mach_absolute_time();
#endif
	
	m_uqLastUse = (uqEnd - m_uqStartingTime);
	m_uqSum += m_uqLastUse;
	m_uqCount++;

	m_bStarted = false;
}

void util_perf_item::resetClock(void)
{
	m_uqSum = 0;
	m_uqCount = 0;
	m_uqLastUse = 0;
	m_bStarted = false;
}
	
void util_perf_item::dump(const wxChar * szKey, double dFreq)
{
	if (m_bStarted)
		wxLogTrace(TRACE_PERF, _T("%50s: running"),szKey);
	else
		wxLogTrace(TRACE_PERF, _T("%50s: [count %10d][total time %14.10f][ave time %14.10f][last time %14.10f]"),
				   szKey,(int)m_uqCount,
				   (((double)m_uqSum) / dFreq),
				   (((double)m_uqSum) / (dFreq * (double)m_uqCount)),
				   (((double)m_uqLastUse) / dFreq));
}

//////////////////////////////////////////////////////////////////
	
util_perf::util_perf(void)
{
#if defined(__WXMSW__)
	INT64 uqFreq;
	QueryPerformanceFrequency((LARGE_INTEGER *)&uqFreq);
	m_dFreq = (double)uqFreq;
#elif defined(__WXMAC__)
	mach_timebase_info_data_t	timebaseinfo;
	mach_timebase_info(&timebaseinfo);
	m_dFreq = 1.0e9 * (double)timebaseinfo.denom / (double)timebaseinfo.numer;
	wxLogTrace(TRACE_PERF,_T("util_perf:timerbaseinfo [numer %d][denom %d] computed [freq %g]"),
			   timebaseinfo.numer,timebaseinfo.denom,m_dFreq);
#endif
}

//////////////////////////////////////////////////////////////////

void util_perf::startClock(const wxChar * szKey)
{
	MapIterator it = m_map.find(szKey);
	if (it == m_map.end())
		it = m_map.insert( MapValue(szKey,util_perf_item()) ).first;

	it->second.startClock();
}

void util_perf::stopClock(const wxChar * szKey)
{
	MapIterator it = m_map.find(szKey);
	wxASSERT_MSG( (it != m_map.end()), _T("Coding Error") );

	it->second.stopClock();
}

void util_perf::resetClock(const wxChar * szKey)
{
	MapIterator it = m_map.find(szKey);
	wxASSERT_MSG( (it != m_map.end()), _T("Coding Error") );

	it->second.resetClock();
}

void util_perf::dump(const wxChar * szKey)
{
	MapIterator it = m_map.find(szKey);
	wxASSERT_MSG( (it != m_map.end()), _T("Coding Error") );

	it->second.dump(szKey,m_dFreq);
}

void util_perf::dumpAll(const wxChar * szLabel)
{
	wxLogTrace(TRACE_PERF, _T("PERF:DumpAll: %s"), szLabel);
	for (MapIterator it=m_map.begin(); (it != m_map.end()); it++)
		it->second.dump(it->first,m_dFreq);
}

void util_perf::resetAll(void)
{
	wxLogTrace(TRACE_PERF, _T("PERF:ResetAll"));
	for (MapIterator it=m_map.begin(); (it != m_map.end()); it++)
		it->second.resetClock();
}

#endif//DEBUGUTILPERF
