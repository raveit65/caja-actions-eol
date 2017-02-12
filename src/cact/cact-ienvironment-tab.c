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
#include <libintl.h>
#include <stdlib.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-desktop-environment.h>
#include <core/na-settings.h>

#include "base-gtk-utils.h"
#include "cact-main-tab.h"
#include "cact-ienvironment-tab.h"

/* private interface data
 */
struct _CactIEnvironmentTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* columns in the selection count combobox
 */
enum {
	COUNT_SIGN_COLUMN = 0,
	COUNT_LABEL_COLUMN,
	COUNT_N_COLUMN
};

typedef struct {
	gchar *sign;
	gchar *label;
}
	SelectionCountStruct;

/* i18n notes: selection count symbol, respectively 'less than', 'equal to' and 'greater than' */
static SelectionCountStruct st_counts[] = {
		{ "<", N_( "(strictly lesser than)" ) },
		{ "=", N_( "(equal to)" ) },
		{ ">", N_( "(strictly greater than)" ) },
		{ NULL }
};

/* column ordering in the OnlyShowIn/NotShowIn listview
 */
enum {
	ENV_BOOL_COLUMN = 0,
	ENV_LABEL_COLUMN,
	ENV_KEYWORD_COLUMN,
	N_COLUMN
};

/* Pseudo-property, set against the instance
 */
typedef struct {
	gboolean on_selection_change;
}
	IEnvironData;

#define IENVIRON_TAB_PROP_DATA				"cact-ienviron-tab-data"

static guint st_initializations = 0;		/* interface initialization count */

static GType         register_type( void );
static void          interface_base_init( CactIEnvironmentTabInterface *klass );
static void          interface_base_finalize( CactIEnvironmentTabInterface *klass );

static void          on_base_initialize_gtk( CactIEnvironmentTab *instance, GtkWindow *toplevel, gpointer user_data );
static void          on_base_initialize_window( CactIEnvironmentTab *instance, gpointer user_data );

static void          on_main_selection_changed( CactIEnvironmentTab *instance, GList *selected_items, gpointer user_data );

static void          on_selcount_ope_changed( GtkComboBox *combo, CactIEnvironmentTab *instance );
static void          on_selcount_int_changed( GtkEntry *entry, CactIEnvironmentTab *instance );
static void          on_selection_count_changed( CactIEnvironmentTab *instance );
static void          on_show_always_toggled( GtkToggleButton *togglebutton, CactIEnvironmentTab *instance );
static void          on_only_show_toggled( GtkToggleButton *togglebutton, CactIEnvironmentTab *instance );
static void          on_do_not_show_toggled( GtkToggleButton *togglebutton, CactIEnvironmentTab *instance );
static void          on_desktop_toggled( GtkCellRendererToggle *renderer, gchar *path, BaseWindow *window );
static void          on_try_exec_changed( GtkEntry *entry, CactIEnvironmentTab *instance );
static void          on_try_exec_browse( GtkButton *button, CactIEnvironmentTab *instance );
static void          on_show_if_registered_changed( GtkEntry *entry, CactIEnvironmentTab *instance );
static void          on_show_if_true_changed( GtkEntry *entry, CactIEnvironmentTab *instance );
static void          on_show_if_running_changed( GtkEntry *entry, CactIEnvironmentTab *instance );
static void          on_show_if_running_browse( GtkButton *button, CactIEnvironmentTab *instance );

static void          init_selection_count_combobox( CactIEnvironmentTab *instance );
static gchar        *get_selection_count_selection( CactIEnvironmentTab *instance );
static void          set_selection_count_selection( CactIEnvironmentTab *instance, const gchar *ope, const gchar *uint );
static void          dispose_selection_count_combobox( CactIEnvironmentTab *instance );

static void          init_desktop_listview( CactIEnvironmentTab *instance );
static void          raz_desktop_listview( CactIEnvironmentTab *instance );
static void          setup_desktop_listview( CactIEnvironmentTab *instance, GSList *show );
static void          dispose_desktop_listview( CactIEnvironmentTab *instance );

static IEnvironData *get_ienviron_data( CactIEnvironmentTab *instance );
static void          on_instance_finalized( gpointer user_data, CactIEnvironmentTab *instance );

GType
cact_ienvironment_tab_get_type( void )
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
	static const gchar *thisfn = "cact_ienvironment_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactIEnvironmentTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "CactIEnvironmentTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_TYPE_WINDOW );

	return( type );
}

