/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
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
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-factory-object.h>
#include <core/na-tokens.h>

#include "base-window.h"
#include "cact-application.h"
#include "cact-main-statusbar.h"
#include "base-gtk-utils.h"
#include "cact-main-tab.h"
#include "cact-icommand-tab.h"
#include "cact-ischemes-tab.h"

/* private interface data
 */
struct _CactICommandTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* a data set in the LegendDialog GObject
 */
#define ICOMMAND_TAB_LEGEND_VISIBLE			"cact-icommand-tab-legend-dialog-visible"

/* data set against the instance
 */
typedef struct {
	gboolean  on_selection_change;
	NATokens *tokens;
}
	ICommandData;

#define ICOMMAND_TAB_PROP_DATA				"cact-icommand-tab-data"

static guint st_initializations = 0;	/* interface initialization count */

static GType         register_type( void );
static void          interface_base_init( CactICommandTabInterface *klass );
static void          interface_base_finalize( CactICommandTabInterface *klass );

static void          on_base_initialize_gtk( CactICommandTab *instance, GtkWindow *toplevel, gpointer user_data );
static void          on_base_initialize_window( CactICommandTab *instance, gpointer user_data );

static void          on_tree_view_content_changed( CactICommandTab *instance, NAObject *object, gpointer user_data );
static void          on_main_selection_changed( CactICommandTab *instance, GList *selected_items, gpointer user_data );

static GtkWidget    *get_label_entry( CactICommandTab *instance );
static GtkButton    *get_legend_button( CactICommandTab *instance );
static GtkWindow    *get_legend_dialog( CactICommandTab *instance );
static GtkWidget    *get_parameters_entry( CactICommandTab *instance );
static GtkButton    *get_path_button( CactICommandTab *instance );
static GtkWidget    *get_path_entry( CactICommandTab *instance );
static void          legend_dialog_show( CactICommandTab *instance );
static void          legend_dialog_hide( CactICommandTab *instance );
static void          on_label_changed( GtkEntry *entry, CactICommandTab *instance );
static void          on_legend_clicked( GtkButton *button, CactICommandTab *instance );
static gboolean      on_legend_dialog_deleted( GtkWidget *dialog, GdkEvent *event, CactICommandTab *instance );
static void          on_parameters_changed( GtkEntry *entry, CactICommandTab *instance );
static void          on_path_browse( GtkButton *button, CactICommandTab *instance );
static void          on_path_changed( GtkEntry *entry, CactICommandTab *instance );
static void          on_wdir_browse( GtkButton *button, CactICommandTab *instance );
static void          on_wdir_changed( GtkEntry *entry, CactICommandTab *instance );
static gchar        *parse_parameters( CactICommandTab *instance );
static void          update_example_label( CactICommandTab *instance, NAObjectProfile *profile );

static ICommandData *get_icommand_data( CactICommandTab *instance );
static void          on_instance_finalized( gpointer user_data, CactICommandTab *instance );

GType
cact_icommand_tab_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = register_type();
	}

	return( iface_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_icommand_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactICommandTabInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_INTERFACE, "CactICommandTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_TYPE_WINDOW );

	return( type );
}

