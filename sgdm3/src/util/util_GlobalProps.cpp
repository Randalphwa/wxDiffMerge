// util_GlobalProps.cpp
// interface to persistent config/prop info.
// there should only be one instance of this class.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

static wxConfig *		s_config	= NULL;		// generic form of wxConfig... that maps to platform-specific (registry/ini/config/etc)

//////////////////////////////////////////////////////////////////
// _val represents a single global property of a particular type.
// _val loads the persisted value from register/ini/etc when instantiated
// and saves it back to persistent storage when destroyed.
// 
// each global prop maintains a list of callbacks of objects that are
// interested in knowing when the value is changed.
// 
// we hide _val in a private class here to avoid complicating the
// util_GlobalProps.h header file.
//////////////////////////////////////////////////////////////////

template<class T, class E> class _val
{
private:
	util_cbl	m_cbl;

public:
	_val(E id, T def, const wxChar * szKey)
		: m_key(szKey),
		  m_id(id),
		  m_def(def)
		{
			m_bHadReg = s_config->Read(m_key,&m_reg);
			m_cur     = ( (m_bHadReg) ? m_reg : m_def );

#ifdef DEBUG
//			wxLogTrace(TRACE_GLOBALPROPS, wxString::Format(_T("GlobalProps::GlobalProps: [%s][hadReg %d]:"),m_key.wc_str(),m_bHadReg) << m_cur);
#endif
		};

	~_val(void)
		{
//			wxLogTrace(TRACE_GLOBALPROPS, wxString::Format(_T("GlobalProps::~GlobalProps: Final Value [%s]:"),m_key.wc_str()) << m_cur);
		};

private:
	void _update_registry(void) const
		{
#if 0
			// this version works nicely when we assume only one instance of
			// the application will be running.  we load up from the registry
			// when we launch and we write new values (either when we exit or
			// when we set a variable); we don't need to write the values that
			// haven't changed or that are just the builtin default.
			//
			// but this is less desirable when we have multiple instances running
			// at the same time.

			if (m_bHadReg)
			{
				if (m_cur != m_reg)
					s_config->Write(m_key,m_cur);
			}
			else
			{
				if (m_cur != m_def)
					s_config->Write(m_key,m_cur);
			}
#else
			// this version always write the new value (whether it is different
			// or not) so that the registry is in sync with our cache.  if another
			// instance of the app is started after our instance updates something,
			// it will see the new values -- as expected.

//			wxLogTrace(TRACE_GLOBALPROPS, wxString::Format(_T("GlobalProps::_update_registry: [%s]:"),m_key.wc_str()) << m_cur);

			s_config->Write(m_key,m_cur);
#endif
		};

public:
	T get(bool bDefault=false) const
		{
			if (bDefault)
				return m_def;
			else
				return m_cur;
		};

	T getUncached(bool bDefault=false)
		{
			if (bDefault)
				return m_def;

			m_bHadReg = s_config->Read(m_key,&m_reg);
			m_cur     = ( (m_bHadReg) ? m_reg : m_def );

			return m_cur;
		};
	
	T set(T vNew)
		{
			T vOld = m_cur;
			m_cur = vNew;

			_update_registry();
			
			if (vNew != vOld)		// only need to send change notification if the value actully changed.
				m_cbl.callAll( util_cbl_arg(NULL,(long)m_id) );
	
			return vOld;
		};

	wxString getKey(void) const
		{
			return m_key;
		};
	
	void addCB(util_cbl_fn pfn, void * pData)
		{
			m_cbl.addCB(pfn,pData);
		};

	void delCB(util_cbl_fn pfn, void * pData)
		{
			m_cbl.delCB(pfn,pData);
		};

public:
	wxString	m_key;			// registry key

	E			m_id;			// our ID (GPL_... or GPS_...)
	T			m_def;			// default value
	T			m_reg;			// loaded from registry/ini/config/etc
	T			m_cur;			// current value

	bool		m_bHadReg;		// we found a registry/ini/config/etc value
};

//////////////////////////////////////////////////////////////////

typedef _val<long,GlobalProps::EnumGPL>			TValLong;
typedef _val<wxString,GlobalProps::EnumGPS>		TValString;
	
typedef std::vector<TValLong *>					TVecLong;
typedef std::vector<TValString *>				TVecString;

typedef TVecLong::iterator						TVecLongIterator;
typedef TVecString::iterator					TVecStringIterator;

