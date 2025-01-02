// de_sync_list.cpp -- part of diff engine to take pair-wise
// "common sub-sequence" (css) lists and build a single
// synchronized list reflecting the diff across all
// documents in the file set.
//
// THIS LIST CAN REPRESENT THE CHANGES BETWEEN LINES IN A
// DOCUMENT OR THE INTRALINE CHANGES ON A SINGLE LINE.  WE
// USE IT FOR BOTH TYPES.  The terms END and EOF and EOL
// all refer to the end of the sync list regardless of the
// type.
// 
// this should give us complete diff-related coloring info,
// v-scroll, and patch sync'ing.
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

de_sync_list::de_sync_list(void)
	: m_pHead(NULL), m_pTail(NULL),
	  m_cImportantChangesIntraLine(0), m_cChangesIntraLine(0), m_ChangeKindIntraLine(DE_ATTR__TYPE_MASK)
{
#ifdef DEBUG
	m_bIsLine = false;		// assume intra-line unless told otherwise
#endif

	DEBUG_CTOR(T_DE_SYNC_LIST,L"de_sync_list");
}

de_sync_list::~de_sync_list(void)
{
	delete_list();

	DEBUG_DTOR(T_DE_SYNC_LIST);
}

//////////////////////////////////////////////////////////////////

void de_sync_list::load_T1X(const de_css_list * pcssl1x, bool bStartNewList)
{
	//UTIL_PERF_START_CLOCK(_T("de_sync_list::load_T1X"));

	//////////////////////////////////////////////////////////////////
	// build sync list from cssl for a 2-way-diff.
	// we assume that pcssl1x is the css from "PANEL_T1 vs PANEL_T0"
	// or from "PANEL_EDIT vs PANEL_T0".
	// 
	// we assume that cssl already has gaps filled in.
	//
	// NOTE: this is kinda trivial -- for this particular case (2-way)
	// NOTE: the SYNC LIST is just a copy of the CSS LIST using slightly
	// NOTE: different nodes.
	//////////////////////////////////////////////////////////////////

	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error"));

	if (bStartNewList)
		delete_list();

	const de_css_item * pcss;
	for (pcss=pcssl1x->getHead(); (pcss && !pcss->isEOF()); pcss=pcss->getNext())
	{
		bool bEqual = pcss->isEqual();
		
		_append( new de_sync(PANEL_T1,pcss->get_a(),pcss->getLen_a(),
							 PANEL_T0,pcss->get_b(),pcss->getLen_b(),
							 (bEqual ? DE_ATTR_DIF_2EQ : DE_ATTR_DIF_0EQ)) );
	}

	wxASSERT_MSG( (pcss && pcss->isEOF()), _T("Coding Error") );

	//////////////////////////////////////////////////////////////////
	// append EOF marker
	//////////////////////////////////////////////////////////////////

	_append( new de_sync(PANEL_T1,pcss->get_a(),pcss->getLen_a(),
						 PANEL_T0,pcss->get_b(),pcss->getLen_b(),
						 DE_ATTR_EOF) );

	//UTIL_PERF_STOP_CLOCK(_T("de_sync_list::load_T1X"));
}

//////////////////////////////////////////////////////////////////

void de_sync_list::count_T1X_Changes(de_display_ops dops, de_stats2 * pStats)
{
	wxASSERT_MSG( (pStats), _T("Coding Error!") );

	// compute the number of changes for top-level window's status bar.
	//
	// NOTE: this must be done after load_T1X() and any intra-line fixups.
	//
	// our job is a little complicated because what the user sees as
	// a change may actually be composed of a series of adjacent change
	// sync nodes.  in the following, we try to compute the number as
	// the user would see it.
	//
	// our job is further complicated because an omitted line in the
	// middle of a multi-line change doesn't look like a change here
	// and we have a change sync-node before it and one after it, but
	// the user sees it as one, so we should combine them in our counting.
	//
	// FWIW, the value of m_cImportantChanges should approximate the
	// value of de_de.m_vecDisplayIndex_Change.size() -- when it is
	// computed.

	bool bHideUnimportant = DE_DOP__IS_SET_IGN_UNIMPORTANT(dops);

	typedef enum _t { UNIMPORTANT=0, CHANGE=1, OTHER=2 } t;
	t tPrev = OTHER;
	bool bPrevOmit = false;
	
	pStats->init();

#define CINC(var,val)	Statement( if (tPrev != (val)) pStats->var++; tPrev = val; )

	for (de_sync * pSync=m_pHead; (pSync && (pSync != m_pTail)); pSync=pSync->getNext())
	{
		switch (pSync->getAttr() & DE_ATTR__TYPE_MASK)
		{
		case DE_ATTR_UNKNOWN:
		case DE_ATTR_EOF:
		case DE_ATTR_DIF_2EQ:
		case DE_ATTR_MARK:
		default:
			tPrev = OTHER;
			bPrevOmit = false;
			break;

		case DE_ATTR_OMITTED:
			if (!bPrevOmit)
				pStats->m_cOmittedChunks++;
			bPrevOmit = true;
			break;

		case DE_ATTR_DIF_0EQ:
			if (bHideUnimportant && pSync->isUnimportant())
				CINC( m_cUnimportantChanges, UNIMPORTANT );
			else
				CINC( m_cImportantChanges, CHANGE );
			bPrevOmit = false;
			break;
		}
	}

#undef CINC
}

//////////////////////////////////////////////////////////////////

static bool _compute_attr_from_lengths(long lenT1, long lenT0, long lenT2, de_attr * pAttr)
{
	// when we are looking inside of a non-empty 0-way-eq
	// sequence, we frequently get 0 length sub-sequences.
	// if more than one size is 0, then we should report
	// a 2-way-eq (with length 0), rather than a 0-way-eq.

	de_attr attrTemp;
	int nrZero = (lenT1==0) + (lenT0==0) + (lenT2==0);
	
	switch (nrZero)
	{
	default:
	case 3:
		if (pAttr) *pAttr = DE_ATTR_UNKNOWN;
		return false;			// should not happen
		
	case 2:
		if (lenT1==0)
			if (lenT0==0)
				attrTemp = DE_ATTR_MRG_T1T0EQ;
			else
				attrTemp = DE_ATTR_MRG_T1T2EQ;
		else
			attrTemp = DE_ATTR_MRG_T0T2EQ;
		if (pAttr) *pAttr = attrTemp;
		return true;

	case 1:
	case 0:
		if (pAttr) *pAttr = DE_ATTR_MRG_0EQ|DE_ATTR_CONFLICT;
		return true;
	}
}

//////////////////////////////////////////////////////////////////

