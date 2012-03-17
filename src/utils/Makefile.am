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

bin_PROGRAMS = \
	caja-actions-new										\
	caja-actions-run										\
	caja-actions-schemas									\
	$(NULL)

AM_CPPFLAGS += \
	-I $(top_srcdir)											\
	-I $(top_srcdir)/src										\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"					\
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_UTILS}\"					\
	$(CAJA_ACTIONS_CFLAGS)									\
	$(NULL)

caja_actions_new_SOURCES = \
	caja-actions-new.c										\
	console-utils.c												\
	console-utils.h												\
	$(NULL)

caja_actions_new_LDADD = \
	$(top_builddir)/src/core/libna-core.la						\
	$(CAJA_ACTIONS_LIBS)									\
	$(NULL)

BUILT_SOURCES = \
	caja-actions-run-bindings.h								\
	$(NULL)

caja-actions-run-bindings.h: $(top_srcdir)/src/plugin-tracker/na-tracker-dbus.xml
	dbus-binding-tool --mode=glib-client $< > $@

caja_actions_run_SOURCES = \
	caja-actions-run.c										\
	console-utils.c												\
	console-utils.h												\
	$(NULL)

caja_actions_run_LDADD = \
	$(top_builddir)/src/core/libna-core.la						\
	$(CAJA_ACTIONS_LIBS)									\
	$(NULL)

caja_actions_schemas_SOURCES = \
	caja-actions-schemas.c									\
	console-utils.c												\
	console-utils.h												\
	$(NULL)

caja_actions_schemas_LDADD = \
	$(top_builddir)/src/core/libna-core.la						\
	$(CAJA_ACTIONS_LIBS)									\
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES)											\
	$(NULL)
