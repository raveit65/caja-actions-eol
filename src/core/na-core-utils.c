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
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>
#include <glib/gstdio.h>

#include <api/na-core-utils.h>

#include "na-about.h"

/* minimal and maximal size for loading the content of a file in memory
 * used by na_core_utils_file_is_size_ok()
 */
#define SIZE_MIN		  1
#define SIZE_MAX	1048576		/* 1 MB */

#ifdef NA_ENABLE_DEPRECATED
static GSList  *text_to_string_list( const gchar *text, const gchar *separator, const gchar *default_value );
#endif
static gboolean info_dir_is_writable( GFile *file, const gchar *path );
static gboolean file_is_loadable( GFile *file );
static void     list_perms( const gchar *path, const gchar *message, const gchar *command );

/**
 * na_core_utils_boolean_from_string
 * @string: a string to be converted.
 *
 * Returns: %TRUE if the string evaluates to "true" (case insensitive),
 * %FALSE else.
 *
 * Since: 2.30
 */
gboolean
na_core_utils_boolean_from_string( const gchar *string )
{
	if( !string ) return( FALSE );

	return( g_ascii_strcasecmp( string, "true" ) == 0 || atoi( string ) != 0 );
}

#ifdef NA_ENABLE_DEPRECATED
/**
 * na_core_utils_str_add_prefix:
 * @prefix: the prefix to be prepended.
 * @str: a multiline string.
 *
 * Appends a prefix to each line of the string.
 *
 * Returns: a new string which should be g_free() by the caller.
 *
 * Since: 2.30
 * Deprecated: 3.2
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
#endif /* NA_ENABLE_DEPRECATED */

/**
 * na_core_utils_str_collate:
 * @str1: an UTF-8 encoded string.
 * @str2: an UTF-8 encoded string.
 *
 * Returns:
 * <itemizedlist>
 *   <listitem>
 *     <para>-1 if str1 < str2,</para>
 *   </listitem>
 *   <listitem>
 *     <para>0 if str1 = str2,</para>
 *   </listitem>
 *   <listitem>
 *     <para>+1 if str1 > str2.</para>
 *   </listitem>
 * </itemizedlist>
 *
 * Since: 2.30
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
 * na_core_utils_str_remove_char:
 * @string: source string.
 * @to_remove: the character to remove.
 *
 * Returns: a newly allocated string, which is a copy of the source @string,
 * minus all the found occurrences of the given @to_remove char.
 *
 * The returned string should be g_free() by the caller.
 *
 * Since: 2.30
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
 *
 * Since: 2.30
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
 * na_core_utils_str_split_first_word:
 * @string: a space-separated string.
 * @first: a pointer to a gchar *.
 * @other: a pointer to a gchar *.
 *
 * Split the @string string into two components:
 * <itemizedlist>
 *   <listitem>
 *     <para>the first word which is allocated in @first,</para>
 *   </listitem>
 *   <listitem>
 *     <para>the rest of the string which is allocated in @other.</para>
 *   </listitem>
 * </itemizedlist>
 *
 * The two allocated strings should be g_free() by the caller.
 *
 * Since: 2.30
 */
void
na_core_utils_str_split_first_word( const gchar *string, gchar **first, gchar **other )
{
	gchar **splitted, **iter;

	if( first ){
		*first = NULL;
	}

	if( other ){
		*other = NULL;
	}

	if( string && g_utf8_strlen( string, -1 )){
		splitted = g_strsplit( string, " ", 2 );
		iter = splitted;
		if( first ){
			*first = g_strdup( *iter );
		}
		iter++;
		if( other ){
			*other = g_strdup( *iter );
		}
		g_strfreev( splitted );
	}
}

/**
 * na_core_utils_str_subst:
 * @pattern: the pattern.
 * @key: the key string to be substituted.
 * @subst: the string which will replace @key.
 *
 * Returns:
 * a copy of @pattern where the first occurrence of @key has been
 * substituted with @subst, as a newly allocated string which should be
 * g_free() by the caller,
 * or a copy of @pattern if @key is not found in @pattern.
 */
gchar *
na_core_utils_str_subst( const gchar *pattern, const gchar *key, const gchar *subst )
{
	GString *result;
	gchar *found;

	result = g_string_new( "" );
	found = g_strstr_len( pattern, -1, key );
	if( found ){
		result = g_string_append_len( result, pattern, ( gssize )( found - pattern ));
		result = g_string_append( result, subst );
		result = g_string_append( result, found + g_utf8_strlen( key, -1 ));

	} else {
		result = g_string_append( result, pattern );
	}

	return( g_string_free( result, FALSE ));
}

