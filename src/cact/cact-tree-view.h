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

#ifndef __CACT_TREE_VIEW_H__
#define __CACT_TREE_VIEW_H__

/*
 * SECTION: cact-tree-view
 * @title: CactTreeView
 * @short_description: The Tree View Base Class Definition
 * @include: cact-tree-view.h
 *
 * This is a convenience class to manage a read-only items tree view.
 *
 * The CactTreeView encapsulates the GtkTreeView which displays the items
 * list on the left of the main pane.
 *
 * It is instanciated from CactMainWindow::on_initialize_gtk().
 *
 * A pointer to this CactTreeView is attached to the CactMainWindow at
 * construction time.
 */

#include <api/na-object-item.h>

#include "base-window.h"

G_BEGIN_DECLS

#define CACT_TYPE_TREE_VIEW                ( cact_tree_view_get_type())
#define CACT_TREE_VIEW( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_TREE_VIEW, CactTreeView ))
#define CACT_TREE_VIEW_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_TREE_VIEW, CactTreeViewClass ))
#define CACT_IS_TREE_VIEW( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_TREE_VIEW ))
#define CACT_IS_TREE_VIEW_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_TREE_VIEW ))
#define CACT_TREE_VIEW_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_TREE_VIEW, CactTreeViewClass ))

typedef struct _CactTreeViewPrivate        CactTreeViewPrivate;

typedef struct {
	/*< private >*/
	GObject              parent;
	CactTreeViewPrivate *private;
}
	CactTreeView;

typedef struct _CactTreeViewClassPrivate   CactTreeViewClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass              parent;
	CactTreeViewClassPrivate *private;
}
	CactTreeViewClass;

/**
 * Properties defined by the CactTreeView class.
 * They should be provided at object instantiation time.
 *
 * @TREE_PROP_WINDOW:         the BaseWindow.
 * @TREE_PROP_PARENT:         the widget which is parent of this tree view.
 * @TREE_PROP_WIDGET_NAME:    the tree view widget name.
 * @TREE_PROP_MODE:           management mode.
 * @TREE_PROP_NOTIFY_ALLOWED: whether notifications are allowed.
 */
#define TREE_PROP_WINDOW						"tree-prop-window"
#define TREE_PROP_PARENT						"tree-prop-parent"
#define TREE_PROP_WIDGET_NAME					"tree-prop-widget-name"
#define TREE_PROP_MODE							"tree-prop-mode"
#define TREE_PROP_NOTIFY_ALLOWED				"tree-prop-notify-allowed"

/**
 * Signals emitted by the CactTreeView instance.
 */
#define TREE_SIGNAL_COUNT_CHANGED				"tree-signal-count-changed"
#define TREE_SIGNAL_FOCUS_IN					"tree-signal-focus-in"
#define TREE_SIGNAL_FOCUS_OUT					"tree-signal-focus-out"
#define TREE_SIGNAL_LEVEL_ZERO_CHANGED			"tree-signal-level-zero-changed"
#define TREE_SIGNAL_MODIFIED_STATUS_CHANGED		"tree-signal-modified-status-changed"

typedef enum {
	TREE_MODE_EDITION = 0,
	TREE_MODE_SELECTION,
	/*< private >*/
	TREE_MODE_N_MODES
}
	CactTreeMode;

/**
 * When getting a list of items; these indcators may be OR-ed.
 */
enum {
	TREE_LIST_SELECTED = 1<<0,
	TREE_LIST_MODIFIED = 1<<1,
	TREE_LIST_ALL      = 1<<7,
	TREE_LIST_DELETED  = 1<<8,
};

/**
 * The CactTreeView is attached to the parent BaseWindow via a GObject data.
 * Only CactTreeView itself and CactTreeIEditable interface should use it.
 */
#define WINDOW_DATA_TREE_VIEW					"window-data-tree-view"

GType         cact_tree_view_get_type( void );

CactTreeView *cact_tree_view_new( BaseWindow *window, GtkContainer *parent, const gchar *treeview_name, CactTreeMode mode );

void          cact_tree_view_fill     ( CactTreeView *view, GList *items );

gboolean      cact_tree_view_are_notify_allowed( const CactTreeView *view );
void          cact_tree_view_set_notify_allowed( CactTreeView *view, gboolean allow );

void          cact_tree_view_collapse_all      ( const CactTreeView *view );
void          cact_tree_view_expand_all        ( const CactTreeView *view );
NAObjectItem *cact_tree_view_get_item_by_id    ( const CactTreeView *view, const gchar *id );
GList        *cact_tree_view_get_items         ( const CactTreeView *view );
GList        *cact_tree_view_get_items_ex      ( const CactTreeView *view, guint mode );

void          cact_tree_view_select_row_at_path( CactTreeView *view, GtkTreePath *path );

G_END_DECLS

#endif /* __CACT_TREE_VIEW_H__ */
