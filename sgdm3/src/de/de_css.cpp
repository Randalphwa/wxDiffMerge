// de_css.cpp -- common subsequence for diff engine.
//
// the diff-engine runs an algorithm described in
// "An O(NP) Sequence Comparison Algorithm" by Wu, Manber, Myers,
// and Miller.  [I've added a PDF copy of it to the source tree in
// this directory.]
//
// this implementation is based upon my reading of the paper and
// my reading of the Subversion source (which is also based on this
// paper).  [I didn't use any SVN code, but I'm tainted anyway--not
// that it matters.]
//
// since the paper is a little "thick", i'll translate a little.
// 
// [] the paper is concerned with FINDING THE LENGTH OF A SHORTEST
//    EDIT DISTANCE between 2 sequences of tokens (documents).  by
//    EDIT, they mean the sequence of insert or delete operations
//    to convert the first document into the second.
//
// [] they only describe the data structures and bookkeeping
//    required to compute the length of the path -- the number of
//    operations required.
//
// [] they don't care about recording or reporting the actual path
//    just computing its length -- so, it's up to us to hook into
//    the algorithm and capture it.
//
// [] there may be more than one shortest path -- no thought is
//    given to path quality.
//
// A ds_css object will record a match -- a common subsequence -- in
// the 2 source token streams.  basically, an offset/index into each
// document and a length.  in terms of the paper, these will represent
// each diagonal "slide" in the final path.  we chain these together
// in a doubly-linked list to represent the complete diff [or rather
// the common portions -- the diff is inferred from the gaps between
// them (as well as the end-points)].  GAPS are where the offset of
// the next de_css_item doesn't match the offset+length of the previous
// de_css_item.
//
// The algorithm works by trying to walk diagonally from the beginning
// (0,0) to the end (M,N) and transform document A into B.  it steps:
//        down (j,k)-->(j+1,k)   for a delete,
//       right (j,k)-->(j,k+1)   for an insert, and
//    diagonal (j,k)-->(j+1,k+1) for a common token.
// as it walks along and computes partial solutions, it will identify
// multiple diagonals.  as each diagonal is identified, we *DO* know
// the path back to the beginning. *BUT* we won't know until the
// algorithm finishes whether a particular diagonal is in the final
// path *AND* we don't know next diagonal in the series yet.  SO,
// as the algorithm runs, we let it create a tree of de_css_item's as
// necessary with parent (back) links.  when it is finished we keep
// the final solution and convert the path thru the tree into a
// doubly-linked list and delete the unnecessary nodes.
//
// de_css_list is our dual table/list container that takes care of
// memory for us and runs the diff algorithm.
//
// one restiction of the algorithm is that Len(A) <= Len(B).  so we
// have to do a little extra subscript magic or require our caller
// to order the documents correctly.  WE HIDE THIS DETAIL IN HERE,
// SO THAT THE CALLER CAN SUBMIT THE DOCUMENTS IN ANY ORDER -- (this
// should help reduce confusion in caller during 3-way merges...)
//
// I'm deviating from the paper and changing 'x' to 'a' and 'y' to 'b'.
// These refer to indexes into A[] and B[].  This way caller doesn't
// get confused with our indexes and pixel locations...
//
// after the diff-engine runs, we have a list of common sub-sequences
// (that is, a list of the index/length of places where the two
// documents are equal).  after the algorithm runs, we insert NON-EQUAL
// items to the list to represent gaps -- places where the docs are
// not equal -- for inserts/deletes/changes.
//
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

#ifdef DEBUGUTILPERF
static const wxChar * sszKey_runAlgorithm	= L"de_css_list::runAlgorithm";
static const wxChar * sszKey_compare		= L"de_css_list::_compare";
//static const wxChar * sszKey_snakeEqualLoop	= L"de_css_list::_snake:equalLoop";
static const wxChar * sszKey_deleteTable	= L"de_css_list::_deleteTable";
static const wxChar * sszKey_deleteList		= L"de_css_list::_deleteList";
static const wxChar * sszKey_fillin_gaps	= L"de_css_list::_fillin_gaps";
#endif

//////////////////////////////////////////////////////////////////

bool de_css_src::identical(void) const
{
	if (getLenA() != getLenB())
		return false;
	
	long len = getLenA();

	for (long k=0; k<len; k++)
		if (equal(k,k) == false)
			return false;

	return true;
}

