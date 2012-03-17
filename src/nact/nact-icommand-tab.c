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
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-iprefs.h>

#include "base-window.h"
#include "base-iprefs.h"
#include "nact-application.h"
#include "nact-main-statusbar.h"
#include "nact-gtk-utils.h"
#include "nact-iprefs.h"
#include "nact-iactions-list.h"
#include "nact-main-tab.h"
#include "nact-icommand-tab.h"
#include "nact-iconditions-tab.h"
#include "nact-iadvanced-tab.h"

/* private interface data
 */
struct NactICommandTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* the MateConf key used to read/write size and position of auxiliary dialogs
 */
#define IPREFS_LEGEND_DIALOG				"icommand-legend-dialog"
#define IPREFS_COMMAND_CHOOSER				"icommand-command-chooser"
#define IPREFS_FOLDER_URI					"icommand-folder-uri"

/* a data set in the LegendDialog GObject
 */
#define ICOMMAND_TAB_LEGEND_VISIBLE			"nact-icommand-tab-legend-dialog-visible"
#define ICOMMAND_TAB_STATUSBAR_CONTEXT		"nact-icommand-tab-statusbar-context"

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;
static gboolean st_on_selection_change = FALSE;

static GType      register_type( void );
static void       interface_base_init( NactICommandTabInterface *klass );
static void       interface_base_finalize( NactICommandTabInterface *klass );

static void       on_iactions_list_column_edited( NactICommandTab *instance, NAObject *object, gchar *text, gint column );
static void       on_tab_updatable_selection_changed( NactICommandTab *instance, gint count_selected );
static gboolean   tab_set_sensitive( NactICommandTab *instance );

static void       check_for_label( NactICommandTab *instance, GtkEntry *entry, const gchar *label );
static GtkWidget *get_label_entry( NactICommandTab *instance );
static GtkButton *get_legend_button( NactICommandTab *instance );
static GtkWindow *get_legend_dialog( NactICommandTab *instance );
static GtkWidget *get_parameters_entry( NactICommandTab *instance );
static GtkButton *get_path_button( NactICommandTab *instance );
static GtkWidget *get_path_entry( NactICommandTab *instance );
static void       legend_dialog_show( NactICommandTab *instance );
static void       legend_dialog_hide( NactICommandTab *instance );
static void       on_label_changed( GtkEntry *entry, NactICommandTab *instance );
static void       on_legend_clicked( GtkButton *button, NactICommandTab *instance );
static void       on_parameters_changed( GtkEntry *entry, NactICommandTab *instance );
static void       on_path_browse( GtkButton *button, NactICommandTab *instance );
static void       on_path_changed( GtkEntry *entry, NactICommandTab *instance );
static gchar     *parse_parameters( NactICommandTab *instance );
static void       set_label_label( NactICommandTab *instance, const gchar *color );
static void       update_example_label( NactICommandTab *instance, NAObjectProfile *profile );

GType
nact_icommand_tab_get_type( void )
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
	static const gchar *thisfn = "nact_icommand_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NactICommandTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NactICommandTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_WINDOW_TYPE );

	return( type );
}

static void
interface_base_init( NactICommandTabInterface *klass )
{
	static const gchar *thisfn = "nact_icommand_tab_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NactICommandTabInterfacePrivate, 1 );

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NactICommandTabInterface *klass )
{
	static const gchar *thisfn = "nact_icommand_tab_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
}

/**
 * nact_icommand_tab_initial_load:
 * @window: this #NactICommandTab instance.
 *
 * Initializes the tab widget at initial load.
 *
 * The MateConf preference keys used in this tab were misnamed from v1.11.1
 * up to and including v1.12.0. Starting with v1.12.1, these are migrated
 * here, so that the normal code only makes use of 'good' keys.
 */
void
nact_icommand_tab_initial_load_toplevel( NactICommandTab *instance )
{
	static const gchar *thisfn = "nact_icommand_tab_initial_load_toplevel";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_ICOMMAND_TAB( instance ));

	if( st_initialized && !st_finalized ){

		nact_iprefs_migrate_key( BASE_WINDOW( instance ), "iconditions-legend-dialog", IPREFS_LEGEND_DIALOG );
		nact_iprefs_migrate_key( BASE_WINDOW( instance ), "iconditions-command-chooser", IPREFS_COMMAND_CHOOSER );
		nact_iprefs_migrate_key( BASE_WINDOW( instance ), "iconditions-folder-uri", IPREFS_FOLDER_URI );
	}
}

