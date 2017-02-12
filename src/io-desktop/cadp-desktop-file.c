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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <gio/gio.h>

#include <api/na-core-utils.h>

#include "cadp-desktop-file.h"
#include "cadp-keys.h"

/* private class data
 */
struct _CappDesktopFileClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CappDesktopFilePrivate {
	gboolean   dispose_has_run;
	gchar     *id;
	gchar     *uri;
	gchar     *type;
	GKeyFile  *key_file;
};

static GObjectClass *st_parent_class = NULL;

static GType            register_type( void );
static void             class_init( CappDesktopFileClass *klass );
static void             instance_init( GTypeInstance *instance, gpointer klass );
static void             instance_dispose( GObject *object );
static void             instance_finalize( GObject *object );

static CappDesktopFile *ndf_new( const gchar *uri );
static gchar           *path2id( const gchar *path );
static gchar           *uri2id( const gchar *uri );
static gboolean         check_key_file( CappDesktopFile *ndf );
static void             remove_encoding_part( CappDesktopFile *ndf );

GType
cadp_desktop_file_get_type( void )
{
	static GType class_type = 0;

	if( !class_type ){
		class_type = register_type();
	}

	return( class_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cadp_desktop_file_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CappDesktopFileClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CappDesktopFile ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "CappDesktopFile", &info, 0 );

	return( type );
}

