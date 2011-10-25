#############################################################
#
#cmem 
#
#############################################################
CMEM_VERSION:=0.9.9
CMEM_SOURCE=cmem-$(CMEM_VERSION).tar.gz
CMEM_SITE=./package/amlogic/cmem/src
CMEM_SITE_METHOD=cp
CMEM_INSTALL_STAGING=YES

define CMEM_BUILD_CMDS
        $(MAKE) CC="$(TARGET_CC)" -C $(@D) STAGING_DIR=$(STAGING_DIR) TARGET_DIR=$(STAGING_DIR) libcmem.a
endef

define CMEM_INSTALL_STAGING_CMDS
        install -m 755 $(@D)/libcmem.a $(TARGET_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,cmem))
