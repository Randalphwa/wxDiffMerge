// util_shell_lnk.h
// wrapper for dealing with Windows Shortcuts (.lnk) files.
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_SHELL_LNK_H
#define H_UTIL_SHELL_LNK_H

#if defined(__WXMSW__)
//////////////////////////////////////////////////////////////////

class util_shell_lnk
{
public:
	static util_error ctor(const wxString & pathname,
						   util_shell_lnk ** ppLnk);

	~util_shell_lnk(void);

#if defined(DEBUG)
	wxString debug_dump(const wxString & strLabel) const;
#endif

//	bool equal(const util_shell_lnk * pLnkOther) const;

private:
	util_shell_lnk(void);
	void get_display_name(void);

public:
	wxString * pStrArguments;
	wxString * pStrDescription;
	wxString * pStrDisplayName;
	wxString * pStrIDListPath;
	wxString * pStrTargetPath;
	wxString * pStrWorkingDirectory;
	wxIcon * pIcon;

	void * pVoid_pidl;

};

//////////////////////////////////////////////////////////////////
#endif//__WXMSW__

#endif//H_UTIL_SHELL_LNK_H
