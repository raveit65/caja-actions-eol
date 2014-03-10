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
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/*
 * This program has been written because I do not have found any reliable
 * way of removing a mandatory key (an action or a menu) from the MateConf
 * XML backend. It is based on http://xmlsoft.org/examples/xpath2.c
 */

#include <api/na-core-utils.h>

#include "console-utils.h"

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

static gchar     *xpath            = "";
static gchar     *xmlfile          = "";
static gboolean   version          = FALSE;

static GOptionEntry entries[] = {

	{ "path"                 , 'p', 0, G_OPTION_ARG_STRING        , &xpath,
			N_( "The XPath to the tree to be deleted" ), N_( "<STRING>" ) },
	{ "xml"                  , 'x', 0, G_OPTION_ARG_STRING        , &xmlfile,
			N_( "The filename of the XML backend" ), N_( "<STRING>" ) },
	{ NULL }
};

static GOptionEntry misc_entries[] = {

	{ "version"              , 'v', 0, G_OPTION_ARG_NONE        , &version,
			N_( "Output the version number" ), NULL },
	{ NULL }
};

static GOptionContext  *init_options( void );
static void             exit_with_usage( void );

static void             delete_path( const gchar *xmlfile, const xmlChar *xpath );

int
main( int argc, char** argv )
{
	int status = EXIT_SUCCESS;
	GOptionContext *context;
	GError *error = NULL;
	gchar *help;
	gint errors;

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
		g_printerr( _( "Syntax error: %s\n" ), error->message );
		g_error_free (error);
		exit_with_usage();
	}

	g_option_context_free( context );

	if( version ){
		na_core_utils_print_version();
		exit( status );
	}

	errors = 0;

	if( !xpath || !strlen( xpath )){
		g_printerr( _( "Error: a XPath is mandatory.\n" ));
		errors += 1;
	}

	if( !xmlfile || !strlen( xmlfile )){
		g_printerr( _( "Error: a XML filename is mandatory.\n" ));
		errors += 1;
	}

	if( errors ){
		exit_with_usage();
	}

	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	/* Do the main job */
	delete_path( xmlfile, BAD_CAST( xpath ));

	/* Shutdown libxml */
    xmlCleanupParser();

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

	context = g_option_context_new( _( "Delete a XPath from a XML document." ));
	g_option_context_set_translation_domain( context, GETTEXT_PACKAGE );

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

static void
delete_path( const gchar *xmlfile, const xmlChar *xpathExpr )
{
	xmlDocPtr doc;
	xmlXPathContextPtr xpathCtx;
	xmlXPathObjectPtr xpathObj;
	xmlNodePtr nodePtr;
	int size, i;

	/* Load XML document */
	doc = xmlParseFile( xmlfile );
	if( doc == NULL ){
		g_printerr( _( "Error: unable to parse file '%s'\n" ), xmlfile );
		return;
	}

	/* Create xpath evaluation context */
	xpathCtx = xmlXPathNewContext( doc );
	if( xpathCtx == NULL ){
		g_printerr( _( "Error: unable to create new XPath context\n" ));
		xmlFreeDoc( doc );
		return;
	}

	/* Evaluate xpath expression */
	xpathObj = xmlXPathEvalExpression( xpathExpr, xpathCtx );
	if( xpathObj == NULL ){
		g_printerr( _( "Error: unable to evaluate XPath expression '%s'\n" ), xpathExpr );
		xmlXPathFreeContext( xpathCtx );
		xmlFreeDoc( doc );
		return;
	}

	/* update selected nodes */
	/*update_xpath_nodes(xpathObj->nodesetval, value);*/
	size = ( xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;
	/*g_print( "size=%d\n", size );*/
	for( i=0 ; i<size ; ++i ){
		nodePtr = xpathObj->nodesetval->nodeTab[i];
		/*xmlElemDump( stdout, doc, nodePtr );*/
		xmlUnlinkNode( nodePtr );
	}

	/* Cleanup of XPath data */
	xmlXPathFreeObject( xpathObj );
	xmlXPathFreeContext( xpathCtx );

	/* dump the resulting document */
	xmlDocDump( stdout, doc );

	/* free the document */
	xmlFreeDoc( doc );
}

#else
int
main( void ){
    fprintf( stderr, "XPath support not compiled in\n" );
    exit( 1 );
}
#endif
