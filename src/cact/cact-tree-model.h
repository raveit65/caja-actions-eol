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

#include <api/na-object.h>

#include "cact-tree-view.h"

G_BEGIN_DECLS

#define CACT_TYPE_TREE_MODEL                ( cact_tree_model_get_type())
#define CACT_TREE_MODEL( object )           ( G_TYPE_CHECK_INSTANCE_CAST(( object ), CACT_TYPE_TREE_MODEL, CactTreeModel ))
#define CACT_TREE_MODEL_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST(( klass ), CACT_TYPE_TREE_MODEL, CactTreeModelClass ))
#define CACT_IS_TREE_MODEL( object )        ( G_TYPE_CHECK_INSTANCE_TYPE(( object ), CACT_TYPE_TREE_MODEL ))
#define CACT_IS_TREE_MODEL_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_TREE_MODEL ))
#define CACT_TREE_MODEL_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_TREE_MODEL, CactTreeModelClass ))

typedef struct _CactTreeModelPrivate        CactTreeModelPrivate;

typedef struct {
	/*< private >*/
	GtkTreeModelFilter    parent;
	CactTreeModelPrivate *private;
}
	CactTreeModel;

typedef struct _CactTreeModelClassPrivate   CactTreeModelClassPrivate;

typedef struct {
	/*< private >*/
	GtkTreeModelFilterClass    parent;
	CactTreeModelClassPrivate *private;
}
	CactTreeModelClass;

/**
 * Column ordering in the tree view
 */
enum {
	TREE_COLUMN_ICON = 0,
	TREE_COLUMN_LABEL,
	TREE_COLUMN_NAOBJECT,
	TREE_N_COLUMN
};

GType          cact_tree_model_get_type( void );

CactTreeModel *cact_tree_model_new( BaseWindow *window, GtkTreeView *view, CactTreeMode mode );

GtkTreePath   *cact_tree_model_delete       ( CactTreeModel *model, NAObject *object );
void           cact_tree_model_fill         ( CactTreeModel *model, GList *items );
GtkTreePath   *cact_tree_model_insert_before( CactTreeModel *model, const NAObject *object, GtkTreePath *path );
GtkTreePath   *cact_tree_model_insert_into  ( CactTreeModel *model, const NAObject *object, GtkTreePath *path );

NAObjectItem  *cact_tree_model_get_item_by_id( const CactTreeModel *model, const gchar *id );
GList         *cact_tree_model_get_items     ( const CactTreeModel *model, guint mode );
NAObject      *cact_tree_model_object_at_path( const CactTreeModel *model, GtkTreePath *path );
GtkTreePath   *cact_tree_model_object_to_path( const CactTreeModel *model, const NAObject *object );

G_END_DECLS

#endif /* __CACT_TREE_MODEL_H__ */
