## poi.inc -- GNU Makefile for sgdm3/poi for Linux
## file declarations for dependency checking by users of this library.
##################################################################

POI_HDRS=	../poi/poi.h \
		../poi/poi_dcl.h \
		../poi/poi_item.h \
		../poi/poi_item_table.h

POI_SRCS=	../poi/poi_item.cpp \
		../poi/poi_item_table.cpp

POI_LIB=	sgdm_poi

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(POI_LIB)
MY_OBJDEPS+=	../poi/poi.inc $(POI_HDRS)
MY_LIBDEPS+=	../poi/poi.inc
MY_EXEDEPS+=	../poi/poi.inc $(LIBDIR)/$(LIB_PFX)$(POI_LIB)$(LIB_SFX)

