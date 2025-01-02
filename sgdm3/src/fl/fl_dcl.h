// fl_dcl.h -- forward declarations for this directory
//////////////////////////////////////////////////////////////////

#ifndef H_FL_DCL_H
#define H_FL_DCL_H

//////////////////////////////////////////////////////////////////

typedef int							fl_slot;

//////////////////////////////////////////////////////////////////

typedef unsigned long				fl_invalidate;
#define FL_INV_FULL					((fl_invalidate)0x0001)	// force full invalidate
#define FL_INV_LINENR				((fl_invalidate)0x0002)	// line numbers values need fixing
#define FL_INV_COL					((fl_invalidate)0x0004)	// column values need fixing
#define FL_INV_PROP					((fl_invalidate)0x0008)
#define FL_INV__MASK				((fl_invalidate)0x00ff)

#define FL_INV_IS_SET(inv,flag)		(((inv)&(flag))==(flag))
#define FL_INV_ANY_SET(inv,flags)	(((inv)&(flags)) != 0)

typedef unsigned long				fl_change;
#define FL_MOD_INS_LINE				((fl_change)	0x0100)	// a fl_line has been inserted
#define FL_MOD_DEL_LINE				((fl_change)	0x0200)	// a fl_line has been deleted
#define FL_MOD__MASK				((fl_change)	0xff00)

#define FL_MOD_IS_SET(mod,flag)		(((mod)&(flag))==(flag))
#define FL_MOD_ANY_SET(mod,flags)	(((mod)&(flags)) != 0)

#define FL_CB_IS_MOD(arg)			(FL_MOD_ANY_SET((arg).m_l,FL_MOD__MASK))
#define FL_CB_IS_INV(arg)			(fl_INV_ANY_SET((arg).m_l,FL_INV__MASK))

//////////////////////////////////////////////////////////////////

class fl_fl;
class fl_line;
class fl_run;
class fl_run_list;
class fl_run_list_endpoints;

//////////////////////////////////////////////////////////////////

typedef std::map<const fim_frag *, fl_run_list_endpoints>	TMap_FragToRunList;
typedef TMap_FragToRunList::iterator						TMap_FragToRunListIterator;
typedef TMap_FragToRunList::const_iterator					TMap_FragToRunListConstIterator;
typedef TMap_FragToRunList::value_type						TMap_FragToRunListValue;

//////////////////////////////////////////////////////////////////

#endif//H_FL_DCL_H
