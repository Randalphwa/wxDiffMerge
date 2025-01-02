// de_mark.h
// data structures to let us mark or hard-link lines in each panel.
// this is used to do manual-alignment within the diff-engine -- where
// the user can specify a line from each panel and force us to sync
// them together -- this can be useful if, for example, large functions
// within 2 versions of a source file have been re-arranged and also
// changed and they'd like to see the functions side-by-side (rather
// than the somewhat random sync-ing they might be getting).
//
// we might also use this to delimit certain kinds patches (such as
// an insert-before) that have already been applied (in a 3-way) so
// that the new text in the EDIT panel does not get (bogusly) sync'd.
// consider this case:
//
// T0  T1  T2
// ==  ==  ==
// aa  aa  aa
// xx  yy  zz
// ee  ee  ee
//
// if the user inserts "xx" before "yy" in T1, giving:
// 
// T0  T1  T2
// ==  ==  ==
// aa  aa  aa
// xx  xx  z?
// ..  yy  z?
// ee  ee  ee
//
// should we let the sync do it's thing and try to compare "xx yy" vs "zz"
// or should we insert a "sync-point" and force "zz" to compare against
// only "yy"?
//
// T0  T1  T2
// ==  ==  ==
// aa  aa  aa
// xx  xx  ..
// ----------
// ..  yy  zz
// ee  ee  ee
//
// in the former case, how "zz" gets sync'd will depend upon the
// contents of "xx", "yy", and "zz".  this may cause the user some
// confusion -- because the entire diff aligment of the rest of the
// file could be changed (for example, blank lines within "xx", "yy",
// "zz" or "ee").
//
// for complete sanity, it might be nice to insert a sync-point
// afterwards as well -- to ensure that the alignment doesn't
// change radically.  consider:
//
// T0  T1  T2
// ==  ==  ==
// aa  aa  aa
// xx  xx  ..
// ----------
// ..  yy  zz
// ----------
// ee  ee  ee
// 
//////////////////////////////////////////////////////////////////

#ifndef H_DE_MARK_H
#define H_DE_MARK_H

//////////////////////////////////////////////////////////////////
// a mark "hangs" off of the lines in each panel.  visually, they
// are associated with layout lines.  we let each de_line where there
// is a mark to link to a de_mark; this mark is referenced by the
// corresponding lines in each panel.  the mark has back pointers
// to each of the lines.  both marks and de_lines persist between
// invocations of de_de::run() and all the associated computations.
// [it's only when a line is deleted that we have to worry.]
//
// when de_de::run() begins, _build_vector() builds a vector of the
// de_lines to be analyzed in each panel to give to de_css.  normally
// de_css run on the complete vectors (uses all of each document).
//
// we build a list of marks and use the de_css_src_clipped to trick
// the css engine to run using subsets of the vectors to essentially
// chunk the documents;  these partial css-lists are used to build
// partial sync-lists; these can then be assembled into a single
// sync-list (using a mark-type sync-node between the pieces).
//
// when the display-list is built from the sync-list, we can have
// special rows that represent the mark.  this gives us a place to
// let the user see the forced alignment and maybe click on to
// modify.
//
// note: except for the trival mark, each mark lists the lines that
// note: begin the next subset. 
//////////////////////////////////////////////////////////////////

class de_mark
{
public:
	de_mark(long kSync, de_mark_type t=DE_MARK_TRIVIAL, de_line * pDeLineT0=NULL, de_line * pDeLineT1=NULL, de_line * pDeLineT2=NULL);
	de_mark(const de_mark & ref);
	~de_mark(void);

	void						clearBeforeDelete(void);

	inline void					setDeLine(PanelIndex kPanel, de_line * pDeLine)		{ m_deLines[kPanel] = pDeLine; };
	inline de_line *			getDeLine(PanelIndex kPanel)				const	{ return m_deLines[kPanel]; };

	inline long					getSync(void)								const	{ return m_kSync; };
	inline de_mark_type			getType(void)								const	{ return m_type; };
	inline de_mark_type			addType(de_mark_type t)								{ m_type |= t; return m_type; };

	bool						isTrivial(void) const;

	inline de_css_list *		getCSSL(PanelIndex kPanel)		{ wxASSERT_MSG( (kPanel==PANEL_T0 || kPanel==PANEL_T2), _T("Coding Error")); return &m_cssl[kPanel]; };

protected:
	long						m_kSync;
	de_mark_type				m_type;

	de_line *					m_deLines[__NR_TOP_PANELS__];	// corresponding lines in each panel (relatively unchanging)
	de_css_list					m_cssl[__NR_TOP_PANELS__];		// we only use PANEL_T0 and PANEL_T2
};

//////////////////////////////////////////////////////////////////

typedef std::list<de_mark *>				TList_DeMark;
typedef TList_DeMark::iterator				TList_DeMarkIterator;
typedef TList_DeMark::const_iterator		TList_DeMarkConstIterator;
typedef TList_DeMark::value_type			TList_DeMarkValue;

//////////////////////////////////////////////////////////////////

#endif//H_DE_MARK_H
