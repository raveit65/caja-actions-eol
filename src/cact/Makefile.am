# Caja-Actions
# A Caja extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The GNOME Foundation
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

bin_PROGRAMS = \
	caja-actions-config-tool						\
	$(NULL)

ui_datadir = $(pkgdatadir)/ui

egg_platform_defines = -DEGG_SM_CLIENT_BACKEND_XSMP

AM_CPPFLAGS += \
	-I $(top_srcdir)									\
	-I $(top_srcdir)/src								\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"			\
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_CACT}\"				\
	-DPKGUIDIR=\"$(ui_datadir)\"						\
	$(egg_platform_defines)								\
	$(CAJA_ACTIONS_CFLAGS)							\
	$(NULL)

BUILT_SOURCES = \
	base-marshal.c										\
	base-marshal.h										\
	cact-marshal.c										\
	cact-marshal.h										\
	$(NULL)

caja_actions_config_tool_SOURCES = \
	base-application.c									\
	base-application.h									\
	base-assistant.c									\
	base-assistant.h									\
	base-builder.c										\
	base-builder.h										\
	base-dialog.c										\
	base-dialog.h										\
	base-gtk-utils.c									\
	base-gtk-utils.h									\
	base-isession.c										\
	base-isession.h										\
	base-iunique.c										\
	base-iunique.h										\
	base-keysyms.h										\
	base-window.c										\
	base-window.h										\
	egg-desktop-file.c									\
	egg-desktop-file.h									\
	egg-sm-client.c										\
	egg-sm-client.h										\
	egg-sm-client-private.h								\
	egg-sm-client-xsmp.c								\
	egg-tree-multi-dnd.c								\
	egg-tree-multi-dnd.h								\
	main.c												\
	cact-add-capability-dialog.c						\
	cact-add-capability-dialog.h						\
	cact-add-scheme-dialog.c							\
	cact-add-scheme-dialog.h							\
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
	cact-iaction-tab.c									\
	cact-iaction-tab.h									\
	cact-ibasenames-tab.c								\
	cact-ibasenames-tab.h								\
	cact-icapabilities-tab.c							\
	cact-icapabilities-tab.h							\
	cact-icon-chooser.c									\
	cact-icon-chooser.h									\
	cact-ienvironment-tab.c								\
	cact-ienvironment-tab.h								\
	cact-iexecution-tab.c								\
	cact-iexecution-tab.h								\
	cact-ifolders-tab.c									\
	cact-ifolders-tab.h									\
	cact-icommand-tab.c									\
	cact-icommand-tab.h									\
	cact-imimetypes-tab.c								\
	cact-imimetypes-tab.h								\
	cact-iproperties-tab.c								\
	cact-iproperties-tab.h								\
	cact-ischemes-tab.c									\
	cact-ischemes-tab.h									\
	cact-main-statusbar.c								\
	cact-main-statusbar.h								\
	cact-main-tab.c										\
	cact-main-tab.h										\
	cact-main-toolbar.c									\
	cact-main-toolbar.h									\
	cact-main-window.c									\
	cact-main-window.h									\
	cact-match-list.c									\
	cact-match-list.h									\
	cact-menubar.c										\
	cact-menubar.h										\
	cact-menubar-priv.h									\
	cact-menubar-edit.c									\
	cact-menubar-file.c									\
	cact-menubar-help.c									\
	cact-menubar-maintainer.c							\
	cact-menubar-tools.c								\
	cact-menubar-view.c									\
	cact-preferences-editor.c							\
	cact-preferences-editor.h							\
	cact-providers-list.c								\
	cact-providers-list.h								\
	cact-schemes-list.c									\
	cact-schemes-list.h									\
	cact-sort-buttons.c									\
	cact-sort-buttons.h									\
	cact-tree-ieditable.c								\
	cact-tree-ieditable.h								\
	cact-tree-model.c									\
	cact-tree-model.h									\
	cact-tree-model-priv.h								\
	cact-tree-model-dnd.c								\
	cact-tree-view.c									\
	cact-tree-view.h									\
	$(BUILT_SOURCES)									\
	$(NULL)

base-marshal.h: base-marshal.def $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=base_cclosure_marshal > $@

base-marshal.c: base-marshal.def $(GLIB_GENMARSHAL)
	echo "#include \"base-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=base_cclosure_marshal >> $@

cact-marshal.h: cact-marshal.def $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=cact_cclosure_marshal > $@

cact-marshal.c: cact-marshal.def $(GLIB_GENMARSHAL)
	echo "#include \"cact-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=cact_cclosure_marshal >> $@

caja_actions_config_tool_LDADD = \
	$(top_builddir)/src/core/libna-core.la				\
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

actions_files = \
	caja-actions-config-tool.actions				\
	caja-actions-maintainer.actions					\
	$(NULL)

ui_files = \
	cact-add-capability.ui								\
	cact-add-scheme.ui									\
	cact-assistant-export.ui							\
	cact-assistant-import.ui							\
	cact-icon-chooser.ui								\
	cact-preferences.ui									\
	caja-actions-config-tool.ui						\
	$(NULL)

images_files = \
	locked.png											\
	transparent.png										\
	$(NULL)

ui_data_DATA = \
	$(actions_files)									\
	$(ui_files)											\
	$(images_files)										\
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
	$(ui_data_DATA)										\
	base-marshal.def									\
	cact-marshal.def									\
	$(NULL)
