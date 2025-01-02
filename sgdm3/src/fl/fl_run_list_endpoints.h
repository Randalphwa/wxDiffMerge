// fl_run_list_endpoints.h
// a "run list" is a doubly-linked list of fl_run's and is
// managed by fl_run_list.  an "endpoint" is a convenience
// wrapper for the head and tail.  an "endpoint" may be used
// to represent a subset of the full list.  fl_fl contains a
// fl_run_list that "owns" the contents of the list and whose
// endpoints refer to the entire list.  fl_fl maintains a map
// mapping fragments to the endpoints of the sequence of runs
// that refer to it.
//////////////////////////////////////////////////////////////////

#ifndef H_FL_RUN_LIST_ENDPOINTS_H
#define H_FL_RUN_LIST_ENDPOINTS_H

//////////////////////////////////////////////////////////////////

class fl_run_list_endpoints
{
public:
	fl_run_list_endpoints(fl_run * pHead=NULL, fl_run * pTail=NULL);
	fl_run_list_endpoints(const fl_run_list_endpoints & ep);
	virtual ~fl_run_list_endpoints(void);

	void				setList(fl_run * pHead=NULL, fl_run * pTail=NULL);

	inline fl_run *		getHead(void) const { return m_pHead; };
	inline fl_run *		getTail(void) const { return m_pTail; };

	inline bool			isEmpty(void) const { return ((m_pHead==NULL) && (m_pTail==NULL)); };
	
protected:
	fl_run *			m_pHead;
	fl_run *			m_pTail;
};

//////////////////////////////////////////////////////////////////

#endif//H_FL_RUN_LIST_ENDPOINTS_H
