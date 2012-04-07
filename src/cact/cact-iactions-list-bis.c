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

#include <api/na-object-api.h>

#include "cact-tree-model.h"
#include "cact-iactions-list.h"
#include "cact-iactions-list-priv.h"
#include "cact-window.h"

/* when toggle collapse/expand rows, we want all rows have the same
 * behavior, e.g. all rows collapse, or all rows expand
 * this behavior is fixed by the first rows which will actually be
 * toggled
 */
enum {
	TOGGLE_UNDEFINED,
	TOGGLE_COLLAPSE,
	TOGGLE_EXPAND
};

/* iter on selection prototype
 */
typedef gboolean ( *FnIterOnSelection )( CactIActionsList *, GtkTreeView *, GtkTreeModel *, GtkTreeIter *, NAObject *, gpointer );

/* when iterating while searching for an object
 */
typedef struct {
	NAObject *object;
	gchar    *uuid;
}
	IdToObjectIter;

/* when iterating while searching for the path of an object
 */
typedef struct {
	NAObject    *object;
	gboolean     found;
	GtkTreePath *path;
}
	ObjectToPathIter;

extern gboolean st_iactions_list_initialized;
extern gboolean st_iactions_list_finalized;

static void         decrement_counters( CactIActionsList *instance, IActionsListInstanceData *ialid, GList *items );
static void         do_insert_items( GtkTreeView *treeview, GtkTreeModel *model, GList *items, GtkTreePath *path, GList **parents );
static NAObject    *do_insert_into_first( GtkTreeView *treeview, GtkTreeModel *model, GList *items, GtkTreePath *insert_path, GtkTreePath **new_path );
static void         extend_selection_to_childs( CactIActionsList *instance, GtkTreeView *treeview, GtkTreeModel *model, GtkTreeIter *parent );
static gboolean     get_item_iter( CactTreeModel *model, GtkTreePath *path, NAObject *object, IdToObjectIter *ito );
static gboolean     get_items_iter( CactTreeModel *model, GtkTreePath *path, NAObject *object, GList **items );
static GtkTreePath *get_selection_first_path( GtkTreeView *treeview );
static void         increment_counters( CactIActionsList *instance, IActionsListInstanceData *ialid, GList *items );
static void         iter_on_selection( CactIActionsList *instance, FnIterOnSelection fn_iter, gpointer user_data );
static GtkTreePath *object_to_path( CactIActionsList *instance, CactTreeModel *model, NAObject *object );
static gboolean     object_to_path_iter( CactTreeModel *model, GtkTreePath *path, NAObject *object, ObjectToPathIter *otp );
static gboolean     toggle_collapse_iter( CactIActionsList *instance, GtkTreeView *treeview, GtkTreeModel *model, GtkTreeIter *iter, NAObject *object, gpointer user_data );
static void         toggle_collapse_row( GtkTreeView *treeview, GtkTreePath *path, guint *toggle );
static void         update_parents_edition_status( GList *parents, GList *items );

/**
 * cact_iactions_list_bis_clear_selection:
 * @instance: this instance of the #CactIActionsList interface.
 * @treeview: the #GtkTreeView.
 *
 * Clears the current selection.
 */
void
cact_iactions_list_bis_clear_selection( CactIActionsList *instance, GtkTreeView *treeview )
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection( treeview );
	gtk_tree_selection_unselect_all( selection );
}

/**
 * cact_iactions_list_bis_collapse_to_parent:
 * @instance: this instance of the #CactIActionsList interface.
 *
 * On left arrow, if we are on a first child, then collapse and go to
 * the parent.
 */
void
cact_iactions_list_bis_collapse_to_parent( CactIActionsList *instance )
{
	static const gchar *thisfn = "cact_iactions_list_bis_collapse_to_parent";
	IActionsListInstanceData *ialid;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *listrows;
	GtkTreePath *path;
	gint *indices;
	GtkTreePath *parent_path;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));

	ialid = cact_iactions_list_priv_get_instance_data( instance );
	if( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION ){

		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		selection = gtk_tree_view_get_selection( treeview );
		listrows = gtk_tree_selection_get_selected_rows( selection, &model );
		if( g_list_length( listrows ) == 1 ){

			path = ( GtkTreePath * ) listrows->data;
			/*g_debug( "%s: path_depth=%d", thisfn, gtk_tree_path_get_depth( path ));*/
			if( gtk_tree_path_get_depth( path ) > 1 ){

				indices = gtk_tree_path_get_indices( path );
				if( indices[ gtk_tree_path_get_depth( path )-1 ] == 0 ){

					parent_path = gtk_tree_path_copy( path );
					gtk_tree_path_up( parent_path );
					cact_iactions_list_bis_select_row_at_path( instance, treeview, model, parent_path );
					gtk_tree_view_collapse_row( treeview, parent_path );
					gtk_tree_path_free( parent_path );
				}
			}
		}

		g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
		g_list_free( listrows );
	}
}

