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

#include <gdk/gdkkeysyms.h>

#include <api/na-object-api.h>

#include "base-window.h"
#include "nact-main-menubar.h"
#include "nact-main-tab.h"
#include "nact-marshal.h"
#include "nact-tree-model.h"
#include "nact-iactions-list.h"
#include "nact-iactions-list-priv.h"

/* private interface data
 */
struct NactIActionsListInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* when iterating through a selection
 */
typedef struct {
	gboolean has_menu_or_action;
}
	SelectionIter;

/* signals
 */
enum {
	LIST_COUNT_UPDATED,
	SELECTION_CHANGED,
	FOCUS_IN,
	FOCUS_OUT,
	COLUMN_EDITED,
	STATUS_CHANGED,
	LAST_SIGNAL
};

static gint     st_signals[ LAST_SIGNAL ] = { 0 };
       gboolean st_iactions_list_initialized = FALSE;
       gboolean st_iactions_list_finalized = FALSE;

static GType    register_type( void );
static void     interface_base_init( NactIActionsListInterface *klass );
static void     interface_base_finalize( NactIActionsListInterface *klass );

static void     free_items_callback( NactIActionsList *instance, GList *items );
static void     free_column_edited_callback( NactIActionsList *instance, NAObject *object, gchar *text, gint column );

static gboolean are_profiles_displayed( NactIActionsList *instance, IActionsListInstanceData *ialid );
static void     display_label( GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, NactIActionsList *instance );
static gboolean filter_selection( GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, NactIActionsList *instance );
static gboolean filter_selection_is_homogeneous( GtkTreeSelection *selection, NAObject *object );
static void     filter_selection_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, SelectionIter *str );
static gboolean filter_selection_has_menu_or_action( GtkTreeSelection *selection );
static gboolean filter_selection_is_implicitely_selected( NAObject *object );
static void     filter_selection_set_implicitely_selected_childs( NAObject *object, gboolean select );
static gboolean have_dnd_mode( NactIActionsList *instance, IActionsListInstanceData *ialid );
static gboolean have_filter_selection_mode( NactIActionsList *instance, IActionsListInstanceData *ialid );
static void     inline_edition( NactIActionsList *instance );
static gboolean is_iduplicable_proxy( NactIActionsList *instance, IActionsListInstanceData *ialid );
static gboolean on_button_press_event( GtkWidget *widget, GdkEventButton *event, NactIActionsList *instance );
static void     on_edition_status_changed( NactIActionsList *instance, NAIDuplicable *object );
static gboolean on_focus_in( GtkWidget *widget, GdkEventFocus *event, NactIActionsList *instance );
static gboolean on_focus_out( GtkWidget *widget, GdkEventFocus *event, NactIActionsList *instance );
static gboolean on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, NactIActionsList *instance );
static void     on_label_edited( GtkCellRendererText *renderer, const gchar *path, const gchar *text, NactIActionsList *instance );
static void     on_tab_updatable_item_updated( NactIActionsList *instance, NAObject *object, gboolean force_display );
static void     open_popup( NactIActionsList *instance, GdkEventButton *event );

GType
nact_iactions_list_get_type( void )
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
	static const gchar *thisfn = "nact_iactions_list_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NactIActionsListInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NactIActionsList", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_WINDOW_TYPE );

	return( type );
}

