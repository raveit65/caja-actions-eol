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

#include <string.h>

#include <api/na-data-def.h>
#include <api/na-data-types.h>
#include <api/na-ifactory-provider.h>
#include <api/na-iio-provider.h>
#include <api/na-object-api.h>
#include <api/na-core-utils.h>
#include <api/na-mateconf-utils.h>

#include "nagp-mateconf-provider.h"
#include "nagp-keys.h"
#include "nagp-reader.h"

typedef struct {
	gchar        *path;
	GSList       *entries;
	NAObjectItem *parent;
}
	ReaderData;

static NAObjectItem *read_item( NagpMateConfProvider *provider, const gchar *path, GSList **messages );

static void          read_start_profile_attach_profile( const NAIFactoryProvider *provider, NAObjectProfile *profile, ReaderData *data, GSList **messages );

static gboolean      read_done_item_is_writable( const NAIFactoryProvider *provider, NAObjectItem *item, ReaderData *data, GSList **messages );
static void          read_done_action_read_profiles( const NAIFactoryProvider *provider, NAObjectAction *action, ReaderData *data, GSList **messages );
static void          read_done_action_load_profile( const NAIFactoryProvider *provider, ReaderData *data, const gchar *path, GSList **messages );

static NADataBoxed  *get_boxed_from_path( const NagpMateConfProvider *provider, const gchar *path, ReaderData *reader_data, const NADataDef *def );
static gboolean      is_key_writable( NagpMateConfProvider *mateconf, const gchar *key );

/*
 * nagp_iio_provider_read_items:
 *
 * Note that whatever be the version of the read action, it will be
 * stored as a #NAObjectAction and its set of #NAObjectProfile of the same,
 * latest, version of these classes.
 */
GList *
nagp_iio_provider_read_items( const NAIIOProvider *provider, GSList **messages )
{
	static const gchar *thisfn = "nagp_reader_nagp_iio_provider_read_items";
	NagpMateConfProvider *self;
	GList *items_list = NULL;
	GSList *listpath, *ip;
	NAObjectItem *item;

	g_debug( "%s: provider=%p, messages=%p", thisfn, ( void * ) provider, ( void * ) messages );

	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), NULL );
	g_return_val_if_fail( NAGP_IS_MATECONF_PROVIDER( provider ), NULL );
	self = NAGP_MATECONF_PROVIDER( provider );

	if( !self->private->dispose_has_run ){

		listpath = na_mateconf_utils_get_subdirs( self->private->mateconf, NAGP_CONFIGURATIONS_PATH );

		for( ip = listpath ; ip ; ip = ip->next ){

			item = read_item( self, ( const gchar * ) ip->data, messages );
			if( item ){
				items_list = g_list_prepend( items_list, item );
				na_object_dump( item );
			}
		}

		na_mateconf_utils_free_subdirs( listpath );
	}

	g_debug( "%s: count=%d", thisfn, g_list_length( items_list ));
	return( items_list );
}

/*
 * path is here the full path to an item
 */
static NAObjectItem *
read_item( NagpMateConfProvider *provider, const gchar *path, GSList **messages )
{
	static const gchar *thisfn = "nagp_reader_read_item";
	NAObjectItem *item;
	gchar *full_path;
	gchar *type;
	gchar *id;
	ReaderData *data;

	g_debug( "%s: provider=%p, path=%s", thisfn, ( void * ) provider, path );
	g_return_val_if_fail( NAGP_IS_MATECONF_PROVIDER( provider ), NULL );
	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), NULL );
	g_return_val_if_fail( !provider->private->dispose_has_run, NULL );

	full_path = mateconf_concat_dir_and_key( path, NAGP_ENTRY_TYPE );
	type = na_mateconf_utils_read_string( provider->private->mateconf, full_path, TRUE, NAGP_VALUE_TYPE_ACTION );
	g_free( full_path );
	item = NULL;

	/* an item may have 'Action' or 'Menu' type; defaults to Action
	 */
	if( !type || !strlen( type ) || !strcmp( type, NAGP_VALUE_TYPE_ACTION )){
		item = NA_OBJECT_ITEM( na_object_action_new());

	} else if( !strcmp( type, NAGP_VALUE_TYPE_MENU )){
		item = NA_OBJECT_ITEM( na_object_menu_new());

	} else {
		g_warning( "%s: unknown type '%s' at %s", thisfn, type, path );
	}

	g_free( type );

	if( item ){
		id = g_path_get_basename( path );
		na_object_set_id( item, id );
		g_free( id );

		data = g_new0( ReaderData, 1 );
		data->path = ( gchar * ) path;
		data->entries = na_mateconf_utils_get_entries( provider->private->mateconf, path );
		na_mateconf_utils_dump_entries( data->entries );

		na_ifactory_provider_read_item(
				NA_IFACTORY_PROVIDER( provider ),
				data,
				NA_IFACTORY_OBJECT( item ),
				messages );

		na_mateconf_utils_free_entries( data->entries );
		g_free( data );
	}

	return( item );
}

