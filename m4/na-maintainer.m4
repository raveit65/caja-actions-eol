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

# serial 4 rename macro to NA_MAINTAINER_CHECK_MODE
#                          NA_MAINTAINER_CHECK_FOR_DEPRECATED

dnl define NA_MAINTAINER_CHECK_MODE
dnl
dnl Don''t agree with maintainer mode use
dnl See http://www.gnu.org/software/automake/manual/automake.html#maintainer_002dmode
dnl but mate-autogen.sh forces its usage and mate_common_init requires it
dnl is nonetheless explicitely required by mate_maintainer_mode_defines macro

AC_DEFUN([NA_MAINTAINER_CHECK_MODE],[
	AC_REQUIRE([MATE_MAINTAINER_MODE_DEFINES])

	msg_maintainer_mode="disabled"
	AC_MSG_CHECKING([whether to enable maintainer mode])
	AC_MSG_RESULT([${USE_MAINTAINER_MODE}])

	if test "${USE_MAINTAINER_MODE}" = "yes"; then
		AC_DEFINE([NA_MAINTAINER_MODE],[1],[Define to 1 if we are in maintainer mode])
		AC_SUBST([AM_CPPFLAGS],["${AM_CPPFLAGS} ${DISABLE_DEPRECATED} -DGSEAL_ENABLED"])
		AC_SUBST([AM_CFLAGS],["${AM_CFLAGS} -Werror"])
		msg_maintainer_mode="enabled"
	fi

	AM_CONDITIONAL([NA_MAINTAINER_MODE], [test "${USE_MAINTAINER_MODE}" = "yes"])
])

AC_DEFUN([NA_MAINTAINER_CHECK_FOR_DEPRECATED],[
	AC_ARG_ENABLE(
		[deprecated],
		AC_HELP_STRING(
			[--enable-deprecated],
			[whether to enable deprecated symbols]),
		[enable_deprecated=$enableval],
		[enable_deprecated="no"])

	AC_MSG_CHECKING([whether deprecated symbols should be enabled])
	AC_MSG_RESULT([${enable_deprecated}])

	if test "${enable_deprecated}" = "yes"; then
		AC_DEFINE([NA_ENABLE_DEPRECATED],[1],[Define to 1 if deprecated symbols should be enabled])
	else
		if test "${na_request_for_deprecated}" = "yes"; then
			AC_MSG_WARN([API documentation will be incomplete as deprecated symbols are disabled])
			let na_fatal_count+=1
		fi
	fi

	AM_CONDITIONAL([ENABLE_DEPRECATED], [test "${enable_deprecated}" = "yes"])
])
