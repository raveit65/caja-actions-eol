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
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_CACT}\"				\
	$(egg_platform_defines)								\
	$(CAJA_ACTIONS_CFLAGS)							\
	$(NULL)

BUILT_SOURCES = \
	cact-marshal.c										\
	cact-marshal.h										\
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
	cact-application.c									\
	cact-application.h									\
	cact-assistant-export.c								\
	cact-assistant-export.h								\
	cact-assistant-import.c								\
	cact-assistant-import.h								\
	cact-clipboard.c									\
	cact-clipboard.h									\
	cact-confirm-logout.c								\
	cact-confirm-logout.h								\
	cact-export-ask.c									\
	cact-export-ask.h									\
	cact-export-format.c								\
	cact-export-format.h								\
	cact-gtk-utils.c									\
	cact-gtk-utils.h									\
	cact-iactions-list.c								\
	cact-iactions-list.h								\
	cact-iactions-list-bis.c							\
	cact-iactions-list-priv.c							\
	cact-iactions-list-priv.h							\
	cact-iaction-tab.c									\
	cact-iaction-tab.h									\
	cact-iadvanced-tab.c								\
	cact-iadvanced-tab.h								\
	cact-ifolders-tab.c									\
	cact-ifolders-tab.h									\
	cact-icommand-tab.c									\
	cact-icommand-tab.h									\
	cact-iconditions-tab.c								\
	cact-iconditions-tab.h								\
	cact-iprefs.c										\
	cact-iprefs.h										\
	cact-main.c											\
	cact-main-menubar.c									\
	cact-main-menubar.h									\
	cact-main-menubar-edit.c							\
	cact-main-menubar-edit.h							\
	cact-main-menubar-file.c							\
	cact-main-menubar-file.h							\
	cact-main-menubar-help.c							\
	cact-main-menubar-help.h							\
	cact-main-menubar-maintainer.c						\
	cact-main-menubar-maintainer.h						\
	cact-main-menubar-tools.c							\
	cact-main-menubar-tools.h							\
	cact-main-menubar-view.c							\
	cact-main-menubar-view.h							\
	cact-main-statusbar.c								\
	cact-main-statusbar.h								\
	cact-main-tab.c										\
	cact-main-tab.h										\
	cact-main-toolbar.c									\
	cact-main-toolbar.h									\
	cact-main-window.c									\
	cact-main-window.h									\
	cact-preferences-editor.c							\
	cact-preferences-editor.h							\
	cact-providers-list.c								\
	cact-providers-list.h								\
	cact-schemes-list.c									\
	cact-schemes-list.h									\
	cact-sort-buttons.c									\
	cact-sort-buttons.h									\
	cact-tree-model.c									\
	cact-tree-model.h									\
	cact-tree-model-dnd.c								\
	cact-tree-model-dnd.h								\
	cact-tree-model-priv.h								\
	cact-window.c										\
	cact-window.h										\
	$(BUILT_SOURCES)									\
	$(NULL)

cact-marshal.h: cact-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=cact_marshal > $@

cact-marshal.c: cact-marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"cact-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=cact_marshal >> $@

caja_actions_config_tool_LDADD = \
	$(top_builddir)/src/core/libna-core.la				\
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

pkgdata_DATA = \
	caja-actions-config-tool.actions				\
	caja-actions-maintainer.actions					\
	cact-assistant-export.ui							\
	cact-preferences.ui									\
	caja-actions-config-tool.ui						\
	locked.png											\
	transparent.png										\
	$(NULL)

@INTLTOOL_DESKTOP_RULE@

applications_in_files = cact.desktop.in

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
	cact-marshal.list									\
	$(NULL)
