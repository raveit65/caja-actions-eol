/*
 * Caja Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The MATE Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009, 2010 Pierre Wieser and others (see AUTHORS)
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this Library; see the file COPYING.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307, USA.
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

#include <string.h>

#include <libcaja-extension/caja-file-info.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include "na-mate-vfs-uri.h"
#include "na-selected-info.h"

/* private interface data
 */
struct NAIContextInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static gboolean st_initialized = FALSE;
static gboolean st_finalized   = FALSE;

static GType    register_type( void );
static void     interface_base_init( NAIContextInterface *klass );
static void     interface_base_finalize( NAIContextInterface *klass );

static gboolean v_is_candidate( NAIContext *object, guint target, GList *selection );

static gboolean is_target_location_candidate( const NAIContext *object, NASelectedInfo *current_folder );
static gboolean is_target_toolbar_candidate( const NAIContext *object, NASelectedInfo *current_folder );
static gboolean is_current_folder_inside( const NAIContext *object, NASelectedInfo *current_folder );
static gboolean is_target_selection_candidate( const NAIContext *object, GList *files );
static gboolean is_valid_basenames( const NAIContext *object );
static gboolean is_valid_mimetypes( const NAIContext *object );
static gboolean is_valid_isfiledir( const NAIContext *object );
static gboolean is_valid_schemes( const NAIContext *object );
static gboolean is_valid_folders( const NAIContext *object );

static gboolean validate_schemes( GSList *object_schemes, NASelectedInfo *iter );

/**
 * na_icontext_get_type:
 *
 * Returns: the #GType type of this interface.
 */
GType
na_icontext_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

/*
 * na_icontext_register_type:
 *
 * Registers this interface.
 */
static GType
register_type( void )
{
	static const gchar *thisfn = "na_icontext_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIContextInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIContext", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIContextInterface *klass )
{
	static const gchar *thisfn = "na_icontext_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass%p (%s)", thisfn, ( void * ) klass, G_OBJECT_CLASS_NAME( klass ));

		klass->private = g_new0( NAIContextInterfacePrivate, 1 );

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NAIContextInterface *klass )
{
	static const gchar *thisfn = "na_icontext_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
}

/**
 * na_icontext_is_candidate:
 * @object: a #NAIContext to be checked.
 * @target: the current target.
 * @files: the currently selected items, as a #GList of #NASelectedInfo items.
 *
 * Determines if the given object may be candidate to be displayed in
 * the Caja context menu, depending of the list of currently selected
 * items.
 *
 * Returns: %TRUE if this object succeeds to all tests and is so a
 * valid candidate to be displayed in Caja context menu, %FALSE
 * else.
 */
gboolean
na_icontext_is_candidate( const NAIContext *object, guint target, GList *files )
{
	gboolean is_candidate;

	g_return_val_if_fail( NA_IS_ICONTEXT( object ), FALSE );

	is_candidate = v_is_candidate( NA_ICONTEXT( object ), target, files );

	switch( target ){
		case ITEM_TARGET_LOCATION:
			is_candidate = is_target_location_candidate( object, ( NASelectedInfo * ) files->data );
			break;

		case ITEM_TARGET_TOOLBAR:
			is_candidate = is_target_toolbar_candidate( object, ( NASelectedInfo * ) files->data );
			break;

		case ITEM_TARGET_SELECTION:
		default:
			is_candidate = is_target_selection_candidate( object, files );
	}

	return( is_candidate );
}

/**
 * na_icontext_is_valid:
 * @profile: the #NAObjectProfile to be checked.
 *
 * Returns: %TRUE if this profile is valid, %FALSE else.
 *
 * This function is part of NAIDuplicable::check_status() and is called
 * by NAIDuplicable objects which also implement NAIContext
 * interface. It so doesn't make sense of asking the object for its
 * validity status as it has already been checked before calling the
 * function.
 */
gboolean
na_icontext_is_valid( const NAIContext *object )
{
	gboolean is_valid;

	g_return_val_if_fail( NA_IS_ICONTEXT( object ), FALSE );

	is_valid =
		is_valid_basenames( object ) &&
		is_valid_mimetypes( object ) &&
		is_valid_isfiledir( object ) &&
		is_valid_schemes( object ) &&
		is_valid_folders( object );

	return( is_valid );
}

