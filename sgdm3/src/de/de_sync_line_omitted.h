// de_sync_line_omitted.h
//////////////////////////////////////////////////////////////////

#ifndef H_DE_SYNC_LINE_OMITTED_H
#define H_DE_SYNC_LINE_OMITTED_H

//////////////////////////////////////////////////////////////////

class de_sync_line_omitted : public de_sync		// a special sync node for "omitted" lines
{
public:
	de_sync_line_omitted(void);
	de_sync_line_omitted(PanelIndex kPanel_a, de_line * pLine_a, long len_a,
						 PanelIndex kPanel_b, de_line * pLine_b, long len_b,
						 bool bMLMember);
	de_sync_line_omitted(PanelIndex kPanel_a, de_line * pLine_a, long len_a,
						 PanelIndex kPanel_b, de_line * pLine_b, long len_b,
						 PanelIndex kPanel_c, de_line * pLine_c, long len_c,
						 bool bMLMember);
	
	virtual ~de_sync_line_omitted(void) {};

	de_line *					getFirstLine(PanelIndex kPanel)	const { return m_pLine[kPanel]; };
	
private:
	de_line *					m_pLine[__NR_TOP_PANELS__];
	
#ifdef DEBUG
public:
	virtual void				dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_DE_SYNC_LINE_OMITTED_H
