# File: bcmdrivers/Makefile
#
# Makefile for the Linux kernel modules.
#

LN_NAME=bcm9$(BRCM_CHIP)
LN_DRIVER_DIRS =

-include $(KERNEL_DIR)/.config

ifneq ($(CONFIG_BCM_BOARD),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BOARD_IMPL) opensource/char/board/$(BRCM_BOARD)/$(LN_NAME);
obj-$(CONFIG_BCM_BOARD) += opensource/char/board/$(BRCM_BOARD)/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_SERIAL),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_SERIAL_IMPL) opensource/char/serial/$(LN_NAME);
obj-$(CONFIG_BCM_SERIAL) += opensource/char/serial/$(LN_NAME)/
endif


ifneq ($(CONFIG_BCM_ADSL),)
endif


ifneq ($(CONFIG_BCM_ENDPOINT),)
endif

ifneq ($(CONFIG_BCM_BCMPROF),)
endif

ifneq ($(CONFIG_BCM_SECURITY),)
endif

ifneq ($(CONFIG_BCM_VDSL),)
endif

ifneq ($(CONFIG_BCM_ATMTEST),)
endif



ifneq ($(CONFIG_BCM_WLAN),)
endif

ifneq ($(CONFIG_BCM_HPNA),)
endif

ifneq ($(CONFIG_BCM_NETDEVTEST),)
endif

ifneq ($(CONFIG_BCM_WLAN),)
endif

symlinks:
	find . -lname "*" -name "$(LN_NAME)" -print -exec rm -f "{}" ";"
	$(CONFIG_SHELL) -c "$(LN_DRIVER_DIRS)"

