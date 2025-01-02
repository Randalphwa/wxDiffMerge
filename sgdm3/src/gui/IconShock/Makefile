## Makefile
## simple script to convert PNG's to header files.
## this only needs to be done once we have final PNG artwork
## (because we checkin both the PNG's and the header files).
##################################################################

LIST=\
	Custom__LineNumbers_32.png									\
	Custom__Pilcrow_32.png										\
	Custom__Redo_32.png				Custom__Redo_32__dis.png			\
	Custom__Undo_32.png				Custom__Undo_32__dis.png			\
	Custom__FileDiff_32.png				Custom__FileDiff_32__dis.png			\
	Custom__FileMerge_32.png									\
	Custom__FileEqual_32.png									\
	Custom__FileEquivalent_32.png									\
	Custom__FileQuickMatch_32.png									\
	Custom__FileNoPeer_32.png									\
	Custom__SplitVertically_32.png			Custom__SplitHorizontally_32.png		\
	IS_Vista_Data__fila_32__automerge.png		IS_Vista_Data__fila_32__automerge__dis.png	\
	IS_Vista_Data__fila_32__dop_all.png		IS_Vista_Data__fila_32__dop_all__dis.png	\
	IS_Vista_Data__fila_32__dop_ctx.png		IS_Vista_Data__fila_32__dop_ctx__dis.png	\
	IS_Vista_Data__fila_32__dop_dif.png		IS_Vista_Data__fila_32__dop_dif__dis.png	\
	IS_Vista_Data__fila_back_32.png 		IS_Vista_Data__fila_back_32__dis.png 		\
	IS_Vista_Data__fila_down_32.png 		IS_Vista_Data__fila_down_32__dis.png 		\
	IS_Vista_Data__fila_down_32__conflict.png 	IS_Vista_Data__fila_down_32__conflict__dis.png 	\
	IS_Vista_Data__fila_next_32.png 		IS_Vista_Data__fila_next_32__dis.png 		\
	IS_Vista_Data__fila_up_32.png 			IS_Vista_Data__fila_up_32__dis.png 		\
	IS_Vista_Data__fila_up_32__conflict.png 	IS_Vista_Data__fila_up_32__conflict__dis.png 	\
	IS_Vista_General__scissors_32.png		IS_Vista_General__scissors_32__dis.png		\
	IS_Vista_General__copy_32.png			IS_Vista_General__copy_32__dis.png		\
	IS_Vista_General__paste_32.png			IS_Vista_General__paste_32__dis.png		\
	IS_Vista_General__folder_32.png			IS_Vista_General__folder_32__dis.png		\
	IS_Vista_General__folder_zoom_32.png		IS_Vista_General__folder_zoom_32__dis.png	\
	IS_XMac_General__disk_32.png			IS_XMac_General__disk_32__dis.png		\


default:
	@for f in $(LIST); do \
		( ./png2src/Debug/png2src.exe png/$$f  ||  exit 1 ) \
	done
