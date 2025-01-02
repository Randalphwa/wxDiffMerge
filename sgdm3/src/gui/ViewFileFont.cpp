// ViewFileFont.cpp
// manage the font we use to display all files in file-diff/-merge windows.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <ViewFileFont.h>

//////////////////////////////////////////////////////////////////

static void _cb_string(void * pThis, const util_cbl_arg & arg)
{
	GlobalProps::EnumGPS id = (GlobalProps::EnumGPS)arg.m_l;

	ViewFileFont * pViewFileFont = (ViewFileFont *)pThis;
	pViewFileFont->gp_cb_string(id);
}

//////////////////////////////////////////////////////////////////

ViewFileFont::ViewFileFont(void)
	: m_pFontNormal(NULL),
	  m_pFontBold(NULL)
{
	// constructor can't do much because wxWidgets is not up and running yet.
	// must wait until OnInit().
}

ViewFileFont::~ViewFileFont(void)
{
	gpGlobalProps->delStringCB(GlobalProps::GPS_VIEW_FILE_FONT, _cb_string, this); // unregister for notifications when VIEW_FILE_FONT changes.

	DELETEP(m_pFontNormal);
	DELETEP(m_pFontBold);
}

//////////////////////////////////////////////////////////////////

void ViewFileFont::OnInit(void)
{
	gpGlobalProps->addStringCB(GlobalProps::GPS_VIEW_FILE_FONT, _cb_string, this); // register for notifications when VIEW_FILE_FONT changes.


#if 0
#ifdef DEBUG
	// WXBUG on MAC, the fixedWidthOnly parameter of EnumerateXXX is ignored
	// WXBUG (the code to reference the argument is commented out in 2.6.1).
	{
		class MyFontEnumerator : public wxFontEnumerator
		{
		public:
			virtual bool OnFacename(const wxString & face)
				{
					wxLogTrace(wxTRACE_Messages,_T("Facename: [%s]"),face.wc_str());
					return true;	// continue enumeration
				};
			virtual bool OnFontEncoding(const wxString & face, const wxString & encoding)
				{
					wxLogTrace(wxTRACE_Messages,_T("FontEncoding: [%s][%s]"),face.wc_str(),encoding.wc_str());
					return true;	// continue enumeration
				};
		};

		MyFontEnumerator fe;
		fe.EnumerateFacenames(wxFONTENCODING_DEFAULT,true);
	}
#endif
#endif

	_load_from_gp();
}

//////////////////////////////////////////////////////////////////

void ViewFileFont::gp_cb_string(GlobalProps::EnumGPS id)
{
	// we get called when the VIEW_FILE_FONT global prop changes.
	// we need to re-render the font and notify all views.

	MY_ASSERT( (id==GlobalProps::GPS_VIEW_FILE_FONT) );

	_load_from_gp();
}

//////////////////////////////////////////////////////////////////

void ViewFileFont::_load_from_gp(void)
{
	//////////////////////////////////////////////////////////////////
	// load a font using the font descriptor in global props.  if the
	// descriptor is bogus ***OR*** the font metrics are incomplete,
	// fallback to a sane font.
	//////////////////////////////////////////////////////////////////

	wxFont * pFontNormal = gpGlobalProps->createNormalFont(GlobalProps::GPS_VIEW_FILE_FONT);
	wxFont * pFontBold   = gpGlobalProps->createBoldFont  (GlobalProps::GPS_VIEW_FILE_FONT);
	
	DELETEP(m_pFontNormal);
	DELETEP(m_pFontBold);

	m_pFontNormal = pFontNormal;
	m_pFontBold   = pFontBold;
	
	//////////////////////////////////////////////////////////////////
	// notify all views that the font has changes and that they need to re-layout/-paint.

	m_cbl.callAll( util_cbl_arg(this,0) );
}

