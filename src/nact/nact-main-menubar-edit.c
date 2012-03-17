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

#include <api/na-core-utils.h>

#include <core/na-io-provider.h>
#include <core/na-iprefs.h>

#include "nact-application.h"
#include "nact-iactions-list.h"
#include "nact-main-tab.h"
#include "nact-main-menubar-edit.h"
#include "nact-preferences-editor.h"

static GList  *prepare_for_paste( NactMainWindow *window );
static GList  *get_deletables( NAUpdater *updater, GList *tree, GSList **not_deletable );
static GSList *get_deletables_rec( NAUpdater *updater, GList *tree );
static gchar  *add_non_deletable_msg( const NAObjectItem *item, gint reason );
static void    update_clipboard_counters( NactMainWindow *window );

/**
 * nact_main_menubar_edit_on_update_sensitivities:
 * @window: the #NactMainWindow main window.
 * user_data: the data passed to the function via the signal.
 * @mis: the #MenubarIndicatorsStruct struct.
 *
 * Update sensitivity of items of the Edit menu.
 */
void
nact_main_menubar_edit_on_update_sensitivities( NactMainWindow *window, gpointer user_data, MenubarIndicatorsStruct *mis )
{
	gboolean cut_enabled;
	gboolean copy_enabled;
	gboolean paste_enabled;
	gboolean paste_into_enabled;
	gboolean duplicate_enabled;
	gboolean delete_enabled;
	GList *is;
	NAObject *parent_item;
	NAObject *selected_action;
	NAObject *selected_item;
	gboolean are_parents_writable;
	gboolean is_clipboard_empty;

	is_clipboard_empty = ( mis->clipboard_menus + mis->clipboard_actions + mis->clipboard_profiles == 0 );

	/* cut requires a non-empty selection
	 * and that all parents are writable (as implies a delete operation)
	 */
	cut_enabled = mis->treeview_has_focus || mis->popup_handler;
	cut_enabled &= mis->count_selected > 0;
	are_parents_writable = TRUE;
	for( is = mis->selected_items ; is ; is = is->next ){
		parent_item = ( NAObject * ) na_object_get_parent( is->data );
		if( parent_item ){
			if( !na_updater_is_item_writable( mis->updater, NA_OBJECT_ITEM( parent_item ), NULL )){
				are_parents_writable = FALSE;
				break;
			}
		} else if( !mis->is_level_zero_writable ){
			are_parents_writable = FALSE;
			break;
		}
	}
	cut_enabled &= are_parents_writable;
	nact_main_menubar_enable_item( window, "CutItem", cut_enabled );

	/* copy only requires a non-empty selection */
	copy_enabled = mis->treeview_has_focus || mis->popup_handler;
	copy_enabled &= mis->count_selected > 0;
	nact_main_menubar_enable_item( window, "CopyItem", copy_enabled );

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
	paste_enabled = mis->treeview_has_focus || mis->popup_handler;
	paste_enabled &= !is_clipboard_empty;
	paste_enabled &= mis->count_selected <= 1;
	if( mis->clipboard_profiles ){
		paste_enabled &= mis->count_selected == 1;
		if( paste_enabled ){
			selected_action = NA_OBJECT(
					NA_IS_OBJECT_PROFILE( mis->selected_items->data )
							? na_object_get_parent( mis->selected_items->data )
							: mis->selected_items->data );
			paste_enabled &= NA_IS_OBJECT_ACTION( selected_action );
			paste_enabled &= na_updater_is_item_writable( mis->updater, NA_OBJECT_ITEM( selected_action ), NULL );
		}
	} else {
		paste_enabled &= mis->has_writable_providers;
		if( mis->count_selected ){
			selected_item = NA_OBJECT( mis->selected_items->data );
			paste_enabled &= NA_IS_OBJECT_ITEM( selected_item );
			if( paste_enabled ){
				parent_item = ( NAObject * ) na_object_get_parent( selected_item );
				paste_enabled &= parent_item
						? nact_window_is_item_writable( NACT_WINDOW( window ), NA_OBJECT_ITEM( parent_item ), NULL )
						: mis->is_level_zero_writable;
			}
		} else {
			paste_enabled &= mis->is_level_zero_writable;
		}
	}
	nact_main_menubar_enable_item( window, "PasteItem", paste_enabled );

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
	paste_into_enabled = mis->treeview_has_focus || mis->popup_handler;
	paste_into_enabled &= !is_clipboard_empty;
	paste_into_enabled &= mis->count_selected <= 1;
	if( mis->clipboard_profiles ){
		paste_into_enabled &= mis->count_selected == 1;
		if( paste_into_enabled ){
			selected_action = NA_OBJECT( mis->selected_items->data );
			paste_into_enabled &= NA_IS_OBJECT_ACTION( selected_action );
			paste_into_enabled &= nact_window_is_item_writable( NACT_WINDOW( window ), NA_OBJECT_ITEM( selected_action ), NULL );
		}
	} else {
		paste_into_enabled &= mis->has_writable_providers;
		if( mis->count_selected ){
			selected_item = NA_OBJECT( mis->selected_items->data );
			paste_into_enabled &= NA_IS_OBJECT_MENU( selected_item );
			if( paste_into_enabled ){
				parent_item = ( NAObject * ) na_object_get_parent( selected_item );
				paste_into_enabled &= parent_item
						? nact_window_is_item_writable( NACT_WINDOW( window ), NA_OBJECT_ITEM( parent_item ), NULL )
						: mis->is_level_zero_writable;
			}
		} else {
			paste_into_enabled &= mis->is_level_zero_writable;
		}
	}
	nact_main_menubar_enable_item( window, "PasteIntoItem", paste_into_enabled );

	/* duplicate items will be duplicated besides each one
	 * selection must be non-empty
	 * each parent must be writable
	 * -> so this is the same than cut
	 */
	duplicate_enabled = cut_enabled;
	nact_main_menubar_enable_item( window, "DuplicateItem", duplicate_enabled );

	/* delete is same that cut
	 * but items themselves must be writable (because physically deleted)
	 * this will be checked on delete activated
	 */
	delete_enabled = cut_enabled;
	nact_main_menubar_enable_item( window, "DeleteItem", delete_enabled );

	/* reload items always enabled */

	/* preferences always enabled */
}

