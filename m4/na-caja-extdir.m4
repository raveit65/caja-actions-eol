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

# serial 2 change CACT_ prefix to NA_ (Caja-Actions)

# let the user specify an alternate caja-extension dir
# --with-caja-extdir=<dir>

AC_DEFUN([NA_CAJA_EXTDIR],[
	AC_REQUIRE([_AC_ARG_NA_CAJA_EXTDIR])dnl
	AC_REQUIRE([_AC_NA_CHECK_CAJA_EXTDIR])dnl
	if test "${with_caja_extdir}" = ""; then
		AC_MSG_ERROR([Unable to determine caja extension folder, please use --with-caja-extdir option])
	else
		AC_MSG_NOTICE([installing plugin in ${with_caja_extdir}])
		AC_SUBST([CAJA_EXTENSIONS_DIR],[${with_caja_extdir}])
		AC_DEFINE_UNQUOTED([NA_CAJA_EXTENSIONS_DIR],[${with_caja_extdir}],[Caja extensions directory])
	fi
])

AC_DEFUN([_AC_ARG_NA_CAJA_EXTDIR],[
	AC_ARG_WITH(
		[caja-extdir],
		AC_HELP_STRING(
			[--with-caja-extdir=DIR],
			[caja plugins extension directory @<:@auto@:>@]
		),
	[with_caja_extdir=$withval],
	[with_caja_extdir=""]
	)
])

AC_DEFUN([_AC_NA_CHECK_CAJA_EXTDIR],[
	if test "${with_caja_extdir}" = ""; then
		if test "{PKG_CONFIG}" != ""; then
			with_caja_extdir=`${PKG_CONFIG} --variable=extensiondir libcaja-extension`
		fi
	fi
])
