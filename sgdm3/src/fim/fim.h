// fim.h
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_H
#define H_FIM_H

//////////////////////////////////////////////////////////////////

#include <fim_buf.h>
#include <fim_buf_table.h>
#include <fim_crec.h>
#include <fim_crecvec.h>
#include <fim_frag.h>
#include <fim_patchset.h>
#include <fim_ptable.h>
#include <fim_ptable_table.h>

//////////////////////////////////////////////////////////////////

#define TRACE_FIMBUF_DUMP		wxT("fim_buf_dump")
#define TRACE_CREC_DUMP			wxT("crec_dump")
#define TRACE_FIM_DUMP			wxT("fim_dump")
#define TRACE_FRAG_DUMP			wxT("frag_dump")
#define TRACE_PTABLE_DUMP		wxT("ptable_dump")

//////////////////////////////////////////////////////////////////

extern fim_buf_table *			gpFimBufTable;
extern fim_ptable_table *		gpPTableTable;

//////////////////////////////////////////////////////////////////

#endif//H_FIM_H
