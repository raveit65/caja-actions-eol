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

#include <api/na-mateconf-utils.h>
#include <api/na-iimporter.h>

#include "na-iprefs.h"

/* private interface data
 */
struct NAIPrefsInterfacePrivate {
	MateConfClient *mateconf;
};

#define DEFAULT_ORDER_MODE_INT				IPREFS_ORDER_ALPHA_ASCENDING
#define DEFAULT_ORDER_MODE_STR				"AscendingOrder"

static MateConfEnumStringPair order_mode_table[] = {
	{ IPREFS_ORDER_ALPHA_ASCENDING ,		"AscendingOrder" },
	{ IPREFS_ORDER_ALPHA_DESCENDING,		"DescendingOrder" },
	{ IPREFS_ORDER_MANUAL          ,		"ManualOrder" },
	{ 0, NULL }
};

#define DEFAULT_IMPORT_MODE_INT				IMPORTER_MODE_NO_IMPORT
#define DEFAULT_IMPORT_MODE_STR				"NoImport"

static MateConfEnumStringPair import_mode_table[] = {
	{ IMPORTER_MODE_NO_IMPORT,				DEFAULT_IMPORT_MODE_STR },
	{ IMPORTER_MODE_RENUMBER,				"Renumber" },
	{ IMPORTER_MODE_OVERRIDE,				"Override" },
	{ IMPORTER_MODE_ASK,					"Ask" },
	{ 0, NULL }
};

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;

static GType register_type( void );
static void  interface_base_init( NAIPrefsInterface *klass );
static void  interface_base_finalize( NAIPrefsInterface *klass );

static void  write_string( NAIPrefs *instance, const gchar *name, const gchar *value );

GType
na_iprefs_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = register_type();
	}

	return( iface_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_iprefs_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIPrefsInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIPrefs", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIPrefsInterface *klass )
{
	static const gchar *thisfn = "na_iprefs_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NAIPrefsInterfacePrivate, 1 );

		klass->private->mateconf = mateconf_client_get_default();

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NAIPrefsInterface *klass )
{
	static const gchar *thisfn = "na_iprefs_interface_base_finalize";

	if( !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_object_unref( klass->private->mateconf );

		g_free( klass->private );
	}
}

/**
 * na_iprefs_get_order_mode:
 * @instance: this #NAIPrefs interface instance.
 *
 * Returns: the order mode currently set.
 *
 * Note: this function returns a suitable default value even if the key
 * is not found in MateConf preferences or no schema has been installed.
 *
 * Note: please take care of keeping the default value synchronized with
 * those defined in schemas.
 */
gint
na_iprefs_get_order_mode( NAIPrefs *instance )
{
	gint alpha_order = DEFAULT_ORDER_MODE_INT;
	gint order_int;
	gchar *order_str;

	g_return_val_if_fail( NA_IS_IPREFS( instance ), DEFAULT_ORDER_MODE_INT );

	if( st_initialized && !st_finalized ){

		order_str = na_iprefs_read_string(
				instance,
				IPREFS_DISPLAY_ALPHABETICAL_ORDER,
				DEFAULT_ORDER_MODE_STR );

		if( mateconf_string_to_enum( order_mode_table, order_str, &order_int )){
			alpha_order = order_int;
		}

		g_free( order_str );
	}

	return( alpha_order );
}

/**
 * na_iprefs_set_order_mode:
 * @instance: this #NAIPrefs interface instance.
 * @mode: the new value to be written.
 *
 * Writes the current status of 'alphabetical order' to the MateConf
 * preference system.
 */
void
na_iprefs_set_order_mode( NAIPrefs *instance, gint mode )
{
	const gchar *order_str;

	g_return_if_fail( NA_IS_IPREFS( instance ));

	if( st_initialized && !st_finalized ){

		order_str = mateconf_enum_to_string( order_mode_table, mode );

		write_string(
				instance,
				IPREFS_DISPLAY_ALPHABETICAL_ORDER,
				order_str ? order_str : DEFAULT_ORDER_MODE_STR );
	}
}

/**
 * na_iprefs_get_import_mode:
 * @mateconf: a #GCongClient client.
 * @name: name of the import key to be readen
 *
 * Returns: the import mode currently set.
 *
 * Note: this function returns a suitable default value even if the key
 * is not found in MateConf preferences or no schema has been installed.
 *
 * Note: please take care of keeping the default value synchronized with
 * those defined in schemas.
 */
guint
na_iprefs_get_import_mode( MateConfClient *mateconf, const gchar *name )
{
	gint import_mode = DEFAULT_IMPORT_MODE_INT;
	gint import_int;
	gchar *import_str;
	gchar *path;

	path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );

	import_str = na_mateconf_utils_read_string(
			mateconf,
			path,
			TRUE,
			DEFAULT_IMPORT_MODE_STR );

	if( mateconf_string_to_enum( import_mode_table, import_str, &import_int )){
		import_mode = import_int;
	}

	g_free( import_str );
	g_free( path );

	return( import_mode );
}

