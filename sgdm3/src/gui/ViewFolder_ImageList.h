// ViewFolder_ImageList.h
// maintain a global wxImageList for use by the wxListCtrl in the
// ViewFolder (since we can share the bitmaps, we only create one).
//
// See gpViewFolder_ImageList
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWFOLDER_IMAGELIST_H
#define H_VIEWFOLDER_IMAGELIST_H

//////////////////////////////////////////////////////////////////

class ViewFolder_ImageList : public wxImageList
{
private:
	friend class ViewFolder_ListCtrl;
	ViewFolder_ImageList(void);

public:
	~ViewFolder_ImageList(void);
	
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWFOLDER_IMAGELIST_H
