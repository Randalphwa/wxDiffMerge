// ViewPrintout.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////
// WARNING: the coordinate transformations to get printing to work
// WARNING: are rather tricky -- and, in my opinion, non-obvious.
// WARNING: [but if the wxWidgets folks can get this working at
// WARNING: all and cross-platform, tis a small price to pay.]
//
// let me try to summarize what (i think) i've figured out.
//
// after the various print-/page-setup dialogs have run
// and the user hits OK, the print manager starts calling
// us with page geometry already set up.  that is, they
// take care of paper size and margins and etc and give us
// the dimensions of the interior rectangle.
//
// printing and print-preview both work thru the same mechanism;
// the only difference is the DC backing them -- to a printer,
// a bitmap, or memory, or etc.  ***AND THIS IS WHERE THE
// CONFUSUION BEGINS***
// 
// GetPPIPrinter() and GetPPIScreen() return the resolution
// the currently selected printer and the screen -- regardless
// of the output destination.
//
// GetPageSizeMM() returns the dimensions in MM of the paper.
// this is an absolute and gives the same value for printing
// and preview.
//
// GetPageSizePixels() returns the size of the paper in pixels
// on the currently selected printer.  this is another absolute
// value (same for printing and preview).
//
// dc->GetSize() returns the size of the paper on the device
// and this value depends upon the device and the OS.
//   on Win32: when actually printing, say on a 1200dpi laser
//             printer, you'll get values like 9600x12800 for
//             8.5x11 paper.  on print-preview, you'll get
//             values like 240x320 (at 10% zoom), 480x640 (at
//             20% zoom), and etc, on a 96dpi screen.
//   on Mac:
//   on GTK:
//
// dc->GetPPI() returns the resolution of the DC.
//   on Win32: this value depends upon the device.  when printing,
//             you'll get values like 1200.  when previewing,
//             you'll get values like 96.
//   on Mac:
//   on GTK:
//
// when printing starts, the DC has a 1-to-1 coordinate/pixel
// mapping.  so a 100x100 rectangle will be *tiny* on a laser
// printer and will be large on the preview (and will change
// size as the preview zoom factor is adjusted).
//
// THE BIG PROBLEM IS FONTS.  fonts are specified in POINT
// sizes (where a 72pt font should be 1 inch tall).  it appears
// that they're using the size as a pixel value (or something)
// because fonts come out microscopic on paper and varying
// sizes during preview (again depending upon the zoom factor).
//
// TO FIX THE FONT PROBLEM, we need to install a coordinate
// transformation into the DC -- see SetUserScale() -- this is
// tricky to explain (see samples/printing/printing.cpp in the
// wxWidgets source tree (although it's not really that helpful)).
// [] basically, we need to request a font in screen-logical-units.
//    (i'm thinking this is a bug, but oh well.)  so, there are
//    2 parts:
//    [1] scale up from screen-logical-units to printer-logical-units.
//    [2] when previewing, scale down to the bitmap DC.
//
//             ppi-printer     dc-pixels
//    scale =  -----------  x  --------------
//             ppi-screen      printer-pixels
//
// when printing, the right-term is one and the left is a simple
// coordinate transform.
//
// when previewing, the right-term takes care of the bitmap.
//
// with this transformation installed, fonts work.  a 72pt font
// is *approximately* 1 inch on paper and scaled properly within
// a preview.  it's not exactly 1 inch (more like 60-66pt), but
// it's close enough.
//
// WARNING: changing the scale factor *DOES NOT* change the
// WARNING: values returned by dc->GetSize() or dc->getPPI().
//
// after installing the transformation, drawing must be done
// in screen-units -- so a 1 inch line is ppi-screen units.
// and we have to compute upper bounds ourselves (because
// dc->GetSize() is not updated).  i'm going to call these
// "coords" in what follows.
//////////////////////////////////////////////////////////////////

