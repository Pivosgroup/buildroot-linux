################################################################################
#
# librsvg
#
################################################################################

LIBRSVG_VERSION_MAJOR = 2.26
LIBRSVG_VERSION_MINOR = 3
LIBRSVG_VERSION = $(LIBRSVG_VERSION_MAJOR).$(LIBRSVG_VERSION_MINOR)
LIBRSVG_SITE = http://ftp.gnome.org/pub/GNOME/sources/librsvg/$(LIBRSVG_VERSION_MAJOR)/
LIBRSVG_INSTALL_STAGING = YES
LIBRSVG_CONF_OPT = --disable-tools
LIBRSVG_DEPENDENCIES = libxml2 cairo pango libglib2 gdk-pixbuf

# If we have Gtk2, let's build it first to benefit from librsvg Gtk
# support.
ifeq ($(BR2_PACKAGE_LIBGTK2),y)
LIBRSVG_DEPENDENCIES += libgtk2
endif

$(eval $(autotools-package))