//////////////////////////////////////////////////////////////////

de_css_vars::de_css_vars(void)
	: m_a(0), m_b(0), m_len_a(0), m_len_b(0),
	  m_bEq(false)
{
}

de_css_vars::de_css_vars(long a, long b, long len)
	: m_a(a), m_b(b), m_len_a(len), m_len_b(len),
	  m_bEq(true)
{
	// create an EQUAL CSS VARS
}

de_css_vars::de_css_vars(long a, long len_a,
						 long b, long len_b)
	: m_a(a), m_b(b), m_len_a(len_a), m_len_b(len_b),
	  m_bEq(false)
{
	// create a NON-EQUAL CSS VARS (for inserts/deletes/changes)
}

void de_css_vars::set(long a, long b, long len)
{
	m_a     = a;
	m_b     = b;
	m_len_a = len;
	m_len_b = len;
	m_bEq   = true;
}

void de_css_vars::set(long a, long len_a,
					  long b, long len_b)
{
	m_a     = a;
	m_b     = b;
	m_len_a = len_a;
	m_len_b = len_b;
	m_bEq   = false;
}

de_css_vars::de_css_vars(const de_css_vars * pcss)
{
	setFrom(pcss);
}

void de_css_vars::setFrom(const de_css_vars * pcss)
{
	m_a     = pcss->m_a;
	m_b     = pcss->m_b;
	m_len_a = pcss->m_len_a;
	m_len_b = pcss->m_len_b;
	m_bEq   = pcss->m_bEq;
}

void de_css_vars::shift(long len)
{
	m_a += len;		m_len_a -= len;
	m_b += len;		m_len_b -= len;
}

void de_css_vars::shift(long len_a, long len_b)
{
	m_a += len_a;	m_len_a -= len_a;
	m_b += len_b;	m_len_b -= len_b;
}

void de_css_vars::rel2abs(long ndx_a, long ndx_b)
{
	m_a += ndx_a;
	m_b += ndx_b;
}

//////////////////////////////////////////////////////////////////

de_css_item::de_css_item(long a, long b, long len,
						 de_css_item * pPrev)
	: de_css_vars(a,b,len),
	  m_next(NULL), m_prev(pPrev)
{
	// create an EQUAL CSS ITEM
}

de_css_item::de_css_item(long a, long len_a,
						 long b, long len_b)
	: de_css_vars(a,len_a,b,len_b),
	  m_next(NULL), m_prev(NULL)
{
	// create a NON-EQUAL CSS (for inserts/deletes/changes)
	// caller needs to finish linking into any list.
}

//////////////////////////////////////////////////////////////////

de_css_list::de_css_list(de_css_src * pSrc)
{
	// we have 2 states:
	// [] m_bValid==false -- everything is zeroed
	// [] m_bValid==true  -- algorithm has been run
	// 						(list and stats set)
	//						(everything else zeroed)
	//						(we do not keep reference to pSrc)

	_zero();

	if (pSrc)
		runAlgorithm(pSrc);
}

de_css_list::~de_css_list(void)
{
	_deleteList();
}

//////////////////////////////////////////////////////////////////

void de_css_list::setInvalid(void)
{
	_deleteList();
	m_bValid = false;
}

void de_css_list::_zero(void)
{
	m_pSrc        = NULL;
	m_pArrayFPy   = NULL;	m_pFPy      = NULL;
	m_pArrayFPcss = NULL;	m_pFPcss    = NULL;
	m_pHead       = NULL;	m_pTail     = NULL;
	m_bSwapped    = false;
	m_bValid      = false;
	m_M           = 0;		m_N         = 0;
	m_nrDeletes   = 0;		m_nrInserts = 0;
	// this is unnecessary: m_vec.clear();
}

