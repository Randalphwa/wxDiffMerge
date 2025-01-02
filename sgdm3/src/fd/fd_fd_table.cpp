// fd_fd_table.cpp
// a container of fd_fd.
// we maintain a central table of all fd_fd (folderdiff docs)
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>

//////////////////////////////////////////////////////////////////

fd_fd_table::fd_fd_table(void)
	: m_reftab( _T("fd_fd_table"), true)
{
}

fd_fd_table::~fd_fd_table(void)
{
	// map keys are pointers that we allocated, so we're responsible for
	// deleteing them before the map gets destroyed.
	//
	// but this table should be empty before we get destroyed since all
	// windows should have been closed before the app cleans up globals,
	// so we set 'bRequireEmptyOnDestroy' in util_reftab.
}

//////////////////////////////////////////////////////////////////

fd_fd * fd_fd_table::create(const wxString & path0, const wxString & path1)
{
	poi_item * pPoi0 = gpPoiItemTable->addItem(path0,_T(""));
	poi_item * pPoi1 = gpPoiItemTable->addItem(path1,_T(""));

	fd_fd * pFdFd = find(pPoi0,pPoi1);
	if (!pFdFd)
	{
		// if not found, create a new one for them.
		// 
		// note: the call to fd_fd::setFolders() just stores the directory
		// note: paths, but does not acutally treewalk -- that is done later.

		pFdFd = new fd_fd();
		pFdFd->setFolders(pPoi0,pPoi1);
	}
	
	/*int refCnt =*/ m_reftab.addRef(pFdFd);

//	wxLogTrace(TRACE_FD_DUMP, _T("fd_fd_table::find_or_create: [%p,%d]==[%s,%s]"),pFdFd,refCnt,path0.wc_str(),path1.wc_str());

	return pFdFd;
}

//////////////////////////////////////////////////////////////////

struct _find_param
{
	poi_item *		pPoi0;
	poi_item *		pPoi1;
};

static bool _apply_find_cb(void * /*pVoid_This*/, void * pVoid_FindParam, void * pVoid_Object_Candidate)
{
	fd_fd * pFdFd   = (fd_fd *)pVoid_Object_Candidate;
	struct _find_param * pFindParam = (struct _find_param *)pVoid_FindParam;
	
	return ( (pFdFd->getRootPoi(0) == pFindParam->pPoi0) && (pFdFd->getRootPoi(1) == pFindParam->pPoi1) );
}

fd_fd * fd_fd_table::find(const wxString & path0, const wxString & path1)
{
	// find fd_fd that is rooted at these paths.

	return find(gpPoiItemTable->addItem(path0,_T("")),
				gpPoiItemTable->addItem(path1,_T("")));
}

fd_fd * fd_fd_table::find(poi_item * pPoi0, poi_item * pPoi1)
{
	// find fd_fd that is rooted at these paths.

	struct _find_param findParam;
	findParam.pPoi0 = pPoi0;
	findParam.pPoi1 = pPoi1;
	
	fd_fd * pFdFd = (fd_fd *)m_reftab.apply(_apply_find_cb,this,&findParam);

	return pFdFd;
}

//////////////////////////////////////////////////////////////////
