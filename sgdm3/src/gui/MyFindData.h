// Singleton class to store the last search string from the
// Find Dialog and/or the last "Use Selection for Find"
// request (and if possible on the MAC the last "Use Selection
// for Find" from another application).
//////////////////////////////////////////////////////////////////

#ifndef H_MYFINDDATA_H
#define H_MYFINDDATA_H

//////////////////////////////////////////////////////////////////

class MyFindData
{
public:
	MyFindData(void);
	virtual ~MyFindData(void);

	void set(const wxString & str);
	wxString get(void) const;

#if defined(__WXMAC__)
	// Mac now uses NSFindPboard
#else
	bool haveData(void) const;
protected:
	wxString m_str;
#endif // !MAC
};

//////////////////////////////////////////////////////////////////

#endif//H_MYFINDDATA_H
