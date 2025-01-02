// Resources.h
// declare "external const char" symbols for all .xpm's.
// and include them from Resources.cpp
//////////////////////////////////////////////////////////////////

#ifndef H_RESOURCES_H
#define H_RESOURCES_H

//////////////////////////////////////////////////////////////////
// new names for images in gui/Artwork/*.xpm

extern const char * DiffMergeIcon_xpm[];

//////////////////////////////////////////////////////////////////
// new names for Toolbar icons in gui/IconShock/{hdr,png}/*

#   define TB__AUTO_MERGE				gsz_IS_Vista_Data__fila_32__automerge
#   define TB__AUTO_MERGE__DIS			gsz_IS_Vista_Data__fila_32__automerge__dis
#   define TB__FILE_DIFF				gsz_Custom__FileDiff_32
#   define TB__FILE_DIFF__DIS			gsz_Custom__FileDiff_32__dis
#   define TB__FILE_MERGE				gsz_Custom__FileMerge_32
#   define TB__FOLDER_DIFF				gsz_IS_Vista_General__folder_32
#   define TB__FOLDER_DIFF__DIS			gsz_IS_Vista_General__folder_32__dis
#   define TB__FOLDER_FOLDERS			gsz_IS_Vista_General__folder_zoom_32
#   define TB__FOLDER_FOLDERS__DIS		gsz_IS_Vista_General__folder_zoom_32__dis
#   define TB__FOLDER_EQUAL				gsz_Custom__FileEqual_32
#   define TB__FOLDER_EQUIVALENT		gsz_Custom__FileEquivalent_32
#	define TB__FOLDER_QUICKMATCH		gsz_Custom__FileQuickMatch_32
#   define TB__FOLDER_NOPEER			gsz_Custom__FileNoPeer_32
#   define TB__DOP_ALL					gsz_IS_Vista_Data__fila_32__dop_all
#   define TB__DOP_ALL__DIS				gsz_IS_Vista_Data__fila_32__dop_all__dis
#   define TB__DOP_DIF					gsz_IS_Vista_Data__fila_32__dop_dif
#   define TB__DOP_DIF__DIS				gsz_IS_Vista_Data__fila_32__dop_dif__dis
#   define TB__DOP_CTX					gsz_IS_Vista_Data__fila_32__dop_ctx
#   define TB__DOP_CTX__DIS				gsz_IS_Vista_Data__fila_32__dop_ctx__dis
#   define TB__CHANGE_NEXT				gsz_IS_Vista_Data__fila_down_32
#   define TB__CHANGE_NEXT__DIS			gsz_IS_Vista_Data__fila_down_32__dis
#   define TB__CHANGE_PREV				gsz_IS_Vista_Data__fila_up_32
#   define TB__CHANGE_PREV__DIS			gsz_IS_Vista_Data__fila_up_32__dis
#   define TB__CONFLICT_NEXT			gsz_IS_Vista_Data__fila_down_32__conflict
#   define TB__CONFLICT_NEXT__DIS		gsz_IS_Vista_Data__fila_down_32__conflict__dis
#   define TB__CONFLICT_PREV			gsz_IS_Vista_Data__fila_up_32__conflict
#   define TB__CONFLICT_PREV__DIS		gsz_IS_Vista_Data__fila_up_32__conflict__dis
#   define TB__PILCROW					gsz_Custom__Pilcrow_32
#   define TB__LINE_NUMBERS				gsz_Custom__LineNumbers_32
#   define TB__EDIT_CUT					gsz_IS_Vista_General__scissors_32
#   define TB__EDIT_CUT__DIS			gsz_IS_Vista_General__scissors_32__dis
#   define TB__EDIT_COPY				gsz_IS_Vista_General__copy_32
#   define TB__EDIT_COPY__DIS			gsz_IS_Vista_General__copy_32__dis
#   define TB__EDIT_PASTE				gsz_IS_Vista_General__paste_32
#   define TB__EDIT_PASTE__DIS			gsz_IS_Vista_General__paste_32__dis
#   define TB__EDIT_UNDO				gsz_Custom__Undo_32
#   define TB__EDIT_UNDO__DIS			gsz_Custom__Undo_32__dis
#   define TB__EDIT_REDO				gsz_Custom__Redo_32
#   define TB__EDIT_REDO__DIS			gsz_Custom__Redo_32__dis
#   define TB__SAVE						gsz_IS_XMac_General__disk_32
#   define TB__SAVE__DIS				gsz_IS_XMac_General__disk_32__dis
#   define TB__APPLY_LEFT				gsz_IS_Vista_Data__fila_next_32
#   define TB__APPLY_LEFT__DIS			gsz_IS_Vista_Data__fila_next_32__dis
#   define TB__APPLY_RIGHT				gsz_IS_Vista_Data__fila_back_32
#   define TB__APPLY_RIGHT__DIS			gsz_IS_Vista_Data__fila_back_32__dis
#	define TB__SPLIT_VERTICALLY			gsz_Custom__SplitVertically_32
#	define TB__SPLIT_HORIZONTALLY		gsz_Custom__SplitHorizontally_32

