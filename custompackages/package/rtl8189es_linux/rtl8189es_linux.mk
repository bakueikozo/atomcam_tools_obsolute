################################################################################
#
# rtl8189es_linux
#
################################################################################

RTL8189ES_LINUX_VERSION = b41d0ad90e33c8640a16602286d034c21494bc26
RTL8189ES_LINUX_SITE = https://github.com/gtxaspec/rtl8189ES_linux.git
RTL8189ES_LINUX_SITE_METHOD = git
RTL8189ES_LINUX_LICENSE = GPLv2
RTL8189ES_LINUX_LICENSE_FILES = COPYING

define RTL8189ES_LINUX_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -j4 ARCH=mips CROSS_COMPILE=$(TARGET_CROSS) KSRC=$(LINUX_DIR) -C $(@D)
endef

$(eval $(kernel-module))

define RTL8189ES_LINUX_KERNEL_MODULES_INSTALL
	set ;\
	cp $(@D)/*.ko $(TARGET_DIR)/lib/modules
endef

$(eval $(generic-package))