/**
 * cact_iactions_list_bis_delete:
 * @window: this #CactIActionsList instance.
 * @list: list of #NAObject to be deleted.
 *
 * Deletes the specified list from the underlying tree store.
 *
 * This function takes care of repositionning a new selection if
 * possible, and refilter the display model.
 */
void
cact_iactions_list_bis_delete( CactIActionsList *instance, GList *items, gboolean select_at_end )
{
	static const gchar *thisfn = "cact_iactions_list_bis_delete";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GList *it;
	IActionsListInstanceData *ialid;

	g_debug( "%s: instance=%p, items=%p (count=%d), select_at_end=%s",
			thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ), select_at_end ? "True":"False" );
	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		model = gtk_tree_view_get_model( treeview );

		ialid = cact_iactions_list_priv_get_instance_data( instance );
		ialid->selection_changed_allowed = FALSE;

		decrement_counters( instance, ialid, items );

		for( it = items ; it ; it = it->next ){
			if( path ){
				gtk_tree_path_free( path );
			}

			path = cact_tree_model_remove( CACT_TREE_MODEL( model ), NA_OBJECT( it->data ));

			ialid->modified_items = cact_iactions_list_remove_rec( ialid->modified_items, NA_OBJECT( it->data ));

			g_debug( "%s: object=%p (%s, ref_count=%d)", thisfn,
					( void * ) it->data, G_OBJECT_TYPE_NAME( it->data ), G_OBJECT( it->data )->ref_count );
		}

		gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( model ));

		ialid->selection_changed_allowed = TRUE;

		if( path ){
			if( select_at_end ){
				cact_iactions_list_bis_select_row_at_path( instance, treeview, model, path );
			}
			gtk_tree_path_free( path );
		}
	}
}

/**
 * cact_iactions_list_bis_expand_to_first_child:
 * @instance: this #CactIActionsList interface.
 *
 * On right arrow, expand the parent if it has childs, and select the
 * first child.
 */
void
cact_iactions_list_bis_expand_to_first_child( CactIActionsList *instance )
{
	static const gchar *thisfn = "cact_iactions_list_bis_expand_to_first_child";
	IActionsListInstanceData *ialid;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *listrows;
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreePath *child_path;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));

	ialid = cact_iactions_list_priv_get_instance_data( instance );
	if( ialid->management_mode == IACTIONS_LIST_MANAGEMENT_MODE_EDITION ){
		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		selection = gtk_tree_view_get_selection( treeview );
		listrows = gtk_tree_selection_get_selected_rows( selection, &model );

		if( g_list_length( listrows ) == 1 ){
			path = ( GtkTreePath * ) listrows->data;
			gtk_tree_model_get_iter( model, &iter, path );
			if( gtk_tree_model_iter_has_child( model, &iter )){
				child_path = gtk_tree_path_copy( path );
				gtk_tree_path_append_index( child_path, 0 );
				gtk_tree_view_expand_row( treeview, child_path, FALSE );
				cact_iactions_list_bis_select_row_at_path( instance, treeview, model, child_path );
				gtk_tree_path_free( child_path );
			}
		}

		g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
		g_list_free( listrows );
	}
}

/**
 * cact_iactions_list_bis_get_item:
 * @window: this #CactIActionsList instance.
 * @id: the id of the searched object.
 *
 * Returns: a pointer on the #NAObject which has this id, or NULL.
 *
 * The returned pointer is owned by IActionsList (actually by the
 * underlying tree store), and should not be released by the caller.
 */
