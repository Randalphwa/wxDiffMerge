// de_sync_list.h
//////////////////////////////////////////////////////////////////

#ifndef H_DE_SYNC_LIST_H
#define H_DE_SYNC_LIST_H

//////////////////////////////////////////////////////////////////

class de_sync_list
{
public:
	friend class de_de;		// TODO do we still need this as a friend ??
	de_sync_list(void);
	~de_sync_list(void);

	inline de_sync *			getHead(void)			const { return m_pHead; };
	inline de_sync *			getTail(void)			const { return m_pTail; };
	
	void						delete_list(void);

	void						load_T1X(const de_css_list * pcssl1x, bool bStartNewList=true);		// cssl from "PANEL_T1 vs PANEL_T0" or "PANEL_T1 vs PANEL_B"
	void						count_T1X_Changes(de_display_ops dops, de_stats2 * pStats2);

	void						load_T1T0T2(const de_css_list * pcssl10,	// cssl from "PANEL_T1 vs PANEL_T0"
											const de_css_list * pcssl12,	// cssl from "PANEL_T1 vs PANEL_T2"
											const de_css_src *  psrc02,		// css_src for "PANEL_T0 vs PANEL_T2"
											bool bStartNewList,
											bool bTryHardToAlign);

	void						combine_T1T0T2_conflicts(void);
	void						count_T1T0T2_Changes(de_display_ops dops, de_stats3 * pStats3);

	void						split(de_sync * pSync, long kRow);			// split this sync node at the given row.
	void						split_up_partial_eqs_with_zeros(void);
	void						appendNextToCurrent(de_sync * pSync);

	long						getTotalRows(void)		const;
	long						getTotalRows2(void)		const;
	long						getTotalRows3(void)		const;

	static de_sync_list *		createIntraLineSyncList2(de_attr initialChangeKind,
														 const de_line * pDeLineT1,
														 const de_line * pDeLineK,
														 const rs_ruleset * pRS);
	static de_sync_list *		createIntraLineSyncList3(de_attr initialChangeKind,
														 const de_line * pDeLineT1, const de_line * pDeLineT0, const de_line * pDeLineT2,
														 const rs_ruleset * pRS);

	inline long					getSumImportantChangesIntraLine(void)	const	{ return m_cImportantChangesIntraLine; };
	inline long					getSumChangesIntraLine(void)			const	{ return m_cChangesIntraLine; };
	inline de_attr				getSumChangeKindIntraLine(void)			const	{ return m_ChangeKindIntraLine; };

	void						applyMultiLineIntraLineContexts(de_de * pDeDe, long kSync, PanelIndex kPanel,
																long ndxLineCmp, long nrLines,
																const std::vector<long> & vecLineLens,
																const std::vector<bool> & vecEols,
																rs_context_attrs defaultContextAttrs);

	void						applyIntraLineImportance2(const wxString & strLineT1, const wxString & strLineT0,
														  de_attr initialChangeKind,
														  const de_line * pDeLineLastT1, const de_line * pDeLineLastT0,
														  const rs_ruleset * pRS);
	void						applyIntraLineImportance3(const wxString & strLineT1, const wxString & strLineT0, const wxString & strLineT2,
														  de_attr initialChangeKind,
														  const de_line * pDeLineLastT1, const de_line * pDeLineLastT0, const de_line * pDeLineLastT2,
														  const rs_ruleset * pRS);

	void						applySmoothing2(int threshold);
	void						applySmoothing3(int threshold);

	void						normalize_ChangeKindIntraLine3(void);

private:
	void						_truncate_list(de_sync * pSync);
	de_sync *					_delete_sync(de_sync * pSync);
	void						_insert_after(de_sync * pSync, de_sync * pNew);
	void						_insert_before(de_sync * pSync, de_sync * pNew);
	void						_append(de_sync * pSync);
	void						_append_new102(de_css_vars * pvars10, de_css_vars * pvars12, long lenT1, long lenT0, long lenT2, de_attr attr);
	void						_append_or_combine_new102(de_css_vars * pvars10, de_css_vars * pvars12, long lenT1, long lenT0, long lenT2, de_attr attr);

	void						_applyIntraLineContexts(const de_line * pDeLine, PanelIndex kPanel, rs_context_attrs defaultContextAttrs);

	void						_divide_node2(de_sync * pSync, PanelIndex xPanel, PanelIndex yPanel);
	void						_divide_node3_21(de_sync * pSync, PanelIndex xPanel, PanelIndex yPanel, PanelIndex zPanel);

	de_sync *					m_pHead;
	de_sync *					m_pTail;

	long						m_cImportantChangesIntraLine;
	long						m_cChangesIntraLine;
	de_attr						m_ChangeKindIntraLine;

#ifdef DEBUG
public:
	void						dump(int indent) const;
	bool						m_bIsLine;	// true when line-mode; false when intra-line-mode (for debug output)

#if 0
	void						test_verify_load_xy(wxChar * szLabel,
													const de_css_src_lines & rSrcXY,
													PanelIndex x, PanelIndex y,
													de_attr attrEq) const;
	void						test_verify_load_1x(PanelIndex x, const de_css_src_lines & rSrc1X) const;
	void						test_verify_load_102(const de_css_src_lines & rSrc10,
													 const de_css_src_lines & rSrc12,
													 const de_css_src_lines & rSrc02) const;
#endif
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_DE_SYNC_LIST_H
