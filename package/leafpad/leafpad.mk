################################################################################
#
# leafpad
#
################################################################################

LEAFPAD_VERSION = 0.8.18
LEAFPAD_SITE = http://savannah.nongnu.org/download/leafpad
LEAFPAD_DEPENDENCIES = libgtk2 host-intltool

$(eval $(autotools-package))
