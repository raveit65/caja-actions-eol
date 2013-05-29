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

NA_UTILS_LDADD = \
	$(top_builddir)/src/core/libna-core.la						\
	$(CAJA_ACTIONS_LIBS)									\
	$(NULL)

bin_PROGRAMS = \
	caja-actions-new										\
	caja-actions-print										\
	caja-actions-run										\
	$(NULL)

pkglibexec_PROGRAMS = \
	na-print-schemas											\
	na-set-conf													\
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
	$(NA_UTILS_LDADD)											\
	$(NULL)

caja_actions_print_SOURCES = \
	caja-actions-print.c									\
	console-utils.c												\
	console-utils.h												\
	$(NULL)

caja_actions_print_LDADD = \
	$(NA_UTILS_LDADD)											\
	$(NULL)

BUILT_SOURCES =

if HAVE_GDBUS
BUILT_SOURCES += \
	caja-actions-run-bindings.c								\
	caja-actions-run-bindings.h								\
	$(NULL)

caja-actions-run-bindings.c caja-actions-run-bindings.h: $(top_srcdir)/src/plugin-tracker/na-tracker-gdbus.xml
	gdbus-codegen \
		--interface-prefix org.caja_actions.DBus.Tracker.	\
		--generate-c-code caja-actions-run-bindings			\
		--c-namespace NA_Tracker								\
		--c-generate-object-manager								\
		$<
endif

if HAVE_DBUS_GLIB
BUILT_SOURCES += \
	caja-actions-run-bindings.h								\
	$(NULL)

caja-actions-run-bindings.h: $(top_srcdir)/src/plugin-tracker/na-tracker-dbus-glib.xml
	dbus-binding-tool --mode=glib-client $< > $@
endif

nodist_caja_actions_run_SOURCES = \
	$(BUILT_SOURCES)											\
	$(NULL)

caja_actions_run_SOURCES = \
	caja-actions-run.c										\
	console-utils.c												\
	console-utils.h												\
	$(NULL)

caja_actions_run_LDADD = \
	$(NA_UTILS_LDADD)											\
	$(NULL)

na_print_schemas_SOURCES = \
	na-print-schemas.c											\
	console-utils.c												\
	console-utils.h												\
	$(NULL)

na_print_schemas_LDADD = \
	$(NA_UTILS_LDADD)											\
	$(NULL)

na_set_conf_SOURCES = \
	na-set-conf.c												\
	console-utils.c												\
	console-utils.h												\
	$(NULL)

na_set_conf_LDADD = \
	$(NA_UTILS_LDADD)											\
	$(NULL)

EXTRA_DIST = \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES)											\
	$(NULL)