/**
 * na_icontext_set_scheme:
 * @profile: the #NAIContext to be updated.
 * @scheme: name of the scheme.
 * @selected: whether this scheme is candidate to this profile.
 *
 * Sets the status of a scheme relative to this profile.
 */
void
na_icontext_set_scheme( NAIContext *profile, const gchar *scheme, gboolean selected )
{
	/*static const gchar *thisfn = "na_icontext_set_scheme";*/
	gboolean exist;
	GSList *schemes;

	g_return_if_fail( NA_IS_ICONTEXT( profile ));

	schemes = na_object_get_schemes( profile );
	exist = na_core_utils_slist_find( schemes, scheme );
	/*g_debug( "%s: scheme=%s exist=%s", thisfn, scheme, exist ? "True":"False" );*/

	if( selected && !exist ){
		schemes = g_slist_prepend( schemes, g_strdup( scheme ));
	}
	if( !selected && exist ){
		schemes = na_core_utils_slist_remove_ascii( schemes, scheme );
	}
	na_object_set_schemes( profile, schemes );
	na_core_utils_slist_free( schemes );
}

/**
 * na_icontext_replace_folder:
 * @profile: the #NAIContext to be updated.
 * @old: the old uri.
 * @new: the new uri.
 *
 * Replaces the @old URI by the @new one.
 */
void
na_icontext_replace_folder( NAIContext *profile, const gchar *old, const gchar *new )
{
	GSList *folders;

	g_return_if_fail( NA_IS_ICONTEXT( profile ));

	folders = na_object_get_folders( profile );
	folders = na_core_utils_slist_remove_utf8( folders, old );
	folders = g_slist_append( folders, ( gpointer ) g_strdup( new ));
	na_object_set_folders( profile, folders );
	na_core_utils_slist_free( folders );
}

static gboolean
v_is_candidate( NAIContext *object, guint target, GList *selection )
{
	gboolean is_candidate;

	is_candidate = TRUE;

	if( NA_ICONTEXT_GET_INTERFACE( object )->is_candidate ){
		is_candidate = NA_ICONTEXT_GET_INTERFACE( object )->is_candidate( object, target, selection );
	}

	return( is_candidate );
}

static gboolean
is_target_location_candidate( const NAIContext *object, NASelectedInfo *current_folder )
{
	gboolean is_candidate;

	is_candidate = is_current_folder_inside( object, current_folder );

	return( is_candidate );
}

static gboolean
is_target_toolbar_candidate( const NAIContext *object, NASelectedInfo *current_folder )
{
	gboolean is_candidate;

	is_candidate = is_current_folder_inside( object, current_folder );

	return( is_candidate );
}

static gboolean
is_current_folder_inside( const NAIContext *object, NASelectedInfo *current_folder )
{
	gboolean is_inside;
	GSList *folders, *ifold;
	const gchar *path;
	gchar *current_folder_path;

	is_inside = FALSE;
	current_folder_path = na_selected_info_get_path( current_folder );
	folders = na_object_get_folders( object );

	for( ifold = folders ; ifold && !is_inside ; ifold = ifold->next ){
		path = ( const gchar * ) ifold->data;
		if( path && g_utf8_strlen( path, -1 )){
			is_inside = g_str_has_prefix( current_folder_path, path );
			g_debug( "na_object_object_is_current_folder_inside: current_folder_path=%s, path=%s, is_inside=%s", current_folder_path, path, is_inside ? "True":"False" );
		}
	}

	na_core_utils_slist_free( folders );
	g_free( current_folder_path );

	return( is_inside );
}