void
nagp_reader_read_start( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, GSList **messages  )
{
	static const gchar *thisfn = "nagp_reader_read_start";

	g_return_if_fail( NA_IS_IFACTORY_PROVIDER( provider ));
	g_return_if_fail( NAGP_IS_MATECONF_PROVIDER( provider ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NAGP_MATECONF_PROVIDER( provider )->private->dispose_has_run ){

		g_debug( "%s: provider=%p (%s), reader_data=%p, object=%p (%s), messages=%p",
				thisfn,
				( void * ) provider, G_OBJECT_TYPE_NAME( provider ),
				( void * ) reader_data,
				( void * ) object, G_OBJECT_TYPE_NAME( object ),
				( void * ) messages );

		if( NA_IS_OBJECT_PROFILE( object )){
			read_start_profile_attach_profile( provider, NA_OBJECT_PROFILE( object ), ( ReaderData * ) reader_data, messages );
		}
	}
}

static void
read_start_profile_attach_profile( const NAIFactoryProvider *provider, NAObjectProfile *profile, ReaderData *data, GSList **messages )
{
	na_object_attach_profile( data->parent, profile );
}

NADataBoxed *
nagp_reader_read_data( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, const NADataDef *def, GSList **messages )
{
	static const gchar *thisfn = "nagp_reader_read_data";
	NADataBoxed *boxed;

	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( provider ), NULL );
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	/*g_debug( "%s: reader_data=%p, object=%p (%s), data=%s",
			thisfn,
			( void * ) reader_data,
			( void * ) object, G_OBJECT_TYPE_NAME( object ),
			def->name );*/

	if( !def->mateconf_entry || !strlen( def->mateconf_entry )){
		g_warning( "%s: MateConf entry is not set for NADataDef %s", thisfn, def->name );
		return( NULL );
	}

	boxed = get_boxed_from_path(
			NAGP_MATECONF_PROVIDER( provider ), (( ReaderData * ) reader_data )->path, reader_data, def );

	return( boxed );
}

