// gui_app.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <rs.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <fs.h>
#include <fl.h>
#include <de.h>
#include <xt.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

util_error gui_app::_dumpDiffs(bool * pbHadChanges)
{
	*pbHadChanges = false;
	
	util_error err;
	util_logToString uLog(&err.refExtraInfo());	// prevent modal log/error dialogs
	{
		if (m_cl_args.bFolders)
		{
			//wxLogTrace(wxTRACE_Messages,_T("DumpDiffs: 2 folders given"));

			err = _batchoutput_folders2(pbHadChanges);
		}
		else if (m_cl_args.nrParams == 3)
		{
			//wxLogTrace(wxTRACE_Messages,_T("DumpDiffs: 3 files given"));

			// TODO not currently supported

			err.set(util_error::UE_UNSPECIFIED_ERROR);
		}
		else
		{
			//wxLogTrace(wxTRACE_Messages,_T("DumpDiffs: 2 files given"));

			err = _batchoutput_diff2(SYNC_VIEW,pbHadChanges);
		}
		
	}

	return err;
}


util_error gui_app::_batchoutput_diff2(long kSync, bool * pbHadChanges)
{
	de_de * pDeDe = NULL;
	byte * pBufRaw = NULL;
	size_t lenRaw;
	wxString strOut;

	util_error err;

	// load the files into our own doc.fs_fs and create a de_de on it.

	Doc doc;
	doc.initFileDiff(m_cl_args.pathname[0],m_cl_args.pathname[1],NULL);

	fs_fs * pFsFs = doc.getFsFs();
	err = pFsFs->loadFiles_without_asking();
	if (err.isErr())
		goto Finished;

#if defined(FEATURE_CLI_HTML_EXPORT)
	if (m_cl_args.bDumpHtml)	// one of our html views
	{
		de_display_ops dops;
		de_detail_level detailLevel;
		int colTabWidth = (int) gpGlobalProps->getLong(GlobalProps::GPL_VIEW_FILE_TABSTOP);
		
		if (m_cl_args.bDumpHtmlSxS)
			dops = m_cl_args.dopsHtmlSxS;	// one of [all,ctx,dif] controls how much content we show in a SxS view.
		else if (m_cl_args.bDumpUnified)
			dops = DE_DOP_CTX;				// unified diffs content in a unified view.
		else
			dops = DE_DOP_DIF;				// normal/traditional diffs content in a traditional view.

		if (m_cl_args.bDumpIgnoreUnimportant)
			dops |= DE_DOP_IGN_UNIMPORTANT;
		
		if (m_cl_args.bDumpHtmlIntraLine)
			detailLevel = DE_DETAIL_LEVEL__CHAR;
		else
			detailLevel = DE_DETAIL_LEVEL__LINE;

		pDeDe = new de_de(pFsFs, dops, dops, detailLevel);
		pDeDe->run();

		if (m_cl_args.bDumpHtmlSxS)
			pDeDe->batchoutput_html_sxs_diff2(kSync, strOut, pbHadChanges, colTabWidth);
		else if (m_cl_args.bDumpUnified)
			pDeDe->batchoutput_html_unified_diff2(kSync, strOut, pbHadChanges, colTabWidth);
		else
			pDeDe->batchoutput_html_traditional_diff2(kSync, strOut, pbHadChanges, colTabWidth);
	}
	else	// text view
#endif
	{
		de_display_ops dops;
		de_detail_level detailLevel = DE_DETAIL_LEVEL__LINE;
	
		if (m_cl_args.bDumpUnified)
			dops = DE_DOP_CTX;
		else
			dops = DE_DOP_DIF;
		
		if (m_cl_args.bDumpIgnoreUnimportant)
			dops |= DE_DOP_IGN_UNIMPORTANT;

		pDeDe = new de_de(pFsFs, dops, dops, detailLevel);
		pDeDe->run();

		if (m_cl_args.bDumpUnified)
			pDeDe->batchoutput_text_unified_diff2(kSync, strOut, pbHadChanges);
		else
			pDeDe->batchoutput_text_traditional_diff2(kSync, strOut, pbHadChanges);
	}
	
	//////////////////////////////////////////////////////////////////
	//
	// We are only called in BATCH MODE from the COMMAND LINE.
	// Historically, we never write to the -diff=path file when
	// there are no changes (we just exit with 0).  This was
	// originally modelled on gnu-diff.  W3956.
	// 
	//////////////////////////////////////////////////////////////////

	if (*pbHadChanges)
	{
		wxASSERT_MSG( (strOut.Length() > 0), _T("Coding Error") );

		// convert wide unicode buffer to 8bit.

#if defined(FEATURE_CLI_HTML_EXPORT)
		if (m_cl_args.bDumpHtml)
		{
			// when writing HTML always use UTF-8 since we put that in the meta-data tag.
			err = util_encoding_export_conv(strOut,wxFONTENCODING_UTF8,&pBufRaw,&lenRaw);
			if (err.isErr())
				goto Finished;
		}
		else
#endif
		{
			// convert to something appropriate for plain text
			// like gnu-diff would print on the console.  it is
			// not clear that there is a correct answer here (if
			// the 2 input files had different encodings, for
			// example).
			//
			// try the system default encoding first.  if that
			// fails, use utf8 -- i don't know what gnu diff
			// does in these cases....
			err = util_encoding_export_conv(strOut,wxFONTENCODING_DEFAULT,&pBufRaw,&lenRaw);
			if (err.isErr())
			{
				err = util_encoding_export_conv(strOut,wxFONTENCODING_UTF8,&pBufRaw,&lenRaw);
				if (err.isErr())
					goto Finished;
			}
		}

		// open and truncate output file only if we have changes to write to it.

		int fd = util_file_open(err,m_cl_args.diffOutput,wxFile::write);
		if (fd == -1)
			goto Finished;
		wxFile file(fd);	// attach fd to wxFile
		
		// write 8bit buffer to the given file

		if (!file.Write((const void *)pBufRaw,lenRaw))
		{
			err.set(util_error::UE_CANNOT_WRITE_FILE);
			goto Finished;
		}
	}

 Finished:
	DELETEP(pDeDe);
	FREEP(pBufRaw);

	return err;
}


