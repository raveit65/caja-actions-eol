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
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-data-types.h>
#include <api/na-object-api.h>
#include <api/na-ifactory-provider.h>

#include "cadp-desktop-file.h"
#include "cadp-desktop-provider.h"
#include "cadp-formats.h"
#include "cadp-keys.h"
#include "cadp-utils.h"
#include "cadp-writer.h"
#include "cadp-xdg-dirs.h"

/* the association between an export format and the functions
 */
typedef struct {
	gchar  *format;
	void   *foo;
}
	ExportFormatFn;

static ExportFormatFn st_export_format_fn[] = {

	{ CADP_FORMAT_DESKTOP_V1,
					NULL },

	{ NULL }
};

static guint           write_item( const NAIIOProvider *provider, const NAObjectItem *item, CappDesktopFile *ndf, GSList **messages );

static void            desktop_weak_notify( CappDesktopFile *ndf, GObject *item );

static void            write_start_write_type( CappDesktopFile *ndp, NAObjectItem *item );
static void            write_done_write_subitems_list( CappDesktopFile *ndp, NAObjectItem *item );

static ExportFormatFn *find_export_format_fn( const gchar *format );

#ifdef NA_ENABLE_DEPRECATED
static ExportFormatFn *find_export_format_fn_from_quark( GQuark format );
#endif

/*
 * This is implementation of NAIIOProvider::is_willing_to_write method
 */
gboolean
cadp_iio_provider_is_willing_to_write( const NAIIOProvider *provider )
{
	return( TRUE );
}

/*
 * CappDesktopProvider is able to write if user data dir exists (or
 * can be created) and is writable
 *
 * This is implementation of NAIIOProvider::is_able_to_write method
 */
gboolean
cadp_iio_provider_is_able_to_write( const NAIIOProvider *provider )
{
	static const gchar *thisfn = "cadp_writer_iio_provider_is_able_to_write";
	gboolean able_to;
	gchar *userdir;

	g_return_val_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ), FALSE );

	able_to = FALSE;

	userdir = cadp_xdg_dirs_get_user_data_dir();

	if( g_file_test( userdir, G_FILE_TEST_IS_DIR )){
		able_to = na_core_utils_dir_is_writable_path( userdir );

	} else if( g_mkdir_with_parents( userdir, 0750 )){
		g_warning( "%s: %s: %s", thisfn, userdir, g_strerror( errno ));

	} else {
		na_core_utils_dir_list_perms( userdir, thisfn );
		able_to = na_core_utils_dir_is_writable_path( userdir );
	}

	g_free( userdir );

	return( able_to );
}

/*
 * This is implementation of NAIIOProvider::write_item method
 */
