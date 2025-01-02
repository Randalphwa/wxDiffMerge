// ViewFileCoord.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

void ViewFileCoord::clear(void)
{
	m_r  = 0;
	m_c = 0;
	m_bSet = false;
}

void ViewFileCoord::set(const ViewFileCoord & ref)
{
	m_r = ref.m_r;
	m_c = ref.m_c;
	m_bSet = ref.m_bSet;
}

void ViewFileCoord::set(int r, int c)
{
	m_r = r;
	m_c = c;
	m_bSet = true;
}

int ViewFileCoord::compare(const ViewFileCoord & ref) const
{
	wxASSERT_MSG( (m_bSet && ref.m_bSet), _T("Coding Error") );

	if (m_r < ref.m_r) return -1;
	if (m_r > ref.m_r) return +1;
	if (m_c < ref.m_c) return -1;
	if (m_c > ref.m_c) return +1;

	return 0;
}
	
int ViewFileCoord::compare(int r, int c) const
{
	wxASSERT_MSG( (m_bSet), _T("Coding Error") );

	if (m_r < r) return -1;
	if (m_r > r) return +1;
	if (m_c < c) return -1;
	if (m_c > c) return +1;

	return 0;
}
