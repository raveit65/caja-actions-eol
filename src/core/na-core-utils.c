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

#include <gio/gio.h>
#include <glib/gstdio.h>

#include <api/na-core-utils.h>

#include "na-iabout.h"

static GSList  *text_to_string_list( const gchar *text, const gchar *separator, const gchar *default_value );
static gboolean info_dir_is_writable( GFile *file, const gchar *path );

/**
 * na_core_utils_boolean_from_string
 * @string: a string to be converted.
 *
 * Returns: %TRUE if the string evaluates to "true" (case insensitive),
 * %FALSE else.
 */
gboolean
na_core_utils_boolean_from_string( const gchar *string )
{
	return( g_ascii_strcasecmp( string, "true" ) == 0 );
}

/**
 * na_core_utils_str_add_prefix:
 * @prefix: the prefix to be prepended.
 * @str: a multiline string.
 *
 * Appends a prefix to each line of the string.
 *
 * Returns: a new string which should be g_free() by the caller.
 */
gchar *
na_core_utils_str_add_prefix( const gchar *prefix, const gchar *str )
{
	GSList *list, *il;
	GString *result;

	list = text_to_string_list( str, "\n", NULL );
	result = g_string_new( "" );

	for( il = list ; il ; il = il->next ){
		g_string_append_printf( result, "%s%s\n", prefix, ( gchar * ) il->data );
	}

	na_core_utils_slist_free( list );

	return( g_string_free( result, FALSE ));
}

/**
 * na_core_utils_str_collate:
 * @str1: an UTF-8 encoded string.
 * @str2: an UTF-8 encoded string.
 *
 * Returns: -1 if str1 < str2, 0 if str1 = str2, +1 if str1 > str2.
 */
int
na_core_utils_str_collate( const gchar *str1, const gchar *str2 )
{
	int res;

	if( str1 && str2 ){
		res = g_utf8_collate( str1, str2 );
	} else if( !str1 && !str2 ){
		res = 0;
	} else if( !str1 ){
		res = -1;
	} else {
		g_return_val_if_fail( str2 == NULL, 0 );
		res = 1;
	}
	return( res );
}

/**
 * na_core_utils_str_get_first_word:
 * @string: a space-separated string.
 *
 * Returns: the first word of @string, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
na_core_utils_str_get_first_word( const gchar *string )
{
	gchar **splitted, **iter;
	gchar *word, *tmp;

	splitted = g_strsplit( string, " ", 0 );
	iter = splitted;
	word = NULL;

	while( *iter ){
		tmp = g_strstrip( *iter );
		if( g_utf8_strlen( tmp, -1 )){
			word = g_strdup( tmp );
			break;
		}
		iter++;
	}

	g_strfreev( splitted );
	return( word );
}

/**
 * na_core_utils_str_remove_char:
 * @string: source string.
 * @to_remove: the character to remove.
 *
 * Returns: a newly allocated string, which is a copy of the source @string,
 * minus all the found occurrences of the given @to_remove char.
 *
 * The returned string should be g_free() by the caller.
 */
gchar *
na_core_utils_str_remove_char( const gchar *string, const gchar *to_remove )
{
	static const gchar *thisfn = "na_core_utils_str_remove_char";
	gchar *removed;
	GRegex *regex;
	GError *error;

	removed = g_strdup( string );

	if( g_utf8_validate( string, -1, NULL )){

		error = NULL;
		regex = g_regex_new( to_remove, 0, 0, &error );
		if( error ){
			g_warning( "%s [g_regex_new] %s", thisfn, error->message );
			g_error_free( error );

		} else {
			g_free( removed );
			removed = g_regex_replace_literal( regex, string, -1, 0, "", 0, &error );
			if( error ){
				g_warning( "%s [g_regex_replace_literal] %s", thisfn, error->message );
				g_error_free( error );
			}
		}
	}

	return( removed );
}

/**
 * na_core_utils_str_remove_suffix:
 * @string: source string.
 * @suffix: suffix to be removed from @string.
 *
 * Returns: a newly allocated string, which is a copy of the source @string,
 * minus the removed @suffix if present. If @strings doesn't terminate with
 * @suffix, then the returned string is equal to source @string.
 *
 * The returned string should be g_free() by the caller.
 */
gchar *
na_core_utils_str_remove_suffix( const gchar *string, const gchar *suffix )
{
	gchar *removed;
	gchar *ptr;

	removed = g_strdup( string );

	if( g_str_has_suffix( string, suffix )){
		ptr = g_strrstr( removed, suffix );
		ptr[0] = '\0';
	}

	return( removed );
}

/**
 * na_core_utils_slist_duplicate:
 * @source_slist: the #GSList to be duplicated.
 *
 * Returns: a #GSList of strings.
 *
 * The returned list should be #na_core_utils_slist_free() by the caller.
 */