guint
cadp_iio_provider_write_item( const NAIIOProvider *provider, const NAObjectItem *item, GSList **messages )
{
	static const gchar *thisfn = "cadp_iio_provider_write_item";
	guint ret;
	CappDesktopFile *ndf;
	gchar *path;
	gchar *userdir;
	gchar *id;
	gchar *bname;
	GSList *subdirs;
	gchar *fulldir;
	gboolean dir_ok;

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );

	if( na_object_is_readonly( item )){
		g_warning( "%s: item=%p is read-only", thisfn, ( void * ) item );
		return( ret );
	}

	ndf = ( CappDesktopFile * ) na_object_get_provider_data( item );

	/* write into the current key file and write it to current path */
	if( ndf ){
		g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), ret );

	} else {
		userdir = cadp_xdg_dirs_get_user_data_dir();
		subdirs = na_core_utils_slist_from_split( CADP_DESKTOP_PROVIDER_SUBDIRS, G_SEARCHPATH_SEPARATOR_S );
		fulldir = g_build_filename( userdir, ( gchar * ) subdirs->data, NULL );
		dir_ok = TRUE;

		if( !g_file_test( fulldir, G_FILE_TEST_IS_DIR )){
			if( g_mkdir_with_parents( fulldir, 0750 )){
				g_warning( "%s: %s: %s", thisfn, userdir, g_strerror( errno ));
				dir_ok = FALSE;
			} else {
				na_core_utils_dir_list_perms( userdir, thisfn );
			}
		}
		g_free( userdir );
		na_core_utils_slist_free( subdirs );

		if( dir_ok ){
			id = na_object_get_id( item );
			bname = g_strdup_printf( "%s%s", id, CADP_DESKTOP_FILE_SUFFIX );
			g_free( id );
			path = g_build_filename( fulldir, bname, NULL );
			g_free( bname );
		}
		g_free( fulldir );

		if( dir_ok ){
			ndf = cadp_desktop_file_new_for_write( path );
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
 * actually writes the item to the existing CappDesktopFile
 * as we have chosen to take advantage of data factory management system
 * we do not need to enumerate each and every elementary data
 *
 * As we want keep comments between through multiple updates, we cannot
 * just delete the .desktop file and recreate it as we are doing for MateConf.
 * Instead of that, we delete at end groups that have not been walked through
 * -> as a side effect, we lose comments inside of these groups :(
 */
static guint
write_item( const NAIIOProvider *provider, const NAObjectItem *item, CappDesktopFile *ndf, GSList **messages )
{
	static const gchar *thisfn = "cadp_iio_provider_write_item";
	guint ret;
	CappDesktopProvider *self;

	g_debug( "%s: provider=%p (%s), item=%p (%s), ndf=%p, messages=%p",
			thisfn,
			( void * ) provider, G_OBJECT_TYPE_NAME( provider ),
			( void * ) item, G_OBJECT_TYPE_NAME( item ),
			( void * ) ndf,
			( void * ) messages );

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), ret );
	g_return_val_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ), ret );
	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( provider ), ret );

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( item ), ret );

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), ret );

	self = CADP_DESKTOP_PROVIDER( provider );

	if( self->private->dispose_has_run ){
		return( NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN );
	}

	ret = NA_IIO_PROVIDER_CODE_OK;

	na_ifactory_provider_write_item( NA_IFACTORY_PROVIDER( provider ), ndf, NA_IFACTORY_OBJECT( item ), messages );

	if( !cadp_desktop_file_write( ndf )){
		ret = NA_IIO_PROVIDER_CODE_WRITE_ERROR;
	}

	return( ret );
}

guint
cadp_iio_provider_delete_item( const NAIIOProvider *provider, const NAObjectItem *item, GSList **messages )
{
	static const gchar *thisfn = "cadp_iio_provider_delete_item";
	guint ret;
	CappDesktopProvider *self;
	CappDesktopFile *ndf;
	gchar *uri;

	g_debug( "%s: provider=%p (%s), item=%p (%s), messages=%p",
			thisfn,
			( void * ) provider, G_OBJECT_TYPE_NAME( provider ),
			( void * ) item, G_OBJECT_TYPE_NAME( item ),
			( void * ) messages );

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), ret );
	g_return_val_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );

	self = CADP_DESKTOP_PROVIDER( provider );

	if( self->private->dispose_has_run ){
		return( NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN );
	}

	ndf = ( CappDesktopFile * ) na_object_get_provider_data( item );

	if( ndf ){
		g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), ret );
		uri = cadp_desktop_file_get_key_file_uri( ndf );
		if( cadp_utils_uri_delete( uri )){
			ret = NA_IIO_PROVIDER_CODE_OK;
		}
		g_free( uri );

	} else {
		g_warning( "%s: CappDesktopFile is null", thisfn );
		ret = NA_IIO_PROVIDER_CODE_OK;
	}

	return( ret );
}

static void
desktop_weak_notify( CappDesktopFile *ndf, GObject *item )
{
	static const gchar *thisfn = "cadp_writer_desktop_weak_notify";

	g_debug( "%s: ndf=%p (%s), item=%p (%s)",
			thisfn, ( void * ) ndf, G_OBJECT_TYPE_NAME( ndf ),
			( void * ) item, G_OBJECT_TYPE_NAME( item ));

	g_object_unref( ndf );
}

/*
 * Implementation of NAIIOProvider::duplicate_data
 * Add a ref on CappDesktopFile data, so that unreffing origin object in CACT
 * does not invalid duplicated pointer
 */
