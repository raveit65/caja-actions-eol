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

#include <api/na-object-api.h>
#include <api/na-core-utils.h>

#include <core/na-factory-object.h>
#include <core/na-iprefs.h>
#include <core/na-ipivot-consumer.h>
#include <core/na-io-provider.h>

#include "nact-application.h"
#include "nact-iactions-list.h"
#include "nact-clipboard.h"
#include "nact-main-statusbar.h"
#include "nact-main-toolbar.h"
#include "nact-main-tab.h"
#include "nact-main-menubar.h"
#include "nact-main-menubar-file.h"
#include "nact-main-menubar-edit.h"
#include "nact-main-menubar-view.h"
#include "nact-main-menubar-tools.h"
#include "nact-main-menubar-maintainer.h"
#include "nact-main-menubar-help.h"

#define MENUBAR_PROP_STATUS_CONTEXT			"nact-menubar-status-context"
#define MENUBAR_PROP_MAIN_STATUS_CONTEXT	"nact-menubar-main-status-context"
#define MENUBAR_PROP_ACTIONS_GROUP			"nact-menubar-actions-group"
#define MENUBAR_IPREFS_FILE_TOOLBAR			"main-file-toolbar"
#define MENUBAR_IPREFS_EDIT_TOOLBAR			"main-edit-toolbar"
#define MENUBAR_IPREFS_TOOLS_TOOLBAR		"main-tools-toolbar"
#define MENUBAR_IPREFS_HELP_TOOLBAR			"main-help-toolbar"

enum {
	MENUBAR_FILE_TOOLBAR_POS = 0,
	MENUBAR_EDIT_TOOLBAR_POS,
	MENUBAR_TOOLS_TOOLBAR_POS,
	MENUBAR_HELP_TOOLBAR_POS
};

/* GtkActivatable
 * gtk_action_get_tooltip are only available starting with Gtk 2.16
 * until this is a required level, we must have some code to do the
 * same thing
 */
#undef GTK_HAS_ACTIVATABLE
#if(( GTK_MAJOR_VERSION > 2 ) || ( GTK_MINOR_VERSION >= 16 ))
	#define GTK_HAS_ACTIVATABLE
#endif

#ifndef GTK_HAS_ACTIVATABLE
#define MENUBAR_PROP_ITEM_ACTION			"nact-menubar-item-action"
#endif

static void     on_iactions_list_count_updated( NactMainWindow *window, gint menus, gint actions, gint profiles );
static void     on_iactions_list_selection_changed( NactMainWindow *window, GList *selected );
static void     on_iactions_list_focus_in( NactMainWindow *window, gpointer user_data );
static void     on_iactions_list_focus_out( NactMainWindow *window, gpointer user_data );
static void     on_iactions_list_status_changed( NactMainWindow *window, gpointer user_data );
static void     on_level_zero_order_changed( NactMainWindow *window, gpointer user_data );
static void     on_update_sensitivities( NactMainWindow *window, gpointer user_data );

static gboolean on_delete_event( GtkWidget *toplevel, GdkEvent *event, NactMainWindow *window );
static void     on_destroy_callback( gpointer data );
static void     on_menu_item_selected( GtkMenuItem *proxy, NactMainWindow *window );
static void     on_menu_item_deselected( GtkMenuItem *proxy, NactMainWindow *window );
static void     on_popup_selection_done(GtkMenuShell *menushell, NactMainWindow *window );
static void     on_proxy_connect( GtkActionGroup *action_group, GtkAction *action, GtkWidget *proxy, NactMainWindow *window );
static void     on_proxy_disconnect( GtkActionGroup *action_group, GtkAction *action, GtkWidget *proxy, NactMainWindow *window );