void de_sync_list::load_T1T0T2(const de_css_list * pcssl10,	// cssl from "PANEL_T1 vs PANEL_T0"
							   const de_css_list * pcssl12,	// cssl from "PANEL_T1 vs PANEL_T2"
							   const de_css_src *  psrc02,	// css_src for "PANEL_T0 vs PANEL_T2"
							   bool bStartNewList,
							   bool /*bTryHardToAlign*/)
{
	// TODO add "bool bTryHardToAlign" argument and use it to avoid
	// TODO doing the sub-cssl tricks (for use when the caller is
	// TODO just going to join_up_* the results anyway).

	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error"));

	if (bStartNewList)
		delete_list();

	//////////////////////////////////////////////////////////////////
	// build sync list from 2 cssl's for a 3-way-merge.
	// we assume that the CSS LISTS already have gaps filled in.
	//
	// both CSS LIST's are ordered on PANEL_T1, so we can walk thru
	// them segment-by-segment and create merged list.  this is not
	// as easy as it sounds -- because in addition to just folding
	// the lists together, we would also like to display voids.
	// and we'd like the voids to be somewhat sane.
	//
	// for example, given the 2 CSS LISTS on the left (with xx,ww,vv,zz
	// as literals; AA,BB as variables; and .. as a void), we can merge
	// them in several different ways depending on how AA and BB compare.
	// there are many possibilities, here are 3:
	//
	//     T0  T1     T1  T2            T0  T1  T2       T0  T1  T2       T0  T1  T2
	//     ======     ======            ==========       ==========       ==========
	//     xx  xx     xx  xx            xx  xx  xx       xx  xx  xx       xx  xx  xx
	//     AA  ww     ww  ww     ==>    AA  ww  ww       ..  ww  ww       a1  ww  ww
	//     vv  vv     vv  BB            vv  vv  BB       AA  ..  BB       a2  ..  b1
	//     zz  zz     zz  zz            zz  zz  zz       vv  vv  ..       vv  vv  b2
	//                                                   zz  zz  zz       zz  zz  zz
	//
	// we expand the notion of "conflict" to include multiple adjacent
	// regular changes.
	//
	// we run the folding loop using CSS ITEMS in both CSS LISTS.
	// since these are CONST and we don't want to change the CSS LISTS,
	// we work with a copy of the CSS variables (VARS).
	//
	// we've structured the loop so that on each pass we consume what
	// we can from both lists based upon the state at leading edge of
	// the items/nodes.  the body of the loop has a myriad of branches
	// to determine the current state of the leading edge.  each case
	// only handles a single state.  after a state is handled, we let
	// the loop iterate to process the next state, rather than trying
	// to take any shortcuts.
	//
	// _append_new102() is used to consume content from the leading
	// edge of the VARS and build a SYNC node.  IT UPDATES THE VARS.
	//
	// when we completely consume the content in a VAR/CSS NODE, we
	// advance to the next one in the CSS LIST.
	//
	// this process continues until both lists have been completely
	// consumed.
	//
	//////////////////////////////////////////////////////////////////
	// some notes and assertions:
	// 
	// [] CSS nodes in the CSS LIST should alternate between EQUAL
	//    (those that the CSS ALGORITHM found) and NON-EQUAL (for
	//    the gaps between them).  there shouldn't be 2 consecutive
	//    nodes of the same kind.
	//
	//////////////////////////////////////////////////////////////////

	const de_css_item * pcss10 = pcssl10->getHead();
	const de_css_item * pcss12 = pcssl12->getHead();

	de_css_vars vars10(pcss10);		// working copies of offset/lengths
	de_css_vars vars12(pcss12);

	long lenT0, lenT1, lenT2, lenOdd;	// very temp vars
	de_attr attrTemp;

	while (!pcss10->isEOF() && !pcss12->isEOF())
	{
#if 0
#ifdef DEBUG
		if (m_bIsLine) { vars10.dump(10);	vars12.dump(40); }
#endif
#endif
		
		wxASSERT_MSG( (vars10.get_a() == vars12.get_a()), _T("Coding Error: T1s not in sync") );  // we're sync'ing on T1 as we scan

		if (vars10.isEqual() && vars12.isEqual())				// 3-way-equal segment
		{
			lenT1 = MyMin(vars10.getLen_a(), vars12.getLen_a());	// split longer one if T1's not equal length
			_append_new102(&vars10,&vars12, lenT1,lenT1,lenT1, DE_ATTR_MRG_3EQ);
		}
		else if (vars10.isEqual())								// T1==T0  [THIS SECTION SHOULD MATCH THE NEXT SECTION WITH VARS SWAPPED]
		{
			if (vars12.getLen_a() < vars10.getLen_a())			// T1 shorter in "T1 vs T2" than in "T1 vs T0"
			{
				lenT1 = vars12.getLen_a();						// next node in "T1 vs T2" will be 3-way sync with part after split
				_append_new102(&vars10,&vars12, lenT1,lenT1,vars12.getLen_b(), DE_ATTR_MRG_T1T0EQ);
			}
			else // otherwise, T1 lengths equal or T1 longer on non-equal side
			{
				if (vars12.getLen_b() == 0)		// if nothing in T2, we can avoid sub-diff
				{
					lenT1 = MyMin(vars10.getLen_a(),vars12.getLen_a());
					_append_new102(&vars10,&vars12, lenT1,lenT1,0, DE_ATTR_MRG_T1T0EQ);
				}
				else	// T2 non-empty, try to decide where to put it
				{
					// x x J 
					// K y y
					de_css_vars varsClip(pcss10->getNext()->get_b(),pcss10->getNext()->getLen_b(), vars12.get_b(),vars12.getLen_b());
					de_css_src_clipped srcClip(psrc02,&varsClip);
					de_css_list csslClip(&srcClip);					// run diff-algorithm on "K J"
					de_css_item * pcssSub = csslClip.getHead();
					de_css_item * pcssSubNext = pcssSub->getNext();

					if (pcssSub->isEqual())							// beginning of K and J match, emit "x x ." and set up for "K . J"
					{
						_append_new102(&vars10,&vars12, vars10.getLen_a(),vars10.getLen_b(),0, DE_ATTR_MRG_T1T0EQ);
					}
					else if (!pcssSubNext->isEOF())					// first node non-equal and if second not EOF, then second is equal.
					{	// one or more sub-segments within K and J match, but the beginnings don't.  emit "x x j1" and setup for "K . j2".
						_append_new102(&vars10,&vars12, vars10.getLen_a(),vars10.getLen_b(),pcssSub->getLen_b(), DE_ATTR_MRG_T1T0EQ);
					}
					else	// otherwise, nothing in common in "K J". emit "x x J".  if T1 longer on non-equal side, emit portion of J to avoid large voids.
					{
						lenOdd = vars12.getLen_b();
						if (vars12.getLen_a() > vars10.getLen_a())
							lenOdd = MyMin(lenOdd, vars10.getLen_a());
						_append_new102(&vars10,&vars12, vars10.getLen_a(),vars10.getLen_b(),lenOdd, DE_ATTR_MRG_T1T0EQ);
					}
				}
			}
		}
		else if (vars12.isEqual())								// T1==T2  [THIS SECTION SHOULD MATCH THE PREVIOUS SECTION WITH VARS SWAPPED]
		{
			if (vars10.getLen_a() < vars12.getLen_a())			// T1 shorter in "T1 vs T0"
			{
				lenT1 = vars10.getLen_a();						// next node in "T1 vs T2" will be 3-way sync with part after split
				_append_new102(&vars10,&vars12, lenT1,vars10.getLen_b(),lenT1, DE_ATTR_MRG_T1T2EQ);
			}
			else // otherwise, T1 lengths equal or T1 longer on non-equal side
			{
				if (vars10.getLen_b() == 0)		// if nothing in T0, we can avoid sub-diff
				{
					lenT1 = MyMin(vars10.getLen_a(),vars12.getLen_a());
					_append_new102(&vars10,&vars12, lenT1,0,lenT1, DE_ATTR_MRG_T1T2EQ);
				}
				else	// T0 non-empty, try to decide where to put it
				{
					// J x x
					// y y K
					de_css_vars varsClip(vars10.get_b(),vars10.getLen_b(), pcss12->getNext()->get_b(),pcss12->getNext()->getLen_b());
					de_css_src_clipped srcClip(psrc02,&varsClip);
					de_css_list csslClip(&srcClip);					// run diff-algorithm on "J K"
					de_css_item * pcssSub = csslClip.getHead();
					de_css_item * pcssSubNext = pcssSub->getNext();

					if (pcssSub->isEqual())							// beginning of J and K match, emit ". x x" and set up for "J . K"
					{
						_append_new102(&vars10,&vars12, vars12.getLen_a(),0,vars12.getLen_b(), DE_ATTR_MRG_T1T2EQ);
					}
					else if (!pcssSubNext->isEOF())						// first node non-equal and if second not EOF, then second is equal.
					{	// one or more sub-segments within J and K match, but the beginnings don't.  emit "j1 x x" and setup for "j2 . K".
						_append_new102(&vars10,&vars12, vars12.getLen_a(),pcssSub->getLen_a(),vars12.getLen_b(), DE_ATTR_MRG_T1T2EQ);
					}
					else	// otherwise, nothing in common in "J K". emit "J x x".  if T1 longer on non-equal side, emit portion of J to avoid large voids.
					{
						lenOdd = vars10.getLen_b();
						if (vars10.getLen_a() > vars12.getLen_a())
							lenOdd = MyMin(lenOdd, vars12.getLen_a());
						_append_new102(&vars10,&vars12, vars12.getLen_a(),lenOdd,vars12.getLen_b(), DE_ATTR_MRG_T1T2EQ);
					}
				}
			}
		}
		else	// otherwise, 0-way-equal or 2-way (T0==T2) equal
		{
			if ( (vars10.getLen_b()==0) && (vars12.getLen_b()==0) )		// a delete in T0 & T2 relative to T1
			{
				lenT1 = MyMin(vars10.getLen_a(),vars12.getLen_a());
				wxASSERT_MSG( (lenT1 > 0), _T("Coding Error") );
				_append_new102(&vars10,&vars12, lenT1,0,0, DE_ATTR_MRG_T0T2EQ);
			}
			else if (vars10.getLen_b()==0)	// T0 empty, T2 non-empty, so we can avoid sub-diff
			{
				lenT1 = MyMin(vars10.getLen_a(),vars12.getLen_a());			// shorter of two T1's
				attrTemp = (  (lenT1==0)
							? DE_ATTR_MRG_T1T0EQ							// T0 empty, T1 empty, so they're equal
							: DE_ATTR_MRG_0EQ|DE_ATTR_CONFLICT);
				_append_new102(&vars10,&vars12, lenT1,0,vars12.getLen_b(), attrTemp);
			}
			else if (vars12.getLen_b()==0)	// T2 empty, T0 non-empty, so we can avoid sub-diff
			{
				lenT1 = MyMin(vars10.getLen_a(),vars12.getLen_a());			// shorter of two T1's
				attrTemp = (  (lenT1==0)
							? DE_ATTR_MRG_T1T2EQ							// T2 empty, T1 empty, so they're equal
							: DE_ATTR_MRG_0EQ|DE_ATTR_CONFLICT);
				_append_new102(&vars10,&vars12, lenT1,vars10.getLen_b(),0, attrTemp);
			}
			else	// T0 and T2 both not empty
			{
				// T1 not equal to either T0 or T2.  we may have "T0 == T2" -- that is,
				// a common change.  or we may have 2 independent changes -- that is,
				// a simple conflict.  or we may have a combination of the two -- that
				// is, a sub-sequence of one or more of each type within this CSS and
				// that all are across from this T1.  these will all be conflicts, but
				// by identifying sub-sub-sequences we might give better alignment on
				// screen.  for example:
				//
				// T0  T1  T2    might look better as    T0  T1  T2
				// ==========                            ==========
				// xx  xx  xx                            xx  xx  xx
				// AA  zz  BB                            AA  ..  ..
				// BB      DD                            BB  ..  BB
				// CC      zz                            CC  ..  ..
				// DD                                    DD  ..  DD
				// zz                                    zz  zz  zz
				//
				// where '..' is a void and we are called looking at line 2.
				//
				// this is relatively straight-forward when the T1 portion of the
				// CSS's are empty.  when T1 is non-empty, we have to figure out
				// how to distribute it -- we're going to just start pouring it in
				// from the top and try not to create bogus voids, so we constrain
				// how much of T1 we include by limiting it to how much of T0/T2
				// we pickup.
				//
				// the following section is a little different from all of the above:
				// here we have a sub-loop and insert multiple sync nodes.  since we
				// know that both T0 and T2 differ from T1, we can consume both up
				// until we consume the shorter T1. (if we just did one and looped
				// like everyone else, we'd just end up here again.)

				de_css_vars varsClip(vars10.get_b(),vars10.getLen_b(), vars12.get_b(),vars12.getLen_b());
				de_css_src_clipped srcClip(psrc02,&varsClip);
				de_css_list csslClip(&srcClip);						// run diff-algorithm on T0 & T2
				de_css_item * pcssSub = csslClip.getHead();

				while (!pcssSub->getNext()->isEOF())	// break out of loop on final (non-eof) sub-seq
				{
					lenT1 = MyMin(vars10.getLen_a(),vars12.getLen_a());						// shorter of two T1's
					lenT1 = MyMin(lenT1, MyMax(pcssSub->getLen_a(),pcssSub->getLen_b()));	// limited to avoid center panel causing voids on both T0 & T2
					lenT0 = pcssSub->getLen_a();
					lenT2 = pcssSub->getLen_b();

					if (pcssSub->isEqual())
						_append_new102(&vars10,&vars12, lenT1,lenT0,lenT2, DE_ATTR_MRG_T0T2EQ);
					else if (_compute_attr_from_lengths(lenT1,lenT0,lenT2, &attrTemp))
						_append_new102(&vars10,&vars12, lenT1,lenT0,lenT2, attrTemp);

					pcssSub = pcssSub->getNext();
				}

				// handle final (non-EOF) sub-seq differently

				lenT1 = MyMin(vars10.getLen_a(),vars12.getLen_a());							// shorter of two T1's
				if (pcssSub->isEqual())			// final (non-EOF) sub-seq is equal
				{
					lenT0 = pcssSub->getLen_a();
					lenT2 = pcssSub->getLen_b();
					_append_new102(&vars10,&vars12, lenT1,lenT0,lenT2, DE_ATTR_MRG_T0T2EQ);
				}
				else							// final (non-EOF) sub-seq is not equal
				{
					// clip this sub-seq's T[02] if it's associated with the longer T1 to
					// avoid creating a large void and to let it spill over onto the following
					// (top-level) 2-way-equal node.

					lenOdd = MyMin(pcssSub->getLen_a(),pcssSub->getLen_b());
					lenT0  = (vars10.getLen_a() > vars12.getLen_a()) ? lenOdd : pcssSub->getLen_a();
					lenT2  = (vars12.getLen_a() > vars10.getLen_a()) ? lenOdd : pcssSub->getLen_b();

					if (_compute_attr_from_lengths(lenT1,lenT0,lenT2, &attrTemp))
						_append_new102(&vars10,&vars12, lenT1,lenT0,lenT2, attrTemp);
				}
			}
		}

#if 0
#ifdef DEBUG
		if (m_bIsLine) { dump(20); }
#endif
#endif

		if ((vars10.getLen_a()==0) && (vars10.getLen_b()==0) && !pcss10->isEOF())	{	pcss10 = pcss10->getNext();		vars10.setFrom(pcss10);	}
		if ((vars12.getLen_a()==0) && (vars12.getLen_b()==0) && !pcss12->isEOF())	{	pcss12 = pcss12->getNext();		vars12.setFrom(pcss12);	}
	}

	// we hit the EOF node on one of the 2 CSS lists.  this means that we have
	// consumed all of T1 in both lists.  it does not mean that we have reached
	// the EOF node on the other list.  so there may be one more (non-equal) node
	// on it.

	wxASSERT_MSG( (pcss10->getLen_a() == pcss12->getLen_a()), _T("Coding Error") );		// T1 should be same length in both

	if (!pcss10->isEOF())
	{
		wxASSERT_MSG( (pcss12->isEOF()),       _T("Coding Error") );
		wxASSERT_MSG( (pcss10->getLen_a()==0), _T("Coding Error") );
		wxASSERT_MSG( (pcss10->getLen_b()>0),  _T("Coding Error") );
		_append_new102(&vars10,&vars12,0,pcss10->getLen_b(),0, DE_ATTR_MRG_T1T2EQ);
		if ((vars10.getLen_a()==0) && (vars10.getLen_b()==0) && !pcss10->isEOF())	{	pcss10 = pcss10->getNext();		vars10.setFrom(pcss10);	}
	}
	else if (!pcss12->isEOF())
	{
		wxASSERT_MSG( (pcss10->isEOF()),       _T("Coding Error") );
		wxASSERT_MSG( (pcss12->getLen_a()==0), _T("Coding Error") );
		wxASSERT_MSG( (pcss12->getLen_b()>0),  _T("Coding Error") );
		_append_new102(&vars10,&vars12,0,0,pcss12->getLen_b(), DE_ATTR_MRG_T1T0EQ);
		if ((vars12.getLen_a()==0) && (vars12.getLen_b()==0) && !pcss12->isEOF())	{	pcss12 = pcss12->getNext();		vars12.setFrom(pcss12);	}
	}

	wxASSERT_MSG( (pcss10 && pcss10->isEOF()),                _T("Coding Error") );
	wxASSERT_MSG( (pcss12 && pcss12->isEOF()),                _T("Coding Error") );
	wxASSERT_MSG( (pcss10->getLen_a() == pcss12->getLen_a()), _T("Coding Error") );		// T1 should be same length in both

	//////////////////////////////////////////////////////////////////
	// append EOF marker
	//////////////////////////////////////////////////////////////////

	_append( new de_sync(PANEL_T1,pcss10->get_a(),pcss10->getLen_a(),
						 PANEL_T0,pcss10->get_b(),pcss10->getLen_b(),
						 PANEL_T2,pcss12->get_b(),pcss12->getLen_b(),
						 DE_ATTR_EOF) );
}

//////////////////////////////////////////////////////////////////

void de_sync_list::combine_T1T0T2_conflicts(void)
{
	//////////////////////////////////////////////////////////////////
	// run thru the list we just constructed and set the CONFLICT bit
	// on all nodes that are either a 0-way-equal or that are in a
	// sequence of adjacent changes.
	// 
	// we do this in 2 passes, once forward and once backward, so that
	// we get a kind of closure.
	//
	// the CONFLICT bit is kind of informational and lets us more
	// quickly color spans them differently.
	//////////////////////////////////////////////////////////////////

//	dump(10);

	for (de_sync * pSync=m_pHead; (pSync && (pSync != m_pTail)); pSync=pSync->getNext())
	{
		if (pSync->isMARK())						{ continue;							}	// we do not extend conflicts across marks
		if (pSync->isSameType(DE_ATTR_MRG_3EQ))		{ continue;							}	// 3-way-equal cannot part of a conflict
		if (pSync->isConflict())					{ continue;							}	// already conflict, go on
		if (pSync->isSameType(DE_ATTR_MRG_0EQ))		{ pSync->setConflict(); continue;	}	// 0-way-equal is conflict

		// current is one of 3 possible 2-way-equal nodes.  compare it with previous to see if we should make
		// it part of a multi-node conflict.  (mark current and go on.  we'll mark the previous node in the
		// next loop.)

		de_sync * pSyncPrev = pSync->getPrev();
		if (!pSyncPrev)								{ continue;							}
		
		if (pSyncPrev->isMARK())					{ continue;							}	// we do not extend conflicts across marks
		if (pSyncPrev->isSameType(DE_ATTR_MRG_3EQ))	{ continue;							}	// prev is 3-way-equal, so not continuing conflict
		if (pSyncPrev->isConflict())				{ pSync->setConflict(); continue;	}	// continuing a conflict
		if (!pSyncPrev->isSameType(pSync->m_attr))	{ pSync->setConflict(); continue;	}	// prev is 0-way-equal or a different 2-way-equal
	}

	// run thru the list backwards (starting after the EOF) and propagate the CONFLICT bit back up the list.

	for (de_sync * pSync=m_pTail->getPrev(); (pSync && pSync->getPrev()); pSync=pSync->getPrev())
	{
		de_sync * pSyncPrev = pSync->getPrev();
		if (pSync->isConflict() && !pSyncPrev->isSameType(DE_ATTR_MRG_3EQ) && !pSyncPrev->isMARK())	// current is conflict and previous is not 3-way-equal
			pSyncPrev->setConflict();
	}
}

