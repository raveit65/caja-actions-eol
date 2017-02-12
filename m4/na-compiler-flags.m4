# Caja-Actions
# A Caja extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The GNOME Foundation
# Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
# Copyright (C) 2012-2017 Wolfgang Ulbrich and others (see AUTHORS)
#
# Caja-Actions is free software; you can redistribute it and/or
# modify it under the terms of the GNU General  Public  License  as
# published by the Free Software Foundation; either  version  2  of
# the License, or (at your option) any later version.
#
# Caja-Actions is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even  the  implied  warranty  of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  the  GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public  License
# along with Caja-Actions; see the file  COPYING.  If  not,  see
# <http://www.gnu.org/licenses/>.
#
# Authors:
#   Frederic Ruaudel <grumz@grumz.net>
#   Rodrigo Moya <rodrigo@mate-db.org>
#   Pierre Wieser <pwieser@trychlos.org>
#   ... and many others (see AUTHORS)

dnl MATE_COMPILE_WARNINGS
dnl Turn on many useful compiler warnings
dnl For now, only works on GCC

# serial 3 Fix to correctly handle provided default values

#
# pwi 2009-05-15 shamelessly copied from mate-compiler-flags.m4
# two patch have been submitted (see mate-common #582860)
#

AC_DEFUN([NA_MATE_COMPILE_WARNINGS],[

	_ac_warning_def=m4_default([$1],[yes])
	_ac_compliant_def=m4_default(["$2"],[no])

    dnl ******************************
    dnl More compiler warnings
    dnl ******************************

    AC_ARG_ENABLE(compile-warnings,
                  AC_HELP_STRING([--enable-compile-warnings=@<:@no/minimum/yes/maximum/error@:>@],
                                 [Turn on compiler warnings]),,
                  [enable_compile_warnings="${_ac_warning_def}"])

    warnCFLAGS=
    if test "x$GCC" != xyes; then
	enable_compile_warnings=no
    fi

    warning_flags=
    realsave_CFLAGS="$CFLAGS"

    case "$enable_compile_warnings" in
    no)
	warning_flags=
	;;
    minimum)
	warning_flags="-Wall"
	;;
    yes)
	warning_flags="-Wall -Wmissing-prototypes"
	;;
    maximum|error)
	warning_flags="-Wall -Wmissing-prototypes -Wnested-externs -Wpointer-arith"
	CFLAGS="$warning_flags $CFLAGS"
	for option in -Wno-sign-compare; do
		SAVE_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $option"
		AC_MSG_CHECKING([whether gcc understands $option])
		AC_TRY_COMPILE([], [],
			has_option=yes,
			has_option=no,)
		CFLAGS="$SAVE_CFLAGS"
		AC_MSG_RESULT($has_option)
		if test $has_option = yes; then
		  warning_flags="$warning_flags $option"
		fi
		unset has_option
		unset SAVE_CFLAGS
	done
	unset option
	if test "$enable_compile_warnings" = "error" ; then
	    warning_flags="$warning_flags -Werror"
	fi
	;;
    *)
	AC_MSG_ERROR(Unknown argument '$enable_compile_warnings' to --enable-compile-warnings)
	;;
    esac
    CFLAGS="$realsave_CFLAGS"
    AC_MSG_CHECKING(what warning flags to pass to the C compiler)
    AC_MSG_RESULT($warning_flags)

    AC_ARG_ENABLE(iso-c,
                  AC_HELP_STRING([--enable-iso-c],
                                 [Try to warn if code is not ISO C ]),,
                  [enable_iso_c="${_ac_compliant_def}"])

    AC_MSG_CHECKING(what language compliance flags to pass to the C compiler)
    complCFLAGS=
    if test "x$enable_iso_c" != "xno"; then
		if test "x$GCC" = "xyes"; then
			if test "x${_ac_compliant_def}" = "x"; then
				case " $CFLAGS " in
					*[\ \	]-ansi[\ \	]*) ;;
				    *) complCFLAGS="$complCFLAGS -ansi" ;;
				esac
				case " $CFLAGS " in
				    *[\ \	]-pedantic[\ \	]*) ;;
				    *) complCFLAGS="$complCFLAGS -pedantic" ;;
				esac
			else
				complCFLAGS="$complCFLAGS ${_ac_compliant_def}"
			fi
		fi
    fi
    AC_MSG_RESULT($complCFLAGS)

    WARN_CFLAGS="$warning_flags $complCFLAGS"
    AC_SUBST(WARN_CFLAGS)
])

dnl For C++, do basically the same thing.

AC_DEFUN([MATE_CXX_WARNINGS],[
  AC_ARG_ENABLE(cxx-warnings,
                AC_HELP_STRING([--enable-cxx-warnings=@<:@no/minimum/yes@:>@]
                               [Turn on compiler warnings.]),,
                [enable_cxx_warnings="m4_default([$1],[minimum])"])

  AC_MSG_CHECKING(what warning flags to pass to the C++ compiler)
  warnCXXFLAGS=
  if test "x$GXX" != xyes; then
    enable_cxx_warnings=no
  fi
  if test "x$enable_cxx_warnings" != "xno"; then
    if test "x$GXX" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCXXFLAGS="-Wall -Wno-unused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_cxx_warnings" = "xyes"; then
	warnCXXFLAGS="$warnCXXFLAGS -Wshadow -Woverloaded-virtual"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCXXFLAGS)

   AC_ARG_ENABLE(iso-cxx,
                 AC_HELP_STRING([--enable-iso-cxx],
                                [Try to warn if code is not ISO C++ ]),,
                 [enable_iso_cxx=no])

   AC_MSG_CHECKING(what language compliance flags to pass to the C++ compiler)
   complCXXFLAGS=
   if test "x$enable_iso_cxx" != "xno"; then
     if test "x$GXX" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -ansi" ;;
      esac

      case " $CXXFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -pedantic" ;;
      esac
     fi
   fi
  AC_MSG_RESULT($complCXXFLAGS)

  WARN_CXXFLAGS="$CXXFLAGS $warnCXXFLAGS $complCXXFLAGS"
  AC_SUBST(WARN_CXXFLAGS)
])

dnl Bug #637797 
dnl see also http://www.gentoo.org/proj/en/qa/asneeded.xml
dnl the '-Wl,--as-needed' link option is forced in make distcheck

AC_DEFUN([NA_LINK_AS_NEEDED],[
	AC_ARG_ENABLE(
		[as-needed],
		AC_HELP_STRING(
			[--enable-as-needed],
			[Enable '-Wl,--as-needed' link option @<:@no@:>@]
		),
		[enable_as_needed=$enableval],
		[enable_as_needed=no])

	AC_MSG_CHECKING([whether to only link with needed libraries])
	AC_MSG_RESULT([$enable_as_needed])

	if test "x${enable_as_needed}" = "xyes"; then
		AC_SUBST([AM_LDFLAGS],["${AM_LDFLAGS} -Wl,--as-needed"])
	fi
])
