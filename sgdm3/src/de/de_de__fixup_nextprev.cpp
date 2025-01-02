// de_de__fixup_nextprev.cpp -- diff engine -- routines related to the
// next/previous change/conflict links on the rows in the display
// list.
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

void de_de::_fixup_next_prev_change_links(long kSync)
{
	//////////////////////////////////////////////////////////////////
	// update the next/prev change/conflict links on each row.
	// the display list consists of a vector of rows (one row
	// for each row we display on the screen).  each row contains
	// a (de_sync *,offset) -- since a sync node contains a range
	// of lines, this (de_sync *,offset) lets us address the
	// individual line for this row.
	//
	// but note: there are no back links -- from sync node to row.
	// because not all sync nodes will be in the display list (when
	// DOP mode is not _ALL or when _HIDE_OMITTED is set, for example).
	// also, only part of a sync node might be displayed or disjoint
	// portions might be displayed (when DOP mode is _CTX).  so, i
	// don't believe we can put the next/prev change/conflict links
	// in the sync nodes -- we need to put them in with the rows.
	//
	// we also need to be able to quickly dereference them, so that
	// the OnUpdateUI for the next/prev toolbar buttons can hit them
	// (possibly, every time we move the mouse).
	//////////////////////////////////////////////////////////////////
	
	// we only compute this in certain view modes.  each time the
	// display list is build, all row links are invalidated (set to -1).

	m_vecDisplayIndex_Changes[kSync].clear();
	m_vecDisplayIndex_Conflicts[kSync].clear();
	
	switch (m_dops[kSync] & DE_DOP__MODE_MASK)
	{
	default:
		wxASSERT_MSG( (0), _T("Coding Error!") );
	case DE_DOP_EQL:		// we don't display any change/conflict lines in _EQL mode, so nothing to do
		m_vecDisplayIndex_Changes[kSync].push_back(-1);
		m_vecDisplayIndex_Conflicts[kSync].push_back(-1);
		return;	

	case DE_DOP_ALL:	_fixup_npcl_all(kSync);	return;
	case DE_DOP_DIF:	_fixup_npcl_dif(kSync);	return;
	case DE_DOP_CTX:	_fixup_npcl_all(kSync);	return;		// we now use _all for _CTX mode to get just the change/conflict rather than parts of the surrounding text
	}
}

void de_de::_fixup_npcl_all(long kSync)
{
	// update the next/prev change/conflict links on each row.
	// what we need to compute here is the absolute row number of
	// the beginning of the next/prev change/conflict.
	//
	// when in DE_DOP_ALL mode, we want the links to warp to the
	// beginning (the first line) of the next/prev change/conflict.
	//
	// note: because of sync node splitting (for void display) and
	// (in 3-way) the different types of non-EQs, we mave have a
	// sequence of various changes/conflicts sync nodes, which to
	// the user will appear as one change/conflict.
	//
	// so we need to scan -- looking for alternating patches of EQ
	// and non-EQ.

	// WARNING: don't call getDisplayList() because it causes a
	// WARNING: recursive call to run().
	wxASSERT_MSG( (m_bDisplayValid[kSync]), _T("Coding Error") );
	TVector_Display * pDis = &m_vecDisplay[kSync];

	long rowLimit = (long)pDis->size();
	long row;

	// start at top and set the previous links for each row.
	// we define the "previous" to be the beginning of a change
	// or conflict.  [while in the middle of a change, the
	// previous link will point to the beginning of the change.]

	long rowPrevChange = -1;
	long rowPrevConflict = -1;
	long typePrevious = 1;					// see _classify_type()

	for (row=0; row<rowLimit; row++)
	{
		de_row & rDeRow = (*pDis)[row];

		rDeRow.setPrevChange(rowPrevChange);
		rDeRow.setPrevConflict(rowPrevConflict);

		if (rDeRow.isMARK())
			continue;

		long typeCurrent = _classify_type(rDeRow.getSync(),m_dops[kSync]);
		if ((typeCurrent==0) && (typePrevious!=0))
		{
			// current row is a change (of some kind) and previous row was not.

			rowPrevChange = row;
			m_vecDisplayIndex_Changes[kSync].push_back(row);

			if (rDeRow.getSync()->isConflict())
			{
				rowPrevConflict = row;
				m_vecDisplayIndex_Conflicts[kSync].push_back(row);
			}
		}
		typePrevious = typeCurrent;
	}

	m_vecDisplayIndex_Changes[kSync].push_back(-1);
	m_vecDisplayIndex_Conflicts[kSync].push_back(-1);

	// we can now use the display-index vectors to build the next links
	// and not have to repeat this search backwards.

	_fixup_next_links(kSync);
}

