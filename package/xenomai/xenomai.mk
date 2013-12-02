#############################################################
# Xenomai
# URL  : http://xenomai.org
# NOTE : Real-Time Framework for Linux
#
#############################################################

XENOMAI_VERSION = $(call qstrip,$(BR2_PACKAGE_XENOMAI_VERSION))
ifeq ($(XENOMAI_VERSION),)
XENOMAI_VERSION = 2.5.6
endif

XENOMAI_SITE = http://download.gna.org/xenomai/stable/
XENOMAI_SOURCE = xenomai-$(XENOMAI_VERSION).tar.bz2

XENOMAI_INSTALL_STAGING = YES

ifeq ($(BR2_arm),y)
XENOMAI_CPU_TYPE = $(call qstrip,$(BR2_PACKAGE_XENOMAI_CPU_TYPE))
# Set "generic" if not defined
ifeq ($(XENOMAI_CPU_TYPE),)
XENOMAI_CPU_TYPE = generic
endif
XENOMAI_CONF_OPT += --enable-arm-mach=$(XENOMAI_CPU_TYPE)
endif #BR2_arm

ifeq ($(BR2_PACKAGE_XENOMAI_SMP),y)
XENOMAI_CONF_OPT += --enable-smp
endif

XENOMAI_CONF_OPT += --includedir=/usr/include/xenomai/

ifeq ($(BR2_HAVE_DEVFILES),)
define XENOMAI_REMOVE_DEVFILES
	for i in xeno-config xeno-info wrap-link.sh ; do \
		rm -f $(TARGET_DIR)/usr/bin/$$i ; \
	done
endef

XENOMAI_POST_INSTALL_TARGET_HOOKS += XENOMAI_REMOVE_DEVFILES
endif

ifeq ($(BR2_PACKAGE_XENOMAI_TESTSUITE),)
define XENOMAI_REMOVE_TESTSUITE
	rm -rf $(TARGET_DIR)/usr/share/xenomai/
	for i in klatency rtdm xeno xeno-load check-vdso \
		irqloop cond-torture-posix switchtest arith \
		sigtest clocktest cyclictest latency wakeup-time \
		xeno-test cond-torture-native mutex-torture-posix \
		mutex-torture-native ; do \
		rm -f $(TARGET_DIR)/usr/bin/$$i ; \
	done
endef

XENOMAI_POST_INSTALL_TARGET_HOOKS += XENOMAI_REMOVE_TESTSUITE
endif

ifeq ($(BR2_PACKAGE_XENOMAI_RTCAN),)
define XENOMAI_REMOVE_RTCAN_PROGS
	for i in rtcanrecv rtcansend ; do \
		rm -f $(TARGET_DIR)/usr/bin/$$i ; \
	done
	rm -f $(TARGET_DIR)/usr/sbin/rtcanconfig
endef

XENOMAI_POST_INSTALL_TARGET_HOOKS += XENOMAI_REMOVE_RTCAN_PROGS
endif

ifeq ($(BR2_PACKAGE_XENOMAI_ANALOGY),)
define XENOMAI_REMOVE_ANALOGY
	for i in cmd_bits cmd_read cmd_write insn_write \
		insn_bits insn_read ; do \
		rm -f $(TARGET_DIR)/usr/bin/$$i ; \
	done
	rm -f $(TARGET_DIR)/usr/sbin/analogy_config
	rm -f $(TARGET_DIR)/usr/lib/libanalogy.*
endef

XENOMAI_POST_INSTALL_TARGET_HOOKS += XENOMAI_REMOVE_ANALOGY
endif

XENOMAI_REMOVE_SKIN_LIST += $(if $(BR2_PACKAGE_XENOMAI_NATIVE_SKIN),,native)
XENOMAI_REMOVE_SKIN_LIST += $(if $(BR2_PACKAGE_XENOMAI_POSIX_SKIN),,posix)
XENOMAI_REMOVE_SKIN_LIST += $(if $(BR2_PACKAGE_XENOMAI_VXWORKS_SKIN),,vxworks)
XENOMAI_REMOVE_SKIN_LIST += $(if $(BR2_PACKAGE_XENOMAI_PSOS_SKIN),,psos)
XENOMAI_REMOVE_SKIN_LIST += $(if $(BR2_PACKAGE_XENOMAI_RTAI_SKIN),,rtai)
XENOMAI_REMOVE_SKIN_LIST += $(if $(BR2_PACKAGE_XENOMAI_UITRON_SKIN),,uitron)
XENOMAI_REMOVE_SKIN_LIST += $(if $(BR2_PACKAGE_XENOMAI_VRTX_SKIN),,vrtx)

define XENOMAI_REMOVE_SKINS
	for i in $(XENOMAI_REMOVE_SKIN_LIST) ; do \
		rm -f $(TARGET_DIR)/usr/lib/lib$$i.* ; \
		if [ $$i == "posix" ] ; then \
			rm -f $(TARGET_DIR)/usr/lib/posix.wrappers ; \
		fi ; \
	done
endef

XENOMAI_POST_INSTALL_TARGET_HOOKS += XENOMAI_REMOVE_SKINS

# If you use static /dev creation don't forget to update your
#  device_table_dev.txt
ifeq ($(BR2_ROOTFS_DEVICE_CREATION_DYNAMIC_UDEV),y)
XENOMAI_DEPENDENCIES += udev

define XENOMAI_INSTALL_UDEV_RULES
	if test -d $(TARGET_DIR)/etc/udev/rules.d ; then \
		for f in $(@D)/ksrc/nucleus/udev/*.rules ; do \
			cp $$f $(TARGET_DIR)/etc/udev/rules.d/ ; \
		done ; \
	fi;
endef

XENOMAI_POST_INSTALL_TARGET_HOOKS += XENOMAI_INSTALL_UDEV_RULES
endif # udev

define XENOMAI_REMOVE_UDEV_RULES
	if test -d $(TARGET_DIR)/etc/udev/rules.d ; then \
		for f in $(@D)/ksrc/nucleus/udev/*.rules ; do \
			rm -f $(TARGET_DIR)/etc/udev/rules.d/$$f ; \
		done ; \
	fi;
endef

XENOMAI_POST_UNINSTALL_TARGET_HOOKS += XENOMAI_REMOVE_UDEV_RULES

$(eval $(call AUTOTARGETS))