//////////////////////////////////////////////////////////////////

void de_sync_list::count_T1T0T2_Changes(de_display_ops dops, de_stats3 * pStats)
{
	wxASSERT_MSG( (pStats), _T("Coding Error") );

	// scan the sync list and count the number of changes/conflicts.
	// these numbers will be used for the "%d changes / %d conflicts"
	// message in the top-level window's status bar.
	//
	// NOTE: this must be done after load_T1T0T2() and any intra-line fixups.
	//
	// our job is a little complicated because what the user sees as
	// a change or a conflict may actually be composed of a series of
	// adjacent changes.  in the following, we try to compute the number
	// as the user would consider it.
	//
	// our job is further complicated because an omitted line in the
	// middle of a multi-line change doesn't look like a change here
	// and we have a change sync-node before it and one after it, but
	// the user sees it as one, so we should combine them in our counting.
	// 
	// FWIW, the value of m_cImportantChanges should approximate the
	// value of de_de.m_vecDisplayIndex_Change.size() -- when it is
	// computed.

	bool bHideUnimportant = DE_DOP__IS_SET(dops,DE_DOP_IGN_UNIMPORTANT);
	
	typedef enum _t { UNIMPORTANT_CHANGE=0, CHANGE=1,
					  UNIMPORTANT_CONFLICT=2, CONFLICT=3,
					  OTHER=4 } t;
	t tPrev = OTHER;
	bool bPrevOmit = false;

	pStats->init();

#define CINC(var,val)	Statement( if (tPrev != (val)) pStats->var++; tPrev = val; )

	for (de_sync * pSync=m_pHead; (pSync && (pSync != m_pTail)); pSync=pSync->getNext())
	{
		switch (pSync->getAttr() & DE_ATTR__TYPE_MASK)
		{
		case DE_ATTR_UNKNOWN:
		case DE_ATTR_EOF:
		case DE_ATTR_MRG_3EQ:
		case DE_ATTR_MARK:
		default:
			tPrev = OTHER;
			bPrevOmit = false;
			break;

		case DE_ATTR_OMITTED:
			if (!bPrevOmit)
				pStats->m_cOmittedChunks++;
			bPrevOmit = true;
			break;

		case DE_ATTR_MRG_0EQ:
		case DE_ATTR_MRG_T0T2EQ:
		case DE_ATTR_MRG_T1T2EQ:
		case DE_ATTR_MRG_T1T0EQ:
			if (pSync->isConflict())
			{
				if (bHideUnimportant && pSync->isUnimportant())
					CINC( m_cUnimportantConflicts, UNIMPORTANT_CONFLICT );
				else
					CINC( m_cImportantConflicts, CONFLICT );
			}
			else
			{
				if (bHideUnimportant && pSync->isUnimportant())
					CINC( m_cUnimportantChanges, UNIMPORTANT_CHANGE );
				else
					CINC( m_cImportantChanges, CHANGE );
			}
			bPrevOmit = false;
			break;
		}
	}

#undef CINC
}

//////////////////////////////////////////////////////////////////

void de_sync_list::delete_list(void)
{
	_truncate_list(m_pHead);
}

//////////////////////////////////////////////////////////////////

void de_sync_list::_truncate_list(de_sync * pSync)
{
	if (!pSync)
		return;

	m_pTail = pSync->getPrev();
	if (pSync == m_pHead)
		m_pHead = NULL;
	
	while (pSync)
	{
		de_sync * pNext = pSync->getNext();
		delete pSync;
		pSync = pNext;
	}
}

de_sync * de_sync_list::_delete_sync(de_sync * pSync)
{
	if (!pSync)
		return NULL;

	de_sync * pNext = pSync->getNext();
	de_sync * pPrev = pSync->getPrev();

	if (pNext)
		pNext->m_prev = pPrev;
	else
		m_pTail = pPrev;

	if (pPrev)
		pPrev->m_next = pNext;
	else
		m_pHead = pNext;

	delete pSync;

	return pNext;
}

void de_sync_list::_insert_after(de_sync * pSync, de_sync * pNew)
{
	de_sync * pNext = pSync->getNext();

	pNew->m_next = pNext;	
	pNew->m_prev = pSync;
	pSync->m_next = pNew;
	
	if (pNext)
		pNext->m_prev = pNew;
	else
		m_pTail = pNew;
}

void de_sync_list::_insert_before(de_sync * pSync, de_sync * pNew)
{
	de_sync * pPrev = pSync->getPrev();

	pNew->m_next = pSync;	
	pNew->m_prev = pPrev;
	pSync->m_prev = pNew;
	
	if (pPrev)
		pPrev->m_next = pNew;
	else
		m_pHead = pNew;
}

void de_sync_list::_append(de_sync * pSync)
{
	pSync->m_prev = m_pTail;
	if (m_pTail)
		m_pTail->m_next = pSync;
	else
		m_pHead = pSync;
	m_pTail = pSync;
}

void de_sync_list::_append_new102(de_css_vars * pvars10, de_css_vars * pvars12, long lenT1, long lenT0, long lenT2, de_attr attr)
{
	wxASSERT_MSG( (lenT1 || lenT0 || lenT2), _T("Coding Error") );	// require at least one to be non-zero

	if ( ((lenT1==0)+(lenT0==0)+(lenT2==0)) == 2 )
	{
		wxASSERT_MSG( ((attr & DE_ATTR_CONFLICT)==0), _T("Coding Error") );
	}

	_append( new de_sync(PANEL_T1,pvars10->get_a(),lenT1,
						 PANEL_T0,pvars10->get_b(),lenT0,
						 PANEL_T2,pvars12->get_b(),lenT2, attr) );

	pvars10->shift(MyMin(lenT1,pvars10->getLen_a()), MyMin(lenT0,pvars10->getLen_b()));
	pvars12->shift(MyMin(lenT1,pvars12->getLen_a()), MyMin(lenT2,pvars12->getLen_b()));
}

void de_sync_list::_append_or_combine_new102(de_css_vars * pvars10, de_css_vars * pvars12, long lenT1, long lenT0, long lenT2, de_attr attr)
{
#ifdef DEBUG
	wxASSERT_MSG( (lenT1 || lenT0 || lenT2), _T("Coding Error") );	// require at least one to be non-zero

	if ( ((lenT1==0)+(lenT0==0)+(lenT2==0)) == 2 )
	{
		wxASSERT_MSG( ((attr & DE_ATTR_CONFLICT)==0), _T("Coding Error") );
	}

	if (m_pTail)
	{
		wxASSERT_MSG( (m_pTail->getNdx(PANEL_T1)+m_pTail->getLen(PANEL_T1) == pvars10->get_a()), _T("Coding Error") );
		wxASSERT_MSG( (m_pTail->getNdx(PANEL_T1)+m_pTail->getLen(PANEL_T1) == pvars12->get_a()), _T("Coding Error") );

		wxASSERT_MSG( (m_pTail->getNdx(PANEL_T0)+m_pTail->getLen(PANEL_T0) == pvars10->get_b()), _T("Coding Error") );
		wxASSERT_MSG( (m_pTail->getNdx(PANEL_T2)+m_pTail->getLen(PANEL_T2) == pvars12->get_b()), _T("Coding Error") );
	}
#endif
	
	if (m_pTail  &&  m_pTail->getAttr() == attr)
	{
		m_pTail->m_len[PANEL_T0] += lenT0;
		m_pTail->m_len[PANEL_T1] += lenT1;
		m_pTail->m_len[PANEL_T2] += lenT2;
	}
	else	
	{
		_append( new de_sync(PANEL_T1,pvars10->get_a(),lenT1,
							 PANEL_T0,pvars10->get_b(),lenT0,
							 PANEL_T2,pvars12->get_b(),lenT2, attr) );
	}
	
	pvars10->shift(MyMin(lenT1,pvars10->getLen_a()), MyMin(lenT0,pvars10->getLen_b()));
	pvars12->shift(MyMin(lenT1,pvars12->getLen_a()), MyMin(lenT2,pvars12->getLen_b()));
}

void de_sync_list::split(de_sync * pSync, long kRow)
{
	wxASSERT_MSG( ((kRow > 0) && (kRow < pSync->getMaxLen())), _T("Coding Error"));

	de_sync * pNew = new de_sync();

	pNew->m_attr = pSync->m_attr;

	if (pSync->m_pVecSyncList)
	{
		// if we are a line-oriented sync-node and have a list of
		// intra-line sync lists for each of our rows, we need to
		// divvy them up also.

		long jLimit = pSync->getMaxLen();
		pNew->createIntraLineSyncListVector(jLimit - kRow);
		for (long jRow=kRow; jRow<jLimit; jRow++)
		{
			de_sync_list * p = pSync->getIntraLineSyncList(jRow);
			pNew->appendIntraLineSyncList(p);
		}

		// i want to do a:  pSync->m_pVecSyncList->erase(kRow,jLimit);
		// but can't get the std::vector class to behave, so we do it
		// the hard way. -- we might be able to do "it = begin()+kRow;"

		de_sync::TVector_SyncList_Iterator it = pSync->m_pVecSyncList->begin();
		for (long iRow=0; (iRow<kRow); iRow++)
			it++;
		pSync->m_pVecSyncList->erase(it,pSync->m_pVecSyncList->end());

	}

	for (long kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
	{
		if (pSync->m_ndx[kPanel] == -1)
		{
			pNew->m_ndx[kPanel] = -1;
			pNew->m_len[kPanel] = -1;
		}
		else if (kRow < pSync->m_len[kPanel])
		{
			pNew->m_ndx[kPanel]  = pSync->m_ndx[kPanel] + kRow;
			pNew->m_len[kPanel]  = pSync->m_len[kPanel] - kRow;
			pSync->m_len[kPanel] = kRow;
		}
		else
		{
			pNew->m_ndx[kPanel] = pSync->m_ndx[kPanel] + pSync->m_len[kPanel];
			pNew->m_len[kPanel] = 0;
		}

		pNew->m_contextAttrsSpan[kPanel] = pSync->m_contextAttrsSpan[kPanel];
	}

	_insert_after(pSync,pNew);
}

//////////////////////////////////////////////////////////////////

long de_sync_list::getTotalRows(void) const
{
	// return the total number of rows required to display
	// the files w/o regard to the display-ops.  that is,
	// our answer should match the size of the display-list
	// built from this sync-list when DOP is set to _ALL
	// and we are not hiding anything.
	//
	// WARNING: this is a little expensive, so cache the
	// WARNING: result if you can.

	long total = 0;

	for (const de_sync * pSync=m_pHead; (pSync && (pSync != m_pTail)); pSync=pSync->getNext())
		total += pSync->getMaxLen();

	//wxLogTrace(TRACE_DE_DUMP,_T("de_sync_list::getTotalRows: [%ld]"),total);

	return total;
}

long de_sync_list::getTotalRows2(void) const
{
	// return the total number of rows required to display
	// the files w/o regard to the display-ops.  that is,
	// our answer should match the size of the display-list
	// built from this sync-list when DOP is set to _ALL
	// and we are not hiding anything.
	//
	// WARNING: this is a little expensive, so cache the
	// WARNING: result if you can.

	wxASSERT_MSG( (PANEL_T1==PANEL_EDIT), _T("Coding Error"));
	
	long total = 0;

	for (const de_sync * pSync=m_pHead; (pSync && (pSync != m_pTail)); pSync=pSync->getNext())
	{
		long lenT1 = pSync->m_len[PANEL_T1];
		long lenK  = pSync->m_len[PANEL_T0];

		total += MyMax(lenT1,lenK);
	}

	//wxLogTrace(TRACE_DE_DUMP,_T("de_sync_list::getTotalRows2: [%ld]"),total);

	return total;
}

long de_sync_list::getTotalRows3(void) const
{
	// return the total number of rows required to display
	// the files w/o regard to the display-ops.  that is,
	// our answer should match the size of the display-list
	// built from this sync-list when DOP is set to _ALL
	// and we are not hiding anything.
	//
	// WARNING: this is a little expensive, so cache the
	// WARNING: result if you can.

	long total = 0;

	for (const de_sync * pSync=m_pHead; (pSync && (pSync != m_pTail)); pSync=pSync->getNext())
	{
		long lenT1 = pSync->m_len[PANEL_T1];
		long lenT0 = pSync->m_len[PANEL_T0];
		long lenT2 = pSync->m_len[PANEL_T2];

		total += MyMax(lenT1, MyMax(lenT0,lenT2));
	}

	//wxLogTrace(TRACE_DE_DUMP,_T("de_sync_list::getTotalRows3: [%ld]"),total);

	return total;
}

//////////////////////////////////////////////////////////////////
// Intra-Line Diff Related stuff
//////////////////////////////////////////////////////////////////

/*static*/ de_sync_list * de_sync_list::createIntraLineSyncList2(de_attr initialChangeKind,
																 const de_line * pDeLineT1,
																 const de_line * pDeLineT0,
																 const rs_ruleset * pRS)
{
	// create a new sync-list representing an intra-line diff on the given content.

	// we have 3 types of RuleSet context:
	// [1] Important -- content matched by an "important" context rule.
	//     this might be for string literals.
	// [2] Unimportant -- content matched by an "unimportant" context
	//     rule.  this might be for source comments.
	// [3] Default/Null -- content not matched by any context rule.
	//     the importance of this content is governed by the default
	//     context attrs in the RuleSet.

	rs_context_attrs defaultContextAttrs = pRS->getDefaultContextAttrs();
	rs_context_attrs matchStripAttrs     = pRS->getMatchStripAttrs();
	bool bMatchEOLs = RS_ATTRS_RespectEOL(matchStripAttrs);

	// create a new sync-list for this intra-line info.
	// setup for starting CSS using cssString (we don't really
	// need it if we take the short-cut, but it's already has
	// code to do the various length calculations that we need
	// anyway, so we just use it).

	de_sync_list * pThis = new de_sync_list();
	de_css_src_string cssString(pDeLineT1,pDeLineT0,bMatchEOLs);

	if (    !pDeLineT1
		 || !pDeLineT0
		 || (pDeLineT1->getExactUID()==pDeLineT0->getExactUID()))
	{
		// a short-cut:
		// 
		// if either side is null or both sides have identical content,
		// we can create a trivial intra-line-sync-list without the
		// expense of running the full CSS algorithm.
		//
		// if (pDeLineT1 && pDeLineT0) then we got here because we have 2
		// non-void lines that are an exact match.  this should not happen
		// because we're only called for single-line changes -- but if it
		// does, we still want to take the short-cut.

		long lenT1   = cssString.getLenA();
		long lenT0   = cssString.getLenB();
		if ((lenT1 > 0) || (lenT0 > 0))
		{
			de_attr attr = ( (!pDeLineT1 || !pDeLineT0) ? DE_ATTR_DIF_0EQ : DE_ATTR_DIF_2EQ );
			pThis->_append( new de_sync(PANEL_T1, 0,lenT1, PANEL_T0, 0,lenT0, attr       ) );
		}
		
		pThis->_append( new de_sync(PANEL_T1, lenT1,0, PANEL_T0, lenT0,0, DE_ATTR_EOF) );
	}
	else
	{
		// cannot use the short-cut:
		// 
		// run the CSS algorithm on the body of the given lines

		de_css_list cssl;
		cssl.runAlgorithm(&cssString);

		// create a new sync-list and load the CSS into it -- this will do
		// the matchups and voids.

		pThis->load_T1X(&cssl);

		int smoothingThreshold = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD);
		if (smoothingThreshold > 0)
			pThis->applySmoothing2(smoothingThreshold);
	}

	// use the per-line span-context-list to apply context-info to each sync-node.
	// also sub-divide the sync-list into nodes that don't cross contexts.
	
	if (pDeLineT1) pThis->_applyIntraLineContexts(pDeLineT1,PANEL_T1,defaultContextAttrs);
	if (pDeLineT0) pThis->_applyIntraLineContexts(pDeLineT0,PANEL_T0,defaultContextAttrs);
#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("IntraLine: after applyContexts"));
	pThis->dump(15);
#endif
#endif

	//////////////////////////////////////////////////////////////////
	// run thru the sync-list and decide which sync-nodes are important/unimportant.
	//
	// get a local copy of the lines of text.  normally we could just use the
	// getStrLine() by itself, but if the ruleset has respect-eol turned
	// on in the global/default context, the sync-list will also cover the
	// eol chars, so we need to be able to address them as well.

	bool bRespectEOLs = RS_ATTRS_RespectEOL( pRS->getMatchStripAttrs() );
	
	wxString strLineT1, strLineT0;
	if (pDeLineT1)
	{
		strLineT1 += pDeLineT1->getStrLine();
		if (bRespectEOLs)
			strLineT1 += pDeLineT1->getStrEOL();
	}
	if (pDeLineT0)
	{
		strLineT0 += pDeLineT0->getStrLine();
		if (bRespectEOLs)
			strLineT0 += pDeLineT0->getStrEOL();
	}

	pThis->applyIntraLineImportance2(strLineT1,strLineT0, initialChangeKind, pDeLineT1, pDeLineT0, pRS);

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("IntraLine: after applyImportance"));
	pThis->dump(20);
