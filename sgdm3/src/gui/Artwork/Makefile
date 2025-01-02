## Makefile
## simple script to convert PNG's to header files.
## this only needs to be done once we have final PNG artwork
## (because we checkin both the PNG's and the header files).
##################################################################

LIST_BIG=\
	DiffMergeSS.png \
	DiffMergeSSwithBorder.png

LIST_SMALL=


default:	big small

big:
	@for f in $(LIST_BIG); do \
		( ../IconShock/png2src_big/Debug/png2src_big.exe png/$$f  ||  exit 1 ) \
	done


small:
	@for f in $(LIST_SMALL); do \
		( ../IconShock/png2src/Debug/png2src.exe png/$$f  ||  exit 1 ) \
	done
