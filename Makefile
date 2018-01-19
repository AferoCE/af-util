#  Copyright (c) 2017 Afero, Inc. All rights reserved.

include $(TOPDIR)/rules.mk

PKG_NAME:=af-util
PKG_VERSION:=0.1
PKG_RELEASE:=1

USE_SOURCE_DIR:=$(CURDIR)/pkg

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=libtool autoreconf
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/af-util
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Afero Utility Library
  DEPENDS:=
  URL:=http://www.afero.io
endef

define Package/af-util/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
#	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libaf_util.a $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libaf_util.so* $(1)/usr/lib/

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/include/af_log.h $(STAGING_DIR)/usr/include
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/include/af_util.h $(STAGING_DIR)/usr/include
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/include/af_mempool.h $(STAGING_DIR)/usr/include
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libaf_util.so* $(STAGING_DIR)/usr/lib
endef

define Build/Clean
	$(RM) -rf $(CURDIR)/pkg/src/.deps/*
	$(RM) -rf $(CURDIR)/pkg/src/*.o $(CURDIR)/pkg/src/*.lo
	$(RM) -rf $(CURDIR)/pkg/src/linux/*.o $(CURDIR)/pkg/src/.libs/*
	$(RM) -rf $(CURDIR)/pkg/autom4te.cache/*
	$(RM) -rf $(CURDIR)/pkg/ipkg-install/*
	$(RM) -rf $(CURDIR)/pkg/ipkg-ar71xx/$(PKG_NAME)/*
	$(RM) -rf $(CURDIR)/pkg/libtool $(CURDIR)/pkg/config.*
	$(RM) -rf $(CURDIR)/pkg/.quilt_checked  $(CURDIR)/pkg/.prepared $(CURDIR)/pkg/.configured_ $(CURDIR)/pkg/.built
	$(RM) -rf $(CURDIR)/pkg/COPYING $(CURDIR)/pkg/NEWS
	$(RM) -rf $(CURDIR)/pkg/src/Makefile $(CURDIR)/pkg/src/Makefile.in $(CURDIR)/pkg/Makefile $(CURDIR)/pkg/Makefile.in
	$(RM) -rf $(CURDIR)/pkg/aclocal.m4 $(CURDIR)/pkg/ChangeLog  $(CURDIR)/pkg/ABOUT-NLS $(CURDIR)/pkg/AUTHORS $(CURDIR)/pkg/configure
	$(RM) -rf $(CURDIR)/pkg/.source_dir $(CURDIR)/pkg/stamp-h1 $(CURDIR)/pkg/src/libaf_ipc.la

	$(RM) -rf $(STAGING_DIR)/pkginfo/libaf_util.*
	$(RM) -rf $(STAGING_DIR)/usr/lib/libaf_util.so*
	$(RM) -rf $(STAGING_DIR)/usr/include/af_log.h
	$(RM) -rf $(STAGING_DIR)/usr/include/af_util.h
	$(RM) -rf $(1)/usr/lib/libaf_util* $(1)/stamp/.af-ipc_installed
endef




$(eval $(call BuildPackage,af-util))





