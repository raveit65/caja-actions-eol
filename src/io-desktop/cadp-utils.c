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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>

#include <api/na-core-utils.h>

#include "cadp-desktop-provider.h"
#include "cadp-utils.h"

/**
 * cadp_utils_gslist_remove_from:
 * @list: the #GSList from which remove the @string.
 * @string: the string to be removed.
 *
 * Removes a @string from a string list, then frees the removed @string.
 */
GSList *
cadp_utils_gslist_remove_from( GSList *list, const gchar *string )
{
	GSList *is;

	for( is = list ; is ; is = is->next ){
		const gchar *istr = ( const gchar * ) is->data;
		if( !na_core_utils_str_collate( string, istr )){
			g_free( is->data );
			list = g_slist_delete_link( list, is );
			break;
		}
	}

	return( list );
}

/**
 *
 */
gboolean
cadp_utils_uri_delete( const gchar *uri )
{
	gboolean deleted;
	gchar *scheme;
	gchar *path;

	deleted = FALSE;
	scheme = g_uri_parse_scheme( uri );

	if( !strcmp( scheme, "file" )){
		path = g_filename_from_uri( uri, NULL, NULL );

		if( path ){
			deleted = na_core_utils_file_delete( path );
			g_free( path );
		}
	}

	g_free( scheme );

	return( deleted );
}

/**
 * cadp_utils_uri_is_writable:
 * @uri: the URI of the file to be tested.
 *
 * Returns: %TRUE if the file is writable, %FALSE else.
 *
 * Please note that this type of test is subject to race conditions,
 * as the file may become unwritable after a successful test,
 * but before the caller has been able to actually write into it.
 *
 * There is no "super-test". Just try...
 */
gboolean
cadp_utils_uri_is_writable( const gchar *uri )
{
	static const gchar *thisfn = "cadp_utils_uri_is_writable";
	GFile *file;
	GError *error = NULL;
	GFileInfo *info;
	gboolean writable;

	if( !uri || !g_utf8_strlen( uri, -1 )){
		return( FALSE );
	}

	file = g_file_new_for_uri( uri );
	info = g_file_query_info( file,
			G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
			G_FILE_QUERY_INFO_NONE, NULL, &error );

	if( error ){
		g_warning( "%s: g_file_query_info error: %s", thisfn, error->message );
		g_error_free( error );
		g_object_unref( file );
		return( FALSE );
	}

	writable = g_file_info_get_attribute_boolean( info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE );
	if( !writable ){
		g_debug( "%s: %s is not writable", thisfn, uri );
	}

	g_object_unref( info );

	return( writable );
}
