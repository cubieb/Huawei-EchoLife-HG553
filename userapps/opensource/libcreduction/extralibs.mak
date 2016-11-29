#
# Extra run-time libraries
# 
# Copyright (C) 2004 Broadcom Corporation
#

TARGETS += $(LIBDIR)/libcrypt.so.0
TARGETS += $(LIBDIR)/libutil.so.0
TARGETS += $(EXTRALIBDIR)/libgcc_s.so.1
TARGETS += $(LIBDIR)/libm.so.0

ifneq ($(strip $(BRCM_PTHREADS)),)
  TARGETS += $(LIBDIR)/libpthread.so.0
endif

ifneq ($(strip $(BUILD_VODSL)),)  
  TARGETS += $(EXTRALIBDIR)/libstdc++.so.6  
endif

ifneq ($(strip $(BUILD_OPROFILE)),)
	TARGETS += $(LIBDIR)/libstdc++.so.6	
endif