util_error gui_app::_batchoutput_folders2(bool * pbHadChanges)
{
	util_error err;
	Doc doc;
	bool bHaveChanges;

	FD_EXPORT_FORMAT expfmt = FD_EXPORT__TO_FILE;
#if defined(FEATURE_CLI_HTML_EXPORT)
	if (m_cl_args.bDumpHtml)
		expfmt |= FD_EXPORT_FORMAT__HTML;
	else
#endif
		expfmt |= FD_EXPORT_FORMAT__RQ;
	// TODO do we want a batch folder to csv option?

	doc.initFolderDiff(m_cl_args.pathname[0], m_cl_args.pathname[1]);

	fd_fd * pFdFd = doc.getFdFd();

	// we always show different items.
	// we do not show EQUAL/EQUIVALENT items.
	// we show peerless items (files, folders, shortcuts).
	// we do not show folder pairs.
	// we show errors.
	pFdFd->setShowHideFlags(FD_SHOW_HIDE_FLAGS__SINGLES
							|FD_SHOW_HIDE_FLAGS__ERRORS);

	err = pFdFd->loadFolders(NULL, false);
	if (err.isErr())
		goto failed;

	bHaveChanges = pFdFd->haveChanges();
	if (bHaveChanges)
	{
		// when batch diffs are requested from the command line,
		// we only write to the -diff=filename path *IF* there are
		// changes -- this is to match our file-diff behavior.
		err = pFdFd->exportVisibleItems(NULL, expfmt, &m_cl_args.diffOutput);
		if (err.isErr())
			goto failed;
	}

	if (pbHadChanges)
		*pbHadChanges = bHaveChanges;

failed:
	return err;
}
