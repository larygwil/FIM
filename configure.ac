dnl Process this file with autoconf to produce a configure script.
dnl $LastChangedDate: 2017-10-10 23:42:24 +0200 (Tue, 10 Oct 2017) $

AC_PREREQ([2.54])

# we'll move forward, one day or another
#AC_PREREQ([2.61])

# for future use, too
#AC_PROG_CC(cc gcc)
#AC_PROG_CXX(CC g++ gcc c++ cxx)

define([svnversion],esyscmd([sh -c "svnversion | tr  -c '[0-9]' ' ' | sed 's/ *//g;s/^$/-1/g' "]))dnl
# WARNING : here 0.0 denotes the package version, which will be corrected some lines after this.
AC_INIT([fim],[0.6-trunk],[dezperado_FOobAr_autistici_Baz_org, by replacing _FOobAr_ with a @ and _Baz_ with a .])
AC_CONFIG_SRCDIR([src/fim.cpp])

if test x"svnversion" != x ; then
	# The above test gets evaluated when creating the configure script.
	AC_DEFINE(SVN_REVISION,"svnversion",[SVN REVISION])
	AC_SUBST(SVN_REVISION,"svnversion")
	AC_DEFINE(SVN_REVISION_NUMBER,svnversion,[SVN REVISION NUMBER])
	AC_SUBST(SVN_REVISION_NUMBER,svnversion)
fi

AH_TEMPLATE([FIM_CONFIGURATION])
FIM_CONFIGURATION="$0 $@"
AC_SUBST(FIM_CONFIGURATION)
AC_DEFINE_UNQUOTED([FIM_CONFIGURATION],"$FIM_CONFIGURATION")

AC_MSG_NOTICE([
        Welcome to the Fim (revision $SVN_REVISION) configure script!
	This script will create the right Makefile for building Fim on your system.
	But first, it will check if your system has the right configuration for Fim to build.

	There are options to this script, please run :
	  ./configure --help=short
	for a complete list of these options.

	Please read BUGS and README and report any bug found to the Fim maintainer!
])


#AM_CONFIG_HEADER([config.h])
#AC_CONFIG_SRCDIR(pinentry/pinentry.h)

#
#AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)
AC_GNU_SOURCE
#AC_MANTAINER_MODE
#AC_CANONICAL_HOST

#missing_dir=`cd $ac_aux_dir && pwd`
#AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
#AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
#AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
#AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
#AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)



AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_LEX

AC_PROG_INSTALL
AC_PROG_RANLIB

AC_REVISION([$Revision$])

AC_PROG_CXX
#AC_PROG_CXXCPP
AC_CHECK_TOOL(AR, ar, ar)
fim_cv_version=`cat $srcdir/VERSION`
AC_LANG([C++])
#AC_LANG_PUSH([C++])
AM_INIT_AUTOMAKE
#AM_INIT_AUTOMAKE(fim, $fim_cv_version)
#AM_INIT_AUTOMAKE(fim, 1.0)
AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/VERSION'])
CPPFLAGS=''


AC_SUBST(FIM_LIBS)
AC_SUBST(FIM_LIB_OBJECTS)
AC_SUBST(VERSION)
dnl AC_SUBST(HTMLDOCS)
#AC_LANG_POP([C++])

fim="fim"
AC_MSG_CHECKING([for prefix])
if test x$prefix = xNONE; then
  fim_cv_prefix=$ac_default_prefix
else
  fim_cv_prefix=$prefix
fi
AC_MSG_RESULT($fim_cv_prefix)

AC_ISC_POSIX
#AM_C_PROTOTYPES
if test "x$U" != "x"; then
  AC_MSG_ERROR(Compiler not ANSI compliant)
fi
#AC_PROG_CPP
AC_PROG_MAKE_SET

AC_C_INLINE
AC_C_CONST

AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_CHECK_SIZEOF(off_t)
AM_MAINTAINER_MODE

AH_TEMPLATE([HAVE_GETENV])
AC_CHECK_FUNC([getenv], AC_DEFINE([HAVE_GETENV], [1], [If present, the getenv function allows fim to read environment variables.]))
AC_CHECK_FUNC([getline], AC_DEFINE([HAVE_GETLINE], [1], [If present, will be used for stdin reading. (on GNU)]))
AC_CHECK_FUNC([getdelim], AC_DEFINE([HAVE_GETDELIM], [1], [If present, will be used for stdin reading. (on GNU)]))
AC_CHECK_FUNC([fgetln], AC_DEFINE([HAVE_FGETLN], [1], [If present, fgetln() will be used as a replacement for getline(). (on BSD)]))
AC_CHECK_FUNC([get_current_dir_name], AC_DEFINE([HAVE_GET_CURRENT_DIR_NAME], [1], [Should live when _GNU_SOURCE, but it could be not the case.]))
AC_CHECK_FUNC([rsb_lib_init])

AC_CHECK_LIB([m],[pow])
AC_CHECK_LIB([fl],[yylex])
AC_ARG_ENABLE(graphicsmagick, AC_HELP_STRING([--enable-graphicsmagick], [Enable GraphicsMagick library support (EXPERIMENTAL).]), [ if test x$enableval != xno ; then fim_handle_graphicsmagick=yes ; fi ] )
AC_CHECK_LIB([GraphicsMagick],[GetMagickVersion],
  [
if test x"$fim_handle_graphicsmagick" = x"yes" ; then 
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffMagick.o";
  LIBS="$LIBS `GraphicsMagick-config --cppflags --ldflags --libs|tr '\n' ' '` ";
  CXXFLAGS="$CXXFLAGS `GraphicsMagick-config --cppflags |tr '\n' ' '` ";
  AC_DEFINE([HAVE_LIBGRAPHICSMAGICK],[1],[We have libGraphicsMagick])
fi
  ], [])

AC_ARG_ENABLE(imlib2, AC_HELP_STRING([--enable-imlib2], [Enable Imlib2 library support (EXPERIMENTAL,INCOMPLETE).]), [ if test x$enableval != xno ; then fim_handle_imlib2=yes ; fi ] )
AC_CHECK_LIB([Imlib2],[imlib_create_image],
  [
if test x"$fim_handle_imlib2" = x"yes" ; then 
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS Imlib2Device.o";
  LIBS="$LIBS `imlib2-config --libs|tr '\n' ' '` ";
  CXXFLAGS="$CXXFLAGS `imlib2-config --cflags |tr '\n' ' '` ";
  AC_DEFINE([FIM_WITH_LIBIMLIB2],[1],[We have libImlib])
fi
  ], [])

AC_CHECK_PROG([HAVE_FLEX],[flex],[yes],[no])
AC_CHECK_PROG([HAVE_BISON],[bison],[yes],[no])
if test x$HAVE_FLEX  != x"yes" ; then
       	AC_MSG_ERROR([No flex (lexical analyser generator) executable detected!]) ;
fi
if test x$HAVE_BISON != x"yes" ; then
       	AC_MSG_ERROR([No bison (parser generator) executable detected!]) ;
fi

AC_CHECK_PROG([HAVE_ECHO],[echo],[yes],[no])
AC_CHECK_PROG([HAVE_RM],[rm],[yes],[no])
AC_CHECK_PROG([HAVE_SED],[sed],[yes],[no])
AC_CHECK_PROG([HAVE_GREP],[grep],[yes],[no])
AC_CHECK_PROG([HAVE_SORT],[sort],[yes],[no])
AC_CHECK_PROG([HAVE_CPP],[cpp],[yes],[no])
AC_CHECK_PROG([HAVE_AWK],[awk],[yes],[no])
AC_CHECK_PROG([HAVE_CAT],[cat],[yes],[no])
AC_CHECK_PROG([HAVE_MAN],[man],[yes],[no])
#AC_CHECK_PROG([HAVE_WHICH],[which],[yes],[no])
#AC_CHECK_PROG([HAVE_GPG],[gpg],[yes],[no])
#AC_CHECK_PROG([HAVE_LS],[ls],[yes],[no])
#AC_CHECK_PROG([HAVE_CD],[cd],[yes],[no])
#AC_CHECK_PROG([HAVE_CC],[cc],[yes],[no])
dnl
AC_ARG_VAR([LIBTIFF],[libtiff library (contributes to LIBS)])
AC_ARG_VAR([LIBGIF],[libgif library (contributes to LIBS)])
AC_ARG_VAR([LIBJPEG],[libjpeg library (contributes to LIBS)])
AC_ARG_VAR([LIBPS],[libspectre library (contributes to LIBS)])
AC_ARG_VAR([LIBDJVU],[libdjvu libdjvulibre (contributes to LIBS)])
AC_ARG_VAR([LIBPOPPLER],[poppler libdjvulibre (contributes to LIBS)])
dnl
AC_PATH_PROG([RM],[rm],[no])
AC_PATH_PROG([SED],[sed],[no])
AC_PATH_PROG([GREP],[grep],[no])
AC_PATH_PROG([ECHO],[echo],[no])
AC_PATH_PROG([SORT],[sort],[no])
AC_PATH_PROG([CPP],[cpp],[no])
AC_PATH_PROG([AWK],[awk],[no])
AC_PATH_PROG([CAT],[cat],[no])
AC_PATH_PROG([MAN],[man],[no])
#AC_PATH_PROG([CC],[cc],[no])
#AC_PATH_PROG([WHICH],[which],[no])
#AC_PATH_PROG([GPG],[gpg],[no])
#AC_PATH_PROG([LS],[ls],[no])
#AC_PATH_PROG([CD],[cd],[no])
# FIXME: what if we have no grep and/or no sed ?
# (echo is often a shell builtin)
have_default_bindings=yes
have_default_mapping=yes
have_default_config=yes

AC_ARG_ENABLE(default_bindings, AC_HELP_STRING([--disable-default-bindings], [Builtin default minimal key binding]),
[       if test x$enableval = xyes; then
		have_default_bindings=yes
	else
                have_default_bindings=no
        fi
])

AC_ARG_ENABLE(default_mapping, AC_HELP_STRING([--disable-default-mapping], [Builtin default minimal key mapping]),
[       if test x$enableval = xyes; then
                have_default_mapping=yes
	else
		have_default_mapping=no
        fi
])

AC_ARG_ENABLE(default_config, AC_HELP_STRING([--disable-default-config], [Builtin default minimal configuration]),
[       if test x$enableval = xyes; then
                have_default_config=yes
	else
		have_default_config=no
        fi
])

	AH_TEMPLATE([FIM_DEFAULT_CONFIG])
	if test x$have_default_bindings != xno; then
		FIM_DEFS="$FIM_DEFS -D FIM_DEFAULT_CONFIG"
		AC_DEFINE([FIM_DEFAULT_CONFIG])
	fi
	AH_TEMPLATE([FIM_DEFAULT_KEY_CONFIG])
	if test x$have_default_mapping != xno; then
		FIM_DEFS="$FIM_DEFS -D FIM_DEFAULT_KEY_CONFIG"
		AC_DEFINE([FIM_DEFAULT_KEY_CONFIG])
	fi
	AH_TEMPLATE([FIM_DEFAULT_CONFIGURATION])
	if test x$have_default_config != xno; then
		FIM_DEFS="$FIM_DEFS -D FIM_DEFAULT_CONFIGURATION"
		AC_DEFINE([FIM_DEFAULT_CONFIGURATION])
	fi

#CPPFLAGS=$DEPS
#AC_SUBST(LIBSDL_CXXFLAGS)
#AC_SUBST(LIBPNG_CXXFLAGS)
#AC_SUBST(LIBSDL_CXXFLAGS)
#AC_SUBST(LIBPNG_CXXFLAGS)
#AC_SUBST(LIBPNG_LIBS)