static void
class_init( CappDesktopFileClass *klass )
{
	static const gchar *thisfn = "cadp_desktop_file_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CappDesktopFileClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cadp_desktop_file_instance_init";
	CappDesktopFile *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( CADP_IS_DESKTOP_FILE( instance ));
	self = CADP_DESKTOP_FILE( instance );

	self->private = g_new0( CappDesktopFilePrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->key_file = g_key_file_new();
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cadp_desktop_file_instance_dispose";
	CappDesktopFile *self;

	g_return_if_fail( CADP_IS_DESKTOP_FILE( object ));

	self = CADP_DESKTOP_FILE( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "cadp_desktop_file_instance_finalize";
	CappDesktopFile *self;

	g_return_if_fail( CADP_IS_DESKTOP_FILE( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = CADP_DESKTOP_FILE( object );

	g_free( self->private->id );
	g_free( self->private->uri );
	g_free( self->private->type );

	if( self->private->key_file ){
		g_key_file_free( self->private->key_file );
	}

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * cadp_desktop_file_new:
 *
 * Retuns: a newly allocated #CappDesktopFile object.
 */
CappDesktopFile *
cadp_desktop_file_new( void )
{
	CappDesktopFile *ndf;

	ndf = g_object_new( CADP_TYPE_DESKTOP_FILE, NULL );

	return( ndf );
}

/**
 * cadp_desktop_file_new_from_path:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #CappDesktopFile object.
 *
 * Key file has been loaded, and first validity checks made.
 */
CappDesktopFile *
cadp_desktop_file_new_from_path( const gchar *path )
{
	static const gchar *thisfn = "cadp_desktop_file_new_from_path";
	CappDesktopFile *ndf;
	GError *error;
	gchar *uri;

	ndf = NULL;
	g_debug( "%s: path=%s", thisfn, path );
	g_return_val_if_fail( path && g_utf8_strlen( path, -1 ) && g_path_is_absolute( path ), ndf );

	error = NULL;
	uri = g_filename_to_uri( path, NULL, &error );
	if( !uri || error ){
		g_warning( "%s: %s: %s", thisfn, path, error->message );
		g_error_free( error );
		g_free( uri );
		return( NULL );
	}

	ndf = ndf_new( uri );

	g_free( uri );

	g_key_file_load_from_file( ndf->private->key_file, path, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error );
	if( error ){
		g_warning( "%s: %s: %s", thisfn, path, error->message );
		g_error_free( error );
		g_object_unref( ndf );
		return( NULL );
	}

	if( !check_key_file( ndf )){
		g_object_unref( ndf );
		return( NULL );
	}

	return( ndf );
}

/**
 * cadp_desktop_file_new_from_uri:
 * @uri: the URI the desktop file should be loaded from.
 *
 * Retuns: a newly allocated #CappDesktopFile object, or %NULL.
 *
 * Key file has been loaded, and first validity checks made.
 *
 * Note: This function is in particular used when importing a file.
 * So it is rather common that the file not be a .desktop one.
 * Do not warns when file is malformed.
 */
CappDesktopFile *
cadp_desktop_file_new_from_uri( const gchar *uri )
{
	static const gchar *thisfn = "cadp_desktop_file_new_from_uri";
	CappDesktopFile *ndf;
	GError *error;
	gchar *data;
	gsize length;

	ndf = NULL;
	data = NULL;
	length = 0;

	g_debug( "%s: uri=%s", thisfn, uri );
	g_return_val_if_fail( uri && g_utf8_strlen( uri, -1 ), ndf );

	data = na_core_utils_file_load_from_uri( uri, &length );
	g_debug( "%s: length=%lu", thisfn, ( unsigned long ) length );

	/* normally, length and data should be both NULL or both not NULL
	 */
	if( !length || !data ){
		return( NULL );
	}

	error = NULL;
	ndf = ndf_new( uri );
	g_key_file_load_from_data( ndf->private->key_file, data, length, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error );
	g_free( data );

	if( error ){
		if( error->code != G_KEY_FILE_ERROR_GROUP_NOT_FOUND ){
			g_debug( "%s: %s", thisfn, error->message );
		}
		g_error_free( error );
		g_object_unref( ndf );
		return( NULL );
	}

	if( !check_key_file( ndf )){
		g_object_unref( ndf );
		return( NULL );
	}

	return( ndf );
}

/**
 * cadp_desktop_file_new_for_write:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #CappDesktopFile object.
 */
CappDesktopFile *
cadp_desktop_file_new_for_write( const gchar *path )
{
	static const gchar *thisfn = "cadp_desktop_file_new_for_write";
	CappDesktopFile *ndf;
	GError *error;
	gchar *uri;

	ndf = NULL;
	g_debug( "%s: path=%s", thisfn, path );
	g_return_val_if_fail( path && g_utf8_strlen( path, -1 ) && g_path_is_absolute( path ), ndf );

	error = NULL;
	uri = g_filename_to_uri( path, NULL, &error );
	if( !uri || error ){
		g_warning( "%s: %s: %s", thisfn, path, error->message );
		g_error_free( error );
		g_free( uri );
		return( NULL );
	}

	ndf = ndf_new( uri );

	g_free( uri );

	return( ndf );
}

/**
 * cadp_desktop_file_get_key_file:
 * @ndf: the #CappDesktopFile instance.
 *
 * Returns: a pointer to the internal #GKeyFile.
 */
GKeyFile *
cadp_desktop_file_get_key_file( const CappDesktopFile *ndf )
{
	GKeyFile *key_file;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	key_file = NULL;

	if( !ndf->private->dispose_has_run ){

		key_file = ndf->private->key_file;
	}

	return( key_file );
}

/**
 * cadp_desktop_file_get_key_file_uri:
 * @ndf: the #CappDesktopFile instance.
 *
 * Returns: the URI of the key file, as a newly allocated
 * string which should be g_free() by the caller.
 */
gchar *
cadp_desktop_file_get_key_file_uri( const CappDesktopFile *ndf )
{
	gchar *uri;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	uri = NULL;

	if( !ndf->private->dispose_has_run ){

		uri = g_strdup( ndf->private->uri );
	}

	return( uri );
}

static CappDesktopFile *
ndf_new( const gchar *uri )
{
	CappDesktopFile *ndf;

	ndf = g_object_new( CADP_TYPE_DESKTOP_FILE, NULL );

	ndf->private->id = uri2id( uri );
	ndf->private->uri = g_strdup( uri );

	return( ndf );
}

/*
 * The id of the file is equal to the basename, minus the suffix.
 */
static gchar *
path2id( const gchar *path )
{
	gchar *bname;
	gchar *id;

	bname = g_path_get_basename( path );
	id = na_core_utils_str_remove_suffix( bname, CADP_DESKTOP_FILE_SUFFIX );
	g_free( bname );

	return( id );
}

static gchar *
uri2id( const gchar *uri )
{
	gchar *path;
	gchar *id;

	id = NULL;
	path = g_filename_from_uri( uri, NULL, NULL );

	if( path ){
		id = path2id( path );
		g_free( path );
	}

	return( id );
}

static gboolean
check_key_file( CappDesktopFile *ndf )
{
	static const gchar *thisfn = "cadp_desktop_file_check_key_file";
	gboolean ret;
	gchar *start_group;
	gboolean has_key;
	gboolean hidden;
	gchar *type;
	GError *error;

	ret = TRUE;
	error = NULL;

	/* start group must be [Desktop Entry] */
	start_group = g_key_file_get_start_group( ndf->private->key_file );
	if( strcmp( start_group, CADP_GROUP_DESKTOP )){
		g_debug( "%s: %s: invalid start group, found %s, waited for %s",
				thisfn, ndf->private->uri, start_group, CADP_GROUP_DESKTOP );
		ret = FALSE;
	}

	/* must not have Hidden=true value */
	if( ret ){
		has_key = g_key_file_has_key( ndf->private->key_file, start_group, CADP_KEY_HIDDEN, &error );
		if( error ){
			g_debug( "%s: %s: %s", thisfn, ndf->private->uri, error->message );
			ret = FALSE;

		} else if( has_key ){
			hidden = g_key_file_get_boolean( ndf->private->key_file, start_group, CADP_KEY_HIDDEN, &error );
			if( error ){
				g_debug( "%s: %s: %s", thisfn, ndf->private->uri, error->message );
				ret = FALSE;

			} else if( hidden ){
				g_debug( "%s: %s: Hidden=true", thisfn, ndf->private->uri );
				ret = FALSE;
			}
		}
	}

	/* must have no Type (which defaults to Action)
	 * or a known one (Action or Menu)
	 */
	if( ret ){
		type = NULL;
		has_key = g_key_file_has_key( ndf->private->key_file, start_group, CADP_KEY_TYPE, &error );
		if( error ){
			g_debug( "%s: %s", thisfn, error->message );
			g_error_free( error );
			ret = FALSE;

		} else if( has_key ){
			type = g_key_file_get_string( ndf->private->key_file, start_group, CADP_KEY_TYPE, &error );
			if( error ){
				g_debug( "%s: %s", thisfn, error->message );
				g_free( type );
				g_error_free( error );
				ret = FALSE;
			}
		}
		if( ret ){
			if( !type || !strlen( type )){
				type = g_strdup( CADP_VALUE_TYPE_ACTION );

			} else if( strcmp( type, CADP_VALUE_TYPE_MENU ) && strcmp( type, CADP_VALUE_TYPE_ACTION )){
				g_debug( "%s: unmanaged type: %s", thisfn, type );
				g_free( type );
				ret = FALSE;
			}
		}
		if( ret ){
			g_return_val_if_fail( type && strlen( type ), FALSE );
			ndf->private->type = type;
		}
	}

	g_free( start_group );

	return( ret );
}

/**
 * cadp_desktop_file_get_type:
 * @ndf: the #CappDesktopFile instance.
 *
 * Returns: the value for the Type entry as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
cadp_desktop_file_get_file_type( const CappDesktopFile *ndf )
{
	gchar *type;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	type = NULL;

	if( !ndf->private->dispose_has_run ){

		type = g_strdup( ndf->private->type );
	}

	return( type );
}

/**
 * cadp_desktop_file_get_id:
 * @ndf: the #CappDesktopFile instance.
 *
 * Returns: a newly allocated string which holds the id of the Desktop
 * File.
 */
gchar *
cadp_desktop_file_get_id( const CappDesktopFile *ndf )
{
	gchar *value;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	value = NULL;

	if( !ndf->private->dispose_has_run ){

		value = g_strdup( ndf->private->id );
	}

	return( value );
}

/**
 * cadp_desktop_file_get_profiles:
 * @ndf: the #CappDesktopFile instance.
 *
 * Returns: the list of profiles in the file, as a newly allocated GSList
 * which must be na_core_utils_slist_free() by the caller.
 *
 * Silently ignore unknown groups.
 */
GSList *
cadp_desktop_file_get_profiles( const CappDesktopFile *ndf )
{
	GSList *list;
	gchar **groups, **ig;
	gchar *profile_pfx;
	gchar *profile_id;
	guint pfx_len;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	list = NULL;

	if( !ndf->private->dispose_has_run ){

		groups = g_key_file_get_groups( ndf->private->key_file, NULL );
		if( groups ){
			ig = groups;
			profile_pfx = g_strdup_printf( "%s ", CADP_GROUP_PROFILE );
			pfx_len = strlen( profile_pfx );

			while( *ig ){
				if( !strncmp( *ig, profile_pfx, pfx_len )){
					profile_id = g_strdup( *ig+pfx_len );
					list = g_slist_prepend( list, profile_id );
				}

				ig++;
			}

			g_strfreev( groups );
			g_free( profile_pfx );
		}
	}

	return( list );
}

/**
 * cadp_desktop_file_has_profile:
 * @ndf: the #CappDesktopFile instance.
 * @profile_id: the identifier of the profile.
 *
 * Returns: %TRUE if a group can be found in the .desktop file for this profile,
 * %FALSE else.
 *
 * Since: 3.1
 */
gboolean
cadp_desktop_file_has_profile( const CappDesktopFile *ndf, const gchar *profile_id )
{
	gboolean has_profile;
	gchar *group_name;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), FALSE );
	g_return_val_if_fail( profile_id && g_utf8_strlen( profile_id, -1 ), FALSE );

	has_profile = FALSE;

	if( !ndf->private->dispose_has_run ){

		group_name = g_strdup_printf( "%s %s", CADP_GROUP_PROFILE, profile_id );
		has_profile = g_key_file_has_group( ndf->private->key_file, group_name );
		g_free( group_name );
	}

	return( has_profile );
}

/**
 * cadp_desktop_file_remove_key:
 * @ndf: this #CappDesktopFile instance.
 * @group: the group.
 * @key: the key.
 *
 * Removes the specified key.
 *
 * Note that this doesn't work very well for localized keys, as we only
 * remove a key which has the exact same label that the provided one.
 * So we'd have to remove:
 * - key
 * - key[en_US.UTF-8]
 * - key[en_US]
 * - key[en]
 */
void
cadp_desktop_file_remove_key( const CappDesktopFile *ndf, const gchar *group, const gchar *key )
{
	char **locales;
	char **iloc;
	gchar *locale_key;

	g_return_if_fail( CADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_remove_key( ndf->private->key_file, group, key, NULL );

		locales = ( char ** ) g_get_language_names();
		iloc = locales;

		while( *iloc ){
			locale_key = g_strdup_printf( "%s[%s]", key, *iloc );
			g_key_file_remove_key( ndf->private->key_file, group, locale_key, NULL );
			g_free( locale_key );
			iloc++;
		}
	}
}

/**
 * cadp_desktop_file_remove_profile:
 * @ndf: this #CappDesktopFile instance.
 * @profile_id: the id of the profile.
 *
 * Removes the group which describes the specified profile.
 */
void
cadp_desktop_file_remove_profile( const CappDesktopFile *ndf, const gchar *profile_id )
{
	gchar *group_name;

	g_return_if_fail( CADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		group_name = g_strdup_printf( "%s %s", CADP_GROUP_PROFILE, profile_id );
		g_key_file_remove_group( ndf->private->key_file, group_name, NULL );
		g_free( group_name );
	}
}

/**
 * cadp_desktop_file_get_boolean:
 * @ndf: this #CappDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the read value, or the default value if the entry has not
 * been found in the given group.
 */
gboolean
cadp_desktop_file_get_boolean( const CappDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, gboolean default_value )
{
	static const gchar *thisfn = "cadp_desktop_file_get_boolean";
	gboolean value;
	gboolean read_value;
	gboolean has_entry;
	GError *error;

	value = default_value;
	*key_found = FALSE;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), FALSE );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			read_value = g_key_file_get_boolean( ndf->private->key_file, group, entry, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );

			} else {
				value = read_value;
				*key_found = TRUE;
			}
		}
	}

	return( value );
}

/**
 * cadp_desktop_file_get_locale_string:
 * @ndf: this #CappDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the read value, or the default value if the entry has not
 * been found in the given group.
 */
gchar *
cadp_desktop_file_get_locale_string( const CappDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "cadp_desktop_file_get_locale_string";
	gchar *value;
	gchar *read_value;
	GError *error;

	value = g_strdup( default_value );
	*key_found = FALSE;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		error = NULL;

		read_value = g_key_file_get_locale_string( ndf->private->key_file, group, entry, NULL, &error );
		if( !read_value || error ){
			if( error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );
				g_free( read_value );
			}

		} else {
			g_free( value );
			value = read_value;
			*key_found = TRUE;
		}
	}

	return( value );
}

/**
 * cadp_desktop_file_get_string:
 * @ndf: this #CappDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the read value, or the default value if the entry has not
 * been found in the given group.
 */
gchar *
cadp_desktop_file_get_string( const CappDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "cadp_desktop_file_get_string";
	gchar *value;
	gchar *read_value;
	gboolean has_entry;
	GError *error;

	value = g_strdup( default_value );
	*key_found = FALSE;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			read_value = g_key_file_get_string( ndf->private->key_file, group, entry, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );
				g_free( read_value );

			} else {
				g_free( value );
				value = read_value;
				*key_found = TRUE;
			}
		}
	}

	return( value );
}

/**
 * cadp_desktop_file_get_string_list:
 * @ndf: this #CappDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the read value, or the default value if the entry has not
 * been found in the given group.
 */
GSList *
cadp_desktop_file_get_string_list( const CappDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "cadp_desktop_file_get_string_list";
	GSList *value;
	gchar **read_array;
	gboolean has_entry;
	GError *error;

	value = g_slist_append( NULL, g_strdup( default_value ));
	*key_found = FALSE;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			read_array = g_key_file_get_string_list( ndf->private->key_file, group, entry, NULL, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );

			} else {
				na_core_utils_slist_free( value );
				value = na_core_utils_slist_from_array(( const gchar ** ) read_array );
				*key_found = TRUE;
			}

			g_strfreev( read_array );
		}
	}

	return( value );
}

/**
 * cadp_desktop_file_get_uint:
 * @ndf: this #CappDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the read value, or the default value if the entry has not
 * been found in the given group.
 */
guint
cadp_desktop_file_get_uint( const CappDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, guint default_value )
{
	static const gchar *thisfn = "cadp_desktop_file_get_uint";
	guint value;
	gboolean has_entry;
	GError *error;

	value = default_value;
	*key_found = FALSE;

	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), 0 );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			value = ( guint ) g_key_file_get_integer( ndf->private->key_file, group, entry, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );

			} else {
				*key_found = TRUE;
			}
		}
	}

	return( value );
}

