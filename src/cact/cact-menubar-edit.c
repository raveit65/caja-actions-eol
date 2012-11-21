/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The MATE Foundation
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

#include <api/na-core-utils.h>

#include <core/na-io-provider.h>

#include "cact-application.h"
#include "cact-main-tab.h"
#include "cact-menubar-priv.h"
#include "cact-preferences-editor.h"
#include "cact-tree-ieditable.h"

static GList  *prepare_for_paste( BaseWindow *window );
static GList  *get_deletables( NAUpdater *updater, GList *tree, GSList **not_deletable );
static GSList *get_deletables_rec( NAUpdater *updater, GList *tree );
static gchar  *add_non_deletable_msg( const NAObjectItem *item, gint reason );
static void    update_clipboard_counters( BaseWindow *window );

/**
 * cact_menubar_edit_on_update_sensitivities:
 * @bar: this #CactMenubar object.
 *
 * Update sensitivity of items of the Edit menu.
 *
 * Each action (cut, copy, delete, etc.) takes itself care of whether it
 * can safely apply to all the selection or not. The action can at least
 * assume that at least one item will be a valid candidate (this is the
 * Menubar rule).
 */
void
cact_menubar_edit_on_update_sensitivities( const CactMenubar *bar )
{
	gboolean cut_enabled;
	gboolean copy_enabled;
	gboolean paste_enabled;
	gboolean paste_into_enabled;
	gboolean duplicate_enabled;
	gboolean delete_enabled;
	NAObject *parent_item;
	NAObject *selected_action;
	NAObject *selected_item;
	gboolean is_clipboard_empty;

	is_clipboard_empty = ( bar->private->clipboard_menus + bar->private->clipboard_actions + bar->private->clipboard_profiles == 0 );

	/* cut requires a non-empty selection
	 * and that the selection is writable (can be modified, i.e. is not read-only)
	 * and that all parents are writable (as implies a delete operation)
	 */
	duplicate_enabled = bar->private->treeview_has_focus || bar->private->popup_handler;
	duplicate_enabled &= bar->private->count_selected > 0;
	duplicate_enabled &= bar->private->are_parents_writable;
	cut_enabled = duplicate_enabled;
	cut_enabled &= bar->private->are_items_writable;
	cact_menubar_enable_item( bar, "CutItem", cut_enabled );

	/* copy only requires a non-empty selection */
	copy_enabled = bar->private->treeview_has_focus || bar->private->popup_handler;
	copy_enabled &= bar->private->count_selected > 0;
	cact_menubar_enable_item( bar, "CopyItem", copy_enabled );

	/* paste enabled if
	 * - clipboard is not empty
	 * - current selection is not multiple
	 * - if clipboard contains only profiles,
	 *   then current selection must be a profile or an action
	 *   and the action must be writable
	 * - if clipboard contains actions or menus,
	 *   then current selection (if any) must be a menu or an action
	 *   and its parent must be writable
	 */
	paste_enabled = bar->private->treeview_has_focus || bar->private->popup_handler;
	paste_enabled &= !is_clipboard_empty;
	paste_enabled &= bar->private->count_selected <= 1;
	if( bar->private->clipboard_profiles ){
		paste_enabled &= bar->private->count_selected == 1;
		paste_enabled &= bar->private->is_action_writable;
	} else {
		paste_enabled &= bar->private->has_writable_providers;
		if( bar->private->count_selected ){
			paste_enabled &= bar->private->is_parent_writable;
		} else {
			paste_enabled &= bar->private->is_level_zero_writable;
		}
	}
	cact_menubar_enable_item( bar, "PasteItem", paste_enabled );

	/* paste into enabled if
	 * - clipboard is not empty
	 * - current selection is not multiple
	 * - if clipboard contains only profiles,
	 *   then current selection must be an action
	 *   and the action must be writable
	 * - if clipboard contains actions or menus,
	 *   then current selection (if any) must be a menu
	 *   and its parent must be writable
	 */
	paste_into_enabled = bar->private->treeview_has_focus || bar->private->popup_handler;
	paste_into_enabled &= !is_clipboard_empty;
	paste_into_enabled &= bar->private->count_selected <= 1;
	if( bar->private->clipboard_profiles ){
		paste_into_enabled &= bar->private->count_selected == 1;
		if( paste_into_enabled ){
			selected_action = NA_OBJECT( bar->private->selected_items->data );
			paste_into_enabled &= NA_IS_OBJECT_ACTION( selected_action );
			if( paste_into_enabled ){
				paste_into_enabled &= na_object_is_finally_writable( selected_action, NULL );
			}
		}
	} else {
		paste_into_enabled &= bar->private->has_writable_providers;
		if( bar->private->count_selected ){
			selected_item = NA_OBJECT( bar->private->selected_items->data );
			paste_into_enabled &= NA_IS_OBJECT_MENU( selected_item );
			if( paste_into_enabled ){
				parent_item = ( NAObject * ) na_object_get_parent( selected_item );
				paste_into_enabled &= parent_item
						? na_object_is_finally_writable( parent_item, NULL )
						: bar->private->is_level_zero_writable;
			}
		} else {
			paste_into_enabled &= bar->private->is_level_zero_writable;
		}
	}
	cact_menubar_enable_item( bar, "PasteIntoItem", paste_into_enabled );

	/* duplicate items will be duplicated besides each one
	 * selection must be non-empty
	 * each parent must be writable
	 */
	cact_menubar_enable_item( bar, "DuplicateItem", duplicate_enabled );

	/* delete is same that cut
	 * but items themselves must be writable (because physically deleted)
	 * this will be checked on delete activated
	 */
	delete_enabled = cut_enabled;
	cact_menubar_enable_item( bar, "DeleteItem", delete_enabled );

	/* reload items always enabled */

	/* preferences always enabled */
}

