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

# serial 3 prefix message with 'msg_'

dnl define NA_MAINTAINER_MODE

AC_DEFUN([NA_IS_MAINTAINER_MODE],[
	msg_maintainer_mode="disabled"
	AC_MSG_CHECKING([whether enabling maintainer mode])
	AC_MSG_RESULT([${USE_MAINTAINER_MODE}])

	if test "${USE_MAINTAINER_MODE}" = "yes"; then
		AC_DEFINE([NA_MAINTAINER_MODE],[1],[Define to 1 if we are in maintainer mode])
		AC_SUBST([AM_CPPFLAGS],["${AM_CPPFLAGS} ${DISABLE_DEPRECATED} -DGSEAL_ENABLED"])
		AC_SUBST([AM_CFLAGS],["${AM_CFLAGS} -Werror"])
		msg_maintainer_mode="enabled"
	fi

	AM_CONDITIONAL([NA_MAINTAINER_MODE], [test "${USE_MAINTAINER_MODE}" = "yes"])
])

AC_DEFUN([NA_CHECK_FOR_DEPRECATED],[
	AC_ARG_ENABLE(
		[deprecated],
		AC_HELP_STRING(
			[--enable-deprecated],
			[whether to enable deprecated functions @<:@no@:>@]
		),
	[enable_deprecated=$enableval],
	[enable_deprecated="no"]
	)

	AC_MSG_CHECKING([whether deprecated symbols should be enabled])
	AC_MSG_RESULT([${enable_deprecated}])

	if test "${enable_deprecated}" = "yes"; then
		AC_DEFINE([NA_ENABLE_DEPRECATED],[1],[Define to 1 if deprecated functions should be enabled])
	fi

	AM_CONDITIONAL([ENABLE_DEPRECATED], [test "${enable_deprecated}" = "yes"])
])