void
na_core_utils_slist_add_message( GSList **messages, const gchar *format, ... )
{
	va_list va;
	gchar *tmp;

	va_start( va, format );
	tmp = g_markup_vprintf_escaped( format, va );
	va_end( va );

	*messages = g_slist_append( *messages, tmp );
}

/**
 * na_core_utils_slist_duplicate:
 * @slist: the #GSList to be duplicated.
 *
 * Returns: a #GSList of strings.
 *
 * The returned list should be na_core_utils_slist_free() by the caller.
 *
 * Since: 2.30
 */
GSList *
na_core_utils_slist_duplicate( GSList *slist )
{
	GSList *dest_slist, *it;

	dest_slist = NULL;

	for( it = slist ; it != NULL ; it = it->next ){
		dest_slist = g_slist_prepend( dest_slist, g_strdup(( gchar * ) it->data ) );
	}

	dest_slist = g_slist_reverse( dest_slist );

	return( dest_slist );
}

/**
 * na_core_utils_slist_dump:
 * @prefix: a string to be used as a prefix for each outputed line.
 * @list: a list of strings.
 *
 * Dumps the content of a list of strings.
 *
 * Since: 2.30
 */
void
na_core_utils_slist_dump( const gchar *prefix, GSList *list )
{
	static const gchar *thisfn = "na_core_utils_slist_dump";
	GSList *i;
	int c;
	const gchar *thispfx;

	thispfx = ( prefix && strlen( prefix )) ? prefix : thisfn;

	g_debug( "%s: list at %p has %d element(s)", thispfx, ( void * ) list, g_slist_length( list ));

	for( i=list, c=0 ; i ; i=i->next ){
		g_debug( "%s: [%2d] %s (%lu)",
				thispfx, c++, ( gchar * ) i->data, g_utf8_strlen( ( gchar * ) i->data, -1 ));
	}
}

/**
 * na_core_utils_slist_from_split:
 * @text: a string to be splitted.
 * @separator: the string to be used as the separator.
 *
 * Returns: a #GSList with the list of strings after having been splitted.
 *
 * The returned #GSList should be na_core_utils_slist_free() by the caller.
 *
 * Since: 2.30
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
 *
 * Since: 2.30
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
 *
 * Since: 2.30
 */
gchar *
na_core_utils_slist_join_at_end( GSList *slist, const gchar *link )
{
	GSList *is;
	GString *str;

	str = g_string_new( "" );

	for( is = slist ; is ; is = is->next ){
		if( str->len ){
			g_string_append_printf( str, "%s", link );
		}
		g_string_append_printf( str, "%s", ( const gchar * ) is->data );
	}

	return( g_string_free( str, FALSE ));
}

/**
 * na_core_utils_slist_remove_ascii:
 * @slist: the #GSList to be updated.
 * @text: string to remove.
 *
 * Removes a string from a GSList of strings.
 *
 * Returns: the same, updated, @slist.
 *
 * Since: 2.30
 */
GSList *
na_core_utils_slist_remove_ascii( GSList *slist, const gchar *text )
{
	GSList *il;

	for( il = slist ; il ; il = il->next ){

		const gchar *istr = ( const gchar * ) il->data;
		if( !g_ascii_strcasecmp( text, istr )){

			slist = g_slist_remove( slist, ( gconstpointer ) istr );
			return( slist );
		}
	}

	return( slist );
}

/**
 * na_core_utils_slist_remove_utf8:
 * @slist: the #GSList to be updated.
 * @text: the string to be removed.
 *
 * Removes from the @slist the item which has a string which is equal to
 * @text.
 *
 * Returns: the new @slist start position.
 *
 * Since: 2.30
 */
GSList *
na_core_utils_slist_remove_utf8( GSList *slist, const gchar *text )
{
	GSList *is;

	for( is = slist ; is ; is = is->next ){
		const gchar *istr = ( const gchar * ) is->data;
		if( !na_core_utils_str_collate( text, istr )){
			g_free( is->data );
			slist = g_slist_delete_link( slist, is );
			break;
		}
	}

	return( slist );
}

/**
 * na_core_utils_slist_to_array:
 * @slist: a list of strings.
 *
 * Returns: a newly allocated array of strings, which should be
 * g_strfreev() by the caller.
 *
 * Since: 2.30
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
 * @slist: a list of strings.
 *
 * Concatenates a string list to a semi-colon-separated text
 * suitable for an entry in the user interface
 *
 * Returns: a newly allocated string, which should be g_free() by the
 * caller.
 *
 * Since: 2.30
 */
