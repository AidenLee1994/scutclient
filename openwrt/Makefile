#
# Copyright (C) 2026 Dr. Lee
#
# Maintained by Dr. Lee. Based on the original scutclient project
# (https://github.com/scutclient/scutclient) — thanks to the upstream authors.
#
# This is free software, licensed under the GNU General Public License v3.
# See /COPYING for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=scutclient
#If you want to compile your local source code, fill the absolute source path below.
SRCDIR:=
ifneq (,$(SRCDIR))
PKG_VERSION:=$(shell cd $(SRCDIR) && git describe --tags || echo "debug")
PKG_RELEASE:=1
PKG_UNPACK=$(CP) $(SRCDIR)/. $(PKG_BUILD_DIR)
else
PKG_VERSION:=2026.9.9
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/AidenLee1994/scutclient.git
PKG_SOURCE_VERSION:=main
PKG_MIRROR_HASH:=skip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
endif

PKG_MAINTAINER:=Dr. Lee

PKG_LICENSE:=GPL-3.0-or-later
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/scutclient
  SECTION:=net
  CATEGORY:=Network
  TITLE:=SCUT Dr.com client
  DEPENDS:=
  URL:=
endef

define Package/scutclient/description
 Support SCUT private authentication protocol.
endef

define Package/scutclient/conffiles
/etc/config/scutclient
endef

define Package/scutclient/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/openwrt/files/scutclient.config $(1)/etc/config/scutclient
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt/files/scutclient.init $(1)/etc/init.d/scutclient
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt/files/scutclient.hotplug $(1)/etc/hotplug.d/iface/99-scutclient
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scutclient $(1)/usr/bin
endef

$(eval $(call BuildPackage,scutclient))
