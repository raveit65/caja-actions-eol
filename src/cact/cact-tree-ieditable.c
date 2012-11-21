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

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-factory-object.h>
#include <core/na-updater.h>

#include "base-keysyms.h"
#include "cact-application.h"
#include "cact-main-window.h"
#include "cact-tree-ieditable.h"
#include "cact-tree-model.h"
#include "cact-tree-view.h"

/* private interface data
 */
struct _CactTreeIEditableInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* data attached to the CactTreeView
 */
typedef struct {
	NAUpdater     *updater;
	BaseWindow    *window;
	GtkTreeView   *treeview;
	CactTreeModel *model;
	gulong         modified_handler_id;
	gulong         valid_handler_id;
	guint          count_modified;
	gboolean       level_zero_changed;
	GList         *deleted;
	guint          count_deleted;
}
	IEditableData;

#define VIEW_DATA_IEDITABLE				"view-data-ieditable"

static guint st_initializations = 0;	/* interface initialization count */

static GType          register_type( void );
static void           interface_base_init( CactTreeIEditableInterface *klass );
static void           interface_base_finalize( CactTreeIEditableInterface *klass );

static gboolean       on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseWindow *window );
static void           on_label_edited( GtkCellRendererText *renderer, const gchar *path_str, const gchar *text, BaseWindow *window );
static void           on_main_selection_changed( BaseWindow *window, GList *selected_items, gpointer user_data );
static void           on_object_modified_status_changed( CactTreeIEditable *view, NAObject *object, gboolean new_status, BaseWindow *window );
static void           on_object_valid_status_changed( CactTreeIEditable *view, NAObject *object, gboolean new_status, BaseWindow *window );
static void           on_tree_view_level_zero_changed( BaseWindow *window, gboolean is_modified, gpointer user_data );
static void           on_tree_view_modified_status_changed( BaseWindow *window, gboolean is_modified, gpointer user_data );
static void           add_to_deleted_rec( IEditableData *ied, NAObject *object );
static void           decrement_counters( CactTreeIEditable *view, IEditableData *ialid, GList *items );
static GtkTreePath   *do_insert_before( IEditableData *ied, GList *items, GtkTreePath *insert_path );
static GtkTreePath   *do_insert_into( IEditableData *ied, GList *items, GtkTreePath *insert_path );
static void           increment_counters( CactTreeIEditable *view, IEditableData *ied, GList *items );
static void           check_level_zero_status( CactTreeIEditable *instance );
static gchar         *get_items_id_list_str( GList *items_list );
static GtkTreePath   *get_selection_first_path( GtkTreeView *treeview );
static gboolean       get_modification_status( IEditableData *ied );
static IEditableData *get_instance_data( CactTreeIEditable *view );
static void           inline_edition( BaseWindow *window );

GType
cact_tree_ieditable_get_type( void )
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
	static const gchar *thisfn = "cact_tree_ieditable_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactTreeIEditableInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "CactTreeIEditable", &info, 0 );

	g_type_interface_add_prerequisite( type, CACT_TYPE_TREE_VIEW );

	return( type );
}

static void
interface_base_init( CactTreeIEditableInterface *klass )
{
	static const gchar *thisfn = "cact_tree_ieditable_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactTreeIEditableInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( CactTreeIEditableInterface *klass )
{
	static const gchar *thisfn = "cact_tree_ieditable_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/**
 * cact_tree_ieditable_initialize:
 * @instance: the #CactTreeView which implements this interface.
 * @treeview: the embedded #GtkTreeView tree view.
 * @window: the #BaseWindow which embeds the @instance.
 *
 * Initialize the interface, mainly connecting to signals of interest.
 */
void
cact_tree_ieditable_initialize( CactTreeIEditable *instance, GtkTreeView *treeview, BaseWindow *window )
{
	static const gchar *thisfn = "cact_tree_ieditable_initialize";
	IEditableData *ied;
	CactApplication *application;
	GtkTreeViewColumn *column;
	GList *renderers;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));

	g_debug( "%s: instance=%p (%s), treeview=%p, window=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) treeview, ( void * ) window );

	ied = get_instance_data( instance );
	ied->window = window;
	ied->treeview = treeview;
	ied->model = CACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));

	application = CACT_APPLICATION( base_window_get_application( window ));
	ied->updater = cact_application_get_updater( application );

	/* inline label edition with F2 */
	base_window_signal_connect(
			window,
			G_OBJECT( treeview ),
			"key-press-event",
			G_CALLBACK( on_key_pressed_event ));

	/* label edition: inform the corresponding tab */
	column = gtk_tree_view_get_column( treeview, TREE_COLUMN_LABEL );
	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
	base_window_signal_connect(
			window,
			G_OBJECT( renderers->data ),
			"edited",
			G_CALLBACK( on_label_edited ));

	/* monitors status changes to refresh the display */
	na_iduplicable_register_consumer( G_OBJECT( instance ));
	ied->modified_handler_id = base_window_signal_connect(
			window,
			G_OBJECT( instance ),
			IDUPLICABLE_SIGNAL_MODIFIED_CHANGED,
			G_CALLBACK( on_object_modified_status_changed ));
	ied->valid_handler_id = base_window_signal_connect(
			window,
			G_OBJECT( instance ),
			IDUPLICABLE_SIGNAL_VALID_CHANGED,
			G_CALLBACK( on_object_valid_status_changed ));

	/* monitors the reloading of the tree */
	base_window_signal_connect(
			window,
			G_OBJECT( window ),
			TREE_SIGNAL_MODIFIED_STATUS_CHANGED,
			G_CALLBACK( on_tree_view_modified_status_changed ));

	/* monitors the level zero of the tree */
	base_window_signal_connect(
			window,
			G_OBJECT( window ),
			TREE_SIGNAL_LEVEL_ZERO_CHANGED,
			G_CALLBACK( on_tree_view_level_zero_changed ));

	/* monitors the main selection */
	base_window_signal_connect(
			window,
			G_OBJECT( window ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));
}

