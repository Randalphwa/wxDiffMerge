/*
Copyright 2010-2013 SourceGear, LLC
*/

//////////////////////////////////////////////////////////////////

#ifndef H_KEY_LICENSE_H
#define H_KEY_LICENSE_H

char * sgdm_genkey(const char chDomain);
bool sgdm_checkkey(const char * pszKey);
int sgdm_maxkeylen(void);
const char * sgdm_keytemplate(void);

#endif // H_KEY_LICENSE_H