static void
interface_base_init( CactICommandTabInterface *klass )
{
	static const gchar *thisfn = "cact_icommand_tab_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactICommandTabInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( CactICommandTabInterface *klass )
{
	static const gchar *thisfn = "cact_icommand_tab_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/*
 * cact_icommand_tab_init:
 * @instance: this #CactICommandTab instance.
 *
 * Initialize the interface
 * Connect to #BaseWindow signals
 */
void
cact_icommand_tab_init( CactICommandTab *instance )
{
	static const gchar *thisfn = "cact_icommand_tab_init";
	ICommandData *data;

	g_return_if_fail( CACT_IS_ICOMMAND_TAB( instance ));

	g_debug( "%s: instance=%p (%s)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_GTK,
			G_CALLBACK( on_base_initialize_gtk ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_base_initialize_window ));

	cact_main_tab_init( CACT_MAIN_WINDOW( instance ), TAB_COMMAND );

	data = get_icommand_data( instance );
	data->on_selection_change = FALSE;
	data->tokens = NULL;

	g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
}

static void
on_base_initialize_gtk( CactICommandTab *instance, GtkWindow *toplevel, gpointer user_data )
{
	base_gtk_utils_table_to_grid( BASE_WINDOW( instance ), "table220" );
}

/*
 * on_base_initialize_window:
 * @window: this #CactICommandTab instance.
 *
 * Initializes the tab widget at each time the widget will be displayed.
 * Connect signals and setup runtime values.
 */
static void
on_base_initialize_window( CactICommandTab *instance, void *user_data )
{
	static const gchar *thisfn = "cact_icommand_tab_on_base_initialize_window";
	GtkWindow *legend_dialog;
	GtkWidget *label_entry, *path_entry, *parameters_entry, *wdir_entry;
	GtkButton *path_button, *legend_button, *wdir_button;
	ICommandData *data;

	g_return_if_fail( CACT_IS_ICOMMAND_TAB( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	label_entry = get_label_entry( instance );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( label_entry ),
			"changed",
			G_CALLBACK( on_label_changed ));

	path_entry = get_path_entry( instance );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( path_entry ),
			"changed",
			G_CALLBACK( on_path_changed ));

	path_button = get_path_button( instance );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( path_button ),
			"clicked",
			G_CALLBACK( on_path_browse ));

	parameters_entry = get_parameters_entry( instance );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( parameters_entry ),
			"changed",
			G_CALLBACK( on_parameters_changed ));

	legend_button = get_legend_button( instance );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( legend_button ),
			"clicked",
			G_CALLBACK( on_legend_clicked ));

	legend_dialog = get_legend_dialog( instance );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( legend_dialog ),
			"delete-event",
			G_CALLBACK( on_legend_dialog_deleted ));

	wdir_entry = base_window_get_widget( BASE_WINDOW( instance ), "WorkingDirectoryEntry" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( wdir_entry ),
			"changed",
			G_CALLBACK( on_wdir_changed ));

	wdir_button = GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "CommandWorkingDirectoryButton" ));
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( wdir_button ),
			"clicked",
			G_CALLBACK( on_wdir_browse ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_ITEM_UPDATED,
			G_CALLBACK( on_tree_view_content_changed ));

	/* allocate a static fake NATokens object which will be used to build
	 * the example label - this object will be g_object_unref() on instance
	 * finalization
	 */
	data = get_icommand_data( instance );
	if( !data->tokens ){
		data->tokens = na_tokens_new_for_example();
	}
}

static void
on_tree_view_content_changed( CactICommandTab *instance, NAObject *object, gpointer user_data )
{
	GtkWidget *label_widget;
	gchar *label;

	g_return_if_fail( CACT_IS_ICOMMAND_TAB( instance ));

	if( NA_IS_OBJECT_PROFILE( object )){
		label_widget = get_label_entry( instance );
		label = na_object_get_label( object );
		gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
		g_free( label );
	}
}

static void
on_main_selection_changed( CactICommandTab *instance, GList *selected_items, gpointer user_data )
{
	static const gchar *thisfn = "cact_icommand_tab_on_main_selection_changed";
	NAObjectProfile *profile;
	gboolean editable;
	gboolean enable_tab;
	GtkWidget *label_entry, *path_entry, *parameters_entry, *wdir_entry;
	gchar *label, *path, *parameters, *wdir;
	GtkButton *path_button, *wdir_button;
	GtkButton *legend_button;
	ICommandData *data;

	g_return_if_fail( CACT_IS_ICOMMAND_TAB( instance ));

	g_debug( "%s: instance=%p (%s), selected_items=%p (count=%d)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) selected_items, g_list_length( selected_items ));

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_PROFILE, &profile,
			MAIN_PROP_EDITABLE, &editable,
			NULL );

	enable_tab = ( profile != NULL );
	cact_main_tab_enable_page( CACT_MAIN_WINDOW( instance ), TAB_COMMAND, enable_tab );

	data = get_icommand_data( instance );
	data->on_selection_change = TRUE;

	label_entry = get_label_entry( instance );
	label = profile ? na_object_get_label( profile ) : g_strdup( "" );
	label = label ? label : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( label_entry ), label );
	g_free( label );
	gtk_widget_set_sensitive( label_entry, profile != NULL );
	base_gtk_utils_set_editable( G_OBJECT( label_entry ), editable );

	path_entry = get_path_entry( instance );
	path = profile ? na_object_get_path( profile ) : g_strdup( "" );
	path = path ? path : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( path_entry ), path );
	g_free( path );
	gtk_widget_set_sensitive( path_entry, profile != NULL );
	base_gtk_utils_set_editable( G_OBJECT( path_entry ), editable );

	path_button = get_path_button( instance );
	gtk_widget_set_sensitive( GTK_WIDGET( path_button ), profile != NULL );
	base_gtk_utils_set_editable( G_OBJECT( path_button ), editable );

	parameters_entry = get_parameters_entry( instance );
	parameters = profile ? na_object_get_parameters( profile ) : g_strdup( "" );
	parameters = parameters ? parameters : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( parameters_entry ), parameters );
	g_free( parameters );
	gtk_widget_set_sensitive( parameters_entry, profile != NULL );
	base_gtk_utils_set_editable( G_OBJECT( parameters_entry ), editable );

	legend_button = get_legend_button( instance );
	gtk_widget_set_sensitive( GTK_WIDGET( legend_button ), profile != NULL );

	update_example_label( instance, profile );

	wdir_entry = base_window_get_widget( BASE_WINDOW( instance ), "WorkingDirectoryEntry" );
	wdir = profile ? na_object_get_working_dir( profile ) : g_strdup( "" );
	wdir = wdir ? wdir : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( wdir_entry ), wdir );
	g_free( wdir );
	gtk_widget_set_sensitive( wdir_entry, profile != NULL );
	base_gtk_utils_set_editable( G_OBJECT( wdir_entry ), editable );

	wdir_button = GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "CommandWorkingDirectoryButton" ));
	gtk_widget_set_sensitive( GTK_WIDGET( wdir_button ), profile != NULL );
	base_gtk_utils_set_editable( G_OBJECT( wdir_button ), editable );

	data->on_selection_change = FALSE;
}

