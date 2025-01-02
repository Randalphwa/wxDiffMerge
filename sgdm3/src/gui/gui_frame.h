// gui_frame.h
// a frame represents a top-level window.
//////////////////////////////////////////////////////////////////

#ifndef H_GUI_FRAME_H
#define H_GUI_FRAME_H

//////////////////////////////////////////////////////////////////

class gui_frame : public wxFrame
{
private:
	friend class FrameFactory;
	friend class dlg_nag__base;		// lets us steal menu declarations
	gui_frame(const wxString & caption, ToolBarType tbt=TBT_BASIC);

public:
	virtual ~gui_frame(void);

	bool			isFolder(const fd_fd * pFdFd=NULL)		const;
	bool			isFileDiff(const fs_fs * pFsFs=NULL)	const;
	bool			isFileMerge(const fs_fs * pFsFs=NULL)	const;

	bool			isEmptyFrame(void)						const;
	void			revertToEmptyFrame(void);

	util_error		finishLoadingFiles(void);

	void			setCaption(const wxString & strCaption);
	const wxString &getCaption(void)						const	{ return m_strCaption; };

	void			setCaptionStatus(const wxChar ch);

	bool			isEditableFileFrame(void)				const;
	bool			isEditableFileDirty(void)				const;

	void			clearInsertMarkDialog(void)				{ m_pDlgModelessInsertMark = NULL; };
	bool			haveInsertMarkerDialog(void)			const	{ return (m_pDlgModelessInsertMark != NULL); };
	dlg_insert_mark        *getInsertMarkerDialog(void)			const	{ return m_pDlgModelessInsertMark; };
	void			showInsertMarkDialog(de_mark * pDeMarkInitial=NULL);

	inline View            *getView(void)						const	{ return m_pView; };

	bool			haveModelessDialog(void)				const;

	wxString		dumpSupportInfo(const wxString & strIndent)		const;

	void			postEscapeCloseCommand(void);

	void			sendEventToView(wxEvent *pEvent) const;

#include <gui_frame__event_table.h>


public:
	void HACK_remember_drop_target(const wxArrayString & asPathnames);
	void HACK_start_drop_target_timer(void);
	void HACK_on_drop_target_timer(wxTimerEvent & e);
#define HACK__ID_DROP_TARGET 1234
private:
	wxArrayString * HACK__m_pasPathnamesDropTarget;
	wxTimer HACK__m_timerDropTarget;


protected:
	DECLARE_EVENT_TABLE();

#include <gui_frame__iface_defs.h>

private:
	void _loadFolders(  const wxString & s0, const wxString & s1, const cl_args * pArgs=NULL);
	void _loadFileDiff( const wxString & s0, const wxString & s1, const cl_args * pArgs=NULL);
	void _loadFileMerge(const wxString & s0, const wxString & s1, const wxString & s2, const cl_args * pArgs=NULL);
	void _onEdit__delta_or_conflict(bool bNext, bool bConflict);
	bool _onUpdateEdit__delta_or_conflict(bool bNext, bool bConflict);
	void _setTitle(void);
	bool _cleanupEditPanel(bool bCanVeto, bool bDirty);
	void _onUpdateViewFileShow__dop(wxUpdateUIEvent & e, de_display_ops dop);

	int _getDefaultAction(PanelIndex kPanel);
	void _onUpdateViewFileEditApplyDefaultAction(wxUpdateUIEvent & e, PanelIndex kPanel);
	void _onViewFileEditApplyDefaultAction(wxCommandEvent & e, PanelIndex kPanel);

private:
	wxMenuBar *		m_pMenuBar;
	wxMenu *		m_pMenu_File;
	wxMenu *		m_pMenu_Edit;
	wxMenu *		m_pMenu_View;
	wxMenu *		m_pMenu_Export;
	wxMenu *		m_pMenu_Settings;
	wxMenu *		m_pMenu_Help;

	wxToolBar *		m_pToolBar;
	ToolBarType		m_tbt;
	ToolBarType		m_tbtMenu;

	Doc *			m_pDoc;
	View *			m_pView;
	wxString		m_strCaption;
	wxChar			m_chCaptionStatus;

	dlg_insert_mark *	m_pDlgModelessInsertMark;

	wxPoint			m_posRestored;		// non-maximized position of frame
	wxSize			m_sizeRestored;		// non-maximized size of frame

public:
	static wxSize		suggestInitialSize(ToolBarType tbt);
	static wxPoint		suggestInitialPosition(ToolBarType tbt);
	static bool			suggestInitialMaximized(ToolBarType tbt);
	static ToolBarType	suggestInitialType(const cl_args * pArgs);
	static void			verifyVisible(wxPoint * pPosFrame, wxSize * pSizeFrame);
	static void			computeCascadedPosition(wxPoint * pPosFrame, const wxSize * pSizeFrame);
	void				rememberOurSize(void);
	void				rememberOurPosition(void);
	wxPoint				getRestoredPosition(void) const { return m_posRestored; };

private:
	class EmptyView : public wxWindow
	{
	public:
		EmptyView(gui_frame * pFrame);
		virtual ~EmptyView();

	private:
		void			onHtmlLinkClicked(wxHtmlLinkEvent &e);
		void			onKeyDownEvent(wxKeyEvent & e);

	private:
		DECLARE_EVENT_TABLE();

	private:
		typedef enum {	ID_HTML_WINDOW_EMPTY_VIEW = 100 } ID;

		gui_frame               *m_pFrame;
	};

	EmptyView *			 m_pEmptyView;
};

//////////////////////////////////////////////////////////////////

#endif//H_GUI_FRAME_H
