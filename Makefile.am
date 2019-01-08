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

DISTCHECK_CONFIGURE_FLAGS = \
	--with-caja-extdir='$${libdir}/caja/extensions-2.0-distcheck' \
	--enable-silent-rules \
	--enable-as-needed \
	--enable-deprecated \
	--disable-html-manuals \
	--disable-pdf-manuals \
	CFLAGS='-Wno-deprecated-declarations' \ \
	$(NULL)

EXTRA_DIST =	\
	autogen.sh                          \
	gtk-doc.make						\
	intltool-extract.in					\
	intltool-merge.in					\
	intltool-update.in					\
	$(NULL)

DISTCLEANFILES =	\
	intltool-extract					\
	intltool-merge						\
	intltool-update						\
	$(NULL)

# Though the Mate doc says to add these files to DISTCLEANFILES, the
# Automake manual states that distclean target should not remove any
# distributed files (but leave a clean distribution)  
MAINTAINERCLEANFILES =	\
	gtk-doc.make		\
	$(NULL)

pkgdocdir = $(datarootdir)/doc/@PACKAGE@

dist_pkgdoc_DATA =	\
	AUTHORS								\
	ChangeLog							\
	ChangeLog-2008						\
	ChangeLog-2009						\
	ChangeLog-2010						\
	ChangeLog-2011						\
	ChangeLog-2012						\
	COPYING								\
	COPYING-DOCS						\
	INSTALL								\
	MAINTAINERS							\
	NEWS								\
	README								\
	$(NULL)

uninstall-hook:
	-\rm -fr $(DESTDIR)$(pkgdocdir)

# Build ChangeLog from GIT  history
ChangeLog:
	$(AM_V_GEN) if test -d $(top_srcdir)/.git; then \
		GIT_DIR="$(top_srcdir)/.git" git log --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog
