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

/* eggtreednd.h
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
 *
 * Adapted by Pierre Wieser for the needs of Caja Actions
 */

#ifndef __EGG_TREE_MULTI_DND_H__
#define __EGG_TREE_MULTI_DND_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_TREE_MULTI_DRAG_SOURCE					( egg_tree_multi_drag_source_get_type())
#define EGG_TREE_MULTI_DRAG_SOURCE( object )			( G_TYPE_CHECK_INSTANCE_CAST(( object ), EGG_TYPE_TREE_MULTI_DRAG_SOURCE, EggTreeMultiDragSource ))
#define EGG_IS_TREE_MULTI_DRAG_SOURCE( object )			( G_TYPE_CHECK_INSTANCE_TYPE(( object ), EGG_TYPE_TREE_MULTI_DRAG_SOURCE ))
#define EGG_TREE_MULTI_DRAG_SOURCE_GET_IFACE( object )	( G_TYPE_INSTANCE_GET_INTERFACE(( object ), EGG_TYPE_TREE_MULTI_DRAG_SOURCE, EggTreeMultiDragSourceIface ))

typedef struct EggTreeMultiDragSource      EggTreeMultiDragSource;
typedef struct EggTreeMultiDragSourceIface EggTreeMultiDragSourceIface;

struct EggTreeMultiDragSourceIface
{
	GTypeInterface g_iface;

	/* vtable - not signals */
	gboolean        ( *row_draggable )   ( EggTreeMultiDragSource *drag_source, GList *path_list );
	gboolean        ( *drag_data_get )   ( EggTreeMultiDragSource *drag_source, GdkDragContext *context, GtkSelectionData *selection_data, GList *path_list, guint info );
	gboolean        ( *drag_data_delete )( EggTreeMultiDragSource *drag_source, GList *path_list );
	GtkTargetList * ( *get_target_list ) ( EggTreeMultiDragSource *drag_source );
	void            ( *free_target_list )( EggTreeMultiDragSource *drag_source, GtkTargetList *list );
	GdkDragAction   ( *get_drag_actions )( EggTreeMultiDragSource *drag_source );
};

GType    egg_tree_multi_drag_source_get_type( void ) G_GNUC_CONST;

/* initialize drag support on the treeview */
void     egg_tree_multi_drag_add_drag_support( EggTreeMultiDragSource *drag_source, GtkTreeView *tree_view );

/* returns whether the given row can be dragged */
gboolean egg_tree_multi_drag_source_row_draggable( EggTreeMultiDragSource *drag_source, GList *path_list );

/* Fills in selection_data with type selection_data->target based on the row
 * denoted by path, returns TRUE if it does anything
 */
gboolean egg_tree_multi_drag_source_drag_data_get( EggTreeMultiDragSource *drag_source, GdkDragContext *context, GtkSelectionData *selection_data, GList *path_list, guint info );

/* deletes the given row, or returns FALSE if it can't */
gboolean egg_tree_multi_drag_source_drag_data_delete( EggTreeMultiDragSource *drag_source, GList *path_list );

G_END_DECLS

#endif /* __EGG_TREE_MULTI_DND_H__ */
