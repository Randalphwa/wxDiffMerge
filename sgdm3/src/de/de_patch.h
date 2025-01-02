// de_patch.h
// capture details of a patch so that we might apply it.
// builds upon the fim_patch.
//////////////////////////////////////////////////////////////////

#ifndef H_DE_PATCH_H
#define H_DE_PATCH_H

//////////////////////////////////////////////////////////////////

class de_patch : public fim_patch
{
public:
	de_patch(fim_patch_op op,
			 fim_offset docPosWhere,							// location in edit panel
			 fim_length lenDelete,								// length in edit panel to delete when DELETE or REPLACE
			 long lineNrFirstEdit, long lineNrLastEdit,			// 0-based line numbers in the edit buffer document
			 const wxString & strNewTextT0,	fr_prop propT0,		// content to INSERT from or REPLACE with from T0
			 long lineNrFirstT0,   long lineNrLastT0,			// 0-based line numbers in the source document T0
			 const wxString & strNewTextT2,	fr_prop propT2,		// content to INSERT from or REPLACE with from T2
			 long lineNrFirstT2,   long lineNrLastT2,			// 0-based line numbers in the source document T2
			 long rowNrStart, long rowNrEnd)					// 0-based display list row numbers
		: m_opOriginal(op),
		  m_docPosWhere(docPosWhere),
		  m_lenDelete(lenDelete),
		  m_lineNrT0First(lineNrFirstT0), m_lineNrT0Last(lineNrLastT0),
		  m_lineNrEditFirst(lineNrFirstEdit), m_lineNrEditLast(lineNrLastEdit),
		  m_lineNrT2First(lineNrFirstT2), m_lineNrT2Last(lineNrLastT2),
		  m_rowNrStart(rowNrStart), m_rowNrEnd(rowNrEnd),
		  m_propT0(propT0), m_propT2(propT2),
		  m_strNewT0(strNewTextT0), m_strNewT2(strNewTextT2)
		{
			m_opCurrent = op;

#ifdef DEBUG
			switch (m_opOriginal)
			{
			default:
				//case POP_IGNORE:
				wxASSERT_MSG( (0), _T("Coding Error") );
				break;

			case POP_DELETE:
				wxASSERT_MSG( (hasContentEdit()), _T("Coding Error") );
				break;

			case POP_INSERT_L:
				wxASSERT_MSG( (hasContentT0() && !hasContentEdit()), _T("Coding Error") );
				break;
				
			case POP_INSERT_R:
				wxASSERT_MSG( (!hasContentEdit() && hasContentT2()), _T("Coding Error") );
				break;

			case POP_REPLACE_L:
				wxASSERT_MSG( (hasContentT0() && hasContentEdit()), _T("Coding Error") );
				break;

			case POP_REPLACE_R:
				wxASSERT_MSG( (hasContentEdit() && hasContentT2()), _T("Coding Error") );
				break;

			case POP_CONFLICT:
				wxASSERT_MSG( (hasContentT0() || hasContentEdit() || hasContentT2()), _T("Coding Error") );
				break;
			}
#endif
		};

	virtual void				setPatchOpCurrent(fim_patch_op op)	  { m_opCurrent = op; };

	virtual fim_patch_op		getPatchOpCurrent(void)			const { return m_opCurrent; };
	virtual fim_patch_op		getPatchOpOriginal(void)		const { return m_opOriginal; };
	virtual fim_offset			getDocPosWhere(void)			const { return m_docPosWhere; };

	virtual fim_length			getLenDelete(void)				const
		{
			switch (m_opCurrent)
			{
			case POP_DELETE:
			case POP_REPLACE_L:
			case POP_REPLACE_R:
				return m_lenDelete;

			default:
				//case POP_IGNORE:
				//case POP_INSERT_L:
				//case POP_INSERT_R:
				//case POP_CONFLICT:
				wxASSERT_MSG( (0), _T("Coding Error") );
				return 0;
			}
		};

	virtual const wxChar *		getNewText(void)				const
		{
			switch (m_opCurrent)
			{
			case POP_INSERT_L:
			case POP_REPLACE_L:
				return m_strNewT0.wc_str();

			case POP_INSERT_R:
			case POP_REPLACE_R:
				return m_strNewT2.wc_str();

			default:
				//case POP_IGNORE:
				//case POP_DELETE:
				//case POP_CONFLICT:
				wxASSERT_MSG( (0), _T("Coding Error") );
				return NULL;
			}
		};

	virtual fim_length			getNewTextLen(void)				const
		{
			switch (m_opCurrent)
			{
			case POP_INSERT_L:
			case POP_REPLACE_L:
				return m_strNewT0.Length();

			case POP_INSERT_R:
			case POP_REPLACE_R:
				return m_strNewT2.Length();

			default:
				//case POP_IGNORE:
				//case POP_DELETE:
				//case POP_CONFLICT:
				wxASSERT_MSG( (0), _T("Coding Error") );
				return 0;
			}
		};

