// de_css.h -- Common SubSequence for Diff Engine
//////////////////////////////////////////////////////////////////
//
// de_css_src is an interface to let us test/compare A[] and B[]
// without knowing about how they are implemented.  note that we
// use zero-based indexes rather than 1-based as implied by the
// paper.  the paper requires Len(A) <= Len(B)   WE DEVIATE FROM
// THE PAPER AND ALLOW EITHER A OR B TO BE LONGER.
// 
// de_css_vars represents index & length of an identical (common)
// subsequence within 2 documents.  in the paper, these are shown
// as diagonal streaks of 1 or more tokens.  WE DEVIATE FROM THE
// PAPER AND CHANGING 'x' TO 'a' AND 'y' TO 'b'.  These refer to
// indexes into A[] and B[].  This way caller doesn't get confused
// with our indexes and pixel locations...
//
// de_css_item wraps de_css_vars and adds list pointers.
// 
// de_css_list is a container for de_css_item.  it manages their lifetime.
// as we run the diff-engine, it maintains a table of all css's
// identified (partial results, if you will) that represent possible
// solutions.  after the diff-engine is finished (and a SHORTEST
// PATH is found), it represents the head/tail of a doubly-linked
// list of the css's in the solution.
//
// after the diff-engine runs, we have a list of common sub-sequences
// (that is, a list of the index/length of places where the two
// documents are equal).  after the algorithm runs, we insert NON-EQUAL
// items to the list to represent gaps -- places where the docs are
// not equal -- for inserts/deletes/changes.
// 
//////////////////////////////////////////////////////////////////

#ifndef H_DE_CSS_H
#define H_DE_CSS_H

//////////////////////////////////////////////////////////////////
// de_css_src is a semi-abstract class/interface for a pair of data
// sources.  the diff-engine uses this to remain free of any
// particular input data.
//////////////////////////////////////////////////////////////////

class de_css_src
{
public:
	virtual ~de_css_src(void) {};

	virtual long		getLenA(void)			const = 0;	// return Length(A) (M in paper)
	virtual long		getLenB(void)			const = 0;	// return Length(B) (N in paper)
	virtual bool		equal(long a, long b)	const = 0;	// return (A[a]==B[b])

	virtual bool		identical_A(long a1, long a2, long len)	const = 0;	// return (A[a1...a1+len]==A[a2...a2+len])
	virtual bool		identical_B(long b1, long b2, long len)	const = 0;	// return (A[a1...a1+len]==A[a2...a2+len])

	bool				identical(void)			const;
};

//////////////////////////////////////////////////////////////////
// de_css_vars represents the set of parameters/variables in a
// sub-sequence -- a set of offset/lengths -- into a set of documents.
//////////////////////////////////////////////////////////////////

class de_css_vars
{
	friend class de_css_list;

public:
	de_css_vars(void);
	de_css_vars(long a, long b, long len);
	de_css_vars(long a, long len_a,
				long b, long len_b);
	de_css_vars(const de_css_vars * pcss);

	void				set(long a, long b, long len);
	void				set(long a, long len_a,
							long b, long len_b);
	
	void				setFrom(const de_css_vars * pcss);
	void				shift(long len);
	void				shift(long len_a, long len_b);

	void				rel2abs(long ndx_a, long ndx_b);

public:
	inline long			get_a(void)				const { return m_a; };
	inline long			get_b(void)				const { return m_b; };
	inline long			getLen_a(void)			const { return m_len_a; };
	inline long			getLen_b(void)			const { return m_len_b; };

	inline bool			isEqual(void)			const { return m_bEq; };

private:
	long				m_a;			// position in A[]
	long				m_b;			// position in B[]
	long				m_len_a;
	long				m_len_b;

	bool				m_bEq;			// true iff common sub-sequence; false iff gap (ins/del/chg) between common sub-sequences

#ifdef DEBUG
public:
	void				dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////
// de_css_src_clipped
//
// semi-abstract version of de_css_src that implements a clipping
// region on an existing data source.  we can use this to confine
// the diff-engine algorithm to the context of a multi-way conflict,
// for example.

class de_css_src_clipped : public de_css_src
{
public:
	de_css_src_clipped(const de_css_src * pSrc,
					   long offset_a, long len_a,
					   long offset_b, long len_b)
		: m_pSrc(pSrc),
		  m_offset_a(offset_a), m_len_a(len_a),
		  m_offset_b(offset_b), m_len_b(len_b)
		{
			wxASSERT_MSG( ((m_len_a >= 0) && (m_len_b >= 0)), _T("Coding Error") );
		};

