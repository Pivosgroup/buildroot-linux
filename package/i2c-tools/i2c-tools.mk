#############################################################
#
# i2c-tools
#
#############################################################

I2C_TOOLS_VERSION = 3.0.3
I2C_TOOLS_SOURCE = i2c-tools-$(I2C_TOOLS_VERSION).tar.bz2
I2C_TOOLS_SITE = http://dl.lm-sensors.org/i2c-tools/releases/

define I2C_TOOLS_BUILD_CMDS
 $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define I2C_TOOLS_INSTALL_TARGET_CMDS
	for i in i2cdump i2cget i2cset i2cdetect; \
	do \
		$(INSTALL) -m 755 -D $(@D)/tools/$$i $(TARGET_DIR)/usr/bin/$$i; \
	done
endef

$(eval $(call GENTARGETS))
