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

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <libxml/tree.h>
#include <locale.h>
#include <stdlib.h>

#include <api/na-core-utils.h>
#include <api/na-data-types.h>
#include <api/na-ifactory-object-data.h>

#include "console-utils.h"

static gboolean   output_action = FALSE;
static gboolean   output_menu   = FALSE;
static gboolean   output_stdout = FALSE;
static gboolean   version       = FALSE;

static GOptionEntry entries[] = {
	{ "stdout",  's', 0, G_OPTION_ARG_NONE, &output_stdout, N_("Output the schema on stdout"), NULL },
	{ NULL }
};

static GOptionEntry type_entries[] = {
	{ "action",  'a', 0, G_OPTION_ARG_NONE, &output_action, N_("Output the action schemas [default]"), NULL },
	{ "menu",    'm', 0, G_OPTION_ARG_NONE, &output_menu,   N_("Output the menu schemas"), NULL },
	{ NULL }
};

static GOptionEntry misc_entries[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Output the version number"), NULL },
	{ NULL }
};

static GOptionContext *init_options( void );
static void            exit_with_usage( void );

int
main( int argc, char** argv )
{
	int status = EXIT_SUCCESS;
	GOptionContext *context;
	gchar *help;
	GError *error = NULL;
	GSList *msgs = NULL;
	GSList *im;

#if !GLIB_CHECK_VERSION( 2,36, 0 )
	g_type_init();
#endif

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
		g_printerr( _("Syntax error: %s\n" ), error->message );
		g_error_free (error);
		exit_with_usage();
	}

	if( version ){
		na_core_utils_print_version();
		exit( status );
	}

	if( !output_action && !output_menu ){
		output_action = TRUE;
	}

	if( msgs ){
		for( im = msgs ; im ; im = im->next ){
			g_printerr( "%s\n", ( gchar * ) im->data );
		}
		na_core_utils_slist_free( msgs );
		status = EXIT_FAILURE;
	}

	g_option_context_free( context );

	exit( status );
}

/*
 * init options context
 */
static GOptionContext *
init_options( void )
{
	GOptionContext *context;
	gchar *description;
	GOptionGroup *type_group;
	GOptionGroup *misc_group;

	context = g_option_context_new( _( "Output the Caja-Actions MateConf schemas on stdout." ));
	g_option_context_set_translation_domain( context, GETTEXT_PACKAGE );

	g_option_context_set_summary( context, _(
			"As of version 3.1.0, MateConf as an I/O provider is deprecated.\n"
			"This program is no more maintained." ));

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

	description = console_cmdline_get_description();
	g_option_context_set_description( context, description );
	g_free( description );

	type_group = g_option_group_new(
			"type", _( "Type options" ), _( "Type options" ), NULL, NULL );
	g_option_group_add_entries( type_group, type_entries );
	g_option_group_set_translation_domain( type_group, GETTEXT_PACKAGE );
	g_option_context_add_group( context, type_group );

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
	g_printerr( _("Try %s --help for usage.\n"), g_get_prgname());
	exit( EXIT_FAILURE );
}
