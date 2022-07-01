################################################################################
#
# atbm_wifi
#
################################################################################

ATBM_WIFI_VERSION = 54dc01e2cc645753a08db0c48861aa984e2a8ed3
ATBM_WIFI_SITE = https://github.com/gtxaspec/atbm_wifi.git
ATBM_WIFI_SITE_METHOD = git
ATBM_WIFI_LICENSE = GPLv2
ATBM_WIFI_LICENSE_FILES = COPYING

define ATBM_WIFI_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CROSS_COMPILE=$(TARGET_CROSS) KERDIR=$(LINUX_DIR) -C $(@D)
endef

$(eval $(kernel-module))

define ATBM_WIFI_KERNEL_MODULES_INSTALL
	cp $(@D)/driver_install/*.ko $(TARGET_DIR)/lib/modules
endef

$(eval $(generic-package))