/**
 * cact_tree_ieditable_terminate:
 * @instance: the #CactTreeView which implements this interface.
 *
 * Free the data when application quits.
 */
void
cact_tree_ieditable_terminate( CactTreeIEditable *instance )
{
	static const gchar *thisfn = "cact_tree_ieditable_terminate";
	IEditableData *ied;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	ied = get_instance_data( instance );

	na_object_free_items( ied->deleted );

	base_window_signal_disconnect( ied->window, ied->modified_handler_id );
	base_window_signal_disconnect( ied->window, ied->valid_handler_id );

	g_free( ied );
	g_object_set_data( G_OBJECT( instance ), VIEW_DATA_IEDITABLE, NULL );
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseWindow *window )
{
	gboolean stop = FALSE;

	if( event->keyval == CACT_KEY_F2 ){
		inline_edition( window );
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
 */
static void
on_label_edited( GtkCellRendererText *renderer, const gchar *path_str, const gchar *text, BaseWindow *window )
{
	CactTreeView *items_view;
	IEditableData *ied;
	NAObject *object;
	GtkTreePath *path;

	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));

	if( cact_tree_view_are_notify_allowed( items_view )){
		ied = ( IEditableData * ) g_object_get_data( G_OBJECT( items_view ), VIEW_DATA_IEDITABLE );
		path = gtk_tree_path_new_from_string( path_str );
		object = cact_tree_model_object_at_path( ied->model, path );
		na_object_set_label( object, text );

		g_signal_emit_by_name( window, MAIN_SIGNAL_ITEM_UPDATED, object );
	}
}

static void
on_main_selection_changed( BaseWindow *window, GList *selected_items, gpointer user_data )
{
	NAObject *object;
	gboolean object_editable;
	CactTreeView *items_view;
	IEditableData *ied;
	GtkTreeViewColumn *column;
	GList *renderers;
	gboolean editable;

	g_object_get( window, MAIN_PROP_ITEM, &object, MAIN_PROP_EDITABLE, &object_editable, NULL );
	editable = NA_IS_OBJECT( object ) && object_editable;
	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));
	ied = ( IEditableData * ) g_object_get_data( G_OBJECT( items_view ), VIEW_DATA_IEDITABLE );
	column = gtk_tree_view_get_column( ied->treeview, TREE_COLUMN_LABEL );
	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
	g_object_set( G_OBJECT( renderers->data ), "editable", editable, "editable-set", TRUE, NULL );
}

/*
 * modification status of an edited object has changed
 * - refresh the display
 */
static void
on_object_modified_status_changed( CactTreeIEditable *instance, NAObject *object, gboolean is_modified, BaseWindow *window )
{
	static const gchar *thisfn = "cact_tree_ieditable_on_object_modified_status_changed";
	IEditableData *ied;
	gboolean prev_status, status;

	g_debug( "%s: instance=%p, object=%p (%s), is_modified=%s, window=%p",
			thisfn, ( void * ) instance,
			( void * ) object, G_OBJECT_TYPE_NAME( object ), is_modified ? "True":"False",
			( void * ) window );

	ied = ( IEditableData * ) g_object_get_data( G_OBJECT( instance ), VIEW_DATA_IEDITABLE );
	prev_status = get_modification_status( ied );
	gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( ied->model ));

	if( NA_IS_OBJECT_ITEM( object )){
		if( is_modified ){
			ied->count_modified += 1;
		} else {
			ied->count_modified -= 1;
		}
	}

	if( cact_tree_view_are_notify_allowed( CACT_TREE_VIEW( instance ))){
		status = get_modification_status( ied );
		if( status != prev_status ){
			g_signal_emit_by_name( window, TREE_SIGNAL_MODIFIED_STATUS_CHANGED, status );
		}
	}
}