/**
 * cact_menubar_edit_on_cut:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Cut objects are installed both in the clipboard and in the deleted list.
 * Parent pointer is reset to %NULL.
 * Old parent status is re-checked by the tree model delete operation.
 * When pasting later these cut objects:
 * - the first time, we paste the very same object, removing it from the
 *   deleted list, attaching it to a new parent
 *   the object itself is not modified, but the parent is.
 * - the following times, we paste a copy of this object with a new identifier
 *
 * cuts the visible selection
 * - (tree) get new refs on selected items
 * - (main) add selected items to main list of deleted,
 *          moving newref from list_from_tree to main_list_of_deleted
 * - (menu) install in clipboard a copy of selected objects
 * - (tree) remove selected items, unreffing objects
 */
void
cact_menubar_edit_on_cut( GtkAction *gtk_action, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_edit_on_cut";
	GList *items;
	CactClipboard *clipboard;
	GList *to_delete;
	GSList *non_deletables;
	CactTreeView *view;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));

	BAR_WINDOW_VOID( window );

	items = na_object_copyref_items( bar->private->selected_items );
	non_deletables = NULL;
	to_delete = get_deletables( bar->private->updater, items, &non_deletables );

	if( non_deletables ){
		gchar *second = na_core_utils_slist_join_at_end( non_deletables, "\n" );
		base_window_display_error_dlg(
				BASE_WINDOW( window ),
				_( "Not all items have been cut as following ones are not modifiable:" ),
				second );
		g_free( second );
		na_core_utils_slist_free( non_deletables );
	}

	if( to_delete ){
		clipboard = cact_main_window_get_clipboard( CACT_MAIN_WINDOW( window ));
		cact_clipboard_primary_set( clipboard, to_delete, CLIPBOARD_MODE_CUT );
		update_clipboard_counters( window );
		view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
		cact_tree_ieditable_delete( CACT_TREE_IEDITABLE( view ), to_delete, TREE_OPE_DELETE );
	}

	na_object_free_items( items );
}

/**
 * cact_menubar_edit_on_copy:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * copies the visible selection
 * - (tree) get new refs on selected items
 * - (menu) install in clipboard a copy of selected objects
 *          renumbering actions/menus id to ensure unicity at paste time
 * - (menu) release refs on selected items
 * - (menu) refresh actions sensitivy (as selection doesn't change)
 */
void
cact_menubar_edit_on_copy( GtkAction *gtk_action, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_edit_on_copy";
	CactClipboard *clipboard;

	BAR_WINDOW_VOID( window );

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));
	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	clipboard = cact_main_window_get_clipboard( CACT_MAIN_WINDOW( window ));
	cact_clipboard_primary_set( clipboard, bar->private->selected_items, CLIPBOARD_MODE_COPY );
	update_clipboard_counters( window );

	g_signal_emit_by_name( bar, MENUBAR_SIGNAL_UPDATE_SENSITIVITIES );
}

/**
 * cact_menubar_edit_on_paste:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * pastes the current content of the clipboard at the current position
 * (same path, same level)
 * - (menu) get from clipboard a copy of installed items
 *          the clipboard will return a new copy
 *          and renumber its own data for allowing a new paste
 * - (tree) insert new items, the tree store will ref them
 *          attaching each item to its parent
 *          recursively checking edition status of the topmost parent
 *          selecting the first item at end
 * - (menu) unreffing the copy got from clipboard
 */
