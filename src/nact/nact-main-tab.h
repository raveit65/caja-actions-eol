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

#ifndef __NACT_MAIN_TAB_H__
#define __NACT_MAIN_TAB_H__

#include "nact-main-window.h"

/**
 * SECTION: nact_main_tab
 * @short_description: Signals and properties.
 * @include: nact/nact-main-tab.h
 *
 * Here as defined signals and properties common to all tabs.
 */

/* properties set against the GObject instance
 */
#define TAB_UPDATABLE_PROP_EDITED_ACTION				"nact-tab-updatable-edited-action"
#define TAB_UPDATABLE_PROP_EDITED_PROFILE				"nact-tab-updatable-edited-profile"
#define TAB_UPDATABLE_PROP_SELECTED_ROW					"nact-tab-updatable-selected-row"
#define TAB_UPDATABLE_PROP_EDITABLE						"nact-tab-updatable-editable"
#define TAB_UPDATABLE_PROP_REASON						"nact-tab-updatable-reason"

/* signals
 */
#define TAB_UPDATABLE_SIGNAL_ITEM_UPDATED				"nact-tab-updatable-item-updated"
#define TAB_UPDATABLE_SIGNAL_ENABLE_TAB					"nact-tab-updatable-enable-tab"
#define TAB_UPDATABLE_SIGNAL_PROVIDER_CHANGED			"nact-tab-updatable-provider-changed"

/* notebook tabs
 */

enum {
	TAB_ACTION = 0,
	TAB_COMMAND,
	TAB_FOLDERS,
	TAB_CONDITIONS,
	TAB_ADVANCED
};

void     nact_main_tab_enable_page( NactMainWindow *window, gint page, gboolean enabled );
gboolean nact_main_tab_is_page_enabled( NactMainWindow *window, gint page );

G_END_DECLS

#endif /* __NACT_MAIN_TAB_H__ */
