// ViewFilePanel.h
// Window for viewing/editing an individual text file -- one of
// the panels in a file-diff/-merge -- may be one of top or bottom
// panels.
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWFILEPANEL_H
#define H_VIEWFILEPANEL_H

//////////////////////////////////////////////////////////////////

class ViewFilePanel : public wxWindow
{
public:
	ViewFilePanel(wxWindow * pParent, long style, ViewFile * pViewFile, PanelIndex kPanel, long kSync);
	~ViewFilePanel(void);

	void				onSize(wxSizeEvent & e);
	void				onEraseBackground(wxEraseEvent & e);
	void				onSetFocusEvent(wxFocusEvent & e);
	void				onKillFocusEvent(wxFocusEvent & e);

	void				bind_dede_flfl(de_de * pDeDe, fl_fl * pFlFl);
	inline bool			isBound(void) const { return ((m_pDeDe != NULL) && (m_pFlFl != NULL)); };

	void				setShowLineNumbers(bool bOn);

	void				cb_font_change(void);
	void				cb_fl_changed(const util_cbl_arg & arg);
	void				cb_de_changed(const util_cbl_arg & arg);
	void				gp_cb_long(GlobalProps::EnumGPL id);

	inline int			getSync(void)		const { return m_kSync; };
	inline bool			isEditPanel(void)	const { return (m_kSync==SYNC_EDIT) && (m_kPanel==PANEL_EDIT); };

protected:

	ViewFile *			m_pViewFile;
	PanelIndex			m_kPanel;
	long				m_kSync;
	fl_fl *				m_pFlFl;
	de_de *				m_pDeDe;

	bool				m_bShowLineNumbers;
	int					m_nrTopPanels;

	wxTimer				m_timer_MyMouse;

public:
	typedef enum _ctx_id
	{
		CTX__NO_DEFAULT				= -1,
		CTX__NO_MENU				= -2,

		__CTX__START__				= wxID_HIGHEST + 100,

		CTX_DELETE					= wxID_HIGHEST + 101,

		CTX_INSERT_L				= wxID_HIGHEST + 102,
		CTX_INSERT_BEFORE_L			= wxID_HIGHEST + 103,
		CTX_INSERT_AFTER_L			= wxID_HIGHEST + 104,
		CTX_REPLACE_L				= wxID_HIGHEST + 105,

		CTX_INSERT_R				= wxID_HIGHEST + 106,
		CTX_INSERT_BEFORE_R			= wxID_HIGHEST + 107,
		CTX_INSERT_AFTER_R			= wxID_HIGHEST + 108,
		CTX_REPLACE_R				= wxID_HIGHEST + 109,

		CTX_DELETE_MARK				= wxID_HIGHEST + 110,

		CTX_CUT						= wxID_HIGHEST + 111,
		CTX_COPY					= wxID_HIGHEST + 112,
		CTX_PASTE					= wxID_HIGHEST + 113,

		CTX_NEXT_CHANGE				= wxID_HIGHEST + 114,
		CTX_PREV_CHANGE				= wxID_HIGHEST + 115,
		CTX_NEXT_CONFLICT			= wxID_HIGHEST + 116,
		CTX_PREV_CONFLICT			= wxID_HIGHEST + 117,

		CTX_SELECT_PATCH			= wxID_HIGHEST + 118,
		CTX_SELECT_ALL				= wxID_HIGHEST + 119,

		CTX_DIALOG_MARK				= wxID_HIGHEST + 120,

		__CTX__COUNT__				// must be last
	} _ctx_id;

protected:	
	DECLARE_EVENT_TABLE();

	//////////////////////////////////////////////////////////////////

#include <ViewFilePanel__kb.h>
#include <ViewFilePanel__mouse.h>
#include <ViewFilePanel__paint.h>
#include <ViewFilePanel__selection.h>

};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWFILEPANEL_H