static void
interface_base_init( CactIEnvironmentTabInterface *klass )
{
	static const gchar *thisfn = "cact_ienvironment_tab_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactIEnvironmentTabInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( CactIEnvironmentTabInterface *klass )
{
	static const gchar *thisfn = "cact_ienvironment_tab_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/**
 * cact_ienvironment_tab_init:
 * @instance: this #CactIEnvironmentTab instance.
 *
 * Initialize the interface
 * Connect to #BaseWindow signals
 */
void
cact_ienvironment_tab_init( CactIEnvironmentTab *instance )
{
	static const gchar *thisfn = "cact_ienvironment_tab_init";
	IEnvironData *data;

	g_return_if_fail( CACT_IS_IENVIRONMENT_TAB( instance ));

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

	cact_main_tab_init( CACT_MAIN_WINDOW( instance ), TAB_ENVIRONMENT );

	data = get_ienviron_data( instance );
	data->on_selection_change = FALSE;

	g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
}

/*
 * on_base_initialize_gtk:
 * @window: this #CactIEnvironmentTab instance.
 *
 * Initializes the tab widget at initial load.
 */
static void
on_base_initialize_gtk( CactIEnvironmentTab *instance, GtkWindow *toplevel, void *user_data )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_base_initialize_gtk";

	g_return_if_fail( CACT_IS_IENVIRONMENT_TAB( instance ));

	g_debug( "%s: instance=%p (%s), toplevel=%p, user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) toplevel,
			( void * ) user_data );

	init_selection_count_combobox( instance );
	init_desktop_listview( instance );
}

/*
 * on_base_initialize_window:
 * @window: this #CactIEnvironmentTab instance.
 *
 * Initializes the tab widget at each time the widget will be displayed.
 * Connect signals and setup runtime values.
 */
static void
on_base_initialize_window( CactIEnvironmentTab *instance, void *user_data )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_base_initialize_window";
	GtkTreeView *listview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeViewColumn *column;
	GList *renderers;
	guint i;
	const NADesktopEnv *desktops;

	g_return_if_fail( CACT_IS_IENVIRONMENT_TAB( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"SelectionCountSigneCombobox",
			"changed",
			G_CALLBACK( on_selcount_ope_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"SelectionCountNumberEntry",
			"changed",
			G_CALLBACK( on_selcount_int_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ShowAlwaysButton",
			"toggled",
			G_CALLBACK( on_show_always_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"OnlyShowButton",
			"toggled",
			G_CALLBACK( on_only_show_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"DoNotShowButton",
			"toggled",
			G_CALLBACK( on_do_not_show_toggled ));

	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), "EnvironmentsDesktopTreeView" ));
	model = gtk_tree_view_get_model( listview );

	desktops = na_desktop_environment_get_known_list();

	for( i = 0 ; desktops[i].id ; ++i ){
		gtk_list_store_append( GTK_LIST_STORE( model ), &iter );
		gtk_list_store_set(
				GTK_LIST_STORE( model ),
				&iter,
				ENV_BOOL_COLUMN, FALSE,
				ENV_LABEL_COLUMN, gettext( desktops[i].label ),
				ENV_KEYWORD_COLUMN, desktops[i].id,
				-1 );
	}

	column = gtk_tree_view_get_column( listview, ENV_BOOL_COLUMN );
	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( renderers->data ),
			"toggled",
			G_CALLBACK( on_desktop_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"TryExecEntry",
			"changed",
			G_CALLBACK( on_try_exec_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"TryExecButton",
			"clicked",
			G_CALLBACK( on_try_exec_browse ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ShowIfRegisteredEntry",
			"changed",
			G_CALLBACK( on_show_if_registered_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ShowIfTrueEntry",
			"changed",
			G_CALLBACK( on_show_if_true_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ShowIfRunningEntry",
			"changed",
			G_CALLBACK( on_show_if_running_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ShowIfRunningButton",
			"clicked",
			G_CALLBACK( on_show_if_running_browse ));
}

static void
on_main_selection_changed( CactIEnvironmentTab *instance, GList *selected_items, gpointer user_data )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_main_selection_changed";
	NAIContext *context;
	gboolean editable;
	gboolean enable_tab;
	gchar *sel_count, *selcount_ope, *selcount_int;
	GtkWidget *combo, *entry;
	GtkTreeView *listview;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GtkWidget *always_button, *show_button, *notshow_button;
	GtkWidget *browse_button;
	GSList *desktops;
	gchar *text;
	IEnvironData *data;

	g_return_if_fail( CACT_IS_IENVIRONMENT_TAB( instance ));

	g_debug( "%s: instance=%p (%s), selected_items=%p (count=%d)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) selected_items, g_list_length( selected_items ));

	g_object_get( G_OBJECT( instance ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
			NULL );

	enable_tab = ( context != NULL );
	cact_main_tab_enable_page( CACT_MAIN_WINDOW( instance ), TAB_ENVIRONMENT, enable_tab );

	data = get_ienviron_data( instance );
	data->on_selection_change = TRUE;

	/* selection count
	 */
	sel_count = context ? na_object_get_selection_count( context ) : g_strdup( "" );
	na_core_utils_selcount_get_ope_int( sel_count, &selcount_ope, &selcount_int );
	set_selection_count_selection( instance, selcount_ope, selcount_int );
	g_free( selcount_int );
	g_free( selcount_ope );
	g_free( sel_count );

	combo = base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountSigneCombobox" );
	base_gtk_utils_set_editable( G_OBJECT( combo ), editable );

	entry = base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountNumberEntry" );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );

	/* desktop environment
	 */
	raz_desktop_listview( instance );

	always_button = base_window_get_widget( BASE_WINDOW( instance ), "ShowAlwaysButton" );
	show_button = base_window_get_widget( BASE_WINDOW( instance ), "OnlyShowButton" );
	notshow_button = base_window_get_widget( BASE_WINDOW( instance ), "DoNotShowButton" );

	desktops = context ? na_object_get_only_show_in( context ) : NULL;
	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), "EnvironmentsDesktopTreeView" ));
	gtk_toggle_button_set_inconsistent( GTK_TOGGLE_BUTTON( always_button ), context == NULL );

	if( desktops && g_slist_length( desktops )){
		base_gtk_utils_radio_set_initial_state(
				GTK_RADIO_BUTTON( show_button ),
				G_CALLBACK( on_only_show_toggled ), instance, editable, ( context != NULL ));
		gtk_widget_set_sensitive( GTK_WIDGET( listview ), TRUE );

	} else {
		desktops = context ? na_object_get_not_show_in( context ) : NULL;

		if( desktops && g_slist_length( desktops )){
			base_gtk_utils_radio_set_initial_state(
					GTK_RADIO_BUTTON( notshow_button ),
					G_CALLBACK( on_do_not_show_toggled ), instance, editable, ( context != NULL ));
			gtk_widget_set_sensitive( GTK_WIDGET( listview ), TRUE );

		} else {
			base_gtk_utils_radio_set_initial_state(
					GTK_RADIO_BUTTON( always_button ),
					G_CALLBACK( on_show_always_toggled ), instance, editable, ( context != NULL ));
			gtk_widget_set_sensitive( GTK_WIDGET( listview ), FALSE );
			desktops = NULL;
		}
	}

	setup_desktop_listview( instance, desktops );

	/* execution environment
	 */
	entry = base_window_get_widget( BASE_WINDOW( instance ), "TryExecEntry" );
	text = context ? na_object_get_try_exec( context ) : g_strdup( "" );
	text = text && strlen( text ) ? text : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( entry ), text );
	g_free( text );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );

	browse_button = base_window_get_widget( BASE_WINDOW( instance ), "TryExecButton" );
	base_gtk_utils_set_editable( G_OBJECT( browse_button ), editable );

	entry = base_window_get_widget( BASE_WINDOW( instance ), "ShowIfRegisteredEntry" );
	text = context ? na_object_get_show_if_registered( context ) : g_strdup( "" );
	text = text && strlen( text ) ? text : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( entry ), text );
	g_free( text );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );

	entry = base_window_get_widget( BASE_WINDOW( instance ), "ShowIfTrueEntry" );
	text = context ? na_object_get_show_if_true( context ) : g_strdup( "" );
	text = text && strlen( text ) ? text : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( entry ), text );
	g_free( text );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );

	entry = base_window_get_widget( BASE_WINDOW( instance ), "ShowIfRunningEntry" );
	text = context ? na_object_get_show_if_running( context ) : g_strdup( "" );
	text = text && strlen( text ) ? text : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( entry ), text );
	g_free( text );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );

	browse_button = base_window_get_widget( BASE_WINDOW( instance ), "ShowIfRunningButton" );
	base_gtk_utils_set_editable( G_OBJECT( browse_button ), editable );

	data->on_selection_change = FALSE;

	path = gtk_tree_path_new_first();
	if( path ){
		selection = gtk_tree_view_get_selection( listview );
		gtk_tree_selection_select_path( selection, path );
		gtk_tree_path_free( path );
	}
}