void de_css_list::runAlgorithm(de_css_src * pSrc)
{
	// run the diff algorithm on this src.  if we already
	// have a list, delete it (in case we are being reused).

	UTIL_PERF_START_CLOCK(sszKey_runAlgorithm);

	_deleteList();
	m_bValid = false;
	
	long m = pSrc->getLenA();
	long n = pSrc->getLenB();

	m_bSwapped = (m > n);
	m_M = (m_bSwapped) ? n : m;
	m_N = (m_bSwapped) ? m : n;

#ifdef DEBUGUTILPERF
	wxLogTrace(TRACE_DE_DUMP,_T("de_css_list::runAlgorithm: [m_M %ld][m_N %ld][bSwap %d]"),m_M,m_N,m_bSwapped);
#endif

	_init_fp();

	m_pSrc = pSrc;
	_compare();

	_convertToList();
	_deleteTable();
	_delete_fp();

	m_bSwapped = false;
	m_M = m;
	m_N = n;

	_juggle();
	_fillin_gaps();

	m_pSrc = NULL;
	m_bValid = true;

#ifdef DEBUGUTILPERF
	wxLogTrace(TRACE_DE_DUMP,_T("de_css_list::runAlgorithm: [nrDels %ld][nrIns %ld]"),m_nrDeletes,m_nrInserts);
#endif

	UTIL_PERF_STOP_CLOCK(sszKey_runAlgorithm);

#ifdef DEBUG
//	dump(0);
#endif//DEBUG
}

//////////////////////////////////////////////////////////////////

void de_css_list::_compare(void)
{
	UTIL_PERF_START_CLOCK(sszKey_compare);

	long k;
	long delta = m_N - m_M;			// extra width when non-square array
	long p = -1;					// number of deletions

	do
	{
		p++;

		for (k = -p; k <= delta-1; k++)
			_snake(k);

		for (k = delta+p; k >= delta+1; k--)
			_snake(k);

		_snake(delta);

	} while (m_pFPy[delta] != m_N);

	long D = delta + 2*p;			// total edit distance [ 2p=="distance(h + v) from main diagonal", delta=="extra non-square width"]
	long i = D - p;					// number of inserts   [ i = delta + p ]
	
	m_nrDeletes = (m_bSwapped) ? i : p;
	m_nrInserts = (m_bSwapped) ? p : i;

	UTIL_PERF_STOP_CLOCK(sszKey_compare);
}

//////////////////////////////////////////////////////////////////

void de_css_list::_snake(long k)
{
	de_css_item * pSub;
	long y0;

	// pick best starting Y value [index in B] for this iteration (see paper).

	if (m_pFPy[k-1]+1 > m_pFPy[k+1])
	{
		y0   =   m_pFPy[k-1] + 1;
		pSub = m_pFPcss[k-1];
	}
	else
	{
		y0   =   m_pFPy[k+1];
		pSub = m_pFPcss[k+1];
	}

	long x0 = y0 - k;

	// try to walk the diagonal

	//UTIL_PERF_START_CLOCK(sszKey_snakeEqualLoop);
	
	long y = y0;
	long x = x0;
	if (m_bSwapped)
		while ( (x<m_M) && (y<m_N) && (m_pSrc->equal(y,x)) ) { x++; y++; }
	else
		while ( (x<m_M) && (y<m_N) && (m_pSrc->equal(x,y)) ) { x++; y++; }

	//UTIL_PERF_STOP_CLOCK(sszKey_snakeEqualLoop);

	if (y != y0)
	{
		// we were able to walk the the diagonal (at least once)
		// so we create a new subsequence and set it's back ptr
		// to the last subsequence in the path that got us here.
		// update fp to refer to the end of our diagonal.

		long len = y - y0;
		pSub = (m_bSwapped) ? _addCSS(y0,x0,len,pSub) : _addCSS(x0,y0,len,pSub);
	}
	else
	{
		// no diagonal to walk (A & B are different here), so
		// we implictily have a single token insert or delete.
		// update fp for this diagonal with our starting conditions
		// (think of this as a carry forward (a horizontal or
		// vertical step in the grid)).
	}

	m_pFPy  [k] = y;
	m_pFPcss[k] = pSub;
}

//////////////////////////////////////////////////////////////////

de_css_item * de_css_list::_addCSS(long x0, long y0, long len, de_css_item * pPrev)
{
	// as new common subsequences are identified we add them to our table.
	// we keep them all in a table so we can delete them and not leak.
	// 
	// we know the parital-shortest-path that lead us to find this css, but
	// we don't know if this css will be in the final solution and we don't
	// know the rest of the path yet.  SO, we only record the prev ptr.
	//
	// think of this as building a tree (breath-first) with only parent
	// pointers (so far) with all nodes also appended to a vector as they
	// are created.

	de_css_item * pNew = new de_css_item(x0,y0,len,pPrev);

	m_vec.push_back(pNew);

	return pNew;
}

