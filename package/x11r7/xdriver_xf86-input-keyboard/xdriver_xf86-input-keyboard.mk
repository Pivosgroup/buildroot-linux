################################################################################
#
# xdriver_xf86-input-keyboard -- Keyboard input driver
#
################################################################################

XDRIVER_XF86_INPUT_KEYBOARD_VERSION = 1.4.0
XDRIVER_XF86_INPUT_KEYBOARD_SOURCE = xf86-input-keyboard-$(XDRIVER_XF86_INPUT_KEYBOARD_VERSION).tar.bz2
XDRIVER_XF86_INPUT_KEYBOARD_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_INPUT_KEYBOARD_DEPENDENCIES = xserver_xorg-server xproto_inputproto xproto_kbproto xproto_randrproto xproto_xproto

$(eval $(call AUTOTARGETS))