GSList *
na_core_utils_slist_duplicate( GSList *source_slist )
{
	GSList *dest_slist, *it;

	dest_slist = NULL;

	for( it = source_slist ; it != NULL ; it = it->next ){
		dest_slist = g_slist_prepend( dest_slist, g_strdup(( gchar * ) it->data ) );
	}

	dest_slist = g_slist_reverse( dest_slist );

	return( dest_slist );
}

/**
 * na_core_utils_slist_dump:
 * @list: a list of strings.
 *
 * Dumps the content of a list of strings.
 */
void
na_core_utils_slist_dump( GSList *list )
{
	static const gchar *thisfn = "na_core_utils_slist_dump";
	GSList *i;
	int c;

	g_debug( "%s: list at %p has %d element(s)", thisfn, ( void * ) list, g_slist_length( list ));

	for( i=list, c=0 ; i ; i=i->next ){
		g_debug( "%s: [%2d] %s", thisfn, c++, ( gchar * ) i->data );
	}
}

/**
 * na_core_utils_slist_from_split:
 * @@text: a string to be splitted.
 *
 * Returns: a #GSList with the list of strings after having been splitted.
 *
 * The returned #GSList should be #na_core_utils_slist_free() by the caller.
 */
GSList *
na_core_utils_slist_from_split( const gchar *text, const gchar *separator )
{
	GSList *slist;
	gchar **tokens;
	gchar *source, *tmp;

	if( !text ){
		return( NULL );
	}

	source = g_strdup( text );
	tmp = g_strstrip( source );

	if( !g_utf8_strlen( tmp, -1 )){
		return( NULL );
	}

	tokens = g_strsplit( tmp, separator, -1 );
	slist = na_core_utils_slist_from_array(( const gchar ** ) tokens );
	g_strfreev( tokens );

	g_free( source );

	return( slist );
}

/**
 * na_core_utils_slist_from_array:
 * @str_array: an NULL-terminated array of strings.
 *
 * Returns: a #GSList list of strings, which should be #na_core_utils_slist_free()
 * by the caller.
 */
GSList *
na_core_utils_slist_from_array( const gchar **str_array )
{
	GSList *slist;
	gchar **idx;

	slist = NULL;
	idx = ( gchar ** ) str_array;

	while( *idx ){
		slist = g_slist_prepend( slist, g_strstrip( g_strdup( *idx )));
		idx++;
	}

	return( g_slist_reverse( slist ));
}

/**
 * na_core_utils_slist_join_at_end:
 * @slist: the string list to join.
 * @link: the string used to join each element.
 *
 * Returns: a newly allocated string which should be g_free() by the caller.
 */
gchar *
na_core_utils_slist_join_at_end( GSList *slist, const gchar *link )
{
	GSList *is;
	GString *str;

	str = g_string_new( "" );

	for( is = slist ; is ; is = is->next ){
		g_string_append_printf( str, "%s%s", ( const gchar * ) is->data, link );
	}

	return( g_string_free( str, FALSE ));
}

/**
 * na_core_utils_slist_remove_ascii:
 * @list: the GSList to be updated.
 * @text: string to remove.
 *
 * Removes a string from a GSList of strings.
 *
 * Returns the new list after update.
 */
GSList *
na_core_utils_slist_remove_ascii( GSList *list, const gchar *text )
{
	GSList *il;

	for( il = list ; il ; il = il->next ){

		const gchar *istr = ( const gchar * ) il->data;
		if( !g_ascii_strcasecmp( text, istr )){

			list = g_slist_remove( list, ( gconstpointer ) istr );
			return( list );
		}
	}

	return( list );
}

/**
 * na_core_utils_slist_remove_utf8:
 * @list: the GSList to be updated.
 * @str: the string to be removed.
 *
 * Removes from the @list the item which has a string which is equal to
 * @str.
 *
 * Returns: the new @list start position.
 */
GSList *
na_core_utils_slist_remove_utf8( GSList *list, const gchar *str )
{
	GSList *is;

	for( is = list ; is ; is = is->next ){
		const gchar *istr = ( const gchar * ) is->data;
		if( !na_core_utils_str_collate( str, istr )){
			g_free( is->data );
			list = g_slist_delete_link( list, is );
			break;
		}
	}

	return( list );
}

/**
 * na_core_utils_slist_to_array:
 * @slist: a list of strings.
 *
 * Returns: a newly allocated array of strings, which should be
 * g_strfreev() by the caller.
 */
gchar **
na_core_utils_slist_to_array( GSList *slist )
{
	GString *str;
	GSList *is;
	gchar **array;

	str = g_string_new( "" );
	for( is = slist ; is ; is = is->next ){
		g_string_append_printf( str, "%s;", ( const gchar * ) is->data );
	}
	array = g_strsplit( str->str, ";", -1 );
	g_string_free( str, TRUE );

	return( array );
}