guint
cadp_iio_provider_duplicate_data( const NAIIOProvider *provider, NAObjectItem *dest, const NAObjectItem *source, GSList **messages )
{
	static const gchar *thisfn = "cadp_iio_provider_duplicate_data";
	guint ret;
	CappDesktopProvider *self;
	CappDesktopFile *ndf;

	g_debug( "%s: provider=%p (%s), dest=%p (%s), source=%p (%s), messages=%p",
			thisfn,
			( void * ) provider, G_OBJECT_TYPE_NAME( provider ),
			( void * ) dest, G_OBJECT_TYPE_NAME( dest ),
			( void * ) source, G_OBJECT_TYPE_NAME( source ),
			( void * ) messages );

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), ret );
	g_return_val_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( dest ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( source ), ret );

	self = CADP_DESKTOP_PROVIDER( provider );

	if( self->private->dispose_has_run ){
		return( NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN );
	}

	ndf = ( CappDesktopFile * ) na_object_get_provider_data( source );
	g_return_val_if_fail( ndf && CADP_IS_DESKTOP_FILE( ndf ), ret );
	na_object_set_provider_data( dest, g_object_ref( ndf ));
	g_object_weak_ref( G_OBJECT( dest ), ( GWeakNotify ) desktop_weak_notify, ndf );

	return( NA_IIO_PROVIDER_CODE_OK );
}

/**
 * cadp_writer_iexporter_export_to_buffer:
 * @instance: this #NAIExporter instance.
 * @parms: a #NAIExporterBufferParmsv2 structure.
 *
 * Export the specified 'item' to a newly allocated buffer.
 */
guint
cadp_writer_iexporter_export_to_buffer( const NAIExporter *instance, NAIExporterBufferParmsv2 *parms )
{
	static const gchar *thisfn = "cadp_writer_iexporter_export_to_buffer";
	guint code, write_code;
	ExportFormatFn *fmt;
	GKeyFile *key_file;
	CappDesktopFile *ndf;

	g_debug( "%s: instance=%p, parms=%p", thisfn, ( void * ) instance, ( void * ) parms );

	parms->buffer = NULL;
	code = NA_IEXPORTER_CODE_OK;

	if( !parms->exported || !NA_IS_OBJECT_ITEM( parms->exported )){
		code = NA_IEXPORTER_CODE_INVALID_ITEM;
	}

	if( code == NA_IEXPORTER_CODE_OK ){

#ifdef NA_ENABLE_DEPRECATED
		if( parms->version == 1 ){
			fmt = find_export_format_fn_from_quark((( NAIExporterBufferParms * ) parms )->format );
		} else {
			fmt = find_export_format_fn( parms->format );
		}
#else
		fmt = find_export_format_fn( parms->format );
#endif

		if( !fmt ){
			code = NA_IEXPORTER_CODE_INVALID_FORMAT;

		} else {
			ndf = cadp_desktop_file_new();
			write_code = na_ifactory_provider_write_item( NA_IFACTORY_PROVIDER( instance ), ndf, NA_IFACTORY_OBJECT( parms->exported ), &parms->messages );

			if( write_code != NA_IIO_PROVIDER_CODE_OK ){
				code = NA_IEXPORTER_CODE_ERROR;

			} else {
				key_file = cadp_desktop_file_get_key_file( ndf );
				parms->buffer = g_key_file_to_data( key_file, NULL, NULL );
			}

			g_object_unref( ndf );
		}
	}

	g_debug( "%s: returning code=%u", thisfn, code );
	return( code );
}

/**
 * cadp_writer_iexporter_export_to_file:
 * @instance: this #NAIExporter instance.
 * @parms: a #NAIExporterFileParmsv2 structure.
 *
 * Export the specified 'item' to a newly created file.
 */