/*
 * validity status of the edited object has changed
 * - refresh the display
 */
static void
on_object_valid_status_changed( CactTreeIEditable *instance, NAObject *object, gboolean new_status, BaseWindow *window )
{
	static const gchar *thisfn = "cact_tree_ieditable_on_object_valid_status_changed";
	IEditableData *ied;

	g_debug( "%s: instance=%p, new_status=%s, window=%p",
			thisfn, ( void * ) instance, new_status ? "True":"False", ( void * ) window );

	ied = ( IEditableData * ) g_object_get_data( G_OBJECT( instance ), VIEW_DATA_IEDITABLE );
	gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( ied->model ));
}

/*
 * order or list of items at level zero of the tree has changed
 */
static void
on_tree_view_level_zero_changed( BaseWindow *window, gboolean is_modified, gpointer user_data )
{
	CactTreeView *items_view;
	IEditableData *ied;
	gboolean prev_status, status;

	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));
	ied = get_instance_data( CACT_TREE_IEDITABLE( items_view ));
	prev_status = get_modification_status( ied );
	ied->level_zero_changed = is_modified;

	if( cact_tree_view_are_notify_allowed( items_view )){
		status = get_modification_status( ied );
		if( prev_status != status ){
			g_signal_emit_by_name( window, TREE_SIGNAL_MODIFIED_STATUS_CHANGED, status );
		}
	}
}

/*
 * the tree has been reloaded
 */
static void
on_tree_view_modified_status_changed( BaseWindow *window, gboolean is_modified, gpointer user_data )
{
	CactTreeView *items_view;
	IEditableData *ied;

	if( !is_modified ){
		items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));
		ied = get_instance_data( CACT_TREE_IEDITABLE( items_view ));
		ied->count_modified = 0;
		ied->deleted = na_object_free_items( ied->deleted );
		ied->level_zero_changed = FALSE;
	}
}

/**
 * cact_tree_ieditable_delete:
 * @instance: this #CactTreeIEditable instance.
 * @list: list of #NAObject to be deleted.
 * @ope: whether the objects are actually to be deleted, or just moved
 *  to another path of the tree.
 *
 * Deletes the specified list from the underlying tree store.
 *
 * If the items are to be actually deleted, then this function takes care
 * of repositionning a new selection if possible, and refilter the display
 * model.
 * Deleted NAObjectItems are added to the 'deleted' list attached to the
 * view, so that they will be actually deleted from the storage subsystem
 * on save.
 * Deleted NAObjectProfiles are not added to the list, because the deletion
 * will be recorded when saving the action.
 *
 * If the items are to be actually moved to another path of the tree, then
 * neither the 'deleted' list nor the 'count_modified' counter are modified.
 */
void
cact_tree_ieditable_delete( CactTreeIEditable *instance, GList *items, TreeIEditableDeleteOpe ope )
{
	static const gchar *thisfn = "cact_tree_ieditable_delete";
	IEditableData *ied;
	gboolean prev_status, status;
	GtkTreePath *path;
	GList *it;
	NAObjectItem *parent;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));

	g_debug( "%s: instance=%p, items=%p (count=%d), ope=%u",
			thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ), ope );

	cact_tree_view_set_notify_allowed( CACT_TREE_VIEW( instance ), FALSE );
	ied = get_instance_data( instance );
	prev_status = get_modification_status( ied );

	decrement_counters( instance, ied, items );
	path = NULL;

	for( it = items ; it ; it = it->next ){
		if( path ){
			gtk_tree_path_free( path );
		}

		/* check the status of the parent of the deleted object
		 * so potentially update the modification status
		 */
		parent = na_object_get_parent( it->data );
		path = cact_tree_model_delete( ied->model, NA_OBJECT( it->data ));
		if( parent ){
			na_object_check_status( parent );
		} else {
			check_level_zero_status( instance );
		}

		/* record the deleted item in the 'deleted' list,
		 * incrementing the 'modified' count if the deleted item was not yet modified
		 */
		if( ope == TREE_OPE_DELETE ){
			add_to_deleted_rec( ied, NA_OBJECT( it->data ));
		}

		g_debug( "%s: object=%p (%s, ref_count=%d)", thisfn,
				( void * ) it->data, G_OBJECT_TYPE_NAME( it->data ), G_OBJECT( it->data )->ref_count );
	}

	gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( ied->model ));

	cact_tree_view_set_notify_allowed( CACT_TREE_VIEW( instance ), TRUE );

	if( path ){
		if( ope == TREE_OPE_DELETE ){
			cact_tree_view_select_row_at_path( CACT_TREE_VIEW( instance ), path );
		}
		gtk_tree_path_free( path );
	}

	status = get_modification_status( ied );
	if( status != prev_status ){
		g_signal_emit_by_name( G_OBJECT( ied->window ), TREE_SIGNAL_MODIFIED_STATUS_CHANGED, status );

	}
}