static void
on_selcount_ope_changed( GtkComboBox *combo, CactIEnvironmentTab *instance )
{
	on_selection_count_changed( instance );
}

static void
on_selcount_int_changed( GtkEntry *entry, CactIEnvironmentTab *instance )
{
	on_selection_count_changed( instance );
}

static void
on_selection_count_changed( CactIEnvironmentTab *instance )
{
	NAIContext *context;
	gchar *selcount;
	IEnvironData *data;

	data = get_ienviron_data( instance );

	if( !data->on_selection_change ){
		g_object_get( G_OBJECT( instance ), MAIN_PROP_CONTEXT, &context, NULL );

		if( context ){
			selcount = get_selection_count_selection( instance );
			na_object_set_selection_count( context, selcount );
			g_free( selcount );

			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
		}
	}
}

/*
 * the behavior coded here as one main drawback:
 * - user will usually try to go through the radio buttons (always show,
 *   only show in, not show in) just to see what are their effects
 * - but each time we toggle one of these buttons, the list of desktop is raz :(
 *
 * this behavior is inherent because we have to save each modification in the
 * context as soon as this modification is made in the UI, so that user do not
 * have to save each modification before going to another tab/context/item
 *
 * as far as I know, this case is the only which has this drawback...
 */
static void
on_show_always_toggled( GtkToggleButton *toggle_button, CactIEnvironmentTab *instance )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_show_always_toggled";
	NAIContext *context;
	gboolean editable;
	gboolean active;
	GtkTreeView *listview;

	g_return_if_fail( CACT_IS_IENVIRONMENT_TAB( instance ));

	g_debug( "%s: toggle_button=%p (active=%s), instance=%p",
			thisfn,
			( void * ) toggle_button, gtk_toggle_button_get_active( toggle_button ) ? "True":"False",
			( void * ) instance );

	g_object_get( G_OBJECT( instance ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
			NULL );

	if( context ){
		active = gtk_toggle_button_get_active( toggle_button );

		if( editable ){
			listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), "EnvironmentsDesktopTreeView" ));
			gtk_widget_set_sensitive( GTK_WIDGET( listview ), !active );

			if( active ){
				raz_desktop_listview( instance );
				na_object_set_only_show_in( context, NULL );
				na_object_set_not_show_in( context, NULL );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
			}

		} else {
			base_gtk_utils_radio_reset_initial_state(
					GTK_RADIO_BUTTON( toggle_button ), G_CALLBACK( on_show_always_toggled ));
		}
	}
}

