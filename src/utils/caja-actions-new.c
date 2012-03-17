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
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <stdlib.h>

#include <api/na-core-utils.h>
#include <api/na-iio-provider.h>
#include <api/na-object-api.h>

#include <core/na-io-provider.h>
#include <core/na-exporter.h>
#include <core/na-updater.h>

#include <io-mateconf/nagp-keys.h>

#include <io-xml/naxml-formats.h>

#include "console-utils.h"

typedef struct {
	NADataGroup *group;
	gchar       *group_name;
	gchar       *data_name;
	void        *arg_data;
}
	ArgFromDataDef;

static gchar     *label            = "";
static gchar     *tooltip          = "";
static gchar     *icon             = "";
static gboolean   enabled          = FALSE;
static gboolean   disabled         = FALSE;
static gboolean   target_selection = FALSE;
static gboolean   target_location  = FALSE;
static gboolean   nocontext        = FALSE;
static gboolean   target_toolbar   = FALSE;
static gboolean   notoolbar        = FALSE;
static gchar     *label_toolbar    = "";
static gchar     *command          = "";
static gchar     *parameters       = "";
static gchar    **basenames_array  = NULL;
static gboolean   matchcase        = FALSE;
static gboolean   nocase           = FALSE;
static gchar    **mimetypes_array  = NULL;
static gboolean   isfile           = FALSE;
static gboolean   isdir            = FALSE;
static gboolean   accept_multiple  = FALSE;
static gchar    **schemes_array    = NULL;
static gchar    **folders_array    = NULL;
static gchar     *output_dir       = NULL;
static gboolean   output_mateconf     = FALSE;
static gboolean   version          = FALSE;

extern NADataGroup action_data_groups[];			/* defined in na-object-action-factory.c */
extern NADataGroup profile_data_groups[];			/* defined in na-object-profile-factory.c */

static const ArgFromDataDef st_arg_from_data_def[] = {
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_LABEL,            &label },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_TOOLTIP,          &tooltip },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_ICON,             &icon },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_ENABLED,          &enabled },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_SELECTION, &target_selection },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_LOCATION,  &target_location },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_TOOLBAR,   &target_toolbar },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TOOLBAR_LABEL,    &label_toolbar },
		{ profile_data_groups, NA_FACTORY_OBJECT_PROFILE_GROUP,    NAFO_DATA_PATH,             &command },
		{ profile_data_groups, NA_FACTORY_OBJECT_PROFILE_GROUP,    NAFO_DATA_PARAMETERS,       &parameters },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_BASENAMES,        &basenames_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MATCHCASE,        &matchcase },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MIMETYPES,        &mimetypes_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_ISFILE,           &isfile },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_ISDIR,            &isdir },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MULTIPLE,         &accept_multiple },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_SCHEMES,          &schemes_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_FOLDERS,          &folders_array },
		{ NULL }
};

static GOptionEntry st_added_entries[] = {

	{ "disabled"             , 'E', 0, G_OPTION_ARG_NONE        , &disabled,
			N_( "Set it if the item should be disabled at creation" ), NULL },
	{ "nocontext"            , 'C', 0, G_OPTION_ARG_NONE        , &nocontext,
			N_( "Set it if the item doesn't target the context menu" ), NULL },
	{ "notoolbar"            , 'O', 0, G_OPTION_ARG_NONE        , &notoolbar,
			N_( "Set it if the item doesn't target the toolbar" ), NULL },
	{ "nocase"               , 'A', 0, G_OPTION_ARG_NONE        , &nocase,
			N_( "Set it if the basename patterns are case insensitive" ), NULL },
	{ NULL }
};

static GOptionEntry output_entries[] = {

	{ "output-mateconf"         , 'g', 0, G_OPTION_ARG_NONE        , &output_mateconf,
			N_( "Store the newly created action as a MateConf configuration" ), NULL },
	{ "output-dir"           , 'o', 0, G_OPTION_ARG_FILENAME    , &output_dir,
			N_( "The path of the folder where to write the new action as a MateConf dump output [default: stdout]" ), N_( "<PATH>" ) },
	{ NULL }
};

