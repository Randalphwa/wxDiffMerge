// ViewFolder_ListItemAttr.h
// maintain a global set of wxListItemAttr's for use by the wxListCtrl
// in the ViewFolder.  (since all folder views will use the same colors
// we can create only one share it.)
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWFOLDER_LISTITEMATTR_H
#define H_VIEWFOLDER_LISTITEMATTR_H

//////////////////////////////////////////////////////////////////
// there are 3 ways to set fonts on wxListCtrl's. [the docs are a little
// thin/obscure here.]
// 
// [a] we can set the font on the whole widget or
// [b] we can set the font attribute on each line item or
// [c] do both.
//
// which is best probably depends upon what you want:
// [] if everything should have the same font, just set it on the widget
// [] if different types of line items should have different fonts, use
//    the line items.
//
// FWIW, the column headers pick up the font of the widget.
//
// WXBUG setting the font on the widget on the mac changes the font on
// WXBUG column headers, but DOES NOT cause the header line height to
// WXBUG change -- that is determined by the installed "theme" (see 
// WXBUG wxGenericListCtrl in generic/listctrl.cpp).  so the column 
// WXBUG headers get clipped.  [this is not too bad if you just change
// WXBUG the face/family or up the point size from say 10 to 12, but it
// WXBUG looks really stupid if you load up something like 36pt...]
//
// WXBUG if you only set the per-line fonts (and not the widget font),
// WXBUG none of the row line heights get changed.  so both the column
// WXBUG headers and all line items get clipped.  this happens on all
// WXBUG platforms.

// Since I'm going to set it up for all line items to have the same font
// [as opposed to making changed files bold, for example], there's no
// need for the per-line-item fonts.

#define VIEW_FOLDER_LIST_CTRL_PERITEM_FONTS		0

#if   defined(__WXMSW__)
#define VIEW_FOLDER_LIST_CTRL_WIDGET_FONTS		1
#elif defined(__WXGTK__)
#define VIEW_FOLDER_LIST_CTRL_WIDGET_FONTS		1
#elif defined(__WXMAC__)
// turn off all font stuff in the wxListCtrl for ViewFolder since it
// doesn't mesh well with the mac's themes.
#define VIEW_FOLDER_LIST_CTRL_WIDGET_FONTS		0
#else
#error define platform for VIEW_FOLDER_LIST_CTRL font behaviour.
#endif

#define VIEW_FOLDER_LIST_CTRL_USE_FONTS			(VIEW_FOLDER_LIST_CTRL_WIDGET_FONTS | VIEW_FOLDER_LIST_CTRL_PERITEM_FONTS)

//////////////////////////////////////////////////////////////////

class ViewFolder_ListItemAttr
{
private:
	friend class ViewFolder_ListCtrl;
	ViewFolder_ListItemAttr(void);

public:
	~ViewFolder_ListItemAttr(void);

	wxListItemAttr *				getAttr(fd_item::Status s);

	void							addChangeCB(util_cbl_fn pfn, void * pData)	{ m_cbl.addCB(pfn,pData); };
	void							delChangeCB(util_cbl_fn pfn, void * pData)	{ m_cbl.delCB(pfn,pData); };

	void							gp_cb_long(GlobalProps::EnumGPL id);
	void							gp_cb_string(GlobalProps::EnumGPS id);

private:	
	void							_populate(void);

private:
	wxListItemAttr					m_attr[fd_item::__FD_ITEM_STATUS__COUNT__];
	util_cbl						m_cbl;				// callback list for objects wanting to know when we change.
	bool							m_bStale;			// need to reload

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
public:
	wxFont *						getFont(void);
private:
	wxFont *						m_pFont;			// we own this
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWFOLDER_LISTITEMATTR_H
