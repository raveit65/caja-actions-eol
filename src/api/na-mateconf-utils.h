/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The MATE Foundation
 * Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
 *
 * Caja-Actions is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General  Public  License  as
 * published by the Free Software Foundation; either  version  2  of
 * the License, or (at your option) any later version.
 *
 * Caja-Actions is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even  the  implied  warranty  of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  the  GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public  License
 * along with Caja-Actions; see the file  COPYING.  If  not,  see
 * <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Frederic Ruaudel <grumz@grumz.net>
 *   Rodrigo Moya <rodrigo@mate-db.org>
 *   Pierre Wieser <pwieser@trychlos.org>
 *   ... and many others (see AUTHORS)
 */

#ifndef __CAJA_ACTIONS_API_NA_MATECONF_UTILS_H__
#define __CAJA_ACTIONS_API_NA_MATECONF_UTILS_H__

#ifdef HAVE_MATECONF
/**
 * SECTION: mateconf-utils
 * @title: MateConf Misc
 * @short_description: The MateConf Library Utilities
 * @include: runtime/na-mateconf-utils.h
 *
 * Starting with Caja-Actions 3.1.0, MateConf, whether it is used as a
 * preference storage subsystem or as an I/O provider, is deprecated.
 */

#include <mateconf/mateconf-client.h>

G_BEGIN_DECLS

GSList  *na_mateconf_utils_get_subdirs( MateConfClient *mateconf, const gchar *path );
void     na_mateconf_utils_free_subdirs( GSList *subdirs );

gboolean na_mateconf_utils_has_entry  ( GSList *entries, const gchar *entry );
GSList  *na_mateconf_utils_get_entries( MateConfClient *mateconf, const gchar *path );
gboolean na_mateconf_utils_get_bool_from_entries       ( GSList *entries, const gchar *entry, gboolean *value );
gboolean na_mateconf_utils_get_string_from_entries     ( GSList *entries, const gchar *entry, gchar **value );
gboolean na_mateconf_utils_get_string_list_from_entries( GSList *entries, const gchar *entry, GSList **value );
void     na_mateconf_utils_dump_entries                ( GSList *entries );
void     na_mateconf_utils_free_entries                ( GSList *entries );

gboolean na_mateconf_utils_read_bool        ( MateConfClient *mateconf, const gchar *path, gboolean use_schema, gboolean default_value );
gint     na_mateconf_utils_read_int         ( MateConfClient *mateconf, const gchar *path, gboolean use_schema, gint default_value );
gchar   *na_mateconf_utils_read_string      ( MateConfClient *mateconf, const gchar *path, gboolean use_schema, const gchar *default_value );
GSList  *na_mateconf_utils_read_string_list ( MateConfClient *mateconf, const gchar *path );

/* Writing in MateConf is deprecated since 3.1.0
 */
#ifdef NA_ENABLE_DEPRECATED
gboolean na_mateconf_utils_write_bool       ( MateConfClient *mateconf, const gchar *path, gboolean value, gchar **message );
gboolean na_mateconf_utils_write_int        ( MateConfClient *mateconf, const gchar *path, gint value, gchar **message );
gboolean na_mateconf_utils_write_string     ( MateConfClient *mateconf, const gchar *path, const gchar *value, gchar **message );
gboolean na_mateconf_utils_write_string_list( MateConfClient *mateconf, const gchar *path, GSList *value, gchar **message );
gboolean na_mateconf_utils_remove_entry     ( MateConfClient *mateconf, const gchar *path, gchar **message );
GSList  *na_mateconf_utils_slist_from_string( const gchar *value );
gchar   *na_mateconf_utils_slist_to_string  ( GSList *slist );
#endif /* NA_ENABLE_DEPRECATED */

G_END_DECLS

#endif /* HAVE_MATECONF */
#endif /* __CAJA_ACTIONS_API_NA_MATECONF_UTILS_H__ */
