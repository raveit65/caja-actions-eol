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

#ifndef __CACT_MAIN_MENUBAR_H__
#define __CACT_MAIN_MENUBAR_H__

/**
 * SECTION: cact_main_menubar
 * @short_description: Main menubar management.
 * @include: cact/cact-main-menubar.h
 */

#include <api/na-object.h>

#include <core/na-updater.h>

#include "cact-main-window.h"

G_BEGIN_DECLS

/* this structure is updated each time the user interacts in the
 * interface ; it is then used to update action sensitivities
 */
typedef struct {
	gint       selected_menus;
	gint       selected_actions;
	gint       selected_profiles;
	gint       clipboard_menus;
	gint       clipboard_actions;
	gint       clipboard_profiles;
	gint       list_menus;
	gint       list_actions;
	gint       list_profiles;
	gboolean   is_modified;
	gboolean   have_exportables;
	gboolean   treeview_has_focus;
	gboolean   level_zero_order_changed;
	gulong     popup_handler;

	/* set by the cact_main_menubar_on_update_sensitivities() function itself
	 */
	gboolean   is_level_zero_writable;
	gboolean   has_writable_providers;
	guint      count_selected;
	GList     *selected_items;
	NAUpdater *updater;
}
	MenubarIndicatorsStruct;

#define MENUBAR_PROP_INDICATORS				"cact-menubar-indicators"
#define MENUBAR_PROP_UI_MANAGER				"cact-menubar-ui-manager"

void     cact_main_menubar_runtime_init( CactMainWindow *window );
void     cact_main_menubar_dispose( CactMainWindow *window );
gboolean cact_main_menubar_is_level_zero_order_changed( const CactMainWindow *window );
void     cact_main_menubar_open_popup( CactMainWindow *window, GdkEventButton *event );
void     cact_main_menubar_enable_item( CactMainWindow *window, const gchar *name, gboolean enabled );

G_END_DECLS

#endif /* __CACT_CACT_MENUBAR_H__ */
