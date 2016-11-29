# File: bcmdrivers/opensource/char/serial
#
# Makefile for the BCM63xx serial/console driver

obj-$(CONFIG_BCM_SERIAL) += bcm63xx_cons.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ifneq ($(BUILD_SERIAL_OPEN), )
EXTRA_CFLAGS += -DBUILD_SERIAL_OPEN
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