static GtkWidget *
get_label_entry( CactICommandTab *instance )
{
	return( base_window_get_widget( BASE_WINDOW( instance ), "ProfileLabelEntry" ));
}

static GtkButton *
get_legend_button( CactICommandTab *instance )
{
	return( GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "CommandLegendButton" )));
}

static GtkWindow *
get_legend_dialog( CactICommandTab *instance )
{
	return( base_window_get_gtk_toplevel_by_name( BASE_WINDOW( instance ), "LegendDialog" ));
}

static GtkWidget *
get_parameters_entry( CactICommandTab *instance )
{
	return( base_window_get_widget( BASE_WINDOW( instance ), "CommandParametersEntry" ));
}

static GtkButton *
get_path_button( CactICommandTab *instance )
{
	return( GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "CommandPathButton" )));
}

static GtkWidget *
get_path_entry( CactICommandTab *instance )
{
	return( base_window_get_widget( BASE_WINDOW( instance ), "CommandPathEntry" ));
}

static void
legend_dialog_hide( CactICommandTab *instance )
{
	GtkWindow *legend_dialog;
	GtkButton *legend_button;
	gboolean is_visible;

	is_visible = FALSE;
	legend_dialog = get_legend_dialog( instance );
	if( GTK_IS_WINDOW( legend_dialog )){
		is_visible = ( gboolean ) GPOINTER_TO_INT(
				g_object_get_data( G_OBJECT( legend_dialog ), ICOMMAND_TAB_LEGEND_VISIBLE ));
	}

	if( is_visible ){
		base_gtk_utils_save_window_position( BASE_WINDOW( instance ), NA_IPREFS_COMMAND_LEGEND_WSP );
		gtk_widget_hide( GTK_WIDGET( legend_dialog ));

		/* set the legend button state consistent for when the dialog is
		 * hidden by another way (eg. close the edit profile dialog)
		 */
		legend_button = get_legend_button( instance );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( legend_button ), FALSE );

		g_object_set_data( G_OBJECT( legend_dialog ), ICOMMAND_TAB_LEGEND_VISIBLE, GINT_TO_POINTER( FALSE ));
	}
}

static void
legend_dialog_show( CactICommandTab *instance )
{
	GtkWindow *legend_dialog;
	GtkWindow *toplevel;

	legend_dialog = get_legend_dialog( instance );
	gtk_window_set_deletable( legend_dialog, FALSE );

	toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( instance ));
	gtk_window_set_transient_for( GTK_WINDOW( legend_dialog ), toplevel );

	base_gtk_utils_restore_window_position( BASE_WINDOW( instance ), NA_IPREFS_COMMAND_LEGEND_WSP );
	gtk_widget_show( GTK_WIDGET( legend_dialog ));

	g_object_set_data( G_OBJECT( legend_dialog ), ICOMMAND_TAB_LEGEND_VISIBLE, GINT_TO_POINTER( TRUE ));
}

static void
on_label_changed( GtkEntry *entry, CactICommandTab *instance )
{
	NAObjectProfile *profile;
	const gchar *label;
	ICommandData *data;

	data = get_icommand_data( instance );

	if( !data->on_selection_change ){
		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_PROFILE, &profile,
				NULL );

		if( profile ){
			label = gtk_entry_get_text( entry );
			na_object_set_label( profile, label );
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, MAIN_DATA_LABEL );
		}
	}
}

static void
on_legend_clicked( GtkButton *button, CactICommandTab *instance )
{
	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
		legend_dialog_show( instance );

	} else {
		legend_dialog_hide( instance );
	}
}

static gboolean
on_legend_dialog_deleted( GtkWidget *dialog, GdkEvent *event, CactICommandTab *instance )
{
	/*g_debug( "on_legend_dialog_deleted" );*/
	legend_dialog_hide( instance );
	return( TRUE );
}

