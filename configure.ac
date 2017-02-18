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
#   Wolfgang Ulbrich <chat-to-me@raveit.de>
#   ... and many others (see AUTHORS)

AC_PREREQ([2.53])

AC_INIT([Caja-Actions],[1.18.0],[chat-to-me@raveit.de],,[https://github.com/raveit65/caja-actions])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz check-news])

AC_CONFIG_SRCDIR([src/cact/main.c])
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_MACRO_DIR([m4])

# check for compiler characteristics and options
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_STDC_HEADERS
AM_DISABLE_STATIC

# define specific compilation options
NA_COMPILER_WARNINGS([],[-ansi -Wno-overlength-strings -Wformat=2])
NA_COMPILER_LINK_AS_NEEDED
AC_SUBST([AM_CFLAGS],["${AM_CFLAGS} ${WARN_CFLAGS}"])

# other traditional tools
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Mate stuff
MATE_COMMON_INIT
NA_MAINTAINER_CHECK_MODE

# libtool
AM_PROG_LIBTOOL

# we are using pkgconfig for all development libraries we need
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "${PKG_CONFIG}" = "no"; then
	AC_MSG_ERROR([You need to install pkg-config])
fi

glib_required=2.32.1
gtk_required=3.10
gtop_required=2.23.1
xml_required=2.6
sm_required=1.0
intltool_required=0.50.2
caja_required=1.17.1

# localization
# note that this same version is also required by autogen.sh
IT_PROG_INTLTOOL([${intltool_required}])
GETTEXT_PACKAGE=${PACKAGE}
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["${GETTEXT_PACKAGE}"],[gettext package])
AM_GLIB_GNU_GETTEXT

# a counter of fatal warnings emitted
let na_fatal_count=0
na_fatal_list=""

# GLib marshaling
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal, no)
if test "${GLIB_GENMARSHAL}" = "no"; then
	AC_MSG_WARN([glib2-devel installed, but glib-genmarshal not found])
	let na_fatal_count+=1
fi

dnl aclocal complains if the comma is not just after the square bracket
# As of 1.18.0 release, remove Gtk+2 support, only allowing for Gtk+3
dnl aclocal complains if the comma is not just after the square bracket
NA_CHECK_MODULE([GTK3],              [gtk+-3.0],           [${gtk_required}])
NA_CHECK_MODULE([GLIB],              [glib-2.0],           [${glib_required}])
NA_CHECK_MODULE([GMODULE],           [gmodule-2.0],        [${glib_required}])
NA_CHECK_MODULE([GIO_UNIX],          [gio-unix-2.0])
NA_CHECK_MODULE([GTOP],              [libgtop-2.0],        [${gtop_required}])
NA_CHECK_MODULE([LIBXML],            [libxml-2.0],         [${xml_required}])
NA_CHECK_MODULE([SM],                [sm],                 [${sm_required}])
NA_CHECK_MODULE([ICE],               [ice])
NA_CHECK_MODULE([UUID],              [uuid])

# target a file manager (caja, nemo, ...)
NA_TARGET_FILE_MANAGER

AC_SUBST([CAJA_ACTIONS_CFLAGS])
AC_SUBST([CAJA_ACTIONS_LIBS])

# defines log domains when in maintainer mode
NA_LOG_DOMAINS

# print the debug output be printed to the console
AC_DEFINE([CAJA_ACTIONS_DEBUG],["CAJA_ACTIONS_DEBUG"],[Debug environment variable])

# add --with-default-io-provider option
NA_SET_DEFAULT_IO_PROVIDER([na-desktop])

###

AC_CONFIG_FILES([
	Makefile
	data/Makefile
	data/actions/Makefile
	data/exports/Makefile
	docs/Makefile
	docs/cact/Makefile
	docs/reference/Makefile
	docs/reference/version.xml
	m4/Makefile
	icons/Makefile
	icons/16x16/Makefile
	icons/22x22/Makefile
	icons/32x32/Makefile
	icons/48x48/Makefile
	icons/scalable/Makefile
	src/Makefile
	src/api/Makefile
	src/core/Makefile
	src/io-desktop/Makefile
	src/plugin-menu/Makefile
	src/plugin-tracker/Makefile
	src/cact/Makefile
	src/test/Makefile
	src/utils/Makefile
	tools/Makefile
	po/Makefile.in
])

# Enable deprecated functions ?
NA_MAINTAINER_CHECK_FOR_DEPRECATED

# check for gtk-doc
# - starting with gtk-doc 1.16, is able to handle srcdir != builddir
# - API documentation should not be built (would be incomplete) when deprecated
#   symbols are not enabled
GTK_DOC_CHECK([1.16],[--flavour no-tmpl])
if test "${enable_gtk_doc}" = "yes" -a "${enable_deprecated}" = "no"; then
	AC_MSG_WARN([API documentation will be incomplete as deprecated symbols are disabled])
fi

# User documentation
YELP_HELP_INIT

# add --enable-html-manuals and --enable-pdf-manuals configure options
NA_ENABLE_MANUALS

# display and keep configuration informations
config_options="`$as_echo "$ac_configure_args" | sed 's/^ //; s/[\\""\`\$]/\\\\&/g'`" 
AC_DEFINE_UNQUOTED([NA_CONFIG_OPTIONS],["$0 ${config_options}"],["Configure options"])

AC_OUTPUT

msg_gtk_doc=`if test "x${enable_gtk_doc}" = "xno"; then echo "disabled"; else echo "enabled"; fi`

$as_echo "
	${PACKAGE_STRING} configuration summary:

	Installation prefix             ${prefix}
	Build system type               ${ac_cv_build}
	Gtk+ version                    ${GTK3_msg_version}
	GLib version                    ${GLIB_msg_version}
	Default I/O provider            ${with_default_io_provider}
	Targeted file-manager           ${enable_file_manager}"
if test "${enable_file_manager}" = "caja"; then
	$as_echo "	Caja extensions directory       ${with_caja_extdir}"
fi
$as_echo "	Maintainer mode                 ${msg_maintainer_mode}
	Enable deprecated functions     ${enable_deprecated}
	API Reference generation        ${msg_gtk_doc}
	HTML User's Manuals generation  ${msg_html_manuals}
	PDF User's Manuals generation   ${msg_pdf_manuals}
"
if test ${na_fatal_count} -gt 0; then
	$as_echo "${na_fatal_list}
"
	exit 1
fi

#	Preprocessor flags              ${AM_CPPFLAGS}
#	Compiler flags                  ${AM_CFLAGS} ${CAJA_ACTIONS_CFLAGS}
#	Linker flags                    ${CAJA_ACTIONS_LIBS}
