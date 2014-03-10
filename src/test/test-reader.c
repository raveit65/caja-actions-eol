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

#include <glib/gi18n.h>
#include <stdlib.h>

#include <api/na-core-utils.h>

#include <core/na-pivot.h>
#include <core/na-importer.h>

static gchar     *uri     = "";
static gboolean   version = FALSE;

static GOptionEntry entries[] = {

	{ "uri"                  , 'u', 0, G_OPTION_ARG_STRING        , &uri,
			N_( "The URI of the file to be imported" ), N_( "<URI>" ) },
	{ NULL }
};

static GOptionEntry misc_entries[] = {

	{ "version"              , 'v', 0, G_OPTION_ARG_NONE        , &version,
			N_( "Output the version number" ), NULL },
	{ NULL }
};

static GOptionContext  *init_options( void );
static void             check_options( int argc, char **argv, GOptionContext *context );
static void             exit_with_usage( void );

int
main( int argc, char **argv )
{
	NAImporterParms parms;
	GList *import_results;
	NAImporterResult *result;

#if !GLIB_CHECK_VERSION( 2,36, 0 )
	g_type_init();
#endif

	GOptionContext *context = init_options();
	check_options( argc, argv, context );

	NAPivot *pivot = na_pivot_new();
	na_pivot_set_loadable( pivot, !PIVOT_LOAD_DISABLED & !PIVOT_LOAD_INVALID );
	na_pivot_load_items( pivot );

	parms.uris = g_slist_prepend( NULL, uri );
	parms.check_fn = NULL;
	parms.check_fn_data = NULL;
	parms.preferred_mode = IMPORTER_MODE_ASK;
	parms.parent_toplevel = NULL;

	import_results = na_importer_import_from_uris( pivot, &parms );

	result = import_results->data;
	if( result->imported ){
		na_object_dump( result->imported );
		g_object_unref( result->imported );
	}

	na_core_utils_slist_dump( NULL, result->messages );
	na_core_utils_slist_free( result->messages );

	return( 0 );
}

static GOptionContext *
init_options( void )
{
	GOptionContext *context;
	gchar* description;
	GOptionGroup *misc_group;

	context = g_option_context_new( _( "Import a file." ));

#ifdef ENABLE_NLS
	bindtextdomain( GETTEXT_PACKAGE, MATELOCALEDIR );
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
# endif
	textdomain( GETTEXT_PACKAGE );
	g_option_context_add_main_entries( context, entries, GETTEXT_PACKAGE );
#else
	g_option_context_add_main_entries( context, entries, NULL );
#endif

	description = g_strdup_printf( "%s.\n%s", PACKAGE_STRING,
			_( "Bug reports are welcomed at http://bugzilla.gnome.org,"
				" or you may prefer to mail to <maintainer@caja-actions.org>.\n" ));

	g_option_context_set_description( context, description );

	g_free( description );

	misc_group = g_option_group_new(
			"misc", _( "Miscellaneous options" ), _( "Miscellaneous options" ), NULL, NULL );
	g_option_group_add_entries( misc_group, misc_entries );
	g_option_context_add_group( context, misc_group );

	return( context );
}

static void
check_options( int argc, char **argv, GOptionContext *context )
{
	GError *error = NULL;

	if( argc == 1 ){
		g_set_prgname( argv[0] );
		gchar *help = g_option_context_get_help( context, FALSE, NULL );
		g_print( "\n%s", help );
		g_free( help );
		exit( EXIT_SUCCESS );
	}

	if( !g_option_context_parse( context, &argc, &argv, &error )){
		g_printerr( _( "Syntax error: %s\n" ), error->message );
		g_error_free (error);
		exit_with_usage();
	}

	g_option_context_free( context );

	if( version ){
		na_core_utils_print_version();
		exit( EXIT_SUCCESS );
	}

	gint errors = 0;

	if( !uri || !strlen( uri )){
		g_printerr( _( "Error: uri is mandatory.\n" ));
		errors += 1;
	}

	if( errors ){
		exit_with_usage();
	}
}

static void
exit_with_usage( void )
{
	g_printerr( _( "Try %s --help for usage.\n" ), g_get_prgname());
	exit( EXIT_FAILURE );
}
