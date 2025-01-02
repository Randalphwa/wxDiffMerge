// ViewFile__splitter.cpp
// splitter-related stuff on 2-way/3-way file set.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fl.h>
#include <fd.h>
#include <fs.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ViewFile::_my_splitter, wxSplitterWindow)
	EVT_SPLITTER_DCLICK(			wxID_ANY,ViewFile::_my_splitter::onDoubleClickEvent)
	EVT_SPLITTER_SASH_POS_CHANGED(	wxID_ANY,ViewFile::_my_splitter::onSashPosChangedEvent)
	EVT_SIZE(                                ViewFile::_my_splitter::onSize)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

void ViewFile::onSplitterDoubleClick(Split s, long kSync, wxSplitterEvent & /*e*/)
{
	// re-balance on double-click

	//wxLogTrace(wxTRACE_Messages, _T("Splitter::DClick: [split %d]"), s);

	bool bSplitVertical = areSplittersVertical(kSync);

	switch (s)
	{
	case SPLIT_V1:
		if (!m_nbPage[kSync].m_pWinSplitter[SPLIT_V2])
		{
			m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->SetProportion(0.5f);
			if (bSplitVertical)
				adjustHorizontalScrollbar(kSync);
			else
				adjustVerticalScrollbar(kSync);
			return;
		}
		//FALL-THRU INTENDED
	case SPLIT_V2:
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->SetProportion(0.33f);
		m_nbPage[kSync].m_pWinSplitter[SPLIT_V2]->SetProportion(0.33f);
		if (bSplitVertical)
			adjustHorizontalScrollbar(kSync);
		else
			adjustVerticalScrollbar(kSync);
		return;

	default:
		wxASSERT_MSG( 0, _T("Coding Error") );
	}
}