static const GtkActionEntry entries[] = {

		{ "FileMenu", NULL, N_( "_File" ) },
		{ "EditMenu", NULL, N_( "_Edit" ) },
		{ "ViewMenu", NULL, N_( "_View" ) },
		{ "ViewToolbarMenu", NULL, N_( "_Toolbars" ) },
		{ "ToolsMenu", NULL, N_( "_Tools" ) },
		{ "MaintainerMenu", NULL, N_( "_Maintainer" ) },
		{ "HelpMenu", NULL, N_( "_Help" ) },

		{ "NewMenuItem", NULL, N_( "New _menu" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'New menu' item */
				N_( "Insert a new menu at the current position" ),
				G_CALLBACK( nact_main_menubar_file_on_new_menu ) },
		{ "NewActionItem", GTK_STOCK_NEW, N_( "_New action" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'New action' item */
				N_( "Define a new action" ),
				G_CALLBACK( nact_main_menubar_file_on_new_action ) },
		{ "NewProfileItem", NULL, N_( "New _profile" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'New profile' item */
				N_( "Define a new profile attached to the current action" ),
				G_CALLBACK( nact_main_menubar_file_on_new_profile ) },
		{ "SaveItem", GTK_STOCK_SAVE, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'Save' item */
				N_( "Record all the modified actions. Invalid actions will be silently ignored" ),
				G_CALLBACK( nact_main_menubar_file_on_save ) },
		{ "QuitItem", GTK_STOCK_QUIT, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'Quit' item */
				N_( "Quit the application" ),
				G_CALLBACK( nact_main_menubar_file_on_quit ) },
		{ "CutItem" , GTK_STOCK_CUT, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Cut item */
				N_( "Cut the selected item(s) to the clipboard" ),
				G_CALLBACK( nact_main_menubar_edit_on_cut ) },
		{ "CopyItem" , GTK_STOCK_COPY, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Copy item */
				N_( "Copy the selected item(s) to the clipboard" ),
				G_CALLBACK( nact_main_menubar_edit_on_copy ) },
		{ "PasteItem" , GTK_STOCK_PASTE, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Paste item */
				N_( "Insert the content of the clipboard just before the current position" ),
				G_CALLBACK( nact_main_menubar_edit_on_paste ) },
		{ "PasteIntoItem" , NULL, N_( "Paste _into" ), "<Shift><Ctrl>V",
				/* i18n: tooltip displayed in the status bar when selecting the Paste Into item */
				N_( "Insert the content of the clipboard as first child of the current item" ),
				G_CALLBACK( nact_main_menubar_edit_on_paste_into ) },
		{ "DuplicateItem" , NULL, N_( "D_uplicate" ), "",
				/* i18n: tooltip displayed in the status bar when selecting the Duplicate item */
				N_( "Duplicate the selected item(s)" ),
				G_CALLBACK( nact_main_menubar_edit_on_duplicate ) },
		{ "DeleteItem", GTK_STOCK_DELETE, NULL, "Delete",
				/* i18n: tooltip displayed in the status bar when selecting the Delete item */
				N_( "Delete the selected item(s)" ),
				G_CALLBACK( nact_main_menubar_edit_on_delete ) },
		{ "ReloadActionsItem", GTK_STOCK_REFRESH, N_( "_Reload the items" ), "F5",
				/* i18n: tooltip displayed in the status bar when selecting the 'Reload' item */
				N_( "Cancel your current modifications and reload the initial list of menus and actions" ),
				G_CALLBACK( nact_main_menubar_edit_on_reload ) },
		{ "PreferencesItem", GTK_STOCK_PREFERENCES, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'Preferences' item */
				N_( "Edit your preferences" ),
				G_CALLBACK( nact_main_menubar_edit_on_prefererences ) },
		{ "ExpandAllItem" , NULL, N_( "_Expand all" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Expand all item */
				N_( "Entirely expand the items hierarchy" ),
				G_CALLBACK( nact_main_menubar_view_on_expand_all ) },
		{ "CollapseAllItem" , NULL, N_( "_Collapse all" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Collapse all item */
				N_( "Entirely collapse the items hierarchy" ),
				G_CALLBACK( nact_main_menubar_view_on_collapse_all ) },

		{ "ImportItem" , GTK_STOCK_CONVERT, N_( "_Import assistant..." ), "",
				/* i18n: tooltip displayed in the status bar when selecting the Import item */
				N_( "Import one or more actions from external (XML) files into your configuration" ),
				G_CALLBACK( nact_main_menubar_tools_on_import ) },
		{ "ExportItem", NULL, N_( "E_xport assistant..." ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Export item */
				N_( "Export one or more actions from your configuration to external XML files" ),
				G_CALLBACK( nact_main_menubar_tools_on_export ) },

		{ "DumpSelectionItem", NULL, N_( "_Dump the selection" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Dump selection item */
				N_( "Recursively dump selected items" ),
				G_CALLBACK( nact_main_menubar_maintainer_on_dump_selection ) },
		{ "BriefTreeStoreDumpItem", NULL, N_( "_Brief tree store dump" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the BriefTreeStoreDump item */
				N_( "Briefly dump the tree store" ),
				G_CALLBACK( nact_main_menubar_maintainer_on_brief_tree_store_dump ) },
		{ "ListModifiedItems", NULL, N_( "_List modified items" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the ListModifiedItems item */
				N_( "List the modified items" ),
				G_CALLBACK( nact_main_menubar_maintainer_on_list_modified_items ) },
		{ "DumpClipboard", NULL, N_( "_Dump the clipboard" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the DumpClipboard item */
				N_( "Dump the content of the clipboard object" ),
				G_CALLBACK( nact_main_menubar_maintainer_on_dump_clipboard ) },

		{ "HelpItem" , GTK_STOCK_HELP, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Help item */
				N_( "Display help about this program" ),
				G_CALLBACK( nact_main_menubar_help_on_help ) },
		{ "AboutItem", GTK_STOCK_ABOUT, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the About item */
				N_( "Display informations about this program" ),
				G_CALLBACK( nact_main_menubar_help_on_about ) },
};

static const GtkToggleActionEntry toolbar_entries[] = {

		{ "ViewFileToolbarItem", NULL, N_( "_File" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'View File toolbar' item */
				N_( "Display the File toolbar" ),
				G_CALLBACK( nact_main_menubar_view_on_toolbar_file ), FALSE },
		{ "ViewEditToolbarItem", NULL, N_( "_Edit" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'View Edit toolbar' item */
				N_( "Display the Edit toolbar" ),
				G_CALLBACK( nact_main_menubar_view_on_toolbar_edit ), FALSE },
		{ "ViewToolsToolbarItem", NULL, N_( "_Tools" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'View Tools toolbar' item */
				N_( "Display the Tools toolbar" ),
				G_CALLBACK( nact_main_menubar_view_on_toolbar_tools ), FALSE },
		{ "ViewHelpToolbarItem", NULL, N_( "_Help" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'View Help toolbar' item */
				N_( "Display the Help toolbar" ),
				G_CALLBACK( nact_main_menubar_view_on_toolbar_help ), FALSE },
};

/**
 * nact_main_menubar_runtime_init:
 * @window: the #NactMainWindow to which the menubar is attached.
 *
 * Creates the menubar.
 * Connects to all possible signals which may have an impact on action
 * sensitivities.
 */
void
nact_main_menubar_runtime_init( NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_init";
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GError *error = NULL;
	guint merge_id;
	GtkAccelGroup *accel_group;
	GtkWidget *menubar, *vbox;
	GtkWindow *toplevel;
	MenubarIndicatorsStruct *mis;
	gboolean has_maintainer_menu;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	/* create the menubar:
	 * - create action group, and insert list of actions in it
	 * - create ui_manager, and insert action group in it
	 * - merge inserted actions group with ui xml definition
	 * - install accelerators in the main window
	 * - pack menu bar in the main window
	 */
	ui_manager = gtk_ui_manager_new();
	g_object_set_data_full( G_OBJECT( window ), MENUBAR_PROP_UI_MANAGER, ui_manager, ( GDestroyNotify ) on_destroy_callback );
	g_debug( "%s: ui_manager=%p", thisfn, ( void * ) ui_manager );

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( ui_manager ),
			"connect-proxy",
			G_CALLBACK( on_proxy_connect ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( ui_manager ),
			"disconnect-proxy",
			G_CALLBACK( on_proxy_disconnect ));

	action_group = gtk_action_group_new( "MenubarActions" );
	g_object_set_data_full( G_OBJECT( window ), MENUBAR_PROP_ACTIONS_GROUP, action_group, ( GDestroyNotify ) on_destroy_callback );
	g_debug( "%s: action_group=%p", thisfn, ( void * ) action_group );

	gtk_action_group_set_translation_domain( action_group, GETTEXT_PACKAGE );
	gtk_action_group_add_actions( action_group, entries, G_N_ELEMENTS( entries ), window );
	gtk_action_group_add_toggle_actions( action_group, toolbar_entries, G_N_ELEMENTS( toolbar_entries ), window );
	gtk_ui_manager_insert_action_group( ui_manager, action_group, 0 );

	merge_id = gtk_ui_manager_add_ui_from_file( ui_manager, PKGDATADIR "/caja-actions-config-tool.actions", &error );
	if( merge_id == 0 || error ){
		g_warning( "%s: error=%s", thisfn, error->message );
		g_error_free( error );
	}

	has_maintainer_menu = FALSE;
#ifdef NA_MAINTAINER_MODE
	has_maintainer_menu = TRUE;
#endif

	if( has_maintainer_menu ){
		merge_id = gtk_ui_manager_add_ui_from_file( ui_manager, PKGDATADIR "/caja-actions-maintainer.actions", &error );
		if( merge_id == 0 || error ){
			g_warning( "%s: error=%s", thisfn, error->message );
			g_error_free( error );
		}
	}

	toplevel = base_window_get_toplevel( BASE_WINDOW( window ));
	accel_group = gtk_ui_manager_get_accel_group( ui_manager );
	gtk_window_add_accel_group( toplevel, accel_group );

	menubar = gtk_ui_manager_get_widget( ui_manager, "/ui/MainMenubar" );
	vbox = base_window_get_widget( BASE_WINDOW( window ), "MenubarVBox" );
	gtk_box_pack_start( GTK_BOX( vbox ), menubar, FALSE, FALSE, 0 );

	/* this creates a submenu in the toolbar */
	/*gtk_container_add( GTK_CONTAINER( vbox ), toolbar );*/

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( toplevel ),
			"delete-event",
			G_CALLBACK( on_delete_event ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			IACTIONS_LIST_SIGNAL_LIST_COUNT_UPDATED,
			G_CALLBACK( on_iactions_list_count_updated ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			IACTIONS_LIST_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_iactions_list_selection_changed ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			IACTIONS_LIST_SIGNAL_FOCUS_IN,
			G_CALLBACK( on_iactions_list_focus_in ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			IACTIONS_LIST_SIGNAL_FOCUS_OUT,
			G_CALLBACK( on_iactions_list_focus_out ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			IACTIONS_LIST_SIGNAL_STATUS_CHANGED,
			G_CALLBACK( on_iactions_list_status_changed ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES,
			G_CALLBACK( on_update_sensitivities ));

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			MAIN_WINDOW_SIGNAL_LEVEL_ZERO_ORDER_CHANGED,
			G_CALLBACK( on_level_zero_order_changed ));

	mis = g_new0( MenubarIndicatorsStruct, 1 );
	g_object_set_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS, mis );

	nact_main_toolbar_init( window, action_group );
}

/**
 * nact_main_menubar_dispose:
 * @window: this #NactMainWindow window.
 *
 * Release internally allocated resources.
 */
void
nact_main_menubar_dispose( NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_dispose";
	MenubarIndicatorsStruct *mis;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );
	g_free( mis );
}

/**
 * nact_main_menubar_is_level_zero_order_changed:
 * @window: the #NactMainWindow main window.
 *
 * Returns: %TRUE if the level zero has changed, %FALSE else.
 */
gboolean
nact_main_menubar_is_level_zero_order_changed( const NactMainWindow *window )
{
	MenubarIndicatorsStruct *mis;

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );

	return( mis->level_zero_order_changed );
}

/**
 * nact_main_menubar_open_popup:
 * @window: this #NactMainWindow window.
 * @event: the mouse event.
 *
 * Opens a popup menu.
 */
void
nact_main_menubar_open_popup( NactMainWindow *instance, GdkEventButton *event )
{
	GtkUIManager *ui_manager;
	GtkWidget *menu;
	MenubarIndicatorsStruct *mis;

	ui_manager = ( GtkUIManager * ) g_object_get_data( G_OBJECT( instance ), MENUBAR_PROP_UI_MANAGER );
	menu = gtk_ui_manager_get_widget( ui_manager, "/ui/Popup" );

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( instance ), MENUBAR_PROP_INDICATORS );
	mis->popup_handler = g_signal_connect( menu, "selection-done", G_CALLBACK( on_popup_selection_done ), instance );

	g_signal_emit_by_name( instance, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );

	gtk_menu_popup( GTK_MENU( menu ), NULL, NULL, NULL, NULL, event->button, event->time );
}

/*
 * when the IActionsList is refilled, update our internal counters so
 * that we are knowing if we have some exportables
 */
static void
on_iactions_list_count_updated( NactMainWindow *window, gint menus, gint actions, gint profiles )
{
	MenubarIndicatorsStruct *mis;
	gchar *status;

	g_debug( "nact_main_menubar_on_iactions_list_count_updated: menus=%u, actions=%u, profiles=%u", menus, actions, profiles );
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );
	mis->list_menus = menus;
	mis->list_actions = actions;
	mis->list_profiles = profiles;
	mis->have_exportables = ( mis->list_actions > 0 );

	/* i18n: note the space at the beginning of the sentence */
	status = g_strdup_printf( _( " %d menu(s), %d action(s), %d profile(s) are currently loaded" ), menus, actions, profiles );
	nact_main_statusbar_display_status( window, MENUBAR_PROP_MAIN_STATUS_CONTEXT, status );
	g_free( status );

	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

/*
 * when the selection changes in IActionsList, see what is selected
 */
static void
on_iactions_list_selection_changed( NactMainWindow *window, GList *selected )
{
	MenubarIndicatorsStruct *mis;

	g_debug( "nact_main_menubar_on_iactions_list_selection_changed: selected=%p (count=%d)",
			( void * ) selected, g_list_length( selected ));
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );
	mis->selected_menus = 0;
	mis->selected_actions = 0;
	mis->selected_profiles = 0;
	na_object_item_count_items( selected, &mis->selected_menus, &mis->selected_actions, &mis->selected_profiles, FALSE );
	g_debug( "nact_main_menubar_on_iactions_list_selection_changed: menus=%d, actions=%d, profiles=%d",
			mis->selected_menus, mis->selected_actions, mis->selected_profiles );

	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

static void
on_iactions_list_focus_in( NactMainWindow *window, gpointer user_data )
{
	MenubarIndicatorsStruct *mis;

	g_debug( "nact_main_menubar_on_iactions_list_focus_in" );
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );
	mis->treeview_has_focus = TRUE;
	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

static void
on_iactions_list_focus_out( NactMainWindow *window, gpointer user_data )
{
	MenubarIndicatorsStruct *mis;

	g_debug( "nact_main_menubar_on_iactions_list_focus_out" );
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );
	mis->treeview_has_focus = FALSE;
	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

static void
on_iactions_list_status_changed( NactMainWindow *window, gpointer user_data )
{
	g_debug( "nact_main_menubar_on_iactions_list_status_changed" );
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

static void
on_level_zero_order_changed( NactMainWindow *window, gpointer user_data )
{
	MenubarIndicatorsStruct *mis;

	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	g_debug( "nact_main_menubar_on_level_zero_order_changed: change=%s", user_data ? "True":"False" );

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );
	mis->level_zero_order_changed = GPOINTER_TO_INT( user_data );
	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

static void
on_update_sensitivities( NactMainWindow *window, gpointer user_data )
{
	static const gchar *thisfn = "nact_main_menubar_on_update_sensitivities";
	NactApplication *application;
	MenubarIndicatorsStruct *mis;

	g_debug( "%s: window=%p, user_data=%p", thisfn, ( void * ) window, ( void * ) user_data );
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	mis->updater = nact_application_get_updater( application );
	mis->is_level_zero_writable = na_pivot_is_level_zero_writable( NA_PIVOT( mis->updater ));

	mis->has_writable_providers = nact_window_has_writable_providers( NACT_WINDOW( window ));
	g_debug( "%s: has_writable_providers=%s", thisfn, mis->has_writable_providers ? "True":"False" );

	mis->selected_items = nact_iactions_list_bis_get_selected_items( NACT_IACTIONS_LIST( window ));
	mis->count_selected = mis->selected_items ? g_list_length( mis->selected_items ) : 0;
	g_debug( "%s: count_selected=%d", thisfn, mis->count_selected );

	nact_main_menubar_file_on_update_sensitivities( window, user_data, mis );
	nact_main_menubar_edit_on_update_sensitivities( window, user_data, mis );
	nact_main_menubar_view_on_update_sensitivities( window, user_data, mis );
	nact_main_menubar_tools_on_update_sensitivities( window, user_data, mis );
	nact_main_menubar_maintainer_on_update_sensitivities( window, user_data, mis );
	nact_main_menubar_help_on_update_sensitivities( window, user_data, mis );

	na_object_unref_selected_items( mis->selected_items );
	mis->selected_items = NULL;
}

/**
 * nact_main_menubar_enable_item:
 * @window: the #NactMainWindow main application window.
 * @name: the name of the item in a menu.
 * @enabled: whether this item should be enabled or not.
 *
 * Enable/disable an item in an menu.
 */
void
nact_main_menubar_enable_item( NactMainWindow *window, const gchar *name, gboolean enabled )
{
	GtkActionGroup *group;
	GtkAction *action;

	group = g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_ACTIONS_GROUP );
	action = gtk_action_group_get_action( group, name );
	gtk_action_set_sensitive( action, enabled );
}

static gboolean
on_delete_event( GtkWidget *toplevel, GdkEvent *event, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_on_delete_event";

	g_debug( "%s: toplevel=%p, event=%p, window=%p",
			thisfn, ( void * ) toplevel, ( void * ) event, ( void * ) window );

	nact_main_menubar_file_on_quit( NULL, window );

	return( TRUE );
}

/*
 * this callback is declared when attaching ui_manager and actions_group
 * as data to the window ; it is so triggered on NactMainWindow::finalize()
 */
static void
on_destroy_callback( gpointer data )
{
	static const gchar *thisfn = "nact_main_menubar_on_destroy_callback";

	g_debug( "%s: data=%p (%s)", thisfn,
			( void * ) data, G_OBJECT_TYPE_NAME( data ));

	g_object_unref( G_OBJECT( data ));
}

/*
 * gtk_activatable_get_related_action() and gtk_action_get_tooltip()
 * are only available starting with Gtk 2.16
 */
static void
on_menu_item_selected( GtkMenuItem *proxy, NactMainWindow *window )
{
	GtkAction *action;
	gchar *tooltip;

	/*g_debug( "nact_main_menubar_on_menu_item_selected: proxy=%p (%s), window=%p (%s)",
			( void * ) proxy, G_OBJECT_TYPE_NAME( proxy ),
			( void * ) window, G_OBJECT_TYPE_NAME( window ));*/

	tooltip = NULL;

#ifdef GTK_HAS_ACTIVATABLE
	action = gtk_activatable_get_related_action( GTK_ACTIVATABLE( proxy ));
	if( action ){
		tooltip = ( gchar * ) gtk_action_get_tooltip( action );
	}
#else
	action = GTK_ACTION( g_object_get_data( G_OBJECT( proxy ), MENUBAR_PROP_ITEM_ACTION ));
	if( action ){
		g_object_get( G_OBJECT( action ), "tooltip", &tooltip, NULL );
	}
#endif

	if( tooltip ){
		nact_main_statusbar_display_status( window, MENUBAR_PROP_STATUS_CONTEXT, tooltip );
	}

#ifndef GTK_HAS_ACTIVATABLE
	g_free( tooltip );
#endif
}

static void
on_menu_item_deselected( GtkMenuItem *proxy, NactMainWindow *window )
{
	nact_main_statusbar_hide_status( window, MENUBAR_PROP_STATUS_CONTEXT );
}

static void
on_popup_selection_done(GtkMenuShell *menushell, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_on_popup_selection_done";
	MenubarIndicatorsStruct *mis;

	g_debug( "%s", thisfn );

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );
	g_signal_handler_disconnect( menushell, mis->popup_handler );
	mis->popup_handler = ( gulong ) 0;
}

static void
on_proxy_connect( GtkActionGroup *action_group, GtkAction *action, GtkWidget *proxy, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_on_proxy_connect";

	g_debug( "%s: action_group=%p (%s), action=%p (%s), proxy=%p (%s), window=%p (%s)",
			thisfn,
			( void * ) action_group, G_OBJECT_TYPE_NAME( action_group ),
			( void * ) action, G_OBJECT_TYPE_NAME( action ),
			( void * ) proxy, G_OBJECT_TYPE_NAME( proxy ),
			( void * ) window, G_OBJECT_TYPE_NAME( window ));

	if( GTK_IS_MENU_ITEM( proxy )){

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( proxy ),
				"select",
				G_CALLBACK( on_menu_item_selected ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( proxy ),
				"deselect",
				G_CALLBACK( on_menu_item_deselected ));

#ifndef GTK_HAS_ACTIVATABLE
		g_object_set_data( G_OBJECT( proxy ), MENUBAR_PROP_ITEM_ACTION, action );
#endif
	}
}

static void
on_proxy_disconnect( GtkActionGroup *action_group, GtkAction *action, GtkWidget *proxy, NactMainWindow *window )
{
	/* signal handlers will be automagically disconnected on BaseWindow::dispose */
}