/**
 * na_core_utils_slist_to_text:
 * @strlist: a list of strings.
 *
 * Concatenates a string list to a semi-colon-separated text
 * suitable for an entry in the user interface
 *
 * Returns: a newly allocated string, which should be g_free() by the
 * caller.
 */
gchar *
na_core_utils_slist_to_text( GSList *strlist )
{
	GSList *ib;
	gchar *tmp;
	gchar *text = g_strdup( "" );

	for( ib = strlist ; ib ; ib = ib->next ){
		if( strlen( text )){
			tmp = g_strdup_printf( "%s; ", text );
			g_free( text );
			text = tmp;
		}
		tmp = g_strdup_printf( "%s%s", text, ( gchar * ) ib->data );
		g_free( text );
		text = tmp;
	}

	return( text );
}

/**
 * na_core_utils_slist_find:
 * @list: the GSList of strings to be searched.
 * @str: the searched string.
 *
 * Search for a string in a string list.
 *
 * Returns: %TRUE if the string has been found in list.
 */
gboolean
na_core_utils_slist_find( GSList *list, const gchar *str )
{
	GSList *il;

	for( il = list ; il ; il = il->next ){
		const gchar *istr = ( const gchar * ) il->data;
		if( !na_core_utils_str_collate( str, istr )){
			return( TRUE );
		}
	}

	return( FALSE );
}

/**
 * na_core_utils_slist_are_equal:
 * @first: a GSList of strings.
 * @second: another GSList of strings to be compared with @first.
 *
 * Compare two string lists, without regards to the order.
 *
 * Returns: %TRUE if the two lists have same content.
 */
gboolean
na_core_utils_slist_are_equal( GSList *first, GSList *second )
{
	GSList *il;

	for( il = first ; il ; il = il->next ){
		const gchar *str = ( const gchar * ) il->data;
		if( !na_core_utils_slist_find( second, str )){
			return( FALSE );
		}
	}

	for( il = second ; il ; il = il->next ){
		const gchar *str = ( const gchar * ) il->data;
		if( !na_core_utils_slist_find( first, str )){
			return( FALSE );
		}
	}

	return( TRUE );
}

/**
 * na_core_utils_slist_free:
 * @list: a #GSList list of strings.
 *
 * Releases the strings and the list itself.
 */
void
na_core_utils_slist_free( GSList *slist )
{
	g_slist_foreach( slist, ( GFunc ) g_free, NULL );
	g_slist_free( slist );
}

/**
 * na_core_utils_gstring_joinv:
 * @start: a prefix to be written at the beginning of the output string.
 * @separator: a string to be used as separator.
 * @list: the list of strings to be concatenated.
 *
 * Concatenates a gchar **list of strings to a new string.
 *
 * Returns: a newly allocated string which should be g_free() by the caller.
 */
gchar *
na_core_utils_gstring_joinv( const gchar *start, const gchar *separator, gchar **list )
{
	GString *tmp_string = g_string_new( "" );
	int i;

	g_return_val_if_fail( list != NULL, NULL );

	if( start != NULL ){
		tmp_string = g_string_append( tmp_string, start );
	}

	if( list[0] != NULL ){
		tmp_string = g_string_append( tmp_string, list[0] );
	}

	for( i = 1 ; list[i] != NULL ; i++ ){
		if( separator ){
			tmp_string = g_string_append( tmp_string, separator );
		}
		tmp_string = g_string_append( tmp_string, list[i] );
	}

	return( g_string_free( tmp_string, FALSE ));
}

/*
 * split a text buffer in lines
 */
static GSList *
text_to_string_list( const gchar *text, const gchar *separator, const gchar *default_value )
{
	GSList *strlist = NULL;
	gchar **tokens;
	gchar *tmp;
	gchar *source = g_strdup( text );

	tmp = g_strstrip( source );
	if( !strlen( tmp ) && default_value ){
		strlist = g_slist_append( strlist, g_strdup( default_value ));

	} else {
		tokens = g_strsplit( source, separator, -1 );
		strlist = na_core_utils_slist_from_array(( const gchar ** ) tokens );
		g_strfreev( tokens );
	}

	g_free( source );
	return( strlist );
}

/**
 * na_core_utils_dir_is_writable_path:
 * @path: the path of the directory to be tested.
 *
 * Returns: %TRUE if the directory is writable, %FALSE else.
 *
 * Please note that this type of test is subject to race conditions,
 * as the directory may become unwritable after a successful test,
 * but before the caller has been able to actually write into it.
 *
 * There is no "super-test". Just try...
 */