#endif
#endif

	return pThis;
}

//////////////////////////////////////////////////////////////////

void de_sync_list::_applyIntraLineContexts(const de_line * pDeLine, PanelIndex kPanel, rs_context_attrs defaultContextAttrs)
{
	//////////////////////////////////////////////////////////////////
	// THIS FUNCTION IS ONLY USED WHEN WE REFER TO AN INTRA-LINE SYNC-LIST.
	//////////////////////////////////////////////////////////////////
	//
	// run thru the intra-line sync-list and apply RuleSet Contexts
	// from the per-line spans.  we want to split sync nodes that
	// cross contexts -- so that we only have one context in a node
	// (for a panel).  [each panel may have a different context, but
	// within one panel it won't]  for example, with C/C++ source:
	//
	// Panel A:  String abc = "123" // hello
	// Panel B:  String abc = def
	//
	// We cut the change ["123" // hello] vs [def] into 3 parts --
	// a string, default context (the space), and a comment.
	// the first change ["123"] vs [def] after the splits are of
	// in different contexts, but that's ok.

	de_sync *       pSync = m_pHead;

	const de_span * pSpan = pDeLine->getFirstSpan();
	if (pSpan)										// see note [1]
	{
		size_t offsetSpan     = pSpan->getOffset();
		size_t lenSpan        = pSpan->getLen();

		while (pSync && !pSync->isEND())
		{
			size_t offsetSync = pSync->m_ndx[kPanel];
			size_t lenSync    = pSync->m_len[kPanel];

			MY_ASSERT( (offsetSpan==offsetSync) );

			const rs_context * pContext = pSpan->getContext();
			pSync->setSpanContextAttrs(kPanel, ((pContext) ? pContext->getContextAttrs() : defaultContextAttrs));

			if (lenSpan < lenSync)
			{
				// the context changed in this middle of this sync node.

				split(pSync,(long)lenSpan);			// split the node at context change.
				pSync      = pSync->getNext();		// advance to new second half.

				pSpan      = pSpan->getNext();		// advance to next span
				if (!pSpan) break;					// see note [1]
				offsetSpan = pSpan->getOffset();
				lenSpan    = pSpan->getLen(); 
			}
			else if (lenSpan > lenSync)
			{
				// the current context extends past this sync node

				pSync      = pSync->getNext();		// advance to next sync node

				offsetSpan += lenSync;				// pretend that we consumed part
				lenSpan    -= lenSync;				// of this span
			}
			else /* lenSpan == lenSync */
			{
				// both ended at the same point

				pSync      = pSync->getNext();		// advance to next sync node
			
				pSpan      = pSpan->getNext();		// advance to next span
				if (!pSpan) break;					// see note [1]
				offsetSpan = pSpan->getOffset();
				lenSpan    = pSpan->getLen(); 
			}
		}
	}
	
	//////////////////////////////////////////////////////////////////
	// Note [1]: in theory both the sync list and the span list should
	// end together.  but the sync list will always have an "EOF" (aka
	// EOL aka END) node.  the span list only includes the EOL if
	// respect-eol was set globally and optionally in the context.  so
	// there are times when the span list will end before the sync list.
	//
	// that is, the sync list always has an END node -- which does not
	// represent content -- it's a node to represent the attributes open
	// at the end of the line.
	//
	// on the span side, if respect-eol was set, the last span will
	// cover the actual chars used in the eol.
	//////////////////////////////////////////////////////////////////

	const rs_context * pCTXEOL = pDeLine->getContextEOL();
	rs_context_attrs attrs = ((pCTXEOL) ? pCTXEOL->getContextAttrs() : defaultContextAttrs);

	while (pSync)
	{
		pSync->setSpanContextAttrs(kPanel, attrs);
		pSync = pSync->getNext();
	}
}

