// ConfigMacros.h
//////////////////////////////////////////////////////////////////

#ifndef H_CONFIGMACROS_H
#define H_CONFIGMACROS_H

//////////////////////////////////////////////////////////////////

#ifndef MyMax
#	define MyMax(a,b)		(((a)>(b)) ? (a) : (b))
#	define MyMin(a,b)		(((a)<(b)) ? (a) : (b))
#endif

#ifndef NrElements
#	define NrElements(a)	((sizeof(a))/(sizeof(a[0])))
#endif

#ifndef Statement
#	define Statement( s )	do { s } while (0)
#endif

#ifndef DELETEP
#	define DELETEP(p)		Statement( if (p) { delete p; p = NULL; } )
#endif

#ifndef FREEP
#	define FREEP(p)			Statement( if (p) { free(p); p = NULL; } )
#endif

//////////////////////////////////////////////////////////////////
// DELETE_LIST -- delete elements of an inline (m_next/m_prev ptr style) linked list
// and zero pointer given.

#ifndef DELETE_LIST
#	define DELETE_LIST(t,pHead)		Statement(	t * pTemp=(pHead);					\
											  	while (pTemp)						\
												{	t * pNext=pTemp->getNext();		\
													delete pTemp;					\
													pTemp=pNext;					\
												}									\
												pHead=NULL;							)
#endif

//////////////////////////////////////////////////////////////////
// CLEAR_STL_LIST -- delete the elements in a STL list of pointers and clear the list.

#ifndef CLEAR_STL_LIST
#	define CLEAR_STL_LIST(tit,l)	Statement(	for (tit it=(l).begin(); (it!=(l).end()); it++)		\
 												{	delete *it;										\
												}													\
												l.clear();											)
#endif

//////////////////////////////////////////////////////////////////

#ifndef BEGIN_EXTERN_C
#	ifdef __cplusplus
#		define BEGIN_EXTERN_C	extern "C" {
#		define END_EXTERN_C		}
#	else
#		define BEGIN_EXTERN_C	/**/
#		define END_EXTERN_C		/**/
#	endif /* __cplusplus */
#endif /* EXTERN_C */

//////////////////////////////////////////////////////////////////

typedef unsigned char byte;

//////////////////////////////////////////////////////////////////
// a stupid little macro to silence compiler warning on Win32.
// i tend to do the following:
//    bool bResult = _some_function(...)
//    wxASSERT_MSG((bResult),_T("Coding Error"));
// which works fine when debug is on, but causes VS2003 to complain
// that bResult was initialized but not referenced -- warning C4189.
// so rather than turn off the warning with a pragma (and miss actual
// errors), i'm going to try this.

#define MY_ASSERT_MSG(b,str)	Statement( if (!(b)) { wxASSERT_MSG((b),(str)); } )
#define MY_ASSERT(b)			Statement( if (!(b)) { wxASSERT_MSG((b),_T("Coding Error")); } )

//////////////////////////////////////////////////////////////////

#endif//H_CONFIGMACROS_H