void
cact_menubar_edit_on_paste( GtkAction *gtk_action, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_edit_on_paste";
	GList *items;
	CactTreeView *view;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );

	items = prepare_for_paste( window );

	if( items ){
		view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
		cact_tree_ieditable_insert_items( CACT_TREE_IEDITABLE( view ), items, NULL );
		na_object_free_items( items );
	}
}

/**
 * cact_menubar_edit_on_paste_into:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * pastes the current content of the clipboard as the first child of
 * currently selected item
 * - (menu) get from clipboard a copy of installed items
 *          the clipboard will return a new copy
 *          and renumber its own data for allowing a new paste
 * - (tree) insert new items, the tree store will ref them
 *          attaching each item to its parent
 *          recursively checking edition status of the topmost parent
 *          selecting the first item at end
 * - (menu) unreffing the copy got from clipboard
 */
void
cact_menubar_edit_on_paste_into( GtkAction *gtk_action, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_edit_on_paste_into";
	GList *items;
	CactTreeView *view;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );

	items = prepare_for_paste( window );
	if( items ){
		view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
		cact_tree_ieditable_insert_into( CACT_TREE_IEDITABLE( view ), items );
		na_object_free_items( items );
	}
}

static GList *
prepare_for_paste( BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_edit_prepare_for_paste";
	GList *items, *it;
	CactClipboard *clipboard;
	NAObjectAction *action;
	gboolean relabel;
	gboolean renumber;

	BAR_WINDOW_VALUE( window, NULL );

	clipboard = cact_main_window_get_clipboard( CACT_MAIN_WINDOW( window ));
	items = cact_clipboard_primary_get( clipboard, &renumber );
	action = NULL;

	/* if pasted items are profiles, then setup the target action
	 */
	for( it = items ; it ; it = it->next ){

		if( NA_IS_OBJECT_PROFILE( it->data )){
			if( !action ){
				g_object_get( G_OBJECT( window ), MAIN_PROP_ITEM, &action, NULL );
				g_return_val_if_fail( NA_IS_OBJECT_ACTION( action ), NULL );
			}
		}

		relabel = na_updater_should_pasted_be_relabeled( bar->private->updater, NA_OBJECT( it->data ));
		na_object_prepare_for_paste( it->data, relabel, renumber, action );
		na_object_check_status( it->data );
	}

	g_debug( "%s: action=%p (%s)",
			thisfn, ( void * ) action, action ? G_OBJECT_TYPE_NAME( action ): "(null)" );

	return( items );
}

/**
 * cact_menubar_edit_on_duplicate:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * duplicate is just as paste, with the difference that content comes
 * from the current selection, instead of coming from the clipboard
 *
 * this is nonetheless a bit more complicated because when we duplicate
 * some items (e.g. a multiple selection), we expect to see the new
 * items just besides the original ones...
 */
void
cact_menubar_edit_on_duplicate( GtkAction *gtk_action, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_edit_on_duplicate";
	NAObjectAction *action;
	GList *items, *it;
	GList *dup;
	NAObject *obj;
	gboolean relabel;
	CactTreeView *view;

	BAR_WINDOW_VOID( window );

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));
	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	items = na_object_copyref_items( bar->private->selected_items );

	for( it = items ; it ; it = it->next ){
		obj = NA_OBJECT( na_object_duplicate( it->data, DUPLICATE_REC ));
		action = NULL;

		/* duplicating a profile
		 * as we insert in sibling mode, the parent doesn't change
		 */
		if( NA_IS_OBJECT_PROFILE( obj )){
			action = NA_OBJECT_ACTION( na_object_get_parent( it->data ));
		}

		relabel = na_updater_should_pasted_be_relabeled( bar->private->updater, obj );
		na_object_prepare_for_paste( obj, relabel, TRUE, action );
		na_object_set_origin( obj, NULL );
		na_object_check_status( obj );
		dup = g_list_prepend( NULL, obj );
		view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
		cact_tree_ieditable_insert_items( CACT_TREE_IEDITABLE( view ), dup, it->data );
		na_object_free_items( dup );
	}

	na_object_free_items( items );
}

/**
 * cact_menubar_edit_on_delete:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * deletes the visible selection
 * - (tree) get new refs on selected items
 * - (tree) remove selected items, unreffing objects
 * - (main) add selected items to main list of deleted,
 *          moving newref from list_from_tree to main_list_of_deleted
 * - (tree) select next row (if any, or previous if any, or none)
 *
 * note that we get from selection a list of trees, but we don't have
 * yet ensured that each element of this tree is actually deletable
 * each branch of this list must be recursively deletable in order
 * this branch itself be deleted
 */
