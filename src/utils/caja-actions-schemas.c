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

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <libxml/tree.h>
#include <stdlib.h>

#include <api/na-core-utils.h>
#include <api/na-data-types.h>
#include <api/na-ifactory-object-data.h>

#include <io-mateconf/nagp-keys.h>

#include <io-xml/naxml-keys.h>

#include "console-utils.h"

typedef struct {
	NADataGroup *group;
	gchar       *group_name;
	gchar       *data_name;
}
	SchemaFromDataDef;

extern NADataGroup action_data_groups[];			/* defined in na-object-action-factory.c */
extern NADataGroup profile_data_groups[];			/* defined in na-object-profile-factory.c */

static const SchemaFromDataDef st_schema_from_data_def[] = {
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_LABEL },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_TOOLTIP },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_ICON },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_ENABLED },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_SELECTION },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_TOOLBAR },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TOOLBAR_LABEL },
		{ profile_data_groups, NA_FACTORY_OBJECT_PROFILE_GROUP,    NAFO_DATA_PATH },
		{ profile_data_groups, NA_FACTORY_OBJECT_PROFILE_GROUP,    NAFO_DATA_PARAMETERS },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_BASENAMES },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MATCHCASE },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MIMETYPES },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_ISFILE },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_ISDIR },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MULTIPLE },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_SCHEMES },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_FOLDERS },
		{ NULL }
};

static gboolean   output_stdout = FALSE;
static gboolean   version       = FALSE;

static GOptionEntry entries[] = {
	{ "stdout" , 's', 0, G_OPTION_ARG_NONE, &output_stdout, N_("Output the schema on stdout"), NULL },
	{ NULL }
};

static GOptionEntry misc_entries[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &version, N_("Output the version number"), NULL },
	{ NULL }
};

