// fl_run.cpp -- a run -- a span of text to be displayed within a
// line with the same properties.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fl.h>

//////////////////////////////////////////////////////////////////

fl_run::fl_run(fl_line * pLine,
			   const fim_frag * pFrag, fr_offset offset, fim_length len, fr_prop prop)
	: m_next(NULL), m_prev(NULL),
	  m_pLine(pLine),
	  m_pFrag(pFrag), m_offsetFrag(offset), m_lenInFrag(len), m_propFrag(prop)
{
	DEBUG_CTOR(T_FL_RUN,L"fl_run");
}

fl_run::~fl_run(void)
{
	DEBUG_DTOR(T_FL_RUN);
}

//////////////////////////////////////////////////////////////////

const wxChar * fl_run::getContent(void) const	// returns a TEMPORARY DATA POINTER
{
	const wxChar * pBuf	= m_pFrag->getTemporaryDataPointer(m_offsetFrag);

	return pBuf;
}

bool fl_run::isTAB(void) const
{
	if (m_lenInFrag != 1) return false;

	const wxChar * pBuf	= getContent();

	return (*pBuf == 0x0009);
}

bool fl_run::isLF(void) const
{
	if (m_lenInFrag != 1) return false;

	const wxChar * pBuf	= getContent();

	return (*pBuf == 0x000a);
}

bool fl_run::isCR(void) const
{
	if (m_lenInFrag != 1) return false;

	const wxChar * pBuf	= getContent();

	return (*pBuf == 0x000d);
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fl_run::dump(int indent) const
{
	wxASSERT_MSG( (m_pFrag), _T("Coding Error") );

	const wxChar * pVal;
	if (isLF())			pVal = _T("[LF]");
	else if (isCR())	pVal = _T("[CR]");
	else				pVal = _T("");	

	const wxChar * pBol = ((m_pLine->getFirstRunOnLine()==this) ? _T("BOL") : _T(""));

	wxLogTrace(TRACE_FLRUN_DUMP, _T("%*cFL_RUN: [%p][propFrag %x][frag %p][offset %ld][len %ld][line %p] %s %s"),
			   indent,' ',this,
			   m_propFrag,
			   m_pFrag,m_offsetFrag,m_lenInFrag,
			   m_pLine,
			   pVal,pBol);
	if (!pVal[0])
		util_print_buffer_cl(TRACE_FLRUN_DUMP,m_pFrag->getTemporaryDataPointer(m_offsetFrag),m_lenInFrag);
}
#endif