void de_sync_list::applyMultiLineIntraLineContexts(de_de * pDeDe, long kSync, PanelIndex kPanel,
												   long ndxLineCmp, long nrLines,
												   const std::vector<long> & vecLineLens,
												   const std::vector<bool> & vecEols,
												   rs_context_attrs defaultContextAttrs)
{
	if (nrLines == 0)
		return;

	//UTIL_PERF_START_CLOCK(_T("de_sync_list::applyMultiLineIntraLineContexts"));

	//////////////////////////////////////////////////////////////////
	// THIS FUNCTION IS ONLY USED WHEN WE REFER TO AN INTRA-LINE SYNC-LIST.
	//////////////////////////////////////////////////////////////////
	//
	// run thru the intra-line sync-list and apply RuleSet Contexts
	// from the per-line spans.  we want to split sync nodes that
	// cross contexts -- so that we only have one context in a node
	// (for a panel).  [each panel may have a different context, but
	// within one panel it won't]
	//
	// this is a multi-line version of _applyIntraLineContexts() that
	// is used when we are doing complex multi-line intra-line analysis.

	de_sync * pSync = m_pHead;
	long offsetLineK = 0;
	const de_line * pDeLineLast = NULL;

	for (long kLine=0; kLine<nrLines; kLine++)
	{
		const de_line * pDeLineK = pDeDe->getDeLineFromNdx(kSync,kPanel,ndxLineCmp+kLine);
		if (!pDeLineK)
			continue;

		pDeLineLast = pDeLineK;		// remember last non-void line in the multi-line set.

		long lenLineK = vecLineLens[kLine];
		bool bIncludedEOL = vecEols[kLine];

		const de_span * pSpan = pDeLineK->getFirstSpan();
		if (pSpan)
		{
			size_t offsetSpan = pSpan->getOffset();
			size_t lenSpan    = pSpan->getLen();

			while (pSync && !pSync->isEND())
			{
				size_t offsetSync = pSync->m_ndx[kPanel];
				size_t lenSync    = pSync->m_len[kPanel];

				MY_ASSERT( (offsetLineK+offsetSpan == offsetSync) );

				const rs_context * pContext = pSpan->getContext();
				pSync->setSpanContextAttrs(kPanel, ((pContext) ? pContext->getContextAttrs() : defaultContextAttrs));

				if (lenSpan > lenSync)
				{
					// the current context extends past this sync node

					pSync      = pSync->getNext();		// advance to next sync node

					offsetSpan += lenSync;				// pretend that we consumed part
					lenSpan    -= lenSync;				// of this span
				}
				else
				{
					if (lenSpan < lenSync)				// the context changed in this middle of this sync node.
						split(pSync,(long)lenSpan);		// split the node at context change.
					// else both ended at the same point

					pSync      = pSync->getNext();		// advance to next (second half if we split).
					pSpan      = pSpan->getNext();		// advance to next span

					if (!pSpan)							// reached end of this line
						break;
					
					offsetSpan = pSpan->getOffset();
					lenSpan    = pSpan->getLen(); 
				}
			}
		}

		// since our sync-list was constructed from a bunch of concatenated
		// lines, there aren't sync-nodes for the EOL chars for the lines
		// -- unless we included EOL's in the analysis.

		if (bIncludedEOL)
		{
			long lenEOL = (long)pDeLineK->getStrEOL().Len();
			while (lenEOL  &&  pSync && !pSync->isEND())
			{
				if (lenEOL < pSync->getLen(kPanel))
					split(pSync,lenEOL);
				lenEOL -= pSync->getLen(kPanel);

				const rs_context * pCTXEOL = pDeLineK->getContextEOL();
				rs_context_attrs attrs = ((pCTXEOL) ? pCTXEOL->getContextAttrs() : defaultContextAttrs);
				pSync->setSpanContextAttrs(kPanel, attrs);

				pSync = pSync->getNext();
			}
		}

		offsetLineK += lenLineK;
	}

	// take care of the end of the sync list -- this may cover the EOL on
	// the last line in the multi-line set and any trailing VOIDS.

	wxASSERT_MSG( (pDeLineLast), _T("Coding Error") );
	if (pDeLineLast && pSync)
	{
		const rs_context * pCTXEOL = pDeLineLast->getContextEOL();
		rs_context_attrs attrs = ((pCTXEOL) ? pCTXEOL->getContextAttrs() : defaultContextAttrs);

		while (pSync)
		{
			pSync->setSpanContextAttrs(kPanel, attrs);
			pSync = pSync->getNext();
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_sync_list::applyMultiLineIntraLineContexts"));
}

//////////////////////////////////////////////////////////////////

static bool s_isAllSpaces(const wxChar * szGiven, size_t offset, size_t len)
{
	// return true if substring only contains spaces

	const wxChar * sz = szGiven+offset;

	for (size_t k=0; k<len; k++)
		if (sz[k] != 0x0020)
			return false;

	return true;
}

static bool s_isAllSpacesOrTabs(const wxChar * szGiven, size_t offset, size_t len)
{
	// return true if substring only contains spaces or tabs

	const wxChar * sz = szGiven+offset;

	for (size_t k=0; k<len; k++)
		if ((sz[k] != 0x0020)  &&  (sz[k] != 0x0009))
			return false;

	return true;
}

static bool s_isWhiteSpace(rs_context_attrs attrs, const wxChar * sz, size_t offset, size_t len)
{
	if (len == 0)
		return false;

	if (RS_ATTRS_TabIsWhite(attrs))
		return s_isAllSpacesOrTabs(sz,offset,len);
	else
		return s_isAllSpaces(sz,offset,len);
}

static bool s_isEOL(const wxChar * szGiven, size_t offset, size_t len)
{
	// return true if substring only contains CR, LF, or CRLF

	if (len == 0)
		return false;

	const wxChar * sz = szGiven+offset;

	if (len == 1)
		return (sz[0] == 0x000a  ||  sz[0] == 0x000d);

	if (len == 2)
		return (sz[0] == 0x000d  &&  sz[1] == 0x000a);

	return false;
}

static bool s_strnicmp(const wxString & str1, size_t offset1, size_t len1,
					   const wxString & str2, size_t offset2, size_t len2)
{
	// return true if they are equal when ignoring case

	if (len1 != len2)	// can't be equal if they are different lengths
		return false;
	
	const wxChar * sz1 = str1.wc_str() + offset1;
	const wxChar * sz2 = str2.wc_str() + offset2;

	for (size_t k=0; k<len1; k++)
		if (wxTolower(sz1[k]) != wxTolower(sz2[k]))
			return false;

	return true;
}

static bool s_ign_case(rs_context_attrs attr1, const wxString & str1, size_t offset1, size_t len1,
					   rs_context_attrs attr2, const wxString & str2, size_t offset2, size_t len2)
{
	// return true if we CAN ignore case on both and if they ARE equal when ignoring case

	return (   !RS_ATTRS_RespectCase(attr1)
			&& !RS_ATTRS_RespectCase(attr2)
			&& s_strnicmp(str1,offset1,len1,
						  str2,offset2,len2));
}

//////////////////////////////////////////////////////////////////

void de_sync_list::_divide_node2(de_sync * pSync, PanelIndex xPanel, PanelIndex yPanel)
{
	//////////////////////////////////////////////////////////////////
	// THIS FUNCTION IS ONLY USED WHEN WE REFER TO AN INTRA-LINE SYNC-LIST.
	//////////////////////////////////////////////////////////////////
	//
	// convert a 's/a/b/' style change into 's/a//' and 's//b/'

	wxASSERT_MSG( (pSync->isSameType(DE_ATTR_DIF_0EQ)), _T("Coding Error!") );

	de_sync * pNew = new de_sync();

	pNew->m_attr = pSync->m_attr;

	wxASSERT_MSG( (!pSync->m_pVecSyncList), _T("Coding Error!") );	// don't use on line-oriented node

	for (long kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
	{
		if (kPanel == xPanel)
		{
			pNew->m_ndx[kPanel] = pSync->m_ndx[kPanel] + pSync->m_len[kPanel];
			pNew->m_len[kPanel] = 0;
		}
		else if (kPanel == yPanel)
		{
			pNew->m_ndx[kPanel]  = pSync->m_ndx[kPanel];
			pNew->m_len[kPanel]  = pSync->m_len[kPanel];
			pSync->m_len[kPanel] = 0;
		}
		else
		{
			wxASSERT_MSG( (pSync->m_ndx[kPanel] == -1), _T("Coding Error!") );

			pNew->m_ndx[kPanel] = -1;
			pNew->m_len[kPanel] = -1;
		}

		pNew->m_contextAttrsSpan[kPanel] = pSync->m_contextAttrsSpan[kPanel];
	}

	_insert_after(pSync,pNew);
}

//////////////////////////////////////////////////////////////////

void de_sync_list::applyIntraLineImportance2(const wxString & strLineT1, const wxString & strLineT0,
											 de_attr initialChangeKind,
											 const de_line * pDeLineLastT1, const de_line * pDeLineLastT0,
											 const rs_ruleset * pRS)
{
	wxASSERT_MSG( (DE_ATTR__IS_KIND(initialChangeKind,DE_ATTR__DIF_KIND)), _T("Coding Error!") );

	//////////////////////////////////////////////////////////////////
	// THIS FUNCTION IS ONLY USED WHEN WE REFER TO AN INTRA-LINE SYNC-LIST.
	//////////////////////////////////////////////////////////////////
	//
	// run thru the intra-line sync-list and use the per-panel-per-sync-node
	// context-attributes to decide if the sync-node should be marked
	// important or unimportant.
	//
	// before we can do that, we need to convert any sync-nodes that are part
	// whitespace (that is, whitespace on one side, but not the other) into
	// separate nodes (that is, convert a CHANGE to an INSERT and a DELETE).
	// mixing whitespace and non-whitespace messes up the important/unimportant
	// computation.  [because of the way spans were applied, we only need to
	// look for whitespace at the start of a sync-node.]
	//
	// finally, also, keep track of the number of changes within the line.
	//
	// this may be used on single-line or multi-line intra-line sync-list.  for
	// the single-line case, the strings should match the content on the given
	// de_lines.  for the multi-line case, the strings should be the concatenation
	// of the lines and the de_lines should be the last non-null line in each panel.
	// and whether EOL's are present or not present depending upon whether they
	// were included in the analysis.
	
	m_cImportantChangesIntraLine = 0;
	m_cChangesIntraLine = 0;
	m_ChangeKindIntraLine = initialChangeKind;

	bool bGlobalRespectEOLs = RS_ATTRS_RespectEOL( pRS->getMatchStripAttrs() );

	for (de_sync * pSync=m_pHead; (pSync && !pSync->isEND()); pSync=pSync->getNext())
	{
		size_t offsetT1 = pSync->getNdx(PANEL_T1);	size_t lenT1 = pSync->getLen(PANEL_T1);
		size_t offsetT0 = pSync->getNdx(PANEL_T0);	size_t lenT0 = pSync->getLen(PANEL_T0);

		rs_context_attrs attrT1 = pSync->getSpanContextAttrs(PANEL_T1);
		rs_context_attrs attrT0 = pSync->getSpanContextAttrs(PANEL_T0);

		if (pSync->isSameType(DE_ATTR_DIF_0EQ))
		{
			int bIsWhiteT1 = s_isWhiteSpace(attrT1,strLineT1,offsetT1,lenT1) ? 1 : 0;
			int bIsWhiteT0 = s_isWhiteSpace(attrT0,strLineT0,offsetT0,lenT0) ? 1 : 0;
			int cIsWhite   = bIsWhiteT1 + bIsWhiteT0;

			// [when RS_ATTR_TAB_IS_WHITE is set]
			// whitespace can either be spaces or tabs.  the intra-line diff-engine
			// sees spaces and tabs as different characters (because they are).  for
			// our purposes here (separating white from non-white in a single node),
			// we don't care about the space vs tab distinction.  for example, if we
			// have a _DIF_0EQ node and one side is SPACE and one side is TAB, we
			// don't need to do the _divide_node2() -- both T0 and T1 will have the
			// same importance.

			if ((cIsWhite == 1)  &&  (lenT1 > 0)  &&  (lenT0 > 0) )
			{
				_divide_node2(pSync,PANEL_T1,PANEL_T0);
		
				offsetT1 = pSync->getNdx(PANEL_T1);	lenT1 = pSync->getLen(PANEL_T1);
				offsetT0 = pSync->getNdx(PANEL_T0);	lenT0 = pSync->getLen(PANEL_T0);
			}
		}
		
		// if content in both are completely unimportant, then we can just stop
		// here and say the change is unimportant.
		//
		// if either or both are important, then we need to look further to see
		// if it only contains an ignorable change (such as whitespace) before
		// we decide.  otherwise, we error on side of safety and conclude that
		// this is important.

		bool bU1 = RS_ATTRS_IsUnimportant(attrT1);
		bool bU0 = RS_ATTRS_IsUnimportant(attrT0);

		if (bU1 && bU0)								// are both completely unimportant
			pSync->setUnimportant(true);
		else										// the overall context is important
		{
			if ((lenT1 == 0) && (lenT0 == 0))		// both panels empty -- should not happen
			{
				wxASSERT_MSG( (0), _T("Coding Error!") );
			}
			else
			{
				// if already unimportant || (we have whitespace and are not respecting it)
				// || (we have EOL in the analysis, we have an EOL, and we are not respecting it),
				// then set unimportant.

				if (lenT1 > 0)
					bU1 =  (   bU1
							|| (   (!RS_ATTRS_RespectWhite(attrT1))
								&& (s_isWhiteSpace(attrT1,strLineT1,offsetT1,lenT1)))
							|| (   bGlobalRespectEOLs
								&& !RS_ATTRS_RespectEOL(attrT1)
								&& s_isEOL(strLineT1,offsetT1,lenT1)));
				if (lenT0 > 0)
					bU0 =  (   bU0
							|| (   (!RS_ATTRS_RespectWhite(attrT0))
								&& (s_isWhiteSpace(attrT0,strLineT0,offsetT0,lenT0)))
							|| (   bGlobalRespectEOLs
								&& !RS_ATTRS_RespectEOL(attrT0)
								&& s_isEOL(strLineT0,offsetT0,lenT0)));
					
				if (lenT0 == 0)						// panel-T1 is the only one with actual content
				{
					wxASSERT_MSG( (pSync->isSameType(DE_ATTR_DIF_0EQ)), _T("Coding Error!") );

					pSync->setUnimportant(bU1);
				}
				else if (lenT1 == 0)				// panel-T0 is the only one with actual content
				{
					wxASSERT_MSG( (pSync->isSameType(DE_ATTR_DIF_0EQ)), _T("Coding Error!") );

					pSync->setUnimportant(bU0);
				}
				else /* ((lenT1 > 0) && (lenT0 > 0)) */	// both panels have content
				{
					// if we still think one or both sides are important,
					// see if we can mark them unimportant if only a change
					// of case, if we don't respect case.

					bool bUBoth = bU1 & bU0;		// are both already known to be unimportant

					if (   !bUBoth											// if we still think one or both are important
						&& pSync->isSameType(DE_ATTR_DIF_0EQ)				// and we are different
						&& s_ign_case(attrT1,strLineT1,offsetT1,lenT1,		// and they are equal when ignoring case (if we can do that),
									  attrT0,strLineT0,offsetT0,lenT0))
						pSync->setUnimportant(true);						// then the change is really unimportant.
					else													// otherwise, we either dont' need to check case
						pSync->setUnimportant(bUBoth);						// or we already know the answer.
				}
			}
		}

		if (pSync->isSameType(DE_ATTR_DIF_0EQ))
		{
			m_ChangeKindIntraLine &= DE_ATTR_DIF_0EQ;
			m_cChangesIntraLine++;
			if (!pSync->isUnimportant())
				m_cImportantChangesIntraLine++;
		}
	}

	if (m_pHead && m_pHead->isEND())
	{
		// if the intra-line sync-list is just an END marker, then the above
		// loop did not get entered.  this happens for blank lines (when the
		// EOL chars are not included in the line).

		wxASSERT_MSG( (!DE_ATTR__IS_TYPE(initialChangeKind,DE_ATTR_DIF_2EQ)), _T("Coding Error") );
		if (DE_ATTR__IS_TYPE(initialChangeKind,DE_ATTR_DIF_2EQ))
		{
			wxASSERT_MSG( ((pDeLineLastT1!=NULL) && (pDeLineLastT0!=NULL)), _T("Coding Error!") );
			// this line is not a change -- we have a blank line opposite a blank line.
		}
		else /* _0EQ */
		{
			wxASSERT_MSG( ((pDeLineLastT1==NULL) || (pDeLineLastT0==NULL)), _T("Coding Error!") );
			// we have a blank line opposite a void.

			m_cChangesIntraLine++;

			// the importance of the line is determined by the context of the EOL.

			bool bImportant = false;
			if (pDeLineLastT1)
			{
				const rs_context * pCtxEol = pDeLineLastT1->getContextEOL();
				rs_context_attrs rsAttrs = ((pCtxEol) ? pCtxEol->getContextAttrs() : pRS->getDefaultContextAttrs());
				if (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs))	// if overall-is-important and blank-lines-are-important
					bImportant = true;
			}
			if (!bImportant && pDeLineLastT0)
			{
				const rs_context * pCtxEol = pDeLineLastT0->getContextEOL();
				rs_context_attrs rsAttrs = ((pCtxEol) ? pCtxEol->getContextAttrs() : pRS->getDefaultContextAttrs());
				if (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs))	// if overall-is-important and blank-lines-are-important
					bImportant = true;
			}
			if (bImportant)
				m_cImportantChangesIntraLine++;
		}
	}

	wxASSERT_MSG( (DE_ATTR__IS_KIND(m_ChangeKindIntraLine,DE_ATTR__DIF_KIND)), _T("Coding Error"));
}

//////////////////////////////////////////////////////////////////

/*static*/ de_sync_list * de_sync_list::createIntraLineSyncList3(de_attr initialChangeKind,
																 const de_line * pDeLineT1, const de_line * pDeLineT0, const de_line * pDeLineT2,
																 const rs_ruleset * pRS)
{
	// create a new sync-list representing an intra-line diff on the given content

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("createIntraLineSyncList3:"));
	if (pDeLineT1) pDeLineT1->dump(5);
	if (pDeLineT0) pDeLineT0->dump(10);
	if (pDeLineT2) pDeLineT2->dump(15);
#endif
#endif

	// we have 3 types of RuleSet context:
	// [1] Important -- content matched by an "important" context rule.
	//     this might be for string literals.
	// [2] Unimportant -- content matched by an "unimportant" context
	//     rule.  this might be for source comments.
	// [3] Default/Null -- content not matched by any context rule.
	//     the importance of this content is governed by the default
	//     context attrs in the RuleSet.

	rs_context_attrs defaultContextAttrs = pRS->getDefaultContextAttrs();
	rs_context_attrs matchStripAttrs     = pRS->getMatchStripAttrs();
	bool bMatchEOLs = RS_ATTRS_RespectEOL(matchStripAttrs);

	// create a new sync-list for this intra-line info.
	// setup for starting CSS using cssString (we don't really
	// need it if we take the short-cut, but it's already has
	// code to do the various length calculations that we need
	// anyway, so we just use it).

	de_sync_list * pThis = new de_sync_list();
	de_css_src_string cssString0(pDeLineT1,pDeLineT0,bMatchEOLs);
	de_css_src_string cssString2(pDeLineT1,pDeLineT2,bMatchEOLs);
	de_css_src_string cssString02(pDeLineT0,pDeLineT2,bMatchEOLs);

	// try to short-cut the construction of the intra-line list.
	// if 2 of 3 are void, we can create a trivial intra-line list
	// without the expense of running the full CSS algorithm.

	long lenT1 = cssString0.getLenA();
	long lenT0 = cssString0.getLenB();
	long lenT2 = cssString2.getLenB();
	
	if (!pDeLineT1 && !pDeLineT0)	// T1 & T0 are void, T2 should have content -- short-cut
	{
		wxASSERT_MSG( (pDeLineT2), _T("Coding Error") );
		if (lenT2 > 0)
			pThis->_append( new de_sync(PANEL_T1,0,lenT1, PANEL_T0,0,lenT0, PANEL_T2,0,lenT2, DE_ATTR_MRG_T1T0EQ) );
		pThis->_append( new de_sync(PANEL_T1,lenT1,0, PANEL_T0,lenT0,0, PANEL_T2,lenT2,0, DE_ATTR_EOF) );
	}
	else if (!pDeLineT1 && !pDeLineT2)	// T1 & T2 are void, T0 should have content -- short-cut
	{
		wxASSERT_MSG( (pDeLineT0), _T("Coding Error") );
		if (lenT0 > 0)
			pThis->_append( new de_sync(PANEL_T1,0,lenT1, PANEL_T0,0,lenT0, PANEL_T2,0,lenT2, DE_ATTR_MRG_T1T2EQ) );
		pThis->_append( new de_sync(PANEL_T1,lenT1,0, PANEL_T0,lenT0,0, PANEL_T2,lenT2,0, DE_ATTR_EOF) );
	}
	else if (!pDeLineT0 && !pDeLineT2)	// T0 & T2 are void, T1 should have content -- short-cut
	{
		wxASSERT_MSG( (pDeLineT1), _T("Coding Error") );
		if (lenT1 > 0)
			pThis->_append( new de_sync(PANEL_T1,0,lenT1, PANEL_T0,0,lenT0, PANEL_T2,0,lenT2, DE_ATTR_MRG_T0T2EQ) );
		pThis->_append( new de_sync(PANEL_T1,lenT1,0, PANEL_T0,lenT0,0, PANEL_T2,lenT2,0, DE_ATTR_EOF) );
	}
	else
	{
		// cannot use the short-cut:
		//
		// run the CSS algorithm on the body of the given lines

		de_css_list cssl0;
		cssl0.runAlgorithm(&cssString0);
	
		de_css_list cssl2;
		cssl2.runAlgorithm(&cssString2);

		pThis->load_T1T0T2(&cssl0,&cssl2,&cssString02,true,true);

		int smoothingThreshold = (int)gpGlobalProps->getLong(GlobalProps::GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD);
		if (smoothingThreshold > 0)
			pThis->applySmoothing3(smoothingThreshold);
	}
	
	// use the per-line span-context-list to apply context-info to each sync-node.
	// also sub-divide the sync-list into nodes that don't cross contexts.
	
	if (pDeLineT1) pThis->_applyIntraLineContexts(pDeLineT1,PANEL_T1,defaultContextAttrs);
	if (pDeLineT0) pThis->_applyIntraLineContexts(pDeLineT0,PANEL_T0,defaultContextAttrs);
	if (pDeLineT2) pThis->_applyIntraLineContexts(pDeLineT2,PANEL_T2,defaultContextAttrs);

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("createIntraLineSyncList3: after applyContexts"));
	pThis->dump(25);