void ViewPrintout::initializeCoordinateSystem(wxDC * dc)
{
	//////////////////////////////////////////////////////////////////
	// this must be called for each page -- because DC can change
	// between pages -- for example, when playing with the zoom
	// factor on the preview window.
	//////////////////////////////////////////////////////////////////
	
	int wPagePixels,hPagePixels;
	int wDCPixels,	hDCPixels;
	int wPpiPrinter,hPpiPrinter;
	int wPpiScreen,	hPpiScreen;

	GetPageSizePixels(&wPagePixels,&hPagePixels);
	dc->GetSize(&wDCPixels,&hDCPixels);
	GetPPIPrinter(&wPpiPrinter,&hPpiPrinter);
	GetPPIScreen(&wPpiScreen,&hPpiScreen);

//	wxLogTrace(wxTRACE_Messages, _T("PageSize: pixels [%d,%d]; DC Size [%d,%d]"),
//			   wPagePixels,hPagePixels, wDCPixels,hDCPixels);
//	wxLogTrace(wxTRACE_Messages, _T("PPI: Printer [%d,%d] Screen [%d,%d]"),
//			   wPpiPrinter,hPpiPrinter, wPpiScreen,hPpiScreen);

	double dScaleX = ((double)(wPpiPrinter * wDCPixels)) / ((double)(wPpiScreen * wPagePixels));
	double dScaleY = ((double)(hPpiPrinter * hDCPixels)) / ((double)(hPpiScreen * hPagePixels));
	dc->SetUserScale(dScaleX,dScaleY);

//	wxLogTrace(wxTRACE_Messages, _T("Scale: [%g,%g]"), dScaleX,dScaleY);

	//////////////////////////////////////////////////////////////////
	// here is our coordinate system
	//////////////////////////////////////////////////////////////////

	m_xCoordMax     = wPagePixels * wPpiScreen / wPpiPrinter;
	m_yCoordMax     = hPagePixels * hPpiScreen / hPpiPrinter;
	m_xCoordPerInch = wPpiScreen;
	m_yCoordPerInch = hPpiScreen;
	
//	wxLogTrace(wxTRACE_Messages, _T("Coords: max [%d,%d] per-inch [%d,%d]"),
//			   m_xCoordMax,m_yCoordMax, m_xCoordPerInch,m_yCoordPerInch);
}

//////////////////////////////////////////////////////////////////

void ViewPrintout::allocateFonts(GlobalProps::EnumGPS eKey,
								 wxFont ** ppFontNormal,
								 wxFont ** ppFontBold,
								 wxFont ** ppFontBoldUnderline)
{
	// caller must free these fonts

	// TODO replace this with a printer font

	wxASSERT_MSG( (ppFontNormal), _T("Coding Error!") );
	
	wxFont * pFont = gpGlobalProps->createNormalFont(eKey);
	*ppFontNormal = pFont;

	if (ppFontBold)
	{
		wxFont * pFontBold = new wxFont(pFont->GetPointSize(),pFont->GetFamily(),pFont->GetStyle(),
										wxFONTWEIGHT_BOLD,false,pFont->GetFaceName(),wxFONTENCODING_SYSTEM);
		*ppFontBold = pFontBold;
	}

	if (ppFontBoldUnderline)
	{
		wxFont * pFontBoldUnderline = new wxFont(pFont->GetPointSize(),pFont->GetFamily(),pFont->GetStyle(),
												 wxFONTWEIGHT_BOLD,true,pFont->GetFaceName(),wxFONTENCODING_SYSTEM);
		*ppFontBoldUnderline = pFontBoldUnderline;
	}
}

//////////////////////////////////////////////////////////////////

/*static*/ wxString ViewPrintout::_elide(wxString strInput, int wAvailable, wxDC * dc)
{
	wxCoord wText,hText;
	dc->GetTextExtent(strInput,&wText,&hText);
	if (wText < wAvailable)
		return strInput;

	// we do a real stupid job -- just chop from the left as necessary.
	// this is intended for pathnames, and will elide the root/parent
	// directories first and save the individual file name for last.

	wxString strDot(_T("..."));
	wxCoord wDot,hDot;
	dc->GetTextExtent(strDot,&wDot,&hDot);
				
	wxString work = strInput;
	wxArrayInt ai;
	dc->GetPartialTextExtents(work,ai);
	wxCoord wLast = ai.Last();
	size_t nrLast = ai.GetCount() - 1;
	size_t nr;
				
	for (nr=0; (nr < nrLast); nr++)
		if (wDot + wLast - ai.Item(nr) < wAvailable)
			break;

	wxString strResult = strDot + work.Mid(nr+1);

	return strResult;
}
