// ViewFilePanel__paint.h
// portion of ViewFilePanel primarily concerned with painting window.
//////////////////////////////////////////////////////////////////

public:
	int					getPixelsPerCol(void);
	int					getPixelsPerRow(void);
	int					getXPixelLineNr(void);
	int					getXPixelLineNrRight(void);
	int					getXPixelIcon(void);
	int					getXPixelBar(void);
	int					getXPixelText(void);
	int					getDigitsLineNr(void);
	int					getColsDisplayable(void);		// returns {width,height} of document area in
	int					getRowsDisplayable(void);		// {columns,rows} based upon the current font.

	void				OnPaint(wxPaintEvent & e);

protected:
	void				_OnPaint(wxPaintEvent & e);
	void				_recalc(void);
	void				_prepareStringForDrawing(const wxChar * sz, long len,
												 bool bDisplayInvisibles, int cColTabWidth, int & col,
												 wxString & strOut);
	void				_drawCaret(wxDC & dc, int x, int y);
	void				_drawString(wxDC & dc,
									const wxChar * sz, long len,
									bool bDisplayInvisibles, int cColTabWidth,
									const wxColour & clrFg, const wxColour & clrBg,
									wxCoord & x, wxCoord y, int & col, int row);
	void				_drawLongString(wxDC & dc,
										const wxChar * sz, long len,
										bool bDisplayInvisibles, int cColTabWidth,
										const wxColour & clrFg, const wxColour & clrBg,
										wxCoord & x, wxCoord y, int & col, int row);
	void				_fakeDrawString(wxDC & dc,
										const wxChar * sz, long len,
										bool bDisplayInvisibles, int cColTabWidth,
										wxCoord & x, int & col);
	int					_computeDigitsRequiredForLineNumber(void) const;

private:
	bool				m_bRecalc;
	int					m_rowsDisplayable, m_colsDisplayable;	// nr {rows,cols} in client area
	int					m_pixelsPerRow,    m_pixelsPerCol;		// cell size
	int					m_nrDigitsLineNr;
	int					m_xPixelLineNr, m_xPixelLineNrRight, m_xPixelIcon, m_xPixelBar, m_xPixelText;	// left margin stuff