/**
 * add_to_deleted_rec:
 * @list: list of deleted objects.
 * @object: the object to be added from the list.
 *
 * Recursively adds to the 'deleted' list the NAObjectItem object currently being deleted.
 *
 * Increments the 'modified' count with the deleted items which were not modified.
 *
 * Only increments the 'deleted' count for items we may have to actually remove
 * from the backend, i.e. those who have a non-null provider.
 */
static void
add_to_deleted_rec( IEditableData *ied, NAObject *object )
{
	GList *subitems, *it;

	if( NA_IS_OBJECT_ITEM( object )){

		if( !g_list_find( ied->deleted, object )){
			ied->deleted = g_list_prepend( ied->deleted, object );

			if( na_object_is_modified( object )){
				ied->count_modified -= 1;
			}

			if( na_object_get_provider( object )){
				ied->count_deleted += 1;
			}
		}

		if( NA_IS_OBJECT_MENU( object )){
			subitems = na_object_get_items( object );
			for( it = subitems ; it ; it = it->next ){
				add_to_deleted_rec( ied, it->data );
			}
		}
	}
}

/*
 * update the total count of elements
 */
static void
decrement_counters( CactTreeIEditable *view, IEditableData *ied, GList *items )
{
	static const gchar *thisfn = "cact_tree_editable_decrement_counters";
	gint menus, actions, profiles;

	g_debug( "%s: view=%p, ied=%p, items=%p (count=%u)",
			thisfn, ( void * ) view, ( void * ) ied, ( void * ) items, g_list_length( items ));

	na_object_count_items( items, &menus, &actions, &profiles );
	menus *= -1;
	actions *= -1;
	profiles *= -1;
	g_signal_emit_by_name( G_OBJECT( ied->window ), TREE_SIGNAL_COUNT_CHANGED, FALSE, menus, actions, profiles );
}

/**
 * cact_tree_ieditable_remove_deleted:
 * @instance: this #CactTreeIEditable *instance.
 * @messages: a pointer to a #GSList of error messages.
 *
 * Actually removes the deleted items from the underlying I/O storage subsystem.
 *
 * @messages #GSList is only filled up in case of an error has occured.
 * If there is no error (cact_tree_ieditable_remove_deleted() returns %TRUE),
 * then the caller may safely assume that @messages is returned in the
 * same state that it has been provided.
 *
 * Returns: %TRUE if all candidate items have been successfully deleted,
 * %FALSE else.
 */
gboolean
cact_tree_ieditable_remove_deleted( CactTreeIEditable *instance, GSList **messages )
{
	static const gchar *thisfn = "cact_tree_ieditable_remove_deleted";
	gboolean delete_ok;
	IEditableData *ied;
	GList *it;
	NAObjectItem *item;
	GList *not_deleted;

	g_return_val_if_fail( CACT_IS_TREE_IEDITABLE( instance ), TRUE );

	delete_ok = TRUE;
	ied = get_instance_data( instance );
	not_deleted = NULL;

	for( it = ied->deleted ; it ; it = it->next ){
		item = NA_OBJECT_ITEM( it->data );
		g_debug( "%s: item=%p (%s)", thisfn, ( void * ) item, G_OBJECT_TYPE_NAME( item ));
		na_object_dump_norec( item );

		if( na_updater_delete_item( ied->updater, item, messages ) != NA_IIO_PROVIDER_CODE_OK ){
			not_deleted = g_list_prepend( not_deleted, na_object_ref( item ));
			delete_ok = FALSE;
		}
	}

	ied->deleted = na_object_free_items( ied->deleted );

	/* items that we cannot delete are reinserted in the tree view
	 * in the state they were when they were deleted
	 * (i.e. possibly modified)
	 */
	if( not_deleted ){
		cact_tree_ieditable_insert_items( instance, not_deleted, NULL );
		na_object_free_items( not_deleted );
	}

	return( delete_ok );
}