/**
 * cadp_desktop_file_set_boolean:
 * @ndf: this #CappDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given boolean value.
 */
void
cadp_desktop_file_set_boolean( const CappDesktopFile *ndf, const gchar *group, const gchar *key, gboolean value )
{
	g_return_if_fail( CADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_set_boolean( ndf->private->key_file, group, key, value );
	}
}

/**
 * cadp_desktop_file_set_locale_string:
 * @ndf: this #CappDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given string value.
 *
 * Until v 3.0.4, locale strings used to be written for all available locales
 * e.g. 'en_US.UTF-8', 'en_US', 'en.UTF-8', 'en', 'C'.
 *
 * Starting with v 3.0.4, encoding part of the locale is no more written.
 *
 * The better solution would be to include in the UI a listbox with all
 * available locales, letting the user choose himself which locale he wish
 * modify.
 *
 * A first fallback would be to set some sort of user preferences: whether
 * to write all available locales, whether to write all but C locales, ...
 *
 * As of v 3.0.4, we choose:
 * - always write the first locale, which should obviously be the user locale;
 * - also write all locales derived from the first (e.g. en_US, en_GB, en);
 * - when the prefix of the locale changes, stop to write other locales unless
 *   the first prefix was 'en', as we suppose that the C locale will always be
 *   in english.
 *
 * The locale prefix is identified by '_' or '@' character.
 */
