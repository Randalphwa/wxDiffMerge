// de_dcl.h
//////////////////////////////////////////////////////////////////

#ifndef H_DE_DCL_H
#define H_DE_DCL_H

//////////////////////////////////////////////////////////////////
// we run the diff-engine 2 ways on a set of files (the contents
// of an fs_fs). these are indexed using SYNC_
// 
// [] SYNC_VIEW -- represents the original set of documents as
// loaded from disk; and may include either 2 or 3 files (a simple
// diff or a merge).  the diff-engine result for this is relatively
// static -- it only needs to change if the source files are explicitly
// reloaded by the user.
// 
// [] SYNC_EDIT -- represents the live diff of their editing session.
// prior to 6/28/06 this was always a simple diff against T1 (the
// common ancestor) where PANEL_B was cloned from T1 when the editing
// panel was enabled.  NOW, SYNC_EDIT is a 2 or 3 way diff (just like
// the other SYNC_) where the center panel is the edit buffer.

#define SYNC_VIEW		0
#define SYNC_EDIT		1
#define __NR_SYNCS__	2

//////////////////////////////////////////////////////////////////

#define DE_CHG_VIEW_CHG		0x01
#define DE_CHG_EDIT_CHG		0x02
#define DE_CHG__CHG_MASK	(DE_CHG_VIEW_CHG|DE_CHG_EDIT_CHG)


#define DE_CHG_VIEW_RUN		0x10
#define DE_CHG_EDIT_RUN		0x20
#define DE_CHG__RUN_MASK	(DE_CHG_VIEW_RUN|DE_CHG_EDIT_RUN)


#define DE_CHG_VIEW_DISP	0x0100
#define DE_CHG_EDIT_DISP	0x0200
#define DE_CHG__DISP_MASK	(DE_CHG_VIEW_DISP|DE_CHG_EDIT_DISP)


#define DE_CHG__VIEW_MASK	(DE_CHG_VIEW_CHG|DE_CHG_VIEW_RUN|DE_CHG_VIEW_DISP)
#define DE_CHG__EDIT_MASK	(DE_CHG_EDIT_CHG|DE_CHG_EDIT_RUN|DE_CHG_EDIT_DISP)

//////////////////////////////////////////////////////////////////

typedef unsigned short				de_attr;

// node types -- these are treated as an enum rather than bits (they are
// not consecutive, but are aranged in a pattern that makes it easier to
// read debug dumps).

#define DE_ATTR_UNKNOWN				((de_attr)0x0000)
#define DE_ATTR_EOF					((de_attr)0x0001)		// EOF node
#define DE_ATTR_OMITTED				((de_attr)0x0002)		// special "omitted" node
#define DE_ATTR_MARK				((de_attr)0x0004)		// marker for manual sync point

#define DE_ATTR_DIF_0EQ				((de_attr)0x0020)		// 10  00 set when both in {T1,T0} are different
#define DE_ATTR_DIF_2EQ				((de_attr)0x0023)		// 10  11 set when both in {T1,T0} are same

#define DE_ATTR_MRG_0EQ				((de_attr)0x0030)		// 11 000 set when all in {T1,T0,T2} are different
#define DE_ATTR_MRG_T0T2EQ			((de_attr)0x0033)		// 11 011 set if T0 == T2 (and T1 is different)
#define DE_ATTR_MRG_T1T2EQ			((de_attr)0x0035)		// 11 101 set if T1 == T2 (and T0 is different)
#define DE_ATTR_MRG_T1T0EQ			((de_attr)0x0036)		// 11 110 set if T1 == T0 (and T2 is different)
#define DE_ATTR_MRG_3EQ				((de_attr)0x0037)		// 11 111 set when all in {T1,T0,T2} are same.

#define DE_ATTR__TYPE_MASK			((de_attr)0x00ff)

#define DE_ATTR__DIF_KIND			((de_attr)0x0020)
#define DE_ATTR__MRG_KIND			((de_attr)0x0030)
#define DE_ATTR__KIND_MASK			((de_attr)0x00f0)

// modifiers for various types (not all combinations make sense)

#define DE_ATTR_CONFLICT			((de_attr)0x8000)		// node is member of conflict (may be 0-way-equal or within a sequence of adjacent incompatible changes)
#define DE_ATTR_UNIMPORTANT			((de_attr)0x4000)		// node only contains unimportant changes
#define DE_ATTR_PARTIAL				((de_attr)0x2000)		// partial node because of multi-line-intra-line splitting (don't assume LEN's are equal for EQ nodes)
															//   (PARTIAL is for Intra-Line nodes to help indicate that a particular span was placed on different
															//   lines in different panels)
#define DE_ATTR_SMOOTHED			((de_attr)0x1000)		// an EQ node that was changed to NEQ by threshold-smoothing
#define DE_ATTR_ML_MEMBER			((de_attr)0x0800)		// ML_MEMBER: a single line sync-node created by cutting up a multi-line change after multi-line-intra-line analysis or omitted line adjacent to change.
#define DE_ATTR_NONEXACTEQ			((de_attr)0x0400)		// a non-exact-EQ node that was changed to NEQ by _split_up_eqs[23]