gboolean
na_core_utils_dir_is_writable_path( const gchar *path )
{
	static const gchar *thisfn = "na_core_utils_path_is_writable";
	GFile *file;
	gboolean writable;

	if( !path || !g_utf8_strlen( path, -1 )){
		g_warning( "%s: empty path", thisfn );
		return( FALSE );
	}

	file = g_file_new_for_path( path );
	writable = info_dir_is_writable( file, path );
	g_object_unref( file );

	return( writable );
}

/**
 * na_core_utils_dir_is_writable_uri:
 * @uri: the URI of the directory to be tested.
 *
 * Returns: %TRUE if the directory is writable, %FALSE else.
 *
 * Please note that this type of test is subject to race conditions,
 * as the directory may become unwritable after a successful test,
 * but before the caller has been able to actually write into it.
 *
 * There is no "super-test". Just try...
 */
gboolean
na_core_utils_dir_is_writable_uri( const gchar *uri )
{
	static const gchar *thisfn = "na_core_utils_dir_is_writable_uri";
	GFile *file;
	gboolean writable;

	if( !uri || !g_utf8_strlen( uri, -1 )){
		g_warning( "%s: empty uri", thisfn );
		return( FALSE );
	}

	file = g_file_new_for_uri( uri );
	writable = info_dir_is_writable( file, uri );
	g_object_unref( file );

	return( writable );
}

static gboolean
info_dir_is_writable( GFile *file, const gchar *path_or_uri )
{
	static const gchar *thisfn = "na_core_utils_info_dir_is_writable";
	GError *error = NULL;
	GFileInfo *info;
	GFileType type;
	gboolean writable;

	info = g_file_query_info( file,
			G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
			G_FILE_QUERY_INFO_NONE, NULL, &error );

	if( error ){
		if( error->code != G_IO_ERROR_NOT_FOUND ){
			g_warning( "%s: g_file_query_info error: %s", thisfn, error->message );
		}
		g_error_free( error );
		return( FALSE );
	}

	type = g_file_info_get_file_type( info );
	if( type != G_FILE_TYPE_DIRECTORY ){
		g_debug( "%s: %s is not a directory", thisfn, path_or_uri );
		g_object_unref( info );
		return( FALSE );
	}

	writable = g_file_info_get_attribute_boolean( info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE );
	if( !writable ){
		g_debug( "%s: %s is not writable", thisfn, path_or_uri );
	}

	g_object_unref( info );

	return( writable );
}

/**
 * na_core_utils_file_delete:
 * @path: the path of the file to be deleted.
 *
 * Returns: %TRUE if the file is successfully deleted, %FALSE else.
 */
gboolean
na_core_utils_file_delete( const gchar *path )
{
	static const gchar *thisfn = "na_core_utils_file_delete";
	gboolean deleted = FALSE;

	if( !path || !g_utf8_strlen( path, -1 )){
		return( FALSE );
	}

	if( g_unlink( path ) == 0 ){
		deleted = TRUE;

	} else {
		g_warning( "%s: %s: %s", thisfn, path, g_strerror( errno ));
	}

	return( deleted );
}

/**
 * na_core_utils_file_exists:
 * @fname: a file full URI.
 *
 * Returns: %TRUE if the specified file exists, %FALSE else.
 *
 * Race condition: cf. na_core_utils_dir_is_writable() comment.
 */
gboolean
na_core_utils_file_exists( const gchar *fname )
{
	GFile *file;
	gboolean exists;

	file = g_file_new_for_uri( fname );
	exists = g_file_query_exists( file, NULL );
	g_object_unref( file );

	return( exists );
}

/**
 * na_core_utils_print_version:
 *
 * Print a version message on the console
 *
 * caja-actions-new (Caja-Actions) v 2.29.1
 * Copyright (C) 2005-2007 Frederic Ruaudel
 * Copyright (C) 2009, 2010 Pierre Wieser
 * Caja-Actions is free software, licensed under GPLv2 or later.
 */
void
na_core_utils_print_version( void )
{
	gchar *copyright;

	g_print( "\n" );
	g_print( "%s (%s) v %s\n", g_get_prgname(), PACKAGE_NAME, PACKAGE_VERSION );
	copyright = na_iabout_get_copyright( TRUE );
	g_print( "%s\n", copyright );
	g_free( copyright );

	g_print( "%s is free software, and is provided without any warranty. You may\n", PACKAGE_NAME );
	g_print( "redistribute copies of %s under the terms of the GNU General Public\n", PACKAGE_NAME );
	g_print( "License (see COPYING).\n" );
	g_print( "\n" );

	g_debug( "Current system runs Glib %d.%d.%d, Gtk+ %d.%d.%d",
			glib_major_version, glib_minor_version, glib_micro_version,
			gtk_major_version, gtk_minor_version, gtk_micro_version );
	g_debug( "%s", "" );
}
