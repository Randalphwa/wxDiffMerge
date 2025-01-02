// Glance.h
// Window for drawing the files-at-a-glance.
//////////////////////////////////////////////////////////////////

#ifndef H_GLANCE_H
#define H_GLANCE_H

//////////////////////////////////////////////////////////////////

class Glance : public wxWindow
{
public:
	Glance(wxWindow * pParent, long style, ViewFile * pViewFile, long kSync);
	virtual ~Glance(void);

	void				OnPaint(wxPaintEvent & e);
	void				onSize(wxSizeEvent & e);
	void				onEraseBackground(wxEraseEvent & e);
	void				onMouseLeftDown(wxMouseEvent & e);
	void				onMouseLeftUp(wxMouseEvent & e);
	void				onMouseMotion(wxMouseEvent & e);
	void				onMouseLeaveWindow(wxMouseEvent & e);
//	void				onMouseEventWheel(wxMouseEvent & e);
//	void				onMouseMiddleDown(wxMouseEvent & e);

	void				bind_dede(de_de * pDeDe);

	void				cb_de_changed(const util_cbl_arg & arg);
	void				gp_cb_long(GlobalProps::EnumGPL id);

    virtual bool		AcceptsFocus(void) const { return false; };

protected:
	virtual void		_drawCompressed(wxAutoBufferedPaintDC & dc) = 0;
	virtual void		_drawExpanded(wxAutoBufferedPaintDC & dc)   = 0;

	void				_freePens(wxPen ** paPens);
	void				_freeBrushes(wxBrush ** paBrushes);

	void				_getFirstRowVisibleForBar(const de_sync ** ppSync, long * pOffsetInSync) const;
	void				_getLastRowVisibleForBar(PanelIndex kPanel, const de_sync ** ppSync, long * pOffsetInSync) const;

	void				_warp_scroll_on_y_mouse(long yClick);
	void				_show_line_numbers_in_status_bar_from_y_mouse(long yClick);
	void				_clear_status_bar_from_y_mouse(void);

protected:
	ViewFile *			m_pViewFile;	// we do not own this
	de_de *				m_pDeDe;		// we do not own this
	int					m_kSync;

	bool				m_bWeCapturedTheMouse;

protected:
	typedef std::vector<const de_sync *>	TVecSyncNode;
	typedef std::vector<long>				TVecSyncOffset;

	TVecSyncNode		m_vecHitSync;
	TVecSyncOffset		m_vecHitOffset;
	long				m_ndxHitEnd;

	void				_initHitVectors(int yPixelsClientSize);

	//////////////////////////////////////////////////////////////////

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

class Glance2 : public Glance
{
public:
	Glance2(wxWindow * pParent, long style, ViewFile * pViewFile, long kSync);
	virtual ~Glance2(void);

protected:
	virtual void		_drawCompressed(wxAutoBufferedPaintDC & dc);
	virtual void		_drawExpanded(wxAutoBufferedPaintDC & dc);

	void				_draw_bars(wxAutoBufferedPaintDC & dc, long yBarStart, long yBarEndT0, long yBarEndT1);

	void				_getPens(wxPen ** paPens);
	void				_getBrushes(wxBrush ** paBrushes);
};

//////////////////////////////////////////////////////////////////

class Glance3 : public Glance
{
public:
	Glance3(wxWindow * pParent, long style, ViewFile * pViewFile, long kSync);
	virtual ~Glance3(void);

protected:
	virtual void		_drawCompressed(wxAutoBufferedPaintDC & dc);
	virtual void		_drawExpanded(wxAutoBufferedPaintDC & dc);

	void				_draw_bars(wxAutoBufferedPaintDC & dc, long yBarStart, long yBarEndT0, long yBarEndT1, long yBarEndT2);

	void				_getPens(wxPen ** paPens);
	void				_getBrushes(wxBrush ** paBrushes);
};

//////////////////////////////////////////////////////////////////

#endif//H_GLANCE_H
