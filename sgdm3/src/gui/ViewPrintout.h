// ViewPrintout.h
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWPRINTOUT_H
#define H_VIEWPRINTOUT_H

//////////////////////////////////////////////////////////////////

typedef void ViewPrintoutFileData;

//////////////////////////////////////////////////////////////////

class ViewPrintout : public wxPrintout
{
public:
	ViewPrintout(const wxString & strTitle)
		: wxPrintout(strTitle)
		{};
	virtual ~ViewPrintout(void) {};

	void						initializeCoordinateSystem(wxDC * dc);
	void						allocateFonts(GlobalProps::EnumGPS eKey,
											  wxFont ** ppFontNormal,
											  wxFont ** ppFontBold=NULL,
											  wxFont ** ppFontBoldUnderline=NULL);	// caller must free these

	static wxString				_elide(wxString strInput, int wAvailable, wxDC * dc);

	virtual void				OnPreparePrinting(void) = 0;
	virtual void				GetPageInfo(int * minPage, int * maxPage, int * pageFrom, int * pageTo) = 0;
	virtual void				OnBeginPrinting(void) = 0;
	virtual void				OnEndPrinting(void) = 0;
	virtual bool				OnBeginDocument(int startPage, int endPage) = 0;
	virtual void				OnEndDocument(void) = 0;
	virtual bool				HasPage(int page) = 0;
	virtual bool				OnPrintPage(int kPage) = 0;

protected:
	int m_xCoordMax;
	int m_yCoordMax;
	int m_xCoordPerInch;
	int m_yCoordPerInch;
};

//////////////////////////////////////////////////////////////////

class ViewPrintoutFile : public ViewPrintout
{
public:
	ViewPrintoutFile(const wxString & strTitle, ViewFile * pView, ViewPrintoutFileData * pData=NULL);
	virtual ~ViewPrintoutFile(void);

	inline ViewPrintoutFileData *		getData(void) const { return m_pFileData; };

    virtual void				OnPreparePrinting(void);
	virtual void				GetPageInfo(int * minPage, int * maxPage, int * pageFrom, int * pageTo);
    virtual void				OnBeginPrinting(void);
    virtual void				OnEndPrinting(void);
    virtual bool				OnBeginDocument(int startPage, int endPage);
    virtual void				OnEndDocument(void);
    virtual bool				HasPage(int page);
	virtual bool				OnPrintPage(int kPage);

	void						drawPageHeaderFooter(wxDC * dc, int kPage, int kPanel,
													 wxString strPageTitle,
													 wxString strCol,
													 wxString strFooter,
													 bool bJustMeasure,
													 int * pyCoordBodyTop, int * pyCoordBodyBottom,
													 int * pxCoordBodyLeft, int * pxCoordLineNrRight);

protected:
	void						_drawString(wxDC * dc,
											const wxChar * sz, long len,
											int cTabStop,
											wxCoord & xCoord, wxCoord yCoord, int & col);
	wxString					_getModeString(void);

	ViewFile *					m_pView;
	ViewPrintoutFileData *		m_pFileData;
	long						m_kSync;
	int							m_cRowsPerPage;
	int							m_cPages;
	int							m_bShowLineNumbers;
	int							m_cDigitsLineNumbers[3];
};

//////////////////////////////////////////////////////////////////

class ViewPrintoutFileDiff : public ViewPrintoutFile
{
public:
	ViewPrintoutFileDiff(ViewFileDiff * pView, ViewPrintoutFileData * pData=NULL);
};

class ViewPrintoutFileMerge : public ViewPrintoutFile
{
public:
	ViewPrintoutFileMerge(ViewFileMerge * pViewMerge, ViewPrintoutFileData * pData=NULL);
};

//////////////////////////////////////////////////////////////////

class ViewPrintoutFolder : public ViewPrintout
{
public:
	ViewPrintoutFolder(ViewFolder * pView, ViewPrintoutFolderData * pData=NULL);
	virtual ~ViewPrintoutFolder(void);

	inline ViewPrintoutFolderData *		getData(void) const { return m_pFolderData; };

