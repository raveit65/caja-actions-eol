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

dnl check for GDBus (GIO 2.26) or fallback to dbus-glib if present
dnl as of N-A 3.2, this is only required by Tracker plugin
dnl we output with two defined (or not) variables:
dnl   HAVE_DBUS_GLIB if glib < 2.26 and we have dbus-glib-1
dnl   HAVE_GDBUS for glib 2.26 and above
dnl
dnl pwi 2012-01-27 this M4sh will become useless as soon as we
dnl                start requiring glib >= 2.30
dnl                required by GDBusObjectManagerServer

AC_DEFUN([NA_CHECK_FOR_GDBUS],[
	_na_have_gdbus="no"
	_na_have_dbus_glib="no"
	
	PKG_CHECK_MODULES([GDBUS],[gio-2.0 >= 2.30],[
		_na_have_gdbus="yes"
		],[
		PKG_CHECK_MODULES([DBUS_GLIB],[dbus-glib-1],[
			_na_have_dbus_glib="yes"
		])
	])

	if test "${_na_have_gdbus}" = "yes"; then
		AC_DEFINE_UNQUOTED([HAVE_GDBUS],[1],[Whether GDbus is available])
		CAJA_ACTIONS_CFLAGS="${CAJA_ACTIONS_CFLAGS} ${GDBUS_CFLAGS}"
		CAJA_ACTIONS_LIBS="${CAJA_ACTIONS_LIBS} ${GDBUS_LIBS}"
	fi
	if test "${_na_have_dbus_glib}" = "yes"; then
		AC_DEFINE_UNQUOTED([HAVE_DBUS_GLIB],[1],[Whether Dbus-GLib is available])
		CAJA_ACTIONS_CFLAGS="${CAJA_ACTIONS_CFLAGS} ${DBUS_GLIB_CFLAGS}"
		CAJA_ACTIONS_LIBS="${CAJA_ACTIONS_LIBS} ${DBUS_GLIB_LIBS}"
	fi
	
	AM_CONDITIONAL([HAVE_GDBUS],[test "${_na_have_gdbus}" = "yes"])
	AM_CONDITIONAL([HAVE_DBUS_GLIB],[test "${_na_have_dbus_glib}" = "yes"])
])