/**
 * nact_main_menubar_edit_on_cut:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
 *
 * cuts the visible selection
 * - (tree) get new refs on selected items
 * - (main) add selected items to main list of deleted,
 *          moving newref from list_from_tree to main_list_of_deleted
 * - (menu) install in clipboard a copy of selected objects
 * - (tree) remove selected items, unreffing objects
 */
void
nact_main_menubar_edit_on_cut( GtkAction *gtk_action, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_edit_on_cut";
	NactApplication *application;
	NAUpdater *updater;
	GList *items;
	NactClipboard *clipboard;
	GList *to_delete;
	GSList *non_deletables;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = nact_application_get_updater( application );
	items = nact_iactions_list_bis_get_selected_items( NACT_IACTIONS_LIST( window ));

	non_deletables = NULL;
	to_delete = get_deletables( updater, items, &non_deletables );

	if( non_deletables ){
		gchar *second = na_core_utils_slist_join_at_end( non_deletables, "\n" );
		base_window_error_dlg(
				BASE_WINDOW( window ),
				GTK_MESSAGE_INFO,
				_( "Not all items have been cut as following ones are not modifiable:" ),
				second );
		g_free( second );
		na_core_utils_slist_free( non_deletables );
	}

	if( to_delete ){
		nact_main_window_move_to_deleted( window, to_delete );
		clipboard = nact_main_window_get_clipboard( window );
		nact_clipboard_primary_set( clipboard, to_delete, CLIPBOARD_MODE_CUT );
		update_clipboard_counters( window );
		nact_iactions_list_bis_delete( NACT_IACTIONS_LIST( window ), to_delete, TRUE );
	}

	na_object_unref_selected_items( items );
}

/**
 * nact_main_menubar_edit_on_copy:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
 *
 * copies the visible selection
 * - (tree) get new refs on selected items
 * - (menu) install in clipboard a copy of selected objects
 *          renumbering actions/menus id to ensure unicity at paste time
 * - (menu) release refs on selected items
 * - (menu) refresh actions sensitivy (as selection doesn't change)
 */
