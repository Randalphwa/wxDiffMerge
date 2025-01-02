// xt_tool_table.h
// a container to hold all of the defined external tools.
// there should only be one global instance of this class
// and a temporary copy while the OptionsDialog is up.
// Individually configured tools are stored in the per-user profile.
//////////////////////////////////////////////////////////////////

#ifndef H_XT_TOOL_TABLE_H
#define H_XT_TOOL_TABLE_H

BEGIN_EXTERN_C

//////////////////////////////////////////////////////////////////

class xt_tool_table
{
public:
	xt_tool_table(void);
	xt_tool_table(const xt_tool_table & xtt);		// copy constructor
	~xt_tool_table(void);

	void						OnInit(bool bBuiltinOnly=false);

	void						addTool(xt_tool * pxt);
	void						replaceTool(int index, xt_tool * pxtNew, bool bDelete=true);
	void						deleteTool(int index);
	void						moveToolUpOne(int index);
	void						moveToolDownOne(int index);
	int							getIndex(const xt_tool * pxt) const;

	inline int					getCountTools(void)		const { return (int)m_vec.size(); };
	inline const xt_tool *		getNthTool(int n)		const { return ((n >= getCountTools()) ? NULL : m_vec[n]); };

	int							allocateArrayOfNames(wxString ** array) const;
	void						freeArrayOfNames(wxString * array) const;

//	const xt_tool *				findExternalTool(int nrPoi,
//												 poi_item * pPoiTable[]) const;
	const xt_tool *				findExternalTool(int nrParams,
												 const wxString & s0,
												 const wxString & s1,
												 const wxString & s2) const;
	const xt_tool *				findExternalTool(const wxString & s0,
												 const wxString & s1,
												 const wxString & s2,
												 const wxString & sResultPathname) const;

	void						doExport(void) const;

	wxString					dumpSupportInfo(void) const;

private:
	bool						_doImport(const wxString & strSaved);
	const xt_tool *				_do_automatic_best_guess(int nrPoi, poi_item * pPoiTable[]) const;
	void						_load_builtin_tools(void);
	void						_exportTool(wxDataOutputStream & dos, const xt_tool * pXT, int index) const;

private:
	typedef std::vector<xt_tool *>				TVector_Tools;
	typedef TVector_Tools::value_type			TVector_ToolsValue;
	typedef TVector_Tools::iterator				TVector_ToolsIterator;
	typedef TVector_Tools::const_iterator		TVector_ToolsConstIterator;

	TVector_Tools				m_vec;

#ifdef DEBUG
public:
	void						dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

END_EXTERN_C

#endif//H_XT_TOOL_TABLE_H
