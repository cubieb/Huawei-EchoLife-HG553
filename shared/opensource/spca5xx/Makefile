VERSION    = 00.60.00.1
CVSVERSION = "$Experimental work Michel Xhaard && Reza Jelveh 03/02/2004"
DEFINES    =

###
# The following flags enable experimental features.
# By default, these are enabled for development versions of the driver, and
# disabled for release versions.

# Optional: Enable driver debugging
DEFINES   += -DSPCA50X_ENABLE_DEBUG

# Optional: Enable direct register read/write for PAC207 development
#DEFINES   += -DSPCA5XX_ENABLE_REGISTERPLAY

###
# The following flags enable features that aren't yet implemented, and
# therefore are disabled by default.

# Optional: Enable compression
DEFINES   += -DSPCA50X_ENABLE_COMPRESSION

###
# Rest of Makefile follows here. You probably won't need to touch this.

# Setup defines
DEFINES   += -DCONFIG_USB_SPCA5XX_MODULE=1 -DMODULE -D__KERNEL__
DEFINES   += -DVID_HARDWARE_SPCA5XX=0xFF -DSPCA5XX_VERSION=\"$(VERSION)\"


ifneq ($(KERNELRELEASE),)   # We were called by kbuild
CFLAGS += $(DEFINES) 
obj-m += spca5xx.o
spca5xx-objs := drivers/usb/spca5xx.o drivers/usb/spcadecoder.o 
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/bcm963xx 
else   # We were called from command line

KERNEL_VERSION = 2.6.21.5
KERNELDIR := $(KERNEL_DIR) 
PWD  := $(shell pwd)
MODULE_INSTALLDIR = $(INSTALL_DIR)/lib/modules/kernel/drivers/usb/media/

# Targets, don't change!
default:
	@echo '   Building SPCA5XX driver for 2.5/2.6 kernel.'
	@echo '   Remember: you must have read/write access to your kernel source tree.'
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) CC=$(CC) modules

.PHONY: install
install:
	mkdir -p $(MODULE_INSTALLDIR)
	rm -f $(MODULE_INSTALLDIR)spca50x.ko
	rm -f $(MODULE_INSTALLDIR)et61x.ko
	install -c -m 0644 spca5xx.ko $(MODULE_INSTALLDIR)
#	/sbin/depmod -ae

uninstall:
	rm -f $(MODULE_INSTALLDIR)/spca5xx.ko
#	/sbin/depmod -aq

endif

############################################################################## 
# OTHER TARGETS 
##############################################################################
clean:
	rm -r -f drivers/usb/*.o drivers/usb/.spcadecoder.o.cmd \
	drivers/usb/.spca5xx.o.cmd  *.o *.ko *.mod.* .[a-z]* core *.i
	
############################################################################## 