AC_ARG_ENABLE(system, AC_HELP_STRING([--disable-system], [Disable system() and pipe() calls.]),
[       if test x$enableval = xno ; then
                fim_system_calls=no
        fi
])

dnl new: 
AC_ARG_ENABLE(framebuffer, AC_HELP_STRING([--disable-framebuffer], [Disable the framebuffer device driver.]),
[       if test x$enableval = xno ; then
                fim_handle_framebuffer=no
        fi
])

AC_MSG_NOTICE([Checking for headers required by the Linux framebuffer device.])
AC_CHECK_HEADERS([linux/fb.h], [break], AC_MSG_WARN([[Cannot find linux/fb.h.  Fim Linux framebuffer driver requires it!.]]))
AC_CHECK_HEADERS([linux/vt.h], [break], AC_MSG_WARN([[Cannot find linux/vt.h.  Fim Linux framebuffer driver requires it!.]]))
AC_CHECK_HEADERS([linux/kd.h], [break], AC_MSG_WARN([[Cannot find linux/kd.h.  Fim Linux framebuffer driver requires it!.]]))

if test x$ac_cv_header_linux_fb_h != xyes -o  \
	 x$ac_cv_header_linux_vt_h != xyes -o \
	 x$ac_cv_header_linux_kd_h != xyes ; then
	AC_MSG_WARN([Turning off Linux framebuffer support due to missing headers])
	fim_handle_framebuffer=no 
fi

if test x$fim_handle_framebuffer = xno ; then
	AC_DEFINE(FIM_WITH_NO_FRAMEBUFFER, 1, [Defined when framebuffer device support is turned off.])
fi

AC_HEADER_STDC
AC_FUNC_MMAP

AC_CHECK_FUNCS([bzero memset memcmp strncmp strcpy])
AC_CHECK_FUNCS([bcopy bcmp])
AC_CHECK_FUNCS([drand48 random rand])
AC_CHECK_FUNCS([fileno])
AC_CHECK_FUNCS([fmemopen])
AC_CHECK_HEADERS([wordexp.h glob.h fnmatch.h])
dnl AC_FUNC_REALLOC()	dnl triggers error in automake 1.10.1

#AC_CHECK_HEADERS([stddef.h])
#AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h strings.h], [AC_CHECK_FUNCS(strerror)])

AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([wchar.h])
AC_CHECK_HEADERS([fontconfig/fontconfig.h])
AC_CHECK_HEADERS([fontconfig/fcfreetype.h])

