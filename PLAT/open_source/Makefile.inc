
ifeq ($(MBTK_OPENCPU_AT),y)
include $(TOP)/PLAT/open_source/atcmd/Makefile.inc
endif

ifeq ($(MBTK_OPENCPU_ZBAR),y)
include $(TOP)/PLAT/open_source/zbar/Makefile.inc
endif

include $(TOP)/PLAT/open_source/testlib/Makefile.inc