#define DE_ATTR__MODIFIER_MASK		((de_attr)0xff00)

#define DE_ATTR__IS_SET(attr,bits)	(((attr) & (bits)) == (bits))
#define DE_ATTR__IS_TYPE(attr,bits)	(((attr) & DE_ATTR__TYPE_MASK) == (bits))
#define DE_ATTR__IS_KIND(attr,bits)	(((attr) & DE_ATTR__KIND_MASK) == (bits))

de_attr de_attr_union_attr(de_attr attr1, de_attr attr2);

//////////////////////////////////////////////////////////////////

typedef unsigned short					de_display_ops;

// display mode -- these are treated as an enum rather than bits

#define DE_DOP_ALL					((de_display_ops)0x0000)	// show equal & changes
#define DE_DOP_DIF					((de_display_ops)0x0001)	// show changes only
#define DE_DOP_CTX					((de_display_ops)0x0002)	// show changes with context
#define DE_DOP_EQL					((de_display_ops)0x0003)	// show only equal parts

#define DE_DOP__MODE_MASK			((de_display_ops)0x000f)

#define DE_DOP__IS_MODE(dops,mode)	(((dops) & DE_DOP__MODE_MASK) == (mode))
#define DE_DOP__IS_MODE_ALL(dops)	( DE_DOP__IS_MODE((dops),DE_DOP_ALL) )
#define DE_DOP__IS_MODE_DIF(dops)	( DE_DOP__IS_MODE((dops),DE_DOP_DIF) )
#define DE_DOP__IS_MODE_CTX(dops)	( DE_DOP__IS_MODE((dops),DE_DOP_CTX) )
#define DE_DOP__IS_MODE_EQL(dops)	( DE_DOP__IS_MODE((dops),DE_DOP_EQL) )


// modifiers for various modes (not all combinations make sense)

#define DE_DOP_IGN_UNIMPORTANT		((de_display_ops)0x0010)	// ignore unimportant changes (display like/with equal lines)
#define DE_DOP_HIDE_OMITTED			((de_display_ops)0x0020)	// hide lines which were omitted from the comparison

#define DE_DOP__MODIFIER_MASK		((de_display_ops)0x00f0)

#define DE_DOP__IS_SET(dops,bit)				(((dops) & (bit)) == (bit))
#define DE_DOP__IS_SET_IGN_UNIMPORTANT(dops)	( DE_DOP__IS_SET((dops),DE_DOP_IGN_UNIMPORTANT) )
#define DE_DOP__IS_SET_HIDE_OMITTED(dops)		( DE_DOP__IS_SET((dops),DE_DOP_HIDE_OMITTED) )

//////////////////////////////////////////////////////////////////

// diff-engine detail level

typedef enum _de_detail_level { DE_DETAIL_LEVEL__LINE=0,	// must match astrDetailLevel[] in OptionsDlg.cpp
								DE_DETAIL_LEVEL__CHAR=1,
								__DE_DETAIL_LEVEL__NR__
} de_detail_level;

#define DE_DETAIL_LEVEL__USES_IGN_UNIMP(level)		((level)==DE_DETAIL_LEVEL__CHAR)

// diff-engine multi-line intra-line detail level

typedef enum _de_multiline_detail_level { DE_MULTILINE_DETAIL_LEVEL__NONE=0,	// no multi-line intraline analysis
										  DE_MULTILINE_DETAIL_LEVEL__NEQS=1,	// do multi-line on proper NEQs (exclude non-exact-EQs and smoothed-NEQs)
										  DE_MULTILINE_DETAIL_LEVEL__FULL=2,	// do full multi-line intraline analysis on all NEQs
										  __DE_MULTILINE_DETAIL_LEVEL__NR__
} de_multiline_detail_level;

//////////////////////////////////////////////////////////////////

typedef unsigned short de_mark_type;
#define DE_MARK_TRIVIAL				((de_mark_type)0x0001)
#define DE_MARK_USER				((de_mark_type)0x0002)
#define DE_MARK_BEGIN_CONFLICT		((de_mark_type)0x0004)
#define DE_MARK_END_CONFLICT		((de_mark_type)0x0008)

//////////////////////////////////////////////////////////////////

typedef std::vector<long> TVecLongs;
typedef std::vector<bool> TVecBools;

//////////////////////////////////////////////////////////////////

class de_css_item;
class de_css_list;
class de_css_vars;
class de_css_src;
class de_css_src_clipped;
class de_css_src_lines;
class de_css_src_simple_strings;
class de_css_src_string;
class de_de;
class de_line;
class de_mark;
class de_patch;
class de_patch__conflict;
class de_patch__delete;
class de_patch__insert;
class de_patch__replace;
class de_row;
class de_span;
class de_stats2;
class de_stats3;
class de_sync;
class de_sync_line_omitted;
class de_sync_list;

//////////////////////////////////////////////////////////////////

#endif//H_DE_DCL_H
