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

#ifdef HAVE_MATECONF

#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-mateconf-utils.h>

static void        dump_entry( MateConfEntry *entry, void *user_data );
static MateConfValue *read_value( MateConfClient *mateconf, const gchar *path, gboolean use_schema, MateConfValueType type );

#ifdef NA_ENABLE_DEPRECATED
static gboolean    sync_mateconf( MateConfClient *mateconf, gchar **message );
#endif /* NA_ENABLE_DEPRECATED */

/**
 * na_mateconf_utils_get_subdirs:
 * @mateconf: a MateConfClient instance.
 * @path: a full path to be read.
 *
 * Returns: a list of full path subdirectories.
 *
 * The returned list should be na_mateconf_utils_free_subdirs() by the caller.
 *
 * Since: 2.30
 */
GSList *
na_mateconf_utils_get_subdirs( MateConfClient *mateconf, const gchar *path )
{
	static const gchar *thisfn = "na_mateconf_utils_get_subdirs";
	GError *error = NULL;
	GSList *list_subdirs;

	list_subdirs = mateconf_client_all_dirs( mateconf, path, &error );

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		return(( GSList * ) NULL );
	}

	return( list_subdirs );
}

/**
 * na_mateconf_utils_free_subdirs:
 * @subdirs: the subdirectory list as returned from na_mateconf_utils_get_subdirs().
 *
 * Release the list.
 *
 * Since: 2.30
 */
void
na_mateconf_utils_free_subdirs( GSList *subdirs )
{
	na_core_utils_slist_free( subdirs );
}

/**
 * na_mateconf_utils_has_entry:
 * @entries: the list of entries as returned by na_mateconf_utils_get_entries().
 * @entry: the entry to be tested.
 *
 * Returns: %TRUE if the given @entry exists in the specified @entries,
 * %FALSE else.
 *
 * Since: 2.30
 */
gboolean
na_mateconf_utils_has_entry( GSList *entries, const gchar *entry )
{
	GSList *ie;

	for( ie = entries ; ie ; ie = ie->next ){
		gchar *key = g_path_get_basename( mateconf_entry_get_key( ( MateConfEntry * ) ie->data));
		int res = strcmp( key, entry );
		g_free( key );
		if( res == 0 ){
			return( TRUE );
		}
	}

	return( FALSE );
}

/**
 * na_mateconf_utils_get_entries:
 * @mateconf: a  MateConfClient instance.
 * @path: a full path to be read.
 *
 * Loads all the key=value pairs of the specified key.
 *
 * Returns: a list of #MateConfEntry.
 *
 * The returned list is not recursive : it contains only the immediate
 * children of @path. To free the returned list, call
 * na_mateconf_utils_free_entries().
 *
 * Since: 2.30
 */
GSList *
na_mateconf_utils_get_entries( MateConfClient *mateconf, const gchar *path )
{
	static const gchar *thisfn = "na_mateconf_utils_get_entries";
	GError *error = NULL;
	GSList *list_entries;

	list_entries = mateconf_client_all_entries( mateconf, path, &error );

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		return(( GSList * ) NULL );
	}

	return( list_entries );
}

/**
 * na_mateconf_utils_get_bool_from_entries:
 * @entries: a list of #MateConfEntry as returned by na_mateconf_utils_get_entries().
 * @entry: the searched entry.
 * @value: a pointer to a gboolean to be set to the found value.
 *
 * Returns: %TRUE if the entry was found, %FALSE else.
 *
 * If the entry was not found, or was not of boolean type, @value is set
 * to %FALSE.
 *
 * Since: 2.30
 */
gboolean
na_mateconf_utils_get_bool_from_entries( GSList *entries, const gchar *entry, gboolean *value )
{
	GSList *ip;
	MateConfEntry *mateconf_entry;
	MateConfValue *mateconf_value;
	gchar *key;
	gboolean found;

	g_return_val_if_fail( value, FALSE );

	*value = FALSE;
	found = FALSE;

	for( ip = entries ; ip && !found ; ip = ip->next ){
		mateconf_entry = ( MateConfEntry * ) ip->data;
		key = g_path_get_basename( mateconf_entry_get_key( mateconf_entry ));

		if( !strcmp( key, entry )){
			mateconf_value = mateconf_entry_get_value( mateconf_entry );

			if( mateconf_value &&
				mateconf_value->type == MATECONF_VALUE_BOOL ){

					found = TRUE;
					*value = mateconf_value_get_bool( mateconf_value );
			}
		}
		g_free( key );
	}

	return( found );
}

