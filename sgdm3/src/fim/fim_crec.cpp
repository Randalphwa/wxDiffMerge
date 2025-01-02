// fim_crec.cpp -- a change record -- represents one individual change
// or a transaction end-point against a piecetable.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>

//////////////////////////////////////////////////////////////////

void fim_crec::set_ta(Verb v, TAID t)
{
	m_verb      = v;
	m_taid      = t;
	m_offsetBuf = 0;
	m_lenData   = 0;
	m_offsetDoc = 0;
	m_prop      = FR_PROP_ZERO;
	m_propNew   = FR_PROP_ZERO;

#ifdef DEBUG
	dump(0);
#endif
}

void fim_crec::set_text(Verb v, fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop)
{
	m_verb      = v;
	m_taid      = 0;
	m_offsetBuf = offsetBuf;
	m_lenData   = lenData;
	m_offsetDoc = offsetDoc;
	m_prop      = prop;
	m_propNew   = prop;

#ifdef DEBUG
	//dump(0);
#endif
}

void fim_crec::set_prop(fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop, fr_prop propNew)
{
	m_verb      = verb_prop_set;
	m_taid      = 0;
	m_offsetBuf = offsetBuf;
	m_lenData   = lenData;
	m_offsetDoc = offsetDoc;
	m_prop      = prop;
	m_propNew   = propNew;

#ifdef DEBUG
	dump(0);
#endif
}

//////////////////////////////////////////////////////////////////

fim_crec::fim_crec(const fim_crec * pCRec)
{
	// like one of the set_ methods.

	m_verb		= pCRec->m_verb;
	m_taid		= pCRec->m_taid;
	m_offsetBuf = pCRec->m_offsetBuf;
	m_lenData   = pCRec->m_lenData;
	m_offsetDoc = pCRec->m_offsetDoc;
	m_prop      = pCRec->m_prop;
	m_propNew   = pCRec->m_propNew;
}

fim_crec::fim_crec(void)
{
	m_verb		= verb_invalid;
	m_taid		= 0;
	m_offsetBuf = 0;
	m_lenData   = 0;
	m_offsetDoc = 0;
	m_prop      = FR_PROP_ZERO;
	m_propNew   = FR_PROP_ZERO;
}

void fim_crec::reverse(void)
{
	m_verb = (Verb) -m_verb;

	fr_prop propTemp = m_prop;
	m_prop           = m_propNew;
	m_propNew        = propTemp;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fim_crec::dump(int indent) const
{
	const wxChar * szTitle;

	switch (m_verb)
	{
	default:
	case verb_invalid:			szTitle = _T("invalid");		break;
	case verb_ta_begin:			szTitle = _T("ta_begin");		break;
	case verb_ta_end:			szTitle = _T("ta_end");			break;
	case verb_text_insert:		szTitle = _T("text_insert");	break;
	case verb_text_delete:		szTitle = _T("text_delete");	break;
	case verb_prop_set:
	case verb_prop_set2:		szTitle = _T("prop");			break;
	}
	
	wxLogTrace(TRACE_CREC_DUMP, _T("%*cFIM_CREC: [%p][%s][t %ld][ob %ld][len %ld][od %ld][prop %lx,%lx]"),
			   indent,' ',this,szTitle,m_taid,m_offsetBuf,m_lenData,m_offsetDoc,m_prop,m_propNew);
}
#endif