	de_css_src_clipped(const de_css_src * pSrc,
					   const de_css_vars * pVars)
		: m_pSrc(pSrc),
		  m_offset_a(pVars->get_a()), m_len_a(pVars->getLen_a()),
		  m_offset_b(pVars->get_b()), m_len_b(pVars->getLen_b())
		{};

	virtual ~de_css_src_clipped(void) {};

	virtual long		getLenA(void)			const	{ return (m_len_a); };
	virtual long		getLenB(void)			const	{ return (m_len_b); };
	virtual bool		equal(long a, long b)	const	{ return m_pSrc->equal(m_offset_a + a, m_offset_b + b); };

	virtual bool		identical_A(long a1, long a2, long len)	const	{ return m_pSrc->identical_A(m_offset_a+a1, m_offset_a+a2, len); };
	virtual bool		identical_B(long b1, long b2, long len)	const	{ return m_pSrc->identical_B(m_offset_b+b1, m_offset_b+b2, len); };

	virtual long		getOffsetA(void)		const	{ return (m_offset_a); };
	virtual long		getOffsetB(void)		const	{ return (m_offset_b); };

private:
	const de_css_src *	m_pSrc;					// we don't own this
	long				m_offset_a, m_len_a;
	long				m_offset_b, m_len_b;
};

//////////////////////////////////////////////////////////////////
// de_css_item is used to represent a sub-sequence (a _vars) in a css list.
//////////////////////////////////////////////////////////////////

class de_css_item : public de_css_vars
{
private:
	friend class de_css_list;

	de_css_item(long a, long b, long len, de_css_item * pPrev);
	de_css_item(long a, long len_a,
				long b, long len_b);

public:
	inline de_css_item *	getNext(void)			const { return m_next; };
	inline de_css_item *	getPrev(void)			const { return m_prev; };

	bool					isEOF(void)				const { return (m_next==NULL); };

private:
	de_css_item *			m_next;
	de_css_item *			m_prev;
};

//////////////////////////////////////////////////////////////////
// de_css_list is a list of css's and contains the guts of the
// diff-engine algorithm.  instantiate this with a data src (or
// call runAlgorithm with one) and it will take care of all the
// diff-engine details.  getHead() will then return the list of
// css's.
//////////////////////////////////////////////////////////////////

class de_css_list
{
public:
	de_css_list(de_css_src * pSrc=NULL);
	~de_css_list(void);

	void					runAlgorithm(de_css_src * pSrc);
	void					rel2abs(long ndx_a, long ndx_b);

	inline bool				isValid(void)			const { return m_bValid; };
	void					setInvalid(void);

	inline de_css_item *	getHead(void)			const { return m_pHead; };
	inline de_css_item *	getTail(void)			const { return m_pTail; };

	inline long				getNrDeletions(void)	const { return m_nrDeletes; };	// stats
	inline long				getNrInserts(void)		const { return m_nrInserts; };	// stats

private:
	void					_zero(void);

	void					_compare(void);												// see paper
	void					_snake(long k);												// see paper

	de_css_item *			_addCSS(long x0, long y0, long len, de_css_item * pPrev);	// table-mode -- while searching for a solution
	void					_convertToList(void);										// convert from table-mode to list-mode

	void					_init_fp(void);
	void					_delete_fp(void);

	void					_deleteTable(void);
	void					_deleteList(void);

	void					_delete_item(de_css_item * pItem);
	void					_juggle(void);

	void					_fillin_gaps(void);
	void					_insert_before(de_css_item * pcss, de_css_item * pNew);

private:
	de_css_src *			m_pSrc;				// interface to A[] and B[]  (for temporary use only during our _compare())
	bool					m_bValid;			// true if runAlgorithm() has run
	bool					m_bSwapped;			// true if we internally swap A & B to keep Len(A)<=Len(B) (algorithm assumption)

	// working fp arrays (for partial results as we run the algorithm (see paper))
	long *					m_pArrayFPy;		// allocated zero-based array fp[] containing "Y" values
	long *					m_pFPy;				// fp[-(M+1)..(N+1)] of "Y" values as in paper (set to &m_pArrayFPy[M+1])
	
	de_css_item **			m_pArrayFPcss;		// allocated zero-based array css[]
	de_css_item **			m_pFPcss;			// fp[-(M+1)..(N+1)] of "CSS" values -- tail of best path at this step

	// css table (for potential paths as we run the algorithm)
	typedef std::vector<de_css_item *>	TVec;
	typedef TVec::iterator				TVecIterator;
	TVec					m_vec;

	// css list (final solution from algorithm -- a shortest path)
	de_css_item *			m_pHead;
	de_css_item *			m_pTail;

	// various stats
	long					m_nrDeletes;
	long					m_nrInserts;
	long					m_M;
	long					m_N;

#ifdef DEBUG
public:
	void					dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_DE_CSS_H