#endif
#endif

	//////////////////////////////////////////////////////////////////
	// run thru the sync-list and decide which sync-nodes are important/unimportant.
	// and update m_ChangeKindIntraLine based upon the types of changes we see on the line.
	//
	// get a local copy of the lines of text.  normally we could just use the
	// getStrLine() by itself, but if the ruleset has respect-eol turned
	// on in the global/default context, the sync-list will also cover the
	// eol chars, so we need to be able to address them as well.

	bool bRespectEOLs = RS_ATTRS_RespectEOL( pRS->getMatchStripAttrs() );

	wxString strLineT1, strLineT0, strLineT2;
	if (pDeLineT1)
	{
		strLineT1 += pDeLineT1->getStrLine();
		if (bRespectEOLs)
			strLineT1 += pDeLineT1->getStrEOL();
	}
	if (pDeLineT0)
	{
		strLineT0 += pDeLineT0->getStrLine();
		if (bRespectEOLs)
			strLineT0 += pDeLineT0->getStrEOL();
	}
	if (pDeLineT2)
	{
		strLineT2 += pDeLineT2->getStrLine();
		if (bRespectEOLs)
			strLineT2 += pDeLineT2->getStrEOL();
	}

	pThis->applyIntraLineImportance3(strLineT1,strLineT0,strLineT2, initialChangeKind,pDeLineT1,pDeLineT0,pDeLineT2,pRS);

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("createIntraLineSyncList3: after applyImportance"));
	pThis->dump(30);
#endif
#endif

	return pThis;
}

//////////////////////////////////////////////////////////////////

void de_sync_list::normalize_ChangeKindIntraLine3(void)
{
	// normalize the overall change-kind for the line (we use 3 bits
	// to indicate what kind of EQ we have -- see de_dcl.h -- so either
	// 0 bits, 2 bits, or 3 bits should be set.  the &= at [1] can leave
	// us with only 1 bit set -- this needs to be cleared.

	switch (m_ChangeKindIntraLine & DE_ATTR__TYPE_MASK)
	{
	default:
		m_ChangeKindIntraLine = DE_ATTR_MRG_0EQ;
		break;

	case DE_ATTR_MRG_0EQ:
	case DE_ATTR_MRG_T0T2EQ:
	case DE_ATTR_MRG_T1T2EQ:
	case DE_ATTR_MRG_T1T0EQ:
	case DE_ATTR_MRG_3EQ:
		break;
	}
}

//////////////////////////////////////////////////////////////////

void de_sync_list::applyIntraLineImportance3(const wxString & strLineT1, const wxString & strLineT0, const wxString & strLineT2,
											 de_attr initialChangeKind,
											 const de_line * pDeLineLastT1, const de_line * pDeLineLastT0, const de_line * pDeLineLastT2,
											 const rs_ruleset * pRS)
{
#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("applyIntraLineImportance3: [initAttr %x]\n\tstrT1 [%s]\n\tstrT0 [%s]\n\tstrT2 [%s]"),
			   initialChangeKind,strLineT1.wc_str(),strLineT0.wc_str(),strLineT2.wc_str());
