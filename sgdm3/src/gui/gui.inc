## gui.inc -- GNU Makefile for sgdm3/gui for Linux
##################################################################

GUI_HDRS=	AboutBox.h \
		Doc.h \
		FrameFactory.h \
		Glance.h \
		MessageOutputString.h \
		MyBusyInfo.h \
		MyFindData.h \
		MyStaticText.h \
		OptionsDlg.h \
		Resources.h \
		SupportDlg.h \
		TaskListPanel.h \
		View.h \
		ViewFile.h \
		ViewFile__iface_defs.h \
		ViewFile__scroll.h \
		ViewFile__splitter.h \
		ViewFile__subwin.h \
		ViewFileCoord.h \
		ViewFileFont.h \
		ViewFilePanel.h \
		ViewFilePanel__kb.h \
		ViewFilePanel__mouse.h \
		ViewFilePanel__paint.h \
		ViewFilePanel__selection.h \
		ViewFolder.h \
		ViewFolder_ImageList.h \
		ViewFolder_ListCtrl.h \
		ViewFolder_ListItemAttr.h \
		ViewPrintout.h \
		ViewPrintoutFolderData.h \
		cl_args.h \
		dlg_auto_merge.h \
		dlg_auto_merge_result_summary.h \
		dlg_insert_mark.h \
		dlg_open.h \
		dlg_open_autocomplete.h \
		dlg_show_symlink_details.h \
		gui.h \
		gui_app.h \
		gui_dcl.h \
		gui_frame.h \
		gui_frame__event_table.h \
		gui_frame__iface_defs.h

##################################################################

GUI_SRCS=	AboutBox.cpp \
		Doc.cpp \
		FrameFactory.cpp \
		Glance.cpp \
		MessageOutputString.cpp \
		MyBusyInfo.cpp \
		MyFindData.cpp \
		MyStaticText.cpp \
		OptionsDlg.cpp \
		Resources.cpp \
		SupportDlg.cpp \
		TaskListPanel.cpp \
		View.cpp \
		ViewFile.cpp \
		ViewFile__iface_defs.cpp \
		ViewFile__scroll.cpp \
		ViewFile__splitter.cpp \
		ViewFile__subwin.cpp \
		ViewFileCoord.cpp \
		ViewFileFont.cpp \
		ViewFilePanel.cpp \
		ViewFilePanel__kb.cpp \
		ViewFilePanel__kb_cua.cpp \
		ViewFilePanel__kb_mac.cpp \
		ViewFilePanel__mouse.cpp \
		ViewFilePanel__paint.cpp \
		ViewFilePanel__selection.cpp \
		ViewFolder.cpp \
		ViewFolder_ImageList.cpp \
		ViewFolder_ListCtrl.cpp \
		ViewFolder_ListItemAttr.cpp \
		ViewPrintout.cpp \
		ViewPrintoutFile.cpp \
		ViewPrintoutFolder.cpp \
		ViewPrintoutFolderData.cpp \
		cl_args.cpp \
		dlg_auto_merge.cpp \
		dlg_auto_merge_result_summary.cpp \
		dlg_insert_mark.cpp \
		dlg_open.cpp \
		dlg_open_autocomplete.cpp \
		dlg_show_symlink_details.cpp \
		gui_app.cpp \
		gui_app__batchoutput.cpp \
		gui_app__cl_args.cpp \
		gui_frame.cpp \
		gui_frame__event_table.cpp \
		gui_frame__events.cpp \
		gui_frame__events_closing.cpp \
		gui_frame__events_frame.cpp \
		gui_frame__iface_defs.cpp \
		gui_frame__iface_menu.cpp \
		gui_frame__iface_toolbar.cpp \
		gui_frame__iface_windowsize.cpp

##################################################################

GUI_RSCS=	Artwork/*.xpm \
		Resources/*.xpm \
		Resources/Banner/*.xpm \
		Resources/FindPanel/*.xpm \
		Resources/FolderWindow/*.xpm \
		Resources/PaidDialog/*.xpm

##################################################################

MY_CXX_FLAGS+=	-I../gui
MY_LD_LIBS+=	
MY_OBJDEPS+=	../gui/gui.inc $(GUI_HDRS) $(GUI_RSCS)
MY_LIBDEPS+=	../gui/gui.inc
MY_EXEDEPS+=	../gui/gui.inc