void
nact_main_menubar_edit_on_copy( GtkAction *gtk_action, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_edit_on_copy";
	GList *items;
	NactClipboard *clipboard;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	items = nact_iactions_list_bis_get_selected_items( NACT_IACTIONS_LIST( window ));
	clipboard = nact_main_window_get_clipboard( window );
	nact_clipboard_primary_set( clipboard, items, CLIPBOARD_MODE_COPY );
	update_clipboard_counters( window );
	na_object_unref_selected_items( items );

	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

/**
 * nact_main_menubar_edit_on_paste:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
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
nact_main_menubar_edit_on_paste( GtkAction *gtk_action, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_edit_on_paste";
	GList *items;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );

	items = prepare_for_paste( window );
	if( items ){
		nact_iactions_list_bis_insert_items( NACT_IACTIONS_LIST( window ), items, NULL );
		na_object_unref_items( items );
	}
}

/**
 * nact_main_menubar_edit_on_paste_into:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
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
nact_main_menubar_edit_on_paste_into( GtkAction *gtk_action, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_edit_on_paste_into";
	GList *items;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );

	items = prepare_for_paste( window );
	if( items ){
		nact_iactions_list_bis_insert_into( NACT_IACTIONS_LIST( window ), items );
		na_object_unref_items( items );
	}
}

static GList *
prepare_for_paste( NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_edit_prepare_for_paste";
	GList *items, *it;
	NactClipboard *clipboard;
	NAObjectAction *action;
	gboolean relabel;
	gboolean renumber;
	NactApplication *application;
	NAUpdater *updater;

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = nact_application_get_updater( application );

	clipboard = nact_main_window_get_clipboard( window );
	items = nact_clipboard_primary_get( clipboard, &renumber );
	action = NULL;

	/* if pasted items are profiles, then setup the target action
	 */
	for( it = items ; it ; it = it->next ){

		if( NA_IS_OBJECT_PROFILE( it->data )){
			if( !action ){
				g_object_get( G_OBJECT( window ), TAB_UPDATABLE_PROP_EDITED_ACTION, &action, NULL );
				g_return_val_if_fail( NA_IS_OBJECT_ACTION( action ), NULL );
			}
		}

		relabel = nact_main_menubar_edit_is_pasted_object_relabeled( NA_OBJECT( it->data ), NA_PIVOT( updater ));
		na_object_prepare_for_paste( it->data, relabel, renumber, action );
		na_object_check_status( it->data );
	}

	g_debug( "%s: action=%p (%s)",
			thisfn, ( void * ) action, action ? G_OBJECT_TYPE_NAME( action ): "(null)" );

	return( items );
}

/**
 * nact_main_menubar_edit_on_duplicate:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
 *
 * duplicate is just as paste, with the difference that content comes
 * from the current selection, instead of coming from the clipboard
 *
 * this is nonetheless a bit more complicated because when we duplicate
 * some items (e.g. a multiple selection), we expect to see the new
 * items just besides the original ones...
 */
void
nact_main_menubar_edit_on_duplicate( GtkAction *gtk_action, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_edit_on_duplicate";
	NactApplication *application;
	NAUpdater *updater;
	NAObjectAction *action;
	GList *items, *it;
	GList *dup;
	NAObject *obj;
	gboolean relabel;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = nact_application_get_updater( application );

	items = nact_iactions_list_bis_get_selected_items( NACT_IACTIONS_LIST( window ));
	for( it = items ; it ; it = it->next ){
		obj = NA_OBJECT( na_object_duplicate( it->data ));
		action = NULL;

		/* duplicating a profile
		 * as we insert in sibling mode, the parent doesn't change
		 */
		if( NA_IS_OBJECT_PROFILE( obj )){
			action = NA_OBJECT_ACTION( na_object_get_parent( it->data ));
		}

		relabel = nact_main_menubar_edit_is_pasted_object_relabeled( obj, NA_PIVOT( updater ));
		na_object_prepare_for_paste( obj, relabel, TRUE, action );
		na_object_set_origin( obj, NULL );
		na_object_check_status( obj );
		dup = g_list_prepend( NULL, obj );
		nact_iactions_list_bis_insert_items( NACT_IACTIONS_LIST( window ), dup, it->data );
		na_object_unref_items( dup );
	}

	na_object_unref_selected_items( items );
}

