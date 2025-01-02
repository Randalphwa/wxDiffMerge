// util_bitmap_button_validator.h
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_BITMAP_BUTTON_VALIDATOR_H
#define H_UTIL_BITMAP_BUTTON_VALIDATOR_H

//////////////////////////////////////////////////////////////////

class BitmapButtonValidator : public wxValidator
{
public:
	BitmapButtonValidator(wxColor * pClr, int margin);
	BitmapButtonValidator(const BitmapButtonValidator & val);
	~BitmapButtonValidator(void);

	virtual wxObject *	Clone(void) const;
	
	bool				Copy(const BitmapButtonValidator & val);

	virtual bool		TransferToWindow(void);
	virtual bool		TransferFromWindow(void);
	virtual bool		Validate(wxWindow * parent);

protected:
	void				_setButtonColor(void);

private:
	wxColor *			m_pClr;
	int					m_margin;
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_BITMAP_BUTTON_VALIDATOR_H