/**
 * nact_icommand_tab_runtime_init:
 * @window: this #NactICommandTab instance.
 *
 * Initializes the tab widget at each time the widget will be displayed.
 * Connect signals and setup runtime values.
 */
void
nact_icommand_tab_runtime_init_toplevel( NactICommandTab *instance )
{
	static const gchar *thisfn = "nact_icommand_tab_runtime_init_toplevel";
	GtkWidget *label_entry, *path_entry, *parameters_entry;
	GtkButton *path_button, *legend_button;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_ICOMMAND_TAB( instance ));

	if( st_initialized && !st_finalized ){

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

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				MAIN_WINDOW_SIGNAL_SELECTION_CHANGED,
				G_CALLBACK( on_tab_updatable_selection_changed ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				IACTIONS_LIST_SIGNAL_COLUMN_EDITED,
				G_CALLBACK( on_iactions_list_column_edited ));
	}
}

void
nact_icommand_tab_all_widgets_showed( NactICommandTab *instance )
{
	static const gchar *thisfn = "nact_icommand_tab_all_widgets_showed";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_ICOMMAND_TAB( instance ));

	if( st_initialized && !st_finalized ){
	}
}

/**
 * nact_icommand_tab_dispose:
 * @window: this #NactICommandTab instance.
 *
 * Called at instance_dispose time.
 */
void
nact_icommand_tab_dispose( NactICommandTab *instance )
{
	static const gchar *thisfn = "nact_icommand_tab_dispose";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_ICOMMAND_TAB( instance ));

	if( st_initialized && !st_finalized ){
		legend_dialog_hide( instance );
	}
}

static void
on_iactions_list_column_edited( NactICommandTab *instance, NAObject *object, gchar *text, gint column )
{
	GtkWidget *label_widget;

	g_return_if_fail( NACT_IS_ICOMMAND_TAB( instance ));

	if( st_initialized && !st_finalized ){

		if( NA_IS_OBJECT_PROFILE( object )){
			label_widget = get_label_entry( instance );
			gtk_entry_set_text( GTK_ENTRY( label_widget ), text );
		}
	}
}

static void
on_tab_updatable_selection_changed( NactICommandTab *instance, gint count_selected )
{
	static const gchar *thisfn = "nact_icommand_tab_on_tab_updatable_selection_changed";
	NAObjectItem *item;
	NAObjectProfile *profile;
	gboolean enable_tab;
	GtkWidget *label_entry, *path_entry, *parameters_entry;
	gchar *label, *path, *parameters;
	gboolean editable;
	GtkButton *path_button;
	GtkButton *legend_button;

	g_debug( "%s: instance=%p, count_selected=%d", thisfn, ( void * ) instance, count_selected );
	g_return_if_fail( NACT_IS_ICOMMAND_TAB( instance ));

	if( st_initialized && !st_finalized ){

		st_on_selection_change = TRUE;

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &item,
				TAB_UPDATABLE_PROP_EDITED_PROFILE, &profile,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		enable_tab = tab_set_sensitive( instance );

		label_entry = get_label_entry( instance );
		label = profile ? na_object_get_label( profile ) : g_strdup( "" );
		label = label ? label : g_strdup( "" );
		gtk_entry_set_text( GTK_ENTRY( label_entry ), label );
		check_for_label( instance, GTK_ENTRY( label_entry ), label );
		g_free( label );
		gtk_widget_set_sensitive( label_entry, profile != NULL );
		nact_gtk_utils_set_editable( GTK_OBJECT( label_entry ), editable );

		path_entry = get_path_entry( instance );
		path = profile ? na_object_get_path( profile ) : g_strdup( "" );
		path = path ? path : g_strdup( "" );
		gtk_entry_set_text( GTK_ENTRY( path_entry ), path );
		g_free( path );
		gtk_widget_set_sensitive( path_entry, profile != NULL );
		nact_gtk_utils_set_editable( GTK_OBJECT( path_entry ), editable );

		path_button = get_path_button( instance );
		gtk_widget_set_sensitive( GTK_WIDGET( path_button ), profile != NULL );
		nact_gtk_utils_set_editable( GTK_OBJECT( path_button ), editable );

		parameters_entry = get_parameters_entry( instance );
		parameters = profile ? na_object_get_parameters( profile ) : g_strdup( "" );
		parameters = parameters ? parameters : g_strdup( "" );
		gtk_entry_set_text( GTK_ENTRY( parameters_entry ), parameters );
		g_free( parameters );
		gtk_widget_set_sensitive( parameters_entry, profile != NULL );
		nact_gtk_utils_set_editable( GTK_OBJECT( parameters_entry ), editable );

		legend_button = get_legend_button( instance );
		gtk_widget_set_sensitive( GTK_WIDGET( legend_button ), profile != NULL );

		st_on_selection_change = FALSE;
	}
}

