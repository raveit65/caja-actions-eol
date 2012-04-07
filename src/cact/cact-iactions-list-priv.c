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

#include "base-window.h"
#include "cact-iactions-list-priv.h"

#define IACTIONS_LIST_DATA_INSTANCE		"cact-iactions-list-instance-data"

static gchar *v_get_treeview_name( CactIActionsList *instance );

GtkTreeView *
cact_iactions_list_priv_get_actions_list_treeview( CactIActionsList *instance )
{
	gchar *widget_name;
	GtkTreeView *treeview = NULL;

	widget_name = v_get_treeview_name( instance );
	if( widget_name ){
		treeview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), widget_name ));
		g_return_val_if_fail( GTK_IS_TREE_VIEW( treeview ), NULL );
		g_free( widget_name );
	}

	return( treeview );
}

/**
 * cact_iactions_list_priv_get_instance_data:
 * @instance: this #CactIActionsList interface.
 *
 * Returns a pointer to the data associated to this instance of the interface.
 */
IActionsListInstanceData *
cact_iactions_list_priv_get_instance_data( CactIActionsList *instance )
{
	IActionsListInstanceData *ialid;

	ialid = ( IActionsListInstanceData * ) g_object_get_data( G_OBJECT( instance ), IACTIONS_LIST_DATA_INSTANCE );

	if( !ialid ){
		ialid = g_new0( IActionsListInstanceData, 1 );
		g_object_set_data( G_OBJECT( instance ), IACTIONS_LIST_DATA_INSTANCE, ialid );
	}

	return( ialid );
}

/**
 * cact_iactions_list_priv_send_list_count_updated_signal:
 * @instance: this instance of #CactIActionsList interface.
 * @ialid: the internal data structure.
 *
 * Send a 'fill' signal with count of items.
 */
void
cact_iactions_list_priv_send_list_count_updated_signal( CactIActionsList *instance, IActionsListInstanceData *ialid )
{
	g_signal_emit_by_name( instance,
							IACTIONS_LIST_SIGNAL_LIST_COUNT_UPDATED,
							ialid->menus, ialid->actions, ialid->profiles );
}

static gchar *
v_get_treeview_name( CactIActionsList *instance )
{
	gchar *name = NULL;

	if( CACT_IACTIONS_LIST_GET_INTERFACE( instance )->get_treeview_name ){
		name = CACT_IACTIONS_LIST_GET_INTERFACE( instance )->get_treeview_name( instance );
	}

	return( name );
}
