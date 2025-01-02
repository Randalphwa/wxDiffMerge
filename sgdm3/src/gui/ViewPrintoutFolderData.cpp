// ViewPrintoutFolderData.cpp
// captures state of folder window when print requested.  (this is
// necessary because the print preview window is another top level
// window independent of the folder window that raised it.)  it is
// also necessary because the folder window can change (manual or
// automatic reload) and/or toolbar/option dialog setting changes.
//
// this is ref-counted because the preview process takes 2 printout
// objects (one for the screen and one scaled for the printer) and
// we want both to use the same data (because it's expensive enough
// to create it the first time).
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

ViewPrintoutFolderData::ViewPrintoutFolderData(const fd_fd * pFdFd)
	: m_refCnt(1)
{
	m_strStats = pFdFd->formatStatsString();

	long cItems = pFdFd->getItemCount();
	for (long kItem=0; kItem<cItems; kItem++)
	{
		_item * pNew = new _item(pFdFd->getItem(kItem));

		m_vec.push_back(pNew);
	}
}

ViewPrintoutFolderData::~ViewPrintoutFolderData(void)
{
	long cItems = getItemCount();
	for (long kItem=0; kItem<cItems; kItem++)
	{
		_item * p = m_vec[kItem];
		delete p;
	}

	m_vec.clear();
}