static GOptionContext *init_options( void );
static int             output_to_stdout( GSList **msgs );
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

	g_type_init();
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

	if( output_stdout ){
		status = output_to_stdout( &msgs );
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
	GOptionGroup *misc_group;

	context = g_option_context_new( _( "Output the Caja Actions MateConf schema on stdout." ));

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

/*
 * writes the schema via MateConfClient
 */
/*static gboolean
write_to_mateconf( gchar **msg )
{
	MateConfClient *mateconf = mateconf_client_get_default();

	gchar *prefix_config = g_strdup_printf( "%s%s", CAJA_ACTIONS_MATECONF_SCHEMASDIR, NA_MATECONF_CONFIG_PATH );
	gchar *prefix_prefs = g_strdup_printf( "%s%s/%s", CAJA_ACTIONS_MATECONF_SCHEMASDIR, CAJA_ACTIONS_MATECONF_BASEDIR, NA_MATECONF_PREFERENCES );

	gboolean ret =
		write_schema( mateconf, prefix_config, MATECONF_VALUE_STRING, ACTION_VERSION_ENTRY, ACTION_VERSION_DESC_SHORT, ACTION_VERSION_DESC_LONG, CAJA_ACTIONS_CONFIG_VERSION, msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_STRING, ACTION_LABEL_ENTRY, ACTION_LABEL_DESC_SHORT, ACTION_LABEL_DESC_LONG, "", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_STRING, ACTION_TOOLTIP_ENTRY, ACTION_TOOLTIP_DESC_SHORT, ACTION_TOOLTIP_DESC_LONG, "", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_STRING, ACTION_ICON_ENTRY, ACTION_ICON_DESC_SHORT, ACTION_ICON_DESC_LONG, "", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_STRING, ACTION_PROFILE_LABEL_ENTRY, ACTION_PROFILE_NAME_DESC_SHORT, ACTION_PROFILE_NAME_DESC_LONG, NA_ACTION_PROFILE_DEFAULT_LABEL, msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_STRING, ACTION_PATH_ENTRY, ACTION_PATH_DESC_SHORT, ACTION_PATH_DESC_LONG, "", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_STRING, ACTION_PARAMETERS_ENTRY, ACTION_PARAMETERS_DESC_SHORT, ACTION_PARAMETERS_DESC_LONG, "", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_LIST, ACTION_BASENAMES_ENTRY, ACTION_BASENAMES_DESC_SHORT, ACTION_BASENAMES_DESC_LONG, "*", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_BOOL, ACTION_MATCHCASE_ENTRY, ACTION_MATCHCASE_DESC_SHORT, ACTION_MATCHCASE_DESC_LONG, "true", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_LIST, ACTION_MIMETYPES_ENTRY, ACTION_MIMETYPES_DESC_SHORT, ACTION_MIMETYPES_DESC_LONG, "*
		/
		 *", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_BOOL, ACTION_ISFILE_ENTRY, ACTION_ISFILE_DESC_SHORT, ACTION_ISFILE_DESC_LONG, "true", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_BOOL, ACTION_ISDIR_ENTRY, ACTION_ISDIR_DESC_SHORT, ACTION_ISDIR_DESC_LONG, "false", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_BOOL, ACTION_MULTIPLE_ENTRY, ACTION_MULTIPLE_DESC_SHORT, ACTION_MULTIPLE_DESC_LONG, "false", msg ) &&
		write_schema( mateconf, prefix_config, MATECONF_VALUE_LIST, ACTION_SCHEMES_ENTRY, ACTION_SCHEMES_DESC_SHORT, ACTION_SCHEMES_DESC_LONG, "file", msg );

	g_free( prefix_prefs );
	g_free( prefix_config );

	mateconf_client_suggest_sync( mateconf, NULL );
	g_object_unref( mateconf );
	return( ret );
}

static gboolean
write_schema( MateConfClient *mateconf, const gchar *prefix, MateConfValueType type, const gchar *entry, const gchar *dshort, const gchar *dlong, const gchar *default_value, gchar **message )
{
	gchar *path = g_strdup_printf( "%s/%s", prefix, entry );
	g_debug( "write_schema: path=%s", path );
	gboolean ret = TRUE;
	GError *error = NULL;

	MateConfSchema *schema = mateconf_schema_new();
	mateconf_schema_set_owner( schema, PACKAGE );
	mateconf_schema_set_type( schema, type );
*/
	/* FIXME: if we write the schema with a 'C' locale, how will it be
	 * localized ?? but get_language_names return a list. Do we have to
	 * write a locale for each element of the list ? for the first one ?
	 */
	/*mateconf_schema_set_locale( schema, "C" );

	mateconf_schema_set_short_desc( schema, dshort );
	mateconf_schema_set_long_desc( schema, dlong );


	MateConfValue *value = NULL;
	if( type == MATECONF_VALUE_LIST ){
		mateconf_schema_set_list_type( schema, MATECONF_VALUE_STRING );

		MateConfValue *first = mateconf_value_new_from_string( MATECONF_VALUE_STRING, default_value, &error );
		GSList *list = NULL;
		list = g_slist_append( list, first );
		value = mateconf_value_new( MATECONF_VALUE_LIST );
		mateconf_value_set_list_type( value, MATECONF_VALUE_STRING );
		mateconf_value_set_list( value, list );
		g_slist_free( list );

	} else {
		value = mateconf_value_new_from_string( type, default_value, &error );
		if( error ){
			*message = g_strdup( error->message );
			g_error_free( error );
			ret = FALSE;
		}
	}

	if( ret ){
		mateconf_schema_set_default_value( schema, value );

		if( !mateconf_client_set_schema( mateconf, path, schema, &error )){
			*message = g_strdup( error->message );
			g_error_free( error );
			ret = FALSE;
		}
	}

	mateconf_schema_free( schema );
	g_free( path );
	return( ret );
}*/

static int
output_to_stdout( GSList **msgs )
{
	static const gchar *thisfn = "caja_actions_schemas_output_to_stdout";
	xmlDocPtr doc;
	xmlNodePtr root_node;
	xmlNodePtr list_node;
	xmlChar *text;
	int textlen;
	SchemaFromDataDef *isch;
	const NADataDef *data_def;

	doc = xmlNewDoc( BAD_CAST( "1.0" ));

	root_node = xmlNewNode( NULL, BAD_CAST( NAXML_KEY_SCHEMA_ROOT ));
	xmlDocSetRootElement( doc, root_node );

	list_node = xmlNewChild( root_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_LIST ), NULL );

	isch = ( SchemaFromDataDef * ) st_schema_from_data_def;
	while( isch->group ){
		data_def = na_data_def_get_data_def( isch->group, isch->group_name, isch->data_name );

		if( data_def ){
			attach_schema_node( doc, list_node, data_def );

		} else {
			g_warning( "%s: group=%s, name=%s: unable to find NADataDef structure", thisfn, isch->group_name, isch->data_name );
		}

		isch++;
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

	schema_node = xmlNewChild( list_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE ), NULL );

	content = BAD_CAST( g_build_path( "/", NAGP_SCHEMAS_PATH, def->mateconf_entry, NULL ));
	xmlNewChild( schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_KEY ), content );
	xmlFree( content );

	xmlNewChild( schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_OWNER ), BAD_CAST( PACKAGE_TARNAME ));

	xmlNewChild( schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_TYPE ), BAD_CAST( na_data_types_get_mateconf_dump_key( def->type )));
	if( def->type == NAFD_TYPE_STRING_LIST ){
		xmlNewChild( schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LISTTYPE ), BAD_CAST( "string" ));
	}

	locale_node = xmlNewChild( schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LOCALE ), NULL );
	xmlNewProp( locale_node, BAD_CAST( "name" ), BAD_CAST( "C" ));

	xmlNewChild( locale_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LOCALE_SHORT ), BAD_CAST( def->short_label ));

	xmlNewChild( locale_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LOCALE_LONG ), BAD_CAST( def->long_label ));

	parent_value_node = def->localizable ? locale_node : schema_node;

	content = xmlEncodeSpecialChars( doc, BAD_CAST( def->default_value ));
	xmlNewChild( parent_value_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_DEFAULT ), content );
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
