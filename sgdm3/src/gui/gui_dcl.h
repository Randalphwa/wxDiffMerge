// gui_dcl.h -- forward declarations for this directory
//////////////////////////////////////////////////////////////////

#ifndef H_GUI_DCL_H
#define H_GUI_DCL_H

//////////////////////////////////////////////////////////////////

#define MY_EXIT_STATUS__OK				0
#define MY_EXIT_STATUS__ABORT_MERGE		1	// interactive abort of 3-way
#define MY_EXIT_STATUS__DIFFERENT		1	// batch detected diffences
#define MY_EXIT_STATUS__FILE_ERROR		2	// some type of IO error
#define MY_EXIT_STATUS__CLARG_SYNTAX	3	// bogus arguments on command line

#define MY_EXIT_STATUS_TYPE__STATIC		0	// exit status is fixed
#define MY_EXIT_STATUS_TYPE__MERGE		1	// exit status waiting on merge result status

//////////////////////////////////////////////////////////////////

extern wxBitmap _src2png2bmp(const unsigned char * szSrc);

typedef enum _toolbar_type { TBT_BASIC=0, TBT_FOLDER, TBT_DIFF, TBT_MERGE, __TBT__COUNT__ } ToolBarType;

typedef enum _findResult
{
	FindResult_NoMatch,		// nothing found
	FindResult_Match,		// match found
	FindResult_BadRegEx,	// invalid regular expression
	FindResult_CantRegExBackwards,	// can't regex search backwards
	FindResult_EmptyString,	// no search string given
} FindResult;

//////////////////////////////////////////////////////////////////
// notebook style interface
// we define 2 notebook pages:
// [0] is the reference -- the documents as loaded
// [1] is the editing page -- where editing takes place
//

//typedef enum _nbIndex	{	NB_REF   =0,
//							NB_EDIT  =1,
//							__NR_NB__=2	}	NBIndex;

//////////////////////////////////////////////////////////////////

class AboutBox;
class Doc;
class FrameFactory;
class Glance;
class Glance2;
class Glance3;
class MessageOutputString;
class MyPreviewFrame;
class MyStaticText;
class OptionsDlg;
class SupportDlg;
//class TaskListPanel;
//class TaskListPanel_AutoMerge;
class View;
class ViewFile;
class ViewFileCoord;
class ViewFileDiff;
class ViewFileFont;
class ViewFileMerge;
class ViewFilePanel;
class ViewFileStatusText;
class ViewFolder;
class ViewFolder_ImageList;
class ViewFolder_ListCtrl;
class ViewFolder_ListItemAttr;
class ViewPrintout;
class ViewPrintoutFile;
class ViewPrintoutFileDiff;
class ViewPrintoutFileMerge;
class ViewPrintoutFolder;
class ViewPrintoutFolderData;
class cl_args;
class dlg_auto_merge;
class dlg_auto_merge_result_summary;
class dlg_insert_mark;
class dlg_insert_mark_help;
class dlg_open;
class gui_app;
class gui_frame;


//////////////////////////////////////////////////////////////////

#include <cl_args.h>

//////////////////////////////////////////////////////////////////

#if 0 && defined(_DEBUG)
class DebugMemoryStats
{
public:
	typedef struct _li
	{
		const wxChar * szName;
		unsigned long nrCur;
		unsigned long nrPeak;
		size_t size;
	} _li;

	typedef enum _t
	{
		T_FIM_FRAG=0,
		T_FL_RUN,
		T_FL_LINE,
		T_DE_LINE,
		T_DE_SYNC,
		T_DE_SYNC_LIST,
		T_DE_SPAN,
		T_DE_ROW,
		__NR_LI__			// must be last
	} _t;

	_li saLi[__NR_LI__];

	void ZeroStats(void)
		{
			memset(saLi,0,sizeof(saLi));
		};

	void ctor(_t t,const wxChar * sz, size_t s)
		{
			saLi[t].szName=(sz);
			saLi[t].nrCur++;
			saLi[t].nrPeak = MyMax(saLi[t].nrCur,saLi[t].nrPeak);
			saLi[t].size=s;
		};

	void dtor(_t t)
		{
			saLi[t].nrCur--;
		}
	
	void DumpStats(void)
		{
#if 0 && defined(DEBUG)
			wxLogTrace(wxTRACE_Messages,_T("===================================================================="));
			size_t sum = 0;
			for (int k=0; k<__NR_LI__; k++)
			{
				wxLogTrace(wxTRACE_Messages,_T("DebugMemoryStats[%2d][%20s]:\t[size %4d][cur %10d][peak %10d][totalCur %10d][%5d]"),
						   k,saLi[k].szName,saLi[k].size,saLi[k].nrCur,saLi[k].nrPeak,
						   saLi[k].size*saLi[k].nrCur,
						   saLi[k].size*saLi[k].nrCur/(1024*1024));
				sum += saLi[k].size*saLi[k].nrCur;
			}
			wxLogTrace(wxTRACE_Messages,_T("DebugMemoryStats: [Sum %10d][%5d]"),sum,sum/(1024*1024));
#endif
		};
	
};
extern DebugMemoryStats gDebugMemoryStats;

#	define DEBUGMEMORYSTATS
#	define DEBUG_CTOR(T,sz)		Statement( DebugMemoryStats::_t t=DebugMemoryStats::T; gDebugMemoryStats.ctor(t,sz,sizeof(*this)); )
#	define DEBUG_DTOR(T)		Statement( DebugMemoryStats::_t t=DebugMemoryStats::T; gDebugMemoryStats.dtor(t); )
#else
#	undef DEBUGMEMORYSTATS
#	define DEBUG_CTOR(T,sz)
#	define DEBUG_DTOR(T)
#endif

//////////////////////////////////////////////////////////////////

#endif//H_GUI_DCL_H
