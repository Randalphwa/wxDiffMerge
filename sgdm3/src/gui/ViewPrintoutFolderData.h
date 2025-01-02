// ViewPrintoutFolderData.h
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWPRINTOUTFOLDERDATA_H
#define H_VIEWPRINTOUTFOLDERDATA_H

//////////////////////////////////////////////////////////////////

class ViewPrintoutFolderData
{
public:
	ViewPrintoutFolderData(const fd_fd * pFdFd);
	~ViewPrintoutFolderData(void);		// use delRef() rather than delete

public:
	inline void					addRef(void)				{ m_refCnt++; };
	inline bool					delRef(void)				{ m_refCnt--; if (m_refCnt==0) { delete this; return true; } else { return false; } };

	inline const wxString		getStats(void)				const { return m_strStats; };
	
	inline long					getItemCount(void)			const { return (long)m_vec.size(); };
	inline fd_item::Status		getItemStatus(int kItem)	const { return m_vec[kItem]->m_status; };
	inline const wxString 		getItemStrLeft(int kItem)	const { return m_vec[kItem]->m_strLeft; };
	inline const wxString 		getItemStrRight(int kItem)	const { return m_vec[kItem]->m_strRight; };
	inline bool					getItemHaveLeft(int kItem)	const { return m_vec[kItem]->m_bHaveLeft; };
	inline bool					getItemHaveRight(int kItem)	const { return m_vec[kItem]->m_bHaveRight; };

private:
	class _item
	{
	public:
		_item(fd_item * pFdItem)
			{
				m_status     = pFdItem->getStatus();
				m_strLeft    = pFdItem->getRelativePathname(0);
				m_strRight   = pFdItem->getRelativePathname(1);
				m_bHaveLeft  = pFdItem->getPoiItem(0) != NULL;
				m_bHaveRight = pFdItem->getPoiItem(1) != NULL;
			};
		
	public:
		fd_item::Status		m_status;
		wxString			m_strLeft;
		wxString			m_strRight;
		bool				m_bHaveLeft;
		bool				m_bHaveRight;
	};

private:
	typedef std::vector<_item *>	TVec;

	TVec			m_vec;
	int				m_refCnt;
	wxString		m_strStats;
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWPRINTOUTFOLDERDATA_H
