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

#include <core/na-iprefs.h>

#include "cact-application.h"
#include "cact-iprefs.h"

/* private interface data
 */
struct CactIPrefsInterfacePrivate {
	MateConfClient *client;
};

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;

static GType       register_type( void );
static void        interface_base_init( CactIPrefsInterface *klass );
static void        interface_base_finalize( CactIPrefsInterface *klass );

static MateConfValue *get_value( MateConfClient *client, const gchar *path, const gchar *entry );
static void        set_value( MateConfClient *client, const gchar *path, const gchar *entry, MateConfValue *value );

GType
cact_iprefs_get_type( void )
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
	static const gchar *thisfn = "cact_iprefs_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactIPrefsInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "CactIPrefs", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( CactIPrefsInterface *klass )
{
	static const gchar *thisfn = "cact_iprefs_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactIPrefsInterfacePrivate, 1 );

		klass->private->client = mateconf_client_get_default();

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( CactIPrefsInterface *klass )
{
	static const gchar *thisfn = "cact_iprefs_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_object_unref( klass->private->client );

		g_free( klass->private );
	}
}

/**
 * cact_iprefs_get_export_format:
 * @window: this #BaseWindow-derived window.
 * @name: name of the export format key to be readen
 *
 * Returns: the export format currently set as a #GQuark.
 *
 * Defaults to exporting as a MateConfEntry (see. #cact-iprefs.h)
 *
 * Note: please take care of keeping the default value synchronized with
 * those defined in schemas.
 */
GQuark
cact_iprefs_get_export_format( const BaseWindow *window, const gchar *name )
{
	GQuark export_format;
	CactApplication *application;
	NAUpdater *updater;
	gchar *format_str;

	export_format = g_quark_from_static_string( IPREFS_EXPORT_FORMAT_DEFAULT );

	g_return_val_if_fail( BASE_IS_WINDOW( window ), export_format );

	if( st_initialized && !st_finalized ){

		application = CACT_APPLICATION( base_window_get_application( window ));
		updater = cact_application_get_updater( application );

		format_str = na_iprefs_read_string(
				NA_IPREFS( updater ),
				name,
				IPREFS_EXPORT_FORMAT_DEFAULT );

		export_format = g_quark_from_string( format_str );

		g_free( format_str );
	}

	return( export_format );
}

/**
 * cact_iprefs_set_export_format:
 * @window: this #BaseWindow-derived window.
 * @format: the new value to be written.
 *
 * Writes the preferred export format' to the MateConf preference system.
 */
void
cact_iprefs_set_export_format( const BaseWindow *window, const gchar *name, GQuark format )
{
	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( st_initialized && !st_finalized ){

		cact_iprefs_write_string(
				window,
				name,
				g_quark_to_string( format ));
	}
}

/**
 * cact_iprefs_migrate_key:
 * @window: a #BaseWindow window.
 * @old_key: the old preference entry.
 * @new_key: the new preference entry.
 *
 * Migrates the content of an entry from an obsoleted key to a new one.
 * Removes the old key, along with the schema associated to it,
 * considering that the version which asks for this migration has
 * installed a schema corresponding to the new key.
 */
void
cact_iprefs_migrate_key( const BaseWindow *window, const gchar *old_key, const gchar *new_key )
{
	static const gchar *thisfn = "cact_iprefs_migrate_key";
	MateConfClient *mateconf_client;
	MateConfValue *value;

	g_debug( "%s: window=%p, old_key=%s, new_key=%s", thisfn, ( void * ) window, old_key, new_key );
	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( st_initialized && !st_finalized ){

		mateconf_client = CACT_IPREFS_GET_INTERFACE( window )->private->client;

		value = get_value( mateconf_client, IPREFS_MATECONF_PREFS_PATH, new_key );
		if( !value ){
			value = get_value( mateconf_client, IPREFS_MATECONF_PREFS_PATH, old_key );
			if( value ){
				set_value( mateconf_client, IPREFS_MATECONF_PREFS_PATH, new_key, value );
				mateconf_value_free( value );
			}
		}

		/* do not remove entries which may still be used by an older N-A version
		 */
	}
}

/**
 * cact_iprefs_write_bool:
 * @window: this #BaseWindow-derived window.
 * @name: the preference entry.
 * @value: the value to be written.
 *
 * Writes the given boolean value.
 */
void
cact_iprefs_write_bool( const BaseWindow *window, const gchar *name, gboolean value )
{
	gchar *path;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( CACT_IS_IPREFS( window ));

	if( st_initialized && !st_finalized ){

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );
		na_mateconf_utils_write_bool( CACT_IPREFS_GET_INTERFACE( window )->private->client, path, value, NULL );
		g_free( path );
	}
}

/**
 * cact_iprefs_write_string:
 * @window: this #BaseWindow-derived window.
 * @name: the preference key.
 * @value: the value to be written.
 *
 * Writes the value as the given MateConf preference.
 */
void
cact_iprefs_write_string( const BaseWindow *window, const gchar *name, const gchar *value )
{
	gchar *path;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( CACT_IS_IPREFS( window ));

	if( st_initialized && !st_finalized ){

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_PREFS_PATH, name );

		na_mateconf_utils_write_string( CACT_IPREFS_GET_INTERFACE( window )->private->client, path, value, NULL );

		g_free( path );
	}
}

static MateConfValue *
get_value( MateConfClient *client, const gchar *path, const gchar *entry )
{
	static const gchar *thisfn = "na_iprefs_get_value";
	GError *error = NULL;
	gchar *fullpath;
	MateConfValue *value;

	fullpath = mateconf_concat_dir_and_key( path, entry );

	value = mateconf_client_get_without_default( client, fullpath, &error );

	if( error ){
		g_warning( "%s: key=%s, %s", thisfn, fullpath, error->message );
		g_error_free( error );
		if( value ){
			mateconf_value_free( value );
			value = NULL;
		}
	}

	g_free( fullpath );

	return( value );
}

static void
set_value( MateConfClient *client, const gchar *path, const gchar *entry, MateConfValue *value )
{
	static const gchar *thisfn = "na_iprefs_set_value";
	GError *error = NULL;
	gchar *fullpath;

	g_return_if_fail( value );

	fullpath = mateconf_concat_dir_and_key( path, entry );

	mateconf_client_set( client, fullpath, value, &error );

	if( error ){
		g_warning( "%s: key=%s, %s", thisfn, fullpath, error->message );
		g_error_free( error );
	}

	g_free( fullpath );
}