static GOptionEntry misc_entries[] = {

	{ "version"              , 'v', 0, G_OPTION_ARG_NONE        , &version,
			N_( "Output the version number" ), NULL },
	{ NULL }
};

#define CANNOT_BOTH		_( "Error: '%s' and '%s' options cannot both be specified.\n" )

static GOptionEntry   *build_option_entries( const ArgFromDataDef *defs, guint nbdefs, const GOptionEntry *adds, guint nbadds );
static GOptionContext *init_options( void );
static NAObjectAction *get_action_from_cmdline( void );
static gboolean        output_to_dir( NAObjectAction *action, gchar *dir, GSList **msgs );
static gboolean        output_to_mateconf( NAObjectAction *action, GSList **msgs );
static gboolean        output_to_stdout( const NAObjectAction *action, GSList **msgs );
static void            exit_with_usage( void );

int
main( int argc, char** argv )
{
	int status = EXIT_SUCCESS;
	GOptionContext *context;
	GError *error = NULL;
	NAObjectAction *action;
	GSList *msg = NULL;
	GSList *im;
	gchar *help;
	gint errors;

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
		g_printerr( _( "Syntax error: %s\n" ), error->message );
		g_error_free (error);
		exit_with_usage();
	}

	if( version ){
		na_core_utils_print_version();
		exit( status );
	}

	errors = 0;

	if( !label || !g_utf8_strlen( label, -1 )){
		g_printerr( _( "Error: an action label is mandatory.\n" ));
		errors += 1;
	}

	if( enabled && disabled ){
		g_printerr( CANNOT_BOTH, "--enabled", "--disabled" );
		errors += 1;
	} else if( !disabled ){
		enabled = TRUE;
	}

	if( target_selection && nocontext ){
		g_printerr( CANNOT_BOTH, "--context", "--nocontext" );
		errors += 1;
	} else if( !nocontext ){
		target_selection = TRUE;
	}

	if( target_toolbar && notoolbar ){
		g_printerr( CANNOT_BOTH, "--toolbar", "--notoolbar" );
		errors += 1;
	}

	if( matchcase && nocase ){
		g_printerr( CANNOT_BOTH, "--match-case", "--nocase" );
		errors += 1;
	} else if( !nocase ){
		matchcase = TRUE;
	}

	if( output_mateconf && output_dir ){
		g_printerr( _( "Error: only one output option may be specified.\n" ));
		errors += 1;
	}

	if( errors ){
		exit_with_usage();
	}

	action = get_action_from_cmdline();

	if( output_mateconf ){
		if( output_to_mateconf( action, &msg )){
			/* i18n: Action <action_label> written to...*/
			g_print( _( "Action '%s' succesfully written to MateConf configuration.\n" ), label );
		}

	} else if( output_dir ){
		output_to_dir( action, output_dir, &msg );

	} else {
		output_to_stdout( action, &msg );
	}

	if( msg ){
		for( im = msg ; im ; im = im->next ){
			g_printerr( "%s\n", ( gchar * ) im->data );
		}
		na_core_utils_slist_free( msg );
		status = EXIT_FAILURE;
	}

	g_object_unref( action );
	g_option_context_free( context );

	exit( status );
}

static GOptionEntry *
build_option_entries( const ArgFromDataDef *defs, guint nbdefs, const GOptionEntry *adds, guint nbadds )
{
	static const gchar *thisfn = "caja_actions_new_build_option_entries";
	GOptionEntry *entries;
	GOptionEntry *ient;
	const GOptionEntry *iadd;
	const ArgFromDataDef *idef;
	const NADataDef *data_def;

	entries = g_new0( GOptionEntry, 1+nbdefs+nbadds );
	ient = entries;

	idef = defs;
	while( idef->group ){
		data_def = na_data_def_get_data_def( idef->group, idef->group_name, idef->data_name );

		if( data_def ){
			ient->long_name = data_def->option_long;
			ient->short_name = data_def->option_short;
			ient->flags = data_def->option_flags;
			ient->arg = data_def->option_arg;
			ient->arg_data = idef->arg_data;
			ient->description = data_def->option_label ? data_def->option_label : data_def->short_label;
			ient->arg_description = data_def->option_arg_label;

		} else {
			g_warning( "%s: group=%s, name=%s: unable to find NADataDef structure", thisfn, idef->group_name, idef->data_name );
		}

		idef++;
		ient++;
	}

	iadd = adds;
	while( iadd->long_name ){
		memcpy( ient, iadd, sizeof( GOptionEntry ));
		iadd++;
		ient++;
	}

	return( entries );
}

