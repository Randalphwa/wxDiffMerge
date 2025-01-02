// de_sync.cpp -- a "sync" node
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

de_sync::de_sync(void)
{
	_init();
}

de_sync::de_sync(PanelIndex kPanel_a, long ndx_a, long len_a,
				 PanelIndex kPanel_b, long ndx_b, long len_b,
				 de_attr attr)
{
	_init();

	m_ndx[kPanel_a] = ndx_a;
	m_len[kPanel_a] = len_a;

	m_ndx[kPanel_b] = ndx_b;
	m_len[kPanel_b] = len_b;

	m_attr = attr;
}

de_sync::de_sync(PanelIndex kPanel_a, long ndx_a, long len_a,
				 PanelIndex kPanel_b, long ndx_b, long len_b,
				 PanelIndex kPanel_c, long ndx_c, long len_c,
				 de_attr attr)
{
	_init();

	m_ndx[kPanel_a] = ndx_a;
	m_len[kPanel_a] = len_a;

	m_ndx[kPanel_b] = ndx_b;
	m_len[kPanel_b] = len_b;

	m_ndx[kPanel_c] = ndx_c;
	m_len[kPanel_c] = len_c;

	m_attr = attr;
}

//////////////////////////////////////////////////////////////////

void de_sync::_init(void)
{
	m_next = NULL;
	m_prev = NULL;
	
	for (int kPanel=0; (kPanel<__NR_TOP_PANELS__); kPanel++)
	{
		m_ndx[kPanel] = -1;
		m_len[kPanel] = -1;
		m_contextAttrsSpan[kPanel] = RS_ATTRS__DEFAULT;
	}

	m_attr = DE_ATTR_UNKNOWN;

	m_pMark = NULL;

	m_pVecSyncList = NULL;

	DEBUG_CTOR(T_DE_SYNC,L"de_sync");
}

//////////////////////////////////////////////////////////////////

de_sync::~de_sync(void)
{
	// next/prev handled by de_sync_list

	if (m_pVecSyncList)
	{
		for (TVector_SyncList_Iterator it=m_pVecSyncList->begin(); (it != m_pVecSyncList->end()); it++)
		{
			de_sync_list * pSyncList = (*it);
			delete pSyncList;
		}
		
		delete m_pVecSyncList;
	}

	DEBUG_DTOR(T_DE_SYNC);
}

//////////////////////////////////////////////////////////////////

long de_sync::getMaxLen(void) const
{
	long max = 0;
	for (size_t kPanel=0; (kPanel<NrElements(m_len)); kPanel++)
		if (m_len[kPanel] > max)
			max = m_len[kPanel];
	return max;
}

long de_sync::getMinLen(void) const
{
	long min = LONG_MAX;
	for (size_t kPanel=0; (kPanel<NrElements(m_len)); kPanel++)
		if ((m_len[kPanel] != -1)  &&  (m_len[kPanel] < min))
			min = m_len[kPanel];
	return min;
}

//////////////////////////////////////////////////////////////////

void de_sync::updateAttrType(de_attr attr)
{
	de_attr a = attr & DE_ATTR__TYPE_MASK;

	m_attr = ((m_attr & ~DE_ATTR__TYPE_MASK) | a);
}

//////////////////////////////////////////////////////////////////

void de_sync::createIntraLineSyncListVector(long cReserve)
{
	wxASSERT_MSG( (!m_pVecSyncList), _T("TODO -- we need to properly delete contents of list (see dtor)"));
	delete m_pVecSyncList;

	m_pVecSyncList = new TVector_SyncList;
	m_pVecSyncList->reserve(cReserve);
	
}

void de_sync::appendIntraLineSyncList(de_sync_list * pSyncList)
{
	if (!m_pVecSyncList)
		createIntraLineSyncListVector(1);

	m_pVecSyncList->push_back(pSyncList);
}

de_sync_list * de_sync::getIntraLineSyncList(long row) const
{
	wxASSERT_MSG( (m_pVecSyncList), _T("Coding Error!") );

	return (*m_pVecSyncList)[row];
}

//////////////////////////////////////////////////////////////////

void de_sync::changeToMark(de_mark * pMark)
{
	m_attr = DE_ATTR_MARK;
	m_pMark = pMark;

	wxASSERT_MSG( (m_len[PANEL_T0] == 0), _T("Coding Error") );
	wxASSERT_MSG( (m_len[PANEL_T1] == 0), _T("Coding Error") );	
	wxASSERT_MSG( ((m_len[PANEL_T2] == 0) || (m_len[PANEL_T2] == -1)), _T("Coding Error") );
}

//////////////////////////////////////////////////////////////////

de_attr de_attr_union_attr(de_attr attr1, de_attr attr2)
{
	de_attr attrType = (attr1 & attr2 & DE_ATTR__TYPE_MASK);
	switch (attrType & DE_ATTR__KIND_MASK)
	{
	case DE_ATTR__MRG_KIND:		// normalize union
		switch (attrType)
		{
		default:
			attrType = DE_ATTR_MRG_0EQ;
			break;

		case DE_ATTR_MRG_0EQ:
		case DE_ATTR_MRG_T0T2EQ:
		case DE_ATTR_MRG_T1T2EQ:
		case DE_ATTR_MRG_T1T0EQ:
		case DE_ATTR_MRG_3EQ:
			break;
		}
		break;
	case DE_ATTR__DIF_KIND:		// no normalization needed
		break;
	default:	// other kinds (EOF, MARKS, etc) -- should not happen
		break;
	}

	de_attr attrModifier = ((attr1 | attr2) & DE_ATTR__MODIFIER_MASK);

	return (attrModifier | attrType);
}

//////////////////////////////////////////////////////////////////

void de_sync::unionAttr(de_attr attr)
{
	m_attr = de_attr_union_attr(m_attr,attr);
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void de_sync::dump(int indent) const
{
	wxLogTrace(TRACE_DE_DUMP,
			   _T("%*cDE_SYNC:[%08lx] attr [%x] ndx[%ld %ld %ld] len[%ld %ld %ld] rsAttr[%x %x %x]"),
			   indent,' ',this,m_attr,
			   m_ndx[PANEL_T1],m_ndx[PANEL_T0],m_ndx[PANEL_T2],
			   m_len[PANEL_T1],m_len[PANEL_T0],m_len[PANEL_T2],
			   m_contextAttrsSpan[PANEL_T1],m_contextAttrsSpan[PANEL_T0],m_contextAttrsSpan[PANEL_T2]);
}
#endif//DEBUG