void
cadp_desktop_file_set_locale_string( const CappDesktopFile *ndf, const gchar *group, const gchar *key, const gchar *value )
{
	char **locales;
	guint i;
	gchar *prefix;
	gboolean write;

	g_return_if_fail( CADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		locales = ( char ** ) g_get_language_names();
		/*
		en_US.UTF-8
		en_US
		en.UTF-8
		en
		C
		*/

		prefix = g_strdup( locales[0] );
		for( i = 0 ; prefix[i] ; ++i ){
			if( prefix[i] == '_' || prefix[i] == '@' || prefix[i] == '.' ){
				prefix[i] = '\0';
				break;
			}
		}

		/* using locales[0] writes a string with, e.g. Label[en_US.UTF-8]
		 * after that trying to read the same key with another locale, even en_US.utf-8,
		 * fails ans returns an empty string.
		 * so write all available locales for the string, so that there is a chance at
		 * least one of these will be used as default
		 *
		 * pwi 2010-12-30 v 3.0.4
		 * no more write encoding part of the locale as desktop files are supposed to
		 * be UTF-8 encoded.
		 */
		for( i = 0 ; i < g_strv_length( locales ) ; ++i ){
			write = FALSE;

			if( g_strstr_len( locales[i], -1, "." )){
				continue;
			}

			/* write the locale string for all locales derived from the first one */
			if( !strncmp( locales[i], prefix, strlen( prefix ))){
				write = TRUE;

			/* also write other locales if first was a 'en'-derivative */
			} else if( !strcmp( prefix, "en" )){
				write = TRUE;
			}

			if( write ){
				g_key_file_set_locale_string( ndf->private->key_file, group, key, locales[i], value );
			}
		}

		g_free( prefix );
	}
}

/**
 * cadp_desktop_file_set_string:
 * @ndf: this #CappDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given string value.
 */
void
cadp_desktop_file_set_string( const CappDesktopFile *ndf, const gchar *group, const gchar *key, const gchar *value )
{
	g_return_if_fail( CADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_set_string( ndf->private->key_file, group, key, value );
	}
}

/**
 * cadp_desktop_file_set_string_list:
 * @ndf: this #CappDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given list value.
 */
void
cadp_desktop_file_set_string_list( const CappDesktopFile *ndf, const gchar *group, const gchar *key, GSList *value )
{
	gchar **array;

	g_return_if_fail( CADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		array = na_core_utils_slist_to_array( value );
		g_key_file_set_string_list( ndf->private->key_file, group, key, ( const gchar * const * ) array, g_slist_length( value ));
		g_strfreev( array );
	}
}

/**
 * cadp_desktop_file_set_uint:
 * @ndf: this #CappDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given uint value.
 */