NAObject *
cact_iactions_list_bis_get_item( CactIActionsList *instance, const gchar *id )
{
	NAObject *item = NULL;
	GtkTreeView *treeview;
	CactTreeModel *model;
	IdToObjectIter *ito;

	g_return_val_if_fail( CACT_IS_IACTIONS_LIST( instance ), NULL );

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		model = CACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));

		ito = g_new0( IdToObjectIter, 1 );
		ito->uuid = ( gchar * ) id;

		cact_tree_model_iter( model, ( FnIterOnStore ) get_item_iter, ito );

		item = ito->object;

		g_free( ito );
	}

	return( item );
}

/**
 * cact_iactions_list_bis_get_items:
 * @window: this #CactIActionsList instance.
 *
 * Returns: the current tree.
 *
 * The returned #GList content is owned by the underlying tree model,
 * and should only be g_list_free() by the caller.
 */
GList *
cact_iactions_list_bis_get_items( CactIActionsList *instance )
{
	GList *items = NULL;
	GtkTreeView *treeview;
	CactTreeModel *model;

	g_return_val_if_fail( CACT_IS_IACTIONS_LIST( instance ), NULL );

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		model = CACT_TREE_MODEL( gtk_tree_view_get_model( treeview ));

		cact_tree_model_iter( model, ( FnIterOnStore ) get_items_iter, &items );

		items = g_list_reverse( items );
	}

	return( items );
}

/**
 * cact_iactions_list_bis_get_selected_items:
 * @window: this #CactIActionsList instance.
 *
 * Returns: the currently selected rows as a list of #NAObjects.
 *
 * We acquire here a new reference on objects corresponding to actually
 * selected rows, and their childs.
 *
 * The caller may safely call na_object_free_items_list() on the
 * returned list.
 */
GList *
cact_iactions_list_bis_get_selected_items( CactIActionsList *instance )
{
	GList *items = NULL;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GList *it, *listrows;
	NAObject *object;
	GtkTreePath *path;

	g_return_val_if_fail( CACT_IS_IACTIONS_LIST( instance ), NULL );

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		selection = gtk_tree_view_get_selection( treeview );
		listrows = gtk_tree_selection_get_selected_rows( selection, &model );

		for( it = listrows ; it ; it = it->next ){
			path = ( GtkTreePath * ) it->data;
			gtk_tree_model_get_iter( model, &iter, path );
			gtk_tree_model_get( model, &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );
			g_debug( "cact_iactions_list_get_selected_items: object=%p", ( void * ) object );
			items = g_list_prepend( items, na_object_ref( object ));
			g_object_unref( object );
		}

		g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
		g_list_free( listrows );

		items = g_list_reverse( items );
	}

	return( items );
}

/**
 * cact_iactions_list_bis_insert_at_path:
 * @instance: this #CactIActionsList instance.
 * @items: a list of items to be inserted (e.g. from a paste).
 * @path: insertion path.
 *
 * Inserts the provided @items list in the treeview.
 *
 * This function takes care of repositionning a new selection if
 * possible, and refilter the display model.
 */
void
cact_iactions_list_bis_insert_at_path( CactIActionsList *instance, GList *items, GtkTreePath *insert_path )
{
	static const gchar *thisfn = "cact_iactions_list_bis_insert_at_path";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GList *parents;
	IActionsListInstanceData *ialid;

	g_debug( "%s: instance=%p, items=%p (%d items)",
			thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ));
	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));
	g_return_if_fail( CACT_IS_WINDOW( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		model = gtk_tree_view_get_model( treeview );
		g_return_if_fail( CACT_IS_TREE_MODEL( model ));

		do_insert_items( treeview, model, items, insert_path, &parents );

		update_parents_edition_status( parents, items );
		ialid = cact_iactions_list_priv_get_instance_data( instance );
		increment_counters( instance, ialid, items );

		gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( model ));

		cact_iactions_list_bis_select_row_at_path( instance, treeview, model, insert_path );
	}
}

/**
 * cact_iactions_list_bis_insert_items:
 * @instance: this #CactIActionsList instance.
 * @items: a list of items to be inserted (e.g. from a paste).
 * @sibling: the #NAObject besides which the insertion should occurs ;
 * this may prevent e.g. to go inside a menu.
 *
 * Inserts the provided @items list in the treeview.
 *
 * The provided @items list is supposed to be homogeneous, i.e. referes
 * to only profiles, or only actions or menus.
 *
 * If the @items list contains profiles, they can only be inserted
 * into an action, or before another profile.
 *
 * If new item is a #NAActionMenu or a #NAAction, it will be inserted
 * before the current action or menu.
 *
 * This function takes care of repositionning a new selection if
 * possible, and refilter the display model.
 */