static void
interface_base_init( NactIActionsListInterface *klass )
{
	static const gchar *thisfn = "nact_iactions_list_interface_base_init";

	if( !st_iactions_list_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NactIActionsListInterfacePrivate, 1 );

		/**
		 * nact-iactions-list-count-updated:
		 *
		 * This signal is emitted byIActionsList to its implementor when
		 * it has been asked to refill the list.
		 *
		 * It sends as arguments to the connected handlers the total
		 * count of menus, actions and profiles in the stored list.
		 */
		st_signals[ LIST_COUNT_UPDATED ] = g_signal_new(
				IACTIONS_LIST_SIGNAL_LIST_COUNT_UPDATED,
				G_TYPE_OBJECT,
				G_SIGNAL_RUN_LAST,
				0,						/* no default handler */
				NULL,
				NULL,
				nact_marshal_VOID__INT_INT_INT,
				G_TYPE_NONE,
				3,
				G_TYPE_INT,
				G_TYPE_INT,
				G_TYPE_INT );

		/**
		 * nact-iactions-list-selection-changed:
		 *
		 * This signal is emitted byIActionsList to its implementor,
		 * in response to the "changed" Gtk signal, each time the
		 * selection has changed in the treeview.
		 *
		 * It is not just a proxy, as we add a list of currently selected
		 * objects as user_data (see #nact_iactions_list_on_treeview_selection_changed()).
		 *
		 * Note that IActionsList is itself connected to this signal,
		 * in order to convert the signal to an interface API
		 * (see #on_iactions_list_selection_changed()).
		 *
		 * The main window is typically the only interested. It will
		 * setup current item and profiles, before emitting another
		 * signal targeting the notebook tabs
		 * (see. MAIN_WINDOW_SIGNAL_SELECTION_CHANGED signal).
		 */
		st_signals[ SELECTION_CHANGED ] = g_signal_new_class_handler(
				IACTIONS_LIST_SIGNAL_SELECTION_CHANGED,
				G_TYPE_OBJECT,
				G_SIGNAL_RUN_CLEANUP,
				G_CALLBACK( free_items_callback ),
				NULL,
				NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE,
				1,
				G_TYPE_POINTER );

		/**
		 * nact-iactions-list-focus-in:
		 *
		 * This signal is emitted byIActionsList when it gains the focus.
		 * In particular, edition menu is disabled outside of the treeview.
		 */
		st_signals[ FOCUS_IN ] = g_signal_new(
				IACTIONS_LIST_SIGNAL_FOCUS_IN,
				G_TYPE_OBJECT,
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE,
				1,
				G_TYPE_POINTER );

		/**
		 * nact-iactions-list-focus-out:
		 *
		 * This signal is emitted byIActionsList when it looses the focus.
		 * In particular, edition menu is disabled outside of the treeview.
		 */
		st_signals[ FOCUS_OUT ] = g_signal_new(
				IACTIONS_LIST_SIGNAL_FOCUS_OUT,
				G_TYPE_OBJECT,
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE,
				1,
				G_TYPE_POINTER );

		/**
		 * nact-iactions-list-column-edited:
		 *
		 * This signal is emitted byIActionsList when there has been an
		 * inline edition in one of the columns.
		 * The edition tabs should updates their own entries.
		 */
		st_signals[ COLUMN_EDITED ] = g_signal_new_class_handler(
				IACTIONS_LIST_SIGNAL_COLUMN_EDITED,
				G_TYPE_OBJECT,
				G_SIGNAL_RUN_CLEANUP,
				G_CALLBACK( free_column_edited_callback ),
				NULL,
				NULL,
				nact_marshal_VOID__POINTER_POINTER_INT,
				G_TYPE_NONE,
				3,
				G_TYPE_POINTER,
				G_TYPE_POINTER,
				G_TYPE_INT );

		/**
		 * nact-iactions-list-status-changed:
		 *
		 * This signal is emitted byIActionsList to its implementor
		 * when the status of an item has changed.
		 */
		st_signals[ STATUS_CHANGED ] = g_signal_new(
				IACTIONS_LIST_SIGNAL_STATUS_CHANGED,
				G_TYPE_OBJECT,
				G_SIGNAL_RUN_LAST,
				0,						/* no default handler */
				NULL,
				NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE,
				1,
				G_TYPE_POINTER );

		st_iactions_list_initialized = TRUE;
	}
}

static void
free_items_callback( NactIActionsList *instance, GList *items )
{
	g_debug( "nact_iactions_list_free_items_callback: selection=%p (%d items)",
			( void * ) items, g_list_length( items ));

	na_object_unref_selected_items( items );
}

static void
free_column_edited_callback( NactIActionsList *instance, NAObject *object, gchar *text, gint column )
{
	static const gchar *thisfn = "nact_iactions_list_free_column_edited_callback";

	g_debug( "%s: instance=%p, object=%p (%s), text=%s, column=%d",
			thisfn, ( void * ) instance, ( void * ) object, G_OBJECT_TYPE_NAME( object ), text, column );

	g_free( text );
}

static void
interface_base_finalize( NactIActionsListInterface *klass )
{
	static const gchar *thisfn = "nact_iactions_list_interface_base_finalize";

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_iactions_list_finalized = TRUE;

		g_free( klass->private );
	}
}

/**
 * nact_iactions_list_initial_load_toplevel:
 * @instance: this #NactIActionsList *instance.
 *
 * Allocates and initializes the ActionsList widget.
 *
 * GtkTreeView is created with NactTreeModel model
 * NactTreeModel
 *   implements EggTreeMultiDragSourceIface
 *   is derived from GtkTreeModelFilter
 *     GtkTreeModelFilter is built on top of GtkTreeStore
 *
 * Please note that management mode for the list should have been set
 * before calling this function.
 */
