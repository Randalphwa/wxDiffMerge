// util_bitmap_button_validator.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

BitmapButtonValidator::BitmapButtonValidator(wxColor * pClr, int margin)
	: wxValidator()
{
	m_pClr = pClr;
	m_margin = margin;
}

BitmapButtonValidator::BitmapButtonValidator(const BitmapButtonValidator & val)
	: wxValidator()
{
	Copy(val);
}

BitmapButtonValidator::~BitmapButtonValidator(void)
{
}

//////////////////////////////////////////////////////////////////

wxObject * BitmapButtonValidator::Clone(void) const
{
	return new BitmapButtonValidator(*this);
}
	
bool BitmapButtonValidator::Copy(const BitmapButtonValidator & val)
{
	wxValidator::Copy(val);

	m_pClr = val.m_pClr;
	m_margin = val.m_margin;

	return true;
}

void BitmapButtonValidator::_setButtonColor(void)
{
	wxBitmapButton * pButton = (wxBitmapButton *)m_validatorWindow;

	int w,h;
	pButton->GetClientSize(&w,&h);
	w -= m_margin;
	h -= m_margin;
	if ( (w < 0) || (h < 0) )
		return;

	wxBitmap bitmap(w,h);

	wxMemoryDC dc;
	dc.SelectObject(bitmap);
//	dc.SetPen(wxNullPen);	// causes assert/crash on GTK
	dc.SetBrush(wxBrush(*m_pClr,wxSOLID));
	dc.DrawRectangle(0,0,w,h);
	dc.SetBrush(wxNullBrush);
	dc.SelectObject(wxNullBitmap);

	pButton->SetBitmapLabel(bitmap);
}
		
bool BitmapButtonValidator::TransferToWindow(void)
{
	if (!m_validatorWindow)
		return false;

	_setButtonColor();
	return true;
}
	
bool BitmapButtonValidator::TransferFromWindow(void)
{
	return true;
}

bool BitmapButtonValidator::Validate(wxWindow *WXUNUSED(parent))
{
	return true;
}
