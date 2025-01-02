// fim_dcl.h
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_DCL_H
#define H_FIM_DCL_H

//////////////////////////////////////////////////////////////////

typedef size_t		fim_length;	// length of a buffer of raw content data (like a size_t)
typedef size_t		fim_offset;	// an absoulte document offset

typedef size_t		fb_offset;	// location of some raw content data within a fim_buf (growbuf) -- like an array index or a handle to movable data
typedef size_t		fr_offset;	// location of some raw content within a fim_frag (and relative to frag's origin within fim_buf)

//////////////////////////////////////////////////////////////////
// fragment properties -- these are per-frag properties associated with
// the span of content represented by the frag.  they have *NO* meaning
// the piecetable -- it's just a bit-mask.  the piecetable will split/
// coalesce frags as necessary to respect the bitmask, **BUT** it has no
// idea what the bits mean.
//
// furthermore, the PIECETABLE knows nothing of line boundaries or anything;
// it just manages the content.  so, for example, if the user puts the
// cursor in the middle of line and starts typing, the LAYOUT may want to:
// [] begin a transaction,
// [] set a "dirty bit" on all of the content on the current line,
// [] insert the new text,
// [] end transaction.
// how all of this is handled is a question for the layout.

typedef unsigned long		fr_prop;
#define FR_PROP_ZERO		((fr_prop) 0x00000000)
#define FR_PROP__INSERTED	((fr_prop) 0x00000001)
// note that we don't define any meaning for the bits here -- that's
// for a higher level.
#define FR_PROP_TEST(p,v)	( ((p)&(v)) == (v) )

//////////////////////////////////////////////////////////////////
// fragment operations -- every change the piecetable makes to a
// document can be expressed as one or more of these operations.

typedef enum _frag_op		{	FOP_INVALID=0,
								FOP_INSERT_INITIAL,
								FOP_INSERT_BEFORE,
								FOP_INSERT_BEFORE_R,
								FOP_INSERT_AFTER,
								FOP_INSERT_AFTER_R,
								FOP_SPLIT,
								FOP_JOIN_BEFORE,
								FOP_JOIN_AFTER,
								FOP_DELETE_SELF,
								FOP_DELETE_SELF_R,
								FOP_SET_PROP,
							} fr_op;

//////////////////////////////////////////////////////////////////

typedef unsigned long				pt_stat;			// piecetable status -- a combination of bits
#define PT_STAT_ZERO				((pt_stat) 0x0000)

#define PT_STAT_IS_DIRTY			((pt_stat) 0x0001)	// dirty -- document changed since loaded or last save
#define PT_STAT_HAVE_AUTO_MERGED	((pt_stat) 0x0002)	// has auto-merge already been used
#define PT_STAT_CAN_UNDO			((pt_stat) 0x0004)	// can undo
#define PT_STAT_CAN_REDO			((pt_stat) 0x0008)	// can redo
#define PT_STAT_PATHNAME_CHANGE		((pt_stat) 0x0010)	// ptable rebound to different poi (SaveAs)
#define PT_STAT__STATE_MASK			((pt_stat) 0x00ff)	// mask for above states

#define PT_STAT_AUTOSAVE_BEGIN		((pt_stat) 0x0100)	// auto-save has started
#define PT_STAT_AUTOSAVE_END		((pt_stat) 0x0200)	// auto-save has finished
#define PT_STAT_AUTOSAVE_ERROR		((pt_stat) 0x0400)	// auto-save had an error
#define PT_STAT__STATE_EVENT		((pt_stat) 0xff00)	// mask for above events

#define PT_STAT_TEST(s,v)			( ((s)&(v)) == (v) )

//////////////////////////////////////////////////////////////////

typedef enum _fim_eol_mode	{	FIM_MODE_UNSET=0,
								FIM_MODE_LF,
								FIM_MODE_CRLF,
								FIM_MODE_CR,
							} fim_eol_mode;

#if   defined(__WXGTK__)
#	define FIM_MODE_NATIVE_CLIPBOARD		FIM_MODE_LF
#	define FIM_MODE_NATIVE_CLIPBOARD_STR	_T("\n")
#	define FIM_MODE_NATIVE_DISK				FIM_MODE_LF
#	define FIM_MODE_NATIVE_DISK_STR			_T("\n")
#elif defined(__WXMSW__)
#	define FIM_MODE_NATIVE_CLIPBOARD		FIM_MODE_CRLF
#	define FIM_MODE_NATIVE_CLIPBOARD_STR	_T("\r\n")
#	define FIM_MODE_NATIVE_DISK				FIM_MODE_CRLF
#	define FIM_MODE_NATIVE_DISK_STR			_T("\r\n")
#elif defined(__WXMAC__)
// Mac Classic (os 9 and below) used CR as their eol character
// Max OS X uses LF like Unix systems.
// Files on disk should be LF.
// But I'm seeing CR in clipboard pastes from TextEdit.
// so for now, the platform native values for mac are set differently.
#	define FIM_MODE_NATIVE_CLIPBOARD		FIM_MODE_CR
#	define FIM_MODE_NATIVE_CLIPBOARD_STR	_T("\r")
#	define FIM_MODE_NATIVE_DISK				FIM_MODE_LF
#	define FIM_MODE_NATIVE_DISK_STR			_T("\n")
#endif

//////////////////////////////////////////////////////////////////

typedef enum _patch_op
{
	POP__FIRST__,	// must be first
	POP_IGNORE,
	POP_DELETE,
	POP_INSERT_L, POP_INSERT_R,
	POP_REPLACE_L, POP_REPLACE_R,
	POP_CONFLICT,
	POP__LAST__		// must be last

} fim_patch_op;

//////////////////////////////////////////////////////////////////

class fim_buf;
class fim_buf_table;
class fim_crec;
class fim_crecvec;
class fim_frag;
class fim_patch;
class fim_patchset;
class fim_ptable;
class fim_ptable_table;

//////////////////////////////////////////////////////////////////

#endif//H_FIM_DCL_H
