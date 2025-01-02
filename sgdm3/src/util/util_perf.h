// util_perf.h -- performance timing routines
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_PERF_H
#define H_UTIL_PERF_H

//////////////////////////////////////////////////////////////////

#ifdef DEBUGUTILPERF	// QueryPerformanceCounter() is Win32-only

#if defined(__WXMAC__)
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#endif

class util_perf_item
{
public:
	util_perf_item(void);
	util_perf_item(const util_perf_item & upi);

	void startClock(void);
	void stopClock(void);
	void resetClock(void);
	void dump(const wxChar * szKey, double dFreq);
	
public:	
#if defined(__WXMSW__)
	INT64			m_uqSum, m_uqCount, m_uqStartingTime, m_uqLastUse;
#elif defined(__WXMAC__)
	uint64_t		m_uqSum, m_uqCount, m_uqStartingTime, m_uqLastUse;
#endif
	bool			m_bStarted;
};
	
class util_perf
{
public:
	util_perf(void);

	void startClock(const wxChar * szKey);
	void stopClock(const wxChar * szKey);
	void resetClock(const wxChar * szKey);
	void dump(const wxChar * szKey);
	void dumpAll(const wxChar * szLabel);
	void resetAll(void);
	
private:
	// we create a hash on the pointer value of the key,
	// so be sure to use static pointers for keys rather
	// than wxString values.
	typedef std::map<const wxChar *,util_perf_item>			Map;
	typedef Map::value_type									MapValue;
	typedef Map::iterator									MapIterator;
	typedef std::pair<MapIterator,bool>						MapInsertResult;

	Map				m_map;
	double			m_dFreq;
};

extern util_perf 		gUtilPerf;

#define UTIL_PERF_START_CLOCK(szKey)		Statement( gUtilPerf.startClock((szKey)); )
#define UTIL_PERF_STOP_CLOCK(szKey)			Statement( gUtilPerf.stopClock((szKey)); )
#define UTIL_PERF_RESET_CLOCK(szKey)		Statement( gUtilPerf.resetClock((szKey)); )
#define UTIL_PERF_DUMP(szKey)				Statement( gUtilPerf.dump((szKey)); )
#define UTIL_PERF_DUMP_ALL(szKey)			Statement( gUtilPerf.dumpAll((szKey)); )
#define UTIL_PERF_RESET_ALL()				Statement( gUtilPerf.resetAll(); )

#else//DEBUGUTILPERF

#define UTIL_PERF_START_CLOCK(szKey)		do {} while (0)
#define UTIL_PERF_STOP_CLOCK(szKey)			do {} while (0)
#define UTIL_PERF_RESET_CLOCK(szKey)		do {} while (0)
#define UTIL_PERF_DUMP(szKey)				do {} while (0)
#define UTIL_PERF_DUMP_ALL(szKey)			do {} while (0)
#define UTIL_PERF_RESET_ALL()				do {} while (0)

//////////////////////////////////////////////////////////////////

#endif//DEBUGUTILPERF
#endif//H_UTIL_PERF_H