void
nact_iactions_list_initial_load_toplevel( NactIActionsList *instance )
{
	static const gchar *thisfn = "nact_iactions_list_initial_load_toplevel";
	GtkWidget *label;
	GtkTreeView *treeview;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	IActionsListInstanceData *ialid;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		ialid = nact_iactions_list_priv_get_instance_data( instance );
		ialid->selection_changed_allowed = FALSE;

		/* associates the ActionsList to the label */
		label = base_window_get_widget( BASE_WINDOW( instance ), "ActionsListLabel" );
		gtk_label_set_mnemonic_widget( GTK_LABEL( label ), GTK_WIDGET( treeview ));

		nact_tree_model_initial_load( BASE_WINDOW( instance ), treeview );
		gtk_tree_view_set_enable_tree_lines( treeview, TRUE );

		/* create visible columns on the tree view
		 */
		/* icon: no header */
		column = gtk_tree_view_column_new_with_attributes(
				"icon",
				gtk_cell_renderer_pixbuf_new(),
				"pixbuf", IACTIONS_LIST_ICON_COLUMN,
				NULL );
		gtk_tree_view_append_column( treeview, column );

		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
				"label",
				renderer,
				"text", IACTIONS_LIST_LABEL_COLUMN,
				NULL );
		gtk_tree_view_column_set_sort_column_id( column, IACTIONS_LIST_LABEL_COLUMN );
		gtk_tree_view_column_set_cell_data_func(
				column, renderer, ( GtkTreeCellDataFunc ) display_label, instance, NULL );
		gtk_tree_view_append_column( treeview, column );
	}
}

/**
 * nact_iactions_list_runtime_init_toplevel:
 * @window: this #NactIActionsList *instance.
 * @items: list of #NAObject actions and menus as provided by #NAPivot.
 *
 * Allocates and initializes the ActionsList widget.
 */
void
nact_iactions_list_runtime_init_toplevel( NactIActionsList *instance, GList *items )
{
	static const gchar *thisfn = "nact_iactions_list_runtime_init_toplevel";
	GtkTreeView *treeview;
	NactTreeModel *model;
	gboolean have_dnd;
	gboolean have_filter_selection;
	gboolean is_proxy;
	GtkTreeSelection *selection;
	IActionsListInstanceData *ialid;
	GtkTreeViewColumn *column;
	GList *renderers;

	g_debug( "%s: instance=%p, items=%p (%d items)",
			thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ));
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));

		ialid = nact_iactions_list_priv_get_instance_data( instance );
		ialid->selection_changed_allowed = FALSE;
		have_dnd = have_dnd_mode( instance, ialid );
		have_filter_selection = have_filter_selection_mode( instance, ialid );

		if( have_filter_selection ){
			selection = gtk_tree_view_get_selection( treeview );
			gtk_tree_selection_set_select_function( selection, ( GtkTreeSelectionFunc ) filter_selection, instance, NULL );
		}

		nact_tree_model_runtime_init( model, have_dnd );

		/* set up selection control */
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( gtk_tree_view_get_selection( treeview )),
				"changed",
				G_CALLBACK( nact_iactions_list_on_treeview_selection_changed ));

		/* catch press 'Enter' */
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( treeview ),
				"key-press-event",
				G_CALLBACK( on_key_pressed_event ));

		/* catch double-click */
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( treeview ),
				"button-press-event",
				G_CALLBACK( on_button_press_event ));

		/* updates the treeview display when an item is modified */
		ialid->tab_updated_handler = base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				TAB_UPDATABLE_SIGNAL_ITEM_UPDATED,
				G_CALLBACK( on_tab_updatable_item_updated ));

		/* enable/disable edit menu item accelerators depending of
		 * which widget has the focus */
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( treeview ),
				"focus-in-event",
				G_CALLBACK( on_focus_in ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( treeview ),
				"focus-out-event",
				G_CALLBACK( on_focus_out ));

		/* label edition: inform the corresponding tab */
		column = gtk_tree_view_get_column( treeview, IACTIONS_LIST_LABEL_COLUMN );
		renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( renderers->data ),
				"edited",
				G_CALLBACK( on_label_edited ));

		/* records NactIActionsList as a proxy for edition status
		 * modification */
		is_proxy = is_iduplicable_proxy( instance, ialid );
		if( is_proxy ){
			na_iduplicable_register_consumer( G_OBJECT( instance ));

			base_window_signal_connect(
					BASE_WINDOW( instance ),
					G_OBJECT( instance ),
					NA_IDUPLICABLE_SIGNAL_STATUS_CHANGED,
					G_CALLBACK( on_edition_status_changed ));
		}

		/* fill the model after having connected the signals
		 * so that callbacks are triggered at last
		 */
		nact_iactions_list_fill( instance, items );

		/* force the treeview to have the focus at start
		 */
		gtk_widget_grab_focus( GTK_WIDGET( treeview ));
	}
}