/**
 * cact_tree_ieditable_get_deleted:
 * @instance: this #CactTreeIEditable *instance.
 *
 * Returns: a copy of the 'deleted' list, which should be na_object_free_items()
 * by the caller.
 */
GList *
cact_tree_ieditable_get_deleted( CactTreeIEditable *instance )
{
	GList *deleted;
	IEditableData *ied;

	g_return_val_if_fail( CACT_IS_TREE_IEDITABLE( instance ), NULL );

	ied = get_instance_data( instance );
	deleted = na_object_copyref_items( ied->deleted );

	return( deleted );
}

/**
 * cact_tree_ieditable_insert_items:
 * @instance: this #CactTreeIEditable instance.
 * @items: a list of items to be inserted (e.g. from a paste).
 * @sibling: the #NAObject besides which the insertion should occurs;
 *  this is mainly used for inserting duplicated items besides each of
 *  original ones; may be %NULL.
 *
 * Inserts the provided @items list just before the @sibling object,
 * or at the current position in the treeview if @sibling is %NULL.
 *
 * The provided @items list is supposed to be homogeneous, i.e. referes
 * to only profiles, or only actions or menus.
 *
 * If the @items list contains profiles, they can only be inserted
 * into an action, or before another profile.
 *
 * If new item is a #NAObjectMenu or a #NAObjectAction, it will be inserted
 * before the current action or menu.
 *
 * This function takes care of repositionning a new selection (if possible)
 * on the lastly inserted item, and to refresh the display.
 */
void
cact_tree_ieditable_insert_items( CactTreeIEditable *instance, GList *items, NAObject *sibling )
{
	static const gchar *thisfn = "cact_tree_ieditable_insert_items";
	IEditableData *ied;
	GtkTreePath *insert_path;
	NAObject *object, *parent;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));
	g_return_if_fail( items );

	g_debug( "%s: instance=%p, items=%p (count=%d), sibling=%p",
			thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ), ( void * ) sibling );

	insert_path = NULL;
	ied = get_instance_data( instance );

	if( sibling ){
		insert_path = cact_tree_model_object_to_path( ied->model, sibling );

	} else {
		insert_path = get_selection_first_path( ied->treeview );
		object = cact_tree_model_object_at_path( ied->model, insert_path );
		g_debug( "%s: current object at insertion path is %p", thisfn, ( void * ) object );

		/* as a particular case, insert a new profile _into_ current action
		 */
		if( NA_IS_OBJECT_ACTION( object ) && NA_IS_OBJECT_PROFILE( items->data )){
			cact_tree_ieditable_insert_into( instance, items );
			gtk_tree_path_free( insert_path );
			return;
		}

		/* insert a new NAObjectItem before current NAObjectItem
		 */
		if( NA_IS_OBJECT_PROFILE( object ) && NA_IS_OBJECT_ITEM( items->data )){
			parent = ( NAObject * ) na_object_get_parent( object );
			gtk_tree_path_free( insert_path );
			insert_path = cact_tree_model_object_to_path( ied->model, parent );
		}
	}

	cact_tree_ieditable_insert_at_path( instance, items, insert_path );
	gtk_tree_path_free( insert_path );
}

/**
 * cact_tree_ieditable_insert_at_path:
 * @instance: this #CactTreeIEditable instance.
 * @items: a list of items to be inserted (e.g. from a paste).
 * @path: insertion path.
 *
 * Inserts the provided @items list in the treeview.
 *
 * This function takes care of repositionning a new selection if
 * possible, and refilter the display model.
 */
void
cact_tree_ieditable_insert_at_path( CactTreeIEditable *instance, GList *items, GtkTreePath *insert_path )
{
	static const gchar *thisfn = "cact_tree_ieditable_insert_at_path";
	IEditableData *ied;
	GtkTreePath *actual_path;
	gboolean prev_status, status;
	NAObjectItem *parent;
	GList *it;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));

	g_debug( "%s: instance=%p, items=%p (count=%d)",
		thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ));

	cact_tree_view_set_notify_allowed( CACT_TREE_VIEW( instance ), FALSE );
	ied = get_instance_data( instance );
	prev_status = get_modification_status( ied );

	actual_path = do_insert_before( ied, items, insert_path );

	parent = na_object_get_parent( items->data );
	if( parent ){
		na_object_check_status( parent );
	} else {
		for( it = items ; it ; it = it->next ){
			na_object_check_status( it->data );
		}
		g_signal_emit_by_name( ied->window, TREE_SIGNAL_LEVEL_ZERO_CHANGED, TRUE );
	}

	/* post insertion
	 */
	status = get_modification_status( ied );
	if( prev_status != status ){
		g_signal_emit_by_name( ied->window, TREE_SIGNAL_MODIFIED_STATUS_CHANGED, status );
	}
	cact_tree_view_set_notify_allowed( CACT_TREE_VIEW( instance ), TRUE );

	increment_counters( instance, ied, items );
	gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( ied->model ));
	cact_tree_view_select_row_at_path( CACT_TREE_VIEW( instance ), actual_path );
	gtk_tree_path_free( actual_path );
}