void
cact_menubar_edit_on_delete( GtkAction *gtk_action, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_edit_on_delete";
	GList *items;
	GList *to_delete;
	GSList *non_deletables;
	CactTreeView *view;

	BAR_WINDOW_VOID( window );

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));
	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	items = na_object_copyref_items( bar->private->selected_items );
	non_deletables = NULL;
	to_delete = get_deletables( bar->private->updater, items, &non_deletables );

	if( non_deletables ){
		gchar *second = na_core_utils_slist_join_at_end( non_deletables, "\n" );
		base_window_display_error_dlg(
				BASE_WINDOW( window ),
				_( "Not all items have been deleted as following ones are not modifiable:" ),
				second );
		g_free( second );
		na_core_utils_slist_free( non_deletables );
	}

	if( to_delete ){
		view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
		cact_tree_ieditable_delete( CACT_TREE_IEDITABLE( view ), to_delete, TREE_OPE_DELETE );
	}

	na_object_free_items( items );
}

static GList *
get_deletables( NAUpdater *updater, GList *selected, GSList **non_deletables )
{
	GList *to_delete;
	GList *it;
	GList *subitems;
	GSList *sub_deletables;
	guint reason;
	NAObjectItem *item;

	to_delete = NULL;
	for( it = selected ; it ; it = it->next ){

		if( NA_IS_OBJECT_PROFILE( it->data )){
			item = na_object_get_parent( it->data );
		} else {
			item = NA_OBJECT_ITEM( it->data );
		}

		if( !na_object_is_finally_writable( item, &reason )){
			*non_deletables = g_slist_prepend(
					*non_deletables, add_non_deletable_msg( NA_OBJECT_ITEM( it->data ), reason ));
			continue;
		}

		if( NA_IS_OBJECT_MENU( it->data )){
			subitems = na_object_get_items( it->data );
			sub_deletables = get_deletables_rec( updater, subitems );

			if( sub_deletables ){
				*non_deletables = g_slist_concat( *non_deletables, sub_deletables );
				continue;
			}
		}

		to_delete = g_list_prepend( to_delete, na_object_ref( it->data ));
	}

	return( to_delete );
}

static GSList *
get_deletables_rec( NAUpdater *updater, GList *tree )
{
	GSList *msgs;
	GList *it;
	GList *subitems;
	guint reason;

	msgs = NULL;
	for( it = tree ; it ; it = it->next ){

		if( !na_object_is_finally_writable( it->data, &reason )){
			msgs = g_slist_prepend(
					msgs, add_non_deletable_msg( NA_OBJECT_ITEM( it->data ), reason ));
			continue;
		}

		if( NA_IS_OBJECT_MENU( it->data )){
			subitems = na_object_get_items( it->data );
			msgs = g_slist_concat( msgs, get_deletables_rec( updater, subitems ));
		}
	}

	return( msgs );
}

static gchar *
add_non_deletable_msg( const NAObjectItem *item, gint reason )
{
	gchar *msg;
	gchar *label;
	gchar *reason_str;

	label = na_object_get_label( item );
	reason_str = na_io_provider_get_readonly_tooltip( reason );

	msg = g_strdup_printf( "%s: %s", label, reason_str );

	g_free( reason_str );
	g_free( label );

	return( msg );
}

/*
 * as we are coming from cut or copy to clipboard, report selection
 * counters to clipboard ones
 */
static void
update_clipboard_counters( BaseWindow *window )
{
	BAR_WINDOW_VOID( window );

	bar->private->clipboard_menus = bar->private->selected_menus;
	bar->private->clipboard_actions = bar->private->selected_actions;
	bar->private->clipboard_profiles = bar->private->selected_profiles;

	g_debug( "cact_menubar_update_clipboard_counters: menus=%d, actions=%d, profiles=%d",
			bar->private->clipboard_menus, bar->private->clipboard_actions, bar->private->clipboard_profiles );

	g_signal_emit_by_name( bar, MENUBAR_SIGNAL_UPDATE_SENSITIVITIES );
}

/**
 * cact_menubar_edit_on_reload:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Reload items from I/O storage subsystems.
 */
void
cact_menubar_edit_on_reload( GtkAction *gtk_action, BaseWindow *window )
{
	cact_main_window_reload( CACT_MAIN_WINDOW( window ));
}

/**
 * cact_menubar_edit_on_preferences:
 * @gtk_action: the #GtkAction action.
 * @window: the #BaseWindow main window.
 *
 * Edit preferences.
 */
void
cact_menubar_edit_on_prefererences( GtkAction *gtk_action, BaseWindow *window )
{
	cact_preferences_editor_run( window );
}
