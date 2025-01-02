// ViewFileFont.h
// a singleton class to wrap/manage the font we use to draw files
// in file-diff/-merge windows.
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWFILEFONT_H
#define H_VIEWFILEFONT_H

//////////////////////////////////////////////////////////////////

class ViewFileFont
{
public:
	ViewFileFont(void);		// TODO protect this so that on one can get created
	~ViewFileFont(void);

	void					OnInit(void);

	void					addChangeCB(util_cbl_fn pfn, void * pData)	{ m_cbl.addCB(pfn,pData); };
	void					delChangeCB(util_cbl_fn pfn, void * pData)	{ m_cbl.delCB(pfn,pData); };

	inline const wxFont *	getNormalFont(void) const { return m_pFontNormal; };
	inline const wxFont *	getBoldFont(void)	const { return m_pFontBold;   };

private:
	void					_load_from_gp(void);

	wxFont *				m_pFontNormal;
	wxFont *				m_pFontBold;
	util_cbl				m_cbl;	// callback list for objects that want to know when we change

public:
	void					gp_cb_string(GlobalProps::EnumGPS id);
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWFILEFONT_H