guint
cadp_writer_iexporter_export_to_file( const NAIExporter *instance, NAIExporterFileParmsv2 *parms )
{
	static const gchar *thisfn = "cadp_writer_iexporter_export_to_file";
	guint code, write_code;
	gchar *id, *folder_path, *dest_path;
	ExportFormatFn *fmt;
	CappDesktopFile *ndf;

	g_debug( "%s: instance=%p, parms=%p", thisfn, ( void * ) instance, ( void * ) parms );

	parms->basename = NULL;
	code = NA_IEXPORTER_CODE_OK;

	if( !parms->exported || !NA_IS_OBJECT_ITEM( parms->exported )){
		code = NA_IEXPORTER_CODE_INVALID_ITEM;
	}

	if( code == NA_IEXPORTER_CODE_OK ){

#ifdef NA_ENABLE_DEPRECATED
		if( parms->version == 1 ){
			fmt = find_export_format_fn_from_quark((( NAIExporterFileParms * ) parms )->format );
		} else {
			fmt = find_export_format_fn( parms->format );
		}
#else
		fmt = find_export_format_fn( parms->format );
#endif

		if( !fmt ){
			code = NA_IEXPORTER_CODE_INVALID_FORMAT;

		} else {
			id = na_object_get_id( parms->exported );
			parms->basename = g_strdup_printf( "%s%s", id, CADP_DESKTOP_FILE_SUFFIX );
			g_free( id );

			folder_path = g_filename_from_uri( parms->folder, NULL, NULL );
			dest_path = g_strdup_printf( "%s/%s", folder_path, parms->basename );
			g_free( folder_path );

			ndf = cadp_desktop_file_new_for_write( dest_path );
			write_code = na_ifactory_provider_write_item( NA_IFACTORY_PROVIDER( instance ), ndf, NA_IFACTORY_OBJECT( parms->exported ), &parms->messages );

			if( write_code != NA_IIO_PROVIDER_CODE_OK ){
				code = NA_IEXPORTER_CODE_ERROR;

			} else if( !cadp_desktop_file_write( ndf )){
				code = NA_IEXPORTER_CODE_UNABLE_TO_WRITE;
			}

			g_free( dest_path );
			g_object_unref( ndf );
		}
	}

	g_debug( "%s: returning code=%u", thisfn, code );
	return( code );
}

guint
cadp_writer_ifactory_provider_write_start( const NAIFactoryProvider *provider, void *writer_data,
							const NAIFactoryObject *object, GSList **messages  )
{
	if( NA_IS_OBJECT_ITEM( object )){
		write_start_write_type( CADP_DESKTOP_FILE( writer_data ), NA_OBJECT_ITEM( object ));
	}

	return( NA_IIO_PROVIDER_CODE_OK );
}

static void
write_start_write_type( CappDesktopFile *ndp, NAObjectItem *item )
{
	cadp_desktop_file_set_string(
			ndp,
			CADP_GROUP_DESKTOP,
			CADP_KEY_TYPE,
			NA_IS_OBJECT_ACTION( item ) ? CADP_VALUE_TYPE_ACTION : CADP_VALUE_TYPE_MENU );
}

/*
 * when writing to .desktop file a profile which has both a path and parameters,
 * then concatenate these two fields to the 'Exec' key
 */
