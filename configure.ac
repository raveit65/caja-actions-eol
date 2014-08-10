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
#   Wolfgang Ulbrich <chat-to-me@raveit.de>
#   ... and many others (see AUTHORS)

AC_PREREQ([2.53])

AC_INIT([Caja-Actions],[1.8.0],[chat-to-me@raveit.de],,[https://github.com/NiceandGently/caja-actions])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz])

AC_CONFIG_SRCDIR([src/plugin-menu/caja-module.c])
AC_CONFIG_HEADERS([src/config.h])

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

AC_CONFIG_MACRO_DIR([m4])

# pwi 2009-05-15 disabled as MATE_COMMON_INIT takes care of that
#AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS")

# don't agree with maintainer mode use
# see http://www.gnu.org/software/automake/manual/automake.html#index-AM_005fMAINTAINER_005fMODE-1001
# but mate-autogen.sh forces its usage and mate_common_init requires it
AM_MAINTAINER_MODE

# check for compiler characteristics and options
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_STDC_HEADERS
AM_DISABLE_STATIC

# other traditional tools
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Mate stuff
MATE_COMMON_INIT
MATE_MAINTAINER_MODE_DEFINES
NA_LINK_AS_NEEDED
NA_MATE_COMPILE_WARNINGS([],[-ansi -Wno-overlength-strings -Wformat=2])
AC_SUBST([AM_CFLAGS],["${AM_CFLAGS} ${WARN_CFLAGS}"])

# GLib marshaling
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

# libtool
AM_PROG_LIBTOOL

# localization
# note that this same version is also required by autogen.sh
IT_PROG_INTLTOOL([0.35.5])
GETTEXT_PACKAGE=${PACKAGE}
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["${GETTEXT_PACKAGE}"],[gettext package])
AM_GLIB_GNU_GETTEXT

# Enable deprecated functions ?
NA_CHECK_FOR_DEPRECATED

# note that requiring through pkg-config the version number of the
# caja-extension library actually returns the version number of
# caja itself (e.g. 1.6.0) instead of the library version info
# (1.1.0) ; for the usual final user, this is very analog to requiring
# a whole mate version
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "${PKG_CONFIG}" = "no"; then
	AC_MSG_ERROR([You need to install pkg-config])
fi

# Oldest supported distribution: Ubuntu 10 LTS (Mate 2.30): april 2013
glib_required=2.24.0
gtk_required=2.20.0
NA_CHECK_FOR_GTK

# GDBus comes in GIO with 2.26
# so uses GDBus if present, or fallback into dbus-glib-1
NA_CHECK_MODULE([GLIB],   [glib-2.0 >= ${glib_required}])
NA_CHECK_MODULE([GMODULE],[gmodule-2.0 >= ${glib_required}])
NA_CHECK_FOR_GDBUS

dnl aclocal complains if the comma is not just after the square bracket
NA_CHECK_MODULE([GIO_UNIX],          [gio-unix-2.0])
NA_CHECK_MODULE([GTOP],              [libgtop-2.0 >= 2.23.1])
NA_CHECK_MODULE([LIBXML],            [libxml-2.0 >= 2.6])
NA_CHECK_MODULE([SM],                [sm >= 1.0])
NA_CHECK_MODULE([ICE],               [ice])
NA_CHECK_MODULE([UUID],              [uuid])
NA_CHECK_MODULE([CAJA_EXTENSION],[libcaja-extension >= 1.4.0])

AC_SUBST([CAJA_ACTIONS_CFLAGS])
AC_SUBST([CAJA_ACTIONS_LIBS])

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


# defines log domains when in maintainer mode
NA_LOG_DOMAINS

# add --enable-html-manuals and --enable-pdf-manuals configure options
NA_ENABLE_MANUALS

# add --with-default-io-provider option
NA_SET_DEFAULT_IO_PROVIDER([na-desktop])

# when working in a test environment, caja extensions are typically
# installed in a non-standard location ; lets specify this location here
NA_CAJA_EXTDIR

# define NA_MAINTAINER_MODE variable
NA_IS_MAINTAINER_MODE
AC_DEFINE([CAJA_ACTIONS_DEBUG],["CAJA_ACTIONS_DEBUG"],[Debug environment variable])

# Check for menu update function
AC_CHECK_LIB([caja-extension],[caja_menu_item_new])
AC_CHECK_FUNCS([caja_menu_provider_emit_items_updated_signal])
AC_CHECK_FUNCS([caja_menu_provider_get_toolbar_items])

# display and keep configuration informations
config_options="`$as_echo "$ac_configure_args" | sed 's/^ //; s/[\\""\`\$]/\\\\&/g'`" 
AC_DEFINE_UNQUOTED([NA_CONFIG_OPTIONS],["$0 ${config_options}"],["Configure options"])

AC_OUTPUT

msg_gtk_doc=`if test "x${enable_gtk_doc}" = "xno"; then echo "disabled"; else echo "enabled"; fi`
msg_gtk_version=""
if test "${have_gtk3}" = "yes"; then msg_gtk_version="Gtk+ 3.0"; 
	elif  test "${have_gtk2}" = "yes"; then msg_gtk_version="Gtk+ 2.0"; fi
$as_echo "
	${PACKAGE_STRING} configuration summary:

	Installation prefix             ${prefix}
	Build system type               ${ac_cv_build}
	Caja extensions directory       ${with_caja_extdir}
	Gtk+ version                    ${msg_gtk_version}
	Default I/O provider            ${with_default_io_provider}
	Enable deprecated functions     ${enable_deprecated}
	Maintainer mode                 ${msg_maintainer_mode}
	API Reference generation        ${msg_gtk_doc}
	HTML User's Manuals generation  ${msg_html_manuals}
	PDF User's Manuals generation   ${msg_pdf_manuals}
"
#	Preprocessor flags              ${AM_CPPFLAGS}
#	Compiler flags                  ${AM_CFLAGS} ${CAJA_ACTIONS_CFLAGS}
#	Linker flags                    ${CAJA_ACTIONS_LIBS}
