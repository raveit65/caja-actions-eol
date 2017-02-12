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

#include <core/na-io-provider.h>
#include <core/na-iprefs.h>

#include "cact-application.h"
#include "cact-main-statusbar.h"
#include "cact-main-toolbar.h"
#include "cact-main-tab.h"
#include "cact-menubar-priv.h"
#include "cact-sort-buttons.h"
#include "cact-tree-view.h"

/* private class data
 */
struct _CactMenubarClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data is externalized in cact-menubar-priv.h
 * in order to be avaible to all cact-menubar-derived files.
 */

static const GtkActionEntry entries[] = {

		{ "FileMenu",          NULL, N_( "_File" ) },
		{ "EditMenu",          NULL, N_( "_Edit" ) },
		{ "ViewMenu",          NULL, N_( "_View" ) },
		{ "ViewToolbarMenu",   NULL, N_( "_Toolbars" ) },
		{ "ToolsMenu",         NULL, N_( "_Tools" ) },
		{ "MaintainerMenu",    NULL, N_( "_Maintainer" ) },
		{ "HelpMenu",          NULL, N_( "_Help" ) },
		{ "NotebookLabelMenu", NULL, N_( "Notebook _tabs" ) },

		{ "NewMenuItem", NULL, N_( "New _menu" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'New menu' item */
				N_( "Insert a new menu at the current position" ),
				G_CALLBACK( cact_menubar_file_on_new_menu ) },
		{ "NewActionItem", GTK_STOCK_NEW, N_( "_New action" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'New action' item */
				N_( "Define a new action" ),
				G_CALLBACK( cact_menubar_file_on_new_action ) },
		{ "NewProfileItem", NULL, N_( "New _profile" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'New profile' item */
				N_( "Define a new profile attached to the current action" ),
				G_CALLBACK( cact_menubar_file_on_new_profile ) },
		{ "SaveItem", GTK_STOCK_SAVE, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'Save' item */
				N_( "Record all the modified actions. Invalid actions will be silently ignored" ),
				G_CALLBACK( cact_menubar_file_on_save ) },
		{ "QuitItem", GTK_STOCK_QUIT, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'Quit' item */
				N_( "Quit the application" ),
				G_CALLBACK( cact_menubar_file_on_quit ) },
		{ "CutItem" , GTK_STOCK_CUT, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Cut item */
				N_( "Cut the selected item(s) to the clipboard" ),
				G_CALLBACK( cact_menubar_edit_on_cut ) },
		{ "CopyItem" , GTK_STOCK_COPY, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Copy item */
				N_( "Copy the selected item(s) to the clipboard" ),
				G_CALLBACK( cact_menubar_edit_on_copy ) },
		{ "PasteItem" , GTK_STOCK_PASTE, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Paste item */
				N_( "Insert the content of the clipboard just before the current position" ),
				G_CALLBACK( cact_menubar_edit_on_paste ) },
		{ "PasteIntoItem" , NULL, N_( "Paste _into" ), "<Shift><Ctrl>V",
				/* i18n: tooltip displayed in the status bar when selecting the Paste Into item */
				N_( "Insert the content of the clipboard as first child of the current item" ),
				G_CALLBACK( cact_menubar_edit_on_paste_into ) },
		{ "DuplicateItem" , NULL, N_( "D_uplicate" ), "",
				/* i18n: tooltip displayed in the status bar when selecting the Duplicate item */
				N_( "Duplicate the selected item(s)" ),
				G_CALLBACK( cact_menubar_edit_on_duplicate ) },
		{ "DeleteItem", GTK_STOCK_DELETE, NULL, "Delete",
				/* i18n: tooltip displayed in the status bar when selecting the Delete item */
				N_( "Delete the selected item(s)" ),
				G_CALLBACK( cact_menubar_edit_on_delete ) },
		{ "ReloadActionsItem", GTK_STOCK_REFRESH, N_( "_Reload the items" ), "F5",
				/* i18n: tooltip displayed in the status bar when selecting the 'Reload' item */
				N_( "Cancel your current modifications and reload the initial list of menus and actions" ),
				G_CALLBACK( cact_menubar_edit_on_reload ) },
		{ "PreferencesItem", GTK_STOCK_PREFERENCES, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'Preferences' item */
				N_( "Edit your preferences" ),
				G_CALLBACK( cact_menubar_edit_on_prefererences ) },
		{ "ExpandAllItem" , NULL, N_( "_Expand all" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Expand all item */
				N_( "Entirely expand the items hierarchy" ),
				G_CALLBACK( cact_menubar_view_on_expand_all ) },
		{ "CollapseAllItem" , NULL, N_( "_Collapse all" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Collapse all item */
				N_( "Entirely collapse the items hierarchy" ),
				G_CALLBACK( cact_menubar_view_on_collapse_all ) },

		{ "ImportItem" , GTK_STOCK_CONVERT, N_( "_Import assistant..." ), "",
				/* i18n: tooltip displayed in the status bar when selecting the Import item */
				N_( "Import one or more actions from external files into your configuration" ),
				G_CALLBACK( cact_menubar_tools_on_import ) },
		{ "ExportItem", NULL, N_( "E_xport assistant..." ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Export item */
				N_( "Export one or more actions from your configuration to external files" ),
				G_CALLBACK( cact_menubar_tools_on_export ) },

		{ "DumpSelectionItem", NULL, N_( "_Dump the selection" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the Dump selection item */
				N_( "Recursively dump selected items" ),
				G_CALLBACK( cact_menubar_maintainer_on_dump_selection ) },
		{ "BriefTreeStoreDumpItem", NULL, N_( "_Brief tree store dump" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the BriefTreeStoreDump item */
				N_( "Briefly dump the tree store" ),
				G_CALLBACK( cact_menubar_maintainer_on_brief_tree_store_dump ) },
		{ "ListModifiedItems", NULL, N_( "_List modified items" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the ListModifiedItems item */
				N_( "List the modified items" ),
				G_CALLBACK( cact_menubar_maintainer_on_list_modified_items ) },
		{ "DumpClipboard", NULL, N_( "_Dump the clipboard" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the DumpClipboard item */
				N_( "Dump the content of the clipboard object" ),
				G_CALLBACK( cact_menubar_maintainer_on_dump_clipboard ) },
		{ "FunctionTest", NULL, "_Test a function", NULL,
				"Test a function (see cact-menubar-maintainer.c",
				G_CALLBACK( cact_menubar_maintainer_on_test_function ) },

		{ "HelpItem" , GTK_STOCK_HELP, N_( "Contents" ), "F1",
				/* i18n: tooltip displayed in the status bar when selecting the Help item */
				N_( "Display help about this program" ),
				G_CALLBACK( cact_menubar_help_on_help ) },
		{ "AboutItem", GTK_STOCK_ABOUT, NULL, NULL,
				/* i18n: tooltip displayed in the status bar when selecting the About item */
				N_( "Display information about this program" ),
				G_CALLBACK( cact_menubar_help_on_about ) },
};

static const GtkToggleActionEntry toolbar_entries[] = {

		{ "ViewFileToolbarItem", NULL, N_( "_File" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'View File toolbar' item */
				N_( "Display the File toolbar" ),
				G_CALLBACK( cact_menubar_view_on_toolbar_file ), FALSE },
		{ "ViewEditToolbarItem", NULL, N_( "_Edit" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'View Edit toolbar' item */
				N_( "Display the Edit toolbar" ),
				G_CALLBACK( cact_menubar_view_on_toolbar_edit ), FALSE },
		{ "ViewToolsToolbarItem", NULL, N_( "_Tools" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'View Tools toolbar' item */
				N_( "Display the Tools toolbar" ),
				G_CALLBACK( cact_menubar_view_on_toolbar_tools ), FALSE },
		{ "ViewHelpToolbarItem", NULL, N_( "_Help" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'View Help toolbar' item */
				N_( "Display the Help toolbar" ),
				G_CALLBACK( cact_menubar_view_on_toolbar_help ), FALSE },
};

static const GtkRadioActionEntry tabs_pos_entries[] = {

		{ "TabsPosLeftItem", NULL, N_( "On the _left" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'Set tabs position on the left' item */
				N_( "Display the notebook tabs on the left side" ),
				GTK_POS_LEFT },
		{ "TabsPosRightItem", NULL, N_( "On the _right" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting the 'Set tabs position on the right' item */
				N_( "Display the notebook tabs on the right side" ),
				GTK_POS_RIGHT },
		{ "TabsPosTopItem", NULL, N_( "On the _top" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'Set tabs position on the top' item */
				N_( "Display the notebook tabs on the top side" ),
				GTK_POS_TOP },
		{ "TabsPosBottomItem", NULL, N_( "On the _bottom" ), NULL,
				/* i18n: tooltip displayed in the status bar when selecting 'Set tabs position on the bottom' item */
				N_( "Display the notebook tabs on the bottom side" ),
				GTK_POS_BOTTOM },
};

#define MENUBAR_PROP_STATUS_CONTEXT			"menubar-status-context"
#define MENUBAR_PROP_MAIN_STATUS_CONTEXT	"menubar-main-status-context"

/* signals
 */
enum {
	UPDATE_SENSITIVITIES,
	LAST_SIGNAL
};

static const gchar  *st_ui_menubar_actions     = PKGUIDIR "/caja-actions-config-tool.actions";
static const gchar  *st_ui_maintainer_actions  = PKGUIDIR "/caja-actions-maintainer.actions";

static gint          st_signals[ LAST_SIGNAL ] = { 0 };
static GObjectClass *st_parent_class           = NULL;

static GType    register_type( void );
static void     class_init( CactMenubarClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *application );
static void     instance_finalize( GObject *application );

static void     on_base_initialize_window( BaseWindow *window, gpointer user_data );
static void     on_ui_manager_proxy_connect( GtkUIManager *ui_manager, GtkAction *action, GtkWidget *proxy, BaseWindow *window );
static void     on_menu_item_selected( GtkMenuItem *proxy, BaseWindow *window );
static void     on_menu_item_deselected( GtkMenuItem *proxy, BaseWindow *window );

static void     on_open_context_menu( BaseWindow *window, GdkEventButton *event, const gchar *popup, gpointer user_data );
static void     on_popup_selection_done( GtkMenuShell *menushell, BaseWindow *window );
static void     on_tree_view_count_changed( BaseWindow *window, gboolean reset, gint menus, gint actions, gint profiles );
static void     on_tree_view_focus_in( BaseWindow *window, gpointer user_data );
static void     on_tree_view_focus_out( BaseWindow *window, gpointer user_data );
static void     on_tree_view_modified_status_changed( BaseWindow *window, gboolean is_modified, gpointer user_data );
static void     on_tree_view_selection_changed( BaseWindow *window, GList *selected, gpointer user_data );

static void     on_update_sensitivities( CactMenubar *bar, BaseWindow *window );

GType
cact_menubar_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_menubar_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactMenubarClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactMenubar ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "CactMenubar", &info, 0 );

	return( type );
}

static void
class_init( CactMenubarClass *klass )
{
	static const gchar *thisfn = "cact_menubar_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	/**
	 * CactMenubar::menubar-signal-update-sensitivities
	 *
	 * This signal is emitted by the CactMenubar object on itself when
	 * menu items sensitivities have to be refreshed.
	 *
	 * Signal arg.: None
	 *
	 * Handler prototype:
	 * void ( *handler )( CactMenubar *bar, gpointer user_data );
	 */
	st_signals[ UPDATE_SENSITIVITIES ] = g_signal_new(
			MENUBAR_SIGNAL_UPDATE_SENSITIVITIES,
			CACT_TYPE_MENUBAR,
			G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			0,					/* no default handler */
			NULL,
			NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0 );

	klass->private = g_new0( CactMenubarClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_menubar_instance_init";
	CactMenubar *self;

	g_return_if_fail( CACT_IS_MENUBAR( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_MENUBAR( instance );

	self->private = g_new0( CactMenubarPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cact_menubar_instance_dispose";
	CactMenubarPrivate *priv;

	g_return_if_fail( CACT_IS_MENUBAR( object ));

	priv = CACT_MENUBAR( object )->private;

	if( !priv->dispose_has_run ){

		g_debug( "%s: object=%p (%s)",
				thisfn,
				( void * ) object, G_OBJECT_TYPE_NAME( object ));

		priv->dispose_has_run = TRUE;

		base_window_signal_disconnect(
				priv->window,
				priv->update_sensitivities_handler_id );

		g_object_unref( priv->action_group );
		g_object_unref( priv->notebook_group );
		g_object_unref( priv->ui_manager );
		g_object_unref( priv->sort_buttons );

		if( priv->selected_items ){
			g_list_free( priv->selected_items );
		}

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *instance )
{
	static const gchar *thisfn = "cact_menubar_instance_finalize";
	CactMenubar *self;

	g_return_if_fail( CACT_IS_MENUBAR( instance ));

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	self = CACT_MENUBAR( instance );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( instance );
	}
}

/**
 * cact_menubar_new:
 * @window: the window which embeds the menubar, usually the #CactMainWindow.
 *
 * The created menubar attachs itself to the @window; it also connect a weak
 * reference to this same @window, thus automatically g_object_unref() -ing
 * itself at @window finalization time.
 *
 * The menubar also takes advantage of #BaseWindow messages to initialize
 * its Gtk widgets.
 *
 * Returns: a new #CactMenubar object.
 */
CactMenubar *
cact_menubar_new( BaseWindow *window )
{
	CactMenubar *bar;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	bar = g_object_new( CACT_TYPE_MENUBAR, NULL );

	bar->private->window = window;
	bar->private->sort_buttons = cact_sort_buttons_new( window );

	base_window_signal_connect(
			window,
			G_OBJECT( window ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_base_initialize_window ));

	g_object_set_data( G_OBJECT( window ), WINDOW_DATA_MENUBAR, bar );

	return( bar );
}

static void
on_base_initialize_window( BaseWindow *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_menubar_on_base_initialize_window";
	GError *error;
	guint merge_id;
	GtkAccelGroup *accel_group;
	GtkWidget *menubar, *vbox;
	GtkWindow *toplevel;
	gboolean has_maintainer_menu;
	CactApplication *application;
	guint tabs_pos;

	BAR_WINDOW_VOID( window );

	if( !bar->private->dispose_has_run ){

		g_debug( "%s: window=%p (%s), user_data=%p",
				thisfn,
				( void * ) window, G_OBJECT_TYPE_NAME( window ),
				( void * ) user_data );

		/* create the menubar:
		 * - create action group, and insert list of actions in it
		 * - create the ui manager, and insert action group in it
		 * - merge inserted actions group with ui xml definition
		 * - install accelerators in the main window
		 * - pack menu bar in the main window
		 *
		 * "disconnect-proxy" signal is never triggered.
		 */
		bar->private->ui_manager = gtk_ui_manager_new();
		g_debug( "%s: ui_manager=%p", thisfn, ( void * ) bar->private->ui_manager );

		base_window_signal_connect(
				window,
				G_OBJECT( bar->private->ui_manager ),
				"connect-proxy",
				G_CALLBACK( on_ui_manager_proxy_connect ));

		tabs_pos = na_iprefs_get_tabs_pos( NULL );
		bar->private->notebook_group = gtk_action_group_new( "NotebookActions" );
		g_debug( "%s: notebook_group=%p", thisfn, ( void * ) bar->private->notebook_group );
		gtk_action_group_set_translation_domain( bar->private-> notebook_group, GETTEXT_PACKAGE );
		gtk_action_group_add_radio_actions( bar->private->notebook_group, tabs_pos_entries, G_N_ELEMENTS( tabs_pos_entries ), tabs_pos, G_CALLBACK( cact_menubar_view_on_tabs_pos_changed ), window );
		gtk_ui_manager_insert_action_group( bar->private->ui_manager, bar->private->notebook_group, 0 );

		bar->private->action_group = gtk_action_group_new( "MenubarActions" );
		g_debug( "%s: action_group=%p", thisfn, ( void * ) bar->private->action_group );
		gtk_action_group_set_translation_domain( bar->private->action_group, GETTEXT_PACKAGE );
		gtk_action_group_add_actions( bar->private->action_group, entries, G_N_ELEMENTS( entries ), window );
		gtk_action_group_add_toggle_actions( bar->private->action_group, toolbar_entries, G_N_ELEMENTS( toolbar_entries ), window );
		gtk_ui_manager_insert_action_group( bar->private->ui_manager, bar->private->action_group, 0 );

		error = NULL;
		merge_id = gtk_ui_manager_add_ui_from_file( bar->private->ui_manager, st_ui_menubar_actions, &error );
		if( merge_id == 0 || error ){
			g_warning( "%s: error=%s", thisfn, error->message );
			g_error_free( error );
		}

		has_maintainer_menu = FALSE;
#ifdef NA_MAINTAINER_MODE
		has_maintainer_menu = TRUE;
#endif
		if( has_maintainer_menu ){
			error = NULL;
			merge_id = gtk_ui_manager_add_ui_from_file( bar->private->ui_manager, st_ui_maintainer_actions, &error );
			if( merge_id == 0 || error ){
				g_warning( "%s: error=%s", thisfn, error->message );
				g_error_free( error );
			}
		}

		toplevel = base_window_get_gtk_toplevel( window );
		accel_group = gtk_ui_manager_get_accel_group( bar->private->ui_manager );
		gtk_window_add_accel_group( toplevel, accel_group );

		menubar = gtk_ui_manager_get_widget( bar->private->ui_manager, "/ui/MainMenubar" );
		vbox = base_window_get_widget( window, "MenubarVBox" );
		gtk_box_pack_start( GTK_BOX( vbox ), menubar, FALSE, FALSE, 0 );

		/* this creates a submenu in the toolbar */
		/*gtk_container_add( GTK_CONTAINER( vbox ), toolbar );*/

		/* initialize the private data
		 */
		application = CACT_APPLICATION( base_window_get_application( bar->private->window ));
		bar->private->updater = cact_application_get_updater( application );
		bar->private->is_level_zero_writable = na_updater_is_level_zero_writable( bar->private->updater );
		bar->private->has_writable_providers =
				( na_io_provider_find_writable_io_provider( NA_PIVOT( bar->private->updater )) != NULL );

		g_debug( "%s: na_updater_is_level_zero_writable=%s, na_io_provider_find_writable_io_provider=%s",
				thisfn,
				bar->private->is_level_zero_writable ? "True":"False",
				bar->private->has_writable_providers ? "True":"False" );

		/* connect to all signal which may have an influence on the menu
		 * items sensitivity
		 */
		base_window_signal_connect(
				window,
				G_OBJECT( window ),
				MAIN_SIGNAL_CONTEXT_MENU,
				G_CALLBACK( on_open_context_menu ));

		base_window_signal_connect(
				window,
				G_OBJECT( window ),
				TREE_SIGNAL_COUNT_CHANGED,
				G_CALLBACK( on_tree_view_count_changed ));

		base_window_signal_connect(
				window,
				G_OBJECT( window ),
				TREE_SIGNAL_FOCUS_IN,
				G_CALLBACK( on_tree_view_focus_in ));

		base_window_signal_connect(
				window,
				G_OBJECT( window ),
				TREE_SIGNAL_FOCUS_OUT,
				G_CALLBACK( on_tree_view_focus_out ));

		base_window_signal_connect(
				window,
				G_OBJECT( window ),
				TREE_SIGNAL_MODIFIED_STATUS_CHANGED,
				G_CALLBACK( on_tree_view_modified_status_changed ));

		base_window_signal_connect(
				window,
				G_OBJECT( window ),
				MAIN_SIGNAL_SELECTION_CHANGED,
				G_CALLBACK( on_tree_view_selection_changed ));

		bar->private->update_sensitivities_handler_id =
				base_window_signal_connect(
						window,
						G_OBJECT( bar ),
						MENUBAR_SIGNAL_UPDATE_SENSITIVITIES,
						G_CALLBACK( on_update_sensitivities ));

		cact_menubar_file_initialize( bar );
		cact_main_toolbar_init( window, bar->private->action_group );
	}
}

/*
 * action: GtkAction, GtkToggleAction
 * proxy:  GtkImageMenuItem, GtkCheckMenuItem, GtkToolButton
 */
static void
on_ui_manager_proxy_connect( GtkUIManager *ui_manager, GtkAction *action, GtkWidget *proxy, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_on_ui_manager_proxy_connect";

	g_debug( "%s: ui_manager=%p (%s), action=%p (%s), proxy=%p (%s), window=%p (%s)",
			thisfn,
			( void * ) ui_manager, G_OBJECT_TYPE_NAME( ui_manager ),
			( void * ) action, G_OBJECT_TYPE_NAME( action ),
			( void * ) proxy, G_OBJECT_TYPE_NAME( proxy ),
			( void * ) window, G_OBJECT_TYPE_NAME( window ));

	if( GTK_IS_MENU_ITEM( proxy )){

		base_window_signal_connect(
				window,
				G_OBJECT( proxy ),
				"select",
				G_CALLBACK( on_menu_item_selected ));

		base_window_signal_connect(
				window,
				G_OBJECT( proxy ),
				"deselect",
				G_CALLBACK( on_menu_item_deselected ));
	}
}

/*
 * gtk_activatable_get_related_action() and gtk_action_get_tooltip()
 * are only available starting with Gtk 2.16
 */
static void
on_menu_item_selected( GtkMenuItem *proxy, BaseWindow *window )
{
	GtkAction *action;
	const gchar *tooltip;

	/*g_debug( "cact_menubar_on_menu_item_selected: proxy=%p (%s), window=%p (%s)",
			( void * ) proxy, G_OBJECT_TYPE_NAME( proxy ),
			( void * ) window, G_OBJECT_TYPE_NAME( window ));*/

	tooltip = NULL;
	action = gtk_activatable_get_related_action( GTK_ACTIVATABLE( proxy ));

	if( action ){
		tooltip = gtk_action_get_tooltip( action );

		if( tooltip ){
			cact_main_statusbar_display_status( CACT_MAIN_WINDOW( window ), MENUBAR_PROP_STATUS_CONTEXT, tooltip );
		}
	}
}

static void
on_menu_item_deselected( GtkMenuItem *proxy, BaseWindow *window )
{
	cact_main_statusbar_hide_status( CACT_MAIN_WINDOW( window ), MENUBAR_PROP_STATUS_CONTEXT );
}

/*
 * Opens a popup menu.
 */
static void
on_open_context_menu( BaseWindow *window, GdkEventButton *event, const gchar *popup, gpointer user_data )
{
	static const gchar *thisfn = "cact_menubar_on_open_context_menu";
	GtkWidget *menu;

	BAR_WINDOW_VOID( window );

	menu = gtk_ui_manager_get_widget( bar->private->ui_manager, popup );
	if( menu ){
		bar->private->popup_handler =
				g_signal_connect(
						menu,
						"selection-done",
						G_CALLBACK( on_popup_selection_done ),
						window );
		if( event ){
			gtk_menu_popup( GTK_MENU( menu ), NULL, NULL, NULL, NULL, event->button, event->time );
		} else {
			gtk_menu_popup( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time ());
		}
	} else {
		g_warning( "%s: menu not found: %s", thisfn, popup );
	}
}

static void
on_popup_selection_done(GtkMenuShell *menushell, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_on_popup_selection_done";

	BAR_WINDOW_VOID( window );

	g_debug( "%s", thisfn );

	g_signal_handler_disconnect( menushell, bar->private->popup_handler );
	bar->private->popup_handler = ( gulong ) 0;
}

/*
 * when the tree view is refilled, update our internal counters so
 * that we are knowing if we have some exportables
 */
static void
on_tree_view_count_changed( BaseWindow *window, gboolean reset, gint menus, gint actions, gint profiles )
{
	static const gchar *thisfn = "cact_menubar_on_tree_view_count_changed";
	gchar *status;

	BAR_WINDOW_VOID( window );

	g_debug( "%s: window=%p, reset=%s, menus=%d, actions=%d, profiles=%d",
			thisfn, ( void * ) window, reset ? "True":"False", menus, actions, profiles );

	if( reset ){
		bar->private->count_menus = menus;
		bar->private->count_actions = actions;
		bar->private->count_profiles = profiles;

	} else {
		bar->private->count_menus += menus;
		bar->private->count_actions += actions;
		bar->private->count_profiles += profiles;
	}

	bar->private->have_exportables = ( bar->private->count_menus + bar->private->count_actions > 0 );

	/* i18n: note the space at the beginning of the sentence */
	status = g_strdup_printf(
			_( " %d menu(s), %d action(s), %d profile(s) are currently loaded" ),
			bar->private->count_menus, bar->private->count_actions, bar->private->count_profiles );
	cact_main_statusbar_display_status( CACT_MAIN_WINDOW( window ), MENUBAR_PROP_MAIN_STATUS_CONTEXT, status );
	g_free( status );

	g_signal_emit_by_name( bar, MENUBAR_SIGNAL_UPDATE_SENSITIVITIES );
}

static void
on_tree_view_focus_in( BaseWindow *window, gpointer user_data )
{
	BAR_WINDOW_VOID( window );

	g_debug( "cact_menubar_on_tree_view_focus_in" );

	bar->private->treeview_has_focus = TRUE;
	g_signal_emit_by_name( bar, MENUBAR_SIGNAL_UPDATE_SENSITIVITIES );
}

static void
on_tree_view_focus_out( BaseWindow *window, gpointer user_data )
{
	BAR_WINDOW_VOID( window );

	g_debug( "cact_menubar_on_tree_view_focus_out" );

	bar->private->treeview_has_focus = FALSE;
	g_signal_emit_by_name( bar, MENUBAR_SIGNAL_UPDATE_SENSITIVITIES );
}

/*
 * the count of modified NAObjectItem has changed
 */
static void
on_tree_view_modified_status_changed( BaseWindow *window, gboolean is_modified, gpointer user_data )
{
	static const gchar *thisfn = "cact_menubar_on_tree_view_modified_status_changed";

	g_debug( "%s: window=%p, is_modified=%s, user_data=%p",
			thisfn, ( void * ) window, is_modified ? "True":"False", ( void * ) user_data );

	BAR_WINDOW_VOID( window );

	if( !bar->private->dispose_has_run ){

		bar->private->is_tree_modified = is_modified;
		g_signal_emit_by_name( bar, MENUBAR_SIGNAL_UPDATE_SENSITIVITIES );
	}
}

/*
 * when the selection changes in the tree view, see what is selected
 *
 * It happens that this function is triggered after all tabs have already
 * dealt with the MAIN_SIGNAL_SELECTION_CHANGED signal
 *
 * We are trying to precompute here all indicators which are needed to
 * make actions sensitive. As a multiple selection may have multiple
 * sort of indicators, we assure here that at least one item will be a
 * valid candidate to the target action, the action taking care itself
 * of applying to valid candidates, and rejecting the others.
 */
static void
on_tree_view_selection_changed( BaseWindow *window, GList *selected, gpointer user_data )
{
	static const gchar *thisfn = "cact_menubar_on_tree_view_selection_changed";
	NAObject *first;
	NAObject *selected_action;
	NAObject *row, *parent;
	GList *is;

	BAR_WINDOW_VOID( window );

	g_debug( "%s: selected_items=%p (count=%d)", thisfn, ( void * ) selected, g_list_length( selected ));

	/* count the items
	 */
	bar->private->count_selected = g_list_length( selected );

	if( selected ){
		na_object_item_count_items( selected, &bar->private->selected_menus, &bar->private->selected_actions, &bar->private->selected_profiles, FALSE );
		g_debug( "%s: selected_menus=%d, selected_actions=%d, selected_profiles=%d",
				thisfn,
				bar->private->selected_menus, bar->private->selected_actions, bar->private->selected_profiles );
	}

	/* take a ref of the list of selected items
	 */
	if( bar->private->selected_items ){
		g_list_free( bar->private->selected_items );
	}
	bar->private->selected_items = g_list_copy( selected );

	/* check if the parent of the first selected item is writable
	 * (File: New menu/New action)
	 * (Edit: Paste menu or action)
	 */
	first = NULL;
	if( selected ){
		first = ( NAObject *) selected->data;
		if( NA_IS_OBJECT_PROFILE( first )){
			first = NA_OBJECT( na_object_get_parent( first ));
		}
		first = ( NAObject * ) na_object_get_parent( first );
	}
	if( first ){
		bar->private->is_parent_writable = na_object_is_finally_writable( first, NULL );
		g_debug( "%s: parent of first selected is not null: is_parent_writable=%s",
				thisfn, bar->private->is_parent_writable ? "True":"False" );
	} else {
		bar->private->is_parent_writable = bar->private->is_level_zero_writable;
		g_debug( "%s: first selected is at level zero: is_level_zero_writable=%s",
				thisfn, bar->private->is_level_zero_writable ? "True":"False" );
	}

	/* check is only an action is selected, or only profile(s) of a same action
	 * (File: New profile)
	 * (Edit: Paste a profile)
	 */
	bar->private->enable_new_profile = TRUE;
	selected_action = NULL;
	for( is = selected ; is ; is = is->next ){

		if( NA_IS_OBJECT_MENU( is->data )){
			bar->private->enable_new_profile = FALSE;
			break;

		} else if( NA_IS_OBJECT_ACTION( is->data )){
			if( !selected_action ){
				selected_action = NA_OBJECT( is->data );
			} else {
				bar->private->enable_new_profile = FALSE;
				break;
			}

		} else if( NA_IS_OBJECT_PROFILE( is->data )){
			first = NA_OBJECT( na_object_get_parent( is->data ));
			if( !selected_action ){
				selected_action = first;
			} else if( selected_action != first ){
				bar->private->enable_new_profile = FALSE;
				break;
			}
		}
	}
	if( selected_action ){
		bar->private->is_action_writable = na_object_is_finally_writable( selected_action, NULL );
	} else {
		bar->private->enable_new_profile = FALSE;
	}

	/* check that selection is not empty and that each selected item is writable
	 * and that all parents are writable
	 * if some selection is at level zero, then it must be writable
	 * (Edit: Cut/Delete)
	 */
	if( selected ){
		bar->private->are_parents_writable = TRUE;
		bar->private->are_items_writable = TRUE;
		for( is = selected ; is ; is = is->next ){
			row = ( NAObject * ) is->data;
			if( NA_IS_OBJECT_PROFILE( row )){
				row = NA_OBJECT( na_object_get_parent( row ));
			}
			gchar *label = na_object_get_label( row );
			gboolean writable = na_object_is_finally_writable( row, NULL );
			g_debug( "%s: label=%s, writable=%s", thisfn, label, writable ? "True":"False" );
			g_free( label );
			bar->private->are_items_writable &= writable;
			parent = ( NAObject * ) na_object_get_parent( row );
			if( parent ){
				bar->private->are_parents_writable &= na_object_is_finally_writable( parent, NULL );
			} else {
				bar->private->are_parents_writable &= bar->private->is_level_zero_writable;
			}
		}
	}

	g_signal_emit_by_name( bar, MENUBAR_SIGNAL_UPDATE_SENSITIVITIES );
}

static void
on_update_sensitivities( CactMenubar *bar, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_on_update_sensitivities";

	g_debug( "%s: bar=%p, window=%p", thisfn, ( void * ) bar, ( void * ) window );

	cact_menubar_file_on_update_sensitivities( bar );
	cact_menubar_edit_on_update_sensitivities( bar );
	cact_menubar_view_on_update_sensitivities( bar );
	cact_menubar_tools_on_update_sensitivities( bar );
	cact_menubar_maintainer_on_update_sensitivities( bar );
	cact_menubar_help_on_update_sensitivities( bar );
}

/**
 * cact_menubar_enable_item:
 * @bar: this #CactMenubar instance.
 * @name: the name of the item in a menu.
 * @enabled: whether this item should be enabled or not.
 *
 * Enable/disable an item in an menu.
 */
void
cact_menubar_enable_item( const CactMenubar *bar, const gchar *name, gboolean enabled )
{
	GtkAction *action;

	if( !bar->private->dispose_has_run ){

		action = gtk_action_group_get_action( bar->private->action_group, name );
		gtk_action_set_sensitive( action, enabled );
	}
}

/**
 * cact_menubar_save_items:
 * @window: the #BaseWindow
 */
void
cact_menubar_save_items( BaseWindow *window )
{
	cact_menubar_file_save_items( window );
}