void de_css_list::_convertToList(void)
{
	// we've found A SHORTEST PATH and the last common subseqence is given
	// in m_pFPcss[delta] (the actual path may be a little longer if it
	// doesn't end with a common subsequence).

	// append a special css of length zero and referring to the end of both
	// docs at the end of our path to help callers deal with any odd remainders.

	long delta = m_N - m_M;
	m_pFPcss[delta] = (m_bSwapped) ? _addCSS(m_N,m_M,0,m_pFPcss[delta]) : _addCSS(m_M,m_N,0,m_pFPcss[delta]);
	
	// we need to convert the tree/vector to a regular doubly-linked list.
	// we do this by walking up the tree (via PARENT (aka PREV) pointers)
	// from this leaf and setting the NEXT pointers.

	m_pTail = m_pFPcss[delta];
	for (m_pHead=m_pTail; (m_pHead->m_prev); m_pHead=m_pHead->m_prev)
		m_pHead->m_prev->m_next = m_pHead;
}

void de_css_list::_init_fp(void)
{
	long size = m_M + m_N + 3;

	m_pArrayFPy   =    (long *)malloc(size * sizeof(long) );
	m_pArrayFPcss = (de_css_item **)calloc(size, sizeof(de_css_item *));
	
	for (long k=0; (k < size); k++)
		m_pArrayFPy[k] = -1;

	// convert to arrays where we can use negative subscripts

	m_pFPy   = &m_pArrayFPy[m_M+1];
	m_pFPcss = &m_pArrayFPcss[m_M+1];
}

void de_css_list::_delete_fp(void)
{
	FREEP(m_pArrayFPy);		m_pFPy   = NULL;
	FREEP(m_pArrayFPcss);	m_pFPcss = NULL;
}

void de_css_list::_deleteTable(void)
{
	UTIL_PERF_START_CLOCK(sszKey_deleteTable);

	// delete all the tree nodes that we no longer need and clear the
	// vector.  only css's in the final solution will have NEXT pointers
	// (except for the tail) .

	for (TVecIterator it=m_vec.begin(); (it < m_vec.end()); it++)
	{
		de_css_item * p = (*it);
		if ((p->m_next==NULL) && (p != m_pTail))
			delete p;
	}

#ifdef DEBUGUTILPERF
	wxLogTrace(TRACE_DE_DUMP,_T("de_css_list::_deleteTable: [size %ld]"),(long)m_vec.size());
#endif

	m_vec.clear();

	UTIL_PERF_STOP_CLOCK(sszKey_deleteTable);
}

void de_css_list::_deleteList(void)
{
	UTIL_PERF_START_CLOCK(sszKey_deleteList);

#ifdef DEBUGUTILPERF
	long sum = 0;
#endif

	while (m_pHead)
	{
		de_css_item * p = m_pHead->m_next;
		delete m_pHead;
		m_pHead = p;

#ifdef DEBUGUTILPERF
		sum++;
#endif
	}
	m_pTail = NULL;

#ifdef DEBUGUTILPERF
	wxLogTrace(TRACE_DE_DUMP,_T("de_css_list::_deleteList: [size %ld]"),sum);
#endif

	UTIL_PERF_STOP_CLOCK(sszKey_deleteList);
}

//////////////////////////////////////////////////////////////////

void de_css_list::_delete_item(de_css_item * pItem)
{
	// unlink/delete a single item from the list.
	// CAN ONLY BE CALLED AFTER LCS ALGORITHM HAS FINISHED AND LIST IS BUILT.

	if (!pItem)
		return;
	
	de_css_item * pPrev = pItem->m_prev;
	de_css_item * pNext = pItem->m_next;

	if (pNext)
		pNext->m_prev = pPrev;
	else
		m_pTail = pPrev;

	if (pPrev)
		pPrev->m_next = pNext;
	else
		m_pHead = pNext;

	delete pItem;
}

//////////////////////////////////////////////////////////////////