AC_HEADER_DIRENT()
AC_CHECK_HEADERS([stdarg.h sys/ioctl.h ioctl.h sysexits.h])
AC_CHECK_HEADERS([sys/time.h sys/resource.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([unix.h])
AC_CHECK_HEADERS([libgen.h])
AC_CHECK_HEADERS([sys/user.h])
AC_CHECK_HEADERS([sys/mman.h])
AC_CHECK_HEADERS([cstdio stdio.h], [break],AC_MSG_ERROR([[Cannot find cstdio or stdio.h.  Fim requires at least one or the other.]]))

AC_CHECK_HEADERS([climits limits.h], [break], AC_MSG_ERROR([[Cannot find limits.h nor climits.  Fim requires it!.]]))

AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([int *])
AC_CHECK_SIZEOF([size_t])
AC_C_BIGENDIAN([fim_big_endian=yes],[fim_big_endian=no],[fim_big_endian=hmmm])

if test "x$fim_big_endian" = "xyes" ; then
	AH_TEMPLATE([FIM_BIG_ENDIAN])
	AC_DEFINE([FIM_BIG_ENDIAN])
fi


dnl ============= begin misc external programs detection
dnl ============= end misc external programs detection

dnl if test "X$ac_cv_header_cstdio" != "Xyes" && test "X$ac_cv_header_stdio_h" != "Xyes"; then
dnl   AC_MSG_ERROR([[Cannot find cstdio or stdio.h.  Fim requires at least one or the other.]])
dnl fi

dnl if test "X$ac_cv_header_linux_fb_h" != "Xyes" ; then
dnl   AC_MSG_ERROR([[Cannot find linux/fb.h.  Fim requires it!.]])
dnl fi



dnl AC_CHECK_FUNCS(setrlimit getsid)

AC_TYPE_SIGNAL

dnl ==================
AH_TEMPLATE([FIM_WANT_STATIC_BINARY])
AC_ARG_ENABLE(static, AC_HELP_STRING([--enable-static], [Want a static binary. (disabled by default)]), [       if test x$enableval = xyes; then fim_cv_want_static=1 ; else fim_cv_want_static=0 ; fi ],[ fim_cv_want_static=0 ])
if test x$fim_cv_want_static == x1 ; then 
	LIBS="$LIBS --static"
        LIBDJVU_STATIC_EXTRA="-lpthread"
	# too many dependencies for the following!
        LIBPOPPLER_STATIC_EXTRA=""
        LIBSPECTRE_STATIC_EXTRA=""
        #LIBPOPPLER_STATIC_EXTRA="-lfreetype -llcms"
        #LIBSPECTRE_STATIC_EXTRA="-Wl,--start-group -lgs -lfontconfig -ldl -ljbig2dec -ljasper -lpthread -lpng -lppd -lpaper -lavahi-common -lavahi-client -lexpat -lfreetype -lgnutls -ldbus-1 -ltasn1 -lcupsimage -lcups -lgcrypt -Wl,--end-group "
	LIBTIFF_STATIC_EXTRA="-ljpeg -lz"
	AC_DEFINE_UNQUOTED([FIM_WANT_STATIC_BINARY], $fim_cv_want_static [Want a static binary. (disabled by default)])
fi
dnl ==================


AC_ARG_WITH(regex, AC_HELP_STRING([--with-regex], [Use the GNU regex library instead of the POSIX one]),
        [fim_cv_regex=yes],
        [AC_CHECK_FUNCS(regcomp, fim_cv_regex=no, fim_cv_regex=yes)])

if test X$fim_cv_regex = Xno ; then
AC_CACHE_CHECK([whether your system regexp library is completely broken],
        [fim_cv_regex_broken],
        AC_TRY_RUN([
#include <unistd.h>
#include <regex.h>
int main() { regex_t blah ; regmatch_t p; p.rm_eo = p.rm_eo; return regcomp(&blah, "foo.*bar", REG_NOSUB) || regexec (&blah, "foobar", 0, NULL, 0); }],
        fim_cv_regex_broken=no, fim_cv_regex_broken=yes, fim_cv_regex_broken=yes))
        if test X$fim_cv_regex_broken = Xyes ; then
                echo "Using the included GNU regex instead." >&AC_FD_MSG
                fim_cv_regex=yes
        fi
fi

if test X$fim_cv_regex = Xyes; then
        AC_DEFINE(USE_GNU_REGEX,1,[ Define if you want to use the included (GNU) regex.c. ])
        AC_LIBOBJ(regex)
fi



AC_MSG_CHECKING(where to put the documentation...)
dnl AC_ARG_WITH(docdir, AC_HELP_STRING([--with-docdir=PATH], [Specify where to put the documentation]),
dnl         [fim_cv_docdir=$withval],
dnl        [fim_cv_docdir='${prefix}/doc/fim-${fim_cv_version}'])
# FIXME : the above line seems troublesome, BUT IT IS NOT (see the muttrc for this trick).

AC_MSG_RESULT($fim_cv_docdir)

dnl docdir=$fim_cv_docdir
dnl AC_SUBST(docdir)
AC_SUBST(fim_cv_version)

dnl ============= gif file handling

AC_ARG_ENABLE(gif, AC_HELP_STRING([--disable-gif], [Disable gif support.]),
[       if test x$enableval = xno ; then
		fim_handle_gif=no
        fi
])
if test x$fim_handle_gif != xno ; then
  fim_handle_gif=yes
  have_gif=yes
  if test x$with_libgif != xno && test -z "$LIBGIF"; then
    AC_CHECK_LIB(gif,DGifOpenFileHandle ,
      have_gif=yes,
      have_gif=no
      AC_MSG_WARN([*** GIF loader will not be built (GIF library not found) ***]))
    if test x"$have_gif" = xyes; then
      AC_MSG_CHECKING([for gif.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <gif_lib.h>],
        have_gif=yes,
        have_gif=no)
      AC_MSG_RESULT($have_gif)
      if test x"$have_gif" = xyes; then
        LIBGIF='-lgif'
      else
          AC_MSG_WARN([*** GIF loader will not be built (GIF header file not found) ***])
      fi
    fi
  fi

#  if test x$with_libgif != xno ; then
# the following needs thorough testing
  if test x"$have_gif" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffGif.o"
  LIBS="$LIBS $LIBGIF"
  AC_DEFINE([FIM_HANDLE_GIF])
  else
    fim_handle_gif=no
  fi

#  if test x$with_libgif != xno && test -z "$LIBGIF"; then
  #
#  fi
else
  fim_handle_gif=no
fi
AC_SUBST(LIBGIF)
AM_CONDITIONAL(ENABLE_GIF, test x$have_gif = xyes)


dnl ============= tiff file handling

AC_ARG_ENABLE(tiff, AC_HELP_STRING([--disable-tiff], [Disable tiff support.]),
[       if test x$enableval = xno ; then
		fim_handle_tiff=no
        fi
])
if test x$fim_handle_tiff != xno ; then
  fim_handle_tiff=yes
  have_tiff=yes
  if test x$with_libtiff != xno && test -z "$LIBTIFF"; then
    AC_CHECK_LIB(tiff,TIFFOpen ,
      have_tiff=yes,
      have_tiff=no
      AC_MSG_WARN([*** TIFF loader will not be built (TIFF library not found) ***]) , [$LIBTIFF_STATIC_EXTRA])
    if test x"$have_tiff" = xyes; then
      AC_MSG_CHECKING([for tiff.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <tiff.h>],
        have_tiff=yes,
        have_tiff=no)
      AC_MSG_RESULT($have_tiff)
      if test x"$have_tiff" = xyes; then
        LIBTIFF="-ltiff $LIBTIFF_STATIC_EXTRA"
      else
          AC_MSG_WARN([*** TIFF loader will not be built (TIFF header file not found) ***])
      fi
    fi
  fi

#  if test x$with_libtiff != xno ; then
# the following needs thorough testing
  if test x"$have_tiff" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffTiff.o"
  LIBS="$LIBS $LIBTIFF"
  AC_DEFINE([FIM_HANDLE_TIFF])
  else
    fim_handle_tiff=no
  fi

#  if test x$with_libtiff != xno && test -z "$LIBTIFF"; then
  #
#  fi
else
  fim_handle_tiff=no
fi
AC_SUBST(LIBTIFF)
AM_CONDITIONAL(ENABLE_TIFF, test x$have_tiff = xyes)


dnl ============= curses: optional
AC_CHECK_LIB([curses],[noecho])
dnl ============= ncurses: optional
AC_CHECK_LIB([ncurses],[noecho])

dnl ============= cacalib : optional
AC_CHECK_LIB(caca, caca_create_canvas,have_cacalib=yes,have_cacalib=no)

dnl ============= GNU gpm, unofficial yet

#    AC_CHECK_LIB(gpm,Gpm_Open,
#      ,true,
#      AC_MSG_ERROR(*** gpm lib not found ... no problem ... ***))
#LIBS="$LIBS -lgpm"

dnl ============= C++ version handling

fim_cxx_standard=98
AC_ARG_ENABLE(c++11, AC_HELP_STRING([--enable-c++11], [Enable C++11 standard constructs (requires appropriate CXXFLAGS, e.g. -std=c++11).]), [       if test x$enableval = xno ; then true; else fim_cxx_standard=11; fi ])
AC_ARG_ENABLE(c++14, AC_HELP_STRING([--enable-c++14], [Enable C++14 standard constructs (requires appropriate CXXFLAGS, e.g. -std=c++14) (EXPERIMENTAL).]), [       if test x$enableval = xno ; then true; else fim_cxx_standard=14; fi ])
AC_ARG_ENABLE(c++17, AC_HELP_STRING([--enable-c++17], [Enable C++17 standard constructs (requires appropriate CXXFLAGS, e.g. -std=c++17) (EXPERIMENTAL).]), [       if test x$enableval = xno ; then true; else fim_cxx_standard=17; fi ])
dnl ============= no readline case handling

fim_want_readline=yes
AC_ARG_ENABLE(readline, AC_HELP_STRING([--disable-readline], [Disable readline support (EXPERIMENTAL).]),
[       if test x$enableval = xno ; then
		fim_want_readline=no
	else
		fim_want_readline=yes
        fi
])

dnl ============= GNU readline : essential

dnl Please note that when the readline library is compiled with the curses library, this check will fail, because
dnl we cannot know in advance if it is so.
AH_TEMPLATE([FIM_USE_READLINE])
AH_TEMPLATE([FIM_HANDLE_GIF])
AH_TEMPLATE([FIM_HANDLE_TIFF])

have_readline=no

# Sometimes the readline library is linked to the curses library, and we here try to detect this case..
if test x$have_readline = xno; then
  if test x$fim_want_readline = xyes ; then
    AC_CHECK_LIB(readline, rl_replace_line,
      have_readline=yes
      LIBS="$LIBS -lreadline"
      AC_DEFINE([FIM_USE_READLINE])
      ,have_readline=no
      AC_MSG_ERROR(*** GNU readline (or curses library ?) not found (used LIBS=${LIBS}) ! please install them or specify them in e.g. LIBS=... or run ./configure --disable-readline ***)
      ,[]
      )
  else
      have_readline=no
      AC_MSG_NOTICE(*** Fim will be built withoud GNU readline library headers ! Are you sure ?  ***)
  fi
fi

dnl FIXME : we try to detecht curses, although we do not know if the preceding did use it
if test x$have_readline = xyes; then
  if test x$fim_want_readline = xyes ; then
    AC_CHECK_LIB(curses, tputs,
      have_curses=yes
      LIBS="$LIBS"
      ,have_curses=no
      )
  fi
fi

dnl ============= jpeg file handling

#AC_ARG_WITH(jpeg, [  --disable-jpeg ], [ disable jpeg file support])
AC_ARG_ENABLE(jpeg, AC_HELP_STRING([--disable-jpeg], [Disable jpeg support.]),
[       if test x$enableval = xno ; then
		fim_handle_jpeg=no
        fi
])
if test x$fim_handle_jpeg != xno ; then
  fim_handle_jpeg=yes
  have_jpeg=yes
  if test x$with_libjpeg != xno && test -z "$LIBJPEG"; then
    AC_CHECK_LIB(jpeg, jpeg_destroy_decompress,
      have_jpeg=yes,
      have_jpeg=no
      AC_MSG_WARN([*** JPEG loader will not be built (JPEG library not found) ***]))
    if test x"$have_jpeg" = xyes; then
      AC_MSG_CHECKING([for jpeglib.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <jpeglib.h>],
        have_jpeg=yes,
        have_jpeg=no)
      AC_MSG_RESULT($have_jpeg)
      if test x"$have_jpeg" = xyes; then
        LIBJPEG='-ljpeg'
      else
          AC_MSG_WARN([*** JPEG loader will not be built (JPEG header file not found) ***])
      fi
    fi
  fi

#  if test x$with_libjpeg != xno ; then
# the following needs thorough testing
  if test x"$have_jpeg" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffJpeg.o"
  LIBS="$LIBS $LIBJPEG"
  AC_DEFINE([HAVE_LIBJPEG],[1],[We have libjpeg])
  else
    fim_handle_jpeg=no
  fi

#  if test x$with_libjpeg != xno && test -z "$LIBJPEG"; then
#     AC_MSG_ERROR([
#     Checks for JPEG loader failed. You can build without it by passing
#    --disable-jpeg to ./configure. ])
#  fi
else
  fim_handle_jpeg=no
fi
AC_SUBST(LIBJPEG)
AM_CONDITIONAL(ENABLE_JPEG, test x$have_jpeg = xyes)

dnl ============= dvi file handling (via TeX-Guy)
dnl 
dnl AC_ARG_ENABLE(dvi, AC_HELP_STRING([--disable-dvi], [Disable dvi support.]),
dnl [       if test x$enableval = xno ; then
dnl 		fim_handle_dvi=no
dnl         fi
dnl ])
dnl if test x$fim_handle_dvi != xno ; then
dnl   fim_handle_dvi=yes
dnl   have_dvi=yes
dnl   if test x$with_libdvi != xno && test -z "$LIBDVI"; then
dnl     AC_CHECK_LIB(dvilib2, DVI_init,
dnl       have_dvi=yes,
dnl       have_dvi=no
dnl       AC_MSG_WARN([*** DVI loader will not be built (libdvi29 library not found) ***]))
dnl     if test x"$have_dvi" = xyes; then
dnl       AC_MSG_CHECKING([for libdvi29.h])
dnl       AC_TRY_CPP(
dnl [#include <stdio.h>
dnl #undef PACKAGE
dnl #undef VERSION
dnl #undef HAVE_STDLIB_H
dnl #include <libdvi29.h>],
dnl         have_dvi=yes,
dnl         have_dvi=no)
dnl       AC_MSG_RESULT($have_dvi)
dnl       if test x"$have_dvi" = xyes; then
dnl         LIBDVI='-ldvilib2'
dnl       else
dnl           AC_MSG_WARN([*** DVI loader will not be built (libdvi29.h header file not found) ***])
dnl       fi
dnl     fi
dnl   fi
dnl 
dnl # the following needs thorough testing
dnl   if test x"$have_dvi" == xyes ; then
dnl   FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffDvi.o"
dnl   LIBS="$LIBS $LIBDVI"
dnl   AC_DEFINE([HAVE_LIBSPECTRE],[1],[We have libdvilib2])
dnl   else
dnl     fim_handle_dvi=no
dnl   fi
dnl 
dnl #  if test x$with_libdvi != xno && test -z "$LIBDVI"; then
dnl #     AC_MSG_ERROR([
dnl #     Checks for DVI loader failed. You can build without it by passing
dnl #    --disable-dvi to ./configure. ])
dnl #  fi
dnl else
dnl   fim_handle_dvi=no
dnl fi
dnl AC_SUBST(LIBDVI)
dnl AM_CONDITIONAL(ENABLE_DVI, test x$have_dvi = xyes)

dnl ============= ps file handling

AC_ARG_ENABLE(ps, AC_HELP_STRING([--disable-ps], [Disable ps support.]),
[       if test x$enableval = xno ; then
		fim_handle_ps=no
        fi
])
if test x$fim_handle_ps != xno ; then
  fim_handle_ps=yes
  have_ps=yes
  if test x$with_libps != xno && test -z "$LIBPS"; then
    AC_CHECK_LIB(spectre, spectre_page_render,
      have_ps=yes,
      have_ps=no
      AC_MSG_WARN([*** PS loader will not be built (spectre library not found) ***]) , [$LIBSPECTRE_STATIC_EXTRA])
    if test x"$have_ps" = xyes; then
      AC_MSG_CHECKING([for libspectre/spectre.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <libspectre/spectre.h>],
        have_ps=yes,
        have_ps=no)
      AC_MSG_RESULT($have_ps)
      if test x"$have_ps" = xyes; then
        LIBPS="-lspectre $LIBSPECTRE_STATIC_EXTRA"
      else
          AC_MSG_WARN([*** PS loader will not be built (PS header file not found) ***])
      fi
    fi
  fi

# the following needs thorough testing
  if test x"$have_ps" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffPs.o"
  LIBS="$LIBS $LIBPS"
  AC_DEFINE([HAVE_LIBSPECTRE],[1],[We have libspectre])
  else
    fim_handle_ps=no
  fi

#  if test x$with_libps != xno && test -z "$LIBPS"; then
#     AC_MSG_ERROR([
#     Checks for PS loader failed. You can build without it by passing
#    --disable-ps to ./configure. ])
#  fi
else
  fim_handle_ps=no
fi
AC_SUBST(LIBPS)
AM_CONDITIONAL(ENABLE_PS, test x$have_ps = xyes)



dnl ============= djvu file handling

AC_ARG_ENABLE(djvu, AC_HELP_STRING([--disable-djvu], [Disable djvu support.]),
[       if test x$enableval = xno ; then
		fim_handle_djvu=no
        fi
])
if test x$fim_handle_djvu != xno ; then
  fim_handle_djvu=yes
  have_djvu=yes
  if test x$with_libdjvu != xno && test -z "$LIBDJVU"; then
    AC_CHECK_LIB(djvulibre, ddjvu_page_create_by_pageno,
      have_djvu=yes,
      have_djvu=no
      AC_MSG_WARN([*** DJVU loader will not be built (DJVU library not found) ***]), [$LIBDJVU_STATIC_EXTRA])
    if test x"$have_djvu" = xyes; then
      AC_MSG_CHECKING([for libdjvu/ddjvuapi.h])
      AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <libdjvu/ddjvuapi.h>],
        have_djvu=yes,
        have_djvu=no)
      AC_MSG_RESULT($have_djvu)
      if test x"$have_djvu" = xyes; then
        LIBDJVU="-ldjvulibre $LIBDJVU_STATIC_EXTRA"
      else
          AC_MSG_WARN([*** DJVU loader will not be built (DJVU header file not found) ***])
      fi
    fi
  fi

# the following needs thorough testing
  if test x"$have_djvu" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffDjvu.o"
  LIBS="$LIBS $LIBDJVU"
  AC_DEFINE([HAVE_LIBDJVU],[1],[We have libdjvulibre])
  else
    fim_handle_djvu=no
  fi

#  if test x$with_libdjvu != xno && test -z "$LIBDJVU"; then
#     AC_MSG_ERROR([
#     Checks for DJVU loader failed. You can build without it by passing
#    --disable-djvu to ./configure. ])
#  fi
else
  fim_handle_djvu=no
fi
AC_SUBST(LIBDJVU)
AM_CONDITIONAL(ENABLE_DJVU, test x$have_djvu = xyes)


dnl ============= pdf file handling with libpoppler 
dnl
dnl By default, we disable poppler : its API is still immature.
dnl
fim_handle_pdf=no
AC_ARG_ENABLE(poppler, AC_HELP_STRING([--enable-poppler], [Enable poppler (PDF rendering) support. This will add LIBPOPPLER_CXXFLAGS="-I/usr/include/poppler/" to the compilation flags. You can override this by passing configure a suitable LIBPOPPLER_CXXFLAGS variable.]),
[       if test x$enableval = xyes ; then
		fim_handle_pdf=yes
        fi
], [$LIBPOPPLER_STATIC_EXTRA])
dnl FIXME : checking only for libpoppler-glib, should also check for libpoppler !
if test x$fim_handle_pdf != xno ; then
  fim_handle_pdf=yes
  have_poppler=yes
        if test x$with_libpoppler != xno && test -z "$LIBPOPPLER"; then
dnl FIXME : the following is a dirty hack
          AC_CHECK_LIB(poppler, gfree,
            have_poppler=yes,
            have_poppler=no
      AC_MSG_WARN([*** POPPLER loader will not be built (POPPLER library not found) ***]) )
    if test x"$have_poppler" = xyes; then
      AC_MSG_CHECKING([for poppler/poppler-config.h])
       AC_TRY_CPP(
       [#include <stdio.h>
       #undef PACKAGE
       #undef VERSION
       #undef HAVE_STDLIB_H
       /* fim uses these headers. let us hope poppler will stabilize soon!*/
       #include <poppler/poppler-config.h>
       #include <poppler/PDFDoc.h>
       #include <poppler/OutputDev.h>
       #include <poppler/SplashOutputDev.h>
       #include <poppler/splash/SplashBitmap.h>
       #include <poppler/splash/SplashTypes.h>
       #include <poppler/Page.h>
       #include <poppler/GlobalParams.h>	/* globalParams lives here */
       ],
               have_poppler=yes,
               have_poppler=no)
      have_poppler=yes
      AC_MSG_RESULT($have_poppler)
      if test x"$have_poppler" = xyes; then
        LIBPOPPLER="-lpoppler $LIBPOPPLER_STATIC_EXTRA"
	# FIXME: the following is a dirty hack!
	if test x"$LIBPOPPLER_CXXFLAGS" = x ; then
        	LIBPOPPLER_CXXFLAGS="-I/usr/include/poppler/"
	fi
        CXXFLAGS="$CXXFLAGS $LIBPOPPLER_CXXFLAGS -DFIM_LIBPOPPLER_VERSION_MAYBE=$have_poppler_maybe_version"
dnl	are we sure poppler does not have something like the following ?
dnl        LIBPOPPLER_LIBS=`$LIBPOPPLER_CONFIG --libs`
      else
          AC_MSG_WARN([*** POPPLER/PDF loader will not be built (POPPLER header file not found) ***])
      fi
    fi
  fi

# the following needs thorough testing
  if test x"$have_poppler" == xyes ; then
  FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffPdf.o"
  LIBS="$LIBS $LIBPOPPLER"
  AC_DEFINE([HAVE_LIBPOPPLER],[1],[We have libpoppler (Warning: the API could still break! (as of v.0.8.7--0.24.1))])
  else
    fim_handle_pdf=no
  fi

#  if test x$with_libpoppler != xno && test -z "$LIBPOPPLER"; then
#     AC_MSG_ERROR([
#     Checks for POPPLER loader failed. You can build without it by passing
#    --disable-poppler to ./configure. ])
#  fi
else
  fim_handle_pdf=no
fi
AC_SUBST(LIBPOPPLER)
AM_CONDITIONAL(ENABLE_POPPLER, test x$have_poppler = xyes)

dnl FIXME : we have two libpoppler checks ... :
dnl ================== pdf checks ===================================================
AC_ARG_ENABLE(pdf,
              [AC_HELP_STRING([--enable-pdf], [Compile with pdf support])],enable_pdf="$enableval",enable_pdf=yes)

if test "x$enable_pdf" = "xyes"; then
    POPPLER_REQUIRED=0.8.7
    # dear fim hackers, you should install pkg-config to get this macro expanded
    PKG_CHECK_MODULES(POPPLER, poppler >= $POPPLER_REQUIRED,enable_pdf=yes,enable_pdf=no)

    if test "x$enable_pdf" = "xyes"; then
            AC_DEFINE([ENABLE_PDF], [1], [Enable pdf support.])
            SHELL_LIBS="$SHELL_LIBS $POPPLER_LIBS"
            SHELL_CXXFLAGS="$SHELL_CXXFLAGS $POPPLER_CXXFLAGS"

            fim_save_LIBS=$LIBS
            LIBS="$LIBS $POPPLER_LIBS"
            LIBS=$fim_save_LIBS

    else
            AC_MSG_WARN(["PDF support is disabled since poppler library version $POPPLER_REQUIRED or newer not found"])
    fi
fi

AM_CONDITIONAL(ENABLE_PDF, test x$enable_pdf = xyes)
dnl ================== end of pdf checks ============================================

dnl ================================================================================
fim_handle_bmp=yes
AC_ARG_ENABLE(bmp, AC_HELP_STRING([--disable-bmp], [Disable BMP file support (enabled by default).]),
[       if test x$enableval = xno ; then
		fim_handle_bmp=no
        fi
])
if test x$fim_handle_bmp = xyes; then
	AC_MSG_NOTICE([Enabling BMP file format support.])
	AC_DEFINE(FIM_WITH_BMP, 1, [BMP file support.])
	FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffBmp.o"
else
	AC_MSG_NOTICE([Disabling BMP file format support.])
	AC_DEFINE(FIM_WITH_BMP, 0, [BMP file support.])
fi
dnl ================================================================================
fim_handle_pcx=yes
AC_ARG_ENABLE(pcx, AC_HELP_STRING([--disable-pcx], [Disable PCX file support (enabled by default).]),
[       if test x$enableval = xno ; then
		fim_handle_pcx=no
        fi
])
if test x$fim_handle_pcx = xyes; then
	AC_MSG_NOTICE([Enabling PCX file format support.])
	AC_DEFINE(FIM_WITH_PCX, 1, [PCX file support.])
	FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffPcx.o"
else
	AC_MSG_NOTICE([Disabling PCX file format support.])
	AC_DEFINE(FIM_WITH_PCX, 0, [PCX file support.])
fi
dnl ================================================================================

## png file handling

AC_ARG_WITH(png-prefix, AC_HELP_STRING([--with-png-prefix=PFX], [prefix where libpng is installed (optional)]),
libpng_config_prefix="$withval", libpng_config_prefix="")

fim_handle_png=yes

AC_ARG_ENABLE(png, AC_HELP_STRING([--disable-png], [Disable png support.]),
[       if test x$enableval = xno ; then
		fim_handle_png=no
        fi
])

if test x$fim_handle_png = xyes; then
	# support for custom library configurator path
        if test x$libpng_config_prefix != x; then
                LIBPNG_CONFIG="$libpng_config_prefix/bin/libpng-config"
        else
		# default path configurator
                AC_PATH_PROG(LIBPNG_CONFIG, libpng-config, no)
        fi
        if test "x$LIBPNG_CONFIG" = "xno"; then
      		AC_MSG_WARN([*** PNG loader will not be built (PNG library not found) ***])
		fim_handle_png=no
        else
                CXXFLAGS="$CXXFLAGS `$LIBPNG_CONFIG --cflags`"
		if test x$fim_cv_want_static == x1 ; then 
                LIBS="$LIBS `$LIBPNG_CONFIG --static --libs`"
		else
                LIBS="$LIBS `$LIBPNG_CONFIG --libs`"
		fi
		# finish this !
                FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffPng.o"
                AC_DEFINE(FIM_WITH_LIBPNG, 1, [Defined, if libpng support is enabled])
        fi
fi

#AM_CONDITIONAL(BUILD_PNG, test x$fim_handle_png = xyes)

dnl ================================================================================

fim_handle_xyz=no

AC_ARG_ENABLE(xyz, AC_HELP_STRING([--enable-xyz], [Enable the sample bogus 'xyz' file loading support. This makes only sense in order to learn how to include a new file format and wish to play with src/FbiStuffXyz.cpp in the while.]),
[       if test x$enableval = xyes ; then
		fim_handle_xyz=yes
        fi
])

if test x$fim_handle_xyz = xyes; then
	LIBXYZ="" # one would set an actual value here
	LIBS="$LIBS $LIBXYZ"
	FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffXyz.o"
	AC_DEFINE(FIM_WITH_LIBXYZ, 1, [Defined if sample bogus 'xyz' loader support is enabled.])
fi

dnl ================================================================================

## raw/nef file handling via UFRaw

fim_handle_ufraw=no

AC_ARG_ENABLE(ufraw, AC_HELP_STRING([--enable-ufraw], [Enable UFRaw support (EXPERIMENTAL).]),
[       if test x$enableval = xno ; then
		fim_handle_ufraw=no
	else
		fim_handle_ufraw=yes
        fi
])

if test x$fim_handle_ufraw = xyes; then
	# support for custom library configurator path
	CXXFLAGS="$CXXFLAGS $UFRAW_CXXFLAGS"
	LIBS="$LIBS $UFRAW_LIBS"
	FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffUFRaw.o" # FIXME
	AC_DEFINE(FIM_WITH_UFRAW, 1, [Defined, if UFRaw support is enabled])
	AC_MSG_NOTICE([Enabling UFRaw / libufraw.])
fi

dnl ================================================================================

## archive file handling via libarchive

fim_handle_archive=no

AC_ARG_ENABLE(archive, AC_HELP_STRING([--enable-archive], [Enable libarchive support (please provide ARCHIVE_LIBS, ARCHIVE_CXXFLAGS) (EXPERIMENTAL, UNFINISHED).]),
[       if test x$enableval = xno ; then
		fim_handle_archive=no
	else
		fim_handle_archive=yes
        fi
])

if test x$fim_handle_archive = xyes; then
	# support for custom library configurator path
	CXXFLAGS="$CXXFLAGS $ARCHIVE_CXXFLAGS"
	LIBS="$LIBS $ARCHIVE_LIBS -larchive"
	AC_DEFINE(FIM_WITH_ARCHIVE, 1, [Defined, if (EXPERIMENTAL, UNFINISHED) libarchive support is enabled])
	AC_MSG_NOTICE([Enabling libarchive.])
fi

dnl ================================================================================

AC_CHECK_LIB(exif, exif_data_new_from_data,have_libexif=yes,have_libexif=no)
AC_ARG_ENABLE(exif, AC_HELP_STRING([--disable-exif], [JPEG EXIF tags support with libexif.]), [if test x$enableval != xno; then fim_cv_libexif=yes; fi],[fim_cv_libexif=yes;])

if test x$fim_cv_libexif = xyes; then
	if test x$have_libexif = xyes; then
      			AC_MSG_NOTICE([Enabling libexif usage.])
			CXXFLAGS="$CXXFLAGS";
			LIBS="$LIBS -lexif";
	else
		        AC_MSG_ERROR([Sorry, it does not seem like libexif is installed or detectable.
			Please run --disable-exif or install libexif in a system-wide directory instead.
			])
	fi
else
	fim_cv_libexif=no;
fi

dnl ================================================================================

AC_ARG_ENABLE(jasper, AC_HELP_STRING([--enable-jasper], [Turns on JPEG-2000 files support using JasPer (EXPERIMENTAL)]),
	[if test x$enableval = xyes; then
		fim_cv_libjasper=yes;
		CXXFLAGS="$CXXFLAGS";
		LIBS="$LIBS -ljasper";
		AC_CHECK_LIB(jasper, jas_init,have_libjasper=yes,have_libjasper=no)
		if test x$have_libjasper = xyes; then
			AC_MSG_NOTICE([Enabling support of JPEG-2K files with JasPer.]);
  			FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS FbiStuffJasPer.o";
		else
			fim_cv_libjasper=no;
		        AC_MSG_ERROR([Sorry, it does not seem like libjasper is installed or detectable.
			Please run --disable-jasper or install libjasper in a system-wide directory instead.
			]);
		fi
	fi],[fim_cv_libjasper=no;])

dnl ================================================================================

AC_CHECK_LIB(aa, aa_getfirst,fim_cv_libaa=yes,fim_cv_libaa=no)
AC_ARG_ENABLE(aa, AC_HELP_STRING([--disable-aa], [Ascii Art (-o aa) support (via aalib)]), [if test x$enableval != xno; then fim_cv_libaa=yes; else fim_cv_libaa=no; fi],[fim_cv_libaa=yes;])

if test x$fim_cv_libaa != xno; then
	if test x$ac_cv_lib_aa_aa_getfirst = xyes; then
      		AC_MSG_NOTICE([Enabling aalib driver.])
		fim_cv_aalib=yes
	        if test x$libaa_config_prefix != x; then
			LIBAA_CONFIG="$libaa_config_prefix/bin/aalib-config"
		else
			# default path configurator
			AC_PATH_PROG(LIBAA_CONFIG, aalib-config, no)
		fi
		if test "x$LIBAA_CONFIG" = "xno"; then
			# uhm
	      		AC_MSG_WARN([***  no aalib-config ? ***])
			fim_cv_aalib=no
		else
			CXXFLAGS="$CXXFLAGS `$LIBAA_CONFIG --cflags`"
			LIBS="$LIBS `$LIBAA_CONFIG --libs`"
			# finish this !
		fi
	else
		fim_cv_aalib=no
	fi
else
	fim_cv_aalib=no
	AC_MSG_WARN([Ascii Art (-o aa) support disabled.])
        dnl AC_MSG_ERROR([Sorry, it does not seem like aalib is installed or detectable.
	dnl Please run --disable-aa or install aalib in a system-wide directory instead.
	dnl ])
fi

dnl ================================================================================

## sdl driver

AC_ARG_WITH(sdl-prefix, AC_HELP_STRING([--with-sdl-prefix=PFX], [prefix where libsdl is installed (optional)]),
libsdl_config_prefix="$withval", libsdl_config_prefix="")

fim_handle_sdl='auto'

dnl AC_ARG_ENABLE(sdl, AC_HELP_STRING([--disable-sdl], [Disable libsdl support.]),
dnl [       if test x$enableval = xno ; then
dnl 		fim_handle_sdl=no
dnl         fi
dnl ])

AC_ARG_ENABLE(sdl, AC_HELP_STRING([--disable-sdl], [Disable libsdl support.]),
[       if test x$enableval = xyes ; then fim_handle_sdl=yes ; fi ]
[       if test x$enableval = xno  ; then fim_handle_sdl=no  ; fi ])

# tested with sdl 1.2
if test x$fim_handle_sdl != xno; then
	# support for custom library configurator path
        if test x$libsdl_config_prefix != x; then
                LIBSDL_CONFIG="$libsdl_config_prefix/bin/sdl-config"
        else
                AC_PATH_PROG(LIBSDL_CONFIG, sdl-config, no)
        fi
        if test "x$LIBSDL_CONFIG" = "xno"; then
		if test x$fim_handle_sdl = xyes; then
                	AC_MSG_ERROR([LIBSDL not found (failed detecting the sdl-config program)])
		else
			fim_handle_sdl='no'
		fi
        else
		AC_MSG_NOTICE([Activating LIBSDL.])
		fim_handle_sdl='yes'
                LIBSDL_CXXFLAGS=`$LIBSDL_CONFIG --cflags`
                CXXFLAGS="$CXXFLAGS $LIBSDL_CXXFLAGS"
		LIBSDL_CONFIG_FLAGS=--static-libs
		if $LIBSDL_CONFIG $LIBSDL_CONFIG_FLAGS ; then 
			LIBSDL_CONFIG_FLAGS=--static-libs
		else
			LIBSDL_CONFIG_FLAGS=--libs
		fi
		AC_MSG_NOTICE([Invoking $LIBSDL_CONFIG $LIBSDL_CONFIG_FLAGS.])
                LIBS="$LIBS `$LIBSDL_CONFIG $LIBSDL_CONFIG_FLAGS`"
		# finish this !
                #FIM_LIB_OBJECTS="$FIM_LIB_OBJECTS SDLDevice.o"
                AC_DEFINE(FIM_WITH_LIBSDL, 1, [Defined, if libsdl support is enabled])
        fi
fi

dnl ================================================================================

AH_TEMPLATE([FIM_WANT_EXIFTOOL])
fim_cv_exiftool=no
AC_ARG_ENABLE(exiftool, AC_HELP_STRING([--enable-exiftool], [Turns on exiftool support (EXPERIMENTAL, shall provide the EXIFTOOLLINK variable with library or object files and EXIFTOOLCXXFLAGS). Not recommended: slow. Use --enable-exif instead.]),
	[if test x$enableval = xyes; then
		fim_cv_exiftool=yes
		CXXFLAGS="$CXXFLAGS $EXIFTOOLCXXFLAGS"
		LIBS="$LIBS $EXIFTOOLLINK"
      		AC_MSG_NOTICE([Enabling exiftool interface.])
		AC_DEFINE(FIM_WANT_EXIFTOOL, 1, [If defined, exiftool will be used to get additional file information (EXPERIMENTAL).])
	fi],[fim_cv_exiftool=no])
dnl

#AC_DEFINE([FIM_DEFAULT_KEY_CONFIG]   ,1, [Defined, if GPGME support is enabled])
#AC_DEFINE([FIM_DEFAULT_CONFIG], 1, [Defined, if GPGME support is enabled])

#AC_DEFINE([FIM_DEFAULT_CONFIGURATION], 1, [Defined, if GPGME support is enabled])

#AC_MSG_CHECKING(whether struct dirent defines d_ino)

dnl ==================

fim_cv_autocommands=yes

AC_ARG_ENABLE(autocommands, AC_HELP_STRING([--disable-autocommands], [Fim will by default use autocommands.]),
[if test x$enableval = xno; then
        fim_cv_autocommands=no
fi])

AH_TEMPLATE([FIM_AUTOCMDS])
if test x$fim_cv_autocommands = xyes; then
	have_autocommands=yes
	AC_DEFINE([FIM_AUTOCMDS])
else
	have_autocommands=no
fi


dnl ==================

AC_ARG_ENABLE(hardcoded-font, AC_HELP_STRING([--enable-hardcoded-font], [Enable a hardcoded font in the executable (good as a fallback (FBFONT=fim://) when no PSF format consolefonts files are available).]),
[if test x$enableval = xyes; then
        fim_cv_hardcoded_font=yes;
else
        fim_cv_hardcoded_font=no;
fi],[ fim_cv_hardcoded_font=yes; ])

AH_TEMPLATE([FIM_WANT_HARDCODED_FONT])
if test x$fim_cv_hardcoded_font = xyes; then
	have_hardcoded_font=yes
	AC_DEFINE([FIM_WANT_HARDCODED_FONT],1,[Enable a hardcoded font in the executable.])
else
	have_hardcoded_font=no
fi
AM_CONDITIONAL(FIM_WANT_HARDCODED_CONSOLEFONT, test x$have_hardcoded_font = xyes )


dnl ==================

AC_ARG_ENABLE(long-int-vars, AC_HELP_STRING([--enable-long-int-vars], [Internals shall use 64 bit integers (recommended).]),
[if test x$enableval = xyes; then
        fim_cv_want_int64_int=yes;
else
        fim_cv_want_int64_int=no;
fi],[ fim_cv_want_int64_int=yes; ])

AH_TEMPLATE([FIM_WANT_LONG_INT])
if test x$fim_cv_want_int64_int = xyes; then
	AC_DEFINE([FIM_WANT_LONG_INT],1,[Internals shall use 64 bit integers.])
else
	AC_DEFINE([FIM_WANT_LONG_INT],0,[Internals shall use 64 bit integers.])
fi
dnl ==================

fim_cv_windows=yes

AC_ARG_ENABLE(windows, AC_HELP_STRING([--disable-windows], [Disable windowing system in Fim. (deprecated)]),
[if test x$enableval = xno; then
        fim_cv_windows=no
fi])

AH_TEMPLATE([FIM_WINDOWS])
if test x$fim_cv_windows = xyes; then
	have_windows=yes
	AC_DEFINE([FIM_WINDOWS],1,[Fim windowing support])
else
	have_windows=no
fi


dnl ==================
fim_cv_fimrc=yes

AC_ARG_ENABLE(fimrc, AC_HELP_STRING([--disable-fimrc], [Fim will by default load the ~/.fimrc file.]),
[if test x$enableval = xno; then
        fim_cv_fimrc=no
fi])

AH_TEMPLATE([FIM_NOFIMRC])
if test x$fim_cv_fimrc = xyes; then
	have_fimrc=yes
else
	AC_DEFINE([FIM_NOFIMRC])
fi

dnl ==================
AH_TEMPLATE([FIM_NOHISTORY])
fim_cv_history=yes

AC_ARG_ENABLE(history, AC_HELP_STRING([--disable-history], [Fim will by default load/save a ~/.fim_history file.]),
[if test x$enableval = xno; then
        fim_cv_history=no
fi])

AH_TEMPLATE([FIM_NOHISTORY])
if test x$fim_cv_history = xyes; then
	have_history=yes
else
	AC_DEFINE([FIM_NOHISTORY])
fi

dnl ==================

fim_cv_warnings=no
AC_ARG_ENABLE(screen, AC_HELP_STRING([--disable-screen], [Fim will try to work under screen by default (please read BUGS)]),
[       if test x$enableval = xno; then
		fim_cv_screen=no
	else
                fim_cv_screen=yes
        fi
],[ fim_cv_screen=yes ]
)

dnl ==================

fim_cv_resize_optimizations=yes
AC_ARG_ENABLE(resize-optimizations, AC_HELP_STRING([--disable-resize-optimizations], [Some old (fbi) code is optimized for speedup in fim.]),
[       if test x$enableval = xno; then
		fim_cv_resize_optimizations=no
	else
                fim_cv_resize_optimizations=yes
        fi
],[ fim_cv_resize_optimizations=yes ]
)

dnl ==================

fim_cv_read_stdin_image=yes
AC_ARG_ENABLE(stdin-image-reading, AC_HELP_STRING([--disable-stdin-image-reading], [Fim will read an image from stdin with the -i switch. (experimental)]),
[       if test x$enableval = xno; then
		fim_cv_read_stdin_image=no
	else
                fim_cv_read_stdin_image=yes
        fi
],[ fim_cv_read_stdin_image=yes ]
)

dnl ==================
fim_cv_scan_consolefonts=yes
AC_ARG_ENABLE(scan-consolefonts, AC_HELP_STRING([--enable-scan-consolefonts], [Fim will scan some predefined directory in look for console fonts as a fallback. (experimental)]),
[       if test x$enableval = xyes; then
		fim_cv_scan_consolefonts=yes
	else
		fim_cv_scan_consolefonts=no
        fi
],[ fim_cv_scan_consolefonts=yes ]
)
dnl ==================
fim_cv_fb_console_switch_while_loading=no
AC_ARG_ENABLE(framebuffer-switch-while-loading, AC_HELP_STRING([--enable-framebuffer-switch-while-loading], [Fim will be capable of redrawing while switching framebuffer console (makes only sense on very slow systems).]),
[       if test x$enableval = xyes; then
		fim_cv_fb_console_switch_while_loading=yes
	else
                fim_cv_fb_console_switch_while_loading=no
        fi
],[ fim_cv_fb_console_switch_while_loading=no ]
)
dnl ==================
fim_cv_seek_magic=yes
AC_ARG_ENABLE(seek-magic, AC_HELP_STRING([--disable-seek-magic], [Fim will be capable to seek on a specified magic string.]),
[       if test x$enableval = xno; then
		fim_cv_seek_magic=no
	else
                fim_cv_seek_magic=yes
        fi
],[ fim_cv_seek_magic=yes ]
)
dnl ==================
fim_cv_paths_in_man=yes
AC_ARG_ENABLE(paths-in-man, AC_HELP_STRING([--disable-paths-in-man], [Generate man pages with configure specified paths (e.g. sysconfdir, docdir).]),
[       if test x$enableval = xno; then
		fim_cv_paths_in_man=no
	else
                fim_cv_paths_in_man=yes
        fi
],[ fim_cv_paths_in_man=yes ]
)
dnl ==================
AC_ARG_ENABLE(pushdir, AC_HELP_STRING([--enable-pushdir], [Fim will scan directories for files.]),
[       if test x$enableval = xno; then
		fim_cv_read_dirs=no
	else
                fim_cv_read_dirs=yes
        fi
],[ fim_cv_read_dirs=yes ]
)
dnl ==================
AC_ARG_ENABLE(recursive-pushdir, AC_HELP_STRING([--disable-recursive-pushdir], [Fim will be able to push directories recursively (implies --enable-push-dirs )]),
[       if test x$enableval = xyes; then
		fim_cv_recursive_dirs=yes;
                fim_cv_read_dirs=yes;
	else
                fim_cv_recursive_dirs=no;
        fi
],[ fim_cv_recursive_dirs=yes;fim_cv_read_dirs=yes; ]
)
dnl ==================

AH_TEMPLATE([FIM_RECURSIVE_DIRS])
AH_TEMPLATE([FIM_WANTS_SLOW_RESIZE])
AH_TEMPLATE([FIM_READ_DIRS])
AH_TEMPLATE([FIM_BOZ_PATCH])
AH_TEMPLATE([FIM_READ_STDIN])
AH_TEMPLATE([FIM_READ_STDIN_IMAGE])
AH_TEMPLATE([FIM_SMART_COMPLETION])
AH_TEMPLATE([FIM_LINUX_CONSOLEFONTS_DIR_SCAN])
dnl ==================

if test x$fim_cv_resize_optimizations != xyes; then
  AC_DEFINE([FIM_WANTS_SLOW_RESIZE])
fi

dnl ==================

if test x$fim_cv_read_dirs == xyes; then
  AC_DEFINE([FIM_READ_DIRS])
  if test x$fim_cv_recursive_dirs == xyes; then
	  AC_DEFINE([FIM_RECURSIVE_DIRS])
  fi
fi

dnl ==================

if test x$fim_cv_screen != xno; then
# boz patch is on
	AC_DEFINE([FIM_BOZ_PATCH])
fi

dnl ==================

fim_cv_read_stdin=yes

if test x$fim_cv_read_stdin != xno; then
	AC_DEFINE([FIM_READ_STDIN])
fi

dnl ==================

dnl NOTE THAT IN THE CODE FIM_READ_STDIN_IMAGE depends on FIM_READ_STDIN, too
if test x$fim_cv_read_stdin_image = xyes -a x$fim_cv_read_stdin = xyes; then
	AC_DEFINE([FIM_READ_STDIN_IMAGE])
else
	# we revert to no just for the printout
	fim_cv_read_stdin_image=no
fi

dnl ==================

fim_cv_warnings=no
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Turns on debug options (recommended for debugging)]),
[if test x$enableval = xyes; then
        fim_cv_debug=yes
fi])

dnl ==================

fim_cv_warnings=no
AC_ARG_ENABLE(warnings, AC_HELP_STRING([--enable-warnings], [Turns on compiler warnings (recommended, for debugging)]),
[if test x$enableval = xyes; then
        fim_cv_warnings=yes
fi])

if test x$GCC = xyes ; then
	if test x$fim_cv_debug = xyes; then
        # CXXFLAGS are resetted
        CXXFLAGS="-g"
	fi
fi

if test x$GCC = xyes ; then
	if test x$fim_cv_warnings = xyes ; then
        CXXFLAGS="-Wall -pedantic $CXXFLAGS"
	fi
fi

dnl ==================

dnl ==================

fim_cv_cacalib=no

AC_ARG_ENABLE(cacalib, AC_HELP_STRING([--enable-cacalib], [Turns on cacalib support (STILL INCOMPLETE)]),
	[if test x$enableval = xyes; then
	if test x$have_cacalib = xyes; then
	fim_cv_cacalib=yes
	else
    AC_MSG_ERROR([Sorry, it does not seem like cacalib is installed or detectable.
	Please run --disable-cacalib or install cacalib in a system-wide directory instead.
	])
	fi
	fi])

dnl ==================

# This code still awaits for integration ...
AC_ARG_WITH(cflags, AC_HELP_STRING([--with-cflags=CXXFLAGS], 
	[Specify a custom CXXFLAGS append value.  e.g.: -pg
	 You cannot call this argument multiple times.
	 In case you want completely reset CXXFLAGS use :
	 CXXFLAGS='<all that you want>' ./configure ..
	 Note that Fim is a C++ program, so we set CXXFLAGS=CXXFLAGS.
	]),
        [if test "x$withval" != x; then
                AC_DEFINE_UNQUOTED(CXXFLAGS, "$withval",
                 [custom CXXFLAGS])
		CXXFLAGS="$withval"
		CXXFLAGS="$CXXFLAGS"
		CXXFLAGS="$CXXFLAGS"
                AC_DEFINE_UNQUOTED(CXXFLAGS, "$withval",
                 [custom CXXFLAGS])
		dnl ...else CXXFLAGS="$CXXFLAGS"
        fi],
        [AC_DEFINE_UNQUOTED(EXECSHELL, "/bin/sh")])


dnl ==================

# This code still awaits for integration ...
AC_ARG_WITH(exec-shell, AC_HELP_STRING([--with-exec-shell=SHELL], [Specify alternate shell (ONLY if /bin/sh is broken)]),
        [if test x$withval != xyes; then
                AC_DEFINE_UNQUOTED(EXECSHELL, "$withval",
                 [program to use for shell commands])
         else
                AC_DEFINE_UNQUOTED(EXECSHELL, "/bin/sh")
        fi],
        [AC_DEFINE_UNQUOTED(EXECSHELL, "/bin/sh")])

dnl ==================
AC_ARG_ENABLE(convert, AC_HELP_STRING([--disable-smartautocompletion], [Fim will treat as a proper, unescaped string, nonquoted and nonevaluated strings which happen to match an existing filename.]),
[       if test x$enableval = xno; then
		fim_cv_smartautcompletion=no
	else
                fim_cv_smartautcompletion=yes
        fi
],[ fim_cv_smartautcompletion=yes ]
)
dnl ==================
if test x$fim_cv_smartautcompletion = xyes; then
	AC_DEFINE([FIM_SMART_COMPLETION])
fi
dnl ==================
if test x$fim_cv_scan_consolefonts = xyes; then
	AC_DEFINE([FIM_LINUX_CONSOLEFONTS_DIR_SCAN])
fi
dnl ==================

AC_ARG_ENABLE(convert, AC_HELP_STRING([--disable-convert], [Fim will try to use convert (ImageMagick) as a last resort for opening unknown format files.]),
[       if test x$enableval = xno; then
		fim_cv_convert=no
	else
                fim_cv_convert=yes
        fi
],[ fim_cv_convert=yes ]
)



dnl ==================

AC_ARG_ENABLE(xcftopnm, AC_HELP_STRING([--disable-xcftopnm], [Fim will try to use xcftopnm (Gimp) for opening XCF format files.]),
[       if test x$enableval = xno; then
		fim_cv_xcftopnm=no
	else
                fim_cv_xcftopnm=yes
        fi
],[ fim_cv_xcftopnm=yes ]
)

dnl ==================

fim_cv_inkscape=yes
AC_ARG_ENABLE(inkscape, AC_HELP_STRING([--disable-inkscape], [Fim will try to use Inkscape for opening SVG format files.]),
[       if test x$enableval = xno; then
		fim_cv_inkscape=no
	else
                fim_cv_inkscape=yes
        fi
],[ fim_cv_inkscape=yes ]
)

dnl ==================

AC_ARG_ENABLE(xfig, AC_HELP_STRING([--disable-xfig], [Fim will try to use xfig (fig2dev) for opening FIG format files.]),
[       if test x$enableval = xno; then
		fim_cv_xfig=no
	else
                fim_cv_xfig=yes
        fi
],[ fim_cv_xfig=yes ]
)

dnl ==================

AC_ARG_ENABLE(dia, AC_HELP_STRING([--disable-dia], [Fim will try to use dia for opening DIA format files.]),
[       if test x$enableval = xno; then
		fim_cv_dia=no
	else
                fim_cv_dia=yes
        fi
],[ fim_cv_dia=yes ]
)

dnl ==================
AH_TEMPLATE([FIM_DEFAULT_CONSOLEFONT])
AC_ARG_WITH(default-consolefont, AC_HELP_STRING([--with-default-consolefont=FILE], [Specify a default consolefont (psf or psf.gz format) file.]), [ AC_DEFINE_UNQUOTED(FIM_DEFAULT_CONSOLEFONT, "$withval") ], [fim_cv_defaultconsolefont=""])
dnl ==================
AC_ARG_WITH(custom-hardcoded-consolefont, AC_HELP_STRING([--with-custom-hardcoded-consolefont=FILE], [Specify a custom consolefont to be hardcoded (uncompressed psf format, absolute path) at build time (requires --enable-hardcoded-font).]), [
fim_cv_want_custom_hardcoded_consolefont=true;
AC_SUBST(FIM_CUSTOM_HARDCODED_CONSOLEFONT, "$withval")
], [])
AM_CONDITIONAL(FIM_WANT_CUSTOM_HARDCODED_CONSOLEFONT, test x$fim_cv_want_custom_hardcoded_consolefont = xtrue )
dnl ==================
fim_cv_want_font_magnify_factor=0;
AC_ARG_WITH(font-magnifying-factor, AC_HELP_STRING([--with-font-magnifying-factor=FACTOR], [If FACTOR>0, Will render text magnified by (hardcoded) FACTOR times. If FACTOR<=0, will allow run-time control of it via the _fbfont_magnify_factor variable. If negative, -FACTOR will be used as default. Default is 0 ]), [
fim_cv_want_font_magnify_factor="$withval";
], [])
AM_CONDITIONAL(FIM_WANT_CUSTOM_HARDCODED_CONSOLEFONT, test x$fim_cv_want_custom_hardcoded_consolefont = xtrue )
dnl ==================
dnl # /tmp/__FIM_TEMPORARY_FILE
AC_ARG_WITH(tmpfile, AC_HELP_STRING([--with-tmpfile=FILE], [Specify where to put temporary files created when interacting with external conversion programs.]),
        [fim_cv_tmpfile=$withval],
        [fim_cv_tmpfile=""])
dnl ==================
AC_ARG_ENABLE(custom-status-bar, AC_HELP_STRING([--disable-custom-status-bar], [Customization of status bar via a printf-like string.]),
[       if test x$enableval = xno; then
		fim_cv_custom_status_bar=no;
	else
                fim_cv_custom_status_bar=yes;
        fi
],[ fim_cv_custom_status_bar=yes; ]
)
dnl ==================

AC_ARG_ENABLE(optimizations, AC_HELP_STRING([--enable-optimizations], [Will try to enhance compilation flags (EXPERIMENTAL)]),
[       if test x$enableval = xno; then
		fim_cv_optimizations=no
	else
                fim_cv_optimizations=yes
        fi
],[ fim_cv_optimizations=no ]
)

dnl ==================
AH_TEMPLATE([FIM_USE_CXX_STD])
dnl ==================
AH_TEMPLATE([FIM_TRY_XCFTOPNM])
AH_TEMPLATE([FIM_TRY_CONVERT])
AH_TEMPLATE([FIM_TRY_INKSCAPE])
AH_TEMPLATE([FIM_TRY_XFIG])
AH_TEMPLATE([FIM_TRY_DIA])
AH_TEMPLATE([FIM_USE_ZCAT])

dnl ==================

AC_CHECK_PROG(have_zcat,zcat,yes,no)
        if test x$have_zcat = xyes ; then
		# legacy fbi feature : should not rely on this. FIXME
		AC_MSG_NOTICE([Fim will use 'zcat' to uncompress "gzipped" font files.])
		AC_DEFINE([FIM_USE_ZCAT])
	fi

dnl 20110602 will use man --html-cat instead of obsolete man2html 
dnl AC_CHECK_PROG(have_man2html,man2html,yes,no)
dnl        if test x$have_man2html = xyes ; then
dnl      		AC_MSG_NOTICE([The installer will try to invoke 'man2html' to build html versions of the man pages.])
dnl		HTMLDOCS="FIM.html fim.man.html fimgs.man.html"
dnl	fi

AC_CHECK_PROG(have_convert,convert,yes,no)
        if test x$have_convert = xyes -a x$fim_cv_convert = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'convert' to handle unknown format files, even if no 'convert' is currently in the program path: please install the Imagemagick package to obviate the problem or re-configure with --disable-convert.])
	else
		fim_cv_convert=no
	fi