static void
on_parameters_changed( GtkEntry *entry, CactICommandTab *instance )
{
	NAObjectProfile *profile;
	ICommandData *data;

	data = get_icommand_data( instance );

	if( !data->on_selection_change ){
		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_PROFILE, &profile,
				NULL );

		if( profile ){
			na_object_set_parameters( profile, gtk_entry_get_text( entry ));
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, 0 );
			update_example_label( instance, profile );
		}
	}
}

static void
on_path_browse( GtkButton *button, CactICommandTab *instance )
{
	base_gtk_utils_select_file(
			BASE_WINDOW( instance ),
			_( "Choosing a command" ), NA_IPREFS_COMMAND_CHOOSER_WSP,
			get_path_entry( instance ), NA_IPREFS_COMMAND_CHOOSER_URI );
}

static void
on_path_changed( GtkEntry *entry, CactICommandTab *instance )
{
	NAObjectProfile *profile;
	ICommandData *data;

	data = get_icommand_data( instance );

	if( !data->on_selection_change ){
		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_PROFILE, &profile,
				NULL );

		if( profile ){
			na_object_set_path( profile, gtk_entry_get_text( entry ));
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, 0 );
			update_example_label( instance, profile );
		}
	}
}

static void
on_wdir_browse( GtkButton *button, CactICommandTab *instance )
{
	GtkWidget *wdir_entry;
	NAObjectProfile *profile;

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_PROFILE, &profile,
			NULL );

	if( profile ){
		wdir_entry = base_window_get_widget( BASE_WINDOW( instance ), "WorkingDirectoryEntry" );
		base_gtk_utils_select_dir(
				BASE_WINDOW( instance ), _( "Choosing a working directory" ),
				NA_IPREFS_WORKING_DIR_WSP, wdir_entry, NA_IPREFS_WORKING_DIR_URI );
	}
}

static void
on_wdir_changed( GtkEntry *entry, CactICommandTab *instance )
{
	NAObjectProfile *profile;
	ICommandData *data;

	data = get_icommand_data( instance );

	if( !data->on_selection_change ){
		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_PROFILE, &profile,
				NULL );

		if( profile ){
			na_object_set_working_dir( profile, gtk_entry_get_text( entry ));
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, 0 );
		}
	}
}

/*
 * See core/na-tokens.c for valid parameters
 */
static gchar *
parse_parameters( CactICommandTab *instance )
{
	const gchar *command = gtk_entry_get_text( GTK_ENTRY( get_path_entry( instance )));
	const gchar *param_template = gtk_entry_get_text( GTK_ENTRY( get_parameters_entry( instance )));
	gchar *exec, *returned;
	ICommandData *data;

	data = get_icommand_data( instance );
	exec = g_strdup_printf( "%s %s", command, param_template );
	returned = na_tokens_parse_for_display( data->tokens, exec, FALSE );
	g_free( exec );

	return( returned );
}

static void
update_example_label( CactICommandTab *instance, NAObjectProfile *profile )
{
	/*static const char *thisfn = "cact_iconditions_update_example_label";*/
	gchar *newlabel;
	gchar *parameters;
	GtkWidget *example_widget;

	example_widget = base_window_get_widget( BASE_WINDOW( instance ), "CommandExampleLabel" );

	if( profile ){
		parameters = parse_parameters( instance );
		/*g_debug( "%s: parameters=%s", thisfn, parameters );*/

		/* convert special xml chars (&, <, >,...) to avoid warnings
		 * generated by Pango parser
		 */
		/* i18n: command-line example: Ex.: /bin/ls file1.txt file2.txt */
		newlabel = g_markup_printf_escaped(
				"<i><b><span size=\"small\">%s</span></b></i>", parameters );

		g_free( parameters );

	} else {
		newlabel = g_strdup( "" );
	}

	gtk_label_set_label( GTK_LABEL( example_widget ), newlabel );
	g_free( newlabel );
}

static ICommandData *
get_icommand_data( CactICommandTab *instance )
{
	ICommandData *data;

	data = ( ICommandData * ) g_object_get_data( G_OBJECT( instance ), ICOMMAND_TAB_PROP_DATA );

	if( !data ){
		data = g_new0( ICommandData, 1 );
		/* g_object_set_data_full() would be called after the weak ref function
		 */
		g_object_set_data( G_OBJECT( instance ), ICOMMAND_TAB_PROP_DATA, data );
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, CactICommandTab *instance )
{
	static const gchar *thisfn = "cact_icommand_tab_on_instance_finalized";
	ICommandData *data;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );

	legend_dialog_hide( instance );

	data = get_icommand_data( instance );

	if( data->tokens ){
		g_object_unref( data->tokens );
	}

	g_free( data );
}