void
cact_iactions_list_bis_insert_items( CactIActionsList *instance, GList *items, NAObject *sibling )
{
	static const gchar *thisfn = "cact_iactions_list_bis_insert_items";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreePath *insert_path;
	NAObject *object;

	g_debug( "%s: instance=%p, items=%p (%d items), sibling=%p",
			thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ), ( void * ) sibling );
	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));
	g_return_if_fail( CACT_IS_WINDOW( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		insert_path = NULL;
		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		model = gtk_tree_view_get_model( treeview );
		g_return_if_fail( CACT_IS_TREE_MODEL( model ));

		if( sibling ){
			insert_path = object_to_path( instance, CACT_TREE_MODEL( model ), sibling );

		} else {
			insert_path = get_selection_first_path( treeview );

			/* as a particular case, insert profiles _into_ current action
			 */
			object = cact_tree_model_object_at_path( CACT_TREE_MODEL( model ), insert_path );
			/*g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
			g_debug( "%s: items->data is %s", thisfn, G_OBJECT_TYPE_NAME( items->data ));*/
			if( NA_IS_OBJECT_ACTION( object ) &&
				NA_IS_OBJECT_PROFILE( items->data )){

				cact_iactions_list_bis_insert_into( instance, items );
				gtk_tree_path_free( insert_path );
				return;
			}
		}

		cact_iactions_list_bis_insert_at_path( instance, items, insert_path );
		gtk_tree_path_free( insert_path );
	}
}

/**
 * cact_iactions_list_bis_insert_into:
 * @instance: this #CactIActionsList instance.
 * @items: a list of items to be inserted (e.g. from a paste).
 *
 * Inserts the provided @items list as first childs of the current item.
 *
 * The provided @items list is supposed to be homogeneous, i.e. referes
 * to only profiles, or only actions or menus.
 *
 * If the @items list contains only profiles, they can only be inserted
 * into an action, and the profiles will eventually be renumbered.
 *
 * If new item is a #NAActionMenu or a #NAAction, it will be inserted
 * as first childs of the current menu.
 *
 * This function takes care of repositionning a new selection as the
 * last inserted item, and refilter the display model.
 */
void
cact_iactions_list_bis_insert_into( CactIActionsList *instance, GList *items )
{
	static const gchar *thisfn = "cact_iactions_list_bis_insert_into";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreePath *insert_path;
	GtkTreePath *new_path;
	NAObject *parent;
	IActionsListInstanceData *ialid;

	g_debug( "%s: instance=%p, items=%p (%d items)",
			thisfn, ( void * ) instance, ( void * ) items, g_list_length( items ));
	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));
	g_return_if_fail( CACT_IS_WINDOW( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		insert_path = NULL;
		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		model = gtk_tree_view_get_model( treeview );
		insert_path = get_selection_first_path( treeview );

		ialid = cact_iactions_list_priv_get_instance_data( instance );
		increment_counters( instance, ialid, items );

		parent = do_insert_into_first( treeview, model, items, insert_path, &new_path );

		na_object_check_status_up( parent );

		gtk_tree_model_filter_refilter( GTK_TREE_MODEL_FILTER( model ));

		cact_iactions_list_bis_select_row_at_path( instance, treeview, model, new_path );

		gtk_tree_path_free( new_path );
		gtk_tree_path_free( insert_path );
	}
}

/**
 * cact_iactions_list_bis_list_modified_items:
 * @instance: this #CactIActionsList instance.
 *
 * Dumps the modified items list.
 */
void
cact_iactions_list_bis_list_modified_items( CactIActionsList *instance )
{
	static const gchar *thisfn = "cact_iactions_list_bis_list_modified_items";
	IActionsListInstanceData *ialid;
	GList *it;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		ialid = cact_iactions_list_priv_get_instance_data( instance );

		g_debug( "%s: raw list", thisfn );
		for( it = ialid->modified_items ; it ; it = it->next ){
			g_debug( "%s: %p (%s)", thisfn, ( void * ) it->data, G_OBJECT_TYPE_NAME( it->data ));
		}

		g_debug( "%s: detailed list", thisfn );
		for( it = ialid->modified_items ; it ; it = it->next ){
			na_object_dump( it->data );
		}
	}
}

