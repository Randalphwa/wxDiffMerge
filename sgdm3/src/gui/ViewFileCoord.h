// ViewFileCoord.h -- represent an anchor/selection position in a
// document.
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWFILECOORD_H
#define H_VIEWFILECOORD_H

//////////////////////////////////////////////////////////////////

class ViewFileCoord
{
public:
	ViewFileCoord(void)								{ clear();      };
	ViewFileCoord(int r, int c)						{ set(r,c);     };
	ViewFileCoord(const ViewFileCoord & ref)		{ set(ref);     };

	void					clear(void);
	void					set(const ViewFileCoord & ref);
	void					set(int r, int c);
	int						compare(const ViewFileCoord & ref) const;
	int						compare(int r, int c) const;

	inline int				getRow(void)	const 	{ return m_r; };
	inline int				getCol(void)	const	{ return m_c; };
	inline bool				isSet(void)		const	{ return m_bSet; };

private:
	int						m_r;
	int						m_c;
	bool					m_bSet;
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWFILECOORD_H
