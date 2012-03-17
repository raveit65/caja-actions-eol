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
	caja-actions-config-tool						\
	$(NULL)

egg_platform_defines = -DEGG_SM_CLIENT_BACKEND_XSMP

AM_CPPFLAGS += \
	-I $(top_srcdir)									\
	-I $(top_srcdir)/src								\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"			\
	-DPKGDATADIR=\"$(pkgdatadir)\"						\
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_NACT}\"				\
	$(egg_platform_defines)								\
	$(CAJA_ACTIONS_CFLAGS)							\
	$(NULL)

BUILT_SOURCES = \
	nact-marshal.c										\
	nact-marshal.h										\
	$(NULL)

caja_actions_config_tool_SOURCES = \
	base-application.c									\
	base-application.h									\
	base-application-class.h							\
	base-assistant.c									\
	base-assistant.h									\
	base-builder.c										\
	base-builder.h										\
	base-dialog.c										\
	base-dialog.h										\
	base-iprefs.c										\
	base-iprefs.h										\
	base-window.c										\
	base-window.h										\
	base-window-class.h									\
	egg-desktop-file.c									\
	egg-desktop-file.h									\
	egg-sm-client.c										\
	egg-sm-client.h										\
	egg-sm-client-private.h								\
	egg-sm-client-xsmp.c								\
	egg-tree-multi-dnd.c								\
	egg-tree-multi-dnd.h								\
	nact-application.c									\
	nact-application.h									\
	nact-assistant-export.c								\
	nact-assistant-export.h								\
	nact-assistant-import.c								\
	nact-assistant-import.h								\
	nact-clipboard.c									\
	nact-clipboard.h									\
	nact-confirm-logout.c								\
	nact-confirm-logout.h								\
	nact-export-ask.c									\
	nact-export-ask.h									\
	nact-export-format.c								\
	nact-export-format.h								\
	nact-gtk-utils.c									\
	nact-gtk-utils.h									\
	nact-iactions-list.c								\
	nact-iactions-list.h								\
	nact-iactions-list-bis.c							\
	nact-iactions-list-priv.c							\
	nact-iactions-list-priv.h							\
	nact-iaction-tab.c									\
	nact-iaction-tab.h									\
	nact-iadvanced-tab.c								\
	nact-iadvanced-tab.h								\
	nact-ifolders-tab.c									\
	nact-ifolders-tab.h									\
	nact-icommand-tab.c									\
	nact-icommand-tab.h									\
	nact-iconditions-tab.c								\
	nact-iconditions-tab.h								\
	nact-iprefs.c										\
	nact-iprefs.h										\
	nact-main.c											\
	nact-main-menubar.c									\
	nact-main-menubar.h									\
	nact-main-menubar-edit.c							\
	nact-main-menubar-edit.h							\
	nact-main-menubar-file.c							\
	nact-main-menubar-file.h							\
	nact-main-menubar-help.c							\
	nact-main-menubar-help.h							\
	nact-main-menubar-maintainer.c						\
	nact-main-menubar-maintainer.h						\
	nact-main-menubar-tools.c							\
	nact-main-menubar-tools.h							\
	nact-main-menubar-view.c							\
	nact-main-menubar-view.h							\
	nact-main-statusbar.c								\
	nact-main-statusbar.h								\
	nact-main-tab.c										\
	nact-main-tab.h										\
	nact-main-toolbar.c									\
	nact-main-toolbar.h									\
	nact-main-window.c									\
	nact-main-window.h									\
	nact-preferences-editor.c							\
	nact-preferences-editor.h							\
	nact-providers-list.c								\
	nact-providers-list.h								\
	nact-schemes-list.c									\
	nact-schemes-list.h									\
	nact-sort-buttons.c									\
	nact-sort-buttons.h									\
	nact-tree-model.c									\
	nact-tree-model.h									\
	nact-tree-model-dnd.c								\
	nact-tree-model-dnd.h								\
	nact-tree-model-priv.h								\
	nact-window.c										\
	nact-window.h										\
	$(BUILT_SOURCES)									\
	$(NULL)

nact-marshal.h: nact-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=nact_marshal > $@

nact-marshal.c: nact-marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"nact-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=nact_marshal >> $@

caja_actions_config_tool_LDADD = \
	$(top_builddir)/src/core/libna-core.la				\
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

pkgdata_DATA = \
	caja-actions-config-tool.actions				\
	caja-actions-maintainer.actions					\
	nact-assistant-export.ui							\
	nact-preferences.ui									\
	caja-actions-config-tool.ui						\
	locked.png											\
	transparent.png										\
	$(NULL)

@INTLTOOL_DESKTOP_RULE@

applications_in_files = nact.desktop.in

applications_files = $(applications_in_files:.desktop.in=.desktop)

applicationsdir = $(datadir)/applications

applications_DATA = $(applications_files)

CLEANFILES = \
	$(applications_files)								\
	$(BUILT_SOURCES)									\
	$(NULL)

EXTRA_DIST = \
	$(applications_in_files)							\
	$(pkgdata_DATA)										\
	nact-marshal.list									\
	$(NULL)
