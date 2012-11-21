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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <api/na-object-api.h>

#include "cact-clipboard.h"
#include "cact-menubar-priv.h"
#include "cact-main-window.h"
#include "cact-tree-ieditable.h"

/**
 * cact_menubar_maintainer_on_update_sensitivities:
 * @bar: this #CactMenubar object.
 *
 * Update sensitivities on the Maintainer menu.
 */
void
cact_menubar_maintainer_on_update_sensitivities( const CactMenubar *bar )
{
}

/**
 * cact_menubar_maintainer_on_dump_selection:
 * @action: the #GtkAction of the item.
 * @window: the #BaseWindow main application window.
 *
 * Triggers the "Maintainer/Dump selection" item.
 */
void
cact_menubar_maintainer_on_dump_selection( GtkAction *action, BaseWindow *window )
{
	BAR_WINDOW_VOID( window );

	na_object_dump_tree( bar->private->selected_items );
}

/**
 * cact_menubar_maintainer_on_brief_tree_store_dump:
 * @action: the #GtkAction of the item.
 * @window: the #BaseWindow main application window.
 *
 * Triggers the "Maintainer/Brief treestore dump" item.
 */
void
cact_menubar_maintainer_on_brief_tree_store_dump( GtkAction *action, BaseWindow *window )
{
	CactTreeView *items_view;
	GList *items;

	items_view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
	items = cact_tree_view_get_items( items_view );
	na_object_dump_tree( items );
	na_object_free_items( items );
}

/**
 * cact_menubar_maintainer_on_list_modified_items:
 * @action: the #GtkAction of the item.
 * @window: the #BaseWindow main application window.
 *
 * Triggers the "Maintainer/List modified items" item.
 */
void
cact_menubar_maintainer_on_list_modified_items( GtkAction *action, BaseWindow *window )
{
	CactTreeView *items_view;

	items_view = cact_main_window_get_items_view( CACT_MAIN_WINDOW( window ));
	cact_tree_ieditable_dump_modified( CACT_TREE_IEDITABLE( items_view ));
}

/**
 * cact_menubar_maintainer_on_dump_clipboard:
 * @action: the #GtkAction of the item.
 * @window: the #BaseWindow main application window.
 *
 * Triggers the "Maintainer/Dump clipboard" item.
 */
void
cact_menubar_maintainer_on_dump_clipboard( GtkAction *action, BaseWindow *window )
{
	cact_clipboard_dump( cact_main_window_get_clipboard( CACT_MAIN_WINDOW( window )));
}

/*
 * Test a miscellaneous function
 */
void
cact_menubar_maintainer_on_test_function( GtkAction *action, BaseWindow *window )
{
	BaseApplication *application = base_window_get_application( window );
	gboolean is_willing = base_application_is_willing_to_quit( application );
	g_debug( "cact_menubar_maintainer_on_test_function: willing_to=%s", is_willing ? "True":"False" );
}
