// fl_run_list.h -- a list of runs -- see fl_run_list_endpoints.
//////////////////////////////////////////////////////////////////

#ifndef H_FL_RUN_LIST_H
#define H_FL_RUN_LIST_H

//////////////////////////////////////////////////////////////////

class fl_run_list : public fl_run_list_endpoints
{
public:
	fl_run_list(fl_run * pHead=NULL, fl_run * pTail=NULL);
	fl_run_list(const fim_frag * pFragNew);
	virtual ~fl_run_list(void);

	void				deleteList(void);

	fl_run *			appendNewRun(fl_run * pRunNew);

	void				appendList(fl_run_list & listNew);
	void				insertListBeforeRun(fl_run_list & listNew, fl_run * pRunExisting);
	void				insertListAfterRun(fl_run_list & listNew, fl_run * pRunExisting);
	fl_run_list *		extractSubList(fl_run_list_endpoints & ep);

	fl_run *			splitRun(fl_run * pRun, fim_length len);
};

//////////////////////////////////////////////////////////////////

#endif//H_FL_RUN_LIST_H