	virtual fr_prop				getProp(void)					const
		{
			switch (m_opCurrent)
			{
			case POP_INSERT_L:
			case POP_REPLACE_L:
				return m_propT0;

			case POP_INSERT_R:
			case POP_REPLACE_R:
				return m_propT2;

			default:
				//case POP_IGNORE:
				//case POP_DELETE:
				//case POP_CONFLICT:
				wxASSERT_MSG( (0), _T("Coding Error") );
				return FR_PROP_ZERO;
			}
		};

	inline long					getRowStart(void)				const { return m_rowNrStart; };
	inline long					getRowEnd(void)					const { return m_rowNrEnd; };

	inline bool					hasContentT0(void)				const { return ((m_lineNrT0First != -1)   && (m_lineNrT0Last != -1));   };
	inline bool					hasContentEdit(void)			const { return ((m_lineNrEditFirst != -1) && (m_lineNrEditLast != -1)); };
	inline bool					hasContentT2(void)				const { return ((m_lineNrT2First != -1)   && (m_lineNrT2Last != -1));   };

	inline long					getLineT0First(void)			const { return m_lineNrT0First; };
	inline long					getLineT0Last(void)				const { return m_lineNrT0Last; };
	inline long					getLineT1First(void)			const { return m_lineNrEditFirst; };
	inline long					getLineT1Last(void)				const { return m_lineNrEditLast; };
	inline long					getLineT2First(void)			const { return m_lineNrT2First; };
	inline long					getLineT2Last(void)				const { return m_lineNrT2Last; };

	//////////////////////////////////////////////////////////////////

	static wxString				format_lines_msg(long lineNrFirst, long lineNrLast)
		{
			if (lineNrFirst == -1)
				return _("End of File");
			else if ((lineNrFirst == lineNrLast) || (lineNrLast == -1))
				return wxString::Format( _("line %ld"), lineNrFirst+1);
			else
				return wxString::Format( _("lines %ld..%ld"), lineNrFirst+1, lineNrLast+1);
		};

	virtual wxString	format_edit_lines(void)					const
		{
			return de_patch::format_lines_msg(m_lineNrEditFirst,m_lineNrEditLast);
		};

	virtual wxString	format_src_lines(PanelIndex kPanel)		const
		{
			if (kPanel == PANEL_T0)
				return de_patch::format_lines_msg(m_lineNrT0First,m_lineNrT0Last);
			else
				return de_patch::format_lines_msg(m_lineNrT2First,m_lineNrT2Last);
		};

	virtual wxString	format_summary_msg(void)				const
		{
			switch (m_opCurrent)
			{
			default:
				return wxString();

			case POP_DELETE:
				return wxString::Format( _("Delete %s from center panel."), format_edit_lines().wc_str());
				
			case POP_INSERT_L:
				return wxString::Format(_("Insert %s from left panel at %s in center panel."),
										de_patch::format_lines_msg(m_lineNrT0First,m_lineNrT0Last).wc_str(),
										format_edit_lines().wc_str());

			case POP_INSERT_R:
				return wxString::Format(_("Insert %s from right panel at %s in center panel."),
										de_patch::format_lines_msg(m_lineNrT2First,m_lineNrT2Last).wc_str(),
										format_edit_lines().wc_str());

			case POP_REPLACE_L:
				return wxString::Format(_("Replace %s in center panel with %s from left panel."),
										format_edit_lines().wc_str(),
										de_patch::format_lines_msg(m_lineNrT0First,m_lineNrT0Last).wc_str());

			case POP_REPLACE_R:
				return wxString::Format(_("Replace %s in center panel with %s from right panel."),
										format_edit_lines().wc_str(),
										de_patch::format_lines_msg(m_lineNrT2First,m_lineNrT2Last).wc_str());

			case POP_IGNORE:
				return _("This change/conflict will be ignored by Auto-Merge.");

			case POP_CONFLICT:
				return _("This conflict cannot be automatically merged.");
			}
		};
	
private:
	fim_patch_op		m_opOriginal,		m_opCurrent;
	fim_offset			m_docPosWhere;
	fim_length			m_lenDelete;
	long				m_lineNrT0First,	m_lineNrT0Last;
	long				m_lineNrEditFirst,	m_lineNrEditLast;
	long				m_lineNrT2First,	m_lineNrT2Last;
	long				m_rowNrStart,		m_rowNrEnd;
	fr_prop				m_propT0,			m_propT2;
	wxString			m_strNewT0,			m_strNewT2;

#ifdef DEBUG
public:
	virtual void		dump(int indent)						const
		{
			wxLogTrace(TRACE_PTABLE_DUMP,_T("%*cDE_PATCH: [op cur %d orig %d][offset %ld len %ld][row %ld %ld][lineT0 %ld %ld][lineEdit %ld %ld][lineT2 %ld %ld]"),indent,' ',
					   m_opCurrent,m_opOriginal,
					   m_docPosWhere,m_lenDelete,
					   m_rowNrStart,m_rowNrEnd,
					   m_lineNrT0First, m_lineNrT0Last,
					   m_lineNrEditFirst, m_lineNrEditLast,
					   m_lineNrT2First, m_lineNrT2Last);
		};
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_DE_PATCH_H