/**
 * na_iprefs_set_import_mode:
 * @mateconf: a #GCongClient client.
 * @mode: the new value to be written.
 *
 * Writes the current status of 'import mode' to the MateConf
 * preference system.
 */
void
na_iprefs_set_import_mode( MateConfClient *mateconf, const gchar *name, guint mode )
{
	const gchar *import_str;
	gchar *path;

	path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );

	import_str = mateconf_enum_to_string( import_mode_table, mode );

	na_mateconf_utils_write_string(
			mateconf,
			path,
			import_str ? import_str : DEFAULT_IMPORT_MODE_STR,
			NULL );

	g_free( path );
}

/**
 * na_iprefs_get_mateconf_client:
 * @instance: this #NAIPrefs interface instance.
 *
 * Returns: a MateConfClient object.
 */
MateConfClient *
na_iprefs_get_mateconf_client( const NAIPrefs *instance )
{
	MateConfClient *client;

	g_return_val_if_fail( NA_IS_IPREFS( instance ), NULL );

	client = NULL;

	if( st_initialized && !st_finalized ){

		client = NA_IPREFS_GET_INTERFACE( instance )->private->mateconf;
	}

	return( client );
}

/**
 * na_iprefs_read_bool:
 * @instance: this #NAIPrefs interface instance.
 * @name: the name of the preference entry.
 * @default_value: default value to be returned if the entry is not found,
 * no default value is available in the schema, of there is no schema at
 * all.
 *
 * Returns: the boolean value.
 */
gboolean
na_iprefs_read_bool( const NAIPrefs *instance, const gchar *name, gboolean default_value )
{
	gchar *path;
	gboolean ret;

	g_return_val_if_fail( NA_IS_IPREFS( instance ), FALSE );

	ret = FALSE;

	if( st_initialized && !st_finalized ){

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );
		ret = na_mateconf_utils_read_bool( na_iprefs_get_mateconf_client( instance ), path, TRUE, default_value );
		g_free( path );
	}

	return( ret );
}

/**
 * na_iprefs_read_string:
 * @instance: this #NAIPrefs interface instance.
 * @name: the preference key.
 * @default_value: the default value, used if entry is not found and
 * there is no schema.
 *
 * Returns: the value, as a newly allocated string which should be
 * g_free() by the caller.
 */
gchar *
na_iprefs_read_string( const NAIPrefs *instance, const gchar *name, const gchar *default_value )
{
	gchar *path;
	gchar *value;

	g_return_val_if_fail( NA_IS_IPREFS( instance ), NULL );

	value = NULL;

	if( st_initialized && !st_finalized ){

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );
		value = na_mateconf_utils_read_string( na_iprefs_get_mateconf_client( instance ), path, TRUE, default_value );
		g_free( path );
	}

	return( value );
}

/**
 * na_iprefs_read_string_list:
 * @instance: this #NAIPrefs interface instance.
 * @name: the preference key.
 * @default_value: a default value, used if entry is not found, or there
 * is no default value in the schema, of there is no schema at all.
 *
 * Returns: the list value, which should be na_utils_free_string_list()
 * by the caller.
 */
GSList *
na_iprefs_read_string_list( const NAIPrefs *instance, const gchar *name, const gchar *default_value )
{
	gchar *path;
	GSList *list;

	g_return_val_if_fail( NA_IS_IPREFS( instance ), NULL );

	list = NULL;

	if( st_initialized && !st_finalized ){

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );
		list = na_mateconf_utils_read_string_list( na_iprefs_get_mateconf_client( instance ), path );
		g_free( path );

		if(( !list || !g_slist_length( list )) && default_value ){
			g_slist_free( list );
			list = g_slist_append( NULL, g_strdup( default_value ));
		}
	}

	return( list );
}

/*
 * na_iprefs_write_string:
 * @instance: this #NAIPrefs interface instance.
 * @name: the preference key.
 * @value: the value to be written.
 *
 * Writes the value as the given MateConf preference.
 */
static void
write_string( NAIPrefs *instance, const gchar *name, const gchar *value )
{
	gchar *path;

	g_return_if_fail( NA_IS_IPREFS( instance ));

	if( st_initialized && !st_finalized ){

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );
		na_mateconf_utils_write_string( na_iprefs_get_mateconf_client( instance ), path, value, NULL );
		g_free( path );
	}
}

/**
 * na_iprefs_write_string_list
 * @instance: this #NAIPrefs interface instance.
 * @name: the preference key.
 * @value: the value to be written.
 *
 * Writes the value as the given MateConf preference.
 */
void
na_iprefs_write_string_list( const NAIPrefs *instance, const gchar *name, GSList *list )
{
	gchar *path;

	g_return_if_fail( NA_IS_IPREFS( instance ));

	if( st_initialized && !st_finalized ){

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );
		na_mateconf_utils_write_string_list( na_iprefs_get_mateconf_client( instance ), path, list, NULL );
		g_free( path );
	}
}