/**
 * na_mateconf_utils_get_string_from_entries:
 * @entries: a list of #MateConfEntry as returned by na_mateconf_utils_get_entries().
 * @entry: the searched entry.
 * @value: a pointer to a gchar * to be set to the found value.
 *
 * Returns: %TRUE if the entry was found, %FALSE else.
 *
 * If the entry was not found, or was not of string type, @value is set
 * to %NULL.
 *
 * If @value is returned not NULL, it should be g_free() by the caller.
 *
 * Since: 2.30
 */
gboolean
na_mateconf_utils_get_string_from_entries( GSList *entries, const gchar *entry, gchar **value )
{
	GSList *ip;
	MateConfEntry *mateconf_entry;
	MateConfValue *mateconf_value;
	gchar *key;
	gboolean found;

	g_return_val_if_fail( value, FALSE );

	*value = NULL;
	found = FALSE;

	for( ip = entries ; ip && !found ; ip = ip->next ){
		mateconf_entry = ( MateConfEntry * ) ip->data;
		key = g_path_get_basename( mateconf_entry_get_key( mateconf_entry ));

		if( !strcmp( key, entry )){
			mateconf_value = mateconf_entry_get_value( mateconf_entry );

			if( mateconf_value &&
				mateconf_value->type == MATECONF_VALUE_STRING ){

					found = TRUE;
					*value = g_strdup( mateconf_value_get_string( mateconf_value ));
			}
		}
		g_free( key );
	}

	return( found );
}

/**
 * na_mateconf_utils_get_string_list_from_entries:
 * @entries: a list of #MateConfEntry as returned by na_mateconf_utils_get_entries().
 * @entry: the searched entry.
 * @value: a pointer to a GSList * to be set to the found value.
 *
 * Returns: %TRUE if the entry was found, %FALSE else.
 *
 * If the entry was not found, or was not of string list type, @value
 * is set to %NULL.
 *
 * If @value is returned not NULL, it should be na_core_utils_slist_free()
 * by the caller.
 *
 * Since: 2.30
 */
gboolean
na_mateconf_utils_get_string_list_from_entries( GSList *entries, const gchar *entry, GSList **value )
{
	GSList *ip, *iv;
	MateConfEntry *mateconf_entry;
	MateConfValue *mateconf_value;
	gchar *key;
	gboolean found;
	GSList *list_values;

	g_return_val_if_fail( value, FALSE );

	*value = NULL;
	found = FALSE;

	for( ip = entries ; ip && !found ; ip = ip->next ){
		mateconf_entry = ( MateConfEntry * ) ip->data;
		key = g_path_get_basename( mateconf_entry_get_key( mateconf_entry ));

		if( !strcmp( key, entry )){
			mateconf_value = mateconf_entry_get_value( mateconf_entry );

			if( mateconf_value &&
				mateconf_value->type == MATECONF_VALUE_LIST ){

					found = TRUE;
					list_values = mateconf_value_get_list( mateconf_value );
					for( iv = list_values ; iv ; iv = iv->next ){
						*value = g_slist_append( *value, g_strdup( mateconf_value_get_string(( MateConfValue * ) iv->data )));
					}
			}
		}
		g_free( key );
	}

	return( found );
}

/**
 * na_mateconf_utils_dump_entries:
 * @entries: a list of #MateConfEntry as returned by na_mateconf_utils_get_entries().
 *
 * Dumps the content of the entries.
 *
 * Since: 2.30
 */
void
na_mateconf_utils_dump_entries( GSList *entries )
{
	g_slist_foreach( entries, ( GFunc ) dump_entry, NULL );
}