//////////////////////////////////////////////////////////////////
// we can make these static since GlobalProps is a singleton class.

static TVecLong *		s_longs		= NULL;		// our cache of long parameters
static TVecString *		s_strings	= NULL;		// our cache of string parameters

//////////////////////////////////////////////////////////////////

GlobalProps::GlobalProps(const wxChar * szAppName, const wxChar * szVendorName)
{
	wxConfigBase::DontCreateOnDemand();	// dont implictily create wxConfig, we'll take care of it.

	s_config = new wxConfig(szAppName,szVendorName);
	wxConfigBase::Set(s_config);

	s_longs = new TVecLong();
	s_strings = new TVecString();
	
	// defer most of our init until after the app is ready (see OnInit()).
}

void GlobalProps::OnInit(void)
{
//	wxLogTrace(TRACE_GLOBALPROPS, _T("GlobalProps::OnInit: creating."));

	// load all global props from the persistent registry/ini/config/etc
	// and/or load from built-in defaults.

#define GPL(id,path,def)	s_longs  ->push_back( new TValLong  ((GPL_##id),(def),(path)) );
#define GPS(id,path,def)	s_strings->push_back( new TValString((GPS_##id),(def),(path)) );
#include <util_GlobalProps__defs.h>
#undef GPL
#undef GPS

	wxASSERT_MSG( (s_longs  ->size() == __GPL__COUNT__), _T("Coding Error: GlobalProps:OnInit: s_longs.size() wrong.") );
	wxASSERT_MSG( (s_strings->size() == __GPS__COUNT__), _T("Coding Error: GlobalProps:OnInit: s_strings.size() wrong.") );
}

GlobalProps::~GlobalProps(void)
{
//	wxLogTrace(TRACE_GLOBALPROPS, _T("GlobalProps::~GlobalProps:: dumping prop cache..."));

	// delete each individual global prop.  this will cause
	// the value to be written out to persistent storage.

	for (TVecLongIterator itLong=s_longs->begin(); itLong<s_longs->end(); itLong++)
		delete (*itLong);

	for (TVecStringIterator itString=s_strings->begin(); itString<s_strings->end(); itString++)
		delete (*itString);

	DELETEP(s_longs);
	DELETEP(s_strings);

#if 1
	wxConfigBase::Set(NULL);
	DELETEP(s_config);
#else	
	s_config = NULL;	// we DO NOT delete this because we called Set(), so wxWidgets owns it.
#endif
}

//////////////////////////////////////////////////////////////////

wxColor GlobalProps::getColor(EnumGPL gpl, bool bDefault)	const
{
	// a simple wrapper around a 'long' (0x00RRGGBB) prop to convert it to a wxColor.

	long lv = getLong(gpl, bDefault);

	return wxColor( ((lv>>16)&0xff), ((lv>>8)&0xff), (lv&0xff) );
}

wxString GlobalProps::getColor_CSS(EnumGPL gpl, bool bDefault)	const
{
	// a simple wrapper around a 'long' (0x00RRGGBB) prop to convert it to a CSS color.

	long lv = getLong(gpl, bDefault);

	return wxString::Format( _T("#%06x"), lv);
}

void GlobalProps::setColor(EnumGPL gpl, wxColor & rColor)
{
	unsigned long lv = (rColor.Red() << 16) | (rColor.Green() << 8) | (rColor.Blue());

	setLong(gpl,(long)lv);
}

//////////////////////////////////////////////////////////////////

long GlobalProps::getLong(EnumGPL gpl, bool bDefault)								const	{ return (*s_longs)[gpl]->get(bDefault); }
long GlobalProps::getLongUncached(EnumGPL gpl, bool bDefault)								{ return (*s_longs)[gpl]->getUncached(bDefault); }
long GlobalProps::setLong(EnumGPL gpl, long lValue)											{ return (*s_longs)[gpl]->set(lValue); }
wxString GlobalProps::getLongKey(EnumGPL gpl)										const	{ return (*s_longs)[gpl]->getKey(); }
void GlobalProps::addLongCB(EnumGPL gpl, util_cbl_fn pfn, void * pData)						{ (*s_longs)[gpl]->addCB(pfn,pData); }
void GlobalProps::delLongCB(EnumGPL gpl, util_cbl_fn pfn, void * pData)						{ (*s_longs)[gpl]->delCB(pfn,pData); }