static gboolean
tab_set_sensitive( NactICommandTab *instance )
{
	NAObjectProfile *profile;
	gboolean enable_tab;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &profile,
			NULL );

	enable_tab = ( profile != NULL );
	nact_main_tab_enable_page( NACT_MAIN_WINDOW( instance ), TAB_COMMAND, enable_tab );

	return( enable_tab );
}

static void
check_for_label( NactICommandTab *instance, GtkEntry *entry, const gchar *label )
{
	NAObjectProfile *edited;

	nact_main_statusbar_hide_status(
			NACT_MAIN_WINDOW( instance ),
			ICOMMAND_TAB_STATUSBAR_CONTEXT );

	set_label_label( instance, "black" );

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
			NULL );

	if( edited && g_utf8_strlen( label, -1 ) == 0 ){

		/* i18n: status bar message when the profile label is empty */
		nact_main_statusbar_display_status(
				NACT_MAIN_WINDOW( instance ),
				ICOMMAND_TAB_STATUSBAR_CONTEXT,
				_( "Caution: a label is mandatory for the profile." ));

		set_label_label( instance, "red" );
	}
}

static GtkWidget *
get_label_entry( NactICommandTab *instance )
{
	return( base_window_get_widget( BASE_WINDOW( instance ), "ProfileLabelEntry" ));
}

static GtkButton *
get_legend_button( NactICommandTab *instance )
{
	return( GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "CommandLegendButton" )));
}

static GtkWindow *
get_legend_dialog( NactICommandTab *instance )
{
	return( base_window_get_named_toplevel( BASE_WINDOW( instance ), "LegendDialog" ));
}

static GtkWidget *
get_parameters_entry( NactICommandTab *instance )
{
	return( base_window_get_widget( BASE_WINDOW( instance ), "CommandParametersEntry" ));
}

static GtkButton *
get_path_button( NactICommandTab *instance )
{
	return( GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "CommandPathButton" )));
}

static GtkWidget *
get_path_entry( NactICommandTab *instance )
{
	return( base_window_get_widget( BASE_WINDOW( instance ), "CommandPathEntry" ));
}

static void
legend_dialog_hide( NactICommandTab *instance )
{
	GtkWindow *legend_dialog;
	GtkButton *legend_button;
	gboolean is_visible;

	legend_dialog = get_legend_dialog( instance );
	is_visible = ( gboolean ) GPOINTER_TO_INT(
			g_object_get_data( G_OBJECT( legend_dialog ), ICOMMAND_TAB_LEGEND_VISIBLE ));

	if( is_visible ){
		g_assert( GTK_IS_WINDOW( legend_dialog ));
		base_iprefs_save_named_window_position( BASE_WINDOW( instance ), legend_dialog, IPREFS_LEGEND_DIALOG );
		gtk_widget_hide( GTK_WIDGET( legend_dialog ));

		/* set the legend button state consistent for when the dialog is
		 * hidden by another mean (eg. close the edit profile dialog)
		 */
		legend_button = get_legend_button( instance );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( legend_button ), FALSE );

		g_object_set_data( G_OBJECT( legend_dialog ), ICOMMAND_TAB_LEGEND_VISIBLE, GINT_TO_POINTER( FALSE ));
	}
}