/**
 * nact_iactions_list_all_widgets_showed:
 * @window: this #NactIActionsList *instance.
 */
void
nact_iactions_list_all_widgets_showed( NactIActionsList *instance )
{
	static const gchar *thisfn = "nact_iactions_list_all_widgets_showed";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		nact_iactions_list_bis_select_first_row( instance );
	}
}

/**
 * nact_iactions_list_dispose:
 * @window: this #NactIActionsList *instance.
 */
void
nact_iactions_list_dispose( NactIActionsList *instance )
{
	static const gchar *thisfn = "nact_iactions_list_dispose";
	GtkTreeView *treeview;
	NactTreeModel *model;
	IActionsListInstanceData *ialid;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));

		ialid = nact_iactions_list_priv_get_instance_data( instance );
		g_list_free( ialid->modified_items );
		ialid->modified_items = NULL;

		ialid->selection_changed_allowed = FALSE;
		nact_tree_model_dispose( model );

		g_free( ialid );
	}
}

/**
 * nact_iactions_list_brief_tree_dump:
 * @instance: this #NactIActionsList implementation.
 *
 * Brief dump of the tree store content.
 */
void
nact_iactions_list_brief_tree_dump( NactIActionsList *instance )
{
	GtkTreeView *treeview;
	NactTreeModel *model;

	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));
		nact_tree_model_dump( model );
	}
}

/**
 * nact_iactions_list_collapse_all:
 * @instance: this #NactIActionsList implementation.
 *
 * Collapse all the tree hierarchy.
 */
void
nact_iactions_list_collapse_all( NactIActionsList *instance )
{
	static const gchar *thisfn = "nact_iactions_list_collapse_all";
	GtkTreeView *treeview;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		gtk_tree_view_collapse_all( treeview );
	}
}

/**
 * nact_iactions_list_display_order_change:
 * @instance: this #NactIActionsList implementation.
 * @order_mode: the new order mode.
 *
 * Setup the new order mode.
 */
void
nact_iactions_list_display_order_change( NactIActionsList *instance, gint order_mode )
{
	GtkTreeView *treeview;
	NactTreeModel *model;

	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));
		nact_tree_model_display_order_change( model, order_mode );
	}
}

/**
 * nact_iactions_list_expand_all:
 * @instance: this #NactIActionsList implementation.
 *
 * Expand all the tree hierarchy.
 */
void
nact_iactions_list_expand_all( NactIActionsList *instance )
{
	static const gchar *thisfn = "nact_iactions_list_expand_all";
	GtkTreeView *treeview;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		gtk_tree_view_expand_all( treeview );
	}
}

/**
 * nact_iactions_list_fill:
 * @instance: this #NactIActionsList instance.
 *
 * Fill the listbox with the provided list of items.
 *
 * Menus are expanded, profiles are not.
 * The selection is reset to the first line of the tree, if there is one.
 */
void
nact_iactions_list_fill( NactIActionsList *instance, GList *items )
{
	static const gchar *thisfn = "nact_iactions_list_fill";
	GtkTreeView *treeview;
	NactTreeModel *model;
	gboolean profiles_are_displayed;
	IActionsListInstanceData *ialid;

	g_debug( "%s: instance=%p, items=%p", thisfn, ( void * ) instance, ( void * ) items );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));

		nact_iactions_list_bis_clear_selection( instance, treeview );

		ialid = nact_iactions_list_priv_get_instance_data( instance );
		profiles_are_displayed = are_profiles_displayed( instance, ialid );

		ialid->selection_changed_allowed = FALSE;
		nact_tree_model_fill( model, items, profiles_are_displayed );
		ialid->selection_changed_allowed = TRUE;

		g_list_free( ialid->modified_items );
		ialid->modified_items = NULL;

		g_signal_emit_by_name(
				instance,
				MAIN_WINDOW_SIGNAL_LEVEL_ZERO_ORDER_CHANGED,
				GINT_TO_POINTER( FALSE ));

		ialid->menus = 0;
		ialid->actions = 0;
		ialid->profiles = 0;

		if( profiles_are_displayed ){
			na_object_item_count_items( items, &ialid->menus, &ialid->actions, &ialid->profiles, TRUE );
			nact_iactions_list_priv_send_list_count_updated_signal( instance, ialid );
		}
	}
}

/**
 * nact_iactions_list_get_management_mode:
 * @instance: this #NactIActionsList instance.
 *
 * Returns: the current management mode of the list.
 */