AC_CHECK_PROG(have_xcftopnm,xcftopnm,yes,no)
        if test x$have_xcftopnm = xyes -a x$fim_cv_xcftopnm = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'xcftopnm' to handle XCF format files, even if no 'xcftopnm' is currently in the program path: please install the Gimp package to obviate the problem or re-configure with --disable-xcftopnm.])
	else
		fim_cv_xcftopnm=no
	fi

AC_CHECK_PROG(have_inkscape,inkscape,yes,no)
        if test x$have_inkscape = xyes -a x$fim_cv_inkscape = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'inkscape' to handle SVG format files, even if no 'inkscape' is currently in the program path: please install the Inkscape package to obviate the problem or re-configure with --disable-inkscape.])
	else
		fim_cv_inkscape=no
	fi

AC_CHECK_PROG(have_xfig,fig2dev,yes,no)
        if test x$have_xfig = xyes -a x$fim_cv_xfig = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'xfig' to handle FIG format files, even if no 'fig2dev' is currently in the program path: please install the Xfig package to obviate the problem or re-configure with --disable-xfig.])
	else
		fim_cv_xfig=no
	fi

AC_CHECK_PROG(have_dia,dia,yes,no)
        if test x$have_dia = xyes -a x$fim_cv_dia = xyes ; then
		true