/**
 * cact_tree_ieditable_insert_into:
 * @instance: this #CactTreeIEditable instance.
 * @items: a list of items to be inserted (e.g. from a paste).
 *
 * Inserts the provided @items list as first children of the current item.
 *
 * The provided @items list is supposed to be homogeneous, i.e. referes
 * to only profiles, or only actions or menus.
 *
 * If the @items list contains only profiles, they can only be inserted
 * into an action, and the profiles will eventually be renumbered.
 *
 * If new item is a #NAObjectMenu or a #NAObjectAction, it will be inserted
 * as first children of the current menu.
 *
 * This function takes care of repositionning a new selection as the
 * last inserted item, and refilter the display model.
 */
void
cact_tree_ieditable_insert_into( CactTreeIEditable *instance, GList *items )
{
	static const gchar *thisfn = "cact_tree_ieditable_insert_into";
	IEditableData *ied;
	GtkTreePath *insert_path;
	GtkTreePath *new_path;
	NAObjectItem *parent;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));

	g_debug( "%s: instance=%p, items=%p (count=%d)",
		thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ));

	ied = get_instance_data( instance );
	insert_path = get_selection_first_path( ied->treeview );

	new_path = do_insert_into( ied, items, insert_path );

	parent = na_object_get_parent( items->data );
	na_object_check_status( parent );

	/* post insertion
	 */
	increment_counters( instance, ied, items );
	gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( ied->model ));
	cact_tree_view_select_row_at_path( CACT_TREE_VIEW( instance ), new_path );
	gtk_tree_path_free( new_path );
	gtk_tree_path_free( insert_path );
}

/*
 * inserting a list of objects at path is:
 * - for each item starting from the end, do
 *   > insert the item at path
 *   > recursively insert item children into the insertion path
 *
 * Returns the actual insertion path suitable for the next selection.
 * This path should be gtk_tree_path_free() by the caller.
 */
static GtkTreePath *
do_insert_before( IEditableData *ied, GList *items, GtkTreePath *asked_path )
{
	static const gchar *thisfn = "cact_tree_ieditable_do_insert_before";
	GList *reversed;
	GList *it;
	GtkTreePath *path;
	GtkTreePath *actual_path;

	reversed = g_list_reverse( items );
	actual_path = NULL;

	for( it = reversed ; it ; it = it->next ){
		if( actual_path ){
			path = gtk_tree_path_copy( actual_path );
			gtk_tree_path_free( actual_path );
		} else {
			path = gtk_tree_path_copy( asked_path );
		}
		actual_path = cact_tree_model_insert_before( ied->model, NA_OBJECT( it->data ), path );
		gtk_tree_path_free( path );
		g_debug( "%s: object=%p (%s, ref_count=%d)", thisfn,
				( void * ) it->data, G_OBJECT_TYPE_NAME( it->data ), G_OBJECT( it->data )->ref_count );

		/* recursively insert subitems
		 */
		if( NA_IS_OBJECT_ITEM( it->data )){
			path = do_insert_into( ied, na_object_get_items( it->data ), actual_path );
			gtk_tree_path_free( path );
		}
	}

	/* an l-value here is useless, but makes gcc happy */
	items = g_list_reverse( reversed );

	return( actual_path );
}

/*
 * inserting a list of objects into an object is as:
 * - inserting the last child into the object
 * - then inserting other children just before the last insertion path
 *
 * Returns the actual insertion path suitable for the next selection.
 * This path should be gtk_tree_path_free() by the caller.
 */
static GtkTreePath *
do_insert_into( IEditableData *ied, GList *items, GtkTreePath *asked_path )
{
	GList *copy;
	GList *last;
	GtkTreePath *actual_path;
	GtkTreePath *insertion_path;

	actual_path = NULL;

	if( items ){
		copy = g_list_copy( items );
		last = g_list_last( copy );
		copy = g_list_remove_link( copy, last );

		/* insert the last child into the wished path
		 * and recursively all its children
		 */
		actual_path = cact_tree_model_insert_into( ied->model, NA_OBJECT( last->data ), asked_path );
		gtk_tree_view_expand_to_path( ied->treeview, actual_path );

		if( NA_IS_OBJECT_ITEM( last->data )){
			insertion_path = do_insert_into( ied, na_object_get_items( last->data ), actual_path );
			gtk_tree_path_free( insertion_path );
		}

		/* then insert other objects
		 */
		if( copy ){
			insertion_path = do_insert_before( ied, copy, actual_path );
			gtk_tree_path_free( actual_path );
			actual_path = insertion_path;
			g_list_free( copy );
		}
	}

	return( actual_path );
}

