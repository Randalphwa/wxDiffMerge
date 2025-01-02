// de_mark.cpp
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

de_mark::de_mark(long kSync, de_mark_type t, de_line * pDeLineT0, de_line * pDeLineT1, de_line * pDeLineT2)
	: m_kSync(kSync),
	  m_type(t)
{
	m_deLines[PANEL_T0] = pDeLineT0;
	m_deLines[PANEL_T1] = pDeLineT1;
	m_deLines[PANEL_T2] = pDeLineT2;
}

de_mark::de_mark(const de_mark & /*ref*/)
{
	// WARNING: this is only here to catch code accidentally
	// WARNING: calling the copy-ctor.
	// 
	// the CSSL's that we contain don't copy well.

	wxASSERT_MSG( (0), _T("Coding Error -- don't use this") );
}

de_mark::~de_mark(void)
{
	clearBeforeDelete();
}
//////////////////////////////////////////////////////////////////

bool de_mark::isTrivial(void) const
{
	bool bIsTrivial = ((m_type & DE_MARK_TRIVIAL) == DE_MARK_TRIVIAL);

#ifdef DEBUG
	bool bAnyNonNull = false;
	for (int k=0; k<__NR_TOP_PANELS__; k++)
		if (m_deLines[k])
			bAnyNonNull = true;
	bool bAllNull = !bAnyNonNull;
	wxASSERT_MSG( (bIsTrivial == bAllNull), _T("Coding Error") );
#endif

	return bIsTrivial;
}

//////////////////////////////////////////////////////////////////

void de_mark::clearBeforeDelete(void)
{
	for (int kPanel=PANEL_T0; kPanel<__NR_TOP_PANELS__; kPanel++)
	{
		if (m_deLines[kPanel])
		{
			wxASSERT_MSG( (m_deLines[kPanel]->getMark(m_kSync) == this), _T("Coding Error") );
			m_deLines[kPanel]->setMark(m_kSync,NULL);

			m_deLines[kPanel] = NULL;
		}
	}
}