/*
 * init options context
 */
static GOptionContext *
init_options( void )
{
	GOptionContext *context;
	gchar* description;
	GOptionGroup *output_group;
	GOptionGroup *misc_group;
	GOptionEntry *entries;

	context = g_option_context_new( _( "Define a new action.\n\n"
			"  The created action defaults to be written to stdout.\n"
			"  It can also be written to an output folder, in a file later suitable for an import in NACT.\n"
			"  Or you may choose to directly write the action into your MateConf configuration." ));

	entries = build_option_entries( st_arg_from_data_def, G_N_ELEMENTS( st_arg_from_data_def ), st_added_entries, G_N_ELEMENTS( st_added_entries ) );

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

	g_free( entries );
	g_free( description );

	output_group = g_option_group_new(
			"output", _( "Output of the program" ), _( "Choose where the program creates the action" ), NULL, NULL );
	g_option_group_add_entries( output_group, output_entries );
	g_option_context_add_group( context, output_group );

	misc_group = g_option_group_new(
			"misc", _( "Miscellaneous options" ), _( "Miscellaneous options" ), NULL, NULL );
	g_option_group_add_entries( misc_group, misc_entries );
	g_option_context_add_group( context, misc_group );

	return( context );
}

/*
 * allocate a new action, and fill it with values readen from command-line
 */
static NAObjectAction *
get_action_from_cmdline( void )
{
	NAObjectAction *action;
	NAObjectProfile *profile;
	int i;
	GSList *basenames;
	GSList *mimetypes;
	GSList *schemes;
	GSList *folders;
	gboolean toolbar_same_label;

	action = na_object_action_new_with_defaults();
	profile = NA_OBJECT_PROFILE(( GList * ) na_object_get_items( action )->data );

	na_object_set_label( action, label );
	if( tooltip && g_utf8_strlen( tooltip, -1 )){
		na_object_set_tooltip( action, tooltip );
	}
	if( icon && g_utf8_strlen( icon, -1 )){
		na_object_set_icon( action, icon );
	}
	na_object_set_enabled( action, enabled );
	na_object_set_target_selection( action, target_selection );
	na_object_set_target_toolbar( action, target_toolbar );

	toolbar_same_label = FALSE;
	if( !label_toolbar || !g_utf8_strlen( label_toolbar, -1 )){
		label_toolbar = g_strdup( label );
		toolbar_same_label = TRUE;
	}
	na_object_set_toolbar_same_label( action, toolbar_same_label );
	na_object_set_toolbar_label( action, label_toolbar );

	na_object_set_path( profile, command );
	na_object_set_parameters( profile, parameters );

	i = 0;
	basenames = NULL;
	while( basenames_array != NULL && basenames_array[i] != NULL ){
		basenames = g_slist_append( basenames, g_strdup( basenames_array[i] ));
		i++;
	}
	if( basenames && g_slist_length( basenames )){
		na_object_set_basenames( profile, basenames );
		na_core_utils_slist_free( basenames );
	}

	na_object_set_matchcase( profile, matchcase );

	i = 0;
	mimetypes = NULL;
	while( mimetypes_array != NULL && mimetypes_array[i] != NULL ){
		mimetypes = g_slist_append( mimetypes, g_strdup( mimetypes_array[i] ));
		i++;
	}
	if( mimetypes && g_slist_length( mimetypes )){
		na_object_set_mimetypes( profile, mimetypes );
		na_core_utils_slist_free( mimetypes );
	}

	if( !isfile && !isdir ){
		isfile = TRUE;
	}
	na_object_set_isfile( profile, isfile );
	na_object_set_isdir( profile, isdir );
	na_object_set_multiple( profile, accept_multiple );

	i = 0;
	schemes = NULL;
	while( schemes_array != NULL && schemes_array[i] != NULL ){
		schemes = g_slist_append( schemes, g_strdup( schemes_array[i] ));
		i++;
	}
	if( schemes && g_slist_length( schemes )){
		na_object_set_schemes( profile, schemes );
		na_core_utils_slist_free( schemes );
	}

	i = 0;
	folders = NULL;
	while( folders_array != NULL && folders_array[i] != NULL ){
		folders = g_slist_append( folders, g_strdup( folders_array[i] ));
		i++;
	}
	if( folders && g_slist_length( folders )){
		na_object_set_folders( profile, folders );
		na_core_utils_slist_free( folders );
	}

	return( action );
}

