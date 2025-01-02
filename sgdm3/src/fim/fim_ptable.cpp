// fim_ptable.cpp -- piecetable -- represents document content as
// a list of fragments.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fl.h>

//////////////////////////////////////////////////////////////////

fim_ptable::fim_ptable(fim_buf * pFimBuf)
	: m_pFragHead(NULL),
	  m_pFragTail(NULL),
	  m_pPoiItem(NULL),
	  m_pFlFl(NULL),
	  m_bHadUnicodeBOM(false),
	  m_eolMode(FIM_MODE_UNSET),
	  m_autoSave(-1),
	  m_bCachedAbsoluteLengthValid(false)
{
	// only fim_ptables claim references to fim_buf's.
	// our frags do not.

	if (pFimBuf)
	{
		m_pFimBuf = pFimBuf;
		gpFimBufTable->addRef(pFimBuf);
	}
	else
	{
		m_pFimBuf = gpFimBufTable->create();
	}
}

fim_ptable::~fim_ptable(void)
{
	// destroy layout if we have one.

	DELETEP(m_pFlFl);

	// disassociate from POI
	
	if (m_pPoiItem)
	{
		wxASSERT_MSG( (m_pPoiItem->getPTable()==this), _T("Coding Error") );
		m_pPoiItem->setPTable(NULL);
	}
	
	// verify that all views on this document are gone.

	wxASSERT_MSG( (m_cblFrag.count()==0), _T("Coding Error: fim_ptable::~fim_ptable: cblFrag count > 0") );
	wxASSERT_MSG( (m_cblStat.count()==0), _T("Coding Error: fim_ptable::~fim_ptable: cblStat count > 0") );

	// now destroy the fragment list.

	DELETE_LIST(fim_frag, m_pFragHead);

	gpFimBufTable->unRef(m_pFimBuf);
}

//////////////////////////////////////////////////////////////////

fl_fl * fim_ptable::getLayout(void)
{
	// the piecetable represents the document as a series of chunks.
	// it's only goal is the correct/complete contents (w/ undo/redo)
	// as it undergoes editing.  the layout sits on top of this and
	// interprets the content in terms of rows & columns.
	//
	// since we only have one layout representation (plain text mode
	// -- as opposed to a hex-dump mode, for example),  all views on
	// this piecetable (document) can share the same layout.
	//
	// so we let the piecetable keep track of the single/shared layout.
	// 
	// we defer the creation of this until someone asks for it.  (loadFile()
	// is quicker if we defer creating the layout until after it is done.)

	if (!m_pFlFl)
		m_pFlFl = new fl_fl(this);

	return m_pFlFl;
}

//////////////////////////////////////////////////////////////////

util_error fim_ptable::loadFile(poi_item * pPoiItem,
								bool bSniffEncodingBOM,		// should we sniff for Unicode BOM
								util_encoding enc)			// only used when not sniffing
{
	// read given file into a piecetable.

	wxASSERT_MSG( (pPoiItem->getPTable()==NULL), _T("Coding Error: poi already loaded in another active ptable") );

	wxASSERT_MSG( (m_pPoiItem==NULL), _T("Coding Error: fim_ptable::loadFile: already have file (poi)") );
	wxASSERT_MSG( (m_pFragHead==NULL), _T("Coding Error: fim_ptable::loadFile: already have file (frag)") );

	pt_stat s0 = _computeStat();
	util_error ue = _loadFile(pPoiItem, bSniffEncodingBOM, enc);
	m_crecvec.reset();				// truncate undo stack (because we don't want to go back to an empty buffer)
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_resetAutoSave();

	return ue;
}

//////////////////////////////////////////////////////////////////