void de_de::_fixup_npcl_dif(long kSync)
{
	// update the next/prev change/conflict links on each row.
	//
	// when in DE_DOP_DIF mode, we want the links to warp to the
	// beginning (the first line) of the next/prev change/conflict.
	//
	// but: because we're only showing changes, we can't do the
	// alternating EQ/non-EQ thing that we did in _all().
	//
	// also note: we too may have a sequence of non-EQ's.
	//
	// so we need to scan -- looking for the rows which have "gap"
	// set.

	// WARNING: don't call getDisplayList() because it causes a
	// WARNING: recursive call to run().
	wxASSERT_MSG( (m_bDisplayValid[kSync]), _T("Coding Error") );
	TVector_Display * pDis = &m_vecDisplay[kSync];

	long rowLimit = (long)pDis->size();
	long row;

	// start at top and set the previous links for each row.
	// we define the "previous" to be the beginning of a change
	// or conflict.  [while in the middle of a change, the
	// previous link will point to the beginning of the change.]

	long rowPrevChange = -1;
	long rowPrevConflict = -1;

	for (row=0; row<rowLimit; row++)
	{
		de_row & rDeRow = (*pDis)[row];

		rDeRow.setPrevChange(rowPrevChange);
		rDeRow.setPrevConflict(rowPrevConflict);

		// we don't have to worry about MARKS because we don't put
		// them in the display list when mode is _DIF.

		// see if row is the start of a non-contiguous change/conflict.

		if (   (!rDeRow.isEOF())
			&& (   (rDeRow.haveGap())
				|| ((row==0) && (_classify_type(rDeRow.getSync(),m_dops[kSync])==0))))
		{
			rowPrevChange = row;
			m_vecDisplayIndex_Changes[kSync].push_back(row);

			if (rDeRow.getSync()->isConflict())
			{
				rowPrevConflict = row;
				m_vecDisplayIndex_Conflicts[kSync].push_back(row);
			}
		}
	}

	m_vecDisplayIndex_Changes[kSync].push_back(-1);
	m_vecDisplayIndex_Conflicts[kSync].push_back(-1);

	// we can now use the display-index vectors to build the next links
	// and not have to repeat this search backwards.

	_fixup_next_links(kSync);
}

#if 0
long de_de::_fixup_npcl_ctx_look_forward_for_change(long kSync, long rowStart, long ctxGoal)
{
	// starting at the given rowStart, look foward upto ctxGoal lines
	// and determine the type of the type of the first non-EQ line.
	// we include the current starting line in our search.
	//
	// return 1 on conflict
	// return 0 on ordinary change
	// return -1 on EOF or EQ (or other)

	// WARNING: don't call getDisplayList() because it causes a
	// WARNING: recursive call to run().
	wxASSERT_MSG( (m_bDisplayValid[kSync]), _T("Coding Error") );
	TVector_Display * pDis = &m_vecDisplay[kSync];

	long rowMax = (long)pDis->size();
	long rowLimit = MyMin(rowMax,rowStart+ctxGoal);
	long row;

	for (row=rowStart; row<=rowLimit; row++)
	{
		de_row & rDeRow = (*pDis)[row];

		long type = _classify_type(rDeRow.getSync(),m_dops[kSync]);
		switch (type)
		{
		case -2:			// EOF
			return -1;

		case  0:			// diff
			return (rDeRow.getSync()->isConflict());

		default:			// keep looking
			break;
		}
	}

	return -1;
}
	
void de_de::_fixup_npcl_ctx(long kSync)
{
	// update the next/prev change/conflict links on each row.
	//
	// when in DE_DOP_CTX mode, we want the links to warp to
	// either the beginning of the above-context for the next/prev
	// change/conflict.
	//
	// so we need to look for alternating EQ's/non-EQ's, but when
	// we find a non-EQ, backup a little.  for isolated changes,
	// the above-context will begin with a gap.  but for changes
	// that only have a few EQ lines between them, there won't be
	// a gap and we'll have to settle for fewer lines context lines.

	long ctxGoal = gpGlobalProps->getLong(GlobalProps::GPL_FILE_CONTEXT_GOAL);	// nr lines of context desired

	// WARNING: don't call getDisplayList() because it causes a
	// WARNING: recursive call to run().
	wxASSERT_MSG( (m_bDisplayValid[kSync]), _T("Coding Error") );
	TVector_Display * pDis = &m_vecDisplay[kSync];

	long rowLimit = (long)pDis->size();
	long row;

	// start at top and set the previous links for each row.
	// we define the "previous" to be the beginning of the
	// EQ context above the start of the change/conflict.

	long rowPrevChangeContext = -1;
	long rowPrevConflictContext = -1;
	long typePrevious = 1;					// see _classify_type()
	bool bHaveContext = false;

	for (row=0; row<rowLimit; row++)
	{
		de_row & rDeRow = (*pDis)[row];

		rDeRow.setPrevChange(rowPrevChangeContext);
		rDeRow.setPrevConflict(rowPrevConflictContext);

		// we don't have to worry about MARKS because we don't put
		// them in the display list when mode is _CTX.

		long typeCurrent = _classify_type(rDeRow.getSync(),m_dops[kSync]);

		// if we are sitting on a line with "gap" set, it is
		// the beginning of a new context (or EOF).  search
		// forward a few lines and see what we have.
		//
		// if not on a gap, see if this is the first EQ following
		// a non-EQ.  this can be the start of a pseudo-context.
		// this happens when 2 changes are only a few lines apart.
		// [if the ctxGoal is 3, then 2 changes that are 6 lines
		// apart will not have a gap between them.]  but we don't
		// want to declare the start of a new pseudo-context until
		// we get within our goal.

		long x = -1;

		if (   (rDeRow.haveGap())							// start of gap
			|| ((typeCurrent==1) && (typePrevious==0))		// current is EQ and previous is non-EQ
			|| ((typeCurrent==1) && (!bHaveContext)))		// current is EQ and defered starting pseudo-context
		{
			x = _fixup_npcl_ctx_look_forward_for_change(kSync,row,ctxGoal);
			bHaveContext = (x != -1);
		}

		switch (x)
		{
		case 1:								// conflict
			rowPrevConflictContext = row;
			m_vecDisplayIndex_Conflicts[kSync].push_back(row);
			//FALLTHRU
		case 0:								// ordinary change
			rowPrevChangeContext = row;
			m_vecDisplayIndex_Changes[kSync].push_back(row);
			//FALLTHRU
		default:							// EOF or EQ (or bogus)
			break;
		}

		typePrevious = typeCurrent;
	}

	m_vecDisplayIndex_Changes[kSync].push_back(-1);
	m_vecDisplayIndex_Conflicts[kSync].push_back(-1);

	// we can now use the display-index vectors to build the next links
	// and not have to repeat this search backwards.

	_fixup_next_links(kSync);
}
#endif