wxString GlobalProps::getString(EnumGPS gps, bool bDefault)							const	{ return (*s_strings)[gps]->get(bDefault); }
wxString GlobalProps::getStringUncached(EnumGPS gps, bool bDefault)							{ return (*s_strings)[gps]->getUncached(bDefault); }
wxString GlobalProps::setString(EnumGPS gps, wxString sValue)								{ return (*s_strings)[gps]->set(sValue); }
wxString GlobalProps::getStringKey(EnumGPS gps)										const	{ return (*s_strings)[gps]->getKey(); }
void GlobalProps::addStringCB(EnumGPS gps, util_cbl_fn pfn, void * pData)					{ (*s_strings)[gps]->addCB(pfn,pData); }
void GlobalProps::delStringCB(EnumGPS gps, util_cbl_fn pfn, void * pData)					{ (*s_strings)[gps]->delCB(pfn,pData); }

wxString GlobalProps::saveFont(EnumGPS gps, const wxFont * pFont)							{ return setString(gps, util_font_create_spec_from_font(pFont)); }

//////////////////////////////////////////////////////////////////

wxFont * GlobalProps::createNormalFont(EnumGPS gps, bool bDefault) const
{
	// convenience wrapper for gettting a font from a font-spec string stored in a global prop.
	// 
	// load a font using the font descriptor in global props.
	//
	// WE RETURN A POINTER TO A FONT THAT THE CALLER OWNS AND MUST FREE EVENTUALLY.

	wxFont * pFont = util_font_create_normal_font_from_spec( getString(gps,bDefault) );
	if (pFont)
		return pFont;

	// if the descriptor is bogus ***OR*** the font metrics are
	// incomplete, try to silently fallback to a sane font.
	
	pFont = util_font_create_normal_font(10,wxFONTFAMILY_TELETYPE,_T(""));
	if (pFont)
		return pFont;
	
	pFont = util_font_create_normal_font(10,wxFONTFAMILY_DEFAULT,_T(""));
	return pFont;
}

wxFont * GlobalProps::createBoldFont(EnumGPS gps, bool bDefault) const
{
	// convenience wrapper for gettting a font from a font-spec string stored in a global prop.
	// 
	// load a font using the font descriptor in global props.
	//
	// WE RETURN A POINTER TO A FONT THAT THE CALLER OWNS AND MUST FREE EVENTUALLY.

	wxFont * pFont = util_font_create_bold_font_from_spec( getString(gps,bDefault) );
	if (pFont)
		return pFont;

	// if the descriptor is bogus ***OR*** the font metrics are
	// incomplete, try to silently fallback to a sane font.
	
	pFont = util_font_create_bold_font(10,wxFONTFAMILY_TELETYPE,_T(""));
	if (pFont)
		return pFont;
	
	pFont = util_font_create_bold_font(10,wxFONTFAMILY_DEFAULT,_T(""));
	return pFont;
}

//////////////////////////////////////////////////////////////////

#if defined(DEBUG)
static void _dump_print_data(const wxPrintData * pData, const char * pszLabel)
{
	wxLogTrace(TRACE_GLOBALPROPS, _T("PrintData: %s"),          pszLabel);
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetBin %d]"),			pData->GetBin());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetCollate %d]"),		pData->GetCollate());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetColor %d]"),		pData->GetColour());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetDuplex %d]"),		(int)pData->GetDuplex());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetNoCopies %d]"),		pData->GetNoCopies());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetOrientation %d]"),	pData->GetOrientation());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetPaperId %d]"),		pData->GetPaperId());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[PrinterName %s]"),		pData->GetPrinterName().wc_str());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetQuality %d]"),		pData->GetQuality());
	wxLogTrace(TRACE_GLOBALPROPS, _T("\t[OK %d]"),				pData->Ok());
}

#endif

