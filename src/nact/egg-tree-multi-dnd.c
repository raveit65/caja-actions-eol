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

/* eggtreemultidnd.c
 * Copyright (C) 2001  Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "egg-tree-multi-dnd.h"

#define EGG_TREE_MULTI_DND_STRING		"EggTreeMultiDndString"

typedef struct
{
	guint    pressed_button;
	gint     x;
	gint     y;
	guint    motion_notify_handler;
	guint    button_release_handler;
	guint    drag_data_get_handler;
	GSList  *event_list;
	gboolean pending_event;
}
	EggTreeMultiDndData;

static GType          register_type( void );

static GtkTargetList *v_get_target_list( EggTreeMultiDragSource *drag_source );
static void           v_free_target_list( EggTreeMultiDragSource *drag_source, GtkTargetList *list );
static GdkDragAction  v_get_drag_actions( EggTreeMultiDragSource *drag_source );

static gboolean       on_button_press_event( GtkWidget *widget, GdkEventButton *event, EggTreeMultiDragSource *drag_source );
static gboolean       on_button_release_event( GtkWidget *widget, GdkEventButton *event, EggTreeMultiDragSource *drag_source );
static gboolean       on_motion_event( GtkWidget *widget, GdkEventMotion *event, EggTreeMultiDragSource *drag_source );
static gboolean       on_drag_data_get( GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time );

static void           stop_drag_check( GtkWidget *widget );
static void           selection_foreach( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GList **path_list );
static void           path_list_free( GList *path_list );
static void           set_treeview_data( GtkWidget *treeview, GList *path_list );
static GList         *get_treeview_data( GtkWidget *treeview );

GType
egg_tree_multi_drag_source_get_type( void )
{
	static GType our_type = 0;

	if( !our_type ){
		our_type = register_type();
	}

	return( our_type );
}

static GType
register_type( void )
{
	GType type;

	static const GTypeInfo our_info = {
			sizeof( EggTreeMultiDragSourceIface ),	/* class_size */
			NULL,									/* base_init */
			NULL,									/* base_finalize */
			NULL,
			NULL,									/* class_finalize */
			NULL,									/* class_data */
			0,
			0,										/* n_preallocs */
			NULL
	};

	type = g_type_register_static( G_TYPE_INTERFACE, "EggTreeMultiDragSource", &our_info, 0 );

    return( type );
}

/**
 * egg_tree_multi_drag_add_drag_support:
 */
void
egg_tree_multi_drag_add_drag_support( EggTreeMultiDragSource *drag_source, GtkTreeView *tree_view )
{
	g_return_if_fail( GTK_IS_TREE_VIEW( tree_view ));

	g_signal_connect( G_OBJECT( tree_view ),
			"button_press_event",
			G_CALLBACK( on_button_press_event),
			drag_source );
}

/**
 * egg_tree_multi_drag_source_row_draggable:
 * @drag_source: a #EggTreeMultiDragSource
 * @path: row on which user is initiating a drag
 *
 * Asks the #EggTreeMultiDragSource whether a particular row can be used as
 * the source of a DND operation. If the source doesn't implement
 * this interface, the row is assumed draggable.
 *
 * Return value: %TRUE if the row can be dragged
 **/
gboolean
egg_tree_multi_drag_source_row_draggable( EggTreeMultiDragSource *drag_source, GList *path_list )
{
	EggTreeMultiDragSourceIface *iface = EGG_TREE_MULTI_DRAG_SOURCE_GET_IFACE( drag_source );

	g_return_val_if_fail( EGG_IS_TREE_MULTI_DRAG_SOURCE( drag_source ), FALSE );
	g_return_val_if_fail( iface->row_draggable != NULL, FALSE );
	g_return_val_if_fail( path_list != NULL, FALSE );

	return(( *iface->row_draggable )( drag_source, path_list ));
}

/**
 * egg_tree_multi_drag_source_drag_data_get:
 * @drag_source: a #EggTreeMultiDragSource
 * @path: row that was dragged
 * @selection_data: a #EggSelectionData to fill with data from the dragged row
 *
 * Asks the #EggTreeMultiDragSource to fill in @selection_data with a
 * representation of the row at @path. @selection_data->target gives
 * the required type of the data.  Should robustly handle a @path no
 * longer found in the model!
 *
 * Return value: %TRUE if data of the required type was provided
 **/
gboolean
egg_tree_multi_drag_source_drag_data_get( EggTreeMultiDragSource *drag_source,
											GdkDragContext   *context,
											GtkSelectionData *selection_data,
											GList            *path_list,
											guint             info )
{
	EggTreeMultiDragSourceIface *iface = EGG_TREE_MULTI_DRAG_SOURCE_GET_IFACE( drag_source );

	g_return_val_if_fail( EGG_IS_TREE_MULTI_DRAG_SOURCE( drag_source ), FALSE );
	g_return_val_if_fail( iface->drag_data_get != NULL, FALSE );
	g_return_val_if_fail( path_list != NULL, FALSE );
	g_return_val_if_fail( selection_data != NULL, FALSE );

	return(( *iface->drag_data_get )( drag_source, context, selection_data, path_list, info ));
}