void
cadp_desktop_file_set_uint( const CappDesktopFile *ndf, const gchar *group, const gchar *key, guint value )
{
	g_return_if_fail( CADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_set_integer( ndf->private->key_file, group, key, value );
	}
}

/**
 * cadp_desktop_file_write:
 * @ndf: the #CappDesktopFile instance.
 *
 * Writes the key file to the disk.
 *
 * Returns: %TRUE if write is ok, %FALSE else.
 *
 * Starting with v 3.0.4, locale strings whose identifier include an
 * encoding part are removed from the desktop file when rewriting it
 * (these were wrongly written between v 2.99 and 3.0.3).
 */
gboolean
cadp_desktop_file_write( CappDesktopFile *ndf )
{
	static const gchar *thisfn = "cadp_desktop_file_write";
	gboolean ret;
	gchar *data;
	GFile *file;
	GFileOutputStream *stream;
	GError *error;
	gsize length;

	ret = FALSE;
	error = NULL;
	g_return_val_if_fail( CADP_IS_DESKTOP_FILE( ndf ), ret );

	if( !ndf->private->dispose_has_run ){

		if( ndf->private->key_file ){
			remove_encoding_part( ndf );
		}

		data = g_key_file_to_data( ndf->private->key_file, &length, NULL );
		file = g_file_new_for_uri( ndf->private->uri );
		g_debug( "%s: uri=%s", thisfn, ndf->private->uri );

		stream = g_file_replace( file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &error );
		if( error ){
			g_warning( "%s: g_file_replace: %s", thisfn, error->message );
			g_error_free( error );
			if( stream ){
				g_object_unref( stream );
			}
			g_object_unref( file );
			g_free( data );
			return( FALSE );
		}

		g_output_stream_write( G_OUTPUT_STREAM( stream ), data, length, NULL, &error );
		if( error ){
			g_warning( "%s: g_output_stream_write: %s", thisfn, error->message );
			g_error_free( error );
			g_object_unref( stream );
			g_object_unref( file );
			g_free( data );
			return( FALSE );
		}

		g_output_stream_close( G_OUTPUT_STREAM( stream ), NULL, &error );
		if( error ){
			g_warning( "%s: g_output_stream_close: %s", thisfn, error->message );
			g_error_free( error );
			g_object_unref( stream );
			g_object_unref( file );
			g_free( data );
			return( FALSE );
		}

		g_object_unref( stream );
		g_object_unref( file );
		g_free( data );

		return( TRUE );
	}

	return( FALSE );
}

static void
remove_encoding_part( CappDesktopFile *ndf )
{
	static const gchar *thisfn = "cadp_desktop_file_remove_encoding_part";
	gchar **groups;
	gchar **keys;
	guint ig, ik;
	GRegex *regex;
	GMatchInfo *info;
	GError *error;

	error = NULL;
	regex = g_regex_new( "\\[(.*)\\.(.*)\\]$", G_REGEX_CASELESS | G_REGEX_UNGREEDY, G_REGEX_MATCH_NOTEMPTY, &error );
	if( error ){
		g_warning( "%s: %s", thisfn, error->message );
		g_error_free( error );

	} else {
		groups = g_key_file_get_groups( ndf->private->key_file, NULL );

		for( ig = 0 ; ig < g_strv_length( groups ) ; ++ig ){
			keys = g_key_file_get_keys( ndf->private->key_file, groups[ig], NULL, NULL );

			for( ik = 0 ; ik < g_strv_length( keys ) ; ++ik ){

				if( g_regex_match( regex, keys[ik], 0, &info )){
					g_key_file_remove_key( ndf->private->key_file, groups[ig], keys[ik], &error );
					if( error ){
						g_warning( "%s: %s", thisfn, error->message );
						g_error_free( error );
						error = NULL;
					}
				}

				g_match_info_free( info );
			}

			g_strfreev( keys );
		}

		g_strfreev( groups );
		g_regex_unref( regex );
	}
}