static void
on_only_show_toggled( GtkToggleButton *toggle_button, CactIEnvironmentTab *instance )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_only_show_toggled";
	NAIContext *context;
	gboolean editable;
	gboolean active;
	GSList *show;

	g_debug( "%s: toggle_button=%p (active=%s), instance=%p",
			thisfn,
			( void * ) toggle_button, gtk_toggle_button_get_active( toggle_button ) ? "True":"False",
			( void * ) instance );

	g_object_get( G_OBJECT( instance ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
			NULL );

	if( context ){
		active = gtk_toggle_button_get_active( toggle_button );

		if( editable ){
			if( active ){
				raz_desktop_listview( instance );
				show = na_object_get_only_show_in( context );
				if( show && g_slist_length( show )){
					setup_desktop_listview( instance, show );
				}

			} else {
				na_object_set_only_show_in( context, NULL );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
			}

		} else {
			base_gtk_utils_radio_reset_initial_state(
					GTK_RADIO_BUTTON( toggle_button ), G_CALLBACK( on_only_show_toggled ));
		}
	}
}

static void
on_do_not_show_toggled( GtkToggleButton *toggle_button, CactIEnvironmentTab *instance )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_do_not_show_toggled";
	NAIContext *context;
	gboolean editable;
	gboolean active;
	GSList *show;

	g_debug( "%s: toggle_button=%p (active=%s), instance=%p",
			thisfn,
			( void * ) toggle_button, gtk_toggle_button_get_active( toggle_button ) ? "True":"False",
			( void * ) instance );

	g_object_get( G_OBJECT( instance ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
			NULL );

	if( context ){
		active = gtk_toggle_button_get_active( toggle_button );

		if( editable ){
			if( active ){
				raz_desktop_listview( instance );
				show = na_object_get_not_show_in( context );
				if( show && g_slist_length( show )){
					setup_desktop_listview( instance, show );
				}

			} else {
				na_object_set_not_show_in( context, NULL );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
			}

		} else {
			base_gtk_utils_radio_reset_initial_state(
					GTK_RADIO_BUTTON( toggle_button ), G_CALLBACK( on_do_not_show_toggled ));
		}
	}
}

