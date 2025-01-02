// fim_crecvec -- an unbounded vector of change records.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>

//////////////////////////////////////////////////////////////////

fim_crecvec::fim_crecvec(void)
	: m_pVec(NULL),
	  m_cSizeAllocated(0),
	  m_cSizeTop(0),
	  m_cSizePos(0),
	  m_cSizeClean(0),
	  m_taidNext(1)
{
}

fim_crecvec::~fim_crecvec(void)
{
	FREEP(m_pVec);
}

//////////////////////////////////////////////////////////////////

fim_crec * fim_crecvec::_push_crec(void)
{
	// make space for a new crec -- a "do" -- that is, the original verb/action
	// that may later be undone and maybe redone.
	//
	// to save time/space we bulk allocate an arary of crec's and just return
	// pointers to items in the array.  further, we do the allocations in
	// chunks so that we don't have to realloc very often.
	//
	// when we abandon a 'redo' path (when the user does one or more 'undo's
	// and then starts editing without 'redo'ing), we recycle the crec's.
	//
	// as a convenience, we return a TEMPORARY pointer to the crec we just pushed.

#define CHUNK 1000
	if (!m_pVec)
	{
		m_cSizeAllocated = CHUNK;
		m_pVec = (fim_crec *)malloc(m_cSizeAllocated*sizeof(fim_crec));
	}
	else if (m_cSizePos == m_cSizeAllocated)
	{
		m_cSizeAllocated += CHUNK;
		m_pVec = (fim_crec *)realloc(m_pVec,m_cSizeAllocated*sizeof(fim_crec));
	}
#undef CHUNK

	if (m_cSizeTop > m_cSizePos)		// abandon 'redo' path
	{
		m_cSizeTop = m_cSizePos;

		// TODO when we abandon the 'redo' path, we might also be able to abandon
		// TODO a chunk of text at the end of the fim_buf -- corresponding to all
		// TODO the text that was inserted during the sequence -- provided that the
		// TODO fim_buf is not being shared by multiple fim_ptable's.
		// TODO (see TODO in fim_ptable_table::clone())
	}
	
	if (m_cSizeClean > m_cSizePos)		// last known 'clean/saved state' is in abandoned portion of 'redo' path
		m_cSizeClean = -1;				// mark us un-cleanable (no amount of undo/redo will get us to match what's on disk) until they re-save us.

	m_cSizeTop++;						// claim a space on the top of the stack

	fim_crec * pCRec = &m_pVec[m_cSizePos];

	// note: we don't bother memzero'ing the crec because our caller will populate it.
	
	m_cSizePos++;						// position for next push

	return pCRec;						// return TEMPORARY pointer for convenience
}

//////////////////////////////////////////////////////////////////

const fim_crec * fim_crecvec::push_ta_begin(void)
{
	fim_crec * pCRec = _push_crec();
	pCRec->set_ta(fim_crec::verb_ta_begin,getNewTAID());

	return pCRec;
}

const fim_crec * fim_crecvec::push_ta_end(fim_crec::TAID taidBegin)
{
	fim_crec * pCRec = _push_crec();
	pCRec->set_ta(fim_crec::verb_ta_end,taidBegin);

	return pCRec;
}

