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

AM_CPPFLAGS += \
	-I $(top_srcdir)											\
	-I $(top_srcdir)/src										\
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_PLUGIN_TRACKER}\"			\
	$(CAJA_ACTIONS_CFLAGS)									\
	$(NULL)

caja_extensiondir = $(CAJA_EXTENSIONS_DIR)

caja_extension_LTLIBRARIES = libcaja-actions-tracker.la

BUILT_SOURCES = \
	na-tracker-dbus-glue.h										\
	$(NULL)

na-tracker-dbus-glue.h: na-tracker-dbus.xml
	dbus-binding-tool --mode=glib-server --prefix=na_tracker_dbus $< > $@

libcaja_actions_tracker_la_SOURCES = \
	na-tracker.c												\
	na-tracker.h												\
	na-tracker-dbus.c											\
	na-tracker-dbus.h											\
	na-tracker-plugin.c											\
	$(NULL)

libcaja_actions_tracker_la_LIBADD = \
	$(top_builddir)/src/core/libna-core.la						\
	$(CAJA_ACTIONS_LIBS)									\
	$(NULL)

libcaja_actions_tracker_la_LDFLAGS = \
	-module 													\
	-no-undefined												\
	-avoid-version												\
	$(NULL)

EXTRA_DIST = \
	na-tracker-dbus.xml											\
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES)											\
	$(NULL)
