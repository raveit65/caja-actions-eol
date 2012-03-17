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

#include <gio/gio.h>

#include <api/na-core-utils.h>

#include "nadp-desktop-file.h"
#include "nadp-keys.h"

/* private class data
 */
struct NadpDesktopFileClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NadpDesktopFilePrivate {
	gboolean   dispose_has_run;
	gchar     *id;
	gchar     *path;
	GKeyFile  *key_file;
};

static GObjectClass *st_parent_class = NULL;

static GType            register_type( void );
static void             class_init( NadpDesktopFileClass *klass );
static void             instance_init( GTypeInstance *instance, gpointer klass );
static void             instance_dispose( GObject *object );
static void             instance_finalize( GObject *object );

static NadpDesktopFile *ndf_new( const gchar *path );
static gchar           *path2id( const gchar *path );
static gboolean         check_key_file( NadpDesktopFile *ndf );

GType
nadp_desktop_file_get_type( void )
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
	static const gchar *thisfn = "nadp_desktop_file_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NadpDesktopFileClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NadpDesktopFile ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NadpDesktopFile", &info, 0 );

	return( type );
}

static void
class_init( NadpDesktopFileClass *klass )
{
	static const gchar *thisfn = "nadp_desktop_file_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NadpDesktopFileClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "nadp_desktop_file_instance_init";
	NadpDesktopFile *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( NADP_IS_DESKTOP_FILE( instance ));
	self = NADP_DESKTOP_FILE( instance );

	self->private = g_new0( NadpDesktopFilePrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->key_file = g_key_file_new();
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "nadp_desktop_file_instance_dispose";
	NadpDesktopFile *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NADP_IS_DESKTOP_FILE( object ));
	self = NADP_DESKTOP_FILE( object );

	if( !self->private->dispose_has_run ){

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
	NadpDesktopFile *self;

	g_assert( NADP_IS_DESKTOP_FILE( object ));
	self = NADP_DESKTOP_FILE( object );

	g_free( self->private->id );
	g_free( self->private->path );

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
 * nadp_desktop_file_new_from_path:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #NadpDesktopFile object.
 *
 * Key file has been loaded, and first validity checks made.
 */
NadpDesktopFile *
nadp_desktop_file_new_from_path( const gchar *path )
{
	static const gchar *thisfn = "nadp_desktop_file_new_from_path";
	NadpDesktopFile *ndf;
	GError *error;

	ndf = NULL;
	g_debug( "%s: path=%s", thisfn, path );
	g_return_val_if_fail( path && g_utf8_strlen( path, -1 ) && g_path_is_absolute( path ), ndf );

	ndf = ndf_new( path );

	error = NULL;
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
 * nadp_desktop_file_new_for_write:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #NadpDesktopFile object.
 */
NadpDesktopFile *
nadp_desktop_file_new_for_write( const gchar *path )
{
	static const gchar *thisfn = "nadp_desktop_file_new_for_write";
	NadpDesktopFile *ndf;

	ndf = NULL;
	g_debug( "%s: path=%s", thisfn, path );
	g_return_val_if_fail( path && g_utf8_strlen( path, -1 ) && g_path_is_absolute( path ), ndf );

	ndf = ndf_new( path );

	return( ndf );
}

/**
 * nadp_desktop_file_get_key_file_path:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the full pathname of the key file, as a newly allocated
 * string which should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_key_file_path( const NadpDesktopFile *ndf )
{
	gchar *path;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	path = NULL;

	if( !ndf->private->dispose_has_run ){

		path = g_strdup( ndf->private->path );
	}

	return( path );
}

/*
 * ndf_new:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #NadpDesktopFile object.
 */
static NadpDesktopFile *
ndf_new( const gchar *path )
{
	NadpDesktopFile *ndf;

	ndf = g_object_new( NADP_DESKTOP_FILE_TYPE, NULL );

	ndf->private->id = path2id( path );
	ndf->private->path = g_strdup( path );

	return( ndf );
}

/*
 * path2id:
 * @path: a full pathname.
 *
 * Returns: the id of the file, as a newly allocated string which
 * should be g_free() by the caller.
 *
 * The id of the file is equal to the basename, minus the suffix.
 */
static gchar *
path2id( const gchar *path )
{
	gchar *bname;
	gchar *id;

	bname = g_path_get_basename( path );
	id = na_core_utils_str_remove_suffix( bname, NADP_DESKTOP_FILE_SUFFIX );
	g_free( bname );

	return( id );
}

static gboolean
check_key_file( NadpDesktopFile *ndf )
{
	static const gchar *thisfn = "nadp_desktop_file_check_key_file";
	gboolean ret;
	gchar *start_group;
	gboolean has_key;
	gboolean hidden;
	GError *error;

	ret = TRUE;
	error = NULL;

	/* start group must be 'Desktop Entry' */
	start_group = g_key_file_get_start_group( ndf->private->key_file );
	if( strcmp( start_group, NADP_GROUP_DESKTOP )){
		g_warning( "%s: %s: invalid start group, found %s, waited for %s",
				thisfn, ndf->private->path, start_group, NADP_GROUP_DESKTOP );
		ret = FALSE;
	}

	/* must not have Hidden=true value */
	if( ret ){
		has_key = g_key_file_has_key( ndf->private->key_file, start_group, NADP_KEY_HIDDEN, &error );
		if( error ){
			g_warning( "%s: %s: %s", thisfn, ndf->private->path, error->message );
			ret = FALSE;

		} else if( has_key ){
			hidden = g_key_file_get_boolean( ndf->private->key_file, start_group, NADP_KEY_HIDDEN, &error );
			if( error ){
				g_warning( "%s: %s: %s", thisfn, ndf->private->path, error->message );
				ret = FALSE;

			} else if( hidden ){
				g_debug( "%s: %s: Hidden=true", thisfn, ndf->private->path );
				ret = FALSE;
			}
		}
	}

	g_free( start_group );

	return( ret );
}

/**
 * nadp_desktop_file_get_type:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the value for the Type entry as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_file_type( const NadpDesktopFile *ndf )
{
	static const gchar *thisfn = "nadp_desktop_file_get_file_type";
	gchar *type;
	gboolean has_key;
	GError *error;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	type = NULL;

	if( !ndf->private->dispose_has_run ){

		error = NULL;

		has_key = g_key_file_has_key( ndf->private->key_file, NADP_GROUP_DESKTOP, NADP_KEY_TYPE, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_key ){
			type = g_key_file_get_string( ndf->private->key_file, NADP_GROUP_DESKTOP, NADP_KEY_TYPE, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );
				g_free( type );
				type = NULL;
			}
		}
	}

	return( type );
}

/**
 * nadp_desktop_file_get_id:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: a newly allocated string which holds the id of the Desktop
 * File.
 */
gchar *
nadp_desktop_file_get_id( const NadpDesktopFile *ndf )
{
	gchar *value;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	value = NULL;

	if( !ndf->private->dispose_has_run ){

		value = g_strdup( ndf->private->id );
	}

	return( value );
}

/**
 * nadp_desktop_file_get_profiles:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the list of profiles in the file, as a newly allocated GSList
 * which must be na_core_utils_slist_free() by the caller.
 *
 * Silently ignore unknown groups.
 */
GSList *
nadp_desktop_file_get_profiles( const NadpDesktopFile *ndf )
{
	GSList *list;
	gchar **groups, **ig;
	gchar *profile_pfx;
	gchar *profile_id;
	guint pfx_len;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	list = NULL;

	if( !ndf->private->dispose_has_run ){

		groups = g_key_file_get_groups( ndf->private->key_file, NULL );
		if( groups ){
			ig = groups;
			profile_pfx = g_strdup_printf( "%s ", NADP_GROUP_PROFILE );
			pfx_len = strlen( profile_pfx );

			while( *ig ){

				if( !strncmp( *ig, profile_pfx, pfx_len )){
					profile_id = g_strdup( *ig );
					list = g_slist_prepend( list, profile_id+pfx_len );
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
 * nadp_desktop_file_remove_key:
 * @ndf: this #NadpDesktopFile instance.
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
nadp_desktop_file_remove_key( const NadpDesktopFile *ndf, const gchar *group, const gchar *key )
{
	char **locales;
	char **iloc;
	gchar *locale_key;

	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

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
 * nadp_desktop_file_get_boolean:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
gboolean
nadp_desktop_file_get_boolean( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, gboolean default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_boolean";
	gboolean value;
	gboolean read_value;
	gboolean has_entry;
	GError *error;

	value = default_value;
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), FALSE );

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
 * nadp_desktop_file_get_locale_string:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 *
 * Note that g_key_file_has_key doesn't deal correctly with localized
 * strings which have a key[modifier] (it recognizes them as the key
 *  "key[modifier]", not "key")
 */
gchar *
nadp_desktop_file_get_locale_string( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_locale_string";
	gchar *value;
	gchar *read_value;
	GError *error;

	value = g_strdup( default_value );
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

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
 * nadp_desktop_file_get_string:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
gchar *
nadp_desktop_file_get_string( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_string";
	gchar *value;
	gchar *read_value;
	gboolean has_entry;
	GError *error;

	value = g_strdup( default_value );
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

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
 * nadp_desktop_file_get_string_list:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
GSList *
nadp_desktop_file_get_string_list( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_string_list";
	GSList *value;
	gchar **read_array;
	gboolean has_entry;
	GError *error;

	value = g_slist_append( NULL, g_strdup( default_value ));
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

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
 * nadp_desktop_file_get_uint:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
guint
nadp_desktop_file_get_uint( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, guint default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_uint";
	guint value;
	gboolean has_entry;
	GError *error;

	value = default_value;
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), 0 );

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
 * nadp_desktop_file_set_boolean:
 * @ndf: this #NadpDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given boolean value.
 */
void
nadp_desktop_file_set_boolean( const NadpDesktopFile *ndf, const gchar *group, const gchar *key, gboolean value )
{
	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_set_boolean( ndf->private->key_file, group, key, value );
	}
}

/**
 * nadp_desktop_file_set_locale_string:
 * @ndf: this #NadpDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given string value.
 */
void
nadp_desktop_file_set_locale_string( const NadpDesktopFile *ndf, const gchar *group, const gchar *key, const gchar *value )
{
	char **locales;

	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		locales = ( char ** ) g_get_language_names();
		/*
		en_US.UTF-8
		en_US
		en.UTF-8
		en
		C
		*/
		g_key_file_set_locale_string( ndf->private->key_file, group, key, locales[0], value );
	}
}

/**
 * nadp_desktop_file_set_string:
 * @ndf: this #NadpDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given string value.
 */
void
nadp_desktop_file_set_string( const NadpDesktopFile *ndf, const gchar *group, const gchar *key, const gchar *value )
{
	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_set_string( ndf->private->key_file, group, key, value );
	}
}

/**
 * nadp_desktop_file_set_string_list:
 * @ndf: this #NadpDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given list value.
 */
void
nadp_desktop_file_set_string_list( const NadpDesktopFile *ndf, const gchar *group, const gchar *key, GSList *value )
{
	gchar **array;

	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		array = na_core_utils_slist_to_array( value );
		g_key_file_set_string_list( ndf->private->key_file, group, key, ( const gchar * const * ) array, g_slist_length( value ));
		g_strfreev( array );
	}
}

/**
 * nadp_desktop_file_set_uint:
 * @ndf: this #NadpDesktopFile object.
 * @group: the name of the group.
 * @key: the name of the key.
 * @value: the value to be written.
 *
 * Write the given uint value.
 */
void
nadp_desktop_file_set_uint( const NadpDesktopFile *ndf, const gchar *group, const gchar *key, guint value )
{
	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_set_integer( ndf->private->key_file, group, key, value );
	}
}

/**
 * nadp_desktop_file_write:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Writes the key file to the disk.
 *
 * Returns: %TRUE if write is ok, %FALSE else.
 */
gboolean
nadp_desktop_file_write( NadpDesktopFile *ndf )
{
	static const gchar *thisfn = "nadp_desktop_file_write";
	gboolean ret;
	gchar *data;
	GFile *file;
	GFileOutputStream *stream;
	GError *error;

	ret = FALSE;
	error = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), ret );

	if( !ndf->private->dispose_has_run ){

		data = g_key_file_to_data( ndf->private->key_file, NULL, NULL );
		file = g_file_new_for_path( ndf->private->path );

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

		g_output_stream_write( G_OUTPUT_STREAM( stream ), data, g_utf8_strlen( data, -1 ), NULL, &error );
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
	}

	return( TRUE );
}