/**
 * cact_iactions_list_bis_remove_modified:
 * @instance: this #CactIActionsList instance.
 *
 * Removes the saved item from the modified items list.
 */
void
cact_iactions_list_bis_remove_modified( CactIActionsList *instance, const NAObjectItem *item )
{
	IActionsListInstanceData *ialid;

	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		ialid = cact_iactions_list_priv_get_instance_data( instance );
		ialid->modified_items = g_list_remove( ialid->modified_items, item );

		if( g_list_length( ialid->modified_items ) == 0 ){
			g_list_free( ialid->modified_items );
			ialid->modified_items = NULL;
		}
	}
}

/**
 * cact_iactions_list_bis_select_first_row:
 * @instance: this instance of #CactIActionsList interface.
 *
 * Select the first visible row of the tree.
 */
void
cact_iactions_list_bis_select_first_row( CactIActionsList *instance )
{
	static const gchar *thisfn = "cact_iactions_list_bis_select_first_row";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreePath *path;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );

	treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
	model = gtk_tree_view_get_model( treeview );

	path = gtk_tree_path_new_from_string( "0" );
	cact_iactions_list_bis_select_row_at_path( instance, treeview, model, path );
	gtk_tree_path_free( path );
}

/**
 * cact_iactions_list_bis_select_row_at_path:
 * @instance: this #CactIActionsList interface.
 * @treeview: the embedded #GtkTreeView.
 * @model: the corresponding GtkTreeModel.
 * @path: a #GtkTreePath.
 *
 * Select the row at the required path, or the next following, or
 * the immediate previous.
 */
void
cact_iactions_list_bis_select_row_at_path( CactIActionsList *instance, GtkTreeView *treeview, GtkTreeModel *model, GtkTreePath *path )
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gboolean anything = FALSE;

	selection = gtk_tree_view_get_selection( treeview );
	gtk_tree_selection_unselect_all( selection );

	if( path ){
		g_debug( "cact_iactions_list_bis_select_row_at_path: path=%s", gtk_tree_path_to_string( path ));
		gtk_tree_view_expand_to_path( treeview, path );

		if( gtk_tree_model_get_iter( model, &iter, path )){
			gtk_tree_view_set_cursor( treeview, path, NULL, FALSE );
			anything = TRUE;

		} else if( gtk_tree_path_prev( path ) && gtk_tree_model_get_iter( model, &iter, path )){
			gtk_tree_view_set_cursor( treeview, path, NULL, FALSE );
			anything = TRUE;

		} else {
			gtk_tree_path_next( path );
			if( gtk_tree_model_get_iter( model, &iter, path )){
				gtk_tree_view_set_cursor( treeview, path, NULL, FALSE );
				anything = TRUE;

			} else if( gtk_tree_path_get_depth( path ) > 1 &&
						gtk_tree_path_up( path ) &&
						gtk_tree_model_get_iter( model, &iter, path )){

							gtk_tree_view_set_cursor( treeview, path, NULL, FALSE );
							anything = TRUE;
			}
		}
	}

	/* if nothing can be selected, at least send a message with empty
	 *  selection
	 */
	if( !anything ){
		cact_iactions_list_on_treeview_selection_changed( NULL, instance );
	}
}

/**
 * cact_iactions_list_bis_toggle_collapse:
 * @instance: this #CactIActionsList interface.
 *
 * Toggle or collapse the current subtree.
 */
void
cact_iactions_list_bis_toggle_collapse( CactIActionsList *instance )
{
	int toggle = TOGGLE_UNDEFINED;

	iter_on_selection( instance, ( FnIterOnSelection ) toggle_collapse_iter, &toggle );
}

/**
 * cact_iactions_list_bis_toggle_collapse_object:
 * @instance: the current instance of the #CactIActionsList interface.
 * @item: the item to be toggled/collapsed.
 *
 * Collapse / expand if actions has more than one profile.
 */