void
nagp_reader_read_done( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, GSList **messages  )
{
	static const gchar *thisfn = "nagp_reader_read_done";
	gboolean writable;

	g_return_if_fail( NA_IS_IFACTORY_PROVIDER( provider ));
	g_return_if_fail( NAGP_IS_MATECONF_PROVIDER( provider ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NAGP_MATECONF_PROVIDER( provider )->private->dispose_has_run ){

		g_debug( "%s: provider=%p (%s), reader_data=%p, object=%p (%s), messages=%p",
				thisfn,
				( void * ) provider, G_OBJECT_TYPE_NAME( provider ),
				( void * ) reader_data,
				( void * ) object, G_OBJECT_TYPE_NAME( object ),
				( void * ) messages );

		if( NA_IS_OBJECT_ITEM( object )){
			writable = read_done_item_is_writable( provider, NA_OBJECT_ITEM( object ), ( ReaderData * ) reader_data, messages );
			na_object_set_readonly( object, !writable );
		}

		if( NA_IS_OBJECT_ACTION( object )){
			read_done_action_read_profiles( provider, NA_OBJECT_ACTION( object ), ( ReaderData * ) reader_data, messages );
		}

		g_debug( "%s: quitting for %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
	}
}

static gboolean
read_done_item_is_writable( const NAIFactoryProvider *provider, NAObjectItem *item, ReaderData *data, GSList **messages )
{
	GSList *ie;
	gboolean writable;
	MateConfEntry *mateconf_entry;
	const gchar *key;

	/* check for writability of this item
	 * item is writable if and only if all entries are themselves writable
	 */
	writable = TRUE;
	for( ie = data->entries ; ie && writable ; ie = ie->next ){
		mateconf_entry = ( MateConfEntry * ) ie->data;
		key = mateconf_entry_get_key( mateconf_entry );
		writable = is_key_writable( NAGP_MATECONF_PROVIDER( provider ), key );
	}

	g_debug( "nagp_reader_read_done_item: writable=%s", writable ? "True":"False" );
	return( writable );
}

static void
read_done_action_read_profiles( const NAIFactoryProvider *provider, NAObjectAction *action, ReaderData *data, GSList **messages )
{
	static const gchar *thisfn = "nagp_reader_read_done_action_read_profiles";
	GSList *order;
	GSList *list_profiles;
	GSList *ip;
	gchar *profile_id;
	gchar *profile_path;
	NAObjectId *found;
	NAObjectProfile *profile;

	data->parent = NA_OBJECT_ITEM( action );
	order = na_object_get_items_slist( action );
	list_profiles = na_mateconf_utils_get_subdirs( NAGP_MATECONF_PROVIDER( provider )->private->mateconf, data->path );

	/* read profiles in the specified order
	 * as a protection against bugs in CACT, we check that profile has not
	 * already been loaded
	 */
	for( ip = order ; ip ; ip = ip->next ){
		profile_id = ( gchar * ) ip->data;
		found = na_object_get_item( action, profile_id );
		if( !found ){
			g_debug( "nagp_reader_read_done_action: loading profile=%s", profile_id );
			profile_path = mateconf_concat_dir_and_key( data->path, profile_id );
			read_done_action_load_profile( provider, data, profile_path, messages );
			g_free( profile_path );
		}
	}

	/* append other profiles
	 * this is mandatory for pre-2.29 actions which introduced order of profiles
	 */
	for( ip = list_profiles ; ip ; ip = ip->next ){
		profile_id = g_path_get_basename(( const gchar * ) ip->data );
		found = na_object_get_item( action, profile_id );
		if( !found ){
			g_debug( "nagp_reader_read_done_action: loading profile=%s", profile_id );
			read_done_action_load_profile( provider, data, ( const gchar * ) ip->data, messages );
		}
		g_free( profile_id );
	}

	/* make sure we have at least one profile
	 */
	if( !na_object_get_items_count( action )){
		g_warning( "%s: no profile found in MateConf backend", thisfn );
		profile = na_object_profile_new_with_defaults();
		na_object_attach_profile( action, profile );
	}
}

static void
read_done_action_load_profile( const NAIFactoryProvider *provider, ReaderData *data, const gchar *path, GSList **messages )
{
	gchar *id;
	ReaderData *profile_data;

	NAObjectProfile *profile = na_object_profile_new();

	id = g_path_get_basename( path );
	na_object_set_id( profile, id );
	g_free( id );

	profile_data = g_new0( ReaderData, 1 );
	profile_data->parent = data->parent;
	profile_data->path = ( gchar * ) path;
	profile_data->entries = na_mateconf_utils_get_entries( NAGP_MATECONF_PROVIDER( provider )->private->mateconf, path );

	na_ifactory_provider_read_item(
			NA_IFACTORY_PROVIDER( provider ),
			profile_data,
			NA_IFACTORY_OBJECT( profile ),
			messages );

	na_mateconf_utils_free_entries( profile_data->entries );
	g_free( profile_data );
}

static NADataBoxed *
get_boxed_from_path( const NagpMateConfProvider *provider, const gchar *path, ReaderData *reader_data, const NADataDef *def )
{
	static const gchar *thisfn = "nagp_reader_get_boxed_from_path";
	NADataBoxed *boxed;
	gboolean have_entry;
	gchar *str_value;
	gboolean bool_value;
	GSList *slist_value;
	gint int_value;

	boxed = NULL;
	have_entry = na_mateconf_utils_has_entry( reader_data->entries, def->mateconf_entry );
	g_debug( "%s: entry=%s, have_entry=%s", thisfn, def->mateconf_entry, have_entry ? "True":"False" );

	if( have_entry ){
		gchar *entry_path = mateconf_concat_dir_and_key( path, def->mateconf_entry );
		boxed = na_data_boxed_new( def );

		switch( def->type ){

			case NA_DATA_TYPE_STRING:
			case NA_DATA_TYPE_LOCALE_STRING:
				str_value = na_mateconf_utils_read_string( provider->private->mateconf, entry_path, TRUE, NULL );
				na_boxed_set_from_string( NA_BOXED( boxed ), str_value );
				g_free( str_value );
				break;

			case NA_DATA_TYPE_BOOLEAN:
				bool_value = na_mateconf_utils_read_bool( provider->private->mateconf, entry_path, TRUE, FALSE );
				na_boxed_set_from_void( NA_BOXED( boxed ), GUINT_TO_POINTER( bool_value ));
				break;

			case NA_DATA_TYPE_STRING_LIST:
				slist_value = na_mateconf_utils_read_string_list( provider->private->mateconf, entry_path );
				na_boxed_set_from_void( NA_BOXED( boxed ), slist_value );
				na_core_utils_slist_free( slist_value );
				break;

			case NA_DATA_TYPE_UINT:
				int_value = na_mateconf_utils_read_int( provider->private->mateconf, entry_path, TRUE, 0 );
				na_boxed_set_from_void( NA_BOXED( boxed ), GUINT_TO_POINTER( int_value ));
				break;

			default:
				g_warning( "%s: unknown type=%u for %s", thisfn, def->type, def->name );
				g_free( boxed );
				boxed = NULL;
		}

		g_free( entry_path );
	}

	return( boxed );
}

/*
 * key must be an existing entry (not a dir) to get a relevant return
 * value ; else we get FALSE
 */
static gboolean
is_key_writable( NagpMateConfProvider *mateconf, const gchar *key )
{
	static const gchar *thisfn = "nagp_read_is_key_writable";
	GError *error = NULL;
	gboolean is_writable;

	is_writable = mateconf_client_key_is_writable( mateconf->private->mateconf, key, &error );
	if( error ){
		g_warning( "%s: mateconf_client_key_is_writable: %s", thisfn, error->message );
		g_error_free( error );
		error = NULL;
		is_writable = FALSE;
	}

	return( is_writable );
}
