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

#include <errno.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-data-types.h>
#include <api/na-object-api.h>
#include <api/na-ifactory-provider.h>

#include "nadp-desktop-file.h"
#include "nadp-desktop-provider.h"
#include "nadp-keys.h"
#include "nadp-writer.h"
#include "nadp-xdg-dirs.h"

static guint write_item( const NAIIOProvider *provider, const NAObjectItem *item, NadpDesktopFile *ndf, GSList **messages );

static void  desktop_weak_notify( NadpDesktopFile *ndf, GObject *item );

/*
 * This is implementation of NAIIOProvider::is_willing_to_write method
 */
gboolean
nadp_iio_provider_is_willing_to_write( const NAIIOProvider *provider )
{
	return( TRUE );
}

/*
 * NadpDesktopProvider is able to write if user data dir exists (or
 * can be created) and is writable
 *
 * This is implementation of NAIIOProvider::is_able_to_write method
 */
gboolean
nadp_iio_provider_is_able_to_write( const NAIIOProvider *provider )
{
	static const gchar *thisfn = "nadp_writer_iio_provider_is_able_to_write";
	gboolean able_to;
	gchar *userdir;
	GSList *messages;

	g_return_val_if_fail( NADP_IS_DESKTOP_PROVIDER( provider ), FALSE );

	able_to = FALSE;
	messages = NULL;

	userdir = nadp_xdg_dirs_get_user_data_dir();

	if( g_file_test( userdir, G_FILE_TEST_IS_DIR )){
		able_to = na_core_utils_dir_is_writable_path( userdir );

	} else if( g_mkdir_with_parents( userdir, 0700 )){
		g_warning( "%s: %s: %s", thisfn, userdir, g_strerror( errno ));

	} else {
		able_to = na_core_utils_dir_is_writable_path( userdir );
	}

	g_free( userdir );

	return( able_to );
}

/*
 * This is implementation of NAIIOProvider::write_item method
 */
guint
nadp_iio_provider_write_item( const NAIIOProvider *provider, const NAObjectItem *item, GSList **messages )
{
	static const gchar *thisfn = "nadp_iio_provider_write_item";
	guint ret;
	NadpDesktopFile *ndf;
	gchar *path;
	gchar *userdir;
	gchar *id;
	gchar *bname;
	GSList *subdirs;
	gchar *fulldir;
	gboolean dir_ok;

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NADP_IS_DESKTOP_PROVIDER( provider ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );

	if( na_object_is_readonly( item )){
		g_warning( "%s: item=%p is read-only", thisfn, ( void * ) item );
		return( ret );
	}

	ndf = ( NadpDesktopFile * ) na_object_get_provider_data( item );

	/* write into the current key file and write it to current path */
	if( ndf ){
		g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), ret );

	} else {
		userdir = nadp_xdg_dirs_get_user_data_dir();
		subdirs = na_core_utils_slist_from_split( NADP_DESKTOP_PROVIDER_SUBDIRS, G_SEARCHPATH_SEPARATOR_S );
		fulldir = g_build_filename( userdir, ( gchar * ) subdirs->data, NULL );
		dir_ok = TRUE;
		if( !g_file_test( fulldir, G_FILE_TEST_IS_DIR )){
			if( g_mkdir_with_parents( fulldir, 0700 )){
				g_warning( "%s: %s: %s", thisfn, userdir, g_strerror( errno ));
				dir_ok = FALSE;
			}
		}
		g_free( userdir );
		na_core_utils_slist_free( subdirs );

		if( dir_ok ){
			id = na_object_get_id( item );
			bname = g_strdup_printf( "%s%s", id, NADP_DESKTOP_FILE_SUFFIX );
			g_free( id );
			path = g_build_filename( fulldir, bname, NULL );
			g_free( bname );
		}
		g_free( fulldir );

		if( dir_ok ){
			ndf = nadp_desktop_file_new_for_write( path );
			na_object_set_provider_data( item, ndf );
			g_object_weak_ref( G_OBJECT( item ), ( GWeakNotify ) desktop_weak_notify, ndf );
			g_free( path );
		}
	}

	if( ndf ){
		ret = write_item( provider, item, ndf, messages );
	}

	return( ret );
}

