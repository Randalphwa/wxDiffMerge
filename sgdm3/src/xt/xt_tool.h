// xt_tool.h
// information for representing an external tool.  an external tool
// is a third-party tool that the user would like to use instead of
// us for certain types of files.  this would let them double click
// on a pair of XML files in a folder window and have us spawn a
// real xml gui diff tool, rather than use our text-mode file window.
//////////////////////////////////////////////////////////////////

#ifndef H_XT_TOOL_H
#define H_XT_TOOL_H

BEGIN_EXTERN_C

//////////////////////////////////////////////////////////////////

typedef unsigned long XT_ID;

//////////////////////////////////////////////////////////////////

class xt_tool
{
public:
	xt_tool(void);
	xt_tool(const xt_tool & xt);		// copy constructor

	~xt_tool(void);

	xt_tool *					clone(void) const;

	bool						isEqual(const xt_tool * pxt) const;

	void						setName(const wxString & str)		{ m_strName = str; };
	void						setSuffixes(const wxString & str)	{ m_strSuffixes = str; };
	void						setEnabled2(bool bEnabled)			{ m_bEnabled2 = bEnabled; };
	void						setEnabled3(bool bEnabled)			{ m_bEnabled3 = bEnabled; };
	void						setGui2Exe(const wxString & str)	{ m_strGui2Exe = str; };
	void						setGui2Args(const wxString & str)	{ m_strGui2Args = str; };
	void						setGui3Exe(const wxString & str)	{ m_strGui3Exe = str; };
	void						setGui3Args(const wxString & str)	{ m_strGui3Args = str; };

	inline const wxString &		getName(void)		const { return m_strName; };
	inline const wxString &		getSuffixes(void)	const { return m_strSuffixes; };
	inline bool					getEnabled2(void)	const { return m_bEnabled2; };
	inline bool					getEnabled3(void)	const { return m_bEnabled3; };
	inline const wxString &		getGui2Exe(void)	const { return m_strGui2Exe; };
	inline const wxString &		getGui2Args(void)	const { return m_strGui2Args; };
	inline const wxString &		getGui3Exe(void)	const { return m_strGui3Exe; };
	inline const wxString &		getGui3Args(void)	const { return m_strGui3Args; };

	bool						testPathnameSuffix(const poi_item * pPoi) const;

	inline bool					doIDsMatch(const xt_tool * pxt)	const { return m_ID == pxt->m_ID; };
	inline XT_ID				getID(void)						const { return m_ID; };

	wxString					dumpSupportInfo(void) const;

public:
	static void					apply_left_path(wxString & strArgs, const wxChar * szValue);
	static void					apply_left_title(wxString & strArgs, const wxChar * szValue);
	static void					apply_right_path(wxString & strArgs, const wxChar * szValue);
	static void					apply_right_title(wxString & strArgs, const wxChar * szValue);

	static void					apply_working_path(wxString & strArgs, const wxChar * szValue);
	static void					apply_working_title(wxString & strArgs, const wxChar * szValue);
	static void					apply_baseline_path(wxString & strArgs, const wxChar * szValue);
	static void					apply_other_path(wxString & strArgs, const wxChar * szValue);
	static void					apply_other_title(wxString & strArgs, const wxChar * szValue);
	static void					apply_destination_path(wxString & strArgs, const wxChar * szValue);
	static void					apply_destination_title(wxString & strArgs, const wxChar * szValue);

private:
	XT_ID						m_ID;

	wxString					m_strName;		// user visible name
	wxString					m_strSuffixes;	// space delimited list of suffixes

	bool						m_bEnabled2;	// is this one currently turned on for 2-way diffs?
	bool						m_bEnabled3;	// is this one currently turned on for 3-way merges?

	wxString					m_strGui2Exe;	// pathname of exe for GUI tool for 2-way diffs
	wxString					m_strGui2Args;	// argument template for GUI tool

	wxString					m_strGui3Exe;	// pathname of exe for GUI tool for 3-way merges
	wxString					m_strGui3Args;	// argument template for GUI tool

	// TODO do we need a different set of batch mode exe's and args???

#ifdef DEBUG
public:
	void						dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

END_EXTERN_C

#endif//H_XT_TOOL_H