static void
dump_entry( MateConfEntry *entry, void *user_data )
{
	static const gchar *thisfn = "na_mateconf_utils_dump_entry";
	gchar *str = NULL;
	gboolean str_free = FALSE;
	GSList *value_list, *it;
	MateConfValueType type_list;
	GString *string;

	gchar *key = g_path_get_basename( mateconf_entry_get_key( entry ));
	MateConfValue *value = mateconf_entry_get_value( entry );

	if( value ){
		switch( value->type ){
			case MATECONF_VALUE_STRING:
				str = ( gchar * ) mateconf_value_get_string( value );
				break;

			case MATECONF_VALUE_INT:
				str = g_strdup_printf( "%d", mateconf_value_get_int( value ));
				str_free = TRUE;
				break;

			case MATECONF_VALUE_FLOAT:
				str = g_strdup_printf( "%f", mateconf_value_get_float( value ));
				str_free = TRUE;
				break;

			case MATECONF_VALUE_BOOL:
				str = g_strdup_printf( "%s", mateconf_value_get_bool( value ) ? "True":"False" );
				str_free = TRUE;
				break;

			case MATECONF_VALUE_LIST:
				type_list = mateconf_value_get_list_type( value );
				value_list = mateconf_value_get_list( value );
				switch( type_list ){
					case MATECONF_VALUE_STRING:
						string = g_string_new( "[" );
						for( it = value_list ; it ; it = it->next ){
							if( g_utf8_strlen( string->str, -1 ) > 1 ){
								string = g_string_append( string, "," );
							}
							string = g_string_append( string, ( const gchar * ) mateconf_value_get_string( it->data ));
						}
						string = g_string_append( string, "]" );
						str = g_string_free( string, FALSE );
						break;
					default:
						str = g_strdup( "(undetermined value)" );
				}
				str_free = TRUE;
				break;

			default:
				str = g_strdup( "(undetermined value)" );
				str_free = TRUE;
		}
	}

	g_debug( "%s: key=%s, value=%s", thisfn, key, str );

	if( str_free ){
		g_free( str );
	}

	g_free( key );
}

/**
 * na_mateconf_utils_free_entries:
 * @entries: a list of #MateConfEntry as returned by na_mateconf_utils_get_entries().
 *
 * Releases the provided list.
 *
 * Since: 2.30
 */
void
na_mateconf_utils_free_entries( GSList *entries )
{
	g_slist_foreach( entries, ( GFunc ) mateconf_entry_unref, NULL );
	g_slist_free( entries );
}

/**
 * na_mateconf_utils_read_bool:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key.
 * @use_schema: whether to use the default value from schema, or not.
 * @default_value: default value to be used if schema is not used or
 * doesn't exist.
 *
 * Returns: the required boolean value.
 *
 * Since: 2.30
 */
gboolean
na_mateconf_utils_read_bool( MateConfClient *mateconf, const gchar *path, gboolean use_schema, gboolean default_value )
{
	MateConfValue *value;
	gboolean ret;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), FALSE );

	ret = default_value;

	value = read_value( mateconf, path, use_schema, MATECONF_VALUE_BOOL );
	if( value ){
		ret = mateconf_value_get_bool( value );
		mateconf_value_free( value );
	}

	return( ret );
}

/**
 * na_mateconf_utils_read_int:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key.
 * @use_schema: whether to use the default value from schema, or not.
 * @default_value: default value to be used if schema is not used or
 * doesn't exist.
 *
 * Returns: the required integer value.
 *
 * Since: 2.30
 */
gint
na_mateconf_utils_read_int( MateConfClient *mateconf, const gchar *path, gboolean use_schema, gint default_value )
{
	MateConfValue *value = NULL;
	gint ret;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), FALSE );

	ret = default_value;

	value = read_value( mateconf, path, use_schema, MATECONF_VALUE_INT );

	if( value ){
		ret = mateconf_value_get_int( value );
		mateconf_value_free( value );
	}

	return( ret );
}