gchar *
na_core_utils_slist_to_text( GSList *slist )
{
	GSList *ib;
	gchar *tmp;
	gchar *text = g_strdup( "" );

	for( ib = slist ; ib ; ib = ib->next ){
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
 * na_core_utils_slist_setup_element:
 * @list: the GSList of strings to be setup.
 * @element: the string to add to or remove of the list.
 * @set: whether the @element should be set or removed.
 *
 * Setup the @list so that the @element is once in the @list if @set is %TRUE,
 * or not if @set is %FALSE.
 *
 * Returns: the updated @list.
 *
 * Since: 2.30
 */
GSList *
na_core_utils_slist_setup_element( GSList *list, const gchar *element, gboolean set )
{
	guint count;

	count = na_core_utils_slist_count( list, element );

	if( set && count == 0 ){
		list = g_slist_prepend( list, g_strdup( element ));
	}
	if( !set && count > 0 ){
		list = na_core_utils_slist_remove_ascii( list, element );
	}

	return( list );
}

/**
 * na_core_utils_slist_count:
 * @list: the GSList of strings to be searched.
 * @str: the searched string.
 *
 * Search for a string in a string list.
 *
 * Returns: the count of @ßtr in @list list.
 *
 * Since: 2.30
 */
guint
na_core_utils_slist_count( GSList *list, const gchar *str )
{
	guint count;
	GSList *il;

	count = 0;

	for( il = list ; il ; il = il->next ){
		const gchar *istr = ( const gchar * ) il->data;
		if( !na_core_utils_str_collate( str, istr )){
			count += 1;
		}
	}

	return( count );
}

/**
 * na_core_utils_slist_find_negated:
 * @list: the GSList of strings to be searched.
 * @str: the searched string.
 *
 * Search for a string in a string list which may contain nagated items.
 *
 * Returns: %TRUE if the string has been found in list.
 *
 * Since: 2.30
 */
gboolean
na_core_utils_slist_find_negated( GSList *list, const gchar *str )
{
	GSList *il;

	for( il = list ; il ; il = il->next ){
		const gchar *istr = g_strstrip( g_strdup( ( const gchar * ) il->data ));

		if( istr[0] == '!' ){
			gchar *istrdup = g_strdup( istr+1 );
			int match = na_core_utils_str_collate( str, istrdup );
			g_free( istrdup );
			if( match == 0 ){
				return( TRUE );
			}

		} else if( na_core_utils_str_collate( str, istr ) == 0 ){
				return( TRUE );
		}
	}

	return( FALSE );
}

/**
 * na_core_utils_slist_are_equal:
 * @a: a GSList of strings.
 * @b: another GSList of strings to be compared with @first.
 *
 * Compare two string lists, without regards to the order.
 *
 * Returns: %TRUE if the two lists have same content.
 *
 * Since: 2.30
 */
gboolean
na_core_utils_slist_are_equal( GSList *a, GSList *b )
{
	GSList *il;

	for( il = a ; il ; il = il->next ){
		const gchar *str = ( const gchar * ) il->data;
		if( na_core_utils_slist_count( b, str ) == 0 ){
			return( FALSE );
		}
	}

	for( il = b ; il ; il = il->next ){
		const gchar *str = ( const gchar * ) il->data;
		if( na_core_utils_slist_count( a, str ) == 0 ){
			return( FALSE );
		}
	}

	return( TRUE );
}

/**
 * na_core_utils_slist_free:
 * @slist: a #GSList list of strings.
 *
 * Releases the strings and the list itself.
 *
 * Since: 2.30
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
 *
 * Since: 2.30
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

/***
 * na_core_utils_selcount_get_ope:
 * @selcount: the selection count condition string.
 * @ope: a pointer to a newly allocated string which will contains the
 *  operation code.
 * @uint: a pointer to a newly allocated string which will contains the
 *  relevant integer.
 *
 * Parses a selection count string, and extract the operation code and the
 * relevant integer.
 *
 * The two returned strings must be g_free() by the caller.
 *
 * Since: 2.30
 */
void
na_core_utils_selcount_get_ope_int( const gchar *selcount, gchar **ope, gchar **uint )
{
	gchar *dup, *dup2;
	guint uint_int;

	g_return_if_fail( ope && uint );

	*ope = NULL;
	*uint = NULL;

	dup = g_strstrip( g_strdup( selcount ));
	*ope = g_strdup( " " );
	*ope[0] = dup[0];

	dup2 = g_strstrip( g_strdup( dup+1 ));
	uint_int = abs( atoi( dup2 ));
	*uint = g_strdup_printf( "%d", uint_int );

	g_free( dup2 );
	g_free( dup );
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
 *
 * Since: 2.30
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
 *
 * Since: 2.30
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
 * na_core_utils_dir_list_perms:
 * @path: the path of the directory to be tested.
 * @message: a message to be printed if not %NULL.
 *
 * Displays the permissions of the directory on debug output.
 *
 * Since: 3.1
 */
void
na_core_utils_dir_list_perms( const gchar *path, const gchar *message )
{
	list_perms( path, message, "ls -ld" );
}

/**
 * na_core_utils_dir_split_ext:
 * @string: the input path or URI to be splitted.
 * @first: a pointer to a buffer which will contain the first part of the split.
 * @ext: a pointer to a buffer which will contain the extension part of the path.
 *
 * Split the given @string, returning the first part and the extension in newly
 * allocated buffers which should be g_free() by the caller.
 *
 * The extension is set to an empty string if no extension is detected.
 *
 * Since: 2.30
 */
void
na_core_utils_dir_split_ext( const gchar *string, gchar **first, gchar **ext )
{
	gchar *dupped;
	gchar **array, **iter;

	dupped = g_strreverse( g_strdup( string ));
	array = g_strsplit( dupped, ".", 2 );

	if( g_strv_length( array ) == 1 ){
		if( ext ){
			*ext = g_strdup( "" );
		}
		if( first ){
			*first = g_strreverse( g_strdup(( const gchar * ) *array ));
		}
	} else {
		if( ext ){
			*ext = g_strreverse( g_strdup(( const gchar * ) *array ));
		}
		iter = array;
		++iter;
		if( first ){
			*first = g_strreverse( g_strdup(( const gchar * ) *iter ));
		}
	}

	g_strfreev( array );
	g_free( dupped );
}

/**
 * na_core_utils_file_delete:
 * @path: the path of the file to be deleted.
 *
 * Returns: %TRUE if the file is successfully deleted, %FALSE else.
 *
 * Since: 2.30
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
 * @uri: a file URI.
 *
 * Returns: %TRUE if the specified file exists, %FALSE else.
 *
 * Race condition: cf. na_core_utils_dir_is_writable_path() and
 * na_core_utils_dir_is_writable_uri() comments.
 *
 * Since: 2.30
 */
gboolean
na_core_utils_file_exists( const gchar *uri )
{
	GFile *file;
	gboolean exists;

	file = g_file_new_for_uri( uri );
	exists = g_file_query_exists( file, NULL );
	g_object_unref( file );

	return( exists );
}

/**
 * na_core_utils_file_is_loadable:
 * @uri: the URI to be checked.
 *
 * Checks that the file is suitable to be loaded in memory, because
 * it is not empty, and its size is reasonable (less than 1MB).
 * Also checks that a file is a regular file (or a symlink to a
 * regular file).
 *
 * Returns: whether the file is suitable to be loaded in memory.
 *
 * Since: 3.1
 */
gboolean
na_core_utils_file_is_loadable( const gchar *uri )
{
	static const gchar *thisfn = "na_core_utils_file_is_loadable";
	GFile *file;
	gboolean isok;

	g_debug( "%s: uri=%s", thisfn, uri );

	isok = FALSE;
	file = g_file_new_for_uri( uri );

	isok = file_is_loadable( file );

	g_object_unref( file );

	return( isok );
}

static gboolean
file_is_loadable( GFile *file )
{
	static const gchar *thisfn = "na_core_utils_file_is_loadable";
	GError *error;
	GFileInfo *info;
	guint64 size;
	GFileType type;
	gboolean isok;
	GFile *target_file;

	error = NULL;
	isok = FALSE;
	info = g_file_query_info( file,
			G_FILE_ATTRIBUTE_STANDARD_SIZE "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
			G_FILE_QUERY_INFO_NONE, NULL, &error );

	if( !info ){
		if( error ){
			g_debug( "%s: %s", thisfn, error->message );
			g_error_free( error );
		}

	} else {
		size = g_file_info_get_attribute_uint64( info, G_FILE_ATTRIBUTE_STANDARD_SIZE );
		g_debug( "%s: size=%lu", thisfn, ( unsigned long ) size );
		isok = ( size >= SIZE_MIN && size <= SIZE_MAX );
	}

	if( isok ){
		type = g_file_info_get_file_type( info );
		g_debug( "%s: type=%u", thisfn, ( unsigned ) type );

		if( type != G_FILE_TYPE_REGULAR ){
			isok = FALSE;

			if( type == G_FILE_TYPE_SYMBOLIC_LINK ){
				const char *target = g_file_info_get_symlink_target( info );
				if( target && strlen( target )){
					target_file = g_file_resolve_relative_path( file, target );
					if( target_file ){
						isok = file_is_loadable( target_file );
						g_object_unref( target_file );
					}
				}
			}
		}
	}

	g_object_unref( info );

	return( isok );
}

/**
 * na_core_utils_file_list_perms:
 * @path: the path of the file to be tested.
 * @message: a message to be printed if not %NULL.
 *
 * Displays the permissions of the file on debug output.
 *
 * Since: 3.2
 */
void
na_core_utils_file_list_perms( const gchar *path, const gchar *message )
{
	list_perms( path, message, "ls -l" );
}

static void
list_perms( const gchar *path, const gchar *message, const gchar *command )
{
	static const gchar *thisfn = "na_core_utils_list_perms";
	gchar *cmd;
	gchar *out, *err;
	GError *error;

	error = NULL;
	cmd = g_strdup_printf( "%s %s", command, path );

	if( !g_spawn_command_line_sync( cmd, &out, &err, NULL, &error )){
		g_warning( "%s: %s", thisfn, error->message );
		g_error_free( error );

	} else {
		g_debug( "%s: out=%s", message, out );
		g_debug( "%s: err=%s", message, err );
		g_free( out );
		g_free( err );
	}

	g_free( cmd );
}

/**
 * na_core_utils_file_load_from_uri:
 * @uri: the URI the file must be loaded from.
 * @length: a pointer to the length of the read content.
 *
 * Loads the file into a newly allocated buffer, and set up the length of the
 * read content if not %NULL.
 *
 * Returns: the newly allocated buffer which contains the file content, or %NULL.
 * This buffer should be g_free() by the caller.
 *
 * Since: 2.30
 */
gchar *
na_core_utils_file_load_from_uri( const gchar *uri, gsize *length )
{
	static const gchar *thisfn = "na_core_utils_file_load_from_uri";
	gchar *data;
	GFile *file;
	GError *error;

	g_debug( "%s: uri=%s, length=%p", thisfn, uri, ( void * ) length );

	error = NULL;
	data = NULL;
	if( length ){
		*length = 0;
	}

	file = g_file_new_for_uri( uri );

	if( !g_file_load_contents( file, NULL, &data, length, NULL, &error )){
		g_free( data );
		data = NULL;
		if( length ){
			*length = 0;
		}
		if( error ){
			g_debug( "%s: %s", thisfn, error->message );
			g_error_free( error );
		}
	}

	g_object_unref( file );

	return( data );
}

/**
 * na_core_utils_print_version:
 *
 * Print a version message on the console
 *
 * <programlisting>
 *   caja-actions-new (Caja-Actions) v 2.29.1
 *   Copyright (C) 2005-2007 Frederic Ruaudel
 *   Copyright (C) 2009, 2010, 2011, 2012 Pierre Wieser
 *   Caja-Actions is free software, licensed under GPLv2 or later.
 * </programlisting>
 *
 * Since: 2.30
 */
void
na_core_utils_print_version( void )
{
	gchar *copyright;

	g_print( "\n" );
	g_print( "%s (%s) v %s\n", g_get_prgname(), PACKAGE_NAME, PACKAGE_VERSION );
	copyright = na_about_get_copyright( TRUE );
	g_print( "%s\n", copyright );
	g_free( copyright );

	g_print( "%s is free software, and is provided without any warranty. You may\n", PACKAGE_NAME );
	g_print( "redistribute copies of %s under the terms of the GNU General Public\n", PACKAGE_NAME );
	g_print( "License (see COPYING).\n" );
	g_print( "\n" );

	g_debug( "Program has been compiled against Glib %d.%d.%d, Gtk+ %d.%d.%d",
			GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION,
			GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION );

	g_debug( "Current system runs Glib %d.%d.%d, Gtk+ %d.%d.%d\n",
			glib_major_version, glib_minor_version, glib_micro_version,
			gtk_major_version, gtk_minor_version, gtk_micro_version );
}