/**
 * egg_tree_multi_drag_source_drag_data_delete:
 * @drag_source: a #EggTreeMultiDragSource
 * @path: row that was being dragged
 *
 * Asks the #EggTreeMultiDragSource to delete the row at @path, because
 * it was moved somewhere else via drag-and-drop. Returns %FALSE
 * if the deletion fails because @path no longer exists, or for
 * some model-specific reason. Should robustly handle a @path no
 * longer found in the model!
 *
 * Return value: %TRUE if the row was successfully deleted
 **/
gboolean
egg_tree_multi_drag_source_drag_data_delete( EggTreeMultiDragSource *drag_source, GList *path_list )
{
	EggTreeMultiDragSourceIface *iface = EGG_TREE_MULTI_DRAG_SOURCE_GET_IFACE( drag_source );

	g_return_val_if_fail( EGG_IS_TREE_MULTI_DRAG_SOURCE( drag_source ), FALSE );
	g_return_val_if_fail( iface->drag_data_delete != NULL, FALSE );
	g_return_val_if_fail( path_list != NULL, FALSE );

  return(( *iface->drag_data_delete )( drag_source, path_list ));
}

static GtkTargetList *
v_get_target_list( EggTreeMultiDragSource *drag_source )
{
	EggTreeMultiDragSourceIface *iface = EGG_TREE_MULTI_DRAG_SOURCE_GET_IFACE( drag_source );

	if( iface->get_target_list ){
		return( iface->get_target_list( drag_source ));
	}

	return( NULL );
}

static void
v_free_target_list( EggTreeMultiDragSource *drag_source, GtkTargetList *list )
{
	EggTreeMultiDragSourceIface *iface = EGG_TREE_MULTI_DRAG_SOURCE_GET_IFACE( drag_source );

	if( iface->free_target_list ){
		iface->free_target_list( drag_source, list );

	} else {
		gtk_target_list_unref( list );
	}
}

static GdkDragAction
v_get_drag_actions( EggTreeMultiDragSource *drag_source )
{
	EggTreeMultiDragSourceIface *iface = EGG_TREE_MULTI_DRAG_SOURCE_GET_IFACE( drag_source );

	if( iface->get_drag_actions ){
		return( iface->get_drag_actions( drag_source ));
	}

	return( 0 );
}

static gboolean
on_button_press_event( GtkWidget *widget, GdkEventButton *event, EggTreeMultiDragSource *drag_source )
{
	GtkTreeView         *tree_view;
	GtkTreePath         *path = NULL;
	GtkTreeViewColumn   *column = NULL;
	gint                 cell_x, cell_y;
	GtkTreeSelection    *selection;
	EggTreeMultiDndData *priv_data;
	gboolean             call_parent;

	if( event->window != gtk_tree_view_get_bin_window( GTK_TREE_VIEW( widget ))){
		return( FALSE );
	}

	if( event->button == 3 ){
		return( FALSE );
	}

	tree_view = GTK_TREE_VIEW( widget );
	priv_data = g_object_get_data( G_OBJECT( tree_view ), EGG_TREE_MULTI_DND_STRING );
	if( priv_data == NULL ){
		priv_data = g_new0( EggTreeMultiDndData, 1 );
		priv_data->pending_event = FALSE;
		g_object_set_data( G_OBJECT( tree_view ), EGG_TREE_MULTI_DND_STRING, priv_data );
	}

	if( g_slist_find( priv_data->event_list, event )){
		return( FALSE );
	}

	if( priv_data->pending_event ){
		/* save the event to be propagated in order */
		priv_data->event_list = g_slist_append( priv_data->event_list, gdk_event_copy(( GdkEvent * ) event ));
		return( TRUE );
	}

	if( event->type == GDK_2BUTTON_PRESS ){
		return( FALSE );
	}

	gtk_tree_view_get_path_at_pos( tree_view, event->x, event->y, &path, &column, &cell_x, &cell_y );

	if( path ){
		selection = gtk_tree_view_get_selection( tree_view );

		call_parent = ( event->state & ( GDK_CONTROL_MASK | GDK_SHIFT_MASK ) ||
				!gtk_tree_selection_path_is_selected( selection, path) ||
				event->button != 1 );

		if( call_parent ){
			( GTK_WIDGET_GET_CLASS( tree_view ))->button_press_event( widget, event );
		}

		if( gtk_tree_selection_path_is_selected( selection, path )){
			priv_data->pressed_button = event->button;
			priv_data->x = event->x;
			priv_data->y = event->y;
			priv_data->pending_event = TRUE;

			if( !call_parent ){
				priv_data->event_list = g_slist_append( priv_data->event_list, gdk_event_copy(( GdkEvent * ) event ));
			}

			priv_data->motion_notify_handler =
				g_signal_connect( G_OBJECT( tree_view ),
						"motion_notify_event",
						G_CALLBACK( on_motion_event ),
						drag_source );

			priv_data->button_release_handler =
				g_signal_connect( G_OBJECT( tree_view ),
						"button_release_event",
						G_CALLBACK( on_button_release_event ),
						drag_source );

			if( priv_data->drag_data_get_handler == 0 ){
				priv_data->drag_data_get_handler =
					g_signal_connect( G_OBJECT( tree_view ),
						"drag_data_get",
						G_CALLBACK( on_drag_data_get ),
						NULL );
			}
		}

		gtk_tree_path_free (path);

		/* We called the default handler so we don't let the default handler run */
		return( TRUE );
	}

	return( FALSE );
}