util_error fim_ptable::_loadFile(poi_item * pPoiItem,
								 bool bSniffEncodingBOM,	// should we sniff
								 util_encoding enc)			// only used if no sniffing
{
	// read given file into a piecetable.

	util_error ue;
	
	//////////////////////////////////////////////////////////////////
	// the piecetable support either reading the entire file in one chunk
	// and inserting it -- or -- reading and inserting in chunks.
	//
	// BUT all of the multi-byte-2-wide-char routines have a problem:  there
	// is no way for them to indicate that they are in the middle of multi-byte
	// sequence -- they either successfully convert an entire buffer or they
	// convert what they can (and return a smaller count) or the fail -- there
	// is no way for them to signal that they are in the middle of a sequence.
	// (iconv doesn't have this problem, but the wxWidgets classes don't necessarily
	// use it -- and can't return the incomplete status, even if they did.)
	// 
	// so we read the entire file into a buffer and convert it into another
	// buffer and insert it into the piecetable in one chunk.

	byte * rawBuffer = NULL;
	wxChar * wideBuffer = NULL;
	size_t lenWideBuffer = 0;

	bool bHadBOM = false;
	off_t lenBOM = 0;

	off_t lenFile;

	//////////////////////////////////////////////////////////////////
	// fetch the current date/time modified on the file.  we can use
	// this as a reference to periodically check if the file gets
	// changed by another app behind our back.

	ue = pPoiItem->getDateTimeModified(&m_dtmAsLoaded);
	m_dtmChecked = m_dtmAsLoaded;
	if (ue.isErr())
		goto Failed;

	//////////////////////////////////////////////////////////////////
	// wxWidgets file io routines spew MessageBox()'s on error -- most annoying.
	// capture error messages for display later.  use local scoping to limit
	// lifetime of log/error diversion and open and read file.

	{
		util_logToString uLog(&ue.refExtraInfo());
		wxFile file;

		// don't use wxFile::Open() because of call to wxLogSysError().

		int fd = util_file_open(ue, pPoiItem->getFullPath(),wxFile::read);
		if (fd == -1)
			goto Failed;
		file.Attach(fd);
		
#if 0
        // for some reason, under WSL, this returns 0 for
        // small files...
		lenFile = file.Length();
#else
        // ... but this works
		lenFile = file.SeekEnd(0);
        file.Seek(0);
#endif
		if (lenFile == 0)									// empty file is easy
			goto Success;

		if (bSniffEncodingBOM && (lenFile < 2))
		{
			ue.set(util_error::UE_NO_UNICODE_BOM);
			goto Failed;
		}
		
		// allocate a zeroed buffer to read the file into.  we add some padding
		// to the end because wxWidgets routines require "null" termination on buffer
		// and a "null" can require multiple bytes if we are decoding utf16/32...

		rawBuffer = (byte *)calloc((lenFile+10),sizeof(byte));
		if (!rawBuffer)
		{
			ue.set(util_error::UE_CANNOT_ALLOC);
			goto Failed;
		}

		if (bSniffEncodingBOM)
		{
			// if we should look for BOM, read first few bytes and sniff it.
			// if no BOM, no sense reading the entire file and then giving up.
			// we need 2, 3, or 4 bytes for all defined BOM's.

			off_t lenMaxBOM = MyMin(lenFile,4);
			if (file.Read(rawBuffer,lenMaxBOM) == -1)				// cannot read file ??
			{
				ue.set(util_error::UE_CANNOT_READ_FILE);			// ue.m_strExtraInfo should have something in it.
				goto Failed;
			}

			lenBOM = util_encoding_sniff_bom(rawBuffer,lenMaxBOM,&enc);
			if (lenBOM == -1)
			{
				ue.set(util_error::UE_NO_UNICODE_BOM);
				goto Failed;
			}
			if (lenFile > lenMaxBOM)	// read remainder of file into buffer
			{
				if (file.Read(rawBuffer+lenMaxBOM,lenFile-lenMaxBOM) == -1)				// cannot read file ??
				{
					ue.set(util_error::UE_CANNOT_READ_FILE);		// ue.m_strExtraInfo should have something in it.
					goto Failed;
				}
			}
			bHadBOM = true;
		}
		else	// otherwise, read entire file at once.
		{
			if (file.Read(rawBuffer,lenFile) == -1)				// cannot read file ??
			{
				ue.set(util_error::UE_CANNOT_READ_FILE);		// ue.m_strExtraInfo should have something in it.
				goto Failed;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// convert buffer from whatever disk encoding into unicode
	// and insert into our document.  (if there was a BOM, skip
	// past it.)

	if (lenFile > lenBOM)	// file has data (not empty or just BOM)
	{
		ue = util_encoding_does_buffer_have_nulls(enc, rawBuffer+lenBOM, lenFile-lenBOM);
		if (ue.isErr())
			goto Failed;

		ue = _import_conv(enc, rawBuffer+lenBOM, &wideBuffer,&lenWideBuffer);
		if (ue.isErr())
			goto Failed;

		_insertText(0,wideBuffer,lenWideBuffer,FR_PROP_ZERO);
	}

Success:
	//////////////////////////////////////////////////////////////////
	// taste the content and try to determine what type of EOL's it has.
	// that is, see if this is a Windows or Linux or MAC type of file.
	// if we don't have enough data to determine the line type, we'll
	// leave it set to _UNSET.

	m_eolMode = _guessEolMode();
	
	//////////////////////////////////////////////////////////////////
	// remember the encoding and whether we had a BOM.

	m_bHadUnicodeBOM = bHadBOM;
	m_enc = enc;

	//////////////////////////////////////////////////////////////////
	// everything worked, clean up

	FREEP(wideBuffer);
	FREEP(rawBuffer);

	if (m_pPoiItem)
	{
		wxASSERT_MSG( (m_pPoiItem == pPoiItem), _T("Coding Error") );
		wxASSERT_MSG( (m_pPoiItem->getPTable()==this), _T("Coding Error") );
	}
	else
	{
		m_pPoiItem = pPoiItem;
		m_pPoiItem->setPTable(this);				// let poi know that we are the official piecetable for it.
	}
	
	//wxLogTrace(TRACE_FIM_DUMP,_T("fim_ptable::loadFile[%s]: Succeeded"),pPoiItem->getFullPath().wc_str());
	wxASSERT_MSG( (ue.isOK()), _T("Coding Error") );
	return ue;

Failed:
	FREEP(wideBuffer);
	FREEP(rawBuffer);

//	wxLogTrace(TRACE_FIM_DUMP,_T("fim_ptable::loadFile[%s]: Failed [%s]"),
//			   util_printable_s(pPoiItem->getFullPath()).wc_str(),
//			   util_printable_s(ue.getExtraInfo()).wc_str());
	wxASSERT_MSG( (ue.isErr()), _T("Coding Error") );
	return ue;
}

//////////////////////////////////////////////////////////////////

util_error fim_ptable::reload(void)
{
	// re-read file into piecetable.

	wxASSERT_MSG( (m_pPoiItem), _T("Coding Error") );

	pt_stat s0 = _computeStat();
	util_error ue = _reload();
	m_crecvec.reset();				// truncate undo stack (because we don't want to go back to an empty buffer)
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_resetAutoSave();

	return ue;
}

void fim_ptable::_delete_all(void)
{
	// delete the entire contents of the document in preparation for
	// reloading -- we also kill the undo-stack and raw-buffer-info,
	// so this is not recoverable (can't undo this step).

	m_crecvec.reset();		// truncate undo stack before we begin (avoids unnecessary realloc()'s during what follows)

	// delete the current contents first

	while (m_pFragHead)
		_deleteText(0,m_pFragHead->getFragLength());

	m_crecvec.reset();		// truncate undo stack before we begin (avoids unnecessary realloc()'s during what follows)

	m_pFimBuf->resetBuffer();	// truncate raw-buffer as we no longer have any references to any of the content.
}

util_error fim_ptable::_reload(void)
{
	// re-read file into piecetable.

	_delete_all();

	// TODO Reload() causes the file to be re-read from disk.  Should we re-evaluate
	// TODO our choice of character encoding?  That is, should we re-sniff and see
	// TODO if the BOM changed and/or see if the ruleset encoding values changed
	// TODO and maybe use a different encoding here (or (re-)ask them) what to do?
	// TODO
	// TODO for now, we just reload using the original encoding (which may or may not
	// TODO be correct) -- if they want to change encoding on the fly, let them close
	// TODO the window and re-open it.
	// TODO
	// TODO i don't mind doing something smarter here -- if they explicitly hit File|Reload
	// TODO but if we auto-reloading because of window activation, we don't want to
	// TODO bother them -- especially if it was because a dialog disappeared...
	// TODO
	// TODO we should probably push this question up to the FS level -- since it's the
	// TODO only level that knows why this particular encoding was chosen and knows 
	// TODO the state of the ruleset with relation to what it was when we were first
	// TODO loaded.
	
	return _loadFile(m_pPoiItem,m_bHadUnicodeBOM,m_enc);
}

//////////////////////////////////////////////////////////////////

util_error fim_ptable::_import_conv(util_encoding enc, const byte * pBufSrc, wxChar ** ppWideBuffer, size_t * pLenWideBuffer)
{
	// convert raw, multi-byte encoded data in source buffer into unicode and insert in document.
	// 
	// WARNING: we avoid the auto-converting constructor in wxString because it's not done yet.
	// 
	// return true if everything went well.

	//////////////////////////////////////////////////////////////////
	// see discussion in util_enc on wxFONTENCODING_SYSTEM and _DEFAULT.
	// passing _DEFAULT to the wxCSConv's ctor causes an assert.  it
	// then substitutes _SYSTEM.  the following is to avoid the assert
	// message box.
	//////////////////////////////////////////////////////////////////

	util_encoding encNormalized = ((enc == wxFONTENCODING_DEFAULT) ? wxFONTENCODING_SYSTEM : enc);
	util_error ue;
	wxMBConv * pConv = NULL;
	
	//////////////////////////////////////////////////////////////////
	// wxCSConv's ctor silently fails if it cannot create a converter
	// for the named encoding (like EUC-JP when it's not installed on
	// Win32).  [on Win32 at least, the converter is lazily created,
	// so it doesn't test for a valid conversion until it tries to use
	// it.]
	//
	// either way, it silently falls back to iso-8859-1 without telling
	// us.  ***AND*** there's no documented routine to tell us that this
	// has happened.
	// 
	// it also queues up an error in wxWidget's error logger that gets
	// displayed as a message-box the next time we go idle.
	// 
	// so the user sees that we've loaded the document (incorrectly
	// using 8859-1) and diff'd it and everything --- AND THEN --- they
	// get a message about us not understanding the encoding.  This is
	// really bogus.
	//
	// so caller needs to divert the error log around the ctor and check for
	// log activity.  (SIGH)
	//////////////////////////////////////////////////////////////////

	{
		util_logToString uLog(&ue.refExtraInfo());

		pConv = util_encoding_create_conv(encNormalized);		// we need to delete this

		size_t lenNeeded = pConv->MB2WC(NULL,(const char *)pBufSrc,0);

		if (ue.getExtraInfo().Length() > 0)
		{
//			wxLogTrace(TRACE_PTABLE_DUMP,
//					   _T("fim_ptable::_import_conv: ERROR for [%s] failed [%s]"),
//					   wxFontMapper::GetEncodingName(encNormalized).wc_str(),
//					   ue.getExtraInfo().wc_str());
			ue.set(util_error::UE_CANNOT_IMPORT_CONV,
				   wxString::Format(_("Invalid bytes in file for character encoding '%s'."),
									wxFontMapper::GetEncodingDescription(encNormalized).wc_str()));
			goto Finished;
		}

		//////////////////////////////////////////////////////////////////
		// wxWidgets doesn't document the error return values for MB2WC.
		// looking at the source, sometimes it is -1 and sometimes it is 0.
		// this varies depending upon which subsystem (wxWidgets' wxMBConvUTF8(),
		// Linux's mbstowcs(), or Win32's MultiByteToWideChar() ....)
		//
		// WARNING: "size_t" is defined as "unsigned int" or "unsigned long" so tests
		// WARNING: for (lenNeeded < 0) aren't going to work...
		// 
		// so i'm going to let it bail if we get either.  (there's an odd chance that
		// this will cause it puke on a UTF16 file that only has a BOM, but we should
		// have caught that in the caller.)
		//////////////////////////////////////////////////////////////////

		if ( (lenNeeded == 0) || (lenNeeded == (size_t)-1) )
		{
//			wxLogTrace(TRACE_PTABLE_DUMP, _T("fim_ptable::_import_conv: ERROR [len %ld][type %s]"),
//					   (long)lenNeeded,
//					   wxFontMapper::GetEncodingName(encNormalized).wc_str());
			ue.set(util_error::UE_CANNOT_IMPORT_CONV,
				   wxString::Format(_("Invalid bytes in file for character encoding '%s'."),
									wxFontMapper::GetEncodingDescription(encNormalized).wc_str()));
			goto Finished;
		}

		wxChar * pWideBuffer = (wxChar *)calloc((lenNeeded+1),sizeof(wxChar));
		size_t lenUsed = pConv->MB2WC(pWideBuffer,(const char *)pBufSrc,(lenNeeded+1));
		wxASSERT_MSG( (lenUsed==lenNeeded), _T("Coding Error: conv"));

//		wxLogTrace(TRACE_PTABLE_DUMP, _T("fim_ptable::_import_conv:  [len %ld][type %s]"),
//				   (long)lenUsed,
//				   wxFontMapper::GetEncodingName(encNormalized).wc_str());

		*ppWideBuffer = pWideBuffer;
		*pLenWideBuffer = lenUsed;
	}

Finished:
	DELETEP(pConv);
	return ue;
}

//////////////////////////////////////////////////////////////////

void fim_ptable::insertText(fim_offset docPos, fr_prop prop, const wxString & pData)
{
	insertText(docPos,prop,pData.wc_str(),pData.Len());
}

void fim_ptable::insertText(fim_offset docPos, fr_prop prop, const wxChar * pData)
{
	insertText(docPos,prop,pData,wxStrlen(pData));
}

void fim_ptable::insertText(fim_offset docPos, fr_prop prop, const wxChar * pData, fim_length lenData)
{
	pt_stat s0 = _computeStat();
	_insertText(docPos,pData,lenData,prop);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();
}

void fim_ptable::_insertText(fim_offset docPos,
							 const wxChar * pData, fim_length lenData,
							 fr_prop prop)
{
	// insert the contents of the given buffer (pData,lenData)
	// into the document using an absolute document position.

	if (lenData == 0)
		return;

	// put raw buffer into growbuf so that it can be referenced by a frag.

	fb_offset offsetBuf;
	m_pFimBuf->append(pData,lenData,&offsetBuf);

	// push a change record to describe this entire operation
	// apply the change record to the document and affect the change
	
	const fim_crec * pCRec = m_crecvec.push_text(fim_crec::verb_text_insert,
												 offsetBuf,lenData,docPos,
												 prop,
												 m_pFimBuf);
	_applyCRec_text_insert(pCRec);
}

//////////////////////////////////////////////////////////////////

void fim_ptable::appendText(fr_prop prop, const wxChar * pData, fim_length lenData)
{
	pt_stat s0 = _computeStat();
	_appendText(prop,pData,lenData);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();
}

void fim_ptable::_appendText(fr_prop prop, const wxChar * pData, fim_length lenData)
{
	fim_offset offsetDoc = getAbsoluteOffset(m_pFragTail,
											 ((m_pFragTail) ? m_pFragTail->getFragLength() : 0));

	_insertText(offsetDoc,pData,lenData,prop);
}

//////////////////////////////////////////////////////////////////

void fim_ptable::deleteText(fim_offset docPos, fim_length lenData)
{
	pt_stat s0 = _computeStat();
	_deleteText(docPos,lenData);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();
}

void fim_ptable::_deleteText(fim_offset docPos, fim_length lenData)
{
	// delete lenData-worth of the document content starting at
	// the given absolute document offset.  this may span multiple
	// fragments.

	if (lenData == 0)
		return;

	fim_frag * pFrag;
	fr_offset offsetFrag;
	
	getFragAndOffset(docPos,&pFrag,&offsetFrag);

	// see if the entire content is contained within one frag.  if so, we can
	// post a simple delete.  otherwise, we need to bracket a multi-frag delete.
	// (we need to cut it up into multiple frags only because we want to preserve
	// the props on each frag; if we didn't care about this, we could let _applyCRec
	// deal with removing all the content)

	if (offsetFrag+lenData <= pFrag->getFragLength())
	{
		// simple case.  push a single change record to describe this entire operation
		// and apply the change record to the document and affect the change.
	
		const fim_crec * pCRec = m_crecvec.push_text(fim_crec::verb_text_delete,
													 pFrag->getBufOffset(offsetFrag),lenData,docPos,
													 pFrag->getFragProp(),
													 m_pFimBuf);
		_applyCRec_text_delete(pCRec,pFrag,offsetFrag);

		return;
	}

	// begin ta bracket (so that undo will treat all of our steps as one)
	// WARNING: if we call beginTransaction(), we must not return without calling endTransaction().

	fim_crec::TAID taidBegin = beginTransaction();

	while (1)
	{
		fim_length lenInThisFrag = pFrag->getFragLength() - offsetFrag;
		if (lenInThisFrag > lenData)
			lenInThisFrag = lenData;
		
		const fim_crec * pCRec = m_crecvec.push_text(fim_crec::verb_text_delete,
													 pFrag->getBufOffset(offsetFrag),lenInThisFrag,docPos,
													 pFrag->getFragProp(),
													 m_pFimBuf);
		_applyCRec_text_delete(pCRec,pFrag,offsetFrag);

		lenData -= lenInThisFrag;
		if (lenData == 0)
			break;

		// WARNING: _applyCRec will alter the fragment list (by outright deleting
		// WARNING: pFrag, splitting it, and/or coalescing around the deletion point.
		// WARNING: therefore, we must re-compute (frag,offsetFrag) using the absolute
		// WARNING: document offset (since it won't have changed).  this is a little
		// WARNING: expensive -- but is easier than trying to guess what happened to
		// WARNING: the list.

		getFragAndOffset(docPos,&pFrag,&offsetFrag);

		wxASSERT_MSG( ((pFrag) && (offsetFrag < pFrag->getFragLength())), _T("Coding Error: fim_ptable::deleteText: secondary portion out-of-wack") );
	}
			
	endTransaction(taidBegin);

	return;
}

//////////////////////////////////////////////////////////////////

void fim_ptable::replaceText(fim_offset docPos, fim_length lenOldData,
							 const wxString & pData, fr_prop prop)
{
	replaceText(docPos,lenOldData,pData.wc_str(),pData.Length(),prop);
}

void fim_ptable::replaceText(fim_offset docPos, fim_length lenOldData,
							 const wxChar * pData, fim_length lenNewData,
							 fr_prop prop)
{
	pt_stat s0 = _computeStat();
	_replaceText(docPos,lenOldData,pData,lenNewData,prop);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();
}

void fim_ptable::_replaceText(fim_offset docPos, fim_length lenOldData,
							  const wxChar * pData, fim_length lenNewData,
							  fr_prop prop)
{
	// begin ta bracket (so that undo will treat all of our steps as one)
	// WARNING: if we call beginTransaction(), we must not return without calling endTransaction().

	fim_crec::TAID taidBegin = beginTransaction();

	if (lenOldData)
		_deleteText(docPos,lenOldData);

	if (lenNewData)
		_insertText(docPos,pData,lenNewData,prop);

	endTransaction(taidBegin);
}

//////////////////////////////////////////////////////////////////

void fim_ptable::applyPatchSet(fim_patchset * pPatchSet)
{
	wxASSERT_MSG( (m_crecvec.canAutoMerge()), _T("Coding Error!") );
	
	// use the patch-set to apply multiple patches/changes under one transaction.
	// WARNING: we assume that the set was created in reverse order (the first
	// WARNING: patch should represent the last change in the file) -- this is
	// WARNING: required so that absolute document-offsets don't change for
	// WARNING: subsequent patches in the set.

	pt_stat s0 = _computeStat();
	_applyPatchSet(pPatchSet);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();
}

void fim_ptable::_applyPatchSet(fim_patchset * pPatchSet)
{
	// begin ta bracket (so that undo will treat all of our steps as one)
	// WARNING: if we call beginTransaction(), we must not return without calling endTransaction().

	fim_crec::TAID taidBegin = beginTransaction();

	long nrPatches = pPatchSet->getNrPatches();
	wxASSERT_MSG( (nrPatches > 0), _T("Coding Error") );
	
	for (long kPatch=0; (kPatch<nrPatches); kPatch++)
	{
		fim_patch * pPatch = pPatchSet->getNthPatch(kPatch);

		switch (pPatch->getPatchOpCurrent())
		{
		default:
			wxASSERT_MSG( (0), _T("Coding Error") );
			break;

		case POP_IGNORE:		// user said to omit it.
			break;

		case POP_DELETE:
			_deleteText(pPatch->getDocPosWhere(),pPatch->getLenDelete());
			break;
			
		case POP_INSERT_L:
		case POP_INSERT_R:
			_insertText(pPatch->getDocPosWhere(),pPatch->getNewText(),pPatch->getNewTextLen(),pPatch->getProp());
			break;

		case POP_REPLACE_L:
		case POP_REPLACE_R:
			_deleteText(pPatch->getDocPosWhere(),pPatch->getLenDelete());
			_insertText(pPatch->getDocPosWhere(),pPatch->getNewText(),pPatch->getNewTextLen(),pPatch->getProp());
			break;

		case POP_CONFLICT:		// user chose not to deal with it.
			break;
		}
	}

	endTransaction(taidBegin);
	
}

//////////////////////////////////////////////////////////////////
#if 0	// it works, but we don't need it right now.
fim_length fim_ptable::measureText(const fim_frag * pFragBegin, fr_offset offsetBegin,
								   const fim_frag * pFragEnd, fr_offset offsetEnd) const
{
//	wxLogTrace(TRACE_FIM_DUMP,_T("fim_ptable::measureText: [%p,%ld][%p,%ld]"),
//			   pFragBegin,offsetBegin, pFragEnd,offsetEnd);

	wxASSERT_MSG( (m_pFragHead), _T("CodingError") );
	if (!m_pFragHead)				// should not happen
		return 0;
	
	if (!pFragBegin)				// if not given, assume BOF
	{
		pFragBegin = m_pFragHead;
		offsetBegin = 0;
	}

	if (!pFragEnd)					// if not given, assume EOF
	{
		pFragEnd = m_pFragTail;
		offsetEnd = m_pFragTail->getFragLength();
	}
	
	if (pFragBegin == pFragEnd)
	{
		if (offsetBegin < offsetEnd)
			return offsetEnd - offsetBegin;
		else
			return 0;
	}

	fim_length sum = 0;
	const fim_frag * pFrag;

	for (pFrag=pFragBegin; (pFrag && (pFrag != pFragEnd)); pFrag=pFrag->m_next)
	{
		sum += (pFrag->getFragLength() - offsetBegin);
		offsetBegin = 0;
	}

	wxASSERT_MSG( (pFrag), _T("Coding Error") );	// pFragBegin was after pFragEnd in document
	if (!pFrag)						// should not happen
		return 0;

	sum += offsetEnd;

//	wxLogTrace(TRACE_FIM_DUMP,_T("fim_ptable::measureText: yields [%ld]"),sum);
	
	return sum;
}
#endif
//////////////////////////////////////////////////////////////////

void fim_ptable::clone(const fim_ptable * pPTableSrc, fr_prop propMask)
{
	// populate our ptable by cloning the content of the given one.

	pt_stat s0 = _computeStat();
	_clone(pPTableSrc,propMask);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_resetAutoSave();
}

void fim_ptable::_clone(const fim_ptable * pPTableSrc, fr_prop propMask)
{
	// populate our ptable by cloning the content of the given one.
	// this creates a new and independently editable document that
	// initially contains a copy of the given one.
	//
	// propMask contains the bits for any per-frag props that we should
	// bring along -- at this level, we don't know what the individual
	// prop-bits mean, we just AND-OFF the zeros.

	wxASSERT_MSG( (m_pFragHead==NULL), _T("Coding Error: fim_ptable::_clone: already have file (frag)") );

	m_crecvec.reset();				// truncate undo stack (because we don't want to go back to an empty buffer)

	for (const fim_frag * pFrag=pPTableSrc->getFirstFrag(); pFrag; pFrag=pFrag->getNext())
		_appendText( (pFrag->getFragProp() & propMask), pFrag->getTemporaryDataPointer(), pFrag->getFragLength() );

	m_enc = pPTableSrc->getEncoding();
	m_bHadUnicodeBOM = pPTableSrc->getHadUnicodeBOM();
	m_eolMode = pPTableSrc->getEolMode();

	m_crecvec.reset();				// truncate undo stack (because we don't want to go back to an empty buffer)
}

//////////////////////////////////////////////////////////////////

void fim_ptable::re_clone(const fim_ptable * pPTableSrc, fr_prop propMask)
{
	// re-clone -- delete the content of this (edit-buffer) document
	// and recreate it by cloning the src-buffer.  we use this when
	// reloading the src-buffer from disk and making the edit-buffer
	// match the newly loaded content.
	//
	// this is basically a delete-all() and clone(), but does some
	// extra stuff to conserve memory and minimize broadcasts.

	pt_stat s0 = _computeStat();
	_re_clone(pPTableSrc,propMask);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_resetAutoSave();
}

void fim_ptable::_re_clone(const fim_ptable * pPTableSrc, fr_prop propMask)
{
	_delete_all();

	_clone(pPTableSrc,propMask);
}

//////////////////////////////////////////////////////////////////
// set/clear one or more property bits on a span of text.

void fim_ptable::turnOnProp(fim_frag * pFrag, fr_offset offsetFrag, fim_length lenData, fr_prop prop)
{
	pt_stat s0 = _computeStat();
	_turnOnOrOffProp(true,pFrag,offsetFrag,lenData,prop);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();
}

void fim_ptable::turnOffProp(fim_frag * pFrag, fr_offset offsetFrag, fim_length lenData, fr_prop prop)
{
	pt_stat s0 = _computeStat();
	_turnOnOrOffProp(false,pFrag,offsetFrag,lenData,prop);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();
}

void fim_ptable::_turnOnOrOffProp(bool bOn, fim_frag * pFrag, fr_offset offsetFrag, fim_length lenData, fr_prop prop)
{
	// if (bOn): OR on bits set in prop given onto lenData-worth of document content starting at the given
	//           fragment-relative address (pFrag,offsetFrag).  this may span multiple fragments.
	// if (!bOn): AND off bits set in prop given....

	if (lenData == 0)
		return;

	_fixupFragAndOffset(&pFrag,&offsetFrag);

	// compute absolute document offset of (pFrag,offsetFrag) for undo/redo

	fim_offset offsetDoc = getAbsoluteOffset(pFrag,offsetFrag);

	// see if the entire content is contained within one frag.  if so, we can
	// post a simple operation (which may do multiple frag ops, but we don't
	// care about that at this level).  if not, we have to do the transaction
	// thing -- we need to cut it up into multiple frags only because we want
	// to preserve the old props on each frag.
	//
	// we do try to short-circuit this -- if all the bits in the mask given (prop)
	// are already on/off in the frag, we don't need to do anything.

#define ON_DIFFERENT(mask,current)		(((current) & (mask)) != (mask))
#define OFF_DIFFERENT(mask,current)		(((current) & (mask)) != (0))
#define DIFFERENT(b,mask,current)		((b) ? ON_DIFFERENT(mask,current) : OFF_DIFFERENT(mask,current))

#define ON_NEW_PROP(mask,current)		((current) | (mask))
#define OFF_NEW_PROP(mask,current)		((current) & ~(mask))
#define NEW_PROP(b,mask,current)		((b) ? ON_NEW_PROP(mask,current) : OFF_NEW_PROP(mask,current))

	if (offsetFrag+lenData <= pFrag->getFragLength())
	{
		fr_prop propCurrent = pFrag->getFragProp();
		
		if (DIFFERENT(bOn,prop,propCurrent))
		{
			// simple case.  push a single change record to describe this entire operation
			// and apply the change record to the document and affect the change.
	
			const fim_crec * pCRec = m_crecvec.push_prop(pFrag->getBufOffset(offsetFrag),lenData,offsetDoc,
														 propCurrent, NEW_PROP(bOn,prop,propCurrent));
			_applyCRec_prop(pCRec,pFrag,offsetFrag);
		}
		
		return;
	}

	// begin ta bracket (so that undo will treat all of our steps as one)
	// WARNING: if we call beginTransaction(), we must not return without calling endTransaction().

	fim_crec::TAID taidBegin = beginTransaction();

	while (1)
	{
		fim_length lenInThisFrag = pFrag->getFragLength() - offsetFrag;
		if (lenInThisFrag > lenData)
			lenInThisFrag = lenData;

		fr_prop propCurrent = pFrag->getFragProp();
		
		if (DIFFERENT(bOn,prop,propCurrent))
		{
			const fim_crec * pCRec = m_crecvec.push_prop(pFrag->getBufOffset(offsetFrag),lenInThisFrag,offsetDoc,
														 propCurrent, NEW_PROP(bOn,prop,propCurrent));
			_applyCRec_prop(pCRec,pFrag,offsetFrag);
		}
		
		lenData -= lenInThisFrag;
		if (lenData == 0)
			break;

		// WARNING: _applyCRec will alter the fragment list, so we need to re-find our place.

		offsetDoc += lenInThisFrag;
		
		getFragAndOffset(offsetDoc,&pFrag,&offsetFrag);

		wxASSERT_MSG( ((pFrag) && (offsetFrag < pFrag->getFragLength())), _T("Coding Error: fim_ptable::_turnOnProp: secondary portion out-of-wack") );
	}

	endTransaction(taidBegin);

	// TODO it is possible if all of the frags already had set all of the bits in the
	// TODO mask, then our short-circuiting will have resulted in an empty transaction.
	// TODO do we want to roll it back now -- or would that confuse the user if they
	// TODO hit the undo button and it undid the previous change....

	return;

}

//////////////////////////////////////////////////////////////////
// expose transaction blocking to caller -- let them do some multi-step
// operations that we'll treat as atomic -- like auto-merge.
// callers must balance these calls:
//    taid = begin();
//    {...}
//    end(taid);

fim_crec::TAID fim_ptable::beginTransaction(void)
{
	// we don't bother applying ta markers since there's nothing for it to do.
	// we don't bother sending change callback, since there's no real change.
	// we *DO* try to send a status change callback because the undo stack has advanced.

	pt_stat s0 = _computeStat();
	fim_crec::TAID taid = m_crecvec.push_ta_begin()->getTAID();
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	return taid;
}

void fim_ptable::endTransaction(fim_crec::TAID taid)
{
	// we don't bother applying ta markers since there's nothing for it to do.
	// we don't bother sending change callback, since there's no real change.
	// we *DO* try to send a status change callback because the undo stack has advanced.

	pt_stat s0 = _computeStat();
	m_crecvec.push_ta_end(taid);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status
}

//////////////////////////////////////////////////////////////////
// undo/redo involve unapplying/reapplying a change record.

bool fim_ptable::undo(fim_offset * pDocPos)
{
	pt_stat s0 = _computeStat();
	bool bResult = _undo(pDocPos);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();

	return bResult;
}

bool fim_ptable::_undo(fim_offset * pDocPos)
{
	// perform an UNDO operation on the current document.
	//
	// return true iff we have a suggestion for where
	// the caret should be afterwards.  this is problematic
	// since we may have either a simple insert/delete or
	// we may have a complex/nested sequence of disjoint
	// changes (such as the undo of the auto-merge).

	if (!m_crecvec.canUndo())
		return false;

	// fetch top crec and invert it

	fim_crec crecUndo( m_crecvec.getUndoCRec() );
	crecUndo.reverse();

	if (crecUndo.getVerb() != fim_crec::verb_ta_begin)
	{
		_applyCRec(&crecUndo);	// a simple single-step change
		if (crecUndo.isText() || crecUndo.isProp())
		{
			*pDocPos = crecUndo.getDocOffset();
			return true;
		}
		return false;
	}

	// otherwise, the crec is the beginning of a transaction block.
	// we must apply all crec's thru the ta-end for this taid.
	// (remember ta's can be nested so we need to match taid's)

	bool bSetDocPos = false;

	fim_crec::TAID taidStop = crecUndo.getTAID();
	while (1)
	{
		fim_crec crecUndo2( m_crecvec.getUndoCRec() );
		crecUndo2.reverse();

		if ((crecUndo2.getVerb() == fim_crec::verb_ta_end) && (crecUndo2.getTAID() == taidStop))
			return bSetDocPos;

		_applyCRec(&crecUndo2);
		if (crecUndo2.isText() || crecUndo2.isProp())
		{
			*pDocPos = crecUndo2.getDocOffset();
			bSetDocPos = true;
		}
	}

	/*NOTREACHED*/
	return false;
}

//////////////////////////////////////////////////////////////////

bool fim_ptable::redo(fim_offset * pDocPos)
{
	pt_stat s0 = _computeStat();
	bool bResult = _redo(pDocPos);
	pt_stat s1 = _computeStat();

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	_considerAutoSave();

	return bResult;
}

bool fim_ptable::_redo(fim_offset * pDocPos)
{
	if (!m_crecvec.canRedo())
		return false;

	const fim_crec * pCRecRedo = m_crecvec.getRedoCRec();

	if (pCRecRedo->getVerb() != fim_crec::verb_ta_begin)
	{
		_applyCRec(pCRecRedo);	// a simple single-step change
		if (pCRecRedo->isText() || pCRecRedo->isProp())
		{
			*pDocPos = pCRecRedo->getDocOffset();
			return true;
		}
		return false;
	}
	
	// otherwise, the crec is the beginning of a transaction block.
	// we must apply all crec's thru the ta-end for this taid.
	// (remember ta's can be nested so we need to match taid's)

	bool bSetDocPos = false;

	fim_crec::TAID taidStop = pCRecRedo->getTAID();
	while (1)
	{
		const fim_crec * pCRecRedo2 = m_crecvec.getRedoCRec();

		if ((pCRecRedo2->getVerb() == fim_crec::verb_ta_end) && (pCRecRedo2->getTAID() == taidStop))
			return bSetDocPos;

		_applyCRec(pCRecRedo2);
		if (pCRecRedo2->isText() || pCRecRedo2->isProp())
		{
			*pDocPos = pCRecRedo2->getDocOffset();
			bSetDocPos = true;
		}
	}

	/*NOTREACHED*/
	return false;
}

//////////////////////////////////////////////////////////////////
// all externally generated manipulations (insertText, deleteText, etc)
// get turned into one or more Change-Records (crec's) that then get 'applied'.
//
// these crec's go into the  Undo/Redo history so that we may un-play them
// and revert the document or re-play them and redo the changes.
//
// externally generated changes never directly change the document;
// they always go thru a crec for consistency.

void fim_ptable::_applyCRec(const fim_crec * pCRec)
{
	// apply this change record to the document.

	switch (pCRec->getVerb())
	{
	default:							// should not happen (silences compiler warnings)
	case fim_crec::verb_invalid:		// should not happen
		wxASSERT_MSG( (0), _T("Coding Error: fim_ptable::_applyCRec: invalid verb."));
		return;
		
	case fim_crec::verb_ta_begin:		// this does not change the doc, it's just bracketing for undo/redo
	case fim_crec::verb_ta_end:			// this does not change the doc, it's just bracketing for undo/redo
		return;

	case fim_crec::verb_text_insert:
		_applyCRec_text_insert(pCRec);
		return;

	case fim_crec::verb_text_delete:
		_applyCRec_text_delete(pCRec);
		return;

	case fim_crec::verb_prop_set:
	case fim_crec::verb_prop_set2:
		_applyCRec_prop(pCRec);
		return;
	}
}

void fim_ptable::_applyCRec_text_insert(const fim_crec * pCRec, fim_frag * pFragHint, fr_offset offsetFragHint)
{
	// apply an insertText change record to the document.

	wxASSERT_MSG( (pCRec->getVerb()==fim_crec::verb_text_insert), _T("Coding Error: fim_ptable::_applyCRec_text_insert: wrong crec type") );

	// the crec is self contained -- that is, it completely describes
	// what needs to be done and where.  the frag and offset-in-the-frag
	// is a hint.  when given, it means the caller already knows where
	// the location of absolute-document-offset in the crec.

	fim_frag * pFrag     = NULL;
	fr_offset offsetFrag = 0;

	_getFragAndOffsetFromHintOrDocOffset(pFragHint,offsetFragHint,
										 pCRec->getDocOffset(),
										 &pFrag,&offsetFrag);

	// create a new fragment for the new text and insert it into
	// the fragment list and then try to coalesce frags around the
	// insertion point.  (we could try to see if the new text would
	// coalesce and adjust the frags without allocating and inserting
	// a new one first (like when undoing what's effectively a ltrim/rtrim)
	// but that a little messy -- and increases the set of notification
	// types we need to broadcast.)

	fim_length workingLength = pCRec->getBufLength();

	fim_frag * pFragNew = new fim_frag(m_pFimBuf,
									   pCRec->getBufOffset(),
									   workingLength,
									   pCRec->getProp());

	if (m_bCachedAbsoluteLengthValid)
		m_cachedAbsoluteLength += workingLength;

	if (!pFrag)										// first fragment in an empty document
	{
		_linkNewFrag(pFragNew);						// insert new frag
		return;
	}

	if (offsetFrag == 0)							// at left edge -- prepend
	{
		_linkNewFragBeforeFrag(pFrag,pFragNew);		// insert new frag before this frag and try to coalesce
		return;
	}

	if (offsetFrag == pFrag->getFragLength())		// at right edge -- append
	{
		_linkNewFragAfterFrag(pFrag,pFragNew);		// insert new frag after this frag and try to coalesce
		return;
	}

	if (offsetFrag < pFrag->getFragLength())		// in middle of frag
	{
		_split(pFrag,offsetFrag);					// split frag into 2 parts (and NO coalescing)
		_linkNewFragAfterFrag(pFrag,pFragNew);		// insert new frag between parts and try to coalesce
		return;
	}

	// offsetFrag > fragment length

	wxASSERT_MSG( (0), _T("Coding Error: fim_ptable::_applyCRec_text_insert: insert & offset not within frag") );
	delete pFragNew;
	return;
}

void fim_ptable::_applyCRec_text_delete(const fim_crec * pCRec, fim_frag * pFragHint, fr_offset offsetFragHint)
{
	// apply this change record to the document.

	wxASSERT_MSG( (pCRec->getVerb()==fim_crec::verb_text_delete), _T("Coding Error: fim_ptable::_applyCRec_text_delete: wrong crec type") );

	// the crec is self contained -- that is, it completely describes
	// what needs to be done and where.  the frag and offset-in-the-frag
	// is a hint.  when given, it means the caller already knows where
	// the location of absolute-document-offset in the crec.

	fim_frag * pFrag     = NULL;
	fr_offset offsetFrag = 0;

	_getFragAndOffsetFromHintOrDocOffset(pFragHint,offsetFragHint,
										 pCRec->getDocOffset(),
										 &pFrag,&offsetFrag);

	// cause the content represented by this crec to be removed from the fragment list.

	wxASSERT_MSG( (offsetFrag <= pFrag->getFragLength()),
				  _T("Coding Error: fim_ptable::_applyCRec: delete & offset not within frag") );

	if (offsetFrag > 0)										// if we begin in the middle of a fragment
	{
		_split(pFrag,offsetFrag);							// split frag into 2 parts (the left half we keep) (and NO coalescing)

		pFrag = pFrag->m_next;								// advance to right half (where we'll start deleting)
		offsetFrag = 0;
	}
		
	fim_length workingLength = pCRec->getBufLength();

	if (m_bCachedAbsoluteLengthValid)
		m_cachedAbsoluteLength -= workingLength;

	while (workingLength)
	{
		fim_length lenFrag   = pFrag->getFragLength();

		if (workingLength < lenFrag)						// effectively an ltrim on this frag and we're done
		{
			_split(pFrag,workingLength);					// split frag into 2 parts (and NO coalescing)
			_unlinkFrag(pFrag);								// delete the left half (and coalesce)
			return;
		}

		if (workingLength == lenFrag)						// exact match on this frag; delete it and we're done
		{
			_unlinkFrag(pFrag);								// delete this frag completely (and coalesce)
			return;
		}

		// need all of this frag and then some
		// TODO this case might not be necessary since i put the
		// TODO code in deleteText() to cut it up into fragments.
		// TODO if it is necessary, deal with coalescing
//		wxLogTrace(TRACE_PTABLE_DUMP, _T("fim_ptable::_applyCRec: delete spanning multiple fragments [%ld][%ld]"), workingLength,lenFrag);
		wxASSERT_MSG( 0, _T("delete spanning multiple fragments -- TODO") );

		// fim_frag * pFragNext = pFrag->m_next;
		// _unlinkFrag(pFrag,FOP_DELETE_SELF,false);			// delete this frag -- BUT DO NOT COALESCE -- because we're walking the list
		// workingLength -= lenFrag;
		// pFrag = pFragNext;
	}
}

void fim_ptable::_applyCRec_prop(const fim_crec * pCRec, fim_frag * pFragHint, fr_offset offsetFragHint)
{
	// apply this change record to the document.

	wxASSERT_MSG( ((pCRec->getVerb()==fim_crec::verb_prop_set) || (pCRec->getVerb()==fim_crec::verb_prop_set2)),
				  _T("Coding Error: fim_ptable::_applyCRec_text_delete: wrong crec type") );

	// the crec is self contained -- that is, it completely describes
	// what needs to be done and where.  the frag and offset-in-the-frag
	// is a hint.  when given, it means the caller already knows where
	// the location of absolute-document-offset in the crec.

	fim_frag * pFrag     = NULL;
	fr_offset offsetFrag = 0;

	_getFragAndOffsetFromHintOrDocOffset(pFragHint,offsetFragHint,
										 pCRec->getDocOffset(),
										 &pFrag,&offsetFrag);


	if (offsetFrag > 0)										// see if we begin in the middle of a fragment
	{
		_split(pFrag,offsetFrag);							// split frag into 2 parts (the left half we keep) (and NO coalescing)

		pFrag = pFrag->m_next;								// advance to right half (where we'll start modifying)
		offsetFrag = 0;
	}
		
	fim_length workingLength = pCRec->getBufLength();
	while (workingLength)
	{
		fim_length lenFrag   = pFrag->getFragLength();

		if (workingLength < lenFrag)						// modify left portion of this frag and we're done
		{
			_split(pFrag,workingLength);					// split frag into 2 parts (and NO coalescing)
			_setProp(pFrag,pCRec->getNewProp());			// apply new props to left half (and coalesce)
			return;
		}

		if (workingLength == lenFrag)						// exact match on this frag; delete it and we're done
		{
			_setProp(pFrag,pCRec->getNewProp());			// apply new props to this frag completely (and coalesce)
			return;
		}

		// need all of this frag and then some
		// TODO this case might not be necessary since i put the
		// TODO code in _turnOnProp() to cut it up into fragments.
		// TODO if it is necessary, deal with coalescing
//		wxLogTrace(TRACE_PTABLE_DUMP, _T("fim_ptable::_applyCRec: prop spanning multiple fragments [%ld][%ld]"), workingLength,lenFrag);
		wxASSERT_MSG( 0, _T("setProp spanning multiple fragments -- TODO -- you can continue from this") );

		// fim_frag * pFragNext = pFrag->m_next;
		// ....(pFrag,FOP...,false);			// set prop on frag -- BUT DO NOT COALESCE -- because we're walking the list
		// workingLength -= lenFrag;
		// pFrag = pFragNext;
	}
}

//////////////////////////////////////////////////////////////////
// when we apply a crec, we make one or more changes to the fragment
// list representing the content of the document.
// 
// we maintain a double-linked list of frags (with a head and tail
// pointer in our class).
//
// a single crec (to insert or delete a character, for example) may
// translate into a series of fragment manipulations -- such as
// splitting a frag to insert a new one between the parts and
// coalescing frags to minimize the number of frags required
// to represent a span of text.  after each change to the list
// we send a notification to our views to let them re-sync.
// each list manipulation must be of fine enough granularity
// that the views can follow along.  therefore, the list must
// be in a valid and complete state (although not necessarily
// coalesced) at all times.
//
// _linkNewFrag{Before,After}Frag() are the primary mechanisms
// for inserting a new fragment into the list -- and normally
// they are the 'fine grained step' -- but when splitting a frag
// we use them to do the linking and inhibit the notification so
// that _split() can do it with the right notification code.
//
// crec's provide us with information that we need to perform
// undo/redo operations -- that is, manipulate the fragment list.
// views watching the piecetable do NOT get to see them.
//
// views subscribe to CB events on low level fragment manipulations
// using addChangeCB() and delChangeCB().

void fim_ptable::_linkNewFragBeforeFrag(fim_frag * pFrag, fim_frag * pFragNew, fr_op fop, bool bCoalesce)
{
	wxASSERT_MSG( (pFrag && m_pFragHead && m_pFragTail), _T("Coding Error: fim_ptable::_linkNewFragBeforeFrag: null frag or invalid/empty list") );
	
#if 0
#ifdef _DEBUG
	wxLogTrace(TRACE_FRAG_DUMP, _T("fragInsertBefore: [fop %d]"), fop);
	pFragNew->dump(10);
	pFrag->dump(10);
#endif
#endif

	// insert pFragNew before pFrag in the fragment list and
	// optionally try to coalesce around the insertion point.
	// if pFrag was head of list, update it too.

	fim_frag * pFragOldPrev = pFrag->m_prev;
	
	pFragNew->m_prev = pFragOldPrev;
	pFrag->m_prev    = pFragNew;

	pFragNew->m_next = pFrag;
	if (pFragOldPrev)
		pFragOldPrev->m_next = pFragNew;
	else
		m_pFragHead  = pFragNew;
	
	m_cblFrag.callAll( util_cbl_arg(pFragNew,fop) );	// notify all views of change

	if (bCoalesce)
	{
		// since prepending is fairly common, we aggressively try to
		// coalesce *and* do it in a way to try to preserve the original
		// frag(s) rather than the new one.
		// 
		// we have [... pFragOldPrev (may be null) <--> pFragNew <--> pFrag ...]

		if (_coalesceLeft(pFrag))				// try to combine [pFragNew,pFrag] into [pFrag] and delete [pFragNew]
		{
			// now we have [... pFragOldPrev (may be null) <--> pFrag ...]

			if (pFragOldPrev)
				_coalesceRight(pFragOldPrev);	// try to combine [pFragOldPrev,pFrag] into [pFragOldPrev] and delete [pFrag]
			return;
		}

		// we still have [... pFragOldPrev (may be null) <--> pFragNew <--> pFrag ...]
			
		if (pFragOldPrev)
			_coalesceRight(pFragOldPrev);		// try to combine [pFragOldPrev,pFragNew] into [pFragOldPrev] and delete [pFragNew]

		return;
	}
}

void fim_ptable::_linkNewFragAfterFrag(fim_frag * pFrag, fim_frag * pFragNew, fr_op fop, bool bCoalesce)
{
	wxASSERT_MSG( (pFrag && m_pFragHead && m_pFragTail), _T("Coding Error: fim_ptable::_linkNewFragAfterFrag: null frag or invalid/empty list") );

#if 0
#ifdef _DEBUG
	wxLogTrace(TRACE_FRAG_DUMP, _T("fragInsertAfter: [fop %d]"), fop);
	pFrag->dump(10);
	pFragNew->dump(10);
#endif
#endif

	// insert pFragNew after pFrag in the fragment list and
	// optionally try to coalesce around the insertion point.
	// if pFrag was tail of list, update it too.

	fim_frag * pFragOldNext = pFrag->m_next;
	
	pFragNew->m_next = pFragOldNext;
	pFrag->m_next    = pFragNew;

	pFragNew->m_prev = pFrag;
	if (pFragOldNext)
		pFragOldNext->m_prev = pFragNew;
	else
		m_pFragTail  = pFragNew;

	m_cblFrag.callAll( util_cbl_arg(pFragNew,fop) );	// notify all views of change

	if (bCoalesce)
	{
		// since appending is fairly common -- typing a line of text
		// at the end of the document, for example -- we aggressively
		// try to coalesce *and* do it in a way to try to preserve
		// the original frag rather than the new one.
		//
		// we have [... pFrag <--> pFragNew <--> pFragOldNext (may be null) ...]

		if (_coalesceRight(pFrag))			// try to combine [pFrag,pFragNew] into [pFrag] and delete [pFragNew]
		{
			// now we have [... pFrag <--> pFragOldNext (may be null) ...]

			if (pFragOldNext)
				_coalesceRight(pFrag);		// try to combine [pFrag,pFragOldNext] into [pFrag] and delete [pFragOldNext]
			return;
		}

		// we still have [... pFrag <--> pFragNew <--> pFragOldNext (may be null) ...]

		if (pFragOldNext)
			_coalesceLeft(pFragOldNext);	// try to combine [pFragNew,pFragOldNext] into [pFragOldNext] and delete [pFragNew]

		return;
	}
}

void fim_ptable::_linkNewFrag(fim_frag * pFrag)
{
	// the first insertion into an empty list is always a little special.

	wxASSERT_MSG( (pFrag && !m_pFragHead && !m_pFragTail), _T("Coding Error: fim_ptable::_linkNewFrag: null frag or non empty list") );
	
	pFrag->m_next = NULL;
	pFrag->m_prev = NULL;

	m_pFragHead = pFrag;
	m_pFragTail = pFrag;

	m_cblFrag.callAll( util_cbl_arg(pFrag,FOP_INSERT_INITIAL) );	// notify all views of change
}

void fim_ptable::_unlinkFrag(fim_frag * pFrag, fr_op fop, bool bCoalesce)
{
#if 0
#ifdef _DEBUG
	wxLogTrace(TRACE_FRAG_DUMP, _T("fragUnlink: [fop %d]"), fop);
	pFrag->dump(10);
#endif
#endif

	// remove frag from the list and delete it.  optionally try to coalesce.

	fim_frag * pFragNext = pFrag->m_next;
	fim_frag * pFragPrev = pFrag->m_prev;

	if (pFragNext)
		pFragNext->m_prev = pFragPrev;
	else
		m_pFragTail = pFragPrev;

	if (pFragPrev)
		pFragPrev->m_next = pFragNext;
	else
		m_pFragHead = pFragNext;

	// note: in the case of normal delete (FOP_DELETE_SELF) we need to keep
	// note: the fragment length intact so that views know how much was removed.
	// note: in the case of an unlink because of coalesce (FOP_JOIN_*), our
	// note: caller has already zeroed the length -- because it was transfered
	// note: to one of the adjacent frags.

	// note: we null the link pointers in this frag before notifying the views.
	// note: this is because the frag is no longer in the list and we don't
	// note: want the views to wander the list incoherently.

	pFrag->m_next = NULL;
	pFrag->m_prev = NULL;

	m_cblFrag.callAll( util_cbl_arg(pFrag,fop) );	// notify all views of change

	delete pFrag;

	if (bCoalesce && pFragPrev && pFragNext)
	{
		// we had [... pFragPrev <--> pFrag <--> pFragNext ...] and deleted pFrag
		// 
		// try to combine [pFragPrev,pFragNext] into [pFragPrev] and delete [pFragNext]

		_coalesceRight(pFragPrev);
	}
}

void fim_ptable::_split(fim_frag * pFrag, fr_offset offset)
{
#if 0
#ifdef _DEBUG
	wxLogTrace(TRACE_FRAG_DUMP, _T("fragSplit:"));
#endif
#endif

	// divide/split "pFrag" at "offset" into 2 frags and update fragment list.
	// create new frag for tail portion.  we do not coalesce afterwards (it
	// would just re-join what we just split).

	// we want to avoid zero-length splits (on either end)
	wxASSERT_MSG( ((offset > 0) && (offset < pFrag->getFragLength())), _T("Coding Error: fim_ptable::_split: zero-length split") );

	fim_frag * pFragTail = new fim_frag(m_pFimBuf,
										pFrag->getBufOffset(offset),
										pFrag->getFragLength()-offset,
										pFrag->getFragProp());
	pFrag->m_lenData = offset;		// truncate our frag at offset because rest is now in pFragTail

	_linkNewFragAfterFrag(pFrag,pFragTail,FOP_SPLIT,false);
}

bool fim_ptable::_coalesceRight(fim_frag * pFrag)
{
	// try to coalesce this frag and the one on the right.  (keeping the given one in the list)

	if (!pFrag->_canCoalesceWithNext())
		return false;

	fim_frag * pFragNext = pFrag->m_next;

#if 0
#ifdef _DEBUG
	wxLogTrace(TRACE_FRAG_DUMP, _T("fragCoalesceRight:"));
	pFrag->dump(10);
	pFragNext->dump(10);
#endif
#endif

	pFrag->m_lenData += pFragNext->getFragLength();	// steal content from frag on right

	pFragNext->m_offsetData = pFrag->getBufOffset() + pFrag->getFragLength();	// remove content from frag on the right 
	pFragNext->m_lenData    = 0;												// just so there's no confusion

	// this may seem backwards, but the current frag (pFrag) is absorbing the
	// content of the next frag (pFragNext).  and it (pFragNext) will be deleted.
	// the report to viewers will go up with pFragNext, so from its point of view,
	// it joined with the frag before it.
	// 
	// we turn off coalescing in the unlink, so we don't go too recursive
	// and mess up list calculations of our callers -- besides, it should
	// have no effect (it will effectively do coalesceRight(pFrag,pFrag->next->next)
	// which should fail since pFrag->next and pFrag->next->next should have
	// already been coalesced).

	_unlinkFrag(pFragNext,FOP_JOIN_BEFORE,false);
	return true;
}

bool fim_ptable::_coalesceLeft(fim_frag * pFrag)
{
	// try to coalesce this frag and the one on the left.  (keeping the given one in the list)

	if (!pFrag->_canCoalesceWithPrev())
		return false;
	
	fim_frag * pFragPrev = pFrag->m_prev;

#if 0
#ifdef _DEBUG
	wxLogTrace(TRACE_FRAG_DUMP, _T("fragCoalesceLeft:"));
	pFragPrev->dump(10);
	pFrag->dump(10);
#endif
#endif

	pFrag->m_offsetData = pFragPrev->getBufOffset();	// steal content from frag on left
	pFrag->m_lenData   += pFragPrev->getFragLength();

	pFragPrev->m_lenData = 0;	// remove content from frag on left just so there's no confusion

	_unlinkFrag(pFragPrev,FOP_JOIN_AFTER,false);
	return true;
}

void fim_ptable::_setProp(fim_frag * pFrag, fr_prop prop)
{
#if 0
#ifdef _DEBUG
	wxLogTrace(TRACE_FRAG_DUMP, _T("fragSetProp:"));
#endif
#endif

	// set this prop on the frag and try to coalesce.

	pFrag->m_prop = prop;

	m_cblFrag.callAll( util_cbl_arg(pFrag,FOP_SET_PROP) );	// notify all views of change

	fim_frag * pFragPrev = pFrag->m_prev;
	
	// we have [... pFragPrev (may be null) <--> pFrag <--> pFragNext (may be null) ...]
	
	if (pFragPrev && _coalesceRight(pFragPrev))		// try to combine [pFragPrev,pFrag] into [pFragPrev] and delete [pFrag]
	{
		// now we have [... pFragPrev <--> pFragNext (may be null)...]

		_coalesceRight(pFragPrev);					// try to combine [pFragPrev,pFragNext] into [pFragPrev] and delete [pFragNext]
		return;
	}

	// we still have [... pFragPrev (may be null) <--> pFrag <--> pFragNext (may be null) ...]
			
	_coalesceRight(pFrag);							// try to combine [pFrag,pFragNext] into [pFrag] and delete [pFragNext]
}

//////////////////////////////////////////////////////////////////
// we have 2 schemes to address a position in the document.
// [] the usual absolute-document-offset
// [] fragment and relative-offset

fim_offset fim_ptable::getAbsoluteOffset(const fim_frag * pFrag, fr_offset offsetFrag) const
{
	//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_ptable::getAbsoluteOffset: [%p,%ld]"),pFrag,offsetFrag);

	// compute the absolute document offset of the (pFrag,offsetFrag).

	if (!pFrag)					// screw case: (null,offset) --> offset
		return offsetFrag;
	
	// find the absolute document offset of the beginning of the given fragment.

	fim_offset offsetDoc = 0;

	const fim_frag * p;

	for (p=m_pFragHead; ((p) && (p!=pFrag)); p=p->m_next)
		offsetDoc += p->getFragLength();

	wxASSERT_MSG( (p==pFrag), _T("Coding Error: fim_ptable::getAbsoluteOffset: frag not found") );

	// compute the absolute document offset to a position in the middle of this fragment.
	// (this may be past the end of the current fragment, but we don't care)

	return offsetDoc+offsetFrag;
}

void fim_ptable::getFragAndOffset(fim_offset offsetDoc, fim_frag ** ppFrag, fr_offset * pOffsetFrag) const
{
	//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_ptable::getFragAndOffset: [%ld]"),offsetDoc);

	// find the fragment and offset within it that represents
	// the given absolute document offset.
	//
	// note: there is an ambiguity at the boundary between 2 frags.
	// note: we prefer to report the left edge (offset 0) of the
	// note: frag on the right -- rather then the right edge (end)
	// note: of the frag on the left.


	if ((offsetDoc==0) && (!m_pFragHead))				// screwcase we allow: requesting absolute-document-offset 0 when empty list.
	{
		*ppFrag      = NULL;							// return NULL frag -- this is OK
		*pOffsetFrag = 0;
		return;
	}

	fim_offset offsetSoFar = 0;

	for (fim_frag * p=m_pFragHead; (p); p=p->m_next)
	{
		if (offsetSoFar+p->getFragLength() > offsetDoc)	// end of frag is past where we want, so must be inside this one
		{
			*ppFrag      = p;
			*pOffsetFrag = offsetDoc - offsetSoFar;		// offset within this frag
			return;
		}

		offsetSoFar += p->getFragLength();
	}

	if (offsetSoFar == offsetDoc)						// screwcase we allow: non-empty list and offset matches right edge of tail frag
	{
		*ppFrag      = m_pFragTail;
		*pOffsetFrag = m_pFragTail->getFragLength();
		return;
	}

	wxASSERT_MSG( (0), _T("Coding Error: fim_ptable::_getFragAndOffset: offset not found") );
}

void fim_ptable::_getFragAndOffsetFromHintOrDocOffset(fim_frag * pFragHint, fr_offset offsetFragHint, fim_offset offsetDoc,
													  fim_frag ** ppFrag, fr_offset * pOffsetFrag) const
{
	// find the fragment and offset within it that represents
	// the given absolute document offset or using the hints
	// given.
	
	if (!pFragHint)		 // if no hint, we have to find the right frag
	{
		getFragAndOffset(offsetDoc,ppFrag,pOffsetFrag);
		return;
	}

	// hint given, use it -- but first some sanity checking in debug mode

#ifdef _DEBUG
	fim_frag * pFragDebug     = NULL;
	fr_offset offsetFragDebug = 0;

	getFragAndOffset(offsetDoc,&pFragDebug,&offsetFragDebug);

	if ((pFragHint==pFragDebug) && (offsetFragHint==offsetFragDebug))
		;					// matched left edge of frag
	else if ((pFragDebug->m_prev) && (pFragHint==pFragDebug->m_prev) && (offsetFragHint==pFragDebug->m_prev->getFragLength()))
		;					// matched right edge of prev frag
	else
		wxASSERT_MSG( (0), _T("Coding Error: fim_ptable::_getFragAndOffsetFromHintOrDocOffset: hint doesn't match") );
#endif

	*ppFrag      = pFragHint;
	*pOffsetFrag = offsetFragHint;
}

void fim_ptable::_fixupFragAndOffset(fim_frag ** ppFrag, fr_offset * pOffsetFrag) const
{
	// sanity check to make sure we're in the right place.
	// we expect this might happen when we're on the right
	// edge exactly and should "more properly" be on the
	// left edge of the next one.

	fim_frag * pFrag = *ppFrag;
	fr_offset offsetFrag = *pOffsetFrag;

	if (!pFrag)
		pFrag = m_pFragHead;

	if (offsetFrag > 0)
	{
		while (offsetFrag >= pFrag->getFragLength())
		{
			//wxLogTrace(TRACE_PTABLE_DUMP, _T("fim_ptable:: fixing up initial frag [%ld][%ld]"), offsetFrag,pFrag->getFragLength());
		
			offsetFrag -= pFrag->getFragLength();
			pFrag = pFrag->m_next;

			wxASSERT_MSG( (pFrag), _T("Coding Error: fim_ptable:: fixing up initial frag -- past the end of the document") );
		}
	}

	*ppFrag = pFrag;
	*pOffsetFrag = offsetFrag;
}

//////////////////////////////////////////////////////////////////

fim_length fim_ptable::getAbsoluteLength(void)
{
	if (m_bCachedAbsoluteLengthValid)
	{
#if 0
#ifdef _DEBUG
		fim_length sum = 0;
		for (fim_frag * p=m_pFragHead; (p); p=p->m_next)
			sum += p->getFragLength();
		wxASSERT_MSG( (sum == m_cachedAbsoluteLength), _T("Coding Error") );
#endif
#endif

		return m_cachedAbsoluteLength;
	}
	
	fim_length sum = 0;
	for (fim_frag * p=m_pFragHead; (p); p=p->m_next)
		sum += p->getFragLength();

	//wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_ptable::getAbsoluteLength: computed [%ld]"),sum);

	m_cachedAbsoluteLength = sum;
	m_bCachedAbsoluteLengthValid = true;
	
	return sum;
}

//////////////////////////////////////////////////////////////////

pt_stat fim_ptable::_computeStat(void) const
{
	pt_stat s = PT_STAT_ZERO;

	if (m_crecvec.isDirty())			s |= PT_STAT_IS_DIRTY;
	if (!m_crecvec.canAutoMerge())		s |= PT_STAT_HAVE_AUTO_MERGED;
	if (m_crecvec.canUndo())			s |= PT_STAT_CAN_UNDO;
	if (m_crecvec.canRedo())			s |= PT_STAT_CAN_REDO;

	return s;
}

bool fim_ptable::hasFinalEOL(void) const
{
	// return true if the last line in the document ends with a EOL marker.
	// NOTE: the piecetable DOES NOT know anything about the line-oriented
	// NOTE: layout (that's fl_'s job).  but we do know enough to look at
	// NOTE: the chars at the end of the last frag in the document and see
	// NOTE: they are CR/LF/CRLF.  (this is probably cheating, but it's fast.)

	if (!m_pFragTail)	// empty document
		return false;
	if (m_pFragTail->getFragLength()==0)	// should not happen
		return false;
	
	const wxChar * pchLast = m_pFragTail->getTemporaryDataPointer(m_pFragTail->getFragLength()-1);

	return ((*pchLast == 0x000a) || (*pchLast == 0x000d));
}

//////////////////////////////////////////////////////////////////

fim_eol_mode fim_ptable::_guessEolMode(void) const
{
	// scan the contents of the document and try to determine
	// what type of file is (Windows, Linux, or Mac) by looking
	// at the EOL chars.
	//
	// we could scan the entire document and make sure all EOLs
	// are the same but that's kinda overkill.  let's just look
	// at the first line and see what it is.
	//
	// if this turns out to be a problem, we can always try
	// sampling multiple lines.

	bool bHaveCR = false;

	for (fim_frag * pFrag=m_pFragHead; (pFrag); pFrag=pFrag->m_next)
	{
		const wxChar * pBuf = pFrag->getTemporaryDataPointer(0);
		fim_length len = pFrag->getFragLength();

		for (fim_length k=0; k<len; k++)
		{
			if (bHaveCR)					// if last char was a CR
			{
				if (pBuf[k] == 0x000a)
					return FIM_MODE_CRLF;		// we have a CRLF
				else
					return FIM_MODE_CR;			// we have CR<something-else>
			}

			if (pBuf[k] == 0x000a)
				return FIM_MODE_LF;

			bHaveCR = (pBuf[k] == 0x000d);
		}
	}

	// document contains zero or one lines.  see if we had
	// a final CR that we're holding on.

	if (bHaveCR)
		return FIM_MODE_CR;

	return FIM_MODE_NATIVE_DISK;
}

//////////////////////////////////////////////////////////////////

void fim_ptable::forceAutoSaveNow(void)
{
	if (m_autoSave < 0)		// autosave turned off on this document
		return;
	
	int interval = gpGlobalProps->getLong(GlobalProps::GPL_MISC_AUTOSAVE_INTERVAL);
	if (interval < 0)		// autosave turned off globally
		return;

	m_autoSave = interval+1;	// fake it

	_considerAutoSave();
}

void fim_ptable::_considerAutoSave(void)
{
	if (m_autoSave < 0)		// autosave turned off on this document
		return;

	// auto-save every n edits

	int interval = gpGlobalProps->getLong(GlobalProps::GPL_MISC_AUTOSAVE_INTERVAL);
	if (interval < 0)		// autosave turned off globally
		return;
	
	m_autoSave++;
	if (m_autoSave < interval)		// not yet time to autosave
		return;
	m_autoSave = 0;

	pt_stat stat = _computeStat();
	m_cblStat.callAll( util_cbl_arg(NULL,stat | PT_STAT_AUTOSAVE_BEGIN) );	// let windows put "Autosaving..." in status bar
	
	util_error ue = _attemptSave(m_pPoiItem);

//	wxLogTrace(TRACE_PTABLE_DUMP,_T("AutoSave: [Result %s (%s)] for [%s]"),
//			   ue.getMessage().wc_str(),ue.getExtraInfo().wc_str(),
//			   util_printable_s(m_pPoiItem->getFullPath()).wc_str());

	if (ue.isOK())
	{
		m_cblStat.callAll( util_cbl_arg(NULL,stat | PT_STAT_AUTOSAVE_END) );	// let windows put "Autosave completed." in status bar
	}
	else
	{
		m_cblStat.callAll( util_cbl_arg(NULL,stat | PT_STAT_AUTOSAVE_ERROR) );	// let windows put "Autosave failed." in status bar
		m_autoSave = -1;		// turn off autosave on this document for now.
	}
	
}

//////////////////////////////////////////////////////////////////

util_error fim_ptable::attemptSave(poi_item * pPoiItem)
{
	// write the contents of this document to the given file.
	// update clean/dirty status.

	pt_stat s0 = _computeStat();

	util_error ue = _attemptSave(pPoiItem);
	if (ue.isOK())
		m_crecvec.markClean();
	
	pt_stat s1 = _computeStat();

//	wxLogTrace(TRACE_PTABLE_DUMP,_T("fim_ptable::Save: [Result %s (%s)] Saving [%s] to [%s]"),
//			   util_printable_s(ue.getMessage()).wc_str(),
//			   util_printable_s(ue.getExtraInfo()).wc_str(),
//			   util_printable_s(m_pPoiItem->getFullPath()).wc_str(),
//			   util_printable_s(pPoiItem->getFullPath()).wc_str());

	if (s1 != s0)
		m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status

	return ue;
}

util_error fim_ptable::attemptSaveWithForceIfNecessary(wxWindow * pParent, poi_item * pPoi)
{
	// write the contents of this ptable to the POI given.
	// if the file is write-protected, try to chmod it and try again.

	util_error ue = attemptSave(pPoi);
	if (ue.isOK())
		return ue;

	if (ue.getErr() == util_error::UE_CANNOT_OPEN_FILE)
	{
		// file might be write-protected, ask the user if we should try
		// to chmod() it and give us write access.  if so, do it and
		// try to write the file again.  this is not guaranteed to work;
		// for example, if the file is on a read-only partition like a CD.

		wxString str(pPoi->getFullPath().wc_str());
		str += _("\n\nCould not open this file for writing.\n\nTry to override file permissions?");

		wxMessageDialog dlg(pParent,str,_("Cannot Open File for Writing"),wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
		int answer = dlg.ShowModal();

		if (answer == wxID_NO)
		{
			ue.set(util_error::UE_CANCELED);
			return ue;
		}

		if (!pPoi->tryToMakeWritable())
		{
			ue.set(util_error::UE_CANNOT_CHMOD_FILE);
			return ue;
		}
				
		// try again to save now that we have done a chmod().
		// this can still fail -- such as when disk is full...

		ue = attemptSave(pPoi);
	}
	
	return ue;
}

//////////////////////////////////////////////////////////////////

void fim_ptable::_createSaveBuffer(wxString & rString)
{
	// populate a string with the entire contents of the document
	// so that we can convert it to the character encoding for disk.

	// we don't skip out early if lenDoc is zero because we need to
	// respect the final EOL props.

	fim_length lenDoc = getAbsoluteLength();

	rString.Empty();
	rString.Alloc( lenDoc+10 );
	
	for (const fim_frag * pFrag=m_pFragHead; (pFrag); pFrag=pFrag->getNext())
	{
		fim_length len = pFrag->getFragLength();
		wxASSERT_MSG( (len > 0), _T("Coding Error") );

		const wxChar * pBuf = pFrag->getTemporaryDataPointer();

		for (fim_length k=0; k<len; k++)
			rString += pBuf[k];
	}

	bool bRequireFinalEOL = gpGlobalProps->getBool(GlobalProps::GPL_MISC_REQUIRE_FINAL_EOL);
	bool bHasFinalEOL = hasFinalEOL();
		
	if (bRequireFinalEOL && !bHasFinalEOL)
	{
		fim_eol_mode eolMode = getEolMode();

		switch (eolMode)
		{
#if defined(__WXGTK__) || defined(__WXMAC__)
		// on GTK and MAC assume LF if not known
		default:				// quiets compiler
		case FIM_MODE_UNSET:
#endif
		case FIM_MODE_LF:
			rString += _T('\n');
			break;

#if defined(__WXMSW__)
		// on WINDOWS assume CRLF if not known
		default:				// quiets compiler
		case FIM_MODE_UNSET:
#endif
		case FIM_MODE_CRLF:
			rString += _T('\r');
			rString += _T('\n');
			break;

		case FIM_MODE_CR:
			rString += _T('\r');
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////

util_error fim_ptable::_attemptSave(poi_item * pPoiItem)
{
	//////////////////////////////////////////////////////////////////
	// write the contents of this document to the given file.
	// NOTE: this may be an auto-save temp-file, the original
	// NOTE: document, or an alternate file name.  so we need
	// NOTE: to be careful to not refer to m_pPoiItem when saving.
	//////////////////////////////////////////////////////////////////

	// WARNING: this is *EXTREMELY* inefficient -- we create a temporary
	// buffer and put the entire document into it (this is a wide buffer).
	// we then create a multi-byte buffer and do the export conversion.
	// IFF all this succeeds, only then do we try to write to disk.
	// this is done on purpose because the wxMBConv and wxCSConv routines
	// are platform-dependent, quite buggy, currently (as of 2.8.0) in
	// flux (as they transition to ToWChar/FromWChar and deprecate
	// MB2WC/WC2MB (which is being done in phases (so some are not
	// implemented on some platforms))).
	//
	// wxBUG: we cannot use pFile->Write(wxString,wxMBConv) because it
	// wxBUG: does the w-2-mb conversion and then calls strlen() on the
	// wxBUG: result -- this might be fine when we are exporting utf8
	// wxBUG: or latin-1, but is bogus when we are exporting to utf16.
	//
	// wxBUG: we do not use ToWChar/FromWChar because they don't work
	// wxBUG: everywhere (as of 2.8.0).
	//
	// wxBUG: wxMBConv::ToWChar() and ::FromWChar() seem like they are named
	// wxBUG: or documented backwards.  their usage by WC2MB() and MB2WC()
	// wxBUG: don't match the documentation (as of 2.8.0).
	//
	// no attempt is made to deal with EOL conventions; we assume that the
	// EOL's that are in the document are ok.
	// 
	// we do attempt to force final EOL character if requested in global preferences.
	//
	// we do convert output to the character encoding that we used when the document
	// was loaded.  if there was a BOM when we read the file, we will put one on it
	// when we write it.
	//
	// WE DO NOT DEAL WITH BACKUP FILES or OTHER VERSIONINGS.
	// WE DO NOT MARK THE CRECVEC CLEAN (not dirty).
	
	wxString strWide;
	byte * pBufRaw = NULL;
	size_t lenRaw = 0;

	_createSaveBuffer(strWide);
	if (strWide.Length() > 0)
	{
		util_error ue1;

		ue1 = util_encoding_export_conv(strWide,m_enc,&pBufRaw,&lenRaw);
		if (ue1.isErr())
			return ue1;
	}

	// we need to free pBufRaw.

	//////////////////////////////////////////////////////////////////
	// wxWidgets file io routines spew MessageBox()'s on error -- most annoying.
	// capture error messages for display later.  use local scoping to limit
	// lifetime of log/error diversion and open and read file.

	util_error ue;
	wxFile file;
	
	{
		util_logToString uLog(&ue.refExtraInfo());

		// don't use wxFile::Open() because of call to wxLogSysError().

		int fd = util_file_open(ue, pPoiItem->getFullPath(), wxFile::write);		// WARNING: this will truncate any existing file
		if (fd == -1)
			goto Failed;
		file.Attach(fd);
		
		if (m_bHadUnicodeBOM)
		{
			// prepend BOM to file since we had one when we read it in.
			// be sure to use the raw version of wxFile::write() so that
			// it doesn't try to convert the buffer from unicode to one of
			// the utf formats....
			//
			// NOTE: we DO NOT write a BOM unless we read one.  there's lots
			// NOTE: of discussion (and ambiguity on the net) about when one
			// NOTE: should and should not write one -- this includes stuff
			// NOTE: like depending on whether the user said UTF16 vs whether
			// NOTE: the user said UTF16BE when the selected an encoding --
			// NOTE: that is, if they explicitly said BE, then the BOM 
			// NOTE: expressly should NOT be present (see the unicode docs).
			// NOTE: that is, the unicode people treat UTF16BE, UTF16LE, and
			// NOTE: UTF16 as 3 different choices which are different.
			// NOTE:
			// NOTE: there's a problem with the design of wxWidgets -- they silently
			// NOTE: fold the BE or LE version to the plain version depending on
			// NOTE: the platform.  so only 2 choices exist in the wxFONTENCODING
			// NOTE: table.  the human-readable label for one of them omits the
			// NOTE: BE or LE.  for example, on intel, the table has UTF16BE and
			// NOTE: UTF16 -- so we can't make a general 3-way chooser dialog....

			byte bom[4];
			int lenBOM = util_encoding_create_bom(m_enc,bom);
			wxASSERT_MSG( (lenBOM > 0), _T("Coding Error") );

			if (!file.Write((const void *)bom,lenBOM))
			{
				ue.set(util_error::UE_CANNOT_WRITE_FILE);
				goto Failed;
			}
		}

		if (pBufRaw && lenRaw)
		{
			if (!file.Write((const void *)pBufRaw,lenRaw))
			{
				ue.set(util_error::UE_CANNOT_WRITE_FILE);
				goto Failed;
			}
		}
	}

//Succeed:
	pPoiItem->incrementSaveCount();
	FREEP(pBufRaw);
	return ue;

Failed:
	FREEP(pBufRaw);
	return ue;
}

//////////////////////////////////////////////////////////////////

void fim_ptable::_resetAutoSave(void)
{
	if (m_autoSave > 0)			// restart the counter, if enabled.
		m_autoSave = 0;
}

void fim_ptable::enableAutoSave(void)
{
	if (m_autoSave < 0)			// turn on autosave on this document, if turned off.
		m_autoSave = 0;			// we require global autosave to be on for it to have any effect.

	//wxLogTrace(TRACE_PTABLE_DUMP,_T("PTable: auto-save enabled for [%s]"),m_pPoiItem->getFullPath().wc_str());
}

//////////////////////////////////////////////////////////////////

void fim_ptable::updateDateTimes(void)
{
	util_error ue;

	ue = m_pPoiItem->getDateTimeModified(&m_dtmAsLoaded);
	m_dtmChecked = m_dtmAsLoaded;
}

void fim_ptable::rebindPoi(poi_item * pPoiNew)
{
	wxASSERT_MSG( (pPoiNew != m_pPoiItem), _T("Coding Error") );

	poi_item * pPoiOld = m_pPoiItem;

	pPoiOld->setPTable(NULL);		// remove 'this' from old POI
	pPoiNew->setPTable(this);		// give 'this' to new POI

	m_pPoiItem = pPoiNew;

	pt_stat s1 = _computeStat() | PT_STAT_PATHNAME_CHANGE;
	m_cblStat.callAll( util_cbl_arg(NULL,s1) );	// notify all views of change in status
}

//////////////////////////////////////////////////////////////////

void fim_ptable::_setPoiItem(poi_item * pPoiItem)
{
	wxASSERT_MSG( (m_pPoiItem == NULL), _T("Coding Error") );

	m_pPoiItem = pPoiItem;
}

//////////////////////////////////////////////////////////////////

bool fim_ptable::hasChangedOnDiskSinceLoading(util_error & ue)
{
	// compare date-time stamp on file now with stamp when we loaded it.

	wxDateTime dtmNow;
	ue = m_pPoiItem->getDateTimeModified(&dtmNow);
	if (ue.isErr())			// we can't stat it now, so we can't tell if it has changed.
		return false;		// just say no because we probably can't reload it anyway.

	return ( ! dtmNow.IsEqualTo(m_dtmAsLoaded));
}

bool fim_ptable::hasChangedOnDiskSinceLastChecked(util_error & ue, bool bUpdate)
{
	// compare date-time stamp on file now with stamp when we last checked it.

	wxDateTime dtmNow;
	ue = m_pPoiItem->getDateTimeModified(&dtmNow);
	if (ue.isErr())			// we can't stat it now, so we can't tell if it has changed.
		return false;		// just say no because we probably can't reload it anyway.

	bool bChanged = ( ! dtmNow.IsEqualTo(m_dtmChecked));

	if (bUpdate)
		m_dtmChecked = dtmNow;
	
	return bChanged;
}

//////////////////////////////////////////////////////////////////

static const wxChar * aszEolModes[] = { _T("Unknown"),	// FIM_MODE_UNSET
										_T("LF"),			// FIM_MODE_LF
										_T("CRLF"),		// FIM_MODE_CRLF
										_T("CR"),			// FIM_MODE_CR
};

wxString fim_ptable::dumpSupportInfo(const wxString & strIndent) const
{
	wxString str;
	wxString strIndent2 = strIndent + _T("\t");

	str += wxString::Format(_T("%sPath: "), strIndent.wc_str());
	if (m_pPoiItem)
		str += m_pPoiItem->getFullPath();
	str += _T("\n");

	str += wxString::Format(_T("%sEOL Style: %s\n"), strIndent2.wc_str(),aszEolModes[m_eolMode]);
	str += wxString::Format(_T("%sEncoding: %s %s\n"),
							strIndent2.wc_str(),
							wxFontMapper::GetEncodingDescription((wxFontEncoding)m_enc).wc_str(),
							((m_bHadUnicodeBOM) ? _T("(with BOM)") : _T("(without BOM)")) );
	str += wxString::Format(_T("%sStats: [0x%08x]\n"), strIndent2.wc_str(),_computeStat());

	return str;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fim_ptable::dump(int indent) const
{
	wxLogTrace(TRACE_FRAG_DUMP, _T("%*c=============================================================================="), indent,' ');
	wxLogTrace(TRACE_FRAG_DUMP, _T("%*cFIM_PTABLE[%p][%s]::frag_list:"), indent,_T(' '),this,
			   ((m_pPoiItem) ? util_printable_s(m_pPoiItem->getFullPath()).wc_str() : _T("")));
	for (const fim_frag * p=m_pFragHead; (p); p=p->m_next)
		p->dump(indent+5);
	wxLogTrace(TRACE_FRAG_DUMP, _T("%*c=============================================================================="), indent,' ');
}
#endif

//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////

#if 0
#ifdef DEBUG
static void _cb_stat_test(void * /*pThis*/, const util_cbl_arg & arg)
{
	//fim_ptable * pPTable	= (fim_ptable *)pThis;
	pt_stat s				= (pt_stat)arg.m_l;

	wxLogTrace(TRACE_PTABLE_DUMP, _T("STAT: [%x][%s][%s][%s][%s]"), s,
			   ((PT_STAT_TEST(s,PT_STAT_IS_DIRTY)) ? _T("DIRTY") : _T("")),
			   ((PT_STAT_TEST(s,PT_STAT_HAVE_AUTO_MERGED)) ? _T("MERGED") : _T("")),
			   ((PT_STAT_TEST(s,PT_STAT_CAN_UNDO)) ? _T("can UNDO") : _T("")),
			   ((PT_STAT_TEST(s,PT_STAT_CAN_REDO)) ? _T("can REDO") : _T("")));
}

static void _cb_frag_test(void * pThis, const util_cbl_arg & arg)
{
	fim_ptable * pPTable	= (fim_ptable *)pThis;
	const fim_frag * pFrag	= (const fim_frag *)arg.m_p;
	fr_op fop				= (fr_op)arg.m_l;

	pPTable->_cb_frag_test(pFrag,fop);
}

void fim_ptable::_cb_frag_test(const fim_frag * pFrag, fr_op fop)
{
	switch (fop)
	{
	default:
	case FOP_INVALID:
		wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: invalid"));
		return;
		
	case FOP_INSERT_INITIAL:
		{
			wxString strFragNew(pFrag->getTemporaryDataPointer(),pFrag->getFragLength());
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: inserting frag [%s] into to empty list"),
					   strFragNew.wc_str());
		}
		return;
		
	case FOP_INSERT_BEFORE:
		{
			wxString strFragNew(pFrag->getTemporaryDataPointer(),pFrag->getFragLength());
			fim_frag * pFragNext = pFrag->m_next;
			wxString strFragRel(pFragNext->getTemporaryDataPointer(),pFragNext->getFragLength());
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: [%ld] frag [%s] inserted before frag [%s]"),
					   getAbsoluteOffset(pFrag,0), strFragNew.wc_str(), strFragRel.wc_str());
		}
		return;
		
	case FOP_INSERT_AFTER:
		{
			wxString strFragNew(pFrag->getTemporaryDataPointer(),pFrag->getFragLength());
			fim_frag * pFragPrev = pFrag->m_prev;
			wxString strFragRel(pFragPrev->getTemporaryDataPointer(),pFragPrev->getFragLength());
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: [%ld] frag [%s] inserted after frag [%s]"),
					   getAbsoluteOffset(pFrag,0), strFragNew.wc_str(), strFragRel.wc_str());
		}
		return;
		
	case FOP_SPLIT:
		{
			wxString strFragNew(pFrag->getTemporaryDataPointer(),pFrag->getFragLength());
			fim_frag * pFragPrev = pFrag->m_prev;
			wxString strFragRel(pFragPrev->getTemporaryDataPointer(),pFragPrev->getFragLength());
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: [%ld] split frag into [%s] and [%s]"),
					   getAbsoluteOffset(pFrag,0), strFragRel.wc_str(), strFragNew.wc_str());
		}
		return;
		
	case FOP_JOIN_BEFORE:
		{
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: frag absorbed on end of previous frag"));
		}
		return;
		
	case FOP_JOIN_AFTER:
		{
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: frag absorbed on front of next frag"));
		}
		return;
		
	case FOP_DELETE_SELF:
		{
			wxString strFrag(pFrag->getTemporaryDataPointer(),pFrag->getFragLength());
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: frag [%s] deleted"),
					   strFrag.wc_str());
		}
		return;

	case FOP_SET_PROP:
		{
			wxString strFrag(pFrag->getTemporaryDataPointer(),pFrag->getFragLength());
			wxLogTrace(TRACE_PTABLE_DUMP, _T("FOP: frag prop set to [%lx] on [%s]"),
					   pFrag->getFragProp(),strFrag.wc_str());
		}
		return;
	}
}

unsigned long fim_ptable::_count_frags(void) const
{
	unsigned long sum = 0;
	for (fim_frag * p=m_pFragHead; (p); p=p->m_next)
		sum++;

	return sum;
}

fim_length fim_ptable::_get_doc_length(void) const
{
	fim_length sum = 0;
	for (fim_frag * p=m_pFragHead; (p); p=p->m_next)
		sum += p->getFragLength();

	return sum;
}

long fim_ptable::_test_compact(void) const
{
	long nrErrors = 0;
	for (fim_frag * p=m_pFragHead; (p); p=p->m_next)
		if (p->_canCoalesceWithNext())
		{
			nrErrors++;
			wxString strFrag(p->getTemporaryDataPointer(),p->getFragLength());
			fim_frag * pNext = p->m_next;
			wxString strFragNext(pNext->getTemporaryDataPointer(),pNext->getFragLength());
			wxLogTrace(TRACE_PTABLE_DUMP, _T("NOTCOMPACT: [%ld] frags [%s][%s]"),
					   getAbsoluteOffset(pNext,0), strFrag.wc_str(), strFragNext.wc_str());
		}

	return nrErrors;
}

wxString fim_ptable::_get_doc(void) const
{
	//////////////////////////////////////////////////////////////////
	// CRUDE HACK FOR TESTING PURPOSES ONLY
	//////////////////////////////////////////////////////////////////

	wxString s;
	for (fim_frag * p=m_pFragHead; (p); p=p->m_next)
		s += wxString(p->getTemporaryDataPointer(),p->getFragLength());

	return s;
}

void fim_ptable::test01(void)
{
	addChangeCB(::_cb_frag_test,this);
	addStatusCB(::_cb_stat_test,this);
	
	//////////////////////////////////////////////////////////////////
	// run some consistency tests on piecetable and fragment list.
	// tests: insert, delete, undo, redo
	//////////////////////////////////////////////////////////////////

	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST01: BEGIN"));

	wxString szMATCH[20];
	fim_frag * pFragTemp;
	fim_offset kPos;
	long kTest = 0;
	
	//////////////////////////////////////////////////////////////////
	// start with empty document
	szMATCH[kTest] = _T(""); kTest++;
	wxASSERT_MSG( (_count_frags()==0), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// *** basic insert tests ***
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// test  : insert 1 string
	// expect: 1 string of correct length
	wxString sz01 = _T("Line 1...Line 2...Line 3...Line 4...");
	insertText(NULL,0, FR_PROP_ZERO, sz01.wc_str(), sz01.Len());	// insert into empty document
	szMATCH[kTest] += sz01;
	wxASSERT_MSG( (_count_frags()==1), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// test  : append 1 string
	// expect: coalesced into 1 frag (since growbuf space will be contiguous)
	wxString sz02 = _T("AAAA...BBBB...CCCC...DDDD...");
	insertText(m_pFragTail,m_pFragTail->getFragLength(), FR_PROP_ZERO, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	wxASSERT_MSG( (_count_frags()==1), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// test  : append a bunch more
	// expect: coalesced into 1 frag (since growbuf space will be contiguous)
	insertText(m_pFragTail,m_pFragTail->getFragLength(), FR_PROP_ZERO, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	insertText(m_pFragTail,m_pFragTail->getFragLength(), FR_PROP_ZERO, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	insertText(m_pFragTail,m_pFragTail->getFragLength(), FR_PROP_ZERO, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	wxASSERT_MSG( (_count_frags()==1), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// test  : insert in middle of a frag
	// expect: 3 frags
	wxString sz03 = _T("XXXXYYYYZZZZ");
	kPos = 20;
	insertText(m_pFragHead,kPos, FR_PROP_ZERO, sz03.wc_str(), sz03.Len());
	szMATCH[kTest+1] = szMATCH[kTest].Left(kPos) + sz03 + szMATCH[kTest].Mid(kPos); kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// test  : insert between 2nd & 3rd frags (using right edge of 2nd frag as ref point)
	// expect: 3 frags (since growbuf space will be contiguous, frag 2 should absorb new content)
	pFragTemp = m_pFragHead->m_next;
	insertText(pFragTemp,pFragTemp->getFragLength(), FR_PROP_ZERO, sz03.wc_str(), sz03.Len());
	kPos = kPos + sz03.Len();
	szMATCH[kTest+1] = szMATCH[kTest].Left(kPos) + sz03 + szMATCH[kTest].Mid(kPos); kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// test  : insert between 2nd & 3rd frags (using left edge of 3rd frag as ref point)
	// expect: 3 frags (since growbuf space will be contiguous, frag 2 should absorb new content)
	pFragTemp = m_pFragTail;
	insertText(pFragTemp,0, FR_PROP_ZERO, sz03.wc_str(), sz03.Len());
	kPos = kPos + sz03.Len();
	szMATCH[kTest+1] = szMATCH[kTest].Left(kPos) + sz03 + szMATCH[kTest].Mid(kPos); kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// test  : prepend 1 string
	// expect: 4 frags
	wxString sz04 = _T("QQQQWWWWEEEE");
	insertText(m_pFragHead,0, FR_PROP_ZERO, sz04.wc_str(), sz04.Len()); // prepend to document
	szMATCH[kTest+1] = sz04 + szMATCH[kTest]; kTest++;
	wxASSERT_MSG( (_count_frags()==4), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// that covers all 4 cases of insert in applyCRec and both types of coalescing
	//////////////////////////////////////////////////////////////////
	// we expect
	// [prop 0][len 12][QQQQWWWWEEEE]
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 0][len 36][XXXXYYYYZZZZXXXXYYYYZZZZXXXXYYYYZZZZ]
	// [prop 0][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// *** basic delete tests ***
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// test  : delete part of left end of a fragment (a ltrim-like operation)
	// expect: 4 fragments & named fragment to be adjusted
	// head frag should have "QQQQWWWWEEEE" before and "WWWWEEEE" after
	deleteText(m_pFragHead,0,4);
	szMATCH[kTest+1] = szMATCH[kTest].Mid(4); kTest++;
	wxASSERT_MSG( (_count_frags()==4), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );
	
	//////////////////////////////////////////////////////////////////
	// test  : delete part of right end of a fragment (a rtrim-like operation)
	// expect: 4 fragments & named fragment to be adjusted
	// head frag should have "WWWWEEEE" before and "WWWW" after
	deleteText(m_pFragHead,4,4);
	szMATCH[kTest+1] = szMATCH[kTest].Left(4) + szMATCH[kTest].Mid(8); kTest++;
	wxASSERT_MSG( (_count_frags()==4), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// test  : delete entire fragment
	// expect: 3 fragments
	// delete head frag containing "WWWW"
	deleteText(m_pFragHead,0,4);
	szMATCH[kTest+1] = szMATCH[kTest].Mid(4); kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// we expect
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 0][len 36][XXXXYYYYZZZZXXXXYYYYZZZZXXXXYYYYZZZZ]
	// [prop 0][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : delete part of middle of a fragment
	// expect: 4 fragments
	// delete first "YYYY" from second frag
	deleteText(m_pFragHead->m_next,4,4);
	kPos = 20 + 4;				// "Line 1...Line 2...Li" + "XXXX"
	szMATCH[kTest+1] = szMATCH[kTest].Left(kPos) + szMATCH[kTest].Mid(kPos+4); kTest++;
	wxASSERT_MSG( (_count_frags()==4), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// we expect
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 0][len 4][XXXX]
	// [prop 0][len 28][ZZZZXXXXYYYYZZZZXXXXYYYYZZZZ]
	// [prop 0][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : delete part of left end of a fragment but by referencing right end of previous fragment
	// expect: 4 fragments
	// delete first "ZZZZ" from third frag
	deleteText(m_pFragHead->m_next,4,4);
	kPos = 20 + 4;				// "Line 1...Line 2...Li" + "XXXX"
	szMATCH[kTest+1] = szMATCH[kTest].Left(kPos) + szMATCH[kTest].Mid(kPos+4); kTest++;
	wxASSERT_MSG( (_count_frags()==4), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// we expect
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 0][len 4][XXXX]
	// [prop 0][len 24][XXXXYYYYZZZZXXXXYYYYZZZZ]
	// [prop 0][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : delete text spanning multiple fragments
	// expect: 3 fragments
	// delete "XXXXXXXX" starting with second fragment and taking part of third
	deleteText(m_pFragHead->m_next,0,8);
	kPos = 20;					// "Line 1...Line 2...Li"
	szMATCH[kTest+1] = szMATCH[kTest].Left(kPos) + szMATCH[kTest].Mid(kPos+8); kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// [A] we expect
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 0][len 20][YYYYZZZZXXXXYYYYZZZZ]
	// [prop 0][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : delete text spanning multiple fragments -- delete entire document contents
	// expect: 0
	deleteText(m_pFragHead,0,szMATCH[kTest].Len());
	szMATCH[kTest+1] = _T(""); kTest++;
	wxASSERT_MSG( (_count_frags()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// [B] we expect an empty document

	dump(0);
	
	//////////////////////////////////////////////////////////////////
	// *** basic undo tests ***
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// test  : one undo
	// expect: doc identical to state [A] above
	undo();
	kTest--;

	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// [A] we expect
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 0][len 20][YYYYZZZZXXXXYYYYZZZZ]
	// [prop 0][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : one redo
	// expect: doc identical to state [B] (empty)
	redo();
	kTest++;
	wxASSERT_MSG( (_count_frags()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// [B] we expect an empty document

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : undo all the way back to the start

	long kTestLimit = kTest;

	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST: undo to beginning"));
	while (kTest > 0)
	{
		undo();
		wxASSERT_MSG( (_get_doc()==szMATCH[--kTest]), _T("TESTFAILED") );
	}

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : redo all the way back to end

	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST: redo to end"));
	while (kTest < kTestLimit)
	{
		redo();
		wxASSERT_MSG( (_get_doc()==szMATCH[++kTest]), _T("TESTFAILED") );
	}

	dump(0);
	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST01: END"));

	//////////////////////////////////////////////////////////////////
	// *** un-register viewer
	//////////////////////////////////////////////////////////////////

	delChangeCB(::_cb_frag_test,this);
	delStatusCB(::_cb_stat_test,this);
}

void fim_ptable::test02(void)
{
	addChangeCB(::_cb_frag_test,this);
	addStatusCB(::_cb_stat_test,this);
	
	//////////////////////////////////////////////////////////////////
	// run some consistency tests on piecetable and fragment list.
	// tests: insert, turnOnProp, undo, redo.
	//////////////////////////////////////////////////////////////////

	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST02: BEGIN"));

	wxString szMATCH[20];
	fim_offset kPos;
	long kTest = 0;
	
	//////////////////////////////////////////////////////////////////
	// start with empty document
	szMATCH[kTest] = _T(""); kTest++;
	wxASSERT_MSG( (_count_frags()==0), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// *** basic insert tests ***
	//////////////////////////////////////////////////////////////////

#define PROP_BIT_1			(0x1)
#define PROP_BIT_2			(0x2)
#define PROP_BIT_3			(0x4)

	//////////////////////////////////////////////////////////////////
	// test  : insert 1 string
	// expect: 1 string of correct length
	wxString sz01 = _T("Line 1...Line 2...Line 3...Line 4...");
	insertText(NULL,0, FR_PROP_ZERO, sz01.wc_str(), sz01.Len());	// insert into empty document
	szMATCH[kTest] += sz01;
	wxASSERT_MSG( (_count_frags()==1), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );
	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : append 1 string
	// expect: 2 frags -- not coalesced because of different props
	wxString sz02 = _T("AAAA...BBBB...CCCC...DDDD...");
	insertText(m_pFragTail,m_pFragTail->getFragLength(), PROP_BIT_1, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	wxASSERT_MSG( (_count_frags()==2), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );
	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : append a bunch more
	// expect: 3 frags -- because of props
	insertText(m_pFragTail,m_pFragTail->getFragLength(), PROP_BIT_2, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	insertText(m_pFragTail,m_pFragTail->getFragLength(), PROP_BIT_2, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	insertText(m_pFragTail,m_pFragTail->getFragLength(), PROP_BIT_2, sz02.wc_str(), sz02.Len()); // append to document
	szMATCH[kTest+1] = szMATCH[kTest] + sz02; kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );
	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : insert in middle of a frag
	// expect: 5 frags
	wxString sz03 = _T("XXXXYYYYZZZZ");
	kPos = 20;
	insertText(m_pFragHead,kPos, (PROP_BIT_1|PROP_BIT_2), sz03.wc_str(), sz03.Len());
	szMATCH[kTest+1] = szMATCH[kTest].Left(kPos) + sz03 + szMATCH[kTest].Mid(kPos); kTest++;
	wxASSERT_MSG( (_count_frags()==5), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// we expect:
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 3][len 12][XXXXYYYYZZZZ]
	// [prop 0][len 16][ne 3...Line 4...]
	// [prop 1][len 28][AAAA...BBBB...CCCC...DDDD...]
	// [prop 2][len 84][AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// *** basic set-prop tests ***
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// test  : change prop on one frag -- OR on 0x4 on second frag (current value 0x3)
	// expect: no change in frag list; frag will have new props
	turnOnProp(m_pFragHead->m_next,0,m_pFragHead->m_next->getFragLength(),PROP_BIT_3);
	szMATCH[kTest+1] = szMATCH[kTest]; kTest++;
	wxASSERT_MSG( (_count_frags()==5), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );
	wxASSERT_MSG( (m_pFragHead->m_next->getFragProp()==(PROP_BIT_3|PROP_BIT_2|PROP_BIT_1)), _T("TESTFAILED") );
	//////////////////////////////////////////////////////////////////
	// we expect:
	// [prop 0][len 20][Line 1...Line 2...Li]
	// [prop 7][len 12][XXXXYYYYZZZZ]
	// [prop 0][len 16][ne 3...Line 4...]
	// [prop 1][len 28][AAAA...BBBB...CCCC...DDDD...]
	// [prop 2][len 84][AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : change prop on part of one frag -- OR on 0x4 on first frag (current value 0x0)
	// expect: no change in frag list; frag will have new props
	turnOnProp(m_pFragHead,0,10,PROP_BIT_3);
	szMATCH[kTest+1] = szMATCH[kTest]; kTest++;
	wxASSERT_MSG( (_count_frags()==6), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );
	wxASSERT_MSG( (m_pFragHead->getFragProp()==(PROP_BIT_3)), _T("TESTFAILED") );
	wxASSERT_MSG( (m_pFragHead->m_next->getFragProp()==(0)), _T("TESTFAILED") );
	//////////////////////////////////////////////////////////////////
	// [A] we expect:
	// [prop 4][len 10][Line 1...L]
	// [prop 0][len 10][ine 2...Li]
	// [prop 7][len 12][XXXXYYYYZZZZ]
	// [prop 0][len 16][ne 3...Line 4...]
	// [prop 1][len 28][AAAA...BBBB...CCCC...DDDD...]
	// [prop 2][len 84][AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : change prop across a series of frags
	// expect: lots of coalescing
	turnOnProp(m_pFragHead,0,szMATCH[kTest].Len(),(PROP_BIT_3|PROP_BIT_2|PROP_BIT_1));
	szMATCH[kTest+1] = szMATCH[kTest]; kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// [B] we expect:
	// [prop 7][len 20][Line 1...Line 2...Li]
	// [prop 7][len 12][XXXXYYYYZZZZ]
	// [prop 7][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);

	//////////////////////////////////////////////////////////////////
	// *** basic undo tests ***
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// test  : one undo
	// expect: doc identical to state [A] above
	undo();
	kTest--;

	wxASSERT_MSG( (_count_frags()==6), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );
	wxASSERT_MSG( (m_pFragHead->getFragProp()==(PROP_BIT_3)), _T("TESTFAILED") );
	wxASSERT_MSG( (m_pFragHead->m_next->getFragProp()==(0)), _T("TESTFAILED") );

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : one redo
	// expect: doc identical to state [B]
	redo();
	kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : change prop across a series of frags
	// expect: lots of coalescing
	turnOffProp(m_pFragHead,0,szMATCH[kTest].Len(),(PROP_BIT_3|PROP_BIT_1));
	szMATCH[kTest+1] = szMATCH[kTest]; kTest++;
	wxASSERT_MSG( (_count_frags()==3), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc_length()==szMATCH[kTest].Len()), _T("TESTFAILED") );
	wxASSERT_MSG( (_test_compact()==0), _T("TESTFAILED") );
	wxASSERT_MSG( (_get_doc()==szMATCH[kTest]), _T("TESTFAILED") );

	//////////////////////////////////////////////////////////////////
	// [B] we expect:
	// [prop 2][len 20][Line 1...Line 2...Li]
	// [prop 2][len 12][XXXXYYYYZZZZ]
	// [prop 2][len 128][ne 3...Line 4...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...AAAA...BBBB...CCCC...DDDD...]

	dump(0);
	
	//////////////////////////////////////////////////////////////////
	// test  : undo all the way back to the start

	long kTestLimit = kTest;

	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST: undo to beginning"));
	while (kTest > 0)
	{
		undo();
		wxASSERT_MSG( (_get_doc()==szMATCH[--kTest]), _T("TESTFAILED") );
	}

	dump(0);

	//////////////////////////////////////////////////////////////////
	// test  : redo all the way back to end

	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST: redo to end"));
	while (kTest < kTestLimit)
	{
		redo();
		wxASSERT_MSG( (_get_doc()==szMATCH[++kTest]), _T("TESTFAILED") );
	}

	dump(0);

	wxLogTrace(TRACE_PTABLE_DUMP, _T("TEST02: END"));

	//////////////////////////////////////////////////////////////////
	// *** un-register viewer
	//////////////////////////////////////////////////////////////////

	delChangeCB(::_cb_frag_test,this);
	delStatusCB(::_cb_stat_test,this);
}
#endif
#endif