guint
cadp_writer_ifactory_provider_write_data(
				const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object,
				const NADataBoxed *boxed, GSList **messages )
{
	static const gchar *thisfn = "cadp_writer_ifactory_provider_write_data";
	CappDesktopFile *ndf;
	guint code;
	const NADataDef *def;
	gchar *profile_id;
	gchar *group_name;
	gchar *str_value;
	gboolean bool_value;
	GSList *slist_value;
	guint uint_value;
	gchar *parms, *tmp;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( writer_data ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
	/*g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));*/

	code = NA_IIO_PROVIDER_CODE_OK;
	ndf = CADP_DESKTOP_FILE( writer_data );
	def = na_data_boxed_get_data_def( boxed );

	if( def->desktop_entry && strlen( def->desktop_entry )){

		if( NA_IS_OBJECT_PROFILE( object )){
			profile_id = na_object_get_id( object );
			group_name = g_strdup_printf( "%s %s", CADP_GROUP_PROFILE, profile_id );
			g_free( profile_id );

		} else {
			group_name = g_strdup( CADP_GROUP_DESKTOP );
		}

		if( !na_data_boxed_is_default( boxed ) || def->write_if_default ){

			switch( def->type ){

				case NA_DATA_TYPE_STRING:
					str_value = na_boxed_get_string( NA_BOXED( boxed ));

					if( !strcmp( def->name, NAFO_DATA_PATH )){
						parms = na_object_get_parameters( object );
						tmp = g_strdup_printf( "%s %s", str_value, parms );
						g_free( str_value );
						g_free( parms );
						str_value = tmp;
					}

					cadp_desktop_file_set_string( ndf, group_name, def->desktop_entry, str_value );
					g_free( str_value );
					break;

				case NA_DATA_TYPE_LOCALE_STRING:
					str_value = na_boxed_get_string( NA_BOXED( boxed ));
					cadp_desktop_file_set_locale_string( ndf, group_name, def->desktop_entry, str_value );
					g_free( str_value );
					break;

				case NA_DATA_TYPE_BOOLEAN:
					bool_value = GPOINTER_TO_UINT( na_boxed_get_as_void( NA_BOXED( boxed )));
					cadp_desktop_file_set_boolean( ndf, group_name, def->desktop_entry, bool_value );
					break;

				case NA_DATA_TYPE_STRING_LIST:
					slist_value = ( GSList * ) na_boxed_get_as_void( NA_BOXED( boxed ));
					cadp_desktop_file_set_string_list( ndf, group_name, def->desktop_entry, slist_value );
					na_core_utils_slist_free( slist_value );
					break;

				case NA_DATA_TYPE_UINT:
					uint_value = GPOINTER_TO_UINT( na_boxed_get_as_void( NA_BOXED( boxed )));
					cadp_desktop_file_set_uint( ndf, group_name, def->desktop_entry, uint_value );
					break;

				default:
					g_warning( "%s: unknown type=%u for %s", thisfn, def->type, def->name );
					code = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;
			}

		} else {
			cadp_desktop_file_remove_key( ndf, group_name, def->desktop_entry );
		}

		g_free( group_name );
	}

	return( code );
}

guint
cadp_writer_ifactory_provider_write_done( const NAIFactoryProvider *provider, void *writer_data,
							const NAIFactoryObject *object, GSList **messages  )
{
	if( NA_IS_OBJECT_ITEM( object )){
		write_done_write_subitems_list( CADP_DESKTOP_FILE( writer_data ), NA_OBJECT_ITEM( object ));
	}

	return( NA_IIO_PROVIDER_CODE_OK );
}

static void
write_done_write_subitems_list( CappDesktopFile *ndp, NAObjectItem *item )
{
	static const gchar *thisfn = "cadp_writer_write_done_write_subitems_list";
	GSList *subitems;
	GSList *profile_groups, *ip;
	gchar *tmp;

	subitems = na_object_get_items_slist( item );
	tmp = g_strdup_printf( "%s (written subitems)", thisfn );
	na_core_utils_slist_dump( tmp, subitems );
	g_free( tmp );

	cadp_desktop_file_set_string_list(
			ndp,
			CADP_GROUP_DESKTOP,
			NA_IS_OBJECT_ACTION( item ) ? CADP_KEY_PROFILES : CADP_KEY_ITEMS_LIST,
			subitems );

	profile_groups = cadp_desktop_file_get_profiles( ndp );
	tmp = g_strdup_printf( "%s (existing profiles)", thisfn );
	na_core_utils_slist_dump( tmp, profile_groups );
	g_free( tmp );

	for( ip = profile_groups ; ip ; ip = ip->next ){
		if( na_core_utils_slist_count( subitems, ( const gchar * ) ip->data ) == 0 ){
			g_debug( "%s: deleting (removed) profile %s", thisfn, ( const gchar * ) ip->data );
			cadp_desktop_file_remove_profile( ndp, ( const gchar * ) ip->data );
		}
	}

	na_core_utils_slist_free( profile_groups );
	na_core_utils_slist_free( subitems );
}

static ExportFormatFn *
find_export_format_fn( const gchar *format )
{
	ExportFormatFn *found;
	ExportFormatFn *i;

	found = NULL;
	i = st_export_format_fn;

	while( i->format && !found ){
		if( !strcmp( i->format, format )){
			found = i;
		}
		i++;
	}

	return( found );
}

#ifdef NA_ENABLE_DEPRECATED
static ExportFormatFn *
find_export_format_fn_from_quark( GQuark format )
{
	ExportFormatFn *found;
	ExportFormatFn *i;

	found = NULL;
	i = st_export_format_fn;

	while( i->format && !found ){
		if( g_quark_from_string( i->format ) == format ){
			found = i;
		}
		i++;
	}

	return( found );
}
#endif
