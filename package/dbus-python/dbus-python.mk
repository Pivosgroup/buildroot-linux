#############################################################
#
# dbus-python
#
#############################################################
DBUS_PYTHON_VERSION = 0.83.0
DBUS_PYTHON_SOURCE = dbus-python-$(DBUS_PYTHON_VERSION).tar.gz
DBUS_PYTHON_SITE = http://dbus.freedesktop.org/releases/dbus-python/
DBUS_PYTHON_INSTALL_STAGING = YES

DBUS_PYTHON_CONF_ENV = am_cv_pathless_PYTHON=python \
		ac_cv_path_PYTHON=$(HOST_DIR)/usr/bin/python \
		am_cv_python_version=$(PYTHON_VERSION) \
		am_cv_python_platform=linux2 \
		am_cv_python_pythondir=/usr/lib/python$(PYTHON_VERSION_MAJOR)/site-packages \
		am_cv_python_pyexecdir=/usr/lib/python$(PYTHON_VERSION_MAJOR)/site-packages \
		am_cv_python_includes=-I$(STAGING_DIR)/usr/include/python$(PYTHON_VERSION_MAJOR)

DBUS_PYTHON_CONF_OPT = --disable-html-docs --disable-api-docs

DBUS_PYTHON_DEPENDENCIES = dbus-glib python host-python

$(eval $(call AUTOTARGETS))
