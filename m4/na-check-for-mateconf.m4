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

# serial 1 creation

dnl pwi 2011-02-14
dnl this is a copy of the original AM_MATECONF_SOURCE_2 which is just a bit hacked
dnl in order to define the conditionals event when we want disabled MateConf
dnl syntax: NA_MATECONF_SOURCE_2([have_mateconf])
dnl
dnl AM_MATECONF_SOURCE_2
dnl Defines MATECONF_SCHEMA_CONFIG_SOURCE which is where you should install schemas
dnl  (i.e. pass to mateconftool-2)
dnl Defines MATECONF_SCHEMA_FILE_DIR which is a filesystem directory where
dnl  you should install foo.schemas files

AC_DEFUN([NA_MATECONF_SOURCE_2],
[
	if test "$1" = "yes"; then
		if test "x$MATECONF_SCHEMA_INSTALL_SOURCE" = "x"; then
			MATECONF_SCHEMA_CONFIG_SOURCE=`mateconftool-2 --get-default-source`
		else
			MATECONF_SCHEMA_CONFIG_SOURCE=$MATECONF_SCHEMA_INSTALL_SOURCE
		fi
	fi

  AC_ARG_WITH([mateconf-source],
              AC_HELP_STRING([--with-mateconf-source=sourceaddress],
                             [Config database for installing schema files.]),
              [MATECONF_SCHEMA_CONFIG_SOURCE="$withval"],)

  AC_SUBST(MATECONF_SCHEMA_CONFIG_SOURCE)

	if test "$1" = "yes"; then
		AC_MSG_RESULT([Using config source $MATECONF_SCHEMA_CONFIG_SOURCE for schema installation])
	fi

  if test "x$MATECONF_SCHEMA_FILE_DIR" = "x"; then
    MATECONF_SCHEMA_FILE_DIR='$(sysconfdir)/mateconf/schemas'
  fi

  AC_ARG_WITH([mateconf-schema-file-dir],
              AC_HELP_STRING([--with-mateconf-schema-file-dir=dir],
                             [Directory for installing schema files.]),
              [MATECONF_SCHEMA_FILE_DIR="$withval"],)

  AC_SUBST(MATECONF_SCHEMA_FILE_DIR)

	if test "$1" = "yes"; then
		AC_MSG_RESULT([Using $MATECONF_SCHEMA_FILE_DIR as install directory for schema files])
	fi

  AC_ARG_ENABLE(schemas-install,
        AC_HELP_STRING([--disable-schemas-install],
                       [Disable the schemas installation]),
     [case ${enableval} in
       yes|no) ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-schemas-install]) ;;
      esac])

	if test "x${enable_schemas_install}" = "xno" -o "$1" != "yes"; then
		msg_schemas_install="disabled"; else
		msg_schemas_install="enabled in ${MATECONF_SCHEMA_FILE_DIR}"
	fi
      
  AM_CONDITIONAL([MATECONF_SCHEMAS_INSTALL], [test "$enable_schemas_install" != no -a "$1" = "yes"])
])

dnl let the user choose whether to compile with MateConf enabled
dnl --enable-mateconf
dnl
dnl defaults to automatically enable MateConf if it is present on the compiling
dnl system, or disable it if it is absent
dnl if --enable-mateconf is specified, then MateConf subsystem must be present

AC_DEFUN([NA_CHECK_FOR_MATECONF],[
	AC_REQUIRE([_AC_NA_ARG_MATECONF])dnl
	AC_REQUIRE([_AC_NA_CHECK_MATECONF])dnl
])

AC_DEFUN([_AC_NA_ARG_MATECONF],[
	AC_ARG_ENABLE(
		[mateconf],
		AC_HELP_STRING(
			[--enable-mateconf],
			[whether to enable MateConf subsystem @<:@auto@:>@]
		),
	[enable_mateconf=$enableval],
	[enable_mateconf="auto"]
	)
])

AC_DEFUN([_AC_NA_CHECK_MATECONF],[
	AC_MSG_CHECKING([whether MateConf is enabled])
	AC_MSG_RESULT([${enable_mateconf}])

	if test "${enable_mateconf}" = "auto"; then
		AC_PATH_PROG([MATECONFTOOL],[mateconftool-2],[no])
		if test "${MATECONFTOOL}" = "no"; then
			enable_mateconf="no"
		else
			enable_mateconf="yes"
		fi
	else
		if test "${enable_mateconf}" = "yes"; then
			AC_PATH_PROG([MATECONFTOOL],[mateconftool-2],[no])
			if test "${MATECONFTOOL}" = "no"; then
				AC_MSG_ERROR([mateconftool-2: program not found])
			fi
		fi
	fi
	
	if test "${enable_mateconf}" = "yes"; then
		AC_SUBST([AM_CPPFLAGS],["${AM_CPPFLAGS} -DHAVE_MATECONF"])
		AC_DEFINE_UNQUOTED([HAVE_MATECONF],[1],[Whether we compile against the MateConf library])
	fi

	NA_MATECONF_SOURCE_2(["${enable_mateconf}"])
	AM_CONDITIONAL([HAVE_MATECONF], [test "${enable_mateconf}" = "yes"])
])