static gboolean
is_target_selection_candidate( const NAIContext *object, GList *files )
{
	gboolean retv = FALSE;
	GSList *basenames, *mimetypes, *schemes;
	gboolean matchcase, multiple, isdir, isfile;
	gboolean test_multiple_file = FALSE;
	gboolean test_file_type = FALSE;
	gboolean test_scheme = FALSE;
	gboolean test_basename = FALSE;
	gboolean test_mimetype = FALSE;
	GList* glob_patterns = NULL;
	GList* glob_mime_patterns = NULL;
	GSList* iter;
	GList* iter1;
	GList* iter2;
	guint dir_count = 0;
	guint file_count = 0;
	guint total_count = 0;
	guint scheme_ok_count = 0;
	guint glob_ok_count = 0;
	guint mime_glob_ok_count = 0;
	gboolean basename_match_ok = FALSE;
	gboolean mimetype_match_ok = FALSE;
	gchar *tmp_pattern, *tmp_filename, *tmp_filename2, *tmp_mimetype, *tmp_mimetype2;

	basenames = na_object_get_basenames( object );
	matchcase = na_object_is_matchcase( object );
	multiple = na_object_is_multiple( object );
	isdir = na_object_is_dir( object );
	isfile = na_object_is_file( object );
	mimetypes = na_object_get_mimetypes( object );
	schemes = na_object_get_schemes( object );

	if( basenames && basenames->next != NULL &&
			g_ascii_strcasecmp(( gchar * )( basenames->data ), "*" ) == 0 ){
		/* if the only pattern is '*' then all files will match, so it
		 * is not necessary to make the test for each of them
		 */
		test_basename = TRUE;

	} else {
		for (iter = basenames ; iter ; iter = iter->next ){

			tmp_pattern = ( gchar * ) iter->data;
			if( !matchcase ){
				/* --> if case-insensitive asked, lower all the string
				 * since the pattern matching function don't manage it
				 * itself.
				 */
				tmp_pattern = g_ascii_strdown(( gchar * ) iter->data, strlen(( gchar * ) iter->data ));
			}

			glob_patterns = g_list_append( glob_patterns, g_pattern_spec_new( tmp_pattern ));

			if( !matchcase ){
				g_free( tmp_pattern );
			}
		}
	}

	if( mimetypes && mimetypes->next != NULL &&
			( g_ascii_strcasecmp(( gchar * )( mimetypes->data ), "*" ) == 0 ||
			  g_ascii_strcasecmp(( gchar * )( mimetypes->data), "*/*") == 0 )){
		/* if the only pattern is '*' or * / * then all mimetypes will
		 * match, so it is not necessary to make the test for each of them
		 */
		test_mimetype = TRUE;

	} else {
		for( iter = mimetypes ; iter ; iter = iter->next ){
			glob_mime_patterns = g_list_append( glob_mime_patterns, g_pattern_spec_new(( gchar * ) iter->data ));
		}
	}

	for( iter1 = files ; iter1 ; iter1 = iter1->next ){

		tmp_filename = na_selected_info_get_path( NA_SELECTED_INFO( iter1->data ));
		g_debug( "na_icontext_is_target_selection_candidate: tmp_filename=%s", tmp_filename );

		if( tmp_filename ){
			tmp_mimetype = na_selected_info_get_mime_type( NA_SELECTED_INFO( iter1->data ));
			g_debug( "na_icontext_is_target_selection_candidate: tmp_mimetype=%s", tmp_mimetype );

			if( !matchcase ){
				/* --> if case-insensitive asked, lower all the string
				 * since the pattern matching function don't manage it
				 * itself.
				 */
				tmp_filename2 = g_ascii_strdown( tmp_filename, strlen( tmp_filename ));
				g_free( tmp_filename );
				tmp_filename = tmp_filename2;
				g_debug( "na_icontext_is_target_selection_candidate: tmp_filename=%s", tmp_filename );
			}

			/* --> for the moment we deal with all mimetypes case-insensitively
			 * note that a symlink to a directory has a 'inode/directory' mimetype
			 * and, in general, a symlink to a target has the target's mimetype
			 */
			tmp_mimetype2 = g_ascii_strdown( tmp_mimetype, strlen( tmp_mimetype ));
			g_free( tmp_mimetype );
			tmp_mimetype = tmp_mimetype2;
			g_debug( "mimetype=%s", tmp_mimetype );

			if( na_selected_info_is_directory( NA_SELECTED_INFO( iter1->data ))){
				dir_count++;
			} else {
				file_count++;
			}

			scheme_ok_count += validate_schemes( schemes, NA_SELECTED_INFO( iter1->data ));

			if( !test_basename ){ /* if it is already ok, skip the test to improve performance */
				basename_match_ok = FALSE;
				iter2 = glob_patterns;
				while( iter2 && !basename_match_ok ){
					if( g_pattern_match_string(( GPatternSpec * ) iter2->data, tmp_filename )){
						basename_match_ok = TRUE;
					}
					iter2 = iter2->next;
				}

				if( basename_match_ok ){
					glob_ok_count++;
				}
			}

			if( !test_mimetype ){ /* if it is already ok, skip the test to improve performance */
				mimetype_match_ok = FALSE;
				iter2 = glob_mime_patterns;
				while( iter2 && !mimetype_match_ok ){
					if (g_pattern_match_string(( GPatternSpec * ) iter2->data, tmp_mimetype )){
						mimetype_match_ok = TRUE;
					}
					iter2 = iter2->next;
				}

				if( mimetype_match_ok ){
					mime_glob_ok_count++;
				}
			}

			g_free( tmp_mimetype );
			g_free( tmp_filename );
		}

		total_count++;
	}

	if(( files != NULL ) && ( files->next == NULL ) && ( !multiple )){
		test_multiple_file = TRUE;

	} else if( multiple ){
		test_multiple_file = TRUE;
	}

	if( isdir && isfile ){
		if( dir_count > 0 || file_count > 0 ){
			test_file_type = TRUE;
		}
	} else if( isdir && !isfile ){
		if( file_count == 0 ){
			test_file_type = TRUE;
		}
	} else if( !isdir && isfile ){
		if( dir_count == 0 ){
			test_file_type = TRUE;
		}
	}

	if( scheme_ok_count == total_count ){
		test_scheme = TRUE;
	}

	if( !test_basename ){ /* if not already tested */
		if( glob_ok_count == total_count ){
			test_basename = TRUE;
		}
	}

	if( !test_mimetype ){ /* if not already tested */
		if( mime_glob_ok_count == total_count ){
			test_mimetype = TRUE;
		}
	}

	if( test_basename && test_mimetype && test_file_type && test_scheme && test_multiple_file ){
		retv = TRUE;
	}

	g_list_foreach (glob_patterns, (GFunc) g_pattern_spec_free, NULL);
	g_list_free (glob_patterns);
	g_list_foreach (glob_mime_patterns, (GFunc) g_pattern_spec_free, NULL);
	g_list_free (glob_mime_patterns);
	na_core_utils_slist_free( schemes );
	na_core_utils_slist_free( mimetypes );
	na_core_utils_slist_free( basenames );

	return retv;
}

