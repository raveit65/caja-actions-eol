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

#ifndef __CACT_MAIN_TAB_H__
#define __CACT_MAIN_TAB_H__

#include <api/na-icontext.h>

#include "cact-main-window.h"

/**
 * SECTION: cact_main_tab
 * @short_description: Signals and properties.
 * @include: cact/cact-main-tab.h
 *
 * Here as defined signals and properties common to all tabs.
 */

/* signals
 *
 * TAB_UPDATABLE_SIGNAL_ITEM_UPDATED: see definition in cact-main-window.c
 */
#define TAB_UPDATABLE_SIGNAL_ITEM_UPDATED				"cact-tab-updatable-item-updated"

/* notebook tabs
 */
enum {
	TAB_ACTION = 0,
	TAB_COMMAND,
	TAB_EXECUTION,
	TAB_BASENAMES,
	TAB_MIMETYPES,
	TAB_FOLDERS,
	TAB_SCHEMES,
	TAB_CAPABILITIES,
	TAB_ENVIRONMENT,
	TAB_PROPERTIES
};

void     cact_main_tab_init           ( CactMainWindow *window, gint page );
void     cact_main_tab_enable_page    ( CactMainWindow *window, gint page, gboolean enabled );
gboolean cact_main_tab_is_page_enabled( CactMainWindow *window, gint page );

G_END_DECLS

#endif /* __CACT_MAIN_TAB_H__ */