static void
on_desktop_toggled( GtkCellRendererToggle *renderer, gchar *path, BaseWindow *window )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_desktop_toggled";
	NAIContext *context;
	gboolean editable;
	GtkTreeView *listview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *tree_path;
	gboolean state;
	gchar *desktop;
	GtkWidget *show_button;
	IEnvironData *data;

	g_debug( "%s: renderer=%p, path=%s, window=%p", thisfn, ( void * ) renderer, path, ( void * ) window );

	data = get_ienviron_data( CACT_IENVIRONMENT_TAB( window ));

	if( !data->on_selection_change ){
		g_object_get( G_OBJECT( window ),
				MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
				NULL );

		if( context ){
			if( editable ){
				listview = GTK_TREE_VIEW( base_window_get_widget( window, "EnvironmentsDesktopTreeView" ));
				model = gtk_tree_view_get_model( listview );
				tree_path = gtk_tree_path_new_from_string( path );
				gtk_tree_model_get_iter( model, &iter, tree_path );
				gtk_tree_path_free( tree_path );
				gtk_tree_model_get( model, &iter, ENV_BOOL_COLUMN, &state, ENV_KEYWORD_COLUMN, &desktop, -1 );
				gtk_list_store_set( GTK_LIST_STORE( model ), &iter, ENV_BOOL_COLUMN, !state, -1 );

				show_button = base_window_get_widget( BASE_WINDOW( window ), "OnlyShowButton" );
				if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( show_button ))){
					na_object_set_only_desktop( context, desktop, !state );
				} else {
					na_object_set_not_desktop( context, desktop, !state );
				}

				g_signal_emit_by_name( G_OBJECT( window ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );

				g_free( desktop );

			} else {
				g_signal_handlers_block_by_func(( gpointer ) renderer, on_desktop_toggled, window );
				gtk_cell_renderer_toggle_set_active( renderer, state );
				g_signal_handlers_unblock_by_func(( gpointer ) renderer, on_desktop_toggled, window );
			}
		}
	}
}

static void
on_try_exec_changed( GtkEntry *entry, CactIEnvironmentTab *instance )
{
	NAIContext *context;
	const gchar *text;

	g_object_get( G_OBJECT( instance ), MAIN_PROP_CONTEXT, &context, NULL );

	if( context ){
		text = gtk_entry_get_text( entry );
		na_object_set_try_exec( context, text );
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
	}
}

static void
on_try_exec_browse( GtkButton *button, CactIEnvironmentTab *instance )
{
	GtkWidget *entry;

	entry = base_window_get_widget( BASE_WINDOW( instance ), "TryExecEntry" );

	base_gtk_utils_select_file(
			BASE_WINDOW( instance ),
			_( "Choosing an executable" ), NA_IPREFS_TRY_EXEC_WSP,
			entry, NA_IPREFS_TRY_EXEC_URI );
}

static void
on_show_if_registered_changed( GtkEntry *entry, CactIEnvironmentTab *instance )
{
	NAIContext *context;
	const gchar *text;

	g_object_get( G_OBJECT( instance ), MAIN_PROP_CONTEXT, &context, NULL );

	if( context ){
		text = gtk_entry_get_text( entry );
		na_object_set_show_if_registered( context, text );
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
	}
}

static void
on_show_if_true_changed( GtkEntry *entry, CactIEnvironmentTab *instance )
{
	NAIContext *context;
	const gchar *text;

	g_object_get( G_OBJECT( instance ), MAIN_PROP_CONTEXT, &context, NULL );

	if( context ){
		text = gtk_entry_get_text( entry );
		na_object_set_show_if_true( context, text );
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
	}
}

