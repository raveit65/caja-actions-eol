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

pkglib_LTLIBRARIES = libna-core.la

AM_CPPFLAGS += \
	-I $(top_srcdir)									\
	-I $(top_srcdir)/src								\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"			\
	-DPKGDATADIR=\"$(pkgdatadir)\"						\
	-DPKGLIBDIR=\""$(pkglibdir)"\"						\
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_CORE}\"				\
	$(CAJA_ACTIONS_CFLAGS)							\
	$(NULL)

libna_core_la_SOURCES = \
	na-core-utils.c										\
	na-data-boxed.c										\
	na-data-def.c										\
	na-data-types.c										\
	na-exporter.c										\
	na-exporter.h										\
	na-export-format.c									\
	na-export-format.h									\
	na-factory-object.c									\
	na-factory-object.h									\
	na-factory-provider.c								\
	na-factory-provider.h								\
	na-mateconf-monitor.c									\
	na-mateconf-utils.c									\
	na-mate-vfs-uri.c									\
	na-mate-vfs-uri.h									\
	na-gtk-utils.c										\
	na-gtk-utils.h										\
	na-iabout.c											\
	na-iabout.h											\
	na-icontext.c										\
	na-icontext-factory.c								\
	na-iduplicable.c									\
	na-iexporter.c										\
	na-ifactory-object.c								\
	na-ifactory-provider.c								\
	na-iimporter.c										\
	na-iio-provider.c									\
	na-io-provider.c									\
	na-io-provider.h									\
	na-importer.c										\
	na-importer.h										\
	na-importer-ask.c									\
	na-importer-ask.h									\
	na-ipivot-consumer.c								\
	na-ipivot-consumer.h								\
	na-iprefs.c											\
	na-iprefs.h											\
	na-module.c											\
	na-module.h											\
	na-object.c											\
	na-object-id.c										\
	na-object-id-factory.c								\
	na-object-item.c									\
	na-object-item-factory.c							\
	na-object-action.c									\
	na-object-action-factory.c							\
	na-object-profile.c									\
	na-object-profile-factory.c							\
	na-object-menu.c									\
	na-object-menu-factory.c							\
	na-pivot.c											\
	na-pivot.h											\
	na-selected-info.c									\
	na-selected-info.h									\
	na-updater.c										\
	na-updater.h										\
	$(NULL)

libna_core_la_LIBADD = \
	$(NULL)

libna_core_la_LDFLAGS = \
	-no-undefined										\
	-avoid-version										\
	$(NULL)

pkgdata_DATA = \
	na-importer-ask.ui									\
	$(NULL)

EXTRA_DIST = \
	$(pkgdata_DATA)										\
	$(NULL)