#endif
#endif

	wxASSERT_MSG( (DE_ATTR__IS_KIND(initialChangeKind,DE_ATTR__MRG_KIND)), _T("Coding Error") );

	//////////////////////////////////////////////////////////////////
	// THIS FUNCTION IS ONLY USED WHEN WE REFER TO AN INTRA-LINE SYNC-LIST.
	//////////////////////////////////////////////////////////////////
	//
	// run thru the intra-line sync-list and use the per-panel-per-sync-node
	// context-attributes to decide if the sync-node should be marked
	// important or unimportant.
	//
	// before we can do that, we need to convert any sync-nodes that are part
	// whitespace (that is, whitespace on one side, but not the other) into
	// separate nodes (that is, convert a CHANGE to an INSERT and a DELETE).
	// mixing whitespace and non-whitespace messes up the important/unimportant
	// computation.  [because of the way spans were applied, we only need to
	// look for whitespace at the start of a sync-node.]
	//
	// finally, also, keep track of the number of changes on the line.
	//
	// this may be used on single-line or multi-line intra-line sync-list.  for
	// the single-line case, the strings should match the content on the given
	// de_lines.  for the multi-line case, the strings should be the concatenation
	// of the lines and the de_lines should be the last non-null line in each panel.
	// and whether EOL's are present or not present depending upon whether they
	// were included in the analysis.

	m_cImportantChangesIntraLine = 0;
	m_cChangesIntraLine = 0;
	m_ChangeKindIntraLine = initialChangeKind;

	bool bGlobalRespectEOLs = RS_ATTRS_RespectEOL( pRS->getMatchStripAttrs() );

	for (de_sync * pSync=m_pHead; (pSync && !pSync->isEND()); pSync=pSync->getNext())
	{
		size_t offsetT1 = pSync->getNdx(PANEL_T1);	size_t lenT1 = pSync->getLen(PANEL_T1);
		size_t offsetT0 = pSync->getNdx(PANEL_T0);	size_t lenT0 = pSync->getLen(PANEL_T0);
		size_t offsetT2 = pSync->getNdx(PANEL_T2);	size_t lenT2 = pSync->getLen(PANEL_T2);

		rs_context_attrs attrT1 = pSync->getSpanContextAttrs(PANEL_T1);
		rs_context_attrs attrT0 = pSync->getSpanContextAttrs(PANEL_T0);
		rs_context_attrs attrT2 = pSync->getSpanContextAttrs(PANEL_T2);

		int bIsWhiteT1 = s_isWhiteSpace(attrT1,strLineT1,offsetT1,lenT1) ? 1 : 0;
		int bIsWhiteT0 = s_isWhiteSpace(attrT0,strLineT0,offsetT0,lenT0) ? 1 : 0;
		int bIsWhiteT2 = s_isWhiteSpace(attrT2,strLineT2,offsetT2,lenT2) ? 1 : 0;
		int cIsWhite   = bIsWhiteT1 + bIsWhiteT0 + bIsWhiteT2;

		// [when RS_ATTR_TAB_IS_WHITE is set]
		// whitespace can be either spaces or tabs.  the intra-line diff-engine
		// sees spaces and tabs as different characters (because they are).  for
		// our purposes here (separating white from non-white in a single node),
		// we don't care about the space vs tab distinction.  for example, if we
		// have a _MRG_T0T2EQ node and T0 & T2 have a SPACE and T1 has a TAB, we
		// don't need to do the _divide_node3_21() -- both T0,T2 and T1 will have
		// the same importance.

		bool bDivided = false;

		switch (cIsWhite)
		{
		case 3:		// all 3 of the panels in this sync node contains whitespace.
			break;	// no need to divide

		case 2:		// two of the panels in this sync node contains whitespace.
			if (!bIsWhiteT1)			// if T0,T2 white; T1 not white
			{
				wxASSERT_MSG( ((lenT0 > 0) && (lenT2 > 0)), _T("Coding Error") );
				if (lenT1 > 0)
				{
					_divide_node3_21(pSync,PANEL_T0,PANEL_T2,PANEL_T1);	// put T0,T2 in one node, T1 in another
					bDivided = true;
				}
			}
			else if (!bIsWhiteT0)		// if T1,T2 white; T0 not white
			{
				wxASSERT_MSG( ((lenT1 > 0) && (lenT2 > 0)), _T("Coding Error") );
				if (lenT0 > 0)
				{
					_divide_node3_21(pSync,PANEL_T1,PANEL_T2,PANEL_T0);	// put T1,T2 in one node, T0 in another
					bDivided = true;
				}
			}
			else
			{
				wxASSERT_MSG( (!bIsWhiteT2), _T("Coding Error") );
				wxASSERT_MSG( ((lenT1 > 0) && (lenT0 > 0)), _T("Coding Error") );
				if (lenT2 > 0)
				{
					_divide_node3_21(pSync,PANEL_T1,PANEL_T0,PANEL_T2);
					bDivided = true;
				}
			}
			break;

		case 1:		// one of the panels in this sync node contains whitespace.
			if (bIsWhiteT1)				// T1 white; T0,T2 not white
			{
				wxASSERT_MSG( (lenT1 > 0), _T("Coding Error") );
				if ((lenT0 > 0) || (lenT2 > 0))
				{
					_divide_node3_21(pSync,PANEL_T0,PANEL_T2,PANEL_T1);	// put T0,T2 in one node, T1 in another
					bDivided = true;
				}
			}
			else if (bIsWhiteT0)		// T0 white; T1,T2 not white
			{
				wxASSERT_MSG( (lenT0 > 0), _T("Coding Error") );
				if ((lenT1 > 0) || (lenT2 > 0))
				{
					_divide_node3_21(pSync,PANEL_T1,PANEL_T2,PANEL_T0);	// put T1,T2 in one node, T0 in another
					bDivided = true;
				}
			}
			else						// T2 white; T1,T0 not white
			{
				wxASSERT_MSG( (bIsWhiteT2), _T("Coding Error") );
				wxASSERT_MSG( (lenT2 > 0), _T("Coding Error") );
				if ((lenT1 > 0) || (lenT0 > 0))
				{
					_divide_node3_21(pSync,PANEL_T1,PANEL_T0,PANEL_T2);
					bDivided = true;
				}
			}
			break;

		default:	// none of the panels in this sync node contains whitespace.
			break;	// no need to divide
		}
		
		// if content in all 3 are completely unimportant, then we can just stop
		// here and say the change is unimportant.
		//
		// if any are important, then we need to look further to see
		// if it only contains an ignorable change (such as whitespace) before
		// we decide.  otherwise, we error on side of safety and conclude that
		// this is important.
		
		bool bU1 = RS_ATTRS_IsUnimportant(attrT1);
		bool bU0 = RS_ATTRS_IsUnimportant(attrT0);
		bool bU2 = RS_ATTRS_IsUnimportant(attrT2);

		if (bU1 && bU0 && bU2)												// are all completely unimportant
			pSync->setUnimportant(true);
		else																// the overall context is important (in at least one panel)
		{
			if (bDivided)		// recompute offsets,lengths,isWhite because we divided the node.
			{
				offsetT1 = pSync->getNdx(PANEL_T1);		lenT1 = pSync->getLen(PANEL_T1);
				offsetT0 = pSync->getNdx(PANEL_T0); 	lenT0 = pSync->getLen(PANEL_T0);
				offsetT2 = pSync->getNdx(PANEL_T2);		lenT2 = pSync->getLen(PANEL_T2);

				bIsWhiteT1 = s_isWhiteSpace(attrT1,strLineT1,offsetT1,lenT1) ? 1 : 0;
				bIsWhiteT0 = s_isWhiteSpace(attrT0,strLineT0,offsetT0,lenT0) ? 1 : 0;
				bIsWhiteT2 = s_isWhiteSpace(attrT2,strLineT2,offsetT2,lenT2) ? 1 : 0;
				cIsWhite   = bIsWhiteT1 + bIsWhiteT0 + bIsWhiteT2;
			}

			// if already unimportant || (we have whitespace and are not respecting it)
			// || (we have EOL in the analysis, we have an EOL, and we are not respecting it),
			// then set unimportant.

			if (lenT1 > 0)
				bU1 =  (   bU1
						|| (   (bIsWhiteT1))
							&& (!RS_ATTRS_RespectWhite(attrT1))
						|| (   bGlobalRespectEOLs
							&& !RS_ATTRS_RespectEOL(attrT1)
							&& s_isEOL(strLineT1,offsetT1,lenT1)));
			if (lenT0 > 0)
				bU0 =  (   bU0
						|| (   (bIsWhiteT0))
							&& (!RS_ATTRS_RespectWhite(attrT0))
						|| (   bGlobalRespectEOLs
							&& !RS_ATTRS_RespectEOL(attrT0)
							&& s_isEOL(strLineT0,offsetT0,lenT0)));
			if (lenT2 > 0)
				bU2 =  (   bU2
						|| (   (bIsWhiteT2))
							&& (!RS_ATTRS_RespectWhite(attrT2))
						|| (   bGlobalRespectEOLs
							&& !RS_ATTRS_RespectEOL(attrT2)
							&& s_isEOL(strLineT2,offsetT2,lenT2)));

			if ((lenT1 == 0) && (lenT0 == 0) && (lenT2 == 0))				// all panels empty -- should not happen
			{
				wxASSERT_MSG( (0), _T("Coding Error!") );
			}
			else if ((lenT0 == 0) && (lenT2 == 0))							// panel T1 is the only one with content
			{
				pSync->setUnimportant(bU1);
			}
			else if ((lenT1 == 0) && (lenT0 == 0))							// panel T2 is the only one with content
			{
				pSync->setUnimportant(bU2);
			}
			else if ((lenT1 == 0) && (lenT2 == 0))							// panel T0 is the only one with content
			{
				pSync->setUnimportant(bU0);
			}
			else if (lenT1 == 0)											// T1 is empty; T0 and T2 have content
			{
				bool bUBoth = bU0 & bU2;									// are both already known to be unimportant
				
				if (   !bUBoth										// if we still think one or both are important
					&& !pSync->isSameType(DE_ATTR_MRG_T0T2EQ)		// and T0,T2 are not equal
					&& s_ign_case(attrT0,strLineT0,offsetT0,lenT0,	// and they are equal when ignoring case (if we can do that),
								  attrT2,strLineT2,offsetT2,lenT2))
					pSync->setUnimportant(true);					// then the change is really unimportant.
				else												// otherwise, we either don't need to check 
					pSync->setUnimportant(bUBoth);					// case or we already know the answer.
			}
			else if (lenT0 == 0)											// T0 is empty; T1 and T2 have content
			{
				bool bUBoth = bU1 & bU2;									// are both already known to be unimportant
				
				if (   !bUBoth										// if we still think one or both are important
					&& !pSync->isSameType(DE_ATTR_MRG_T1T2EQ)		// and T1,T2 are not equal
					&& s_ign_case(attrT1,strLineT1,offsetT1,lenT1,	// and they are equal when ignoring case (if we can do that),
								  attrT2,strLineT2,offsetT2,lenT2))
					pSync->setUnimportant(true);					// then the change is really unimportant.
				else												// otherwise, we either don't need to check 
					pSync->setUnimportant(bUBoth);					// case or we already know the answer.
			}
			else if (lenT2 == 0)											// T2 is empty; T1 and T0 have content
			{
				bool bUBoth = bU1 & bU0;									// are both already known to be unimportant
				
				if (   !bUBoth										// if we still think one or both are important
					&& !pSync->isSameType(DE_ATTR_MRG_T1T0EQ)		// and T1,T0 are not equal
					&& s_ign_case(attrT1,strLineT1,offsetT1,lenT1,	// and they are equal when ignoring case (if we can do that),
								  attrT0,strLineT0,offsetT0,lenT0))
					pSync->setUnimportant(true);					// then the change is really unimportant.
				else												// otherwise, we either don't need to check 
					pSync->setUnimportant(bUBoth);					// case or we already know the answer.
			}
			else															// length of all 3 are > 0
			{
				bool bUAll = bU0 & bU1 & bU2;
				
				if (bUAll)																// if all are unimportant,
					pSync->setUnimportant(true);										//   we already know the answer.
				else if (pSync->isSameType(DE_ATTR_MRG_3EQ))							// if all equal,
					pSync->setUnimportant(bUAll);										//   we already know the answer (no ignore-case needed).
				else if (pSync->isSameType(DE_ATTR_MRG_T0T2EQ))							// if T0 & T2 equal,
					pSync->setUnimportant(s_ign_case(attrT0,strLineT0,offsetT0,lenT0,	//   see if T1 only differs in case.
													 attrT1,strLineT1,offsetT1,lenT1));
				else if (pSync->isSameType(DE_ATTR_MRG_T1T0EQ))							// if T1 & T0 equal,
					pSync->setUnimportant(s_ign_case(attrT1,strLineT1,offsetT1,lenT1,	//   see if T2 only differs in case.
													 attrT2,strLineT2,offsetT2,lenT2));
				else if (pSync->isSameType(DE_ATTR_MRG_T1T2EQ))							// if T1 & T2 equal,
					pSync->setUnimportant(s_ign_case(attrT1,strLineT1,offsetT1,lenT1,	//   see if T0 only differs in case.
													 attrT0,strLineT0,offsetT0,lenT0));
				else // if DE_ATTR_MRG_0EQ
					pSync->setUnimportant( (   s_ign_case(attrT1,strLineT1,offsetT1,lenT1,	//   see if T0 only differs from T1 in case.
														  attrT0,strLineT0,offsetT0,lenT0)
											&& s_ign_case(attrT1,strLineT1,offsetT1,lenT1,	//   see if T2 only differs from T1 in case.
														  attrT2,strLineT2,offsetT2,lenT2)));
			}
		}

		// count number of changes

		if (!pSync->isSameType(DE_ATTR_MRG_3EQ))
		{
			m_ChangeKindIntraLine &= pSync->getAttr();		// [1] turn off the bit on the non-equal side
			m_cChangesIntraLine++;
			if (!pSync->isUnimportant())
				m_cImportantChangesIntraLine++;
		}
	}

	if (m_pHead && m_pHead->isEND())
	{
		// if the intra-line sync-list is just an END marker, then the above
		// loop did not get entered.  this happens for blank lines (when the
		// EOL chars are not included in the line).

		wxASSERT_MSG( (!DE_ATTR__IS_TYPE(initialChangeKind,DE_ATTR_MRG_3EQ)), _T("Coding Error") );
		if (DE_ATTR__IS_TYPE(initialChangeKind,DE_ATTR_MRG_3EQ))
		{
			wxASSERT_MSG( ((pDeLineLastT1!=NULL) && (pDeLineLastT0!=NULL) && (pDeLineLastT2!=NULL)), _T("Coding Error"));
			// this line is not a change -- we have a blank line opposite a blank line.
		}
		else /* _0EQ or one of the _TxTyEQ */
		{
			wxASSERT_MSG( ((pDeLineLastT1==NULL) || (pDeLineLastT0==NULL) || (pDeLineLastT2==NULL)), _T("Coding Error!") );
			// we have either 1 blank line opposite 2 voids or 2 blank lines opposite 1 void.

			m_cChangesIntraLine++;

			// the importance of the line is determined by the context of the EOL.

			bool bImportant = false;
			if (pDeLineLastT1)
			{
				rs_context_attrs rsAttrs = ( (pDeLineLastT1->getContextEOL())
											 ? pDeLineLastT1->getContextEOL()->getContextAttrs()
											 : pRS->getDefaultContextAttrs());
				bImportant |= (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs));
			}
			if (!bImportant && pDeLineLastT0)
			{
				rs_context_attrs rsAttrs = ( (pDeLineLastT0->getContextEOL())
											 ? pDeLineLastT0->getContextEOL()->getContextAttrs()
											 : pRS->getDefaultContextAttrs());
				bImportant |= (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs));
			}
			if (!bImportant && pDeLineLastT2)
			{
				rs_context_attrs rsAttrs = ( (pDeLineLastT2->getContextEOL())
											 ? pDeLineLastT2->getContextEOL()->getContextAttrs()
											 : pRS->getDefaultContextAttrs());
				bImportant |= (!RS_ATTRS_IsUnimportant(rsAttrs) && RS_ATTRS_RespectBlankLines(rsAttrs));
			}
			if (bImportant)
				m_cImportantChangesIntraLine++;
		}
	}

	normalize_ChangeKindIntraLine3();
}

//////////////////////////////////////////////////////////////////

