# Makefile for the bcm963xx board drivers
#


obj-y := board.o bcm63xx_flash.o bcm63xx_led.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
	EXTRA_CFLAGS += -DWIRELESS
endif

ifeq ($(strip $(WIRELESS)),1)
	EXTRA_CFLAGS += -DWIRELESS
endif

ifeq ($(strip $(BUILD_TMP_FIRMWARE)),1)
	EXTRA_CFLAGS += -DSUPPORT_TMP_FIRMWARE
endif

ifeq ($(strip $(HARDWARE_VERSION)),y)        
        EXTRA_CFLAGS += -DSUPPORT_HARDWARE_VERSION
endif

ifneq ($(strip $(BUILD_BORAD_LOG_SECTION)), )
	EXTRA_CFLAGS += -DSUPPORT_LOG_SECTION
endif

ifeq ($(strip $(BUILD_BACKUP_PSI)),y)
	EXTRA_CFLAGS += -DSUPPORT_BACKUP_PSI
endif
-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

