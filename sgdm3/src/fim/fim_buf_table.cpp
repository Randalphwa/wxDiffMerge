// fim_buf_table.cpp
// a container of fim_buf.
// there should only be one global instance of this table.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>

//////////////////////////////////////////////////////////////////

fim_buf_table::fim_buf_table(void)
	: m_reftab( _T("fim_buf_table"), true)
{
}

fim_buf_table::~fim_buf_table(void)
{
	// map keys are pointers that we allocated, so we're responsible for
	// deleteing them before the map gets destroyed.
	//
	// but this table should be empty before we get destroyed since all
	// windows should have been closed before the app cleans up globals,
	// so we set 'bRequireEmptyOnDestroy' in util_reftab.
}

//////////////////////////////////////////////////////////////////

fim_buf * fim_buf_table::create(fim_length lenPreallocateHint)
{
	fim_buf * pBuf = new fim_buf(lenPreallocateHint);

	int refCnt = m_reftab.addRef(pBuf);
	
	MY_ASSERT( (refCnt==1) );
//	wxLogTrace(TRACE_FIMBUF_DUMP, _T("fim_buf_table::create: [%p,%d]"), pBuf,refCnt);

	return pBuf;
}

//////////////////////////////////////////////////////////////////
