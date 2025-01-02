// fs_dcl.h -- forward declarations for this directory
//////////////////////////////////////////////////////////////////

#ifndef H_FS_DCL_H
#define H_FS_DCL_H

//////////////////////////////////////////////////////////////////
// the gui layer treats the panels as if a[__NR_SYNCS__][__NR_TOP_PANELS__]
// but T0 and T2 are identical on both notebook pages.

typedef enum _panel_index { PANEL_T0=0, PANEL_T1=1, PANEL_T2=2, __NR_TOP_PANELS__=3,

							PANEL_EDIT=PANEL_T1,		// WARNING PANEL_EDIT == PANEL_T1
} PanelIndex;

//////////////////////////////////////////////////////////////////

class fs_fs;
class fs_fs_table;

//////////////////////////////////////////////////////////////////

#endif//H_FS_DCL_H
