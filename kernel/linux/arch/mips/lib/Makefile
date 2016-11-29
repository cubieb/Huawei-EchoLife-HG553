#
# Makefile for MIPS-specific library files..
#

ifeq ($(CONFIG_RTL8672), y)
lib-y	+= csum_partial_rtl.o memcpy_rtl.o memcpy-inatomic_rtl.o memset_rtl.o strlen_user.o \
	   strncpy_user.o strnlen_user.o uncached.o csum_partial_copy.o
else
lib-y	+= csum_partial.o memcpy.o memcpy-inatomic.o memset.o strlen_user.o \
	   strncpy_user.o strnlen_user.o uncached.o
endif

obj-y			+= iomap.o
obj-$(CONFIG_PCI)	+= iomap-pci.o

# libgcc-style stuff needed in the kernel
lib-y += ashldi3.o ashrdi3.o lshrdi3.o