/*
 * we pass here after each insertion operation (apart initial fill)
 */
static void
increment_counters( CactTreeIEditable *view, IEditableData *ied, GList *items )
{
	static const gchar *thisfn = "cact_tree_ieditable_increment_counters";
	gint menus, actions, profiles;

	g_debug( "%s: view=%p, ied=%p, items=%p (count=%d)",
			thisfn, ( void * ) view, ( void * ) ied, ( void * ) items, items ? g_list_length( items ) : 0 );

	na_object_count_items( items, &menus, &actions, &profiles );
	g_signal_emit_by_name( G_OBJECT( ied->window ), TREE_SIGNAL_COUNT_CHANGED, FALSE, menus, actions, profiles );
}

/*
 * cact_tree_ieditable_set_items:
 * @instance: this #CactTreeIEditable *instance.
 * @items: a #GList of items to be set in the tree view.
 *
 * The provided list of @items will override the items already in the list
 * which have the same identifier. But:
 * - menus keep their children
 * - entering actions fully override existing ones.
 *
 * Items which would not been found are just ignored.
 * Items which are not of the same type are ignored.
 *
 * This feature is typically used when importing an item whose identifier
 * already exists, and the user have chosen to override existing item.
 * So we should be almost sure that each item actually exists in the view.
 */
void
cact_tree_ieditable_set_items( CactTreeIEditable *instance, GList *items )
{
	static const gchar *thisfn = "cact_tree_ieditable_set_items";
	IEditableData *ied;
	GList *it;
	NAObjectItem *new_item, *old_item;
	gchar *id;
	GtkTreePath *path, *insert_path;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));

	g_debug( "%s: instance=%p, items=%p (count=%d)",
		thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ));

	ied = get_instance_data( instance );

	for( it = items ; it ; it = it->next ){
		new_item = NA_OBJECT_ITEM( it->data );

		id = na_object_get_id( new_item );
		old_item = cact_tree_view_get_item_by_id( CACT_TREE_VIEW( instance ), id );

		if( !old_item ){
			g_warning( "%s: id=%s: item not found - ignored", thisfn, id );

		} else if( G_OBJECT_TYPE( old_item ) != G_OBJECT_TYPE( new_item )){
			g_warning( "%s: id=%s: old is a %s while new is a %s - ignored", thisfn, id,
					G_OBJECT_TYPE_NAME( old_item ), G_OBJECT_TYPE_NAME( new_item ));

		} else if( NA_IS_OBJECT_MENU( old_item )){
			/* hopefully, na_factory_object_copy only copy valuable properties
			 * keeping dynamic variables as parent pointer, provider and provider
			 * data, read-only status - notably children are not impacted by this
			 * copy
			 */
			na_factory_object_copy( NA_IFACTORY_OBJECT( old_item ), NA_IFACTORY_OBJECT( new_item ));

		} else if( NA_IS_OBJECT_ACTION( old_item )){
			/* na_factory_object is not a deep copy, which is fine for the menu
			 * but not for the action - it appears more easier to just substitute
			 * the old item with the new one
			 *
			 * only children of the old item are its own profiles, which are to be
			 * replaced with the profiles provided by the new item
			 */
			path = cact_tree_model_delete( ied->model, NA_OBJECT( old_item ));
			insert_path = cact_tree_model_insert_before( ied->model, NA_OBJECT( new_item ), path );
			gtk_tree_path_free( path );
			gtk_tree_path_free( insert_path );

		} else {
			g_warning( "%s: should not come here!", thisfn );
		}

		g_free( id );
	}
}

/**
 * cact_tree_ieditable_dump_modified:
 * @instance: this #CactTreeIEditable *instance.
 *
 * Dump informations about modified items.
 */
void
cact_tree_ieditable_dump_modified( const CactTreeIEditable *instance )
{
	static const gchar *thisfn = "cact_tree_ieditable_dump_modified";
	IEditableData *ied;

	g_return_if_fail( CACT_IS_TREE_IEDITABLE( instance ));

	ied = get_instance_data(( CactTreeIEditable * ) instance );

	g_debug( "%s:      count_deleted=%u", thisfn, g_list_length( ied->deleted ));
	g_debug( "%s:     count_modified=%u", thisfn, ied->count_modified );
	g_debug( "%s: level_zero_changed=%s", thisfn, ied->level_zero_changed ? "True":"False" );
}

