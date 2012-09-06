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

/**
 * SECTION: cact_tree_model
 * @short_description: #CactTreeModel class definition.
 * @include: cact/cact-tree-model.h
 *
 * CactTreeModel is derived from GtkTreeModelFilter in order to be able
 * to selectively display profiles, whether an action has one or more
 * profiles.
 *
 * CactTreeModel implements EggTreeMultiDragSource and GtkTreeDragDest
 * interfaces.
 *
 * The GtkTreeModelFilter base class embeds a GtkTreeStore.
 */
#ifndef __CACT_TREE_MODEL_H__
#define __CACT_TREE_MODEL_H__

#include <gtk/gtk.h>

#include <api/na-object.h>

#include "base-window.h"

G_BEGIN_DECLS

#define CACT_TREE_MODEL_TYPE				( cact_tree_model_get_type())
#define CACT_TREE_MODEL( object )			( G_TYPE_CHECK_INSTANCE_CAST(( object ), CACT_TREE_MODEL_TYPE, CactTreeModel ))
#define CACT_TREE_MODEL_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST(( klass ), CACT_TREE_MODEL_TYPE, CactTreeModelClass ))
#define CACT_IS_TREE_MODEL( object )		( G_TYPE_CHECK_INSTANCE_TYPE(( object ), CACT_TREE_MODEL_TYPE ))
#define CACT_IS_TREE_MODEL_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TREE_MODEL_TYPE ))
#define CACT_TREE_MODEL_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TREE_MODEL_TYPE, CactTreeModelClass ))

typedef struct CactTreeModelPrivate CactTreeModelPrivate;

typedef struct {
	GtkTreeModelFilter    parent;
	CactTreeModelPrivate *private;
}
	CactTreeModel;

typedef struct CactTreeModelClassPrivate CactTreeModelClassPrivate;

typedef struct {
	GtkTreeModelFilterClass    parent;
	CactTreeModelClassPrivate *private;
}
	CactTreeModelClass;

/* column ordering of the tree view
 */
enum {
	IACTIONS_LIST_ICON_COLUMN = 0,
	IACTIONS_LIST_LABEL_COLUMN,
	IACTIONS_LIST_NAOBJECT_COLUMN,
	IACTIONS_LIST_N_COLUMN
};

/* iter on tree store
 */
typedef gboolean ( *FnIterOnStore )( CactTreeModel *, GtkTreePath *, NAObject *, gpointer );

GType        cact_tree_model_get_type( void );

void         cact_tree_model_initial_load( BaseWindow *window, GtkTreeView *treeview );
void         cact_tree_model_runtime_init( CactTreeModel *model, gboolean have_dnd );
void         cact_tree_model_dispose( CactTreeModel *model );

void         cact_tree_model_display( CactTreeModel *model, NAObject *object );
void         cact_tree_model_display_order_change( CactTreeModel *model, gint order_mode );
void         cact_tree_model_dump( CactTreeModel *model );
void         cact_tree_model_fill( CactTreeModel *model, GList *items, gboolean are_profiles_displayed );
GtkTreePath *cact_tree_model_insert( CactTreeModel *model, const NAObject *object, GtkTreePath *path, NAObject **parent );
GtkTreePath *cact_tree_model_insert_into( CactTreeModel *model, const NAObject *object, GtkTreePath *path, NAObject **parent );
void         cact_tree_model_iter( CactTreeModel *model, FnIterOnStore fn, gpointer user_data );
NAObject    *cact_tree_model_object_at_path( CactTreeModel *model, GtkTreePath *path );
GtkTreePath *cact_tree_model_remove( CactTreeModel *model, NAObject *object );

G_END_DECLS

#endif /* __CACT_TREE_MODEL_H__ */
