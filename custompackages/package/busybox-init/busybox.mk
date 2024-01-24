################################################################################
#
# busybox
#
################################################################################

BUSYBOX_INIT_VERSION = 1.24.1
BUSYBOX_INIT_SITE = http://www.busybox.net/downloads
BUSYBOX_INIT_SOURCE = busybox-$(BUSYBOX_INIT_VERSION).tar.bz2
BUSYBOX_INIT_LICENSE = GPLv2
BUSYBOX_INIT_LICENSE_FILES = LICENSE
BUSYBOX_INIT_INSTALL_TARGET = NO
BUSYBOX_INIT_INSTALL_STAGING = YES

BUSYBOX_INIT_CFLAGS = \
	$(TARGET_CFLAGS)

BUSYBOX_INIT_LDFLAGS = \
	$(TARGET_LDFLAGS)

BUSYBOX_INIT_BUILD_CONFIG = $(BUSYBOX_INIT_DIR)/.config
# Allows the build system to tweak CFLAGS
BUSYBOX_INIT_MAKE_ENV = \
	$(TARGET_MAKE_ENV) \
	CFLAGS="$(BUSYBOX_INIT_CFLAGS)" \
	CFLAGS_busybox="$(BUSYBOX_INIT_CFLAGS_busybox)"
BUSYBOX_INIT_MAKE_OPTS = \
	CC="$(TARGET_CC)" \
	ARCH=$(KERNEL_ARCH) \
	PREFIX="$(TARGET_DIR)" \
	EXTRA_LDFLAGS="$(BUSYBOX_INIT_LDFLAGS)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CONFIG_PREFIX="$(TARGET_DIR)"

ifndef BUSYBOX_INIT_CONFIG_FILE
	BUSYBOX_INIT_CONFIG_FILE = $(call qstrip,$(BR2_PACKAGE_BUSYBOX_INIT_CONFIG))
endif

BUSYBOX_INIT_KCONFIG_FILE = $(BUSYBOX_INIT_CONFIG_FILE)
BUSYBOX_INIT_KCONFIG_EDITORS = menuconfig xconfig gconfig
BUSYBOX_INIT_KCONFIG_OPTS = $(BUSYBOX_INIT_MAKE_OPTS)

define BUSYBOX_INIT_SET_MMU
	$(call KCONFIG_DISABLE_OPT,CONFIG_NOMMU,$(BUSYBOX_INIT_BUILD_CONFIG))
endef

# If we're using static libs do the same for busybox
define BUSYBOX_INIT_PREFER_STATIC
	$(call KCONFIG_ENABLE_OPT,CONFIG_STATIC,$(BUSYBOX_INIT_BUILD_CONFIG))
endef

define BUSYBOX_INIT_KCONFIG_FIXUP_CMDS
	$(BUSYBOX_INIT_SET_MMU)
	$(BUSYBOX_INIT_PREFER_STATIC)
endef

define BUSYBOX_INIT_BUILD_CMDS
	$(BUSYBOX_INIT_MAKE_ENV) $(MAKE) $(BUSYBOX_INIT_MAKE_OPTS) -C $(@D)
endef

# Checks to give errors that the user can understand
# Must be before we call to kconfig-package
ifeq ($(BR2_PACKAGE_BUSYBOX)$(BR_BUILDING),yy)
ifeq ($(call qstrip,$(BR2_PACKAGE_BUSYBOX_INIT_CONFIG)),)
$(error No BusyBox configuration file specified, check your BR2_PACKAGE_BUSYBOX_INIT_CONFIG setting)
endif
endif


define BUSYBOX_INIT_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 4755 $(@D)/busybox $(STAGING_DIR)/bin-init/busybox
endef

$(eval $(kconfig-package))