static void
legend_dialog_show( NactICommandTab *instance )
{
	GtkWindow *legend_dialog;
	GtkWindow *toplevel;

	legend_dialog = get_legend_dialog( instance );
	gtk_window_set_deletable( legend_dialog, FALSE );

	toplevel = base_window_get_toplevel( BASE_WINDOW( instance ));
	gtk_window_set_transient_for( GTK_WINDOW( legend_dialog ), toplevel );

	base_iprefs_position_named_window( BASE_WINDOW( instance ), legend_dialog, IPREFS_LEGEND_DIALOG );
	gtk_widget_show( GTK_WIDGET( legend_dialog ));

	g_object_set_data( G_OBJECT( legend_dialog ), ICOMMAND_TAB_LEGEND_VISIBLE, GINT_TO_POINTER( TRUE ));
}

static void
on_label_changed( GtkEntry *entry, NactICommandTab *instance )
{
	NAObjectProfile *edited;
	const gchar *label;

	if( !st_on_selection_change ){

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
				NULL );

		if( edited ){
			label = gtk_entry_get_text( entry );
			na_object_set_label( edited, label );
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, TRUE );
			check_for_label( instance, entry, label );
		}
	}
}

static void
on_legend_clicked( GtkButton *button, NactICommandTab *instance )
{
	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
		legend_dialog_show( instance );

	} else {
		legend_dialog_hide( instance );
	}
}

static void
on_parameters_changed( GtkEntry *entry, NactICommandTab *instance )
{
	NAObjectProfile *edited;

	if( !st_on_selection_change ){

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
				NULL );

		if( edited ){
			na_object_set_parameters( edited, gtk_entry_get_text( entry ));
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
			update_example_label( instance, edited );
		}
	}
}

static void
on_path_browse( GtkButton *button, NactICommandTab *instance )
{
	gboolean set_current_location = FALSE;
	gchar *uri = NULL;
	NactApplication *application;
	NAUpdater *updater;
	GtkWindow *toplevel;
	GtkWidget *dialog;
	GtkWidget *path_entry;
	const gchar *path;
	gchar *filename;

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( instance )));
	updater = nact_application_get_updater( application );
	toplevel = base_window_get_toplevel( BASE_WINDOW( instance ));

	dialog = gtk_file_chooser_dialog_new(
			_( "Choosing a command" ),
			toplevel,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL
			);

	base_iprefs_position_named_window( BASE_WINDOW( instance ), GTK_WINDOW( dialog ), IPREFS_COMMAND_CHOOSER );

	path_entry = get_path_entry( instance );
	path = gtk_entry_get_text( GTK_ENTRY( path_entry ));

	if( path && strlen( path )){
		set_current_location = gtk_file_chooser_set_filename( GTK_FILE_CHOOSER( dialog ), path );

	} else {
		uri = na_iprefs_read_string( NA_IPREFS( updater ), IPREFS_FOLDER_URI, "file:///bin" );
		gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( dialog ), uri );
		g_free( uri );
	}

	if( gtk_dialog_run( GTK_DIALOG( dialog )) == GTK_RESPONSE_ACCEPT ){
		filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ));
		gtk_entry_set_text( GTK_ENTRY( path_entry ), filename );
	    g_free (filename);
	  }

	uri = gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( dialog ));
	nact_iprefs_write_string( BASE_WINDOW( instance ), IPREFS_FOLDER_URI, uri );
	g_free( uri );

	base_iprefs_save_named_window_position( BASE_WINDOW( instance ), GTK_WINDOW( dialog ), IPREFS_COMMAND_CHOOSER );

	gtk_widget_destroy( dialog );
}

static void
on_path_changed( GtkEntry *entry, NactICommandTab *instance )
{
	NAObjectProfile *edited;

	if( !st_on_selection_change ){

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
				NULL );

		if( edited ){

			na_object_set_path( edited, gtk_entry_get_text( entry ));
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
			update_example_label( instance, edited );
		}
	}
}