void de_css_list::_juggle(void)
{
	// juggle the css list to eliminate minor items.
	// 
	// CAN ONLY BE CALLED AFTER LCS ALGORITHM HAS FINISHED AND LIST IS BUILT.
	// MUST BE CALLED BEFORE GAPS ARE FILLED IN.
	//
	// the LCS algorithm is greedy in that it will accept a one line
	// match as a "common" sequence and then create another item for
	// the next portion of a large change.  [a line containing only a
	// '{' or '}', for example, frequently triggers this in C source
	// code.]
	//
	// for simple (insert-/delete-, not change-type) gaps, if the content
	// at the end of a gap (between two "common" portions, C1 & C2) ends
	// with the same content that is in the first "common" portion (C1),
	// then it is arbitrary which copy of the identical content we use.
	//
	// for example:
	//
	// input       algorithm        juggle
	// ======     ---: ======    ---: ======
	// xx  xx      eq: xx  xx     eq: xx  xx
	// aa  bb     chg: aa  bb    chg: aa  bb
	// yy  yy      eq: yy  yy       : yy  ..
	// cc  zz     ins: cc  ..       : cc  ..
	// yy            : yy  ..     eq: yy  yy
	// zz          eq: zz  zz       : zz  zz
	//
	// given the "input", the LCS/CSS algorithm will generate the list
	// shown in "algorithm".  the juggle calculation will convert this
	// to the list shown in "juggle".  the yy's in the left column (in
	// the 2nd "eq:" line and at the end of the "ins:" gap) are the
	// pivot point.  here we convert a "change, equal, insert" sequence
	// for a "change, equal" sequence.  this tends to bring local changes
	// into clusters.
	// 
	// walk the entire CSS LIST and try to juggle.  this will cause
	// deltas to bubble up and short/trivial common sequences to bubble
	// down (and get coalesced).  since this bubbling may cause additional
	// things to match, we repeat this walk until we reach closure.  [we
	// expect this to only take 1 pass 99+% of the time, but there are
	// occasions where it takes a few more.]

	long nrPasses = 0;
	long nrJuggles;
	do
	{
		nrPasses++;
		nrJuggles = 0;

		de_css_item * pcss=m_pHead;
		while (pcss && pcss->getNext() && !pcss->getNext()->isEOF())
		{
			wxASSERT_MSG( (pcss->isEqual()), _T("Coding Error") );

			de_css_item * pcssNext = pcss->getNext();

			wxASSERT_MSG( (pcssNext->isEqual()), _T("Coding Error") );

			long a1 = pcss->get_a();	long a1Len = pcss->getLen_a();	long a2 = pcssNext->get_a();
			long b1 = pcss->get_b();	long b1Len = pcss->getLen_b();	long b2 = pcssNext->get_b();

			wxASSERT_MSG( (a1Len == b1Len), _T("Coding Error") );

			long lenGapA = a2 - (a1 + a1Len);
			long lenGapB = b2 - (b1 + b1Len);

			wxASSERT_MSG( ((lenGapA > 0) || (lenGapB > 0)), _T("Coding Error: adjacent CSS's") );

			// if we have an insert/delete (one side has a zero length gap) rather
			// than a change (where both gaps are non-zero), see if the complete
			// content (in pcss) of the insert/delete exactly matches the content
			// immediately prior to the next css.  the content that we match
			// ends at the end of the gap, but may extend back into the bottom
			// of the current css.

			if (   ((lenGapB == 0) && (m_pSrc->identical_A(a1, a2-a1Len, a1Len)))
				|| ((lenGapA == 0) && (m_pSrc->identical_B(b1, b2-b1Len, b1Len))) )
			{
				// match found.  we want to shift the content above the the non-zero
				// gap onto the end of the PREVIOUS gap and shift the content of the
				// current css to the bottom of the current gap.  the second step
				// will cause us to have 2 consecutive/adjacent equal nodes.  we can
				// coalesce them into one (pcssNext) with the content of both and
				// delete the current (pcss) one.

//				wxLogTrace(TRACE_DE_DUMP, _T("Juggle: Agap[%d] a1[%d,%d] a2[%d,%d] --> a[%d,%d]     Bgap[%d] b1[%d,%d] b2[%d,%d] --> b[%d,%d]"),
//						   lenGapA,  a1,a1Len, a2,pcssNext->getLen_a(),  pcssNext->m_a-a1Len,pcssNext->m_len_a+a1Len,
//						   lenGapB,  b1,b1Len, b2,pcssNext->getLen_b(),  pcssNext->m_b-b1Len,pcssNext->m_len_b+b1Len);

				pcssNext->m_a -= a1Len;		pcssNext->m_len_a += a1Len;
				pcssNext->m_b -= b1Len;		pcssNext->m_len_b += b1Len;

				_delete_item(pcss);

				nrJuggles++;

				// TODO see if we need/want/care to update the various stats (m_nrDelete/Inserts...)
			}
		
			pcss = pcssNext;
		}

//		wxLogTrace(TRACE_DE_DUMP,_T("Juggle: %ld juggles in pass %ld."), nrJuggles, nrPasses);

	} while (nrJuggles > 0);

//	wxLogTrace(TRACE_DE_DUMP, _T("Juggle: %ld passes."), nrPasses);
}