/**
 * cact_tree_ieditable_is_level_zero_modified:
 * @instance: this #CactTreeIEditable *instance.
 *
 * Returns: %TRUE if the level zero must be saved, %FALSE else.
 */
gboolean
cact_tree_ieditable_is_level_zero_modified( const CactTreeIEditable *instance )
{
	IEditableData *ied;
	gboolean is_modified;

	g_return_val_if_fail( CACT_IS_TREE_IEDITABLE( instance ), FALSE );

	ied = get_instance_data(( CactTreeIEditable * ) instance );
	is_modified = ied->level_zero_changed;

	return( is_modified );
}

static void
check_level_zero_status( CactTreeIEditable *instance )
{
	static const gchar *thisfn = "cact_tree_ieditable_check_level_zero_status";
	gboolean status;
	IEditableData *ied;
	GList *items;
	gchar *pivot_str, *view_str;

	ied = get_instance_data( instance );
	items = na_pivot_get_items( NA_PIVOT( ied->updater ));
	pivot_str = get_items_id_list_str( items );

	items = cact_tree_view_get_items( CACT_TREE_VIEW( instance ));
	view_str = get_items_id_list_str( items );
	na_object_free_items( items );

	status = ( g_utf8_collate( pivot_str, view_str ) != 0 );
	g_debug( "%s: pivot_str='%s', view_str='%s', status=%s", thisfn, pivot_str, view_str, status ? "True":"False" );

	g_free( pivot_str );
	g_free( view_str );

	g_signal_emit_by_name( ied->window, TREE_SIGNAL_LEVEL_ZERO_CHANGED, status );
}

static gchar *
get_items_id_list_str( GList *items_list )
{
	GList *it;
	gchar *id;
	GSList *slist;
	gchar *list_str;

	slist = NULL;

	for( it = items_list ; it ; it = it->next ){
		id = na_object_get_id( it->data );
		slist = g_slist_prepend( slist, id );
	}

	slist = g_slist_reverse( slist );
	list_str = na_core_utils_slist_join_at_end( slist, ";" );
	na_core_utils_slist_free( slist );

	return( list_str );
}

static GtkTreePath *
get_selection_first_path( GtkTreeView *treeview )
{
	GtkTreeSelection *selection;
	GList *selected;
	GtkTreePath *path;

	path = NULL;
	selection = gtk_tree_view_get_selection( treeview );
	selected = gtk_tree_selection_get_selected_rows( selection, NULL );

	if( selected ){
		path = gtk_tree_path_copy(( GtkTreePath * ) selected->data );

	} else {
		path = gtk_tree_path_new_from_string( "0" );
	}

	g_list_foreach( selected, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( selected );

	return( path );
}

static gboolean
get_modification_status( IEditableData *ied )
{
	static const gchar *thisfn = "cact_tree_ieditable_get_modification_status";
	gboolean modified;

	modified = ( ied->count_modified > 0 ||
				ied->count_deleted > 0 ||
				ied->level_zero_changed );

	g_debug( "%s: count_modified=%d, deleted=%d, level_zero_changed=%s, modified=%s",
			thisfn, ied->count_modified, ied->count_deleted,
			ied->level_zero_changed ? "True":"False", modified ? "True":"False" );

	return( modified );
}

static IEditableData *
get_instance_data( CactTreeIEditable *view )
{
	IEditableData *ied;

	ied = ( IEditableData * ) g_object_get_data( G_OBJECT( view ), VIEW_DATA_IEDITABLE );

	if( !ied ){
		ied = g_new0( IEditableData, 1 );
		g_object_set_data( G_OBJECT( view ), VIEW_DATA_IEDITABLE, ied );
	}

	return( ied );
}

/*
 * triggered by 'F2' key
 * let the label be edited
 */
static void
inline_edition( BaseWindow *window )
{
	CactTreeView *items_view;
	IEditableData *ied;
	GtkTreeSelection *selection;
	GList *listrows;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeViewColumn *column;

	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));
	ied = ( IEditableData * ) g_object_get_data( G_OBJECT( items_view ), VIEW_DATA_IEDITABLE );
	selection = gtk_tree_view_get_selection( ied->treeview );
	listrows = gtk_tree_selection_get_selected_rows( selection, &model );

	if( g_list_length( listrows ) == 1 ){
		path = ( GtkTreePath * ) listrows->data;
		column = gtk_tree_view_get_column( ied->treeview, TREE_COLUMN_LABEL );
		gtk_tree_view_set_cursor( ied->treeview, path, column, TRUE );
	}

	g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( listrows );
}