dnl      		AC_MSG_WARN([Fim will try to invoke 'dia' to handle DIA format files, even if no 'dia' is currently in the program path: please install the Dia package to obviate the problem or re-configure with --disable-dia.])
	else
		fim_cv_dia=no
	fi

dnl ==================

        if test x$fim_cv_optimizations = xyes ; then
		uname=`uname`
        	if test x$uname = xLinux ; then
dnl			AC_MSG_NOTICE([we run on linux! Good.])
	     		if grep "^model name.*Pentium(R) 4" /proc/cpuinfo > /dev/null; then
				CXXFLAGS="$CXXFLAGS -march=native"
		     	fi

		else
			AC_MSG_NOTICE([uhm. try to specify your own optimizations..])

		fi
	fi

dnl ==================
        if test x$fim_cxx_standard = x98 ; then
		AC_DEFINE([FIM_USE_CXX_STD], 1998, [])
	fi
        if test x$fim_cxx_standard = x11 ; then
		AC_DEFINE([FIM_USE_CXX_STD], 2011, [])
	fi
        if test x$fim_cxx_standard = x14 ; then
		AC_DEFINE([FIM_USE_CXX_STD], 2014, [])
	fi
        if test x$fim_cxx_standard = x17 ; then
		AC_DEFINE([FIM_USE_CXX_STD], 2017, [])
	fi