/**
 * nact_main_menubar_edit_on_delete:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
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
nact_main_menubar_edit_on_delete( GtkAction *gtk_action, NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_menubar_edit_on_delete";
	NactApplication *application;
	NAUpdater *updater;
	GList *items;
	GList *to_delete;
	GSList *non_deletables;

	g_debug( "%s: gtk_action=%p, window=%p", thisfn, ( void * ) gtk_action, ( void * ) window );
	g_return_if_fail( GTK_IS_ACTION( gtk_action ));
	g_return_if_fail( NACT_IS_MAIN_WINDOW( window ));

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = nact_application_get_updater( application );
	items = nact_iactions_list_bis_get_selected_items( NACT_IACTIONS_LIST( window ));

	non_deletables = NULL;
	to_delete = get_deletables( updater, items, &non_deletables );

	if( non_deletables ){
		gchar *second = na_core_utils_slist_join_at_end( non_deletables, "\n" );
		base_window_error_dlg(
				BASE_WINDOW( window ),
				GTK_MESSAGE_INFO,
				_( "Not all items have been deleted as following ones are not modifiable:" ),
				second );
		g_free( second );
		na_core_utils_slist_free( non_deletables );
	}

	if( to_delete ){
		nact_main_window_move_to_deleted( window, to_delete );
		nact_iactions_list_bis_delete( NACT_IACTIONS_LIST( window ), to_delete, TRUE );
	}

	na_object_unref_selected_items( items );
}

static GList *
get_deletables( NAUpdater *updater, GList *selected, GSList **non_deletables )
{
	GList *to_delete;
	GList *it;
	GList *subitems;
	GSList *sub_deletables;
	gint reason;

	to_delete = NULL;
	for( it = selected ; it ; it = it->next ){

		if( !na_updater_is_item_writable( updater, NA_OBJECT_ITEM( it->data ), &reason )){
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
	gint reason;

	msgs = NULL;
	for( it = tree ; it ; it = it->next ){

		if( !na_updater_is_item_writable( updater, NA_OBJECT_ITEM( it->data ), &reason )){
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
update_clipboard_counters( NactMainWindow *window )
{
	MenubarIndicatorsStruct *mis;

	mis = ( MenubarIndicatorsStruct * ) g_object_get_data( G_OBJECT( window ), MENUBAR_PROP_INDICATORS );

	mis->clipboard_menus = mis->selected_menus;
	mis->clipboard_actions = mis->selected_actions;
	mis->clipboard_profiles = mis->selected_profiles;

	g_debug( "nact_main_menubar_update_clipboard_counters: menus=%d, actions=%d, profiles=%d",
			mis->clipboard_menus, mis->clipboard_actions, mis->clipboard_profiles );

	g_signal_emit_by_name( window, MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES, NULL );
}

/**
 * nact_main_menubar_edit_on_reload:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
 *
 * Reload items from I/O storage subsystems.
 */
void
nact_main_menubar_edit_on_reload( GtkAction *gtk_action, NactMainWindow *window )
{
	nact_main_window_reload( window );
}

/**
 * nact_main_menubar_edit_on_preferences:
 * @gtk_action: the #GtkAction action.
 * @window: the #NactMainWindow main window.
 *
 * Edit preferences.
 */
void
nact_main_menubar_edit_on_prefererences( GtkAction *gtk_action, NactMainWindow *window )
{
	nact_preferences_editor_run( BASE_WINDOW( window ));
}

/**
 * nact_main_menubar_edit_is_pasted_object_relabeled:
 * @object: the considered #NAObject-derived object.
 * @pivot: the #NAPivot instance.
 *
 * Whether the specified object should be relabeled when pasted ?
 *
 * Returns: %TRUE if the object should be relabeled, %FALSE else.
 */
gboolean
nact_main_menubar_edit_is_pasted_object_relabeled( NAObject *object, NAPivot *pivot )
{
	static const gchar *thisfn = "nact_main_menubar_edit_is_pasted_object_relabeled";
	gboolean relabel;

	if( NA_IS_OBJECT_MENU( object )){
		relabel = na_iprefs_read_bool( NA_IPREFS( pivot ), IPREFS_RELABEL_MENUS, FALSE );
	} else if( NA_IS_OBJECT_ACTION( object )){
		relabel = na_iprefs_read_bool( NA_IPREFS( pivot ), IPREFS_RELABEL_ACTIONS, FALSE );
	} else if( NA_IS_OBJECT_PROFILE( object )){
		relabel = na_iprefs_read_bool( NA_IPREFS( pivot ), IPREFS_RELABEL_PROFILES, FALSE );
	} else {
		g_warning( "%s: unknown object type at %p", thisfn, ( void * ) object );
		g_return_val_if_reached( FALSE );
	}

	return( relabel );
}
