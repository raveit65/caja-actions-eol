# Caja Actions
# A Caja extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The MATE Foundation
# Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009, 2010 Pierre Wieser and others (see AUTHORS)
#
# This Program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This Program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this Library; see the file COPYING.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place,
# Suite 330, Boston, MA 02111-1307, USA.
#
# Authors:
#   Frederic Ruaudel <grumz@grumz.net>
#   Rodrigo Moya <rodrigo@mate-db.org>
#   Pierre Wieser <pwieser@trychlos.org>
#   ... and many others (see AUTHORS)

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	doc									\
	m4									\
	icons								\
	src									\
	po									\
	data								\
	$(NULL)

# a foo directory so that make distcheck has a vpath which is prefixed
# with a make variable, instead of hardcoded as returned by pkg-config
DISTCHECK_CONFIGURE_FLAGS = \
	--with-caja-extdir='$${libdir}/caja/extensions-2.0-distcheck'

# do not try to install schemas when making distcheck
DISTCHECK_CONFIGURE_FLAGS += \
	--disable-schemas-install			\
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
	COPYING								\
	INSTALL								\
	MAINTAINERS							\
	NEWS								\
	README								\
	TODO								\
	$(NULL)

uninstall-hook:
	-\rm -fr $(DESTDIR)$(pkgdocdir)