wxPrintData * GlobalProps::createPrintData(void) const
{
	wxPrintData * pData = new wxPrintData;

	// load values from individual global prop values

	pData->SetBin(                 (wxPrintBin) getLong(  GPL_PRINT_DATA_BIN) );
	pData->SetCollate(                          getBool(  GPL_PRINT_DATA_COLLATE) );
	pData->SetColour(                           getBool(  GPL_PRINT_DATA_COLOR) );
	pData->SetDuplex(            (wxDuplexMode) getLong(  GPL_PRINT_DATA_DUPLEX) );
	pData->SetNoCopies(                         getLong(  GPL_PRINT_DATA_COPIES) );
	pData->SetOrientation( (wxPrintOrientation) getLong(  GPL_PRINT_DATA_ORIENTATION) );
	pData->SetPaperId(            (wxPaperSize) getLong(  GPL_PRINT_DATA_PAPER_ID) );
	pData->SetPrinterName(                      getString(GPS_PRINT_DATA_PRINTER_NAME) );
	pData->SetQuality(         (wxPrintQuality) getLong(  GPL_PRINT_DATA_QUALITY) );

#if 1 && defined(DEBUG)
	_dump_print_data(pData, "Loading...");
#endif

	return pData;
}

void GlobalProps::savePrintData(wxPrintData * pData)
{
#if 1 && defined(DEBUG)
	_dump_print_data(pData, "Saving...");
#endif

	// write values to individual global prop values

	setLong(  GPL_PRINT_DATA_BIN,			pData->GetBin());
	setBool(  GPL_PRINT_DATA_COLLATE,		pData->GetCollate());
	setBool(  GPL_PRINT_DATA_COLOR,			pData->GetColour());
	setLong(  GPL_PRINT_DATA_DUPLEX,		pData->GetDuplex());
	setLong(  GPL_PRINT_DATA_COPIES,		pData->GetNoCopies());
	setLong(  GPL_PRINT_DATA_ORIENTATION,	pData->GetOrientation());
	setLong(  GPL_PRINT_DATA_PAPER_ID,		pData->GetPaperId());
	setString(GPS_PRINT_DATA_PRINTER_NAME,	pData->GetPrinterName());
	setLong(  GPL_PRINT_DATA_QUALITY,		pData->GetQuality());
}

wxPageSetupData * GlobalProps::createPageSetupData(void) const
{
	wxPageSetupData * pData = new wxPageSetupData;

	// load values from individual global prop values

	wxPoint pt;

	pt.x = getLong(GPL_PAGE_SETUP_MARGIN_LEFT);
	pt.y = getLong(GPL_PAGE_SETUP_MARGIN_TOP);
	pData->SetMarginTopLeft(pt);

	pt.x = getLong(GPL_PAGE_SETUP_MARGIN_RIGHT);
	pt.y = getLong(GPL_PAGE_SETUP_MARGIN_BOTTOM);
	pData->SetMarginBottomRight(pt);

#if 1 && defined(DEBUG)
	wxLogTrace(TRACE_GLOBALPROPS, _T("PageSetupData: Builtin Defaults"));
	{ pt = pData->GetMarginTopLeft();			wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetMarginTopLeft %d,%d]"),			pt.x,pt.y); }
	{ pt = pData->GetMarginBottomRight();		wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetMarginBottomRight %d,%d]"),		pt.x,pt.y); }
#endif

	return pData;
}

void GlobalProps::savePageSetupData(wxPageSetupData * pData)
{
	// We only need to set the various margin values here
	// because our caller also does a savePrintData() which
	// has all of the other paper size/orientation details.

#if 1 && defined(DEBUG)
	wxLogTrace(TRACE_GLOBALPROPS, _T("PageSetupData: Saving"));
	{ wxPoint pt = pData->GetMarginTopLeft();			wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetMarginTopLeft %d,%d]"),			pt.x,pt.y); }
	{ wxPoint pt = pData->GetMarginBottomRight();		wxLogTrace(TRACE_GLOBALPROPS, _T("\t[GetMarginBottomRight %d,%d]"),		pt.x,pt.y); }
#endif

	// write values to individual global prop values

	wxPoint pt;

	pt = pData->GetMarginTopLeft();
	setLong(GPL_PAGE_SETUP_MARGIN_LEFT,pt.x);
	setLong(GPL_PAGE_SETUP_MARGIN_TOP,pt.y);

	pt = pData->GetMarginBottomRight();
	setLong(GPL_PAGE_SETUP_MARGIN_RIGHT,pt.x);
	setLong(GPL_PAGE_SETUP_MARGIN_BOTTOM,pt.y);
}

//////////////////////////////////////////////////////////////////

void GlobalProps::loadCustomColorData(wxColourData * pData)
{
	wxString str = getString(GlobalProps::GPS_DIALOG_COLOR_CUSTOM_COLORS);
	wxStringTokenizer tkz(str,_T(":"),wxTOKEN_RET_EMPTY);

	int k = 0;
	while (tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();
		if (token.Length() == 6)		// RRGGBB
		{
			unsigned long value;
			if (token.ToULong(&value,16))
			{
				unsigned char red  = (unsigned char)((value >> 16) & 0xff);
				unsigned char green= (unsigned char)((value >>  8) & 0xff);
				unsigned char blue = (unsigned char)((value      ) & 0xff);
				pData->SetCustomColour(k,wxColour(red,green,blue));

				//wxLogTrace(wxTRACE_Messages,_T("Loading Custom Color: [%d] %s"),k,token.wc_str());
			}
		}

		k++;
	}
}

void GlobalProps::saveCustomColorData(wxColourData * pData)
{
	wxString str;

	for (int k=0; k<16; k++)
	{
		wxColour c = pData->GetCustomColour(k);
		if (c.Ok())
			str += wxString::Format(_T("%02x%02x%02x:"),c.Red(),c.Green(),c.Blue());
		else
			str += _T(":");
	}

	//wxLogTrace(wxTRACE_Messages,_T("Saving Custom Colors: %s"),str);

	setString(GlobalProps::GPS_DIALOG_COLOR_CUSTOM_COLORS,str);
}

//////////////////////////////////////////////////////////////////

wxString GlobalProps::dumpSupportInfo(void) const
{
	wxString strMessage;

	strMessage += _T("Global Properties: (current / default)\n");
	for (int kProp=0; kProp<GlobalProps::__GPL__COUNT__; kProp++)
	{
		wxString strKey = getLongKey( (GlobalProps::EnumGPL)kProp );
		// the global props code was written when a long was 4 bytes.
		// the columns don't line up right on a 64-bit machine when
		// there are -1's in the data.
		int vCur = (int)getLong( (GlobalProps::EnumGPL)kProp );
		int vDef = (int)getLong( (GlobalProps::EnumGPL)kProp, true );
		strMessage += wxString::Format(_T("\t%40.40s : %10d [0x%08x] / %10d [0x%08x]\n"),
									   strKey.wc_str(),
									   vCur,vCur,
									   vDef,vDef);
	}
	for (int kProp=0; kProp<GlobalProps::__GPS__COUNT__; kProp++)
	{
		wxString strKey = getStringKey( (GlobalProps::EnumGPS)kProp );
		wxString strValueCur = getString( (GlobalProps::EnumGPS)kProp );
		wxString strValueDef = getString( (GlobalProps::EnumGPS)kProp, true );
		strMessage += wxString::Format(_T("\t%s :\n"),strKey.wc_str());

		if (kProp == GlobalProps::GPS_FILE_RULESET_SERIALIZED)
		{
			strMessage += _T("\t\tcur:\n");
			while (strValueCur.Length() > 0)
			{
				wxString strLeft = strValueCur.Mid(0,MyMin(40,strValueCur.Length()));
				strMessage += wxString::Format(_T("\t\t\t%s\n"),strLeft.wc_str());
				strValueCur = strValueCur.Mid(40);
			}
			strMessage += _T("\t\tdef:\n");
			while (strValueDef.Length() > 0)
			{
				wxString strLeft = strValueDef.Mid(0,MyMin(40,strValueDef.Length()));
				strMessage += wxString::Format(_T("\t\t\t%s\n"),strLeft.wc_str());
				strValueDef = strValueDef.Mid(40);
			}
		}
		else if (kProp == GlobalProps::GPS_LICENSE_KEY)
		{
			// W8784.
			// we shouldn't dump their license key into the stream.
			// lots of folks post the contents of the support dialog
			// to the forum when reporting problems.
			//
			// i'm going to just report whether the property is
			// blank or not -- don't care if valid.
			if (strValueCur.Length() > 0)
				strMessage += _T("\t\tcur:  __omitted__\n");
			else
				strMessage += _T("\t\tcur:\n");
			// there is no need for a "def:" value.
		}
		else
		{
			strMessage += wxString::Format(_T("\t\tcur: %s\n"),util_printable_s(strValueCur).wc_str());
			strMessage += wxString::Format(_T("\t\tdef: %s\n"),util_printable_s(strValueDef).wc_str());
		}
	}
	strMessage += _T("\n");

	return strMessage;
}

