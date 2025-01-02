// de_sync_line_omitted.cpp -- special sync nodes for omitted lines
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>

//////////////////////////////////////////////////////////////////

de_sync_line_omitted::de_sync_line_omitted(void)
	: de_sync()
{
	m_attr = DE_ATTR_OMITTED;
	memset(m_pLine,0,sizeof(m_pLine));
}
de_sync_line_omitted::de_sync_line_omitted(PanelIndex kPanel_a, de_line * pLine_a, long len_a,
										   PanelIndex kPanel_b, de_line * pLine_b, long len_b,
										   bool bMLMember)
{
	m_attr = DE_ATTR_OMITTED;
	if (bMLMember)
		m_attr |= DE_ATTR_ML_MEMBER;
	
	memset(m_pLine,0,sizeof(m_pLine));

	m_pLine[kPanel_a] = pLine_a;
	m_pLine[kPanel_b] = pLine_b;

	m_len[kPanel_a] = len_a;
	m_len[kPanel_b] = len_b;
}

de_sync_line_omitted::de_sync_line_omitted(PanelIndex kPanel_a, de_line * pLine_a, long len_a,
										   PanelIndex kPanel_b, de_line * pLine_b, long len_b,
										   PanelIndex kPanel_c, de_line * pLine_c, long len_c,
										   bool bMLMember)
{
	m_attr = DE_ATTR_OMITTED;
	if (bMLMember)
		m_attr |= DE_ATTR_ML_MEMBER;

	memset(m_pLine,0,sizeof(m_pLine));

	m_pLine[kPanel_a] = pLine_a;
	m_pLine[kPanel_b] = pLine_b;
	m_pLine[kPanel_c] = pLine_c;

	m_len[kPanel_a] = len_a;
	m_len[kPanel_b] = len_b;
	m_len[kPanel_c] = len_c;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void de_sync_line_omitted::dump(int indent) const
{
	wxLogTrace(TRACE_DE_DUMP,
			   _T("%*cDE_SYNC_LINE_OMITTED:[%08lx] attr [%x] len[%ld %ld %ld]"),
			   indent,' ',this,m_attr,
			   m_len[PANEL_T1],m_len[PANEL_T0],m_len[PANEL_T2]);
}
#endif//DEBUG
