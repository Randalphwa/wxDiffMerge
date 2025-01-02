// util_cbl.h
// a generic callback list.
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_CBL_H
#define H_UTIL_CBL_H

//////////////////////////////////////////////////////////////////
// util_cbl_arg -- extra arg for callback.  usage depends upon
// context.  (avoids us having another layer of templates.)

class util_cbl_arg
{
public:
	util_cbl_arg(void * p=NULL, long l=0) : m_p(p), m_l(l) {};
	
	void *		m_p;
	long		m_l;
};

typedef void (*util_cbl_fn)(void *pData, const util_cbl_arg & arg);

class util_cbl
{
private:
	class _item
	{
	public:
		_item(util_cbl_fn pfn, void * pData) : m_pfn(pfn), m_pData(pData) {};
		util_cbl_fn		m_pfn;
		void *			m_pData;
	};

	typedef std::vector<_item *>		TVec;
	typedef TVec::iterator				TVecIterator;
	typedef TVec::const_iterator		TVecConstIterator;

	TVec				m_vec;

public:
	util_cbl(void);
	~util_cbl(void);

	void				addCB(util_cbl_fn pfn, void * pData);
	void				delCB(util_cbl_fn pfn, void * pData);

	void				callAll(const util_cbl_arg & arg);

	long				count(void) const;

#ifdef DEBUG
public:
	void				dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_CBL_H
