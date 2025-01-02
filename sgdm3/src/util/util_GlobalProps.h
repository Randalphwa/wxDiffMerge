// util_GlobalProps.h
// interface to persistent config/prop info.
// there should only be one instance of this class.
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_GLOBALPROPS_H
#define H_UTIL_GLOBALPROPS_H

//////////////////////////////////////////////////////////////////

class GlobalProps
{
public:
	//////////////////////////////////////////////////////////////////
	// define IDs -- GPL_... and GPS_...
	//
	// clients should be able to do:
	// 
	// long lv = gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_EQUAL);
	// wxString sv = gpGlobalProps->getString(GlobalProps::GPS_FOLDER_SUFFIX_FILTER);
	// 
	//////////////////////////////////////////////////////////////////

	typedef enum _gpl {
#define GPL(id,path,def)	GPL_##id,
#define GPS(id,path,def)	/**/
#include <util_GlobalProps__defs.h>
		__GPL__COUNT__			// must be last
#undef GPL
#undef GPS
	} EnumGPL;

	typedef enum _gps {
#define GPL(id,path,def)	/**/
#define GPS(id,path,def)	GPS_##id,
#include <util_GlobalProps__defs.h>
		__GPS__COUNT__			// must be last
#undef GPL
#undef GPS
	} EnumGPS;

public:
	GlobalProps(const wxChar * szAppName, const wxChar * szVendorName);		// TODO make this private and have friend which creates all global classes.
	~GlobalProps(void);		// TODO make this private and have friend which creates all global classes.

private:
	friend class gui_app;
	void			OnInit(void); // defer most of construction until app is ready

public:
	long			getLong(EnumGPL gpl, bool bDefault=false) const;
	long			getLongUncached(EnumGPL gpl, bool bDefault=false);
	long			setLong(EnumGPL gpl, long lValue);
	wxString		getLongKey(EnumGPL gpl) const;
	void			addLongCB(EnumGPL gpl, util_cbl_fn pfn, void * pData);
	void			delLongCB(EnumGPL gpl, util_cbl_fn pfn, void * pData);

	wxString		getString(EnumGPS gps, bool bDefault=false) const;
	wxString		getStringUncached(EnumGPS gps, bool bDefault=false);
	wxString		setString(EnumGPS gps, wxString sValue);
	wxString		getStringKey(EnumGPS gps) const;
	void			addStringCB(EnumGPS gps, util_cbl_fn pfn, void * pData);
	void			delStringCB(EnumGPS gps, util_cbl_fn pfn, void * pData);

	wxColor			getColor(EnumGPL gpl, bool bDefault=false) const;		// convenience wrapper for getting a color from a long
	wxString		getColor_CSS(EnumGPL gpl, bool bDefault=false) const;	// return CSS-style color value
	void			setColor(EnumGPL gpl, wxColor & rColor);

	inline bool		getBool(EnumGPL gpl, bool bDefault=false) const { return (getLong(gpl,bDefault) == 1); };
	inline bool		getBoolUncached(EnumGPL gpl, bool bDefault=false) { return (getLongUncached(gpl,bDefault) == 1); };
	inline void		setBool(EnumGPL gpl, bool bValue)				{ setLong(gpl, ((bValue) ? 1 : 0)); };

	wxFont *		createNormalFont(EnumGPS gps, bool bDefault=false) const;	// convenience wrapper for gettting a font from a font-spec string (caller owns result)
	wxFont *		createBoldFont(EnumGPS gps, bool bDefault=false) const;		// convenience wrapper for gettting a font from a font-spec string (caller owns result)
	wxString		saveFont(EnumGPS gps, const wxFont * pFont);

	wxPrintData *			createPrintData(void) const;		// load print-data from global props (caller owns result)
	void					savePrintData(wxPrintData * pData);

	wxPageSetupData *		createPageSetupData(void) const;	// load page-setup-data from global props (caller owns result)
	void					savePageSetupData(wxPageSetupData * pData);

	void			loadCustomColorData(wxColourData * pData);	// load custom color data from global props
	void			saveCustomColorData(wxColourData * pData);	// save custom color data to global props
	
	wxString		dumpSupportInfo(void) const;
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_GLOBALPROPS_H