void
cact_iactions_list_bis_toggle_collapse_object( CactIActionsList *instance, const NAObject *item )
{
	static const gchar *thisfn = "cact_iactions_list_bis_toggle_collapse";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean iterok, stop;
	NAObject *iter_object;

	g_return_if_fail( CACT_IS_IACTIONS_LIST( instance ));
	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));

	if( st_iactions_list_initialized && !st_iactions_list_finalized ){

		treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
		model = gtk_tree_view_get_model( treeview );
		iterok = gtk_tree_model_get_iter_first( model, &iter );
		stop = FALSE;

		while( iterok && !stop ){

			gtk_tree_model_get( model, &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &iter_object, -1 );
			if( iter_object == item ){

				if( na_object_get_items_count( item ) > 1 ){

					GtkTreePath *path = gtk_tree_model_get_path( model, &iter );

					if( gtk_tree_view_row_expanded( GTK_TREE_VIEW( treeview ), path )){
						gtk_tree_view_collapse_row( GTK_TREE_VIEW( treeview ), path );
						g_debug( "%s: action=%p collapsed", thisfn, ( void * ) item );

					} else {
						gtk_tree_view_expand_row( GTK_TREE_VIEW( treeview ), path, TRUE );
						g_debug( "%s: action=%p expanded", thisfn, ( void * ) item );
					}

					gtk_tree_path_free( path );
				}
				stop = TRUE;
			}

			g_object_unref( iter_object );
			iterok = gtk_tree_model_iter_next( model, &iter );
		}
	}
}

static void
decrement_counters( CactIActionsList *instance, IActionsListInstanceData *ialid, GList *items )
{
	static const gchar *thisfn = "cact_iactions_list_decrement_counters";
	gint menus, actions, profiles;

	g_debug( "%s: instance=%p, ialid=%p, items=%p",
			thisfn, ( void * ) instance, ( void * ) ialid, ( void * ) items );

	menus = 0;
	actions = 0;
	profiles = 0;
	na_object_item_count_items( items, &menus, &actions, &profiles, TRUE );

	ialid->menus -= menus;
	ialid->actions -= actions;
	ialid->profiles -= profiles;

	cact_iactions_list_priv_send_list_count_updated_signal( instance, ialid );
}

static void
do_insert_items( GtkTreeView *treeview, GtkTreeModel *model, GList *items, GtkTreePath *insert_path, GList **list_parents )
{
	static const gchar *thisfn = "cact_iactions_list_bis_do_insert_items";
	GList *reversed;
	GList *it;
	GList *subitems;
	NAObject *obj_parent;
	gpointer updatable;
	GtkTreePath *inserted_path;

	obj_parent = NULL;
	if( list_parents ){
		*list_parents = NULL;
	}

	reversed = g_list_reverse( g_list_copy( items ));

	for( it = reversed ; it ; it = it->next ){

		inserted_path = cact_tree_model_insert( CACT_TREE_MODEL( model ), NA_OBJECT( it->data ), insert_path, &obj_parent );

		g_debug( "%s: object=%p (%s, ref_count=%d)", thisfn,
				( void * ) it->data, G_OBJECT_TYPE_NAME( it->data ), G_OBJECT( it->data )->ref_count );

		if( list_parents ){
			updatable = obj_parent ? obj_parent : it->data;
			if( !g_list_find( *list_parents, updatable )){
				*list_parents = g_list_prepend( *list_parents, updatable );
			}
		}

		/* recursively insert subitems
		 */
		if( NA_IS_OBJECT_ITEM( it->data ) && na_object_get_items_count( it->data )){

			subitems = na_object_get_items( it->data );
			do_insert_into_first( treeview, model, subitems, inserted_path, NULL );
		}

		gtk_tree_path_free( inserted_path );
	}

	g_list_free( reversed );
}