static void
on_show_if_running_changed( GtkEntry *entry, CactIEnvironmentTab *instance )
{
	NAIContext *context;
	const gchar *text;

	g_object_get( G_OBJECT( instance ), MAIN_PROP_CONTEXT, &context, NULL );

	if( context ){
		text = gtk_entry_get_text( entry );
		na_object_set_show_if_running( context, text );
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
	}
}

static void
on_show_if_running_browse( GtkButton *button, CactIEnvironmentTab *instance )
{
	GtkWidget *entry;

	entry = base_window_get_widget( BASE_WINDOW( instance ), "ShowIfRunningEntry" );

	base_gtk_utils_select_file(
			BASE_WINDOW( instance ),
			_( "Choosing an executable" ), NA_IPREFS_SHOW_IF_RUNNING_WSP,
			entry, NA_IPREFS_SHOW_IF_RUNNING_URI );
}

static void
init_selection_count_combobox( CactIEnvironmentTab *instance )
{
	GtkTreeModel *model;
	guint i;
	GtkTreeIter row;
	GtkComboBox *combo;
	GtkCellRenderer *cell_renderer_text;

	model = GTK_TREE_MODEL( gtk_list_store_new( COUNT_N_COLUMN, G_TYPE_STRING, G_TYPE_STRING ));
	i = 0;
	while( st_counts[i].sign ){
		gtk_list_store_append( GTK_LIST_STORE( model ), &row );
		gtk_list_store_set( GTK_LIST_STORE( model ), &row, COUNT_SIGN_COLUMN, st_counts[i].sign, -1 );
		gtk_list_store_set( GTK_LIST_STORE( model ), &row, COUNT_LABEL_COLUMN, gettext( st_counts[i].label ), -1 );
		i += 1;
	}

	combo = GTK_COMBO_BOX( base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountSigneCombobox" ));
	gtk_combo_box_set_model( combo, model );
	g_object_unref( model );

	gtk_cell_layout_clear( GTK_CELL_LAYOUT( combo ));

	cell_renderer_text = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), cell_renderer_text, FALSE );
	gtk_cell_layout_add_attribute( GTK_CELL_LAYOUT( combo ), cell_renderer_text, "text", COUNT_SIGN_COLUMN );

	cell_renderer_text = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), cell_renderer_text, TRUE );
	g_object_set( G_OBJECT( cell_renderer_text ), "xalign", ( gdouble ) 0.0, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL );
	gtk_cell_layout_add_attribute( GTK_CELL_LAYOUT( combo ), cell_renderer_text, "text", COUNT_LABEL_COLUMN );

	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ), 0 );
}

static gchar *
get_selection_count_selection( CactIEnvironmentTab *instance )
{
	GtkComboBox *combo;
	GtkEntry *entry;
	gint index;
	gchar *uints, *selcount;
	guint uinti;

	combo = GTK_COMBO_BOX( base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountSigneCombobox" ));
	index = gtk_combo_box_get_active( combo );
	if( index == -1 ){
		return( NULL );
	}

	entry = GTK_ENTRY( base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountNumberEntry" ));
	uinti = abs( atoi( gtk_entry_get_text( entry )));
	uints = g_strdup_printf( "%d", uinti );
	gtk_entry_set_text( entry, uints );
	g_free( uints );

	selcount = g_strdup_printf( "%s%d", st_counts[index].sign, uinti );

	return( selcount );
}

static void
set_selection_count_selection( CactIEnvironmentTab *instance, const gchar *ope, const gchar *uint )
{
	GtkComboBox *combo;
	GtkEntry *entry;
	gint i, index;

	combo = GTK_COMBO_BOX( base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountSigneCombobox" ));

	index = -1;
	for( i=0 ; st_counts[i].sign && index==-1 ; ++i ){
		if( !strcmp( st_counts[i].sign, ope )){
			index = i;
		}
	}
	gtk_combo_box_set_active( combo, index );

	entry = GTK_ENTRY( base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountNumberEntry" ));
	gtk_entry_set_text( entry, uint );
}

static void
dispose_selection_count_combobox( CactIEnvironmentTab *instance )
{
	GtkWidget *combo;
	GtkTreeModel *model;

	combo = base_window_get_widget( BASE_WINDOW( instance ), "SelectionCountSigneCombobox" );
	if( GTK_IS_COMBO_BOX( combo )){
		model = gtk_combo_box_get_model( GTK_COMBO_BOX( combo ));
		gtk_list_store_clear( GTK_LIST_STORE( model ));
	}
}

static void
init_desktop_listview( CactIEnvironmentTab *instance )
{
	GtkTreeView *listview;
	GtkListStore *model;
	GtkCellRenderer *check_cell, *text_cell;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), "EnvironmentsDesktopTreeView" ));
	model = gtk_list_store_new( N_COLUMN, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING );
	gtk_tree_view_set_model( listview, GTK_TREE_MODEL( model ));
	g_object_unref( model );

	check_cell = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes(
			"boolean",
			check_cell,
			"active", ENV_BOOL_COLUMN,
			NULL );
	gtk_tree_view_append_column( listview, column );

	text_cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
			"label",
			text_cell,
			"text", ENV_LABEL_COLUMN,
			NULL );
	gtk_tree_view_append_column( listview, column );

	gtk_tree_view_set_headers_visible( listview, FALSE );

	selection = gtk_tree_view_get_selection( listview );
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
}