gint
nact_iactions_list_get_management_mode( NactIActionsList *instance )
{
	gint mode = 0;
	IActionsListInstanceData *ialid;

	g_return_val_if_fail( NACT_IS_IACTIONS_LIST( instance ), 0 );

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		ialid = nact_iactions_list_priv_get_instance_data( instance );
		mode = ialid->management_mode;
	}

	return( mode );
}

/**
 * nact_iactions_list_has_modified_items:
 * @window: this #NactIActionsList instance.
 *
 * Returns: %TRUE if at least there is one modified item in the list.
 */
gboolean
nact_iactions_list_has_modified_items( NactIActionsList *instance )
{
	gboolean has_modified = FALSE;
	/*GtkTreeView *treeview;
	NactTreeModel *model;*/
	IActionsListInstanceData *ialid;

	g_return_val_if_fail( NACT_IS_IACTIONS_LIST( instance ), FALSE );

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		ialid = nact_iactions_list_priv_get_instance_data( instance );
		has_modified = ( g_list_length( ialid->modified_items ) > 0 );

		/*treeview = get_actions_list_treeview( instance );
		model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));
		nact_tree_model_iter( model, ( FnIterOnStore ) has_modified_iter, &has_modified );*/
	}

	return( has_modified );
}

/**
 * nact_iactions_list_on_treeview_selection_changed:
 * @selection: current selection.
 * @instance: this instance of the #NactIActionsList interface.
 *
 * This is our handler for "changed" signal emitted by the treeview.
 * The handler is inhibited while filling the list (usually only at
 * runtime init), and while deleting a selection.
 */
void
nact_iactions_list_on_treeview_selection_changed( GtkTreeSelection *selection, NactIActionsList *instance )
{
	GList *selected_items;
	IActionsListInstanceData *ialid;

	ialid = nact_iactions_list_priv_get_instance_data( instance );
	if( ialid->selection_changed_allowed ){

		g_debug( "nact_iactions_list_on_treeview_selection_changed" );
		g_signal_handler_block( instance, ialid->tab_updated_handler );

		selected_items = nact_iactions_list_bis_get_selected_items( instance );
		g_debug( "nact_iactions_list_on_treeview_selection_changed: selection=%p (%d items)", ( void * ) selected_items, g_list_length( selected_items ));
		g_signal_emit_by_name( instance, IACTIONS_LIST_SIGNAL_SELECTION_CHANGED, selected_items );

		g_signal_handler_unblock( instance, ialid->tab_updated_handler );

		/* selection list is freed in cleanup handler for the signal */
	}
}

/**
 * nact_iactions_list_remove_rec:
 * @list: list of modified objects.
 * @object: the object to be removed from the list.
 *
 * When removing from modified list an object which is no more modified,
 * then all subitems of the object have also to be removed
 *
 * Returns: the updated list.
 */
GList *
nact_iactions_list_remove_rec( GList *list, NAObject *object )
{
	GList *subitems, *it;

	if( NA_IS_OBJECT_ITEM( object )){
		subitems = na_object_get_items( object );
		for( it = subitems ; it ; it = it->next ){
			list = nact_iactions_list_remove_rec( list, it->data );
		}
	}

	list = g_list_remove( list, object );

	return( list );
}

/**
 * nact_iactions_list_set_management_mode:
 * @instance: this #NactIActionsList instance.
 * @mode: management mode.
 *
 * Set the management mode for this @instance.
 *
 * For the two known modes (edition mode, export mode), we also allow
 * multiple selection in the list.
 */
void
nact_iactions_list_set_management_mode( NactIActionsList *instance, gint mode )
{
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	gboolean multiple;
	IActionsListInstanceData *ialid;

	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		ialid = nact_iactions_list_priv_get_instance_data( instance );
		ialid->management_mode = mode;

		multiple = ( mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION ||
						mode == IACTIONS_LIST_MANAGEMENT_MODE_EXPORT );

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		selection = gtk_tree_view_get_selection( treeview );
		gtk_tree_selection_set_mode( selection, multiple ? GTK_SELECTION_MULTIPLE : GTK_SELECTION_SINGLE );
	}
}

static gboolean
are_profiles_displayed( NactIActionsList *instance, IActionsListInstanceData *ialid )
{
	gboolean display;

	display = ( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION );

	return( display );
}

/*
 * item modified: italic
 * item not saveable (invalid): red
 */