    virtual void				OnPreparePrinting(void);
	virtual void				GetPageInfo(int * minPage, int * maxPage, int * pageFrom, int * pageTo);
    virtual void				OnBeginPrinting(void);
    virtual void				OnEndPrinting(void);
    virtual bool				OnBeginDocument(int startPage, int endPage);
    virtual void				OnEndDocument(void);
    virtual bool				HasPage(int page);
	virtual bool				OnPrintPage(int kPage);

	void						drawPageHeaderFooter(wxDC * dc, int kPage,
													 wxString strPageTitle,
													 wxString strCol1, wxString strCol2,
													 wxString strFooter,
													 bool bJustMeasure,
													 int * pyCoordBodyTop, int * pyCoordBodyBottom);

protected:
	ViewFolder *				m_pView;
	ViewPrintoutFolderData *	m_pFolderData;
	int							m_cRowsPerPage;
	int							m_cPages;
};

//////////////////////////////////////////////////////////////////
// MyPreviewFrame -- a simple wrapper around wxPreviewFrame to let
// us preserve window coordinates in global props.

class MyPreviewFrame : public wxPreviewFrame
{
public:
	MyPreviewFrame(wxPrintPreview * pPreview, wxWindow * pParent, const wxString & strTitle)
		: wxPreviewFrame(pPreview,pParent,strTitle,
						 _suggestInitialPosition(),_suggestInitialSize())
		{
		};
	virtual ~MyPreviewFrame(void)
		{
			if (IsIconized())		// if minimized, don't record our position/size because it's not what we want to remember.
				return;

			if (IsMaximized())		// if maximized, don't record position/size.
				return;				// TODO consider remembering the fact that we were maximized.
	
			int x,y,w,h;

			GetPosition(&x,&y);
			GetSize(&w,&h);

			//wxLogTrace(wxTRACE_Messages, _T("PrintPreview: remember Position/Size %d,%d %d,%d"), x,y,w,h);

			gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_X, x);
			gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_Y, y);
			gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_W, w);
			gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_H, h);
		};

private:
	static wxPoint	_suggestInitialPosition(void)
		{
#if 1
			// WXBUG?  2013/08/30 See W5268.
			// WXBUG?  With 2.9.5 (at least on MAC) non-default values for the initial
			// WXBUG?  position behave weirdly.  (And usually off-screen somehow -- which
			// WXBUG?  since the print preview is kind of app-modal.)
			// WXBUG?
			// WXBUG?  It may be that having 2 monitors confuses things and may depdend
			// WXBUG?  on their relative positions.
			// WXBUG?
			// WXBUG?  I just noticed that wxPreviewFrame defaults to wxFRAME_FLOAT_ON_PARENT.
			// WXBUG?  I don't know if that affects how the coordinates are interpreted or not.
			// WXBUG?
			// WXBUG?  So I'm going to just use default values for the intial position
			// WXBUG?  and not try to restore it next time.

			return wxDefaultPosition;
#else
			long x = gpGlobalProps->getLong(GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_X);
			long y = gpGlobalProps->getLong(GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_Y);

			// on MSW, if a window is minimized when it is closed, we get -32000 for the x,y
			// values when we ask for the window's position.  (ideally, we shouldn't update
			// the position in global props when minimized, but it might happen).  tweak the
			// x,y values to insure that we are on-screen.

			long deltaX = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X) * 2; // picked somewhat at random
			long deltaY = wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y) * 2;
			long   maxX = wxSystemSettings::GetMetric(wxSYS_SCREEN_X) - deltaX * 10;
			long   maxY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) - deltaY * 10;

			x = MyMax(x,0);
			x = MyMin(x,maxX);
	
			y = MyMax(y,0);
			y = MyMin(y,maxY);

			//wxLogTrace(wxTRACE_Messages,_T("PrintPreview: initialPosition %ld,%ld"),x,y);

			return wxPoint(x,y);
#endif
		};

	static wxSize	_suggestInitialSize(void)
		{
			long w = gpGlobalProps->getLong(GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_W);
			long h = gpGlobalProps->getLong(GlobalProps::GPL_WINDOW_SIZE_PRINT_PREVIEW_H);

			//wxLogTrace(wxTRACE_Messages,_T("PrintPreview: initialSize %ld,%ld"),w,h);

			return wxSize(w,h);
		};
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWPRINTOUT_H