/*
 * actually writes the item to the existing NadpDesktopFile
 * as we have choosen to take advantage of data factory management system
 * we do not need to enumerate each and every elementary data
 *
 * As we want keep comments between through multiple updates, we cannot
 * just delete the .desktop file and recreate it as we are doing for MateConf.
 * Instead of that, we delete each group before updating it, then deleting
 * last groups (not updated ones) at end.
 * -> as a side effect, we lose comments inside of groups :(
 */
static guint
write_item( const NAIIOProvider *provider, const NAObjectItem *item, NadpDesktopFile *ndf, GSList **messages )
{
	static const gchar *thisfn = "nadp_iio_provider_write_item";
	guint ret;
	NadpDesktopProvider *self;

	g_debug( "%s: provider=%p (%s), item=%p (%s), ndf=%p, messages=%p",
			thisfn,
			( void * ) provider, G_OBJECT_TYPE_NAME( provider ),
			( void * ) item, G_OBJECT_TYPE_NAME( item ),
			( void * ) ndf,
			( void * ) messages );

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), ret );
	g_return_val_if_fail( NADP_IS_DESKTOP_PROVIDER( provider ), ret );
	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( provider ), ret );

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( item ), ret );

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), ret );

	self = NADP_DESKTOP_PROVIDER( provider );

	if( self->private->dispose_has_run ){
		return( NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN );
	}

	ret = NA_IIO_PROVIDER_CODE_OK;

	na_ifactory_provider_write_item( NA_IFACTORY_PROVIDER( provider ), ndf, NA_IFACTORY_OBJECT( item ), messages );

	if( !nadp_desktop_file_write( ndf )){
		ret = NA_IIO_PROVIDER_CODE_WRITE_ERROR;
	}

	return( ret );
}

guint
nadp_iio_provider_delete_item( const NAIIOProvider *provider, const NAObjectItem *item, GSList **messages )
{
	static const gchar *thisfn = "nadp_iio_provider_delete_item";
	guint ret;
	NadpDesktopProvider *self;
	NadpDesktopFile *ndf;
	gchar *path;

	g_debug( "%s: provider=%p (%s), item=%p (%s), messages=%p",
			thisfn,
			( void * ) provider, G_OBJECT_TYPE_NAME( provider ),
			( void * ) item, G_OBJECT_TYPE_NAME( item ),
			( void * ) messages );

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), ret );
	g_return_val_if_fail( NADP_IS_DESKTOP_PROVIDER( provider ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );

	self = NADP_DESKTOP_PROVIDER( provider );

	if( self->private->dispose_has_run ){
		return( NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN );
	}

	ndf = ( NadpDesktopFile * ) na_object_get_provider_data( item );

	if( ndf ){
		g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), ret );
		path = nadp_desktop_file_get_key_file_path( ndf );
		if( na_core_utils_file_delete( path )){
			ret = NA_IIO_PROVIDER_CODE_OK;
		}
		g_free( path );

	} else {
		g_warning( "%s: NadpDesktopFile is null", thisfn );
		ret = NA_IIO_PROVIDER_CODE_OK;
	}

	return( ret );
}

static void
desktop_weak_notify( NadpDesktopFile *ndf, GObject *item )
{
	static const gchar *thisfn = "nadp_writer_desktop_weak_notify";

	g_debug( "%s: ndf=%p (%s), item=%p (%s)",
			thisfn, ( void * ) ndf, G_OBJECT_TYPE_NAME( ndf ),
			( void * ) item, G_OBJECT_TYPE_NAME( item ));

	g_object_unref( ndf );
}

#if 0
/*
 * the item comes from being readen from a desktop file
 * -> see if this desktop file is writable ?
 *
 * This is only used to setup the 'read-only' initial status of the
 * NAObjectItem - We don't care of all events which can suddenly make
 * this item becomes readonly (eventually we will deal for errors,
 * and reset the flag at this time)
 *
 * Internal function: do not call from outside the instance.
 */
gboolean
nadp_writer_desktop_is_writable( const NAIIOProvider *provider, const NAObjectItem *item )
{
	static const gchar *thisfn = "nadp_writer_desktop_is_writable";
	gboolean writable;
	NadpDesktopFile *ndf;
	gchar *path;

	writable = FALSE;
	g_return_val_if_fail( NADP_IS_DESKTOP_PROVIDER( provider ), writable );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), writable );

	if( NA_IS_OBJECT_MENU( item )){
		g_warning( "%s: menu are not yet handled by Desktop provider", thisfn );
		return( FALSE );
	}

	ndf = ( NadpDesktopFile * ) na_object_get_provider_data( item );

	if( ndf ){
		g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), writable );
		path = nadp_desktop_file_get_key_file_path( ndf );
		writable = nadp_utils_is_writable_file( path );
		g_free( path );
	}

	return( writable );
}
#endif