dnl ==================

        if test x$fim_cv_convert = xyes ; then
		AC_DEFINE([FIM_TRY_CONVERT])
	fi

        if test x$fim_cv_xcftopnm = xyes ; then
		AC_DEFINE([FIM_TRY_XCFTOPNM])
	fi

        if test x$fim_cv_inkscape = xyes ; then
		AC_DEFINE([FIM_TRY_INKSCAPE])
	fi

        if test x$have_xfig = xyes ; then
		AC_DEFINE([FIM_TRY_XFIG])
	fi

        if test x$have_dia = xyes ; then
		AC_DEFINE([FIM_TRY_DIA])
	fi

dnl ==================

    AH_TEMPLATE([FIM_WITH_LIBJASPER])
            if test x$fim_cv_libjasper = xyes ; then
    		AC_DEFINE([FIM_WITH_LIBJASPER])
    	fi
dnl ==================

    AH_TEMPLATE([FIM_WITH_LIBEXIF])
            if test x$fim_cv_libexif = xyes ; then
    		AC_DEFINE([FIM_WITH_LIBEXIF])
    	fi
dnl ==================

    AH_TEMPLATE([FIM_WITH_AALIB])
            if test x$fim_cv_aalib = xyes ; then
    		AC_DEFINE([FIM_WITH_AALIB])
    	fi

