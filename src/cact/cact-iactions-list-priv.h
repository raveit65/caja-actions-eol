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

#ifndef __CACT_IACTIONS_LIST_PRIV_H__
#define __CACT_IACTIONS_LIST_PRIV_H__

#include <gtk/gtk.h>

#include "cact-iactions-list.h"

G_BEGIN_DECLS

/* data set against the GObject instance
 */
typedef struct {

	/* management mode
	 */
	gint     management_mode;

	/* counters
	 * initialized when filling the list, updated on insert/delete
	 */
	gint     menus;
	gint     actions;
	gint     profiles;

	/* signal management
	 */
	gboolean selection_changed_allowed;
	gulong   tab_updated_handler;

	/* maintains a flat list of modified objects
	 * should be faster than iterating each time this is needed
	 * not very reliable as not all deleted items are removed from it
	 * but enough to enable/disable save
	 */
	GList   *modified_items;
}
	IActionsListInstanceData;

GtkTreeView              *cact_iactions_list_priv_get_actions_list_treeview( CactIActionsList *instance );
IActionsListInstanceData *cact_iactions_list_priv_get_instance_data( CactIActionsList *instance );
void                      cact_iactions_list_priv_send_list_count_updated_signal( CactIActionsList *instance, IActionsListInstanceData *ialid );

G_END_DECLS

#endif /* __CACT_IACTIONS_LIST_PRIV_H__ */