/**
 * na_mateconf_utils_read_string:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key.
 * @use_schema: whether to use the default value from schema, or not.
 * @default_value: default value to be used if schema is not used or
 * doesn't exist.
 *
 * Returns: the required string value in a newly allocated string which
 * should be g_free() by the caller.
 *
 * Since: 2.30
 */
gchar *
na_mateconf_utils_read_string( MateConfClient *mateconf, const gchar *path, gboolean use_schema, const gchar *default_value )
{
	MateConfValue *value = NULL;
	gchar *result;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), NULL );

	result = g_strdup( default_value );

	value = read_value( mateconf, path, use_schema, MATECONF_VALUE_STRING );

	if( value ){
		g_free( result );
		result = g_strdup( mateconf_value_get_string( value ));
		mateconf_value_free( value );
	}

	return( result );
}

/**
 * na_mateconf_utils_read_string_list:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key to be read.
 *
 * Returns: a list of strings,
 * or %NULL if the entry was not found or was not of string list type.
 *
 * The returned list must be released with na_core_utils_slist_free().
 *
 * Since: 2.30
 */
GSList *
na_mateconf_utils_read_string_list( MateConfClient *mateconf, const gchar *path )
{
	static const gchar *thisfn = "na_mateconf_utils_read_string_list";
	GError *error = NULL;
	GSList *list_strings;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), NULL );

	list_strings = mateconf_client_get_list( mateconf, path, MATECONF_VALUE_STRING, &error );

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		return( NULL );
	}

	return( list_strings );
}

#ifdef NA_ENABLE_DEPRECATED
/**
 * na_mateconf_utils_write_bool:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key.
 * @value: the value to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes a boolean at the given @path.
 *
 * Returns: %TRUE if the writing has been successful, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 *
 * Since: 2.30
 * Deprecated: 3.1
 */
