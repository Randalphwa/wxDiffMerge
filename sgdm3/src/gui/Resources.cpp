// Resources.cpp
// a file to include/initialize all XPM's and PNG-HDR files
// so we don't get duplicate errors from the linker.
//
// all files should be included here -- and an
// "extern const char...." declaration be made in Resources.h.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <Resources.h>

//////////////////////////////////////////////////////////////////

#include <Artwork/DiffMergeIcon.xpm>

//////////////////////////////////////////////////////////////////
// Toolbars icons

#include <IconShock/hdr/IS_Vista_Data__fila_32__automerge.h>		// TB__AUTOMERGE
#include <IconShock/hdr/IS_Vista_Data__fila_32__automerge__dis.h>	// TB__AUTOMERGE__DIS
#include <IconShock/hdr/Custom__FileDiff_32.h>						// TB__FILE_DIFF
#include <IconShock/hdr/Custom__FileDiff_32__dis.h>					// TB__FILE_DIFF__DIS
#include <IconShock/hdr/Custom__FileMerge_32.h>						// TB__FILE_MERGE
#include <IconShock/hdr/IS_Vista_General__folder_32.h>				// TB__FOLDER_DIFF
#include <IconShock/hdr/IS_Vista_General__folder_32__dis.h>			// TB__FOLDER_DIFF__DIS
#include <IconShock/hdr/IS_Vista_General__folder_zoom_32.h>			// TB__FOLDER_FOLDERS
#include <IconShock/hdr/IS_Vista_General__folder_zoom_32__dis.h>	// TB__FOLDER_FOLDERS__DIS
#include <IconShock/hdr/Custom__FileEqual_32.h>						// TB__FOLDER_EQUAL
#include <IconShock/hdr/Custom__FileEquivalent_32.h>				// TB__FOLDER_EQUIVALENT
#include <IconShock/hdr/Custom__FileQuickMatch_32.h>				// TB__FOLDER_QUICKMATCH
#include <IconShock/hdr/Custom__FileNoPeer_32.h>					// TB__FOLDER_NOPEER
#include <IconShock/hdr/IS_Vista_Data__fila_32__dop_all.h>			// TB__DOP_ALL
#include <IconShock/hdr/IS_Vista_Data__fila_32__dop_all__dis.h>		// TB__DOP_ALL__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_32__dop_dif.h>			// TB__DOP_DIF
#include <IconShock/hdr/IS_Vista_Data__fila_32__dop_dif__dis.h>		// TB__DOP_DIF__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_32__dop_ctx.h>			// TB__DOP_CTX
#include <IconShock/hdr/IS_Vista_Data__fila_32__dop_ctx__dis.h>		// TB__DOP_CTX__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_down_32.h>				// TB__CHANGE_NEXT
#include <IconShock/hdr/IS_Vista_Data__fila_down_32__dis.h>			// TB__CHANGE_NEXT__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_up_32.h>				// TB__CHANGE_PREV
#include <IconShock/hdr/IS_Vista_Data__fila_up_32__dis.h>			// TB__CHANGE_PREV__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_down_32__conflict.h>		// TB__CONFLICT_NEXT
#include <IconShock/hdr/IS_Vista_Data__fila_down_32__conflict__dis.h>	// TB__CONFLICT_NEXT__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_up_32__conflict.h>			// TB__CONFLICT_PREV
#include <IconShock/hdr/IS_Vista_Data__fila_up_32__conflict__dis.h>		// TB__CONFLICT_PREV__DIS
#include <IconShock/hdr/Custom__Pilcrow_32.h>					// TB__PILCROW
#include <IconShock/hdr/Custom__LineNumbers_32.h>				// TB__LINE_NUMBERS
#include <IconShock/hdr/IS_Vista_General__scissors_32.h>		// TB__CUT
#include <IconShock/hdr/IS_Vista_General__scissors_32__dis.h>
#include <IconShock/hdr/IS_Vista_General__copy_32.h>			// TB__COPY
#include <IconShock/hdr/IS_Vista_General__copy_32__dis.h>
#include <IconShock/hdr/IS_Vista_General__paste_32.h>			// TB__PASTE
#include <IconShock/hdr/IS_Vista_General__paste_32__dis.h>
#include <IconShock/hdr/Custom__Undo_32.h>						// TB__UNDO
#include <IconShock/hdr/Custom__Undo_32__dis.h>
#include <IconShock/hdr/Custom__Redo_32.h>						// TB__REDO
#include <IconShock/hdr/Custom__Redo_32__dis.h>
#include <IconShock/hdr/IS_XMac_General__disk_32.h>				// TB__SAVE
#include <IconShock/hdr/IS_XMac_General__disk_32__dis.h>		// TB__SAVE__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_next_32.h>			// TB__APPLY_LEFT
#include <IconShock/hdr/IS_Vista_Data__fila_next_32__dis.h>		// TB__APPLY_LEFT__DIS
#include <IconShock/hdr/IS_Vista_Data__fila_back_32.h>			// TB__APPLY_RIGHT
#include <IconShock/hdr/IS_Vista_Data__fila_back_32__dis.h>		// TB__APPLY_RIGHT__DIS
#include <IconShock/hdr/Custom__SplitVertically_32.h>			// TB__SPLIT_VERTICALLY
#include <IconShock/hdr/Custom__SplitHorizontally_32.h>			// TB__SPLIT_HORIZONTALLY

//////////////////////////////////////////////////////////////////
// 2013/07/28 moved folder window line item xpm's to ViewFolder_ImageList.cpp 
//////////////////////////////////////////////////////////////////