static NAObject *
do_insert_into_first( GtkTreeView *treeview, GtkTreeModel *model, GList *items, GtkTreePath *insert_path, GtkTreePath **new_path )
{
	static const gchar *thisfn = "cact_iactions_list_do_bis_insert_into_first";
	GList *copy;
	GList *last;
	NAObject *parent;
	gchar *insert_path_str;
	GtkTreePath *inserted_path;
	GList *subitems;

	insert_path_str = gtk_tree_path_to_string( insert_path );
	g_debug( "%s: treeview=%p, model=%p, items=%p (count=%d), insert_path=%p (%s), new_path=%p",
			thisfn,
			( void * ) treeview, ( void * ) model, ( void * ) items, g_list_length( items ),
			( void * ) insert_path, insert_path_str, ( void * ) new_path );
	g_free( insert_path_str );

	parent = NULL;
	copy = g_list_copy( items );
	last = g_list_last( copy );
	copy = g_list_remove_link( copy, last );

	inserted_path = cact_tree_model_insert_into( CACT_TREE_MODEL( model ), NA_OBJECT( last->data ), insert_path, &parent );
	gtk_tree_view_expand_to_path( treeview, inserted_path );

	/* recursively insert subitems
	 */
	if( NA_IS_OBJECT_ITEM( last->data ) && na_object_get_items_count( last->data )){

		subitems = na_object_get_items( last->data );
		do_insert_into_first( treeview, model, subitems, inserted_path, NULL );
	}

	do_insert_items( treeview, model, copy, inserted_path, NULL );

	if( new_path ){
		*new_path = gtk_tree_path_copy( inserted_path );
	}

	g_list_free( copy );
	gtk_tree_path_free( inserted_path );

	return( parent );
}

/*
 * when expanding a selected row which has childs
 */
static void
extend_selection_to_childs( CactIActionsList *instance, GtkTreeView *treeview, GtkTreeModel *model, GtkTreeIter *parent )
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gboolean ok;

	selection = gtk_tree_view_get_selection( treeview );

	ok = gtk_tree_model_iter_children( model, &iter, parent );

	while( ok ){
		GtkTreePath *path = gtk_tree_model_get_path( model, &iter );
		gtk_tree_selection_select_path( selection, path );
		gtk_tree_path_free( path );
		ok = gtk_tree_model_iter_next( model, &iter );
	}
}

/*
 * search for an object, given its id
 */
static gboolean
get_item_iter( CactTreeModel *model, GtkTreePath *path, NAObject *object, IdToObjectIter *ito )
{
	gchar *id;
	gboolean found = FALSE;

	id = na_object_get_id( object );
	found = ( g_ascii_strcasecmp( id, ito->uuid ) == 0 );
	g_free( id );

	if( found ){
		ito->object = object;
	}

	/* stop iteration if found */
	return( found );
}

/*
 * builds the tree
 */
static gboolean
get_items_iter( CactTreeModel *model, GtkTreePath *path, NAObject *object, GList **items )
{
	if( gtk_tree_path_get_depth( path ) == 1 ){
		*items = g_list_prepend( *items, object );
	}

	/* don't stop iteration */
	return( FALSE );
}

