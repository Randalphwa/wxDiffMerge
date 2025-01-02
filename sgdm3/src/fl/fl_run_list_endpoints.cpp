// fl_run_list_endpoints.cpp -- endpoints of a list of runs.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fl.h>

//////////////////////////////////////////////////////////////////

fl_run_list_endpoints::fl_run_list_endpoints(fl_run * pHead, fl_run * pTail)
	: m_pHead(pHead), m_pTail(pTail)
{
}

fl_run_list_endpoints::fl_run_list_endpoints(const fl_run_list_endpoints & ep)
	: m_pHead(ep.m_pHead), m_pTail(ep.m_pTail)
{
}

fl_run_list_endpoints::~fl_run_list_endpoints(void)
{
}

//////////////////////////////////////////////////////////////////

void fl_run_list_endpoints::setList(fl_run * pHead, fl_run * pTail)
{
	m_pHead = pHead;
	m_pTail = pTail;
}

//////////////////////////////////////////////////////////////////
