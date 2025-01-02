// Doc.h
// trivial wrapper for actual "doc's"
// -- (either 2 files, 3 files, or 2 directories).
// -- a convenience for gui_frame.
//////////////////////////////////////////////////////////////////

#ifndef H_DOC_H
#define H_DOC_H

//////////////////////////////////////////////////////////////////

class Doc
{
public:
	Doc(void);
	~Doc(void);

	void			initFolderDiff(const wxString & path0, const wxString & path1);
	void			initFileDiff(const wxString & path0, const wxString & path1, const cl_args * pArgs);
	void			initFileMerge(const wxString & path0, const wxString & path1, const wxString & path2, const cl_args * pArgs);

	inline fd_fd *	getFdFd(void)			const { return m_pFdFd; };
	inline fs_fs *	getFsFs(void)			const { return m_pFsFs; };

protected:
	fd_fd *			m_pFdFd;	// valid when we refer to a folderdiff doc set.  we only own a reference.
	fs_fs *			m_pFsFs;	// valid when we refer to a file diff/merge doc set.  we only own a reference.
};

//////////////////////////////////////////////////////////////////

#endif//H_DOC_H
