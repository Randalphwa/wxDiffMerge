// rs_dcl.h -- forward declarations for this directory.
//////////////////////////////////////////////////////////////////

#ifndef H_RS_DCL_H
#define H_RS_DCL_H

//////////////////////////////////////////////////////////////////

typedef enum _rs_encoding_style { RS_ENCODING_STYLE_LOCAL		=0,	// use local deafult (see wxConvLocal)
								  RS_ENCODING_STYLE_ASK			=1,	// ask before loading the 2 or 3 files in a file set
								  RS_ENCODING_STYLE_ASK_EACH	=2,	// ask for each file (allows each to be different)
								  RS_ENCODING_STYLE_NAMED1		=3,	// use 1 explicitly named character encoding
								  RS_ENCODING_STYLE_NAMED2		=4,	// use 1 with 1 fallback
								  RS_ENCODING_STYLE_NAMED3		=5,	// use 1 with 2 fallback
								  __RS_ENCODING_STYLE__NR__			// must be last
} RS_ENCODING_STYLE;

// Warning: don't change values in RS_ENCODING_STYLE enum without looking at rs_ruleset_dlg.cpp:astrEncodingStyle.

//////////////////////////////////////////////////////////////////
// rs_context_attrs -- RuleSet Context Dependent Attributes.
//
// Unimportant -- is a change within this context treated as
//                completely UNIMPORTANT?  for example, a wording
//                change within a C++ comment can be said to be
//                not important -- as opposed to a change in the
//                code or in a string literal.
//
//                if a context is UNIMPORTANT, then we don't need
//                to worry about ignoring case, whitespace, etc,
//                because we're going to treat the whole context
//                as unimportant.
//
//                if a context is IMPORTANT, then we look inside
//                changes for parts that we can de-emphasize (like
//                changes in WHITESPACE within a comment).
//
//                This can be set globally (for all non-matching
//                content) and locally.
//
// Respect EOL -- do we respect the actual characters used in the
//                EOL?  when true, we include the actual CR, LF,
//                or CRLF chars with the line for diff purposes.
//
//                when not respected, we exclude them from everything.
//
//                The value for this in the default/global context
//                attrs decide how we handle it for the whole file.
//                
//                If globally not respected, this overrides the
//                individual context settings.
//
//                if globally respected, we then look to the
//                individual contexts.
//
// Respect White -- do we respect the WHITESPACE characters in
//                the document?
//
//                The value for this in the default/global context
//                attrs decide if we include the WHITESPACE chars
//                line hashes when running the diff engine.  If set,
//                then a simple change in the WHITESPACE will cause
//                lines to have different hashes and thus not necessarily
//                line up.  If not set, then we strip WHITESPACE before
//                we compute the line hashes.  (we do not strip from
//                the line buffer, so lines that compare as all equal
//                can have intra-line diffs; this will get fixed up
//                afterwards.)
//
//                The value for this in non-global, IMPORTANT contexts
//                determines if changes in WHITESPACE is significant.
//                When set, WHITESPACE changes are also IMPORTANT.
//                When not set, WHITESPACE changes are are UNIMPORTANT.
//
// Respect Case -- do we respect the CASE of characters in the document?
//
//                Like Respect White, the global value affects the hash
//                we compute for running the line-oriented diff.
//
//                Like Respect White, the non-global value affects the
//                significance (importance within important contexts).
//
// Tab Is White -- do we consider TAB to be WHITESPACE ?
//
//
//////////////////////////////////////////////////////////////////

typedef unsigned short					rs_context_attrs;
#define RS_ATTRS_UNIMPORTANT			(0x0001)
#define RS_ATTRS_RESPECT_EOL			(0x0002)
#define RS_ATTRS_RESPECT_WHITE			(0x0004)
#define RS_ATTRS_RESPECT_CASE			(0x0008)
#define RS_ATTRS_TAB_IS_WHITE			(0x0010)

#define RS_ATTRS__DEFAULT				(0x0000)

#define RS_ATTRS__MatchAllBits(a,b)		(((a) & (b)) == (b))
#define RS_ATTRS__MatchAnyBits(a,b)		(((a) & (b)) != (0))

#define RS_ATTRS_IsUnimportant(a)		(((a) & RS_ATTRS_UNIMPORTANT)==RS_ATTRS_UNIMPORTANT)

#define RS_ATTRS_RespectEOL(a)			(((a) & RS_ATTRS_RESPECT_EOL)==RS_ATTRS_RESPECT_EOL)
#define RS_ATTRS_RespectWhite(a)		(((a) & RS_ATTRS_RESPECT_WHITE)==RS_ATTRS_RESPECT_WHITE)
#define RS_ATTRS_RespectCase(a)			(((a) & RS_ATTRS_RESPECT_CASE)==RS_ATTRS_RESPECT_CASE)

// TODO create an explicit blank-lines-are-unimportant flag in the rs_context.
// TODO for now, just use the value of the whitespace flag.  this hack might be
// TODO noticable when diffing python source (since leading white is important,
// TODO but blank lines aren't).
#define RS_ATTRS_RespectBlankLines(a)	(RS_ATTRS_RespectWhite(a))

#define RS_ATTRS_TabIsWhite(a)			(((a) & RS_ATTRS_TAB_IS_WHITE)==RS_ATTRS_TAB_IS_WHITE)

//////////////////////////////////////////////////////////////////

class rs_choose_dlg;
class rs_choose_dlg__ruleset;
class rs_context;
class rs_context_dlg;
class rs_lomit;
class rs_lomit_dlg;
class rs_ruleset;
class rs_ruleset_dlg;
class rs_ruleset_dlg__add;
class rs_ruleset_dlg__edit;
class rs_ruleset_table;

//////////////////////////////////////////////////////////////////

#endif//H_RS_DCL_H