static gboolean
output_to_dir( NAObjectAction *action, gchar *dir, GSList **msgs )
{
	gboolean code;
	gboolean writable_dir;
	gchar *msg;
	NAUpdater *updater;
	GQuark format;
	gchar *fname;

	code = FALSE;
	writable_dir = FALSE;

	if( na_core_utils_dir_is_writable_path( dir )){
		writable_dir = TRUE;

	} else if( g_mkdir_with_parents( dir, 0700 )){
		/* i18n: unable to create <output_dir> dir: <strerror_message> */
		msg = g_strdup_printf( _( "Error: unable to create %s dir: %s" ), dir, g_strerror( errno ));
		*msgs = g_slist_append( *msgs, msg );

	} else {
		writable_dir = na_core_utils_dir_is_writable_path( dir );
	}

	if( writable_dir ){
		updater = na_updater_new();
		format = g_quark_from_string( NAXML_FORMAT_MATECONF_ENTRY );
		fname = na_exporter_to_file( NA_PIVOT( updater ), NA_OBJECT_ITEM( action ), dir, format, msgs );

		if( fname ){
			/* i18n: Action <action_label> written to <output_filename>...*/
			g_print( _( "Action '%s' succesfully written to %s, and ready to be imported in NACT.\n" ), label, fname );
			g_free( fname );
		}

		g_object_unref( updater );
	}

	return( code );
}

/*
 * initialize MateConf as an I/O provider
 * then writes the action
 */
static gboolean
output_to_mateconf( NAObjectAction *action, GSList **msgs )
{
	NAUpdater *updater;
	GList *providers;
	NAIOProvider *provider;
	guint ret;
	gboolean code;

	updater = na_updater_new();
	providers = na_io_provider_get_providers_list( NA_PIVOT( updater ));
	provider = na_io_provider_find_provider_by_id( providers, "na-mateconf" );

	if( provider ){
		na_object_set_provider( action, provider );
		ret = na_updater_write_item( updater, NA_OBJECT_ITEM( action ), msgs );
		code = ( ret == NA_IIO_PROVIDER_CODE_OK );

	} else {
		*msgs = g_slist_append( *msgs, _( "Error: unable to find 'na-mateconf' provider." ));
		code = FALSE;
	}

	g_object_unref( updater );

	return( code );
}

static gboolean
output_to_stdout( const NAObjectAction *action, GSList **msgs )
{
	gboolean ret;
	NAUpdater *updater;
	GQuark format;
	gchar *buffer;

	updater = na_updater_new();
	format = g_quark_from_string( NAXML_FORMAT_MATECONF_ENTRY );
	buffer = na_exporter_to_buffer( NA_PIVOT( updater ), NA_OBJECT_ITEM( action ), format, msgs );
	ret = ( buffer != NULL );

	if( buffer ){
		g_printf( "%s", buffer );
		g_free( buffer );
	}

	g_object_unref( updater );

	return( ret );
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
