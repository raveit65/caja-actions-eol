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

#include "cact-main-toolbar.h"
#include "cact-menubar-priv.h"

static void on_view_toolbar_activated( GtkToggleAction *action, BaseWindow *window, int toolbar_id );

/**
 * cact_menubar_view_on_update_sensitivities:
 * @bar: this #CactMenubar object.
 *
 * Update sensitivity of items of the View menu.
 */
void
cact_menubar_view_on_update_sensitivities( const CactMenubar *bar )
{
	guint count_list;

	/* expand all/collapse all requires at least one item in the list */
	count_list = bar->private->count_menus + bar->private->count_actions + bar->private->count_profiles;
	cact_menubar_enable_item( bar, "ExpandAllItem", count_list > 0 );
	cact_menubar_enable_item( bar, "CollapseAllItem", count_list > 0 );
}

/**
 * cact_menubar_view_on_expand_all:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Triggers View / Expand all item.
 */
void
cact_menubar_view_on_expand_all( GtkAction *gtk_action, BaseWindow *window )
{
	CactTreeView *items_view;

	items_view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
	cact_tree_view_expand_all( items_view );
}

/**
 * cact_menubar_view_on_collapse_all:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Triggers View / Collapse all item.
 */
void
cact_menubar_view_on_collapse_all( GtkAction *gtk_action, BaseWindow *window )
{
	CactTreeView *items_view;

	items_view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
	cact_tree_view_collapse_all( items_view );
}

/**
 * cact_menubar_view_on_toolbar_file:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Triggers View / Toolbar / File item.
 */
void
cact_menubar_view_on_toolbar_file( GtkToggleAction *action, BaseWindow *window )
{
	/*on_view_toolbar_activated( action, window, MENUBAR_IPREFS_FILE_TOOLBAR, "/ui/FileToolbar", MENUBAR_FILE_TOOLBAR_POS );*/
	on_view_toolbar_activated( action, window, MAIN_TOOLBAR_FILE_ID );
}

/**
 * cact_menubar_view_on_toolbar_edit:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Triggers View / Toolbar / Edit item.
 */
void
cact_menubar_view_on_toolbar_edit( GtkToggleAction *action, BaseWindow *window )
{
	/*on_view_toolbar_activated( action, window, MENUBAR_IPREFS_EDIT_TOOLBAR, "/ui/EditToolbar", MENUBAR_EDIT_TOOLBAR_POS );*/
	on_view_toolbar_activated( action, window, MAIN_TOOLBAR_EDIT_ID );
}

/**
 * cact_menubar_view_on_toolbar_tools:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Triggers View / Toolbar / Tools item.
 */
void
cact_menubar_view_on_toolbar_tools( GtkToggleAction *action, BaseWindow *window )
{
	/*on_view_toolbar_activated( action, window, MENUBAR_IPREFS_TOOLS_TOOLBAR, "/ui/ToolsToolbar", MENUBAR_TOOLS_TOOLBAR_POS );*/
	on_view_toolbar_activated( action, window, MAIN_TOOLBAR_TOOLS_ID );
}

/**
 * cact_menubar_view_on_toolbar_help:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Triggers View / Toolbar / Help item.
 */
void
cact_menubar_view_on_toolbar_help( GtkToggleAction *action, BaseWindow *window )
{
	/*on_view_toolbar_activated( action, window, MENUBAR_IPREFS_HELP_TOOLBAR, "/ui/HelpToolbar", MENUBAR_HELP_TOOLBAR_POS );*/
	on_view_toolbar_activated( action, window, MAIN_TOOLBAR_HELP_ID );
}

static void
on_view_toolbar_activated( GtkToggleAction *action, BaseWindow *window, int toolbar_id )
{
	gboolean is_active;

	BAR_WINDOW_VOID( window );

	is_active = gtk_toggle_action_get_active( action );

	cact_main_toolbar_activate( CACT_MAIN_WINDOW( window ), toolbar_id, bar->private->ui_manager, is_active );
}

/*
 * When activating one of the GtkRadioAction which handles the position
 * of the notebook tabs
 * @action: the first GtkRadioAction of the group
 * @current: the activated GtkRadioAction
 *
 * This function is triggered once each time we are activating an item of
 * the menu, after having set the "current_value" to the new value. All
 * GtkRadioButtons items share the same "current_value".
 */
void
cact_menubar_view_on_tabs_pos_changed( GtkRadioAction *action, GtkRadioAction *current, BaseWindow *window )
{
	GtkNotebook *notebook;
	guint new_pos;

	notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
	new_pos = gtk_radio_action_get_current_value( action );
	gtk_notebook_set_tab_pos( notebook, new_pos );
}
