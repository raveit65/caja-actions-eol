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

#ifndef __CACT_TREE_TREE_IEDITABLE_H__
#define __CACT_TREE_TREE_IEDITABLE_H__

/**
 * SECTION: cact-tree_ieditable
 * @title: CactTreeIEditable
 * @short_description: The CactTreeIEditable interface definition
 * @include: cact-tree_ieditable.h
 *
 * This interface is to be implemented by a CactTreeView which would
 * want get edition features, such as inline edition, insert, delete,
 * and so on.
 *
 * CactTreeIEditable maintains the count of modified items.
 * Starting with zero when the tree view is filled up, it is incremented
 * each time an item is modified, inserted or deleted.
 * The modified count is fully recomputed after a save.
 */

#include <api/na-object.h>

#include "base-window.h"

G_BEGIN_DECLS

#define CACT_TREE_IEDITABLE_TYPE                      ( cact_tree_ieditable_get_type())
#define CACT_TREE_IEDITABLE( object )                 ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TREE_IEDITABLE_TYPE, CactTreeIEditable ))
#define CACT_IS_TREE_IEDITABLE( object )              ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TREE_IEDITABLE_TYPE ))
#define CACT_TREE_IEDITABLE_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_TREE_IEDITABLE_TYPE, CactTreeIEditableInterface ))

typedef struct _CactTreeIEditable                     CactTreeIEditable;
typedef struct _CactTreeIEditableInterfacePrivate     CactTreeIEditableInterfacePrivate;

typedef struct {
	/*< private >*/
	GTypeInterface                 parent;
	CactTreeIEditableInterfacePrivate *private;
}
	CactTreeIEditableInterface;

/**
 * Delete operations
 */
typedef enum {
	TREE_OPE_DELETE = 0,
	TREE_OPE_MOVE
}
	TreeIEditableDeleteOpe;

GType    cact_tree_ieditable_get_type( void );

void     cact_tree_ieditable_initialize    ( CactTreeIEditable *instance, GtkTreeView *treeview, BaseWindow *window );
void     cact_tree_ieditable_terminate     ( CactTreeIEditable *instance );

void     cact_tree_ieditable_delete        ( CactTreeIEditable *instance, GList *items, TreeIEditableDeleteOpe ope );
gboolean cact_tree_ieditable_remove_deleted( CactTreeIEditable *instance, GSList **messages );
GList   *cact_tree_ieditable_get_deleted   ( CactTreeIEditable *instance );

void     cact_tree_ieditable_insert_items  ( CactTreeIEditable *instance, GList *items, NAObject *sibling );
void     cact_tree_ieditable_insert_at_path( CactTreeIEditable *instance, GList *items, GtkTreePath *path );
void     cact_tree_ieditable_insert_into   ( CactTreeIEditable *instance, GList *items );

void     cact_tree_ieditable_set_items     ( CactTreeIEditable *instance, GList *items );

void     cact_tree_ieditable_dump_modified         ( const CactTreeIEditable *instance );
gboolean cact_tree_ieditable_is_level_zero_modified( const CactTreeIEditable *instance );

G_END_DECLS

#endif /* __CACT_TREE_TREE_IEDITABLE_H__ */
