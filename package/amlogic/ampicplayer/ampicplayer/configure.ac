AC_PREREQ([2.64])

m4_define([AMPICPLY_MAJOR],0)
m4_define([AMPICPLY_MINOR],9)

AC_INIT([ampicplayer], [AMPICPLY_MAJOR.AMPICPLY_MINOR], [kasin.li@amlogic.com])
AC_CONFIG_SRCDIR([pic_core/pic_app.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE(ampicplayer,[AMPICPLY_MAJOR.AMPICPLY_MINOR])

host_os=linux-gnu
AC_CANONICAL_HOST

dnl ========================check system type====================
# first disable framebuffer first.
want_framebuffer=no
native_system_type=linux
AC_MSG_CHECKING([for native system])
case "$host_os" in
  *mingw*)
    native_system_type=win32
    ;;
  *linux-*)
    native_system_type=unix
	want_framebuffer=yes
    ;;
  *)
    native_system_type=unknown
    ;;
esac
AC_MSG_RESULT([$native_system_type])

if test "x$native_system_type" = "xunknown"; then
	AC_MSG_ERROR([ failed to detect system type.])
fi

AM_CONDITIONAL(A_SYS_WIN32, test "x$native_system_type" = "xwin32")
if test "x$native_system_type" = "xwin32"; then
	AC_DEFINE(WIN32, [1], [runned in windows 32 mode])
fi


# Checks for programs.
AC_PROG_CC
AC_ARG_PROGRAM

# Checks for libraries.

dnl ========================bmp is supported by default====================
have_bmp=yes
AC_ARG_ENABLE(bmp,
[  --disable-bmp        Disable BMP support],
[  have_bmp=no]
)
AM_CONDITIONAL(SUPPORT_BMP, test "x$have_bmp" = "xyes")
if test "x$have_bmp" = "xyes"; then
   AC_DEFINE(USE_BMP, [1], [Define if supporting BMP])
fi


dnl ============================picture type config=========================
dnl libamljpeg
have_amljpeg=no
AC_ARG_ENABLE(amljpeg,
[  --enable-amljpeg        Enable JPEG support],
[  have_amljpeg=yes]
)
if test "x$have_amljpeg" = "xyes"; then
	AC_CHECK_HEADERS(amljpeg.h,[have_amljpeg=yes],[have_amljpeg=no])
fi
AM_CONDITIONAL(SUPPORT_AMLJPEG, test "x$have_amljpeg" = "xyes")
if test "x$have_amljpeg" = "xyes"; then
   AC_DEFINE(USE_AMLJPEG, [1], [Define if supporting amlogic jpeg])
fi

dnl ============================picture type config=========================
dnl libjpeg
have_jpeg=no
AC_ARG_ENABLE(jpeg,
[  --enable-jpeg        Enable JPEG support],
[  have_jpeg=yes]
)
if test "x$have_jpeg" = "xyes"; then
	AC_CHECK_HEADERS(jpeglib.h,[have_jpeg=yes],[have_jpeg=no])
fi
AM_CONDITIONAL(SUPPORT_JPEG, test "x$have_jpeg" = "xyes")
if test "x$have_jpeg" = "xyes"; then
   AC_DEFINE(USE_JPEG, [1], [Define if supporting jpeg])
fi

dnl libpng
have_png=no
AC_ARG_ENABLE(png,
[  --enable-png        Enable PNG support],
[  have_png=yes]
)
if test "x$have_png" = "xyes"; then
	AC_CHECK_HEADERS(png.h,[have_png=yes],[have_png=no])
fi
AM_CONDITIONAL(SUPPORT_PNG, test "x$have_png" = "xyes")
if test "x$have_png" = "xyes"; then
   AC_DEFINE(USE_PNG, [1], [Define if supporting png])
fi

dnl libgif
have_gif=no
AC_ARG_ENABLE(gif,
[  --enable-gif        Enable GIF support],
[  have_gif=yes]
)
if test "x$have_gif" = "xyes"; then
	AC_CHECK_HEADERS(gif_lib.h,[have_gif=yes],[have_gif=no])
fi
AM_CONDITIONAL(SUPPORT_GIF, test "x$have_gif" = "xyes")
if test "x$have_gif" = "xyes"; then
   AC_DEFINE(USE_GIF, [1], [Define if supporting GIF])
fi

var_want_player=yes
AC_ARG_ENABLE(picplayer,
[  --disable-picplayer        Enable Picture player],
[  var_want_player=no]
)
AM_CONDITIONAL(WANT_PLAYER, test "x$var_want_player" = "xyes")

dnl ============================videoout type config=========================
dnl framebuffer
want_framebuffer=yes
AC_ARG_ENABLE(fb,
[  --disable-fb        Disable FrameBuffer support],
[  want_framebuffer=no]
)
AM_CONDITIONAL(SUPPORT_FB, test "x$want_framebuffer" = "xyes")
if test "x$want_framebuffer" = "xyes"; then
   AC_DEFINE(USE_FB, [1], [Define if supporting Framebuffer])
   AC_DEFINE(DEFAULT_FRAMEBUFFER, ["/dev/fb0"], [default framebuffer])
fi

dnl GLES
want_gles=no
AC_ARG_ENABLE(gles,
[  --enable-gles        Enable gles support],
[  want_gles=yes]
)
if test "x$want_gles" = "xyes"; then
	AC_CHECK_HEADERS(GLES2/gl2.h,[want_gles=yes],[want_gles=no])
fi
AM_CONDITIONAL(SUPPORT_GLES, test "x$want_gles" = "xyes")
if test "x$want_gles" = "xyes"; then
   AC_DEFINE(USE_GLES, [1], [Define if supporting Framebuffer])
fi


#library check
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
# AC_FUNC_MALLOC
# AC_FUNC_MMAP
# AC_CHECK_FUNCS([munmap select strerror])

AC_OUTPUT([
Makefile
pic_core/Makefile

player/Makefile
]
)

dnl ==========================================================================
echo "

		ampicplayer $VERSION
		=====================

		system type:			  ${native_system_type}
		
		prefix:                   ${prefix}
		c compiler:               ${CC}
		c++ compiler:	          ${CXX}

		bmp support:           ${have_bmp}
		jpeg support:          ${have_jpeg}
		png support:           ${have_png}
		gif support:           ${have_gif}

		player support:			${var_want_player}

		fb support:           	${want_framebuffer}
		gles support:           ${want_gles}
		
		Now type 'make' to build $PACKAGE
"