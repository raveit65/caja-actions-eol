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

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	m4									\
	icons								\
	src									\
	po									\
	data								\
	docs								\
	tools								\
	$(NULL)

# - caja-extdir is a foo directory so that make distcheck has a vpath
#   which is prefixed with a make variable, instead of hardcoded as returned
#   by pkg-config
# - do not try to install schemas when making distcheck
# - do not rebuild CACT manuals
# - gtk-doc requires to be enabled when making dist
# - MATE_DOC_DEFINES tries to disable scrollkeeper, but the set is
#   overriden but this one

DISTCHECK_CONFIGURE_FLAGS = \
	--with-caja-extdir='$${libdir}/caja/extensions-2.0-distcheck' \
	--enable-silent-rules				\
	--enable-as-needed					\
	--disable-scrollkeeper				\
	--enable-deprecated					\
	--enable-gtk-doc					\
	--disable-html-manuals				\
	--disable-pdf-manuals				\
	$(NULL)

EXTRA_DIST = \
	intltool-extract.in					\
	intltool-merge.in					\
	intltool-update.in					\
	$(NULL)

DISTCLEANFILES = \
	intltool-extract					\
	intltool-merge						\
	intltool-update						\
	$(NULL)

pkgdocdir = $(datarootdir)/doc/@PACKAGE@-@VERSION@

dist_pkgdoc_DATA = \
	AUTHORS								\
	ChangeLog							\
	ChangeLog-2008						\
	ChangeLog-2009						\
	ChangeLog-2010						\
	ChangeLog-2011						\
	COPYING								\
	COPYING-DOCS						\
	INSTALL								\
	MAINTAINERS							\
	NEWS								\
	README								\
	$(NULL)

uninstall-hook:
	-\rm -fr $(DESTDIR)$(pkgdocdir)