static gboolean
is_valid_basenames( const NAIContext *object )
{
	gboolean valid;
	GSList *basenames;

	basenames = na_object_get_basenames( object );
	valid = basenames && g_slist_length( basenames ) > 0;
	na_core_utils_slist_free( basenames );

	if( !valid ){
		na_object_debug_invalid( object, "basenames" );
	}

	return( valid );
}

static gboolean
is_valid_mimetypes( const NAIContext *object )
{
	gboolean valid;
	GSList *mimetypes;

	mimetypes = na_object_get_mimetypes( object );
	valid = mimetypes && g_slist_length( mimetypes ) > 0;
	na_core_utils_slist_free( mimetypes );

	if( !valid ){
		na_object_debug_invalid( object, "mimetypes" );
	}

	return( valid );
}

static gboolean
is_valid_isfiledir( const NAIContext *object )
{
	gboolean valid;
	gboolean isfile, isdir;

	isfile = na_object_is_file( object );
	isdir = na_object_is_dir( object );

	valid = isfile || isdir;

	if( !valid ){
		na_object_debug_invalid( object, "isfiledir" );
	}

	return( valid );
}

static gboolean
is_valid_schemes( const NAIContext *object )
{
	gboolean valid;
	GSList *schemes;

	schemes = na_object_get_schemes( object );
	valid = schemes && g_slist_length( schemes ) > 0;
	na_core_utils_slist_free( schemes );

	if( !valid ){
		na_object_debug_invalid( object, "schemes" );
	}

	return( valid );
}

static gboolean
is_valid_folders( const NAIContext *object )
{
	gboolean valid;
	GSList *folders;

	folders = na_object_get_folders( object );
	valid = folders && g_slist_length( folders ) > 0;
	na_core_utils_slist_free( folders );

	if( !valid ){
		na_object_debug_invalid( object, "folders" );
	}

	return( valid );
}

static gboolean
validate_schemes( GSList *object_schemes, NASelectedInfo *nfi )
{
	gboolean is_ok;
	GSList* iter;
	gchar *scheme;

	is_ok = FALSE;

	for( iter = object_schemes ; iter && !is_ok ; iter = iter->next ){
		scheme = na_selected_info_get_uri_scheme( nfi );

		if( g_ascii_strncasecmp( scheme, ( gchar * ) iter->data, strlen(( gchar * ) iter->data )) == 0 ){
			is_ok = TRUE;
		}

		g_free( scheme );
	}

	return( is_ok );
}