void de_de::_fixup_next_links(long kSync)
{
#if 0
#ifdef DEBUG
	dump_links(10,kSync);
#endif
#endif

	// the prev links are set and vectors built.  now we need to
	// run thru the rows and set the next links using the vectors.

	// WARNING: don't call getDisplayList() because it causes a
	// WARNING: recursive call to run().
	wxASSERT_MSG( (m_bDisplayValid[kSync]), _T("Coding Error") );
	TVector_Display * pDis = &m_vecDisplay[kSync];

	long rowLimit = (long)pDis->size();
	long row;

	long ndxChange = 0;
	long ndxConflict = 0;

	for (row=0; row<rowLimit; row++)
	{
		while ((m_vecDisplayIndex_Changes[kSync][ndxChange] <= row) && (m_vecDisplayIndex_Changes[kSync][ndxChange] != -1))
			ndxChange++;
		while ((m_vecDisplayIndex_Conflicts[kSync][ndxConflict] <= row) && (m_vecDisplayIndex_Conflicts[kSync][ndxConflict] != -1))
			ndxConflict++;
			
		de_row & rDeRow = (*pDis)[row];

		rDeRow.setNextChange  (m_vecDisplayIndex_Changes  [kSync][ndxChange  ]);
		rDeRow.setNextConflict(m_vecDisplayIndex_Conflicts[kSync][ndxConflict]);
	}

#if 0
#ifdef DEBUG
	dump_links(20,kSync);
#endif
#endif
}

//////////////////////////////////////////////////////////////////

long de_de::getNthChangeDisplayIndex(long kSync, long ndx) const
{
	if ( (ndx < 0) || (ndx >= (long)m_vecDisplayIndex_Changes[kSync].size()) )
		return -1;
	return m_vecDisplayIndex_Changes[kSync][ndx];
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void de_de::dump_links(int indent, long kSync)
{
	// WARNING: don't call getDisplayList() because it causes a
	// WARNING: recursive call to run().
	wxASSERT_MSG( (m_bDisplayValid[kSync]), _T("Coding Error") );
	TVector_Display * pDis = &m_vecDisplay[kSync];

	unsigned k;

	wxLogTrace(TRACE_DE_DUMP, _T("%*c_fixup_next_links: [%ld] changes:"),indent,' ', m_vecDisplayIndex_Changes[kSync].size());
	for (k=0; k<m_vecDisplayIndex_Changes[kSync].size(); k++)
	{
		long ndx = m_vecDisplayIndex_Changes[kSync][k];
		if (ndx != -1)
		{
			de_row & rDeRow = (*pDis)[ndx];
			rDeRow.dump(indent+5);
		}
	}

	wxLogTrace(TRACE_DE_DUMP, _T("%*c_fixup_next_links: [%ld] conflicts:"), indent,' ',m_vecDisplayIndex_Conflicts[kSync].size());
	for (k=0; k<m_vecDisplayIndex_Conflicts[kSync].size(); k++)
	{
		long ndx = m_vecDisplayIndex_Conflicts[kSync][k];
		if (ndx != -1)
		{
			de_row & rDeRow = (*pDis)[ndx];
			rDeRow.dump(indent+5);
		}
	}
}
#endif
