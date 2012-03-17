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

#include "nact-iactions-list.h"
#include "nact-clipboard.h"
#include "nact-main-menubar.h"
#include "nact-main-menubar-maintainer.h"

/**
 * nact_main_menubar_maintainer_on_update_sensitivities:
 * @window: the #NactMainWindow main application window.
 * @user_data: user data ?
 * @mis: the #MenubarIndicatorsStruct structure.
 *
 * Update sensitivities on the Maintainer menu.
 */
void
nact_main_menubar_maintainer_on_update_sensitivities( NactMainWindow *window, gpointer user_data, MenubarIndicatorsStruct *mis )
{
}

/**
 * nact_main_menubar_maintainer_on_dump_selection:
 * @action: the #GtkAction of the item.
 * @window: the #NactMainWindow main application window.
 *
 * Triggers the "Maintainer/Dump selection" item.
 */
void
nact_main_menubar_maintainer_on_dump_selection( GtkAction *action, NactMainWindow *window )
{
	GList *items, *it;

	items = nact_iactions_list_bis_get_selected_items( NACT_IACTIONS_LIST( window ));
	for( it = items ; it ; it = it->next ){
		na_object_dump( it->data );
	}

	na_object_unref_selected_items( items );
}

/**
 * nact_main_menubar_maintainer_on_brief_tree_store_dump:
 * @action: the #GtkAction of the item.
 * @window: the #NactMainWindow main application window.
 *
 * Triggers the "Maintainer/Brief treestore dump" item.
 */
void
nact_main_menubar_maintainer_on_brief_tree_store_dump( GtkAction *action, NactMainWindow *window )
{
	nact_iactions_list_brief_tree_dump( NACT_IACTIONS_LIST( window ));
}

/**
 * nact_main_menubar_maintainer_on_list_modified_items:
 * @action: the #GtkAction of the item.
 * @window: the #NactMainWindow main application window.
 *
 * Triggers the "Maintainer/List modified items" item.
 */
void
nact_main_menubar_maintainer_on_list_modified_items( GtkAction *action, NactMainWindow *window )
{
	nact_iactions_list_bis_list_modified_items( NACT_IACTIONS_LIST( window ));
}

/**
 * nact_main_menubar_maintainer_on_dump_clipboard:
 * @action: the #GtkAction of the item.
 * @window: the #NactMainWindow main application window.
 *
 * Triggers the "Maintainer/Dump clipboard" item.
 */
void
nact_main_menubar_maintainer_on_dump_clipboard( GtkAction *action, NactMainWindow *window )
{
	nact_clipboard_dump( nact_main_window_get_clipboard( window ));
}
