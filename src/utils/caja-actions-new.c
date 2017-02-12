/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
 * Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
 * Copyright (C) 2012-2017 Wolfgang Ulbrich and others (see AUTHORS)
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
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <locale.h>
#include <stdlib.h>

#include <api/na-core-utils.h>
#include <api/na-iio-provider.h>
#include <api/na-object-api.h>

#include <core/na-io-provider.h>
#include <core/na-exporter.h>
#include <core/na-updater.h>

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
static gboolean   nolocation       = FALSE;
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
static gchar     *selection_count  = "";
static gchar    **onlyshow_array   = NULL;
static gchar    **notshow_array    = NULL;
static gchar     *try_exec         = "";
static gchar     *show_registered  = "";
static gchar     *show_true        = "";
static gchar     *show_running     = "";
static gchar    **capability_array = NULL;
/* output entries */
static gboolean   output_stdout    = FALSE;
static gboolean   output_desktop   = FALSE;
/* misc entries */
static gboolean   version          = FALSE;

extern NADataGroup action_data_groups[];			/* defined in na-object-action-factory.c */
extern NADataGroup profile_data_groups[];			/* defined in na-object-profile-factory.c */

static const ArgFromDataDef st_arg_from_data_def[] = {
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_LABEL,              &label },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_TOOLTIP,            &tooltip },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_ICON,               &icon },
		{ action_data_groups,  NA_FACTORY_OBJECT_ITEM_GROUP,       NAFO_DATA_ENABLED,            &enabled },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_SELECTION,   &target_selection },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_LOCATION,    &target_location },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TARGET_TOOLBAR,     &target_toolbar },
		{ action_data_groups,  NA_FACTORY_OBJECT_ACTION_GROUP,     NAFO_DATA_TOOLBAR_LABEL,      &label_toolbar },
		{ profile_data_groups, NA_FACTORY_OBJECT_PROFILE_GROUP,    NAFO_DATA_PATH,               &command },
		{ profile_data_groups, NA_FACTORY_OBJECT_PROFILE_GROUP,    NAFO_DATA_PARAMETERS,         &parameters },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_BASENAMES,          &basenames_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MATCHCASE,          &matchcase },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MIMETYPES,          &mimetypes_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_ISFILE,             &isfile },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_ISDIR,              &isdir },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_MULTIPLE,           &accept_multiple },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_SCHEMES,            &schemes_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_FOLDERS,            &folders_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_SELECTION_COUNT,    &selection_count },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_ONLY_SHOW,          &onlyshow_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_NOT_SHOW,           &notshow_array },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_TRY_EXEC,           &try_exec },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_SHOW_IF_REGISTERED, &show_registered },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_SHOW_IF_TRUE,       &show_true },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_SHOW_IF_RUNNING,    &show_running },
		{ profile_data_groups, NA_FACTORY_OBJECT_CONDITIONS_GROUP, NAFO_DATA_CAPABILITITES,      &capability_array },
		{ NULL }
};

static GOptionEntry st_added_entries[] = {

	{ "disabled"             , 'E', 0, G_OPTION_ARG_NONE        , &disabled,
			N_( "Set it if the item should be disabled at creation" ), NULL },
	{ "nocontext"            , 'C', 0, G_OPTION_ARG_NONE        , &nocontext,
			N_( "Set it if the item doesn't target the selection context menu" ), NULL },
	{ "nolocation"           , 'T', 0, G_OPTION_ARG_NONE        , &nolocation,
			N_( "Set it if the item doesn't target the location context menu" ), NULL },
	{ "notoolbar"            , 'O', 0, G_OPTION_ARG_NONE        , &notoolbar,
			N_( "Set it if the item doesn't target the toolbar" ), NULL },
	{ "nocase"               , 'A', 0, G_OPTION_ARG_NONE        , &nocase,
			N_( "Set it if the basename patterns are case insensitive" ), NULL },
	{ NULL }
};