gboolean
na_mateconf_utils_write_bool( MateConfClient *mateconf, const gchar *path, gboolean value, gchar **message )
{
	static const gchar *thisfn = "na_mateconf_utils_write_bool";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), FALSE );

	if( !mateconf_client_set_bool( mateconf, path, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%s, error=%s", thisfn, path, value ? "True":"False", error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}

/**
 * na_mateconf_utils_write_int:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key.
 * @value: the value to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes an integer at the given @path.
 *
 * Returns: %TRUE if the writing has been successful, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 *
 * Since: 2.30
 * Deprecated: 3.1
 */
gboolean
na_mateconf_utils_write_int( MateConfClient *mateconf, const gchar *path, gint value, gchar **message )
{
	static const gchar *thisfn = "na_mateconf_utils_write_int";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), FALSE );

	if( !mateconf_client_set_int( mateconf, path, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%d, error=%s", thisfn, path, value, error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}

/**
 * na_mateconf_utils_write_string:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key.
 * @value: the value to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes a string at the given @path.
 *
 * Returns: %TRUE if the writing has been successful, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 *
 * Since: 2.30
 * Deprecated: 3.1
 */
gboolean
na_mateconf_utils_write_string( MateConfClient *mateconf, const gchar *path, const gchar *value, gchar **message )
{
	static const gchar *thisfn = "na_mateconf_utils_write_string";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), FALSE );

	if( !mateconf_client_set_string( mateconf, path, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%s, error=%s", thisfn, path, value, error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}

/**
 * na_mateconf_utils_write_string_list:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the key.
 * @value: the list of values to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes a list of strings at the given @path.
 *
 * Returns: %TRUE if the writing has been successful, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 *
 * Since: 2.30
 * Deprecated: 3.1
 */
gboolean
na_mateconf_utils_write_string_list( MateConfClient *mateconf, const gchar *path, GSList *value, gchar **message )
{
	static const gchar *thisfn = "na_mateconf_utils_write_string_list";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), FALSE );

	if( !mateconf_client_set_list( mateconf, path, MATECONF_VALUE_STRING, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%p (count=%d), error=%s",
				thisfn, path, ( void * ) value, g_slist_length( value ), error->message );
		g_error_free( error );
		ret = FALSE;
	}

	if( ret ){
		ret = sync_mateconf( mateconf, message );
	}

	return( ret );
}

/**
 * na_mateconf_utils_remove_entry:
 * @mateconf: a MateConfClient instance.
 * @path: the full path to the entry.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Removes an entry from user preferences.
 *
 * Returns: %TRUE if the operation was successful, %FALSE else.
 *
 * Since: 2.30
 * Deprecated: 3.1
 */
gboolean
na_mateconf_utils_remove_entry( MateConfClient *mateconf, const gchar *path, gchar **message )
{
	static const gchar *thisfn = "na_mateconf_utils_remove_entry";
	gboolean ret;
	GError *error = NULL;

	g_return_val_if_fail( MATECONF_IS_CLIENT( mateconf ), FALSE );

	ret = mateconf_client_unset( mateconf, path, &error );
	if( !ret ){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
	}

	if( ret ){
		ret = sync_mateconf( mateconf, message );
	}

	return( ret );
}

/**
 * na_mateconf_utils_slist_from_string:
 * @value: a string of the form [xxx,yyy,...] as read from MateConf.
 *
 * Converts a string representing a list of strings in a MateConf format
 * to a list of strings.
 *
 * Returns: a newly allocated list of strings, which should be
 * na_core_utils_slist_free() by the caller, or %NULL if the provided
 * string was not of the MateConf form.
 *
 * Since: 2.30
 * Deprecated: 3.1
 */
GSList *
na_mateconf_utils_slist_from_string( const gchar *value )
{
	GSList *slist;
	gchar *tmp_string;

	tmp_string = g_strdup( value );
	g_strstrip( tmp_string );

	if( !tmp_string || strlen( tmp_string ) < 3 ){
		g_free( tmp_string );
		return( NULL );
	}

	if( tmp_string[0] != '[' || tmp_string[strlen(tmp_string)-1] != ']' ){
		g_free( tmp_string );
		return( NULL );
	}

	tmp_string += 1;
	tmp_string[strlen(tmp_string)-1] = '\0';
	slist = na_core_utils_slist_from_split( tmp_string, "," );

	return( slist );
}

/**
 * na_mateconf_utils_slist_to_string:
 * @slist: a #GSList to be displayed.
 *
 * Returns: the content of @slist, with the MateConf format, as a newly
 * allocated string which should be g_free() by the caller.
 *
 * Since: 2.30
 * Deprecated: 3.1
 */
gchar *
na_mateconf_utils_slist_to_string( GSList *slist )
{
	GSList *is;
	GString *str = g_string_new( "[" );
	gboolean first;

	first = TRUE;
	for( is = slist ; is ; is = is->next ){
		if( !first ){
			str = g_string_append( str, "," );
		}
		str = g_string_append( str, ( const gchar * ) is->data );
		first = FALSE;
	}

	str = g_string_append( str, "]" );

	return( g_string_free( str, FALSE ));
}
#endif /* NA_ENABLE_DEPRECATED */

static MateConfValue *
read_value( MateConfClient *mateconf, const gchar *path, gboolean use_schema, MateConfValueType type )
{
	static const gchar *thisfn = "na_mateconf_utils_read_value";
	GError *error = NULL;
	MateConfValue *value = NULL;

	if( use_schema ){
		value = mateconf_client_get( mateconf, path, &error );
	} else {
		value = mateconf_client_get_without_default( mateconf, path, &error );
	}

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		if( value ){
			mateconf_value_free( value );
			value = NULL;
		}
	}

	if( value ){
		if( value->type != type ){
			g_warning( "%s: path=%s, found type '%u' while waiting for type '%u'", thisfn, path, value->type, type );
			mateconf_value_free( value );
			value = NULL;
		}
	}

	return( value );
}

#ifdef NA_ENABLE_DEPRECATED
static gboolean
sync_mateconf( MateConfClient *mateconf, gchar **message )
{
	static const gchar *thisfn = "na_mateconf_utils_sync_mateconf";
	gboolean ret = TRUE;
	GError *error = NULL;

	mateconf_client_suggest_sync( mateconf, &error );
	if( error ){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: error=%s", thisfn, error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}
#endif /* NA_ENABLE_DEPRECATED */

#endif /* HAVE_MATECONF */
