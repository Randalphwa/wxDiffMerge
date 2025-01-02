// fim_patchset.cpp -- a list of patches to be applied at once
// -- used by the auto-merge feature.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fl.h>

//////////////////////////////////////////////////////////////////

fim_patchset::fim_patchset(void)
{
}

fim_patchset::~fim_patchset(void)
{
	for (TVec_PatchesIterator it=m_vecPatches.begin(); (it!=m_vecPatches.end()); it++)
	{
		fim_patch * pPatch_it = (*it);
		delete pPatch_it;
	}
}

//////////////////////////////////////////////////////////////////

void fim_patchset::appendPatch(fim_patch * pPatch)
{
	m_vecPatches.push_back(pPatch);
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fim_patchset::dump(int indent) const
{
	wxLogTrace(TRACE_PTABLE_DUMP,_T("%*cFIM_PATCHSET:"),indent,' ');

	for (TVec_PatchesConstIterator it=m_vecPatches.begin(); (it!=m_vecPatches.end()); it++)
	{
		fim_patch * pPatch_it = (*it);
		pPatch_it->dump(indent+5);
	}

}
#endif