static GOptionEntry output_entries[] = {

	{ "stdout" , 's', 0, G_OPTION_ARG_NONE, &output_stdout,  N_("Output the new item content on stdout (default)"), NULL },
	{ "desktop", 'd', 0, G_OPTION_ARG_NONE, &output_desktop, N_("Create the new item as a .desktop file"), NULL },
	{ NULL }
};

static GOptionEntry misc_entries[] = {

	{ "version"              , 'v', 0, G_OPTION_ARG_NONE        , &version,
			N_( "Output the version number" ), NULL },
	{ NULL }
};

#define CANNOT_BOTH		_( "Error: '%s' and '%s' options cannot both be specified\n" )
#define DEPRECATED		_( "'%s' option is deprecated, see %s\n" )

static GOptionEntry   *build_option_entries( const ArgFromDataDef *defs, guint nbdefs, const GOptionEntry *adds, guint nbadds );
static GOptionContext *init_options( void );
static NAObjectAction *get_action_from_cmdline( void );
static gboolean        output_to_desktop( NAObjectAction *action, GSList **msgs );
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

	if( target_location && nolocation ){
		g_printerr( CANNOT_BOTH, "--location", "--nolocation" );
		errors += 1;
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

	if( accept_multiple && strlen( selection_count )){
		g_printerr( CANNOT_BOTH, "--accept-multiple", "--selection-count" );
		errors += 1;
	}

	if( onlyshow_array && notshow_array ){
		g_printerr( CANNOT_BOTH, "--only-show-in", "--not-show-in" );
		errors += 1;
	}

	if( output_stdout && output_desktop ){
		g_printerr( _( "Error: only one output option may be specified.\n" ));
		errors += 1;
	}

	if( errors ){
		exit_with_usage();
	}

	action = get_action_from_cmdline();

	if( output_desktop ){
		output_to_desktop( action, &msg );
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

	context = g_option_context_new( _( "Define a new action." ));

	g_option_context_set_summary( context, _(
			"The created action defaults to be written to stdout.\n"
			"It can also be written to an output folder, in a file later suitable for an import in CACT.\n"
			"Or you may choose to directly write the action into your Caja-Actions configuration." ));

	g_option_context_set_translation_domain( context, GETTEXT_PACKAGE );

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

	g_free( entries );

	description = console_cmdline_get_description();
	g_option_context_set_description( context, description );
	g_free( description );

	output_group = g_option_group_new(
			"output", _( "Output of the program" ), _( "Choose where the program creates the action" ), NULL, NULL );
	g_option_group_add_entries( output_group, output_entries );
	g_option_group_set_translation_domain( output_group, GETTEXT_PACKAGE );
	g_option_context_add_group( context, output_group );

	misc_group = g_option_group_new(
			"misc", _( "Miscellaneous options" ), _( "Miscellaneous options" ), NULL, NULL );
	g_option_group_add_entries( misc_group, misc_entries );
	g_option_group_set_translation_domain( misc_group, GETTEXT_PACKAGE );
	g_option_context_add_group( context, misc_group );

	return( context );
}

/*
 * allocate a new action, and fill it with values read from command-line
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
	gchar *msg;
	GSList *only_show_in;
	GSList *not_show_in;
	GSList *capabilities;

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
	na_object_set_target_location( action, target_location );
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

	mimetypes = NULL;
	if( isfile ){
		msg = g_strdup_printf( DEPRECATED, "accept-files", "mimetype" );
		g_warning( "%s", msg );
		g_free( msg );
	}
	if( isdir ){
		msg = g_strdup_printf( DEPRECATED, "accept-dirs", "mimetype" );
		g_warning( "%s", msg );
		g_free( msg );
	}
	if( isfile && !isdir ){
		mimetypes = g_slist_prepend( mimetypes, g_strdup( "all/allfiles" ));
	} else if( isdir && !isfile ){
		mimetypes = g_slist_prepend( mimetypes, g_strdup( "inode/directory" ));
	}
	i = 0;
	while( mimetypes_array != NULL && mimetypes_array[i] != NULL ){
		mimetypes = g_slist_append( mimetypes, g_strdup( mimetypes_array[i] ));
		i++;
	}
	if( mimetypes && g_slist_length( mimetypes )){
		na_object_set_mimetypes( profile, mimetypes );
		na_core_utils_slist_free( mimetypes );
	}

	if( accept_multiple ){
		msg = g_strdup_printf( DEPRECATED, "accept-multiple", "selection-count" );
		g_warning( "%s", msg );
		g_free( msg );
		selection_count = g_strdup( ">0" );
	}
	if( strlen( selection_count )){
		na_object_set_selection_count( profile, selection_count );
	}

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

	if( onlyshow_array ){
		only_show_in = NULL;
		for( i = 0 ; onlyshow_array[i] && strlen( onlyshow_array[i] ) ; ++i ){
			only_show_in = g_slist_append( only_show_in, g_strdup( onlyshow_array[i] ));
		}
		if( only_show_in && g_slist_length( only_show_in )){
			na_object_set_only_show_in( profile, only_show_in );
			na_core_utils_slist_free( only_show_in );
		}
	}

	if( notshow_array ){
		not_show_in = NULL;
		for( i = 0 ; notshow_array[i] && strlen( notshow_array[i] ) ; ++i ){
			not_show_in = g_slist_append( not_show_in, g_strdup( notshow_array[i] ));
		}
		if( not_show_in && g_slist_length( not_show_in )){
			na_object_set_not_show_in( profile, not_show_in );
			na_core_utils_slist_free( not_show_in );
		}
	}

	if( try_exec && strlen( try_exec )){
		na_object_set_try_exec( profile, try_exec );
	}

	if( show_registered && strlen( show_registered )){
		na_object_set_show_if_registered( profile, show_registered );
	}

	if( show_true && strlen( show_true )){
		na_object_set_show_if_true( profile, show_true );
	}

	if( show_running && strlen( show_running )){
		na_object_set_show_if_running( profile, show_running );
	}

	if( capability_array ){
		capabilities = NULL;
		for( i = 0 ; capability_array[i] && strlen( capability_array[i] ) ; ++i ){
			const gchar *cap = ( const gchar * ) capability_array[i];
			/* 'Owner', 'Readable', 'Writable', 'Executable' and 'Local' */
			if( strcmp( cap, "Owner" ) &&
				strcmp( cap, "Readable" ) &&
				strcmp( cap, "Writable" ) &&
				strcmp( cap, "Executable" ) &&
				strcmp( cap, "Local" )){
					g_warning( "%s: unknown capability", cap );
			}  else {
				capabilities = g_slist_append( capabilities, g_strdup( capability_array[i] ));
			}
		}
		if( capabilities && g_slist_length( capabilities )){
			na_object_set_capabilities( profile, capabilities );
			na_core_utils_slist_free( capabilities );
		}
	}

	return( action );
}

/*
 * write the new item as a .desktop file
 */
static gboolean
output_to_desktop( NAObjectAction *action, GSList **msgs )
{
	NAUpdater *updater;
	NAIOProvider *provider;
	guint ret;
	gboolean code;

	updater = na_updater_new();
	provider = na_io_provider_find_io_provider_by_id( NA_PIVOT( updater ), "na-desktop" );

	if( provider ){
		na_object_set_provider( action, provider );
		ret = na_updater_write_item( updater, NA_OBJECT_ITEM( action ), msgs );
		code = ( ret == NA_IIO_PROVIDER_CODE_OK );

	} else {
		/* i18n: 'na-desktop' is a plugin identifier - do not translate */
		*msgs = g_slist_append( *msgs, _( "Error: unable to find 'na-desktop' i/o provider." ));
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
	gchar *buffer;

	updater = na_updater_new();
	buffer = na_exporter_to_buffer( NA_PIVOT( updater ), NA_OBJECT_ITEM( action ), "Desktop1", msgs );
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
	/* i18: '--help' is a command-line option - do not translate */
	g_printerr( _( "Try %s --help for usage.\n" ), g_get_prgname());
	exit( EXIT_FAILURE );
}
