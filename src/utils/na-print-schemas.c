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

#include <io-mateconf/nagp-keys.h>

#include <io-xml/caxml-keys.h>

#include "console-utils.h"

extern NADataGroup menu_data_groups[];				/* defined in na-object-menu-factory.c */
extern NADataGroup action_data_groups[];			/* defined in na-object-action-factory.c */
extern NADataGroup profile_data_groups[];			/* defined in na-object-profile-factory.c */

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
static NADataGroup    *build_full_action_group( void );
static int             output_to_stdout( NADataGroup *groups, GSList **msgs );
static void            attach_schema_node( xmlDocPtr doc, xmlNodePtr list_node, const NADataDef *data_def );
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
	NADataGroup *full_action_groups = NULL;

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
	if( output_action ){
		full_action_groups = build_full_action_group();
	}

	if( output_stdout ){
		if( output_action ){
			status = output_to_stdout( full_action_groups, &msgs );
		}
		if( output_menu ){
			status = output_to_stdout( menu_data_groups, &msgs );
		}
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
 * build a NADataGroup array with action and profile definitions
 * so that the action schemas also include profile ones
 */
static NADataGroup *
build_full_action_group( void )
{
	guint i, action_count, profile_count;
	NADataGroup *group;

	for( action_count = 0 ; action_data_groups[action_count].group ; ++action_count )
		;
	for( profile_count = 0 ; profile_data_groups[profile_count].group ; ++profile_count )
		;
	group = g_new0( NADataGroup, 1+action_count+profile_count );

	for( i = action_count = 0 ; action_data_groups[action_count].group ; ++action_count, ++i ){
		memcpy( group+i, action_data_groups+action_count, sizeof( NADataGroup ));
	}
	for( profile_count = 0 ; profile_data_groups[profile_count].group ; ++profile_count, ++i ){
		memcpy( group+i, profile_data_groups+profile_count, sizeof( NADataGroup ));
	}

	return( group );
}

static int
output_to_stdout( NADataGroup *groups, GSList **msgs )
{
	xmlDocPtr doc;
	xmlNodePtr root_node;
	xmlNodePtr list_node;
	xmlChar *text;
	int textlen;
	guint ig, id;
	GSList *displayed = NULL;

	doc = xmlNewDoc( BAD_CAST( "1.0" ));

	root_node = xmlNewNode( NULL, BAD_CAST( CAXML_KEY_SCHEMA_ROOT ));
	xmlDocSetRootElement( doc, root_node );
	list_node = xmlNewChild( root_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_LIST ), NULL );

	for( ig = 0 ; groups[ig].group ; ++ig ){
		for( id = 0 ; groups[ig].def[id].name ; ++id ){
			if( groups[ig].def[id].writable ){
				if( !na_core_utils_slist_count( displayed, groups[ig].def[id].name )){
					displayed = g_slist_prepend( displayed, groups[ig].def[id].name );
					attach_schema_node( doc, list_node, ( const NADataDef * ) groups[ig].def+id );
				}
			}
		}
	}

	xmlDocDumpFormatMemoryEnc( doc, &text, &textlen, "UTF-8", 1 );
	g_printf( "%s\n", ( const char * ) text );

	xmlFree( text );
	xmlFreeDoc (doc);
	xmlCleanupParser();

	return( EXIT_SUCCESS );
}

static void
attach_schema_node( xmlDocPtr doc, xmlNodePtr list_node, const NADataDef *def )
{
	xmlNodePtr schema_node;
	xmlChar *content;
	xmlNodePtr parent_value_node;
	xmlNodePtr locale_node;

	schema_node = xmlNewChild( list_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE ), NULL );

	content = BAD_CAST( g_build_path( "/", NAGP_SCHEMAS_PATH, def->mateconf_entry, NULL ));
	xmlNewChild( schema_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_KEY ), content );
	xmlFree( content );

	xmlNewChild( schema_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_OWNER ), BAD_CAST( PACKAGE_TARNAME ));

	xmlNewChild( schema_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_TYPE ), BAD_CAST( na_data_types_get_mateconf_dump_key( def->type )));
	if( def->type == NA_DATA_TYPE_STRING_LIST ){
		xmlNewChild( schema_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_LISTTYPE ), BAD_CAST( "string" ));
	}

	locale_node = xmlNewChild( schema_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_LOCALE ), NULL );
	xmlNewProp( locale_node, BAD_CAST( "name" ), BAD_CAST( "C" ));

	xmlNewChild( locale_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_LOCALE_SHORT ), BAD_CAST( gettext( def->short_label )));

	xmlNewChild( locale_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_LOCALE_LONG ), BAD_CAST( gettext( def->long_label )));

	parent_value_node = def->localizable ? locale_node : schema_node;

	content = xmlEncodeSpecialChars( doc, BAD_CAST( def->default_value ));
	xmlNewChild( parent_value_node, NULL, BAD_CAST( CAXML_KEY_SCHEMA_NODE_DEFAULT ), content );
	xmlFree( content );
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
