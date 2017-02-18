# Caja-Actions
# A Caja extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The GNOME Foundation
# Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009-2014 Pierre Wieser and others (see AUTHORS)
# Copyright (C) 2012-2017 Wolfgang Ulbrich and others (see AUTHORS)
#
# Caja-Actions is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# Caja-Actions is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Caja-Actions; see the file COPYING. If not, see
# <http://www.gnu.org/licenses/>.
#
# Authors:
#   Frederic Ruaudel <grumz@grumz.net>
#   Rodrigo Moya <rodrigo@gnome-db.org>
#   Pierre Wieser <pwieser@trychlos.org>
#   ... and many others (see AUTHORS)

# serial 1 let the user choose a target file-manager

dnl defaults to caja

AC_DEFUN([NA_TARGET_FILE_MANAGER],[

	AC_ARG_ENABLE([file-manager],
		AC_HELP_STRING(
			[--enable-file-manager=@<:@caja|nemo@:>@],
			[the targeted file manager @<:@caja@:>@]),
		[enable_file_manager=$withval],
		[enable_file_manager="caja"])

	if test "${enable_file_manager}" = "caja"; then
		AC_MSG_NOTICE([targeting Caja file-manager])
		AC_REQUIRE([_AC_NA_FILE_MANAGER_CAJA])dnl

	elif test "${enable_file_manager}" = "nemo"; then
		AC_MSG_NOTICE([targeting Nemo file-manager])
	fi
])

# target file manager: caja
# when working in a test environment, caja extensions are typically
# installed in a non-standard location; lets specify this location here
# --with-caja-extdir=<dir>

AC_DEFUN([_AC_NA_FILE_MANAGER_CAJA],[

	AC_ARG_WITH(
		[caja-extdir],
		AC_HELP_STRING(
			[--with-caja-extdir=DIR],
			[caja plugins extension directory @<:@auto@:>@]),
		[with_caja_extdir=$withval],
		[with_caja_extdir=""])

	if test "${with_caja_extdir}" = ""; then
		if test "{PKG_CONFIG}" != ""; then
			with_caja_extdir=`${PKG_CONFIG} --variable=extensiondir libcaja-extension`
		fi
	fi
	if test "${with_caja_extdir}" = ""; then
		AC_MSG_ERROR([Unable to determine caja extension folder, please use --with-caja-extdir option])
	else
		AC_MSG_NOTICE([installing plugins in ${with_caja_extdir}])
		AC_SUBST([CAJA_EXTENSIONS_DIR],[${with_caja_extdir}])
		AC_DEFINE_UNQUOTED([NA_CAJA_EXTENSIONS_DIR],[${with_caja_extdir}],[Caja extensions directory])
	fi

	NA_CHECK_MODULE([CAJA_EXTENSION],[libcaja-extension],[${caja_required}])

	# Check for menu update function
	AC_CHECK_LIB([caja-extension],[caja_menu_item_new])
	AC_CHECK_FUNCS([caja_menu_provider_emit_items_updated_signal])

	#  add toolbar items
	AC_CHECK_FUNCS([caja_menu_provider_get_toolbar_items])
])

AC_DEFUN([_AC_NA_FILE_MANAGER_NEMO],[
])

