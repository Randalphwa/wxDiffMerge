// fl_run_list.cpp -- a list of runs.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fl.h>

//////////////////////////////////////////////////////////////////

static const wxChar * _find_special(const wxChar * pBuf, fim_length lenBuf)
{
	// scan buffer for first eol marker (either a CR or a LF) or a TAB.

	for (fr_offset k=0; (k<lenBuf); k++)
	{
		switch (pBuf[k])
		{
		case 0x0009:			// TAB
		case 0x000a:			// LF
		case 0x000d:			// CR
			return &pBuf[k];
		default:
			break;
		}
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////

fl_run_list::fl_run_list(fl_run * pHead, fl_run * pTail)
	: fl_run_list_endpoints(pHead,pTail)
{
}

//////////////////////////////////////////////////////////////////

fl_run_list::fl_run_list(const fim_frag * pFragNew)
	: fl_run_list_endpoints()
{
	// build list of runs to represent the given fragment.

	fr_offset offset = 0;
	fim_length len   = pFragNew->getFragLength();

	while (len)
	{
		fim_length lenUsed;

		const wxChar * pBuf		= pFragNew->getTemporaryDataPointer(offset);
		const wxChar * pSpecial = _find_special(pBuf,len);

		if (!pSpecial)				lenUsed = len;					// looking at normal text (and no special char in remainder of this frag).
		else if (pSpecial==pBuf)	lenUsed = 1;					// looking at an special character.  put it into a new run (dos CRLF's will get 2 runs).
		else						lenUsed = (pSpecial - pBuf);	// looking at normal text followed by start of special.

		appendNewRun( new fl_run(NULL,pFragNew,offset,lenUsed,pFragNew->getFragProp()) );

		len -= lenUsed;					// account for portion of frag content consumed in this run
		offset += lenUsed;
	}
}

//////////////////////////////////////////////////////////////////

fl_run_list::~fl_run_list(void)
{
	deleteList();
}

//////////////////////////////////////////////////////////////////

void fl_run_list::deleteList(void)
{
	DELETE_LIST(fl_run,m_pHead);
	m_pTail = NULL;
}

//////////////////////////////////////////////////////////////////

fl_run * fl_run_list::appendNewRun(fl_run * pRunNew)
{
	wxASSERT_MSG( (pRunNew && pRunNew->isUnlinked()), _T("Coding Error") );

	pRunNew->m_next = NULL;
	pRunNew->m_prev = m_pTail;

	if (m_pTail)
		m_pTail->m_next = pRunNew;

	m_pTail = pRunNew;

	if (!m_pHead)
		m_pHead = pRunNew;
	
	return pRunNew;
}

//////////////////////////////////////////////////////////////////

void fl_run_list::appendList(fl_run_list & listNew)
{
	// concatenate given list onto end of our list.
	// ownership of individual runs is transferred to us.

	if (listNew.isEmpty())					// empty list, ignore it.
		return;

//	wxLogTrace(TRACE_FLRUN_DUMP, _T("fl_run_list::appendList: appending [%p...%p] onto list [%p...%p]"),
//			   listNew.m_pHead,listNew.m_pTail,m_pHead,m_pTail);

	if (isEmpty())							// if we are empty, simply clone
	{
		m_pHead = listNew.m_pHead;
		m_pTail = listNew.m_pTail;
	}
	else									// proper concatenation
	{
		wxASSERT_MSG( (        m_pTail->m_next==NULL), _T("Coding Error") );
		wxASSERT_MSG( (listNew.m_pHead->m_prev==NULL), _T("Coding Error") );
		wxASSERT_MSG( (listNew.m_pTail->m_next==NULL), _T("Coding Error") );

		m_pTail->m_next = listNew.m_pHead;
		listNew.m_pHead->m_prev = m_pTail;

		m_pTail = listNew.m_pTail;
	}

	listNew.setList(NULL,NULL);
}

void fl_run_list::insertListBeforeRun(fl_run_list & listNew, fl_run * pRunExisting)
{
	// insert the given list into the middle of our list immediately
	// prior to the given run.
	// ownership of individual runs is transferred to us.

	if (listNew.isEmpty())					// empty list, ignore it.
		return;

	wxASSERT_MSG( (!isEmpty()), _T("Coding Error") );

//	wxLogTrace(TRACE_FLRUN_DUMP, _T("fl_run_list::insertListBeforeRun: inserting [%p...%p] before [%p] in list [%p...%p]"),
//			   listNew.m_pHead,listNew.m_pTail,pRunExisting,m_pHead,m_pTail);

	fl_run * pRunExistingPrev = pRunExisting->m_prev;

	listNew.m_pTail->m_next = pRunExisting;
	pRunExisting->m_prev = listNew.m_pTail;

	listNew.m_pHead->m_prev = pRunExistingPrev;
	if (pRunExistingPrev)
		pRunExistingPrev->m_next = listNew.m_pHead;
	else
		m_pHead = listNew.m_pHead;

	listNew.setList(NULL,NULL);
}

void fl_run_list::insertListAfterRun(fl_run_list & listNew, fl_run * pRunExisting)
{
	// insert the given list into the middle of our list immediately
	// after to the given run.
	// ownership of individual runs is transferred to us.

	if (listNew.isEmpty())					// empty list, ignore it.
		return;

	wxASSERT_MSG( (!isEmpty()), _T("Coding Error") );

//	wxLogTrace(TRACE_FLRUN_DUMP, _T("fl_run_list::insertListAfterRun: inserting [%p...%p] after [%p] in list [%p...%p]"),
//			   listNew.m_pHead,listNew.m_pTail,pRunExisting,m_pHead,m_pTail);
	
	fl_run * pRunExistingNext = pRunExisting->m_next;

	listNew.m_pHead->m_prev = pRunExisting;
	pRunExisting->m_next = listNew.m_pHead;

	listNew.m_pTail->m_next = pRunExistingNext;
	if (pRunExistingNext)
		pRunExistingNext->m_prev = listNew.m_pTail;
	else
		m_pTail = listNew.m_pTail;

	listNew.setList(NULL,NULL);
}

fl_run_list * fl_run_list::extractSubList(fl_run_list_endpoints & ep)
{
	// extract a sub-list from somewhere within our list.
	// ownership of individual runs is transferred to the new list.
	// uses the given endpoints to demark the sub-list.
	// 
	// can be thought of as undoing an appendList(), insertList{Before,After}Run()
	// operation.

	if ( (ep.getHead() == m_pHead) && (ep.getTail() == m_pTail) )	// they want everything
	{
		setList(NULL,NULL);
	}
	else if (ep.getHead() == m_pHead)								// remove front portion of our list
	{
		m_pHead = ep.getTail()->m_next;
		m_pHead->m_prev = NULL;
		ep.getTail()->m_next = NULL;
	}
	else if (ep.getTail() == m_pTail)								// remove tail portion of our list
	{
		m_pTail = ep.getHead()->m_prev;
		m_pTail->m_next = NULL;
		ep.getHead()->m_prev = NULL;
	}
	else														// remove a portion from the middle of our list
	{
		fl_run * p1 = ep.getHead()->m_prev;
		fl_run * p2 = ep.getTail()->m_next;
		ep.getHead()->m_prev = NULL;
		ep.getTail()->m_next = NULL;
		p1->m_next = p2;
		p2->m_prev = p1;
	}
		
	return new fl_run_list(ep.getHead(), ep.getTail());		
}

fl_run * fl_run_list::splitRun(fl_run * pRun, fim_length len)
{
	// split the given run into 2 pieces and update the list.

	wxASSERT_MSG( (pRun && (len > 0) && (len < pRun->getLength())), _T("Coding Error: invalid split") );

	// give portion of content after split to a new run
	// and portion before split to existing run (by truncating it).

	fim_length oldLength = pRun->getLength();
	fl_run * pRunNew = new fl_run(pRun->getLine(), pRun->getFrag(), pRun->getFragOffset() + len, oldLength - len, pRun->getFragProp());
	pRun->m_lenInFrag = len;

//	wxLogTrace(TRACE_FLRUN_DUMP, _T("fl_run_list::splitRun: splitting [%p,%ld] into [%p,%ld] and [%p,%ld]"),
//			   pRun,oldLength, pRun,pRun->getLength(), pRunNew,pRunNew->getLength());

	// insert new run after existing run.

	fl_run * pRunNext = pRun->m_next;
	
	pRunNew->m_prev = pRun;
	pRun->m_next    = pRunNew;

	pRunNew->m_next = pRunNext;
	if (pRunNext)
		pRunNext->m_prev = pRunNew;
	else
		m_pTail = pRunNew;

	return pRunNew;
}