void de_sync_list::_divide_node3_21(de_sync * pSync, PanelIndex xPanel, PanelIndex yPanel, PanelIndex zPanel){
	//////////////////////////////////////////////////////////////////
	// THIS FUNCTION IS ONLY USED WHEN WE REFER TO AN INTRA-LINE SYNC-LIST.
	//////////////////////////////////////////////////////////////////
	//
	// convert a 's/a/b/' style change into 's/a//' and 's//b/'
	//
	// put panel x,y in one node and panel z in another

	wxASSERT_MSG( (!pSync->isSameType(DE_ATTR_MRG_3EQ)), _T("Coding Error!") );

	de_sync * pNew = new de_sync();

	pNew->m_attr = pSync->m_attr;

	wxASSERT_MSG( (!pSync->m_pVecSyncList), _T("Coding Error!") );	// don't use on line-oriented node

	for (long kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
	{
		if ((kPanel == xPanel) || (kPanel == yPanel))
		{
			pNew->m_ndx[kPanel] = pSync->m_ndx[kPanel] + pSync->m_len[kPanel];
			pNew->m_len[kPanel] = 0;
		}
		else if (kPanel == zPanel)
		{
			pNew->m_ndx[kPanel]  = pSync->m_ndx[kPanel];
			pNew->m_len[kPanel]  = pSync->m_len[kPanel];
			pSync->m_len[kPanel] = 0;
		}
		else
		{
			wxASSERT_MSG( (pSync->m_ndx[kPanel] == -1), _T("Coding Error!") );

			pNew->m_ndx[kPanel] = -1;
			pNew->m_len[kPanel] = -1;
		}

		pNew->m_contextAttrsSpan[kPanel] = pSync->m_contextAttrsSpan[kPanel];
	}

	wxASSERT_MSG( ((pSync->m_len[0] > 0) || (pSync->m_len[1] > 0) || (pSync->m_len[2] > 0)), _T("Coding Error") );
	wxASSERT_MSG( (( pNew->m_len[0] > 0) || ( pNew->m_len[1] > 0) || ( pNew->m_len[2] > 0)), _T("Coding Error") );
	
	_insert_after(pSync,pNew);
}

//////////////////////////////////////////////////////////////////

void de_sync_list::applySmoothing2(int threshold)
{
	//UTIL_PERF_START_CLOCK(_T("de_sync_list::applySmoothing2"));

	// there are times when having the EXACT diff generates
	// to much visual noise -- consider 2 strings:
	// "hello abcdef world" vs "hello a1c2e3 world"
	// this will report alternating EQ and NEQ in the middle
	// of the line.  and when we display it, every other
	// character will be in reverse video.  the fact that
	// "c" is in both strings is kind of irrelevant when the
	// whole word was changed.  this case happens often when
	// we are comparing two things that are truly different
	// and just happen to be in the same spot in the files.
	//
	// so, we define a smoothing-threshold, n, as: when there
	// is a length n or less EQ between 2 NEQs, silently
	// change it to an NEQ.
	//
	// in the example above, the user should see this as a
	// single 's/abcdef/a1c2e3/' change.
	//
	// NOTE: this works on both line-oriented and intra-line-oriented
	// NOTE: sync lists.
	// NOTE:
	// NOTE: it's really important (for visual sanity) for intra-line
	// NOTE: lists.
	// NOTE:
	// NOTE: for line-oriented, it can be used as a safety feature --
	// NOTE: to glob adjacent changes (separated by n or less lines)
	// NOTE: into a single change.  [think about a blank line between
	// NOTE: two adjacent, modified paragraphs -- where for convenience
	// NOTE: we may want to just show a single 2-paragraph change.]
	// NOTE:
	// NOTE: we assume that importance and span-context have not been
	// NOTE: set yet on the list -- that is, that the list is still in
	// NOTE: alternating EQ/NEQ order and no node dividing has happened
	// NOTE: yet -- that is, we haven't cut up nodes for whitespace.
	// NOTE:
	// NOTE: also, we assume that the multi-line-intra-line to
	// NOTE: multiple single-line-intra-line extraction has not been done;
	// NOTE: that is, that there are no partial nodes yet.

	for (de_sync * pSync=m_pHead; (pSync && !pSync->isEND()); pSync=pSync->getNext())
	{
		if (pSync->isSameType(DE_ATTR_DIF_2EQ) && pSync->getLen(PANEL_T1) <= threshold)
		{
			if (pSync->getPrev() && pSync->getPrev()->isSameType(DE_ATTR_DIF_0EQ)
				&& pSync->getNext() && pSync->getNext()->isSameType(DE_ATTR_DIF_0EQ))
			{
				//pSync->dump(10);
				pSync->setSmoothed(DE_ATTR_DIF_0EQ);
			}
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_sync_list::applySmoothing2"));
}

void de_sync_list::applySmoothing3(int threshold)
{
#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("applySmoothing3: [%d]"),threshold);
	dump(10);
#endif
#endif

	//UTIL_PERF_START_CLOCK(_T("de_sync_list::applySmoothing2"));

	// do the smoothing for a 3-way merge.  this is slightly more
	// complicated than the 2-way version because we have the partial-EQs
	// to deal with.
	//
	// NOTE: this works on both line-oriented and intra-line-oriented
	// NOTE: sync lists.  see the long NOTE: in applySmoothing2().

	for (de_sync * pSync=m_pHead; (pSync && !pSync->isEND()); pSync=pSync->getNext())
	{
		// if:  we are a 3EQ node
		// and: our length is under the threshold
		// and: we have a previous MERGE node
		// and: we have a next MERGE node
		// and: previous node is some kind of change
		// and: next node is some kind of change -- so we are between 2 changes
		// then we want to smooth this node.
		//
		// BTW, if we are a paritial-EQ node, we don't need to do anything here because
		// we do look like a change and combine_T1T0T2_conflicts() will make sure
		// that we get marked appropriately.

		if (   pSync->isSameType(DE_ATTR_MRG_3EQ)
			&& (pSync->getLen(PANEL_T1) <= threshold))
		{
			de_sync * pSyncPrev = pSync->getPrev();
			de_sync * pSyncNext = pSync->getNext();

			if (pSyncPrev && pSyncNext)
			{
//				de_attr attrPrev = pSyncPrev->getAttr() & DE_ATTR__TYPE_MASK;
//				de_attr attrNext = pSyncNext->getAttr() & DE_ATTR__TYPE_MASK;
				de_attr attrPrev = pSyncPrev->getAttr();
				de_attr attrNext = pSyncNext->getAttr();

				if (   DE_ATTR__IS_KIND(attrPrev,DE_ATTR__MRG_KIND)
					&& DE_ATTR__IS_KIND(attrNext,DE_ATTR__MRG_KIND)
					&& !DE_ATTR__IS_TYPE(attrPrev,DE_ATTR_MRG_3EQ)
					&& !DE_ATTR__IS_TYPE(attrNext,DE_ATTR_MRG_3EQ))
				{
					// we want to smooth this node.
#if 0
					// 
					// set the attr-type to match the attr-type of the surrounding nodes.
					// that is, we don't want to create a conflict if we don't have to.
					// for example, if both are T1T0EQ, we should be too.  but if both
					// are different partial-EQs, we have need to be 0EQ to force a conflict.

					if (DE_ATTR__IS_TYPE(attrPrev,DE_ATTR_MRG_0EQ) && DE_ATTR__IS_TYPE(attrNext,DE_ATTR_MRG_0EQ))
					{
						// easy case -- stuck between 2 conflicts -- conflict
						pSync->setSmoothed(DE_ATTR_MRG_0EQ);
					}
					else if (attrPrev != attrNext)
					{
						// easy case -- stuck between incompatible types -- conflict
						pSync->setSmoothed(DE_ATTR_MRG_0EQ);
					}
					else
					{
						// both surrounding changes are of the same type, but we also need
						// to check to see that they are both inserts or deletes.  that is,
						// "x . x" and ". x ." are both T0T2EQs and should be treated differently
						// than "x . x" and "y . y".

						switch (attrPrev)
						{
						default:
							wxASSERT_MSG( (0), _T("Coding Error") );
							break;

						case DE_ATTR_MRG_T0T2EQ:
							if ((pSyncPrev->getLen(PANEL_T0) != 0) == (pSyncNext->getLen(PANEL_T0) != 0))
								pSync->setSmoothed(attrPrev);			// ("x . x" and "y . y") or (". x ." and ". y .")
							else
								pSync->setSmoothed(DE_ATTR_MRG_0EQ);	// ("x . x" and ". y .") or (". x ." and "y . y")
							break;

						case DE_ATTR_MRG_T1T0EQ:
						case DE_ATTR_MRG_T1T2EQ:
							if ((pSyncPrev->getLen(PANEL_T1) != 0) == (pSyncNext->getLen(PANEL_T1) != 0))
								pSync->setSmoothed(attrPrev);
							else
								pSync->setSmoothed(DE_ATTR_MRG_0EQ);
							break;
						}
					}
#else
					// the above just didn't *feel* right. seeing a 3-way-equal line marked red/yellow
					// just because it was between 2 different types of 2-way-equal lines just looked weird.
					// ideally, i'd like to mark it as a 3-way-equal-change -- so it'd appear in pale blue
					// in all three panels (like partial-eq nodes do) -- but we can't do that.
					//
					// as a hack, we just set it to the previous so that it looks like an extension of it.

					pSync->setSmoothed(attrPrev);
#endif
				}
			}
		}
	}

	//UTIL_PERF_STOP_CLOCK(_T("de_sync_list::applySmoothing2"));

#if 0
#ifdef DEBUG
	wxLogTrace(TRACE_DE_DUMP,_T("applySmoothing3: afterward:"),threshold);
	dump(10);
#endif
#endif
}

//////////////////////////////////////////////////////////////////

void de_sync_list::appendNextToCurrent(de_sync * pSync)
{
	// append contents of pSync->getNext() into pSync and remove next from sync-list.

	de_sync * pSyncNext = pSync->getNext();
	wxASSERT_MSG( (pSync && pSyncNext && !pSyncNext->isEND()), _T("Coding Error") );

	// for now assume that the intra-line analysis has not been done yet.
	wxASSERT_MSG( (!pSync->m_pVecSyncList && !pSyncNext->m_pVecSyncList), _T("Coding Error") );

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		pSync->m_len[kPanel] += pSyncNext->m_len[kPanel];

	pSync->unionAttr(pSyncNext->getAttr());

	_delete_sync(pSyncNext);
}

//////////////////////////////////////////////////////////////////

void de_sync_list::split_up_partial_eqs_with_zeros(void)
{
	// split partial-EQ nodes into individual lines when they have
	// either zero length on matching pair or on the odd side.
	// that is, if we have T1T0EQ, split it if lenT1==lenT0==0 or
	// when lenT1==lenT0 > 0 and lenT2==0.
	//
	// NOTE: this is only well defined on 3-way-merge lists.
	// NOTE: (since 2-way-diff lists don't have partial-EQs.)

	de_sync * pSync = m_pHead;
	while (pSync && !pSync->isEND())
	{
		// since we split the list in place, get the current next.
		// it will be immediately after any change we make during
		// this iteration.

		de_sync * pSyncNext = pSync->getNext();

		switch (pSync->getAttr() & DE_ATTR__TYPE_MASK)
		{
		default:
		//case DE_ATTR_UNKNOWN:
		//case DE_ATTR_EOF:
		//case DE_ATTR_OMITTED:
		//case DE_ATTR_MARK:
		//case DE_ATTR_DIF_0EQ:
		//case DE_ATTR_DIF_2EQ:
		//case DE_ATTR_MRG_0EQ:
		//case DE_ATTR_3EQ:
			break;

		case DE_ATTR_MRG_T0T2EQ:
		case DE_ATTR_MRG_T1T0EQ:
		case DE_ATTR_MRG_T1T2EQ:
			if ((pSync->getLen(PANEL_T0) == 0) || (pSync->getLen(PANEL_T1) == 0) || (pSync->getLen(PANEL_T2)))
			{
				long len = MyMax(pSync->getLen(PANEL_T0), MyMax(pSync->getLen(PANEL_T1), pSync->getLen(PANEL_T2)));
				while (len > 1)
				{
					split(pSync,1);
					pSync = pSync->getNext();
					len--;
				}
			}
			break;
		}

		pSync = pSyncNext;
	}
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void de_sync_list::dump(int indent) const
{
	wxLogTrace(TRACE_DE_DUMP, _T("%*c=============================================================================="), indent,' ');
	wxLogTrace(TRACE_DE_DUMP, _T("%*cDE_SYNC_LIST: [%p][isLine %d]"), indent,' ', this,m_bIsLine);
	if (!m_bIsLine)
		wxLogTrace(TRACE_DE_DUMP, _T("%*c[ChangeKindIntraLine %x][cChanges %ld][cImpChanges %ld]"),indent+10,' ',
				   m_ChangeKindIntraLine,m_cChangesIntraLine,m_cImportantChangesIntraLine);
	for (de_sync * p=m_pHead; (p); p=p->getNext())
		p->dump(indent+5);
	wxLogTrace(TRACE_DE_DUMP, _T("%*c=============================================================================="), indent,' ');
}
#endif//DEBUG

//////////////////////////////////////////////////////////////////
// TODO
// TODO this test code is very old -- it may or may not be appropriate
// TODO

#if 0
#ifdef DEBUG
void de_sync_list::test_verify_load_xy(wxChar * szLabel,
									   const de_css_src_lines & rSrcXY,
									   PanelIndex x, PanelIndex y,
									   de_attr attrEq) const
{
	long nrErrors = 0;

#define TEST(e,ln)	Statement( if (!(e)) { nrErrors++; wxASSERT_MSG( (0), wxString::Format(_T("de_sync_list::%s:[%d] [%s]"), szLabel, (ln), #e) ); } )

	wxLogTrace(TRACE_DE_DUMP, _T("%s:test_verify_load:[%d][%d]"),szLabel,x,y);

	long endX = 0;
	long endY = 0;

	for (const de_sync * pSync=m_pHead; (pSync); pSync=pSync->getNext())
	{
		if (pSync->isSameType(DE_ATTR_OMITTED))	continue;

		long ndxX = pSync->m_ndx[x];
		long ndxY = pSync->m_ndx[y];

		long gapX = ndxX - endX;
		long gapY = ndxY - endY;

		TEST( (gapX==0), __LINE__);
		TEST( (gapY==0), __LINE__);

		long lenX = pSync->m_len[x];
		long lenY = pSync->m_len[y];
		
		if (pSync->isEND())
		{
			TEST( (lenX==0), __LINE__);
			TEST( (lenY==0), __LINE__);
		}
		else if (pSync->isSameType(attrEq) || pSync->isSameType(DE_ATTR_MRG_3EQ))
		{
			TEST( (lenX==lenY), __LINE__);

			for (long k=0; k<lenX; k++)
			{
				TEST( (rSrcXY.equal(ndxX+k,ndxY+k)), __LINE__);
			}
		}
		else	// X not equal to Y
		{
			if ((lenX > 0) && (lenY > 0))
			{
				for (long kX=0; kX<lenX; kX++)
					for (long kY=0; kY<lenY; kY++)
						TEST( (!rSrcXY.equal(ndxX+kX,ndxY+kY)), __LINE__);
			}
		}

		endX += pSync->m_len[x];
		endY += pSync->m_len[y];
	}

	TEST( (m_pTail->isEND()), __LINE__);

	wxLogTrace(TRACE_DE_DUMP, _T("%s:test_verify_load:[%d][%d] %s"),szLabel,x,y, ((nrErrors) ? _T("FAILED") : _T("PASSED")) );

#undef TEST
}
void de_sync_list::test_verify_load_1x(PanelIndex kPanel, const de_css_src_lines & rSrc1X) const
{
	test_verify_load_xy(_T("Test1x"), rSrc1X,PANEL_T1,kPanel,DE_ATTR_DIF_2EQ);
}

void de_sync_list::test_verify_load_102(const de_css_src_lines & rSrc10,
										const de_css_src_lines & rSrc12,
										const de_css_src_lines & rSrc02) const
{
	test_verify_load_xy(_T("Test10"), rSrc10,PANEL_T1,PANEL_T0,DE_ATTR_MRG_T1T0EQ);
	test_verify_load_xy(_T("Test12"), rSrc12,PANEL_T1,PANEL_T2,DE_ATTR_MRG_T1T2EQ);
	test_verify_load_xy(_T("Test02"), rSrc02,PANEL_T0,PANEL_T2,DE_ATTR_MRG_T0T2EQ);
}
#endif//DEBUG
#endif
