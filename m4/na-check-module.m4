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

dnl usage:  NA_CHECK_MODULE(var,condition[,error])
dnl
dnl this macro checks that gtk+-2.0 and gtk+-3.0 libraries are not mixed
dnl
dnl if 'error' != 'no', then displays an error message if condition is
dnl not met.
# translit($1, 'a-z', 'A-Z'),

AC_DEFUN([NA_CHECK_MODULE],[
	_cond="$2"
	if test "$3" != ""; then
		_cond="$2 >= $3"
	fi
	PKG_CHECK_MODULES([$1],[${_cond}],[have_$1="yes"],[have_$1="no"])

	#echo "have_gtk2=$have_gtk2 have_gtk3=$have_gtk3"
	#echo "$1_CFLAGS='${$1_CFLAGS}'"
	#echo "$1_LIBS='${$1_LIBS}'"
	#echo "against Gtk2: $(echo ${$1_LIBS} | grep -E 'gtk-@<:@^-@:>@+-2\.0')"
	#echo "against Gtk3: $(echo ${$1_LIBS} | grep -E 'gtk-@<:@^-@:>@+-3\.0')"

	if test "${have_$1}" = "yes"; then
		$1_msg_version=$(pkg-config --modversion $2)
		CAJA_ACTIONS_CFLAGS="${CAJA_ACTIONS_CFLAGS} ${$1_CFLAGS}"
		CAJA_ACTIONS_LIBS="${CAJA_ACTIONS_LIBS} ${$1_LIBS}"
	else
		_NA_CHECK_MODULE_MSG([$4],[$1: condition ${_cond} not satisfied])
	fi
])

AC_DEFUN([_NA_CHECK_MODULE_MSG],[
	if test "$1" = "no"; then
		AC_MSG_RESULT([warning: $2])
	else
		AC_MSG_ERROR([$2])
	fi
])
