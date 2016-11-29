#
# Makefile for the linux kernel.
#

obj-y                          := main.o version.o mounts.o
ifneq ($(CONFIG_BLK_DEV_INITRD),y)
obj-y                          += noinitramfs.o
else
obj-$(CONFIG_BLK_DEV_INITRD)   += initramfs.o
endif
obj-$(CONFIG_GENERIC_CALIBRATE_DELAY) += calibrate.o

mounts-y			:= do_mounts.o
mounts-$(CONFIG_DEVFS_FS)	+= do_mounts_devfs.o
mounts-$(CONFIG_BLK_DEV_RAM)	+= do_mounts_rd.o
mounts-$(CONFIG_BLK_DEV_INITRD)	+= do_mounts_initrd.o
mounts-$(CONFIG_BLK_DEV_MD)	+= do_mounts_md.o
#add by lvxin 00135113 for support bcm96358 FLASH layout  of atpflash.h
ifeq ("$(strip $(PRODUCT_NAME))","hg553")
	CFLAGS += -DSUPPORT_HG553
	CPPFLAGS += -DSUPPORT_HG553
endif
ifeq ("$(CONFIG_ATP_SUPPORT_DOUBLEMIDD)","y")
EXTRA_CFLAGS    		+= -I$(INC_DRIVER_VENDOR_DIR) -I$(INC_DRIVER_HUAWEI_PRD_DIR) -I$(INC_DRIVER_HUAWEI_DIR) -DATP_SUPPORT_DOUBLEMIDD
endif
#end of add by lvxin 00135113

# files to be removed upon make clean
clean-files := ../include/linux/compile.h

# dependencies on generated files need to be listed explicitly

$(obj)/version.o: include/linux/compile.h

# compile.h changes depending on hostname, generation number, etc,
# so we regenerate it always.
# mkcompile_h will make sure to only update the
# actual file if its content has changed.

include/linux/compile.h: FORCE
	@echo '  CHK     $@'
	$(Q)$(CONFIG_SHELL) $(srctree)/scripts/mkcompile_h $@ \
	"$(UTS_MACHINE)" "$(CONFIG_SMP)" "$(CONFIG_PREEMPT)" "$(CC) $(CFLAGS)"