/*
 * Valid parameters :
 *
 * %d : base dir of the (first) selected file(s)/folder(s)
 * %f : the name of the (firstà selected file/folder
 * %h : hostname of the (first) URI
 * %m : list of the basename of the selected files/directories separated by space.
 * %M : list of the selected files/directories with their complete path separated by space.
 * %p : port number of the (first) URI
 * %R : space-separated list of selected URIs
 * %s : scheme of the (first) URI
 * %u : (first) URI
 * %U : username of the (first) URI
 * %% : a percent sign
 */
static gchar *
parse_parameters( NactICommandTab *instance )
{
	GString *tmp_string = g_string_new( "" );

	/* i18n notes: example strings for the command preview */
	gchar *ex_path = _( "/path/to" );
	gchar *ex_files[] = { N_( "file1.txt" ), N_( "file2.txt" ), NULL };
	gchar *ex_dirs[] = { N_(" folder1" ), N_( "folder2" ), NULL };
	gchar *ex_mixed[] = { N_(" file1.txt" ), N_( "folder1" ), NULL };
	gchar *ex_scheme_default = "file";
	gchar *ex_host_default = _( "test.example.net" );
	gchar *ex_one_file = _( "file.txt" );
	gchar *ex_one_dir = _( "folder" );
	gchar *ex_port_default = _( "8080" );
	gchar *ex_one = NULL;
	gchar *ex_list = NULL;
	gchar *ex_path_list = NULL;
	gchar *ex_uri_file1 = _( "file:///path/to/file1.text" );
	gchar *ex_uri_file2 = _( "file:///path/to/file2.text" );
	gchar *ex_uri_folder1 = _( "file:///path/to/a/dir" );
	gchar *ex_uri_folder2 = _( "file:///path/to/another/dir" );
	gchar *ex_uri_list = NULL;
	gchar *ex_scheme;
	gchar *ex_host;
	gboolean is_file, is_dir;
	gboolean accept_multiple;
	GSList *scheme_list;
	guint iter_inc;

	const gchar *command = gtk_entry_get_text( GTK_ENTRY( get_path_entry( instance )));
	const gchar *param_template = gtk_entry_get_text( GTK_ENTRY( get_parameters_entry( instance )));

	gchar *iter = g_strdup( param_template );
	gchar *old_iter = iter;
	gchar *tmp;
	gchar *separator;
	gchar *start;

	g_string_append_printf( tmp_string, "%s ", command );

	nact_iconditions_tab_get_isfiledir( NACT_ICONDITIONS_TAB( instance ), &is_file, &is_dir );
	accept_multiple = nact_iconditions_tab_get_multiple( NACT_ICONDITIONS_TAB( instance ));
	scheme_list = nact_iadvanced_tab_get_schemes( NACT_IADVANCED_TAB( instance ));

	separator = g_strdup_printf( " %s/", ex_path );
	start = g_strdup_printf( "%s/", ex_path );

	if( accept_multiple ){
		if( is_file && is_dir ){
			ex_one = ex_files[0];
			ex_list = na_core_utils_gstring_joinv( NULL, " ", ex_mixed );
			ex_path_list = na_core_utils_gstring_joinv( start, separator, ex_mixed );
			ex_uri_list = g_strjoin( " ", ex_uri_file1, ex_uri_folder1, NULL );

		} else if( is_dir ){
			ex_one = ex_dirs[0];
			ex_list = na_core_utils_gstring_joinv( NULL, " ", ex_dirs );
			ex_path_list = na_core_utils_gstring_joinv( start, separator, ex_dirs );
			ex_uri_list = g_strjoin( " ", ex_uri_folder1, ex_uri_folder2, NULL );

		} else if( is_file ){
			ex_one = ex_files[0];
			ex_list = na_core_utils_gstring_joinv( NULL, " ", ex_files );
			ex_path_list = na_core_utils_gstring_joinv( start, separator, ex_files );
			ex_uri_list = g_strjoin( " ", ex_uri_file1, ex_uri_file2, NULL );
		}
	} else {
		if( is_dir && !is_file ){
			ex_one = ex_one_dir;
			ex_uri_list = g_strdup( ex_uri_folder1 );

		} else {
			ex_one = ex_one_file;
			ex_uri_list = g_strdup( ex_uri_file1 );
		}
		ex_list = g_strdup( ex_one );
		ex_path_list = g_strjoin( "/", ex_path, ex_one, NULL );
	}

	g_free (start);
	g_free (separator);

	if( scheme_list != NULL ){
		ex_scheme = ( gchar * ) scheme_list->data;
		if( g_ascii_strcasecmp( ex_scheme, "file" ) == 0 ){
			if( g_slist_length( scheme_list ) > 1 ){
				ex_scheme = ( gchar * ) scheme_list->next->data;
				ex_host = ex_host_default;
			} else {
				ex_host = "";
			}
		} else {
			ex_host = ex_host_default;
		}
	} else {
		ex_scheme = ex_scheme_default;
		ex_host = "";
	}

	while(( iter = g_strstr_len( iter, strlen( iter ), "%" ))){
		tmp_string = g_string_append_len( tmp_string, old_iter, strlen( old_iter ) - strlen( iter ));
		iter_inc = 1;
		switch( iter[1] ){

			case 'd': /* base dir of the (first) selected file(s)/folder(s) */
				tmp_string = g_string_append( tmp_string, ex_path );
				break;

			case 'f': /* the basename of the (first) selected file/folder */
				tmp_string = g_string_append( tmp_string, ex_one );
				break;

			case 'h': /* hostname of the (first) URI */
				tmp_string = g_string_append( tmp_string, ex_host );
				break;

			case 'm': /* list of the basename of the selected files/directories separated by space */
				tmp_string = g_string_append( tmp_string, ex_list );
				break;

			case 'M': /* list of the selected files/directories with their complete path separated by space. */
				tmp_string = g_string_append( tmp_string, ex_path_list );
				break;

			case 'p': /* port number of the (first) URI */
				tmp_string = g_string_append( tmp_string, ex_port_default );
				break;

			case 'R': /* space-separated list of selected URIs */
				tmp_string = g_string_append( tmp_string, ex_uri_list );
				break;

			case 's': /* scheme of the (first) URI */
				tmp_string = g_string_append( tmp_string, ex_scheme );
				break;

			case 'u': /* (first) URI */
				tmp = g_strjoin( NULL, ex_scheme, "://", ex_path, "/", ex_one, NULL );
				tmp_string = g_string_append( tmp_string, tmp );
				g_free( tmp );
				break;

			case 'U': /* username of the GVfs URI */
				tmp_string = g_string_append( tmp_string, "root" );
				break;

			case '%': /* a percent sign */
				tmp_string = g_string_append_c( tmp_string, '%' );
				break;

			default:
				iter_inc = 1;
				break;
		}
		iter += iter_inc;				/* skip the % sign and the character after. */
		old_iter = iter;				/* store the new start of the string */
	}
	tmp_string = g_string_append_len( tmp_string, old_iter, strlen( old_iter ));

	na_core_utils_slist_free( scheme_list );

	g_free( ex_list );
	g_free( ex_path_list );
	g_free( ex_uri_list );
	g_free( iter );

	return( g_string_free( tmp_string, FALSE ));
}

static void
set_label_label( NactICommandTab *instance, const gchar *color_str )
{
	GtkWidget *label;
	GdkColor color;

	label = base_window_get_widget( BASE_WINDOW( instance ), "ProfileLabelLabel" );
	gdk_color_parse( color_str, &color );
	gtk_widget_modify_fg( label, GTK_STATE_NORMAL, &color );
}

static void
update_example_label( NactICommandTab *instance, NAObjectProfile *profile )
{
	/*static const char *thisfn = "nact_iconditions_update_example_label";*/
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
		/* i18n: command-line example: e.g., /bin/ls file1.txt file2.txt */
		newlabel = g_markup_printf_escaped(
				"<i><b><span size=\"small\">%s %s</span></b></i>", _( "e.g.," ), parameters );

		g_free( parameters );

	} else {
		newlabel = g_strdup( "" );
	}

	gtk_label_set_label( GTK_LABEL( example_widget ), newlabel );
	g_free( newlabel );
}