static GtkTreePath *
get_selection_first_path( GtkTreeView *treeview )
{
	GtkTreeSelection *selection;
	GList *list_selected;
	GtkTreePath *path;

	path = NULL;
	selection = gtk_tree_view_get_selection( treeview );
	list_selected = gtk_tree_selection_get_selected_rows( selection, NULL );

	if( g_list_length( list_selected )){
		path = gtk_tree_path_copy(( GtkTreePath * ) list_selected->data );

	} else {
		path = gtk_tree_path_new_from_string( "0" );
	}

	g_list_foreach( list_selected, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( list_selected );

	return( path );
}

static void
increment_counters( CactIActionsList *instance, IActionsListInstanceData *ialid, GList *items )
{
	static const gchar *thisfn = "cact_iactions_list_increment_counters";
	gint menus, actions, profiles;

	g_debug( "%s: instance=%p, ialid=%p, items=%p (count=%d)",
			thisfn, ( void * ) instance, ( void * ) ialid, ( void * ) items, items ? g_list_length( items ) : 0 );

	menus = 0;
	actions = 0;
	profiles = 0;
	na_object_item_count_items( items, &menus, &actions, &profiles, TRUE );
	/*g_debug( "increment_counters: counted: menus=%d, actions=%d, profiles=%d", menus, actions, profiles );*/

	/*g_debug( "incremente_counters: cs before: menus=%d, actions=%d, profiles=%d", cs->menus, cs->actions, cs->profiles );*/
	ialid->menus += menus;
	ialid->actions += actions;
	ialid->profiles += profiles;
	/*g_debug( "increment_counters: cs after: menus=%d, actions=%d, profiles=%d", cs->menus, cs->actions, cs->profiles );*/

	cact_iactions_list_priv_send_list_count_updated_signal( instance, ialid );
}

static void
iter_on_selection( CactIActionsList *instance, FnIterOnSelection fn_iter, gpointer user_data )
{
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *listrows, *ipath;
	GtkTreePath *path;
	GtkTreeIter iter;
	NAObject *object;
	gboolean stop = FALSE;

	treeview = cact_iactions_list_priv_get_actions_list_treeview( instance );
	selection = gtk_tree_view_get_selection( treeview );
	listrows = gtk_tree_selection_get_selected_rows( selection, &model );
	listrows = g_list_reverse( listrows );

	for( ipath = listrows ; !stop && ipath ; ipath = ipath->next ){

		path = ( GtkTreePath * ) ipath->data;
		gtk_tree_model_get_iter( model, &iter, path );
		gtk_tree_model_get( model, &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );

		stop = fn_iter( instance, treeview, model, &iter, object, user_data );

		g_object_unref( object );
	}

	g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( listrows );
}

static GtkTreePath *
object_to_path( CactIActionsList *instance, CactTreeModel *model, NAObject *object )
{
	ObjectToPathIter *otp;
	GtkTreePath *path = NULL;

	otp = g_new0( ObjectToPathIter, 1 );
	otp->object = object;
	otp->found = FALSE;
	otp->path = NULL;

	cact_tree_model_iter( model, ( FnIterOnStore ) object_to_path_iter, otp );

	if( otp->found ){
		path = gtk_tree_path_copy( otp->path );
		gtk_tree_path_free( otp->path );
	}

	g_free( otp );

	return( path );
}

static gboolean
object_to_path_iter( CactTreeModel *model, GtkTreePath *path, NAObject *object, ObjectToPathIter *otp )
{
	if( object == otp->object ){
		otp->found = TRUE;
		otp->path = gtk_tree_path_copy( path );
	}

	return( otp->found );
}

static gboolean
toggle_collapse_iter( CactIActionsList *instance,
						GtkTreeView *treeview,
						GtkTreeModel *model,
						GtkTreeIter *iter,
						NAObject *object,
						gpointer user_data )
{
	guint count;
	guint *toggle;

	toggle = ( guint * ) user_data;

	if( NA_IS_OBJECT_ITEM( object )){

		GtkTreePath *path = gtk_tree_model_get_path( model, iter );

		if( NA_IS_OBJECT_ITEM( object )){
			count = na_object_get_items_count( object );

			if(( count > 1 && NA_IS_OBJECT_ACTION( object )) ||
				( count > 0 && NA_IS_OBJECT_MENU( object ))){

				toggle_collapse_row( treeview, path, toggle );
			}
		}

		gtk_tree_path_free( path );

		/* do not extend selection */
		if( *toggle == TOGGLE_EXPAND && FALSE ){
			extend_selection_to_childs( instance, treeview, model, iter );
		}
	}

	/* do not stop iteration */
	return( FALSE );
}

/*
 * toggle mode can be undefined, collapse or expand
 * it is set on the first row
 */
static void
toggle_collapse_row( GtkTreeView *treeview, GtkTreePath *path, guint *toggle )
{
	if( *toggle == TOGGLE_UNDEFINED ){
		*toggle = gtk_tree_view_row_expanded( treeview, path ) ? TOGGLE_COLLAPSE : TOGGLE_EXPAND;
	}

	if( *toggle == TOGGLE_COLLAPSE ){
		if( gtk_tree_view_row_expanded( treeview, path )){
			gtk_tree_view_collapse_row( treeview, path );
		}
	} else {
		if( !gtk_tree_view_row_expanded( treeview, path )){
			gtk_tree_view_expand_row( treeview, path, TRUE );
		}
	}
}

static void
update_parents_edition_status( GList *parents, GList *items )
{
	static const gchar *thisfn = "cact_iactions_list_bis_update_parents_edition_status";
	GList *it;

	g_debug( "%s: parents=%p (count=%d), items=%p (count=%d)", thisfn,
			( void * ) parents, g_list_length( parents ),
			( void * ) items, g_list_length( items ));

	/*if( !parents || !g_list_length( parents )){
		parents = g_list_copy( items );
	}*/

	for( it = parents ; it ; it = it->next ){
		na_object_check_status_up( it->data );
	}

	g_list_free( parents );
}
