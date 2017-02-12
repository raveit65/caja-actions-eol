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

#ifndef __CACT_MENUBAR_PRIV_H__
#define __CACT_MENUBAR_PRIV_H__

/*
 * SECTION: cact-menubar-priv
 * @title: CactMenubarPrivate
 * @short_description: The Menubar private data definition
 * @include: cact-menubar-priv.h
 *
 * This file should only be included by cact-menubar -derived files.
 */

#include <core/na-updater.h>

#include "cact-menubar.h"
#include "cact-sort-buttons.h"

G_BEGIN_DECLS

struct _CactMenubarPrivate {
	/*< private >*/
	gboolean         dispose_has_run;

	/* set at instanciation time
	 */
	BaseWindow      *window;
	gulong			 update_sensitivities_handler_id;

	/* set at initialization time
	 */
	NAUpdater       *updater;
	CactSortButtons *sort_buttons;
	GtkUIManager    *ui_manager;
	GtkActionGroup  *action_group;
	GtkActionGroup  *notebook_group;
	gboolean         is_level_zero_writable;
	gboolean         has_writable_providers;

	/* set when the selection changes
	 */
	guint            count_selected;
	GList           *selected_items;
	gboolean         is_parent_writable;		/* new menu/new action/paste menu or action */
	gboolean         enable_new_profile;		/* new profile/paste a profile */
	gboolean         is_action_writable;
	gboolean         are_parents_writable;		/* duplicate */
	gboolean         are_items_writable;		/* cut/delete */

	/* set when the count of modified or deleted NAObjectItem changes
	 * or when the lever zero is changed
	 */
	gboolean         is_tree_modified;

	/* set on focus in/out
	 */
	gboolean         treeview_has_focus;

	/* opening a contextual popup menu
	 */
	gulong           popup_handler;

	/* set when total count of items changes
	 */
	gint             count_menus;
	gint             count_actions;
	gint             count_profiles;
	gboolean         have_exportables;

	/* *** */
	gint            selected_menus;
	gint            selected_actions;
	gint            selected_profiles;
	gint            clipboard_menus;
	gint            clipboard_actions;
	gint            clipboard_profiles;
	/* *** */
};

/* Signal emitted by the CactMenubar object on itself
 */
#define MENUBAR_SIGNAL_UPDATE_SENSITIVITIES		"menubar-signal-update-sensitivities"

/* Convenience macros to get a CactMenubar from a BaseWindow
 */
#define WINDOW_DATA_MENUBAR						"window-data-menubar"

#define BAR_WINDOW_VOID( window ) \
		g_return_if_fail( BASE_IS_WINDOW( window )); \
		CactMenubar *bar = ( CactMenubar * ) g_object_get_data( G_OBJECT( window ), WINDOW_DATA_MENUBAR ); \
		g_return_if_fail( CACT_IS_MENUBAR( bar ));

#define BAR_WINDOW_VALUE( window, value ) \
		g_return_val_if_fail( BASE_IS_WINDOW( window ), value ); \
		CactMenubar *bar = ( CactMenubar * ) g_object_get_data( G_OBJECT( window ), WINDOW_DATA_MENUBAR ); \
		g_return_val_if_fail( CACT_IS_MENUBAR( bar ), value );

/* These functions should only be called from a cact-menubar-derived file
 */
void cact_menubar_enable_item                        ( const CactMenubar *bar, const gchar *name, gboolean enabled );

void cact_menubar_edit_on_update_sensitivities       ( const CactMenubar *bar );

void cact_menubar_edit_on_cut                        ( GtkAction *action, BaseWindow *window );
void cact_menubar_edit_on_copy                       ( GtkAction *action, BaseWindow *window );
void cact_menubar_edit_on_paste                      ( GtkAction *action, BaseWindow *window );
void cact_menubar_edit_on_paste_into                 ( GtkAction *action, BaseWindow *window );
void cact_menubar_edit_on_duplicate                  ( GtkAction *action, BaseWindow *window );
void cact_menubar_edit_on_delete                     ( GtkAction *action, BaseWindow *window );
void cact_menubar_edit_on_reload                     ( GtkAction *action, BaseWindow *window );
void cact_menubar_edit_on_prefererences              ( GtkAction *action, BaseWindow *window );

void cact_menubar_file_initialize                    (       CactMenubar *bar );
void cact_menubar_file_on_update_sensitivities       ( const CactMenubar *bar );

void cact_menubar_file_on_new_menu                   ( GtkAction *action, BaseWindow *window );
void cact_menubar_file_on_new_action                 ( GtkAction *action, BaseWindow *window );
void cact_menubar_file_on_new_profile                ( GtkAction *action, BaseWindow *window );
void cact_menubar_file_on_save                       ( GtkAction *action, BaseWindow *window );
void cact_menubar_file_on_quit                       ( GtkAction *action, BaseWindow *window );

void cact_menubar_file_save_items                    ( BaseWindow *window );

void cact_menubar_help_on_update_sensitivities       ( const CactMenubar *bar );

void cact_menubar_help_on_help                       ( GtkAction *action, BaseWindow *window );
void cact_menubar_help_on_about                      ( GtkAction *action, BaseWindow *window );

void cact_menubar_maintainer_on_update_sensitivities ( const CactMenubar *bar );

void cact_menubar_maintainer_on_dump_selection       ( GtkAction *action, BaseWindow *window );
void cact_menubar_maintainer_on_brief_tree_store_dump( GtkAction *action, BaseWindow *window );
void cact_menubar_maintainer_on_list_modified_items  ( GtkAction *action, BaseWindow *window );
void cact_menubar_maintainer_on_dump_clipboard       ( GtkAction *action, BaseWindow *window );
void cact_menubar_maintainer_on_test_function        ( GtkAction *action, BaseWindow *window );

void cact_menubar_tools_on_update_sensitivities      ( const CactMenubar *bar );

void cact_menubar_tools_on_import                    ( GtkAction *action, BaseWindow *window );
void cact_menubar_tools_on_export                    ( GtkAction *action, BaseWindow *window );

void cact_menubar_view_on_update_sensitivities       ( const CactMenubar *bar );

void cact_menubar_view_on_expand_all                 ( GtkAction *action, BaseWindow *window );
void cact_menubar_view_on_collapse_all               ( GtkAction *action, BaseWindow *window );
void cact_menubar_view_on_toolbar_file               ( GtkToggleAction *action, BaseWindow *window );
void cact_menubar_view_on_toolbar_edit               ( GtkToggleAction *action, BaseWindow *window );
void cact_menubar_view_on_toolbar_tools              ( GtkToggleAction *action, BaseWindow *window );
void cact_menubar_view_on_toolbar_help               ( GtkToggleAction *action, BaseWindow *window );

void cact_menubar_view_on_tabs_pos_changed           ( GtkRadioAction *action, GtkRadioAction *current, BaseWindow *window );

G_END_DECLS

#endif /* __CACT_MENUBAR_PRIV_H__ */