static void
display_label( GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, NactIActionsList *instance )
{
	NAObject *object;
	gchar *label;
	gboolean modified = FALSE;
	gboolean valid = TRUE;
	IActionsListInstanceData *ialid;
	NAObjectItem *item;
	gboolean writable_item;

	gtk_tree_model_get( model, iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );
	g_object_unref( object );
	g_return_if_fail( NA_IS_OBJECT( object ));

	ialid = nact_iactions_list_priv_get_instance_data( instance );
	label = na_object_get_label( object );
	g_object_set( cell, "style-set", FALSE, NULL );
	g_object_set( cell, "foreground-set", FALSE, NULL );
	/*g_debug( "nact_iactions_list_display_label: %s %s", G_OBJECT_TYPE_NAME( object ), label );*/

	if( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION ){

		modified = na_object_is_modified( object );
		valid = na_object_is_valid( object );
		item = NA_IS_OBJECT_PROFILE( object ) ? na_object_get_parent( object ) : NA_OBJECT_ITEM( object );
		writable_item = nact_window_is_item_writable( NACT_WINDOW( instance ), item, NULL );

		if( modified ){
			g_object_set( cell, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL );
		}

		if( !valid ){
			g_object_set( cell, "foreground", "Red", "foreground-set", TRUE, NULL );
		}

		g_object_set( cell, "editable", writable_item, NULL );
	}

	g_object_set( cell, "text", label, NULL );
	g_free( label );
}

/*
 * rationale: it is very difficult to copy anything in the clipboard,
 * and to hope that this will be easily copyable anywhere after.
 * We know how to insert profiles, or how to insert actions or menus,
 * but not how nor where to insert e.g. a mix selection.
 *
 * So a selection must first be homogeneous, i.e. it only contains
 * explicitely selected profiles _or_ menus or actions (and their childs).
 *
 * To simplify the selection management while letting the user actually
 * select almost anything, we are doing following assumptions:
 * - when the user selects one row, all childs are also automatically
 *   selected ; visible childs are setup so that they are known as
 *   'indirectly' selected
 * - when a row is set as 'indirectly' selected, user cannot select
 *   nor unselect it (sort of readonly or mandatory implied selection)
 *   while the parent stays itself selected
 */
static gboolean
filter_selection( GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, NactIActionsList *instance )
{
	static const gchar *thisfn = "nact_iactions_list_filter_selection";
	GList *selected_paths;
	GtkTreeIter iter;
	NAObject *object;

	gtk_tree_model_get_iter( model, &iter, path );
	gtk_tree_model_get( model, &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );
	g_return_val_if_fail( object, FALSE );
	g_return_val_if_fail( NA_IS_OBJECT_ID( object ), FALSE );
	g_object_unref( object );

	/* if there is not yet any selection, then anything is allowed
	 */
	selected_paths = gtk_tree_selection_get_selected_rows( selection, NULL );
	if( !selected_paths || !g_list_length( selected_paths )){
		/*g_debug( "%s: current selection is empty: allowing this one", thisfn );*/
		filter_selection_set_implicitely_selected_childs( object, !path_currently_selected );
		return( TRUE );
	}

	/* if the object at the row is already 'implicitely' selected, i.e.
	 * selected because of the selection of one of its parents, then
	 * nothing is allowed
	 */
	if( filter_selection_is_implicitely_selected( object )){
		g_debug( "%s: implicitely selected item: selection not allowed", thisfn );
		return( FALSE );
	}

	/* object at the row is not 'implicitely' selected: we may so select
	 * or unselect it while the selection stays homogeneous
	 * (rather we set its childs to the corresponding implied status)
	 */
	if( path_currently_selected ||
		filter_selection_is_homogeneous( selection, object )){

			filter_selection_set_implicitely_selected_childs( object, !path_currently_selected );
	}
	return( TRUE );
}

/*
 * does the selection stay homogeneous when adding this object ?
 */
static gboolean
filter_selection_is_homogeneous( GtkTreeSelection *selection, NAObject *object )
{
	gboolean homogeneous;

	if( filter_selection_has_menu_or_action( selection )){
		homogeneous = !NA_IS_OBJECT_PROFILE( object );
	} else {
		homogeneous = NA_IS_OBJECT_PROFILE( object );
	}

	return( homogeneous );
}

static gboolean
filter_selection_has_menu_or_action( GtkTreeSelection *selection )
{
	gboolean has_menu_or_action;
	SelectionIter *str;

	has_menu_or_action = FALSE;
	str = g_new0( SelectionIter, 1 );
	str->has_menu_or_action = has_menu_or_action;
	gtk_tree_selection_selected_foreach( selection, ( GtkTreeSelectionForeachFunc ) filter_selection_iter, str );
	has_menu_or_action = str->has_menu_or_action;
	g_free( str );

	return( has_menu_or_action );
}

static void
filter_selection_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, SelectionIter *str )
{
	NAObject *object;

	gtk_tree_model_get( model, iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );

	if( NA_IS_OBJECT_ITEM( object )){
		str->has_menu_or_action = TRUE;
	}

	g_object_unref( object );
}

