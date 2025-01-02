// gui.h
//////////////////////////////////////////////////////////////////

#ifndef H_GUI_H
#define H_GUI_H

//////////////////////////////////////////////////////////////////

#include <GEN_BuildNumber.h>
#include <branding.h>
#include <Resources.h>

//////////////////////////////////////////////////////////////////

#include <AboutBox.h>
#include <Doc.h>
#include <FrameFactory.h>
#include <Glance.h>
#include <MessageOutputString.h>
#include <MyBusyInfo.h>
#include <MyFindData.h>
#include <MyStaticText.h>
#include <SupportDlg.h>
//#include <TaskListPanel.h>
#include <View.h>
#include <ViewFile.h>
#include <ViewFileCoord.h>
#include <ViewFileFont.h>
#include <ViewFilePanel.h>
#include <ViewFolder_ImageList.h>
#include <ViewFolder_ListCtrl.h>
#include <ViewFolder_ListItemAttr.h>
#include <ViewFolder.h>
#include <ViewPrintout.h>
#include <ViewPrintoutFolderData.h>
#include <OptionsDlg.h>
#include <dlg_auto_merge.h>
#include <dlg_auto_merge_result_summary.h>
#include <dlg_insert_mark.h>
#include <dlg_open.h>
#include <dlg_open_autocomplete.h>
#include <gui_app.h>
#include <gui_frame.h>
#if defined(__WXMSW__)
#include <dlg_show_shortcut_details.h>
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
#include <dlg_show_symlink_details.h>
#endif

//////////////////////////////////////////////////////////////////

#define TRACE_GUI_DUMP		wxT("gui_dump")
#define TRACE_LICENSE		wxT("key_license")

//////////////////////////////////////////////////////////////////

extern MyBusyInfo *					gpMyBusyInfo;
extern MyFindData *					gpMyFindData;

extern FrameFactory *				gpFrameFactory;
extern ViewFileFont *				gpViewFileFont;
extern ViewFolder_ImageList *		gpViewFolderImageList;
extern ViewFolder_ListItemAttr *	gpViewFolderListItemAttr;

extern wxPrintData *				gpPrintData;
extern wxPageSetupData *			gpPageSetupData;

//////////////////////////////////////////////////////////////////

void _attr_to_color2_fg(de_attr attr, wxColour & fg);
bool _attr_to_color2_bg(bool bIntraLine, bool bIgnoreUnimportantChanges, de_attr attr, wxColour & bg);
void _attr_to_color3_fg(PanelIndex kPanel, de_attr attr, wxColour & fg);
bool _attr_to_color3_bg(bool bIntraLine, bool bIgnoreUnimportantChanges, PanelIndex kPanel, de_attr attr, wxColour & bg);

//////////////////////////////////////////////////////////////////

#endif//H_GUI_H