guint
nadp_writer_ifactory_provider_write_start( const NAIFactoryProvider *provider, void *writer_data,
							const NAIFactoryObject *object, GSList **messages  )
{
	if( NA_IS_OBJECT_ITEM( object )){
		nadp_desktop_file_set_string(
				NADP_DESKTOP_FILE( writer_data ),
				NADP_GROUP_DESKTOP,
				NADP_KEY_TYPE,
				NA_IS_OBJECT_ACTION( object ) ? NADP_VALUE_TYPE_ACTION : NADP_VALUE_TYPE_MENU );
	}

	return( NA_IIO_PROVIDER_CODE_OK );
}

guint
nadp_writer_ifactory_provider_write_data(
				const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object,
				const NADataBoxed *boxed, GSList **messages )
{
	static const gchar *thisfn = "nadp_writer_ifactory_provider_write_data";
	NadpDesktopFile *ndf;
	guint code;
	NADataDef *def;
	gchar *profile_id;
	gchar *group_name;
	gchar *str_value;
	gboolean bool_value;
	GSList *slist_value;
	guint uint_value;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( writer_data ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
	/*g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));*/

	code = NA_IIO_PROVIDER_CODE_OK;
	ndf = NADP_DESKTOP_FILE( writer_data );
	def = na_data_boxed_get_data_def( boxed );

	if( def->desktop_entry && strlen( def->desktop_entry )){

		if( NA_IS_OBJECT_PROFILE( object )){
			profile_id = na_object_get_id( object );
			group_name = g_strdup_printf( "%s %s", NADP_GROUP_PROFILE, profile_id );
			g_free( profile_id );

		} else {
			group_name = g_strdup( NADP_GROUP_DESKTOP );
		}

		if( na_data_boxed_is_set( boxed )){

			switch( def->type ){

				case NAFD_TYPE_STRING:
					str_value = na_data_boxed_get_as_string( boxed );
					nadp_desktop_file_set_string( ndf, group_name, def->desktop_entry, str_value );
					g_free( str_value );
					break;

				case NAFD_TYPE_LOCALE_STRING:
					str_value = na_data_boxed_get_as_string( boxed );
					nadp_desktop_file_set_locale_string( ndf, group_name, def->desktop_entry, str_value );
					g_free( str_value );
					break;

				case NAFD_TYPE_BOOLEAN:
					bool_value = GPOINTER_TO_UINT( na_data_boxed_get_as_void( boxed ));
					nadp_desktop_file_set_boolean( ndf, group_name, def->desktop_entry, bool_value );
					break;

				case NAFD_TYPE_STRING_LIST:
					slist_value = ( GSList * ) na_data_boxed_get_as_void( boxed );
					nadp_desktop_file_set_string_list( ndf, group_name, def->desktop_entry, slist_value );
					na_core_utils_slist_free( slist_value );
					break;

				case NAFD_TYPE_UINT:
					uint_value = GPOINTER_TO_UINT( na_data_boxed_get_as_void( boxed ));
					nadp_desktop_file_set_uint( ndf, group_name, def->desktop_entry, uint_value );
					break;

				default:
					g_warning( "%s: unknown type=%u for %s", thisfn, def->type, def->name );
					code = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;
			}

		} else {
			nadp_desktop_file_remove_key( ndf, group_name, def->desktop_entry );
		}

		g_free( group_name );
	}

	return( code );
}

guint
nadp_writer_ifactory_provider_write_done( const NAIFactoryProvider *provider, void *writer_data,
							const NAIFactoryObject *object, GSList **messages  )
{
	GSList *subitems;

	if( NA_IS_OBJECT_ITEM( object )){
		subitems = na_object_get_items_slist( object );

		nadp_desktop_file_set_string_list(
				NADP_DESKTOP_FILE( writer_data ),
				NADP_GROUP_DESKTOP,
				NA_IS_OBJECT_ACTION( object ) ? NADP_KEY_PROFILES : NADP_KEY_ITEMS_LIST,
				subitems );

		na_core_utils_slist_free( subitems );
	}

	return( NA_IIO_PROVIDER_CODE_OK );
}
