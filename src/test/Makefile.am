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

noinst_PROGRAMS = \
	test-reader											\
	test-iface											\
	test-include										\
	test-parse-uris										\
	test-virtuals										\
	test-virtuals-without-test							\
	$(NULL)

AM_CPPFLAGS += \
	-I $(top_srcdir)									\
	-I $(top_srcdir)/src								\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"			\
	-DG_LOG_DOMAIN=\"${NA_LOGDOMAIN_TEST}\"				\
	$(CAJA_ACTIONS_CFLAGS)							\
	$(NULL)

test_reader_SOURCES = \
	test-reader.c										\
	$(NULL)

test_reader_LDADD = \
	$(top_builddir)/src/core/libna-core.la				\
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

test_iface_SOURCES = \
	test-iface.c										\
	test-iface-iface.c									\
	test-iface-iface.h									\
	test-iface-base.c									\
	test-iface-base.h									\
	test-iface-derived.c								\
	test-iface-derived.h								\
	$(NULL)

test_iface_LDADD = \
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

test_include_SOURCES = \
	test-include.c										\
	$(NULL)

test_include_LDADD = \
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

test_parse_uris_SOURCES = \
	test-parse-uris.c									\
	$(NULL)

test_parse_uris_LDADD = \
	$(top_builddir)/src/core/libna-core.la				\
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

test_virtuals_SOURCES = \
	test-virtuals.c										\
	$(NULL)

test_virtuals_LDADD = \
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)

test_virtuals_without_test_SOURCES = \
	test-virtuals-without-test.c						\
	$(NULL)

test_virtuals_without_test_LDADD = \
	$(CAJA_ACTIONS_LIBS)							\
	$(NULL)