dnl ==================

    AH_TEMPLATE([FIM_WITH_CACALIB])
            if test x$fim_cv_cacalib = xyes ; then
    		AC_DEFINE([FIM_WITH_CACALIB])
    		LIBS="$LIBS -lcaca"
    	fi

dnl ==================
    AH_TEMPLATE([FIM_WANT_FB_CONSOLE_SWITCH_WHILE_LOADING])
            if test x$fim_cv_fb_console_switch_while_loading = xyes ; then
    		AC_DEFINE([FIM_WANT_FB_CONSOLE_SWITCH_WHILE_LOADING])
    	fi

dnl ==================
    AH_TEMPLATE([FIM_WANT_SEEK_MAGIC])
            if test x$fim_cv_seek_magic = xyes ; then
    		AC_DEFINE([FIM_WANT_SEEK_MAGIC])
    	fi

dnl ==================
    AH_TEMPLATE([FIM_PATHS_IN_MAN])
            if test x$fim_cv_paths_in_man = xyes ; then
    		AC_DEFINE([FIM_PATHS_IN_MAN])
    	fi

dnl ==================
    AH_TEMPLATE([FIM_WANT_FONT_MAGNIFY_FACTOR])
    AC_DEFINE_UNQUOTED([FIM_WANT_FONT_MAGNIFY_FACTOR],$fim_cv_want_font_magnify_factor)

dnl ==================
    AH_TEMPLATE([FIM_WANT_CUSTOM_INFO_STATUS_BAR])
            if test x$fim_cv_custom_status_bar = xyes ; then
    		AC_DEFINE([FIM_WANT_CUSTOM_INFO_STATUS_BAR])
    	fi

dnl ==================
AH_TEMPLATE([FIM_WANT_FILENAME_MARK_AND_DUMP])
AC_ARG_ENABLE(mark-and-dump, AC_HELP_STRING([--enable-mark-and-dump], [Fim will be capable of marking files while viewing and outputting their names on exit. (enabled by default)]), [       if test x$enableval = xyes; then fim_cv_want_mark_and_dump=1 ; else fim_cv_want_mark_and_dump=0 ; fi ],[ fim_cv_want_mark_and_dump=1 ])
AC_DEFINE_UNQUOTED([FIM_WANT_FILENAME_MARK_AND_DUMP], $fim_cv_want_mark_and_dump, [Fim will be capable of marking files while viewing and outputting their names on exit.])
dnl ==================
AH_TEMPLATE([FIM_WANT_NOSCRIPTING])
AC_ARG_ENABLE(scripting, AC_HELP_STRING([--enable-scripting], [Scripting. (enabled by default)]), [       if test x$enableval = xyes; then fim_cv_want_noscripting=0 ; else fim_cv_want_noscripting=1 ; fi ],[ fim_cv_want_noscripting=0 ])
if test x$fim_cv_want_noscripting == x1 ; then 
AC_DEFINE_UNQUOTED([FIM_WANT_NOSCRIPTING], $fim_cv_want_noscripting, [Scripting. (enabled by default)])
fi
dnl ==================
AH_TEMPLATE([FIM_WANT_NO_OUTPUT_CONSOLE])
AC_ARG_ENABLE(output-console, AC_HELP_STRING([--enable-output-console], [Output console. (enabled by default)]), [       if test x$enableval = xyes; then fim_cv_want_no_output_console=0 ; else fim_cv_want_no_output_console=1 ; fi ],[ fim_cv_want_no_output_console=0 ])
if test x$fim_cv_want_no_output_console == x1 ; then 
AC_DEFINE_UNQUOTED([FIM_WANT_NO_OUTPUT_CONSOLE], $fim_cv_want_no_output_console, [Output console. (enabled by default)])
fi
dnl ==================
AH_TEMPLATE([FIM_WANT_UNICODE])
AC_ARG_ENABLE(unicode, AC_HELP_STRING([--enable-unicode], [Unicode. (Unimplemented, disabled by default)]), [       if test x$enableval = xyes; then fim_cv_want_unicode=1 ; else fim_cv_want_unicode=0 ; fi ],[ fim_cv_want_unicode=0 ])
if test x$fim_cv_want_unicode == x1 ; then 
	# CXXFLAGS="$CXXFLAGS `freetype-config --cflags`"
	# LIBS="$LIBS `freetype-config --libs`"
	AC_DEFINE_UNQUOTED([FIM_WANT_UNICODE], $fim_cv_want_unicode, [Check for freetype library, with no feature behind: do not use this. (disabled by default)])
fi
dnl ==================
AH_TEMPLATE([FIM_WANT_TEXT_RENDERING])
fim_want_astext=yes
AC_ARG_ENABLE(as-text-rendering, AC_HELP_STRING([--disable-as-text-rendering], [Disable as-text rendering (viewing of any file as character bytes -- not to be confused with the ASCII art output).]),
[       if test x$enableval = xno ; then
		fim_want_astext=no
        fi
])
if test x$fim_want_astext = xyes ; then
	AC_DEFINE_UNQUOTED([FIM_WANT_TEXT_RENDERING], 1, [Enables text rendering])
fi
dnl ==================
AH_TEMPLATE([FIM_WANT_RAW_BITS_RENDERING])
fim_want_rbr=yes
AC_ARG_ENABLE(raw-bits-rendering, AC_HELP_STRING([--disable-raw-bits-rendering], [Disable raw bits rendering (viewing of any file as either a 1 or 24 bpp bitmap).]),
[       if test x$enableval = xno ; then
		fim_want_rbr=no
        fi
])
if test x$fim_want_rbr = xyes ; then
	AC_DEFINE_UNQUOTED([FIM_WANT_RAW_BITS_RENDERING], 1, [Enables raw bits rendering])