static gboolean
filter_selection_is_implicitely_selected( NAObject *object )
{
	gboolean selected;

	selected = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( object), "nact-implicit-selection" ));

	return( selected );
}

/*
 * the object is being selected (resp. unselected)
 * recursively set the 'implicit selection' flag for all its childs
 */
static void
filter_selection_set_implicitely_selected_childs( NAObject *object, gboolean select )
{
	GList *childs, *ic;

	if( NA_IS_OBJECT_ITEM( object )){
		childs = na_object_get_items( object );
		for( ic = childs ; ic ; ic = ic->next ){
			g_object_set_data( G_OBJECT( ic->data ), "nact-implicit-selection", GUINT_TO_POINTER(( guint ) select ));
			filter_selection_set_implicitely_selected_childs( NA_OBJECT( ic->data ), select );
		}
	}
}

static gboolean
have_dnd_mode( NactIActionsList *instance, IActionsListInstanceData *ialid )
{
	gboolean have_dnd;

	have_dnd = ( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION );

	return( have_dnd );
}

static gboolean
have_filter_selection_mode( NactIActionsList *instance, IActionsListInstanceData *ialid )
{
	gboolean have_filter;

	have_filter = ( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION );

	return( have_filter );
}

/*
 * triggered by 'F2' key
 * only in edition mode
 */
static void
inline_edition( NactIActionsList *instance )
{
	static const gchar *thisfn = "nact_iactions_list_inline_edition";
	IActionsListInstanceData *ialid;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	GList *listrows;
	GtkTreePath *path;
	GtkTreeViewColumn *column;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));

	ialid = nact_iactions_list_priv_get_instance_data( instance );
	if( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION ){

		ialid->selection_changed_allowed = FALSE;

		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		selection = gtk_tree_view_get_selection( treeview );
		listrows = gtk_tree_selection_get_selected_rows( selection, NULL );

		if( g_list_length( listrows ) == 1 ){
			path = ( GtkTreePath * ) listrows->data;
			column = gtk_tree_view_get_column( treeview, IACTIONS_LIST_LABEL_COLUMN );
			gtk_tree_view_set_cursor( treeview, path, column, TRUE );
		}

		g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
		g_list_free( listrows );

		ialid->selection_changed_allowed = TRUE;
	}
}

static gboolean
is_iduplicable_proxy( NactIActionsList *instance, IActionsListInstanceData *ialid )
{
	gboolean is_proxy;

	is_proxy = ( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION );
	g_debug( "nact_iactions_list_is_iduplicable_proxy: is_proxy=%s", is_proxy ? "True":"False" );

	return( is_proxy );
}

static gboolean
on_button_press_event( GtkWidget *widget, GdkEventButton *event, NactIActionsList *instance )
{
	/*static const gchar *thisfn = "nact_iactions_list_v_on_button_pres_event";
	g_debug( "%s: widget=%p, event=%p, user_data=%p", thisfn, widget, event, user_data );*/

	gboolean stop = FALSE;

	/* double-click of left button */
	if( event->type == GDK_2BUTTON_PRESS && event->button == 1 ){
		nact_iactions_list_bis_toggle_collapse( instance );
		stop = TRUE;
	}

	/* single click on right button */
	if( event->type == GDK_BUTTON_PRESS && event->button == 3 ){
		open_popup( instance, event );
		stop = TRUE;
	}

	return( stop );
}

static void
on_edition_status_changed( NactIActionsList *instance, NAIDuplicable *object )
{
	GtkTreeView *treeview;
	NactTreeModel *model;
	IActionsListInstanceData *ialid;

	ialid = nact_iactions_list_priv_get_instance_data( instance );

	g_debug( "nact_iactions_list_on_edition_status_changed: instance=%p, object=%p (%s)",
			( void * ) instance,
			( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_if_fail( NA_IS_OBJECT( object ));

	treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
	model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));
	nact_tree_model_display( model, NA_OBJECT( object ));

	if( na_object_is_modified( object )){
		if( !g_list_find( ialid->modified_items, object )){
			ialid->modified_items = g_list_prepend( ialid->modified_items, object );
		}
	} else {
		ialid->modified_items = nact_iactions_list_remove_rec( ialid->modified_items, NA_OBJECT( object ));
	}

	/* do not send status-changed signal while filling the tree
	 */
	if( ialid->selection_changed_allowed ){
		g_signal_emit_by_name( instance, IACTIONS_LIST_SIGNAL_STATUS_CHANGED, NULL );
	}
}

