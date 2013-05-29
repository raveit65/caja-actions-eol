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

pkglib_LTLIBRARIES = libna-core.la

exportformat_datadir = $(pkgdatadir)/export-format

importmode_datadir = $(pkgdatadir)/import-mode

ui_datadir = $(pkgdatadir)/ui

AM_CPPFLAGS += \
	-I $(top_srcdir)									\
	-I $(top_srcdir)/src								\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"			\
	-DPKGLIBDIR=\""$(pkglibdir)"\"						\
	-DPKGLIBEXECDIR=\""$(pkglibexecdir)"\"				\
	-DSYSCONFDIR=\"$(sysconfdir)\"						\
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_CORE}\"				\
	-DPKGEXPORTFORMATDIR=\"$(exportformat_datadir)\"	\
	-DPKGIMPORTMODEDIR=\"$(importmode_datadir)\"		\
	-DPKGUIDIR=\"$(ui_datadir)\"						\
	$(CAJA_ACTIONS_CFLAGS)							\
	$(NULL)

BUILT_SOURCES = \
	na-marshal.c										\
	na-marshal.h										\
	$(NULL)

libna_core_la_SOURCES = \
	na-about.c											\
	na-about.h											\
	na-boxed.c											\
	na-core-utils.c										\
	na-data-boxed.c										\
	na-data-def.c										\
	na-data-types.c										\
	na-desktop-environment.c							\
	na-desktop-environment.h							\
	na-exporter.c										\
	na-exporter.h										\
	na-export-format.c									\
	na-export-format.h									\
	na-factory-object.c									\
	na-factory-object.h									\
	na-factory-provider.c								\
	na-factory-provider.h								\
	na-mate-vfs-uri.c									\
	na-mate-vfs-uri.h									\
	na-gtk-utils.c										\
	na-gtk-utils.h										\
	na-icontext.c										\
	na-icontext-factory.c								\
	na-iduplicable.c									\
	na-iexporter.c										\
	na-ifactory-object.c								\
	na-ifactory-provider.c								\
	na-iimporter.c										\
	na-iio-provider.c									\
	na-import-mode.c									\
	na-import-mode.h									\
	na-importer.c										\
	na-importer.h										\
	na-importer-ask.c									\
	na-importer-ask.h									\
	na-io-provider.c									\
	na-io-provider.h									\
	na-ioption.c										\
	na-ioption.h										\
	na-ioptions-list.c									\
	na-ioptions-list.h									\
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
	na-settings.c										\
	na-settings.h										\
	na-timeout.c										\
	na-tokens.c											\
	na-tokens.h											\
	na-updater.c										\
	na-updater.h										\
	$(BUILT_SOURCES)									\
	$(NULL)

na-marshal.h: na-marshal.def $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=na_cclosure_marshal > $@

na-marshal.c: na-marshal.def $(GLIB_GENMARSHAL)
	echo "#include \"na-marshal.h\"" > $@
	$(GLIB_GENMARSHAL) $< --body --prefix=na_cclosure_marshal >> $@

libna_core_la_LIBADD = \
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

libna_core_la_LDFLAGS = \
	-no-undefined										\
	-avoid-version										\
	-fPIC												\
	$(NULL)

ui_data_DATA = \
	na-importer-ask.ui									\
	$(NULL)

exportformat_data_DATA = \
	export-format-ask.png								\
	$(NULL)

importmode_data_DATA = \
	import-mode-ask.png									\
	import-mode-no-import.png							\
	import-mode-override.png							\
	import-mode-renumber.png							\
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES)									\
	$(NULL)

EXTRA_DIST = \
	$(ui_data_DATA)										\
	$(exportformat_data_DATA)							\
	$(importmode_data_DATA)								\
	na-marshal.def										\
	$(NULL)