static gboolean
on_button_release_event( GtkWidget *widget, GdkEventButton *event, EggTreeMultiDragSource *drag_source )
{
	EggTreeMultiDndData *priv_data;
	GSList *l;

	priv_data = g_object_get_data( G_OBJECT( widget ), EGG_TREE_MULTI_DND_STRING );

	for( l = priv_data->event_list ; l != NULL ; l = l->next ){
		gtk_propagate_event( widget, l->data );
	}

	stop_drag_check( widget );

	return( FALSE );
}

static gboolean
on_motion_event( GtkWidget *widget, GdkEventMotion *event, EggTreeMultiDragSource *drag_source )
{
	EggTreeMultiDndData *priv_data;

	priv_data = g_object_get_data( G_OBJECT( widget ), EGG_TREE_MULTI_DND_STRING );

	if( gtk_drag_check_threshold( widget, priv_data->x, priv_data->y, event->x, event->y )){

		GList            *path_list = NULL;
		GtkTreeSelection *selection;
		GtkTreeModel     *model;
		GdkDragContext   *context;

		stop_drag_check( widget );

		selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ));
		gtk_tree_selection_selected_foreach( selection, ( GtkTreeSelectionForeachFunc ) selection_foreach, &path_list );
		path_list = g_list_reverse( path_list );

		model = gtk_tree_view_get_model( GTK_TREE_VIEW( widget ));

		if( egg_tree_multi_drag_source_row_draggable( EGG_TREE_MULTI_DRAG_SOURCE( model ), path_list )){

			GtkTargetList *target_list = v_get_target_list( drag_source );
			GdkDragAction actions = v_get_drag_actions( drag_source );

			context = gtk_drag_begin(
					widget, target_list, actions, priv_data->pressed_button, ( GdkEvent * ) event );

			set_treeview_data( widget, path_list );

			gtk_drag_set_icon_default( context );

			v_free_target_list( drag_source, target_list );

		} else {
			path_list_free( path_list );
		}
	}

	return( TRUE );
}

static gboolean
on_drag_data_get( GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time )
{
	static const gchar *thisfn = "egg_tree_multi_dnd_on_drag_data_get";
	GtkTreeView  *tree_view;
	GtkTreeModel *model;
	GList        *path_list;

	g_debug( "%s: widget=%p, context=%p, selection_data=%p, info=%d, time=%d",
			thisfn, ( void * ) widget, ( void * ) context, ( void * ) selection_data, info, time );

	tree_view = GTK_TREE_VIEW( widget );
	model = gtk_tree_view_get_model( tree_view );
	g_assert( model );
	g_assert( EGG_IS_TREE_MULTI_DRAG_SOURCE( model ));

	path_list = get_treeview_data( widget );
	if( path_list == NULL ){
		return( FALSE );
	}

	/* We can implement the GTK_TREE_MODEL_ROW target generically for
	 * any model; for DragSource models there are some other targets
	 * we also support.
	 */
  return( egg_tree_multi_drag_source_drag_data_get(
		  EGG_TREE_MULTI_DRAG_SOURCE( model ), context, selection_data, path_list, info ));
}

static void
stop_drag_check( GtkWidget *widget )
{
	EggTreeMultiDndData *priv_data;
	GSList *l;

	priv_data = g_object_get_data( G_OBJECT( widget ), EGG_TREE_MULTI_DND_STRING );

	for( l = priv_data->event_list ; l != NULL ; l = l->next ){
		gdk_event_free( l->data );
	}

	g_slist_free( priv_data->event_list );
	priv_data->event_list = NULL;
	priv_data->pending_event = FALSE;

	g_signal_handler_disconnect( widget, priv_data->motion_notify_handler );
	g_signal_handler_disconnect( widget, priv_data->button_release_handler );
}

static void
selection_foreach( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GList **path_list )
{
	*path_list = g_list_prepend( *path_list, gtk_tree_row_reference_new( model, path ));
}

static void
path_list_free( GList *path_list )
{
	g_list_foreach( path_list, ( GFunc ) gtk_tree_row_reference_free, NULL );
	g_list_free( path_list );
}

static void
set_treeview_data( GtkWidget *treeview, GList *path_list )
{
	g_object_set_data_full(
			G_OBJECT( treeview ),
			"egg-tree-view-multi-source-row", path_list, ( GDestroyNotify ) path_list_free );
}

static GList *
get_treeview_data( GtkWidget *treeview )
{
	return( g_object_get_data( G_OBJECT( treeview ), "egg-tree-view-multi-source-row" ));
}