fi
dnl ==================
AH_TEMPLATE([FIM_ALLOW_LOADER_STRING_SPECIFICATION])
fim_want_lss=yes
AC_ARG_ENABLE(loader-string-specification, AC_HELP_STRING([--disable-loader-string-specification], [Disables the chance to specify a file loader.]),
[       if test x$enableval = xno ; then
		fim_want_lss=no
        fi
])
if test x$fim_want_lss = xyes ; then
	AC_DEFINE_UNQUOTED([FIM_ALLOW_LOADER_STRING_SPECIFICATION], 1, [Allows the user to specify a file loader])
fi
dnl ==================

dnl uhmmmmmm : does not seem to work
dnl AC_ARG_ENABLE(verbose_make,[Enables verbose macros passing to the compiler (for a funnier compilation process.)], 
dnl         if test x$enableval = xyes; then
dnl		AC_SUBST(DEFS)
dnl	else
dnl		AC_CONFIG_HEADERS([config.h])
dnl        fi,[])
AC_CONFIG_HEADERS([config.h])


AH_TEMPLATE([FIM_RANDOM])
AC_DEFINE([FIM_RANDOM])
AH_TEMPLATE([FIM_DEFAULT_CONFIG])
AC_DEFINE([FIM_DEFAULT_CONFIG])
AH_TEMPLATE([FIM_NOFB])
AC_DEFINE([FIM_NOFB])
AH_TEMPLATE([FIM_RECORDING])
AC_DEFINE([FIM_RECORDING])
AH_TEMPLATE([FIM_SWITCH_FIXUP])
AC_DEFINE([FIM_SWITCH_FIXUP])
AH_TEMPLATE([FIM_CHECK_DUPLICATES])
AC_DEFINE([FIM_CHECK_DUPLICATES])
AH_TEMPLATE([FIM_CHECK_FILE_EXISTENCE])
AC_DEFINE([FIM_CHECK_FILE_EXISTENCE])
AH_TEMPLATE([FIM_REMOVE_FAILED])
AC_DEFINE([FIM_REMOVE_FAILED])
AH_TEMPLATE([FIM_AUTOSKIP_FAILED])
AC_DEFINE([FIM_AUTOSKIP_FAILED])
AH_TEMPLATE([FIM_COMMAND_AUTOCOMPLETION])
AC_DEFINE([FIM_COMMAND_AUTOCOMPLETION])
AH_TEMPLATE([FIM_NAMESPACES])
AC_DEFINE([FIM_NAMESPACES])
AH_TEMPLATE([FIM_CACHE_DEBUG])
dnl AC_DEFINE([FIM_CACHE_DEBUG])
AH_TEMPLATE([FIM_VERSION])
AC_DEFINE_UNQUOTED([FIM_VERSION],"${VERSION}")
dnl AH_TEMPLATE([FIM_USE_X11_FONTS])
dnl AC_DEFINE([FIM_USE_X11_FONTS])
AH_TEMPLATE([FIM_TMP_FILENAME])
AC_DEFINE_UNQUOTED([FIM_TMP_FILENAME],"$fim_cv_tmpfile")
AH_TEMPLATE([FIM_ITERATED_COMMANDS])
AC_DEFINE([FIM_ITERATED_COMMANDS])
AH_TEMPLATE([FIM_SKIP_KNOWN_FILETYPES])
AC_DEFINE([FIM_SKIP_KNOWN_FILETYPES])
AH_TEMPLATE([FIM_EXPERIMENTAL_ROTATION])
AC_DEFINE([FIM_EXPERIMENTAL_ROTATION])
AH_TEMPLATE([FIM_KEEP_BROKEN_CONSOLE])
dnl AC_DEFINE([FIM_KEEP_BROKEN_CONSOLE])
AH_TEMPLATE([FIM_PIPE_IMAGE_READ])
AC_DEFINE([FIM_PIPE_IMAGE_READ])
AH_TEMPLATE([FIM_USE_DESIGNATED_INITIALIZERS])
dnl AC_DEFINE([FIM_USE_DESIGNATED_INITIALIZERS])
dnl AH_TEMPLATE([FIM_IS_SLOWER_THAN_FBI])
dnl AC_DEFINE([FIM_IS_SLOWER_THAN_FBI])
dnl AH_TEMPLATE([FIM_WITH_LIBSDL])
dnl AC_DEFINE([FIM_WITH_LIBSDL])
AH_TEMPLATE([FBI_AUTHOR])
AH_TEMPLATE([FBI_AUTHOR_NAME])
AH_TEMPLATE([FIM_AUTHOR])
AH_TEMPLATE([FIM_AUTHOR_NAME])
AH_TEMPLATE([FIM_AUTHOR_EMAIL])
AC_DEFINE([FIM_AUTHOR_NAME],["Michele Martone"])
AC_DEFINE([FIM_AUTHOR_EMAIL],["<dezperado _CUT_ autistici _CUT_ org>"])
AC_DEFINE([FIM_AUTHOR],["Michele Martone <dezperado _CUT_ autistici _CUT_ org>"])
# Note: Gerd changed his surname from Knorr to Hoffmann, so I report here his actual surname
AC_DEFINE([FBI_AUTHOR_NAME],["Gerd Hoffmann"])
AC_DEFINE([FBI_AUTHOR],["Gerd Hoffmann <kraxel _CUT_ bytesex.org>"])
dnl ###############################################################################
dnl this is experimental stuff
dnl ###############################################################################
AC_ARG_ENABLE(matrices-rendering, AC_HELP_STRING([--enable-matrices-rendering], [Uses librsb for rendering of Matrix Market files (must have librsb-config in the PATH).]),
[       if test x$enableval = xyes; then
        	if test x$ac_cv_func_rsb_lib_init = xyes; then
AH_TEMPLATE([HAVE_MATRIX_MARKET_DECODER])
AC_DEFINE([HAVE_MATRIX_MARKET_DECODER])
#CXXFLAGS="$CXXFLAGS `librsb-config  --cflags`"
#LIBS="$LIBS `librsb-config --static --ldflags | sed s/-static//g`"
		else
			AC_MSG_ERROR([Cannot detect librsb! Either disable matrix rendering or supply the right LIBS and CXXFLAGS!])
        	fi
	else
		true;
        fi
])
dnl ###############################################################################


dnl ==================

AC_SUBST(LIBS)
AC_SUBST(FIM_DEFS)

AC_SUBST(FIM_LIB_OBJECTS)
AC_SUBST(FIM_SVN_REPOSITORY,[http://svn.savannah.nongnu.org/svn/fbi-improved/])

dnl we want these symbols defined in our code
AC_SUBST(CXXFLAGS)

#AC_CONFIG_FILES([Makefile],[src/Makefile])

AC_OUTPUT(Makefile src/Makefile doc/Makefile distros/Makefile scripts/Makefile
	 scripts/maintenance/cron-build.sh
	 scripts/maintenance/remote-build.sh
	 scripts/maintenance/cron-svndump.pl
	 scripts/maintenance/cron-rsync.sh
	)
#AC_OUTPUT

AC_MSG_NOTICE([
	Fim v${VERSION} has been configured as follows (features marked with * are not recommended: either unfinished or broken: please do NOT use them when building for a distro):

   	default minimal key mapping         : ${have_default_mapping:-no}
   	default minimal key bindings        : ${have_default_bindings:-no}
	default configuration file builtin  : ${have_default_config:-no}
dnl     compile time warnings               : ${fim_cv_warnings:-no} (additional CXXFLAGS: ${CXXFLAGS} )
        resize optimizations                : ${fim_cv_resize_optimizations:-no}
        system,popen calls wrappers enabled : ${fim_system_calls:-yes}
   	will allow push of directories      : ${fim_cv_read_dirs}
   	will recurse directories (-R switch): ${fim_cv_recursive_dirs}
dnl	dvi  file support (dvilib29)        : ${fim_handle_dvi:-no}
	patch for running under screen      : ${fim_cv_screen:-no}
	~/.fimrc startup file support       : ${have_fimrc:-no}
	~/.fim_history load/save support    : ${have_history:-no}
	autocommands enabled                : ${have_autocommands:-no}
dnl	internal windowing system           : ${have_windows:-no}
	hardcoded PSF font (FBFONT=fim://)  : ${have_hardcoded_font:-no}
	will read image files from stdin(-i): ${fim_cv_read_stdin_image:-no}
	GNU regexps (default is Posix)      :*${fim_cv_regex:-no}
        seek magic signatures               : ${fim_cv_seek_magic:-no}
	man page with user configured paths : ${fim_cv_paths_in_man}
        framebuffer console load switchable :*${fim_cv_fb_console_switch_while_loading:-no}
        hardcoded font magnifying factor    : ${fim_cv_want_font_magnify_factor}
	customizable status bar             : ${fim_cv_custom_status_bar:-no}
	use C++ standard (98/11/14/17)      : ${fim_cxx_standard}
dnl
	readline (-lrl) library support     : ${fim_want_readline}
	use 'xcftopnm' for XCF files        :*${fim_cv_xcftopnm}
	use 'inkscape' for SVG files        : ${fim_cv_inkscape}
	use 'fig2dev' for FIG files         : ${fim_cv_xfig}
	use 'convert' for unrecognized files:*${fim_cv_convert}
	use 'exiftool' for EXIF tags in JPEG:*${fim_cv_exiftool}
dnl
        framebuffer (built-in) support      : ${fim_handle_framebuffer:-yes}
	X        (sdllib) support (-o sdl)  : ${fim_handle_sdl}
	ASCII art (aalib) support (-o aa)   : ${fim_cv_aalib}
	X        (Imlib2) support(-o imlib2):*${fim_handle_imlib2:-no}
	coloured ASCII art (cacalib) support:*${fim_cv_cacalib}
dnl
   	EXIF (JPEG) tags support (libexif)  : ${fim_cv_libexif}
	BMP      file support (builtin)     : ${fim_handle_bmp:-no}
	JP2 (JPEG-2K) file support (JasPer) : ${fim_cv_libjasper}
	PCX      file support (builtin)     : ${fim_handle_pcx:-no}
	PNG      file support (libpng)      : ${fim_handle_png:-no}
	JPEG     file support (libjpeg)     : ${fim_handle_jpeg:-no}
	TIFF     file support (libtiff)     : ${fim_handle_tiff:-no}
	GIF      file support (libgif)      : ${fim_handle_gif:-no}
	DJVU     file support (libdjvu)     :*${fim_handle_djvu:-no}
	PDF      file support (libpoppler)  :*${fim_handle_pdf:-no}
	PS       file support (libspectre)  :*${fim_handle_ps:-no}
	NEF/RAW  file support (libufraw)    :*${fim_handle_ufraw:-no}
	TAR/ZIP/.. file support (libarchive):*${fim_handle_archive:-no}
	GraphicsMagick support (libGraphicsMagick):*${fim_handle_graphicsmagick:-no}

        YACC     : ${YACC}
        LEX      : ${LEX}
        CXX      : ${CXX}
        CXXFLAGS : ${CXXFLAGS}
        LIBS     : ${LIBS}
        prefix   : ${prefix}
dnl        LDFLAGS  : ${LDFLAGS}
dnl Still out:
dnl LEXLIB
dnl LIBAA_CONFIG 
dnl LIBPNG_CONFIG 
dnl LIBSDL_CONFIG 
dnl jasper ?
dnl archive ?

	If you are not satisfied with this configuration, please invoke
	  ./configure --help=short
	for a complete list of configure options and variables specific to Fim.
	When you are satisfied, run 'make' to build Fim and its documentation.
	To install the program and its documentation, run 'make install' if you are root,
	or run 'su -c '"make install"' if you are not root.
	RTFM: README, man fim, man fimrc.  Enjoy!
])
