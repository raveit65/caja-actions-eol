/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
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

#ifndef __CAJA_ACTIONS_API_NA_CORE_UTILS_H__
#define __CAJA_ACTIONS_API_NA_CORE_UTILS_H__

/**
 * SECTION: core-utils
 * @title: Core Misc
 * @short_description: The Core Library Utilities
 * @include: caja-action/na-core-utils.h
 */

#include <glib.h>

G_BEGIN_DECLS

/* boolean manipulation
 */
gboolean na_core_utils_boolean_from_string( const gchar *string );

/* string manipulation
 */
#ifdef NA_ENABLE_DEPRECATED
gchar   *na_core_utils_str_add_prefix( const gchar *prefix, const gchar *str );
#endif
int      na_core_utils_str_collate( const gchar *str1, const gchar *str2 );
gchar   *na_core_utils_str_remove_char( const gchar *string, const gchar *to_remove );
gchar   *na_core_utils_str_remove_suffix( const gchar *string, const gchar *suffix );
void     na_core_utils_str_split_first_word( const gchar *string, gchar **first, gchar **other );
gchar   *na_core_utils_str_subst( const gchar *pattern, const gchar *key, const gchar *subst );

/* some functions to get or set GSList list of strings
 */
void     na_core_utils_slist_add_message( GSList **list, const gchar *format, ... );
GSList  *na_core_utils_slist_duplicate( GSList *slist );
void     na_core_utils_slist_dump( const gchar *prefix, GSList *list );
GSList  *na_core_utils_slist_from_array( const gchar **str_array );
GSList  *na_core_utils_slist_from_split( const gchar *text, const gchar *separator );
gchar   *na_core_utils_slist_join_at_end( GSList *slist, const gchar *link );
GSList  *na_core_utils_slist_remove_ascii( GSList *slist, const gchar *text );
GSList  *na_core_utils_slist_remove_utf8( GSList *slist, const gchar *text );
gchar  **na_core_utils_slist_to_array( GSList *slist );
gchar   *na_core_utils_slist_to_text( GSList *slist );
GSList  *na_core_utils_slist_setup_element( GSList *list, const gchar *element, gboolean set );
guint    na_core_utils_slist_count( GSList *list, const gchar *str );
gboolean na_core_utils_slist_find_negated( GSList *list, const gchar *str );
gboolean na_core_utils_slist_are_equal( GSList *a, GSList *b );
void     na_core_utils_slist_free( GSList *slist );

/* some functions for GString manipulations.
 */
gchar   *na_core_utils_gstring_joinv( const gchar *start, const gchar *separator, gchar **list );

/* selection count
 */
void     na_core_utils_selcount_get_ope_int( const gchar *selection_count, gchar **ope, gchar **uint );

/* directory management
 */
gboolean na_core_utils_dir_is_writable_path( const gchar *path );
gboolean na_core_utils_dir_is_writable_uri ( const gchar *uri );
void     na_core_utils_dir_list_perms      ( const gchar *path, const gchar *message );
void     na_core_utils_dir_split_ext       ( const gchar *string, gchar **first, gchar **ext );

/* file management
 */
gboolean na_core_utils_file_delete       ( const gchar *path );
gboolean na_core_utils_file_exists       ( const gchar *uri );
gboolean na_core_utils_file_is_loadable  ( const gchar *uri );
void     na_core_utils_file_list_perms   ( const gchar *path, const gchar *message );
gchar   *na_core_utils_file_load_from_uri( const gchar *uri, gsize *length );

/* miscellaneous
 */
void     na_core_utils_print_version( void );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_CORE_UTILS_H__ */