static void
raz_desktop_listview( CactIEnvironmentTab *instance )
{
	GtkTreeView *listview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean next_ok;

	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), "EnvironmentsDesktopTreeView" ));
	model = gtk_tree_view_get_model( listview );

	if( gtk_tree_model_get_iter_first( model, &iter )){
		next_ok = TRUE;
		while( next_ok ){
			gtk_list_store_set( GTK_LIST_STORE( model ), &iter, ENV_BOOL_COLUMN, FALSE, -1 );
			next_ok = gtk_tree_model_iter_next( model, &iter );
		}
	}
}

static void
setup_desktop_listview( CactIEnvironmentTab *instance, GSList *show )
{
	static const gchar *thisfn = "cact_ienvironment_tab_setup_desktop_listview";
	GtkTreeView *listview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean next_ok, found;
	GSList *ic;
	gchar *keyword;

	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), "EnvironmentsDesktopTreeView" ));
	model = gtk_tree_view_get_model( listview );

	for( ic = show ; ic ; ic = ic->next ){
		if( strlen( ic->data )){
			found = FALSE;
			if( gtk_tree_model_get_iter_first( model, &iter )){
				next_ok = TRUE;
				while( next_ok && !found ){
					gtk_tree_model_get( model, &iter, ENV_KEYWORD_COLUMN, &keyword, -1 );
					if( !strcmp( keyword, ic->data )){
						gtk_list_store_set( GTK_LIST_STORE( model ), &iter, ENV_BOOL_COLUMN, TRUE, -1 );
						found = TRUE;
					}
					g_free( keyword );
					if( !found ){
						next_ok = gtk_tree_model_iter_next( model, &iter );
					}
				}
			}
			if( !found ){
				g_warning( "%s: unable to set %s environment", thisfn, ( const gchar * ) ic->data );
			}
		}
	}
}

static void
dispose_desktop_listview( CactIEnvironmentTab *instance )
{
	GtkWidget *listview;
	GtkTreeModel *model;
	GtkTreeSelection *selection;

	listview = base_window_get_widget( BASE_WINDOW( instance ), "EnvironmentsDesktopTreeView" );
	if( GTK_IS_TREE_VIEW( listview )){
		model = gtk_tree_view_get_model( GTK_TREE_VIEW( listview ));
		selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( listview ));
		gtk_tree_selection_unselect_all( selection );
		gtk_list_store_clear( GTK_LIST_STORE( model ));
	}
}

static IEnvironData *
get_ienviron_data( CactIEnvironmentTab *instance )
{
	IEnvironData *data;

	data = ( IEnvironData * ) g_object_get_data( G_OBJECT( instance ), IENVIRON_TAB_PROP_DATA );

	if( !data ){
		data = g_new0( IEnvironData, 1 );
		g_object_set_data( G_OBJECT( instance ), IENVIRON_TAB_PROP_DATA, data );
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, CactIEnvironmentTab *instance )
{
	static const gchar *thisfn = "cact_ienvironment_tab_on_instance_finalized";
	IEnvironData *data;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );

	data = get_ienviron_data( instance );
	data->on_selection_change = TRUE;
	dispose_selection_count_combobox( instance );
	dispose_desktop_listview( instance );

	g_free( data );
}