//////////////////////////////////////////////////////////////////

void de_css_list::_fillin_gaps(void)
{
	UTIL_PERF_START_CLOCK(sszKey_fillin_gaps);

	// add special non-equal nodes for gaps (inserts/deletes/changes)
	// 
	// CAN ONLY BE CALLED AFTER ALGORITHM HAS FINISHED AND LIST IS BUILT.

	long end_a = 0;
	long end_b = 0;

	for (de_css_item * pcss=m_pHead; (pcss); pcss=pcss->m_next)
	{
		wxASSERT_MSG( (pcss->getLen_a()==pcss->getLen_b()), _T("Coding Error"));

		long gap_a = pcss->get_a() - end_a;
		long gap_b = pcss->get_b() - end_b;

		if ( (gap_a > 0) || (gap_b > 0) )
			_insert_before( pcss, new de_css_item(end_a,gap_a,
												  end_b,gap_b) );

		end_a = pcss->get_a() + pcss->getLen_a();
		end_b = pcss->get_b() + pcss->getLen_b();
	}
		
	UTIL_PERF_STOP_CLOCK(sszKey_fillin_gaps);
}

void de_css_list::_insert_before(de_css_item * pcss, de_css_item * pNew)
{
	// insert item into the list before the one given.
	// 
	// CAN ONLY BE CALLED AFTER ALGORITHM HAS FINISHED AND LIST IS BUILT.

	de_css_item * pPrev = pcss->m_prev;
	pNew->m_prev = pPrev;
	pcss->m_prev = pNew;
	pNew->m_next = pcss;
	if (pPrev)
		pPrev->m_next = pNew;
	else
		m_pHead = pNew;
}

//////////////////////////////////////////////////////////////////

void de_css_list::rel2abs(long ndx_a, long ndx_b)
{
	// adjust the offsets in the CSSL from relative to absolute.
	// this is used after building a sub-cssl on a partition of
	// the documents -- bounded by de_mark's.
	// 
	// CAN ONLY BE CALLED AFTER ALGORITHM HAS FINISHED AND LIST IS BUILT.

	for (de_css_item * pcss=m_pHead; (pcss); pcss=pcss->m_next)
		pcss->rel2abs(ndx_a,ndx_b);
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void de_css_list::dump(int indent) const
{
	wxLogTrace(TRACE_DE_DUMP, _T("%*c=============================================================================="), indent,' ');
	if (m_bValid)
	{
		wxLogTrace(TRACE_DE_DUMP, _T("%*cDE_CSS_LIST: [M %ld][N %ld][NrDel %ld][NrIns %ld]"),indent,' ', m_M,m_N,m_nrDeletes,m_nrInserts);
		for (de_css_item * p=m_pHead; (p); p=p->m_next)
			p->dump(indent+5);
	}
	else
		wxLogTrace(TRACE_DE_DUMP, _T("%*cDE_CSS_LIST: not valid") );
	
	wxLogTrace(TRACE_DE_DUMP, _T("%*c=============================================================================="), indent,' ');
	
}

void de_css_vars::dump(int indent) const
{
	if (m_bEq)
		wxLogTrace(TRACE_DE_DUMP, _T("%*cDE_CSS: [x %ld][y %ld][len %ld]"),indent,' ',m_a,m_b,m_len_a);
	else
		wxLogTrace(TRACE_DE_DUMP, _T("%*cDE_CSS: [x %ld][y %ld][len %ld][len %ld]"),indent,' ',m_a,m_b,m_len_a,m_len_b);
}
#endif//DEBUG
