// Doc.cpp
// trivial wrapper for actual "doc's"
// -- (either 2 files, 3 files, or 2 directories).
// -- a convenience for gui_frame.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <fs.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

Doc::Doc(void)
	: m_pFdFd(NULL),
	  m_pFsFs(NULL)
{
}

//////////////////////////////////////////////////////////////////

Doc::~Doc(void)
{
	if (m_pFdFd)	gpFdFdTable->unRef(m_pFdFd);
	if (m_pFsFs)	gpFsFsTable->unRef(m_pFsFs);
}

//////////////////////////////////////////////////////////////////

void Doc::initFolderDiff(const wxString & path0, const wxString & path1)
{
	wxASSERT_MSG( (m_pFdFd==NULL), _T("Coding Error: Doc::initFolderDiff -- m_pFdFd already set.") );
	wxASSERT_MSG( (m_pFsFs==NULL), _T("Coding Error: Doc::initFolderDiff -- m_pFsFs already set.") );

	m_pFdFd = gpFdFdTable->create(path0,path1);		// create-if-necessary and addRef()
}

//////////////////////////////////////////////////////////////////

void Doc::initFileDiff(const wxString & path0, const wxString & path1, const cl_args * pArgs)
{
	wxASSERT_MSG( (m_pFdFd==NULL), _T("Coding Error: Doc::initFileDiff -- m_pFdFd already set.") );
	wxASSERT_MSG( (m_pFsFs==NULL), _T("Coding Error: Doc::initFileDiff -- m_pFsFs already set.") );

	m_pFsFs = gpFsFsTable->create(path0,path1,pArgs);			// create-if-necessary and addRef()
}

//////////////////////////////////////////////////////////////////

void Doc::initFileMerge(const wxString & path0, const wxString & path1, const wxString & path2, const cl_args * pArgs)
{
	wxASSERT_MSG( (m_pFdFd==NULL), _T("Coding Error: Doc::initFileDiff -- m_pFdFd already set.") );
	wxASSERT_MSG( (m_pFsFs==NULL), _T("Coding Error: Doc::initFileDiff -- m_pFsFs already set.") );

	m_pFsFs = gpFsFsTable->create(path0,path1,path2,pArgs);		// create-if-necessary and addRef()
}
