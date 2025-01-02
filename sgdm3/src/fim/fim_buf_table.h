// fim_buf_table.h
// a container of fim_buf.
// there should only be one global instance of this table.
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_BUF_TABLE_H
#define H_FIM_BUF_TABLE_H

//////////////////////////////////////////////////////////////////

class fim_buf_table
{
public:
	fim_buf_table(void);						// TODO make this private and have friend which creates all global classes.
	~fim_buf_table(void);						// TODO make this private and have friend which creates all global classes.

	fim_buf *				create(fim_length lenPreallocateHint=0);								// create new fim_buf (implicitly addRef())

	inline void				addRef(fim_buf * pBuf)	{ m_reftab.addRef(pBuf); };						// add reference to existing buf
	inline void				unRef(fim_buf * pBuf)	{ if (m_reftab.unRef(pBuf)==0) delete pBuf;  };	// release reference to buf -- delete if zero

private:
	util_reftab				m_reftab;

#ifdef DEBUG
public:
	void					dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_BUF_TABLE_H