const fim_crec * fim_crecvec::push_text(fim_crec::Verb v, fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop,
										const fim_buf * pFimBuf)
{
	// see if we can combine this operation with the previous crec.
	// this will effectively "Glob" the edit with the previous.
	// during interactive editing, it allows multiple keystokes
	// (like adding new text) to be combined so that a single UNDO
	// can remove it.
	//
	// we restrict this to certain len == 1 inserts/deletes.
	// i don't want to do globbing for CUTs/PASTEs/etc.
	//
	// we require buffer to be "dirty" so that we don't coalesce
	// across save-points -- that is, if you type a letter, hit
	// save, and type another letter, we shouldn't coalesce the
	// 2 inserts.  this causes all kinds of problems:
	// [] it prevents us from detecting the buffer being dirty
	//    after the second event -- so the file-save is not enabled.
	// [] undo/redo can't return to the save point -- only before
	//    the first insert or after the second, but not in between.
	
	if ((lenData == 1) && (m_cSizePos > 0) && isDirty())		// "probably normal typing" and we have a previous crec
	{
		fim_crec * pCrecPrev = &m_pVec[m_cSizePos-1];
		if (pCrecPrev->isText() && (pCrecPrev->getVerb() == v) && (pCrecPrev->getProp() == prop))
		{
			// previous crec similar to the one we're supposed to create.
			
			switch (v)
			{
			case fim_crec::verb_text_insert:		// we have an insert-text
				{
					if (   (pCrecPrev->getBufOffset()+pCrecPrev->getBufLength() == offsetBuf)
						&& (pCrecPrev->getDocOffset()+pCrecPrev->getBufLength() == offsetDoc))
					{
						// the previous crec is also an insert-text and OUR INSERT
						// IS IMMEDIATELY AFTER AND CONTIGUOUS TO THE PREVIOUS INSERT,
						// so we can coalesce the inserts so that if the user does an
						// UNDO operation, it'll undo both inserts in one.  this is really
						// useful for interactive typing in the edit buffer -- where you
						// don't want to force the user to undo each keystroke/letter.
						//
						// think of this is globbing the insert -- like emacs does.
						//
						// now, just because we can combine them, it doesn't mean that
						// we always want to.  there are a few cases where we don't:
						//
						// [] when on a word break -- that is, we'd kinda like the undo
						//    to glob by word.  that way if they type an entire paragraph
						//    without making any mistakes, we don't want to make it all
						//    go away on 1 undo; let them undo-by-word.
						// [] others ??

						//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_crecvec::push_text: Could glob insert_text."));

#define MY_IS_WHITE(c)	 ( ((c)==_T(' ')) || ((c)==_T('\t')) || ((c)==_T('\n')) || ((c)==_T('\r')) )

						const wxChar * pchFirstNew = pFimBuf->getTemporaryPointer(offsetBuf);
						const wxChar * pchLastExisting = pFimBuf->getTemporaryPointer(pCrecPrev->getBufOffset()+pCrecPrev->getBufLength()-1);

						if (!MY_IS_WHITE(*pchFirstNew) && MY_IS_WHITE(*pchLastExisting))	// non-white following white marks start of new "word"
							break;

						// coalesce insert-text with previous crec.  return temporary crec
						// to allow caller to "apply" just the delta.

						pCrecPrev->expandRightEdge(lenData);

						m_crecGlobTemp.set_text(v,offsetBuf,lenData,offsetDoc,prop);

						//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_crecvec::push_text: Did glob insert_text."));
						return &m_crecGlobTemp;
					}
				}
				break;

			case fim_crec::verb_text_delete:		// we have a delete-text
				{
					//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_crecvec::delete_text: args [bufOffset %ld][len %ld][docOffset %ld]"),
					//		   offsetBuf,lenData,offsetDoc);
					//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_crecvec::delete_text: prev [bufOffset %ld][len %ld][docOffset %ld]"),
					//		   pCrecPrev->getBufOffset(),pCrecPrev->getBufLength(),pCrecPrev->getDocOffset());
					
					if (   (offsetBuf+lenData == pCrecPrev->getBufOffset())
						&& (offsetDoc+lenData == pCrecPrev->getDocOffset()))
					{
						// we are 1 character to the left of the previous delete
						// (both in the fim_buf and in the document).  we're
						// going to assume that the user pressed BACKSPACE
						// more than once at this location.
						// 
						// so we can glob this delete with the previous if
						// we want to.
						//
						// there are a few cases where we don't:  again, we
						// try to do the word break thing.

						const wxChar * pchLastDeleted = pFimBuf->getTemporaryPointer(pCrecPrev->getBufOffset());
						const wxChar * pchDeleted = pFimBuf->getTemporaryPointer(offsetBuf);

						if (MY_IS_WHITE(*pchDeleted) && !MY_IS_WHITE(*pchLastDeleted))	// white/non-white boundary marks
							break;

						// coalesce delete-text with previous crec.  return temporary crec
						// to allow caller to "apply" just the delta.

						pCrecPrev->expandLeftEdge(lenData);

						m_crecGlobTemp.set_text(v,offsetBuf,lenData,offsetDoc,prop);

						//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_crecvec::push_text: Did glob delete_text BACKSPACE."));
						return &m_crecGlobTemp;
					}

					if (   (offsetDoc == pCrecPrev->getDocOffset())
						&& (offsetBuf == pCrecPrev->getBufOffset()+pCrecPrev->getBufLength()))
					{
						// we are at the same document position as the previous
						// delete. and we are at the end of the buffer position.
						// we're going to assume that the user pressed DELETE
						// more than once at this location.
						//
						// so we can glob this delete with the previous if we
						// want to.
						//
						// again we try to do the word break thing.

						const wxChar * pchDeleted = pFimBuf->getTemporaryPointer(offsetBuf);
						const wxChar * pchLastDeleted = pFimBuf->getTemporaryPointer(pCrecPrev->getBufOffset()+pCrecPrev->getBufLength()-1);

						if (MY_IS_WHITE(*pchDeleted) && !MY_IS_WHITE(*pchLastDeleted))	// white/non-white boundary marks
							break;

						// coalesce delete-text with previous crec.  return temporary crec
						// to allow caller to "apply" just the delta.

						pCrecPrev->expandRightEdge(lenData);

						m_crecGlobTemp.set_text(v,offsetBuf,lenData,offsetDoc,prop);

						//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_crecvec::push_text: Did glob delete_text DELETE."));
						return &m_crecGlobTemp;
					}
				}
				break;

			default:
				wxASSERT_MSG( (0), _T("Coding Error") );
				break;
			}
		}
	}

	// not coalescable -- create a new, regular crec.

	fim_crec * pCRec = _push_crec();
	pCRec->set_text(v,offsetBuf,lenData,offsetDoc,prop);

	return pCRec;
}

const fim_crec * fim_crecvec::push_prop(fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop, fr_prop propNew)
{
	fim_crec * pCRec = _push_crec();
	pCRec->set_prop(offsetBuf,lenData,offsetDoc,prop,propNew);

	return pCRec;
}

//////////////////////////////////////////////////////////////////

const fim_crec * fim_crecvec::getUndoCRec(void)
{
	// return the crec of the first undo rec and move the undo ptr back one.
	// 
	// WE DO NOT REMOVE THIS FROM THE VEC AND THE CALLER MUST NOT DELETE IT.
	//
	// we return a TEMPORARY pointer for the CRec.

	if (!canUndo())
		return NULL;

	// think of m_cSizePos as index of next insert
	// and the first undo is to the left of it.

	const fim_crec * pRec = &m_pVec[--m_cSizePos];

	return pRec;
}

const fim_crec * fim_crecvec::getRedoCRec(void)
{
	// return the crec of the first redo rec and move the undo ptr forward one.
	//
	// WE DO NOT REMOVE THIS FROM THE VEC AND THE CALLER MUST NOT DELETE IT.
	//
	// we return a TEMPORARY pointer for the CRec.

	if (!canRedo())
		return NULL;

	// think of m_cSizePos as index of next insert
	// and the first undo is to the left of it and
	// the first redo is to the right of it.

	const fim_crec * pRec = &m_pVec[m_cSizePos++];

	return pRec;
}

//////////////////////////////////////////////////////////////////

void fim_crecvec::reset(void)
{
	// reset the entire undo stack/vector.
	// we use this to kill the undo stack
	// after loading/reloading the file
	// from disk.
	//
	// (we need this because we let the
	// load routines use the same insert/
	// delete routines as editing does.)
	//
	// since all the CREC's are in one calloc()'d
	// array, we just need to reset the indexes.
	// we don't bother realloc()ing down.

	m_cSizeTop = 0;
	m_cSizePos = 0;
	m_cSizeClean = 0;
	m_taidNext = 1;
}
