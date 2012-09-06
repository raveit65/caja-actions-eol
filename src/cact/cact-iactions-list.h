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

#ifndef __CACT_IACTIONS_LIST_H__
#define __CACT_IACTIONS_LIST_H__

/**
 * SECTION: cact_iactions_list
 * @short_description: #CactIActionsList interface definition.
 * @include: cact/cact-iactions-list.h
 *
 * This same interface is used in the main window (edition mode, default),
 * and in the export assistant (export mode).
 *
 * Counting rows
 *
 *   Counting rows is needed to maintain action sensitivities in the
 *   menubar : at least 'Tools\Export' menu item depends of the content
 *   of the IActionsList.
 *   Rows are first counted when the treeview is primarily filled, or
 *   refilled on demand.
 *   Counters are then incremented in cact_iactions_list_insert() and
 *   cact_iactions_list_delete() functions.
 */

#include <gtk/gtk.h>

#include <api/na-object-item.h>

G_BEGIN_DECLS

#define CACT_IACTIONS_LIST_TYPE							( cact_iactions_list_get_type())
#define CACT_IACTIONS_LIST( object )					( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_IACTIONS_LIST_TYPE, CactIActionsList ))
#define CACT_IS_IACTIONS_LIST( object )					( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_IACTIONS_LIST_TYPE ))
#define CACT_IACTIONS_LIST_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_IACTIONS_LIST_TYPE, CactIActionsListInterface ))

typedef struct CactIActionsList CactIActionsList;

typedef struct CactIActionsListInterfacePrivate CactIActionsListInterfacePrivate;

typedef struct {
	GTypeInterface                    parent;
	CactIActionsListInterfacePrivate *private;

	/**
	 * get_treeview_name:
	 * @instance: this #CactIActionsList implementor.
	 *
	 * Returns: a newly allocated string, which contains the treeview
	 * widget name in its XML UI definition.
	 *
	 * The returned string will be g_free() by IActionsList interface.
	 *
	 * This is a pure virtual function which must be implemented.
	 */
	gchar * ( *get_treeview_name )( CactIActionsList *instance );
}
	CactIActionsListInterface;

/* signals
 */
#define IACTIONS_LIST_SIGNAL_LIST_COUNT_UPDATED			"cact-iactions-list-count-updated"
#define IACTIONS_LIST_SIGNAL_SELECTION_CHANGED			"cact-iactions-list-selection-changed"
#define IACTIONS_LIST_SIGNAL_FOCUS_IN					"cact-iactions-list-focus-in"
#define IACTIONS_LIST_SIGNAL_FOCUS_OUT					"cact-iactions-list-focus-out"
#define IACTIONS_LIST_SIGNAL_COLUMN_EDITED				"cact-iactions-list-column-edited"
#define IACTIONS_LIST_SIGNAL_STATUS_CHANGED				"cact-iactions-list-status-changed"

/* management modes
 * - edition: dnd, filter, multiple selection, item updated signal
 * - export: multiple selection
 */
enum {
	IACTIONS_LIST_MANAGEMENT_MODE_EDITION = 1,
	IACTIONS_LIST_MANAGEMENT_MODE_EXPORT
};

GType     cact_iactions_list_get_type( void );

void      cact_iactions_list_initial_load_toplevel( CactIActionsList *instance );
void      cact_iactions_list_runtime_init_toplevel( CactIActionsList *instance, GList *actions );
void      cact_iactions_list_all_widgets_showed( CactIActionsList *instance );
void      cact_iactions_list_dispose( CactIActionsList *instance );

void      cact_iactions_list_brief_tree_dump( CactIActionsList *instance );
void      cact_iactions_list_collapse_all( CactIActionsList *instance );
void      cact_iactions_list_display_order_change( CactIActionsList *instance, gint order_mode );
void      cact_iactions_list_expand_all( CactIActionsList *instance );
void      cact_iactions_list_fill( CactIActionsList *instance, GList *items );
gint      cact_iactions_list_get_management_mode( CactIActionsList *instance );
gboolean  cact_iactions_list_has_modified_items( CactIActionsList *instance );
void      cact_iactions_list_on_treeview_selection_changed( GtkTreeSelection *selection, CactIActionsList *instance );
GList    *cact_iactions_list_remove_rec( GList *list, NAObject *object );
void      cact_iactions_list_set_management_mode( CactIActionsList *instance, gint mode );

void      cact_iactions_list_bis_clear_selection( CactIActionsList *instance, GtkTreeView *treeview );
void      cact_iactions_list_bis_collapse_to_parent( CactIActionsList *instance );
void      cact_iactions_list_bis_delete( CactIActionsList *instance, GList *items, gboolean select_at_end );
void      cact_iactions_list_bis_expand_to_first_child( CactIActionsList *instance );
NAObject *cact_iactions_list_bis_get_item( CactIActionsList *instance, const gchar *uuid );
GList    *cact_iactions_list_bis_get_items( CactIActionsList *instance );
GList    *cact_iactions_list_bis_get_selected_items( CactIActionsList *instance );
void      cact_iactions_list_bis_insert_at_path( CactIActionsList *instance, GList *items, GtkTreePath *path );
void      cact_iactions_list_bis_insert_items( CactIActionsList *instance, GList *items, NAObject *sibling );
void      cact_iactions_list_bis_insert_into( CactIActionsList *instance, GList *items );
void      cact_iactions_list_bis_list_modified_items( CactIActionsList *instance );
void      cact_iactions_list_bis_remove_modified( CactIActionsList *instance, const NAObjectItem *item );
void      cact_iactions_list_bis_select_first_row( CactIActionsList *instance );
void      cact_iactions_list_bis_select_row_at_path( CactIActionsList *instance, GtkTreeView *treeview, GtkTreeModel *model, GtkTreePath *path );
void      cact_iactions_list_bis_toggle_collapse( CactIActionsList *instance );
void      cact_iactions_list_bis_toggle_collapse_object( CactIActionsList *instance, const NAObject *item );

G_END_DECLS

#endif /* __CACT_IACTIONS_LIST_H__ */
