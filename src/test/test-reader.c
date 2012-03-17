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
	NAIImporterUriParms parms;

	g_type_init();

	GOptionContext *context = init_options();
	check_options( argc, argv, context );

	NAPivot *pivot = na_pivot_new();

	parms.version = 1;
	parms.uri = uri;
	parms.mode = IMPORTER_MODE_ASK;
	parms.imported = NULL;
	parms.check_fn = NULL;
	parms.check_fn_data = NULL;
	parms.messages = NULL;

	guint code = na_importer_import_from_uri( pivot, &parms );

	g_print( "%s: return code from import is %u.\n", g_get_prgname(), code );

	if( parms.imported ){
		na_object_dump( parms.imported );
		g_object_unref( parms.imported );
	}

	na_core_utils_slist_dump( parms.messages );
	na_core_utils_slist_free( parms.messages );

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
