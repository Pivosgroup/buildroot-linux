################################################################################
#
## libssh
#
#################################################################################

LIBSSH_VERSION = 0.5.2
LIBSSH_SITE = http://www.libssh.org/files/0.5
LIBSSH_INSTALL_STAGING = YES
LIBSSH_TEMP_BUILDDIR=output/build/libssh-build
LIBSSH_CONF_OPT += -DWITH_SERVER=OFF
LIBSSH_CONFIGURE_CMDS = \
        (mkdir -p $(LIBSSH_TEMP_BUILDDIR) && rm -rf $(LIBSSH_TEMP_BUILDDIR)/* && \
         cd $(LIBSSH_TEMP_BUILDDIR) && \
        $(LIBSSH_CONF_ENV) $(HOST_DIR)/usr/bin/cmake $(LIBSSH_SRCDIR) \
                -DCMAKE_TOOLCHAIN_FILE="$(BASE_DIR)/toolchainfile.cmake" \
                -DCMAKE_INSTALL_PREFIX="/usr" \
                $(LIBSSH_CONF_OPT) \
        )

LIBSSH_BUILD_CMDS = \
        $(HOST_MAKE_ENV) $(LIBSSH_MAKE_ENV) $(LIBSSH_MAKE) $(LIBSSH_MAKE_OPT) -C $(LIBSSH_TEMP_BUILDDIR)

LIBSSH_INSTALL_STAGING_CMDS = \
        $(TARGET_MAKE_ENV) $(LIBSSH_MAKE_ENV) $(LIBSSH_MAKE) $(LIBSSH_MAKE_OPT) $(LIBSSH_INSTALL_STAGING_OPT) -C $(LIBSSH_TEMP_BUILDDIR)

LIBSSH_INSTALL_TARGET_CMDS = \
        $(TARGET_MAKE_ENV) $(LIBSSH_MAKE_ENV) $(LIBSSH_MAKE) $(LIBSSH_MAKE_OPT) $(LIBSSH_INSTALL_TARGET_OPT) -C $(LIBSSH_TEMP_BUILDDIR) && \
        rm -rf $(LIBSSH_TEMP_BUILDDIR)

$(eval $(call CMAKETARGETS,package/thirdparty,libssh))