extern const char * TB__AUTO_MERGE;				extern const char * TB__AUTO_MERGE__DIS;
extern const char * TB__FILE_DIFF;				extern const char * TB__FILE_DIFF__DIS;
extern const char * TB__FILE_MERGE;				// normal & disabled are the same
extern const char * TB__FOLDER_DIFF;			extern const char * TB__FOLDER_DIFF__DIS;
extern const char * TB__FOLDER_FOLDERS;			extern const char * TB__FOLDER_FOLDERS__DIS;
extern const char * TB__FOLDER_EQUAL;			// normal & disabled are the same
extern const char * TB__FOLDER_EQUIVALENT;		// normal & disabled are the same
extern const char * TB__FOLDER_QUICKMATCH;		// normal & disabled are the same
extern const char * TB__FOLDER_NOPEER;			// normal & disabled are the same
extern const char * TB__DOP_ALL;				extern const char * TB__DOP_ALL__DIS;
extern const char * TB__DOP_DIF;				extern const char * TB__DOP_DIF__DIS;
extern const char * TB__DOP_CTX;				extern const char * TB__DOP_CTX__DIS;
extern const char * TB__CHANGE_NEXT;			extern const char * TB__CHANGE_NEXT__DIS;
extern const char * TB__CHANGE_PREV;			extern const char * TB__CHANGE_PREV__DIS;
extern const char * TB__CONFLICT_NEXT;			extern const char * TB__CONFLICT_NEXT__DIS;
extern const char * TB__CONFLICT_PREV;			extern const char * TB__CONFLICT_PREV__DIS;
extern const char * TB__PILCROW;				// normal & disabled are the same
extern const char * TB__LINE_NUMBERS;			// normal & disabled are the same
extern const char * TB__EDIT_CUT;				extern const char * TB__EDIT_CUT__DIS;
extern const char * TB__EDIT_COPY;				extern const char * TB__EDIT_COPY__DIS;
extern const char * TB__EDIT_PASTE;				extern const char * TB__EDIT_PASTE__DIS;
extern const char * TB__EDIT_UNDO;				extern const char * TB__EDIT_UNDO__DIS;
extern const char * TB__EDIT_REDO;				extern const char * TB__EDIT_REDO__DIS;
extern const char * TB__SAVE;					extern const char * TB__SAVE__DIS;
extern const char * TB__APPLY_LEFT;				extern const char * TB__APPLY_LEFT__DIS;
extern const char * TB__APPLY_RIGHT;			extern const char * TB__APPLY_RIGHT__DIS;
extern const char * TB__SPLIT_VERTICALLY;		// normal & disabled are the same
extern const char * TB__SPLIT_HORIZONTALLY;		// normal & disabled are the same

//////////////////////////////////////////////////////////////////

#endif//H_RESOURCES_H