/*
 * focus is monitored to avoid an accelerator being pressed while on a tab
 * triggers an unwaited operation on the list
 * e.g. when editing an entry field on the tab, pressing Del should _not_
 * delete current row in the list !
 */
static gboolean
on_focus_in( GtkWidget *widget, GdkEventFocus *event, NactIActionsList *instance )
{
	/*static const gchar *thisfn = "nact_iactions_list_on_focus_in";*/
	gboolean stop = FALSE;

	/*g_debug( "%s: widget=%p, event=%p, instance=%p", thisfn, ( void * ) widget, ( void * ) event, ( void * ) instance );*/
	g_signal_emit_by_name( instance, IACTIONS_LIST_SIGNAL_FOCUS_IN, instance );

	return( stop );
}

static gboolean
on_focus_out( GtkWidget *widget, GdkEventFocus *event, NactIActionsList *instance )
{
	/*static const gchar *thisfn = "nact_iactions_list_on_focus_out";*/
	gboolean stop = FALSE;

	/*g_debug( "%s: widget=%p, event=%p, instance=%p", thisfn, ( void * ) widget, ( void * ) event, ( void * ) instance );*/
	g_signal_emit_by_name( instance, IACTIONS_LIST_SIGNAL_FOCUS_OUT, instance );

	return( stop );
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, NactIActionsList *instance )
{
	/*static const gchar *thisfn = "nact_iactions_list_v_on_key_pressed_event";
	g_debug( "%s: widget=%p, event=%p, user_data=%p", thisfn, widget, event, user_data );*/
	gboolean stop = FALSE;

	if( event->keyval == GDK_Return || event->keyval == GDK_KP_Enter ){
		nact_iactions_list_bis_toggle_collapse( instance );
		stop = TRUE;
	}

	if( event->keyval == GDK_F2 ){
		inline_edition( instance );
		stop = TRUE;
	}

	if( event->keyval == GDK_Right ){
		nact_iactions_list_bis_expand_to_first_child( instance );
		stop = TRUE;
	}

	if( event->keyval == GDK_Left ){
		nact_iactions_list_bis_collapse_to_parent( instance );
		stop = TRUE;
	}

	return( stop );
}

/*
 * path: path of the edited row, as a string
 * text: new text
 *
 * - inform tabs so that they can update their fields
 *   data = object_at_row + new_label
 *   this will trigger set the object content, and other updates
 */
static void
on_label_edited( GtkCellRendererText *renderer, const gchar *path_str, const gchar *text, NactIActionsList *instance )
{
	GtkTreeView *treeview;
	NactTreeModel *model;
	NAObject *object;
	GtkTreePath *path;
	gchar *new_text;

	treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
	model = NACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));
	path = gtk_tree_path_new_from_string( path_str );
	object = nact_tree_model_object_at_path( model, path );
	new_text = g_strdup( text );

	g_signal_emit_by_name( instance, IACTIONS_LIST_SIGNAL_COLUMN_EDITED, object, new_text, IACTIONS_LIST_LABEL_COLUMN );
}

/*
 * an item has been updated in one of the tabs
 * update the treeview to reflects its new edition status
 */
static void
on_tab_updatable_item_updated( NactIActionsList *instance, NAObject *object, gboolean force_display )
{
	static const gchar *thisfn = "nact_iactions_list_on_tab_updatable_item_updated";
	GtkTreeView *treeview;
	GtkTreeModel *model;

	g_debug( "%s: instance=%p, object=%p (%s), force_display=%s", thisfn,
			( void * ) instance, ( void * ) object, G_OBJECT_TYPE_NAME( object ),
			force_display ? "True":"False" );
	g_return_if_fail( NACT_IS_IACTIONS_LIST( instance ));
	g_return_if_fail( NA_IS_OBJECT( object ));
	g_return_if_fail( NA_IS_IDUPLICABLE( object ));

	if( object ){
		treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
		model = gtk_tree_view_get_model( treeview );
		if( !na_object_check_status_up( object ) && force_display ){
			on_edition_status_changed( instance, NA_IDUPLICABLE( object ));
		}
	}
}

static void
open_popup( NactIActionsList *instance, GdkEventButton *event )
{
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreePath *path;

	treeview = nact_iactions_list_priv_get_actions_list_treeview( instance );
	if( gtk_tree_view_get_path_at_pos( treeview, event->x, event->y, &path, NULL, NULL, NULL )){
		model = gtk_tree_view_get_model( treeview );
		nact_iactions_list_bis_select_row_at_path( instance, treeview, model, path );
		gtk_tree_path_free( path );
		nact_main_menubar_open_popup( NACT_MAIN_WINDOW( instance ), event );
	}
}
