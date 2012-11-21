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

#include <glib-object.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include <api/na-core-utils.h>

#include "console-utils.h"

/*
 * A program to update the N-A configuration files
 */

#include <core/na-settings.h>

static gchar       *st_option_context = N_( "Set a key=value pair in a key file." );

static gchar       *st_group     = "";
static gchar       *st_key       = "";
static gchar       *st_type      = "";
static gchar       *st_value     = "";
static gboolean     st_version   = FALSE;

static GOptionEntry st_entries[] = {

	{ "group"                , 'g', 0, G_OPTION_ARG_STRING        , &st_group,
			N_( "The group to be updated" ), N_( "<STRING>" ) },
	{ "key"                  , 'k', 0, G_OPTION_ARG_STRING        , &st_key,
			N_( "The key to be updated" ), N_( "<STRING>" ) },
	{ "type"                 , 't', 0, G_OPTION_ARG_STRING        , &st_type,
			/* i18n: 'str', 'int' and 'bool' are literal values: do not translate */
			N_( "The type of the value to be set, may be 'str', 'int' or 'bool'" ), N_( "<STRING>" ) },
	{ "value"                , 'v', 0, G_OPTION_ARG_STRING        , &st_value,
			N_( "The value to be set" ), N_( "<STRING>" ) },
	{ NULL }
};

static GOptionEntry misc_entries[] = {

	{ "version"              , 'v', 0, G_OPTION_ARG_NONE          , &st_version,
			N_( "Output the version number and exit gracefully" ), NULL },
	{ NULL }
};

typedef enum {
	TYPE_NO_TYPE = 0,
	TYPE_STR,
	TYPE_INT,
	TYPE_BOOL
}
	NAType;

static GOptionContext *init_options( void );
static void            exit_with_usage( void );
static int             do_update_conf( const gchar *group, const gchar *key, NAType type, const gchar *value );

int
main( int argc, char** argv )
{
	int status = EXIT_SUCCESS;
	GOptionContext *context;
	GError *error = NULL;
	gchar *help;
	gint errors;
	NAType type;
	gchar *msgerr;

	g_type_init();
	setlocale( LC_ALL, "" );
	console_init_log_handler();

	context = init_options();

	if( argc == 1 ){
		g_set_prgname( argv[0] );
		help = g_option_context_get_help( context, FALSE, NULL );
		g_print( "\n%s", help );
		g_free( help );
		exit( status );
	}

	if( !g_option_context_parse( context, &argc, &argv, &error )){
		g_printerr( _( "Syntax error: %s\n" ), error->message );
		g_error_free (error);
		exit_with_usage();
	}

	g_option_context_free( context );

	if( st_version ){
		na_core_utils_print_version();
		exit( status );
	}

	errors = 0;
	type = TYPE_NO_TYPE;

	if( !st_group || !g_utf8_strlen( st_group, -1 )){
		g_printerr( _( "Error: a group is mandatory.\n" ));
		errors += 1;
	}

	if( !st_key || !g_utf8_strlen( st_key, -1 )){
		g_printerr( _( "Error: a key is mandatory.\n" ));
		errors += 1;
	}

	if( !st_type || !g_utf8_strlen( st_type, -1 )){
		g_printerr( _( "Error: a type is mandatory for setting/updating a value.\n" ));
		errors += 1;

	} else if( !g_utf8_collate( st_type, "str" )){
		type = TYPE_STR;
	} else if( !g_utf8_collate( st_type, "int" )){
		type = TYPE_INT;
	} else if( !g_utf8_collate( st_type, "bool" )){
		type = TYPE_BOOL;
	} else {
		/* i18n: 'str', 'int' and 'bool' are literal values: do not translate */
		msgerr = g_strdup_printf( _( "Error: unknown type: %s. Use 'str', 'int' or 'bool'.\n" ), st_type );
		g_printerr( "%s", msgerr );
		g_free( msgerr );
		errors += 1;
	}

	if( !st_value || !g_utf8_strlen( st_value, -1 )){
		g_printerr( _( "Error: a value is mandatory.\n" ));
		errors += 1;
	}

	if( errors ){
		exit_with_usage();
	}

	g_return_val_if_fail( type != TYPE_NO_TYPE, EXIT_FAILURE );

	status = do_update_conf( st_group, st_key, type, st_value );

	exit( status );
}

/*
 * init options context
 */
static GOptionContext *
init_options( void )
{
	GOptionContext *context;
	gchar* description;
	GOptionGroup *misc_group;

	context = g_option_context_new( gettext( st_option_context ));
	g_option_context_set_translation_domain( context, GETTEXT_PACKAGE );

#ifdef ENABLE_NLS
	bindtextdomain( GETTEXT_PACKAGE, MATELOCALEDIR );
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
# endif
	textdomain( GETTEXT_PACKAGE );
	g_option_context_add_main_entries( context, st_entries, GETTEXT_PACKAGE );
#else
	g_option_context_add_main_entries( context, st_entries, NULL );
#endif

	description = console_cmdline_get_description();
	g_option_context_set_description( context, description );
	g_free( description );

	misc_group = g_option_group_new(
			"misc", _( "Miscellaneous options" ), _( "Miscellaneous options" ), NULL, NULL );
	g_option_group_add_entries( misc_group, misc_entries );
	g_option_group_set_translation_domain( misc_group, GETTEXT_PACKAGE );
	g_option_context_add_group( context, misc_group );

	return( context );
}

/*
 * print a help message and exit with failure
 */
static void
exit_with_usage( void )
{
	g_printerr( _( "Try %s --help for usage.\n" ), g_get_prgname());
	exit( EXIT_FAILURE );
}

static int
do_update_conf( const gchar *group, const gchar *key, NAType type, const gchar *value )
{
	gboolean ok;
	gboolean bvalue;
	int ivalue;

	ok = FALSE;

	switch( type ){
		case TYPE_BOOL:
			bvalue = na_core_utils_boolean_from_string( value );
			ok = na_settings_set_boolean_ex( group, key, bvalue );
			break;

		case TYPE_INT:
			ivalue = atoi( value );
			ok = na_settings_set_int_ex( group, key, ivalue );
			break;

		case TYPE_STR:
			ok = na_settings_set_string_ex( group, key, value );
			break;

		default:
			break;
	}

	return( ok ? EXIT_SUCCESS : EXIT_FAILURE );
}
