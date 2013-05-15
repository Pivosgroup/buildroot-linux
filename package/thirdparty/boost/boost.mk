#############################################################
#
# boost
#
#############################################################
BOOST_VERSION:=1_53_0
BOOST_VERSION2:=1.53.0
BOOST_SOURCE:=boost_$(BOOST_VERSION).tar.bz2
BOOST_SITE:=http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/boost
BOOST_INSTALL_STAGING=YES
BOOST_INSTALL_TARGET=YES

ifneq ($(BR2_PACKAGE_BOOST_DATETIME),y)
BOOST_LIB_CONF += --without-date_time
endif

ifneq ($(BR2_PACKAGE_BOOST_FILESYSTEM),y)
BOOST_LIB_CONF += --without-filesystem
endif

ifneq ($(BR2_PACKAGE_BOOST_GRAPH),y)
BOOST_LIB_CONF += --without-graph
endif

ifneq ($(BR2_PACKAGE_BOOST_GRAPH_PARALLEL),y)
BOOST_LIB_CONF += --without-graph_parallel
endif

ifneq ($(BR2_PACKAGE_BOOST_IOSTREAMS),y)
BOOST_LIB_CONF += --without-iostreams
else
BOOST_DEPENDENCIES += zlib bzip2
endif

ifneq ($(BR2_PACKAGE_BOOST_MATH),y)
BOOST_LIB_CONF += --without-math
endif

ifneq ($(BR2_PACKAGE_BOOST_MPI),y)
BOOST_LIB_CONF += --without-mpi
endif

ifneq ($(BR2_PACKAGE_BOOST_PROGRAM_OPTIONS),y)
BOOST_LIB_CONF += --without-program_options
endif

ifneq ($(BR2_PACKAGE_BOOST_PYTHON),y)
BOOST_LIB_CONF += --without-python
else
BOOST_DEPENDENCIES += python
endif

ifneq ($(BR2_PACKAGE_BOOST_RANDOM),y)
BOOST_LIB_CONF += --without-random
endif

ifneq ($(BR2_PACKAGE_BOOST_REGEX),y)
BOOST_LIB_CONF += --without-regex
endif

ifneq ($(BR2_PACKAGE_BOOST_SERIALIZATION),y)
BOOST_LIB_CONF += --without-serialization
endif

ifneq ($(BR2_PACKAGE_BOOST_SIGNALS),y)
BOOST_LIB_CONF += --without-signals
endif

ifneq ($(BR2_PACKAGE_BOOST_SYSTEM),y)
BOOST_LIB_CONF += --without-system
endif

ifneq ($(BR2_PACKAGE_BOOST_TEST),y)
BOOST_LIB_CONF += --without-test
endif

ifneq ($(BR2_PACKAGE_BOOST_THREAD),y)
BOOST_LIB_CONF += --without-thread
endif

ifneq ($(BR2_PACKAGE_BOOST_WAVE),y)
BOOST_LIB_CONF += --without-wave
endif

define BOOST_BUILD_CMDS
	(cd $(@D);\
		./bootstrap.sh \
	)
	$(SED) "s,using gcc,using gcc : $(ARCH) : $(TARGET_CXX),g" $(@D)/project-config.jam
	$(SED) "s,using python : 2.6 : /usr,using python : $(PYTHON_VERSION_MAJOR) \
			: $(STAGING_DIR)/usr,g" $(@D)/project-config.jam
	(cd $(@D); \
		./bjam install --prefix=$(STAGING_DIR)/usr --layout=system $(BOOST_LIB_CONF) link=shared;\
	)
endef

 define BOOST_INSTALL_TARGET_CMDS
        (cd $(@D); \
                ./bjam stage --stagedir=$(TARGET_DIR)/usr --layout=system $(BOOST_LIB_CONF) link=shared;\
        )
 endef



define BOOST_CLEAN_CMDS
	rm -f $(TARGET_DIR)/include/boost
endef

$(eval $(call GENTARGETS,package/thirdparty,boost))
