# Caja-Actions
# A Caja extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The MATE Foundation
# Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
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

dnl --with-default-io-provider=mateconf|desktop
dnl   Defines the default I/O Provider when creating a new action
dnl   Default to 'desktop'
dnl
dnl usage:  NA_SET_DEFAULT_IO_PROVIDER([default_io_provider])
dnl
dnl ac_define NA_DEFAULT_IO_PROVIDER variable

AC_DEFUN([NA_SET_DEFAULT_IO_PROVIDER],[
	_AC_ARG_NA_WITH_DEFAULT_IO_PROVIDER([$1])
	_CHECK_FOR_DEFAULT_IO_PROVIDER
])

AC_DEFUN([_AC_ARG_NA_WITH_DEFAULT_IO_PROVIDER],[
	AC_ARG_WITH(
		[default-io-provider],
		AS_HELP_STRING(
			[--with-default-io-provider@<:@=na-mateconf|na-desktop@:>@],
			[define default I/O provider  @<:@$1@:>@]),
			[with_default_io_provider=$withval],
			[with_default_io_provider="$1"])
])

AC_DEFUN([_CHECK_FOR_DEFAULT_IO_PROVIDER],[
	AC_MSG_CHECKING([for default I/O provider on new items])
	AC_MSG_RESULT([${with_default_io_provider}])
	if test "x${with_default_io_provider}" != "xna-mateconf"; then
		if test "x${with_default_io_provider}" != "xna-desktop"; then
			AC_MSG_ERROR([a default I/O provider must be specified, must be 'na-mateconf' or 'na-desktop'])
		fi
	fi

	AC_DEFINE_UNQUOTED([NA_DEFAULT_IO_PROVIDER],["${with_default_io_provider}"],[Default I/O Provider])
])
