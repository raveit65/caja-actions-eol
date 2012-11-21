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

#ifndef __CACT_MAIN_TOOLBAR_H__
#define __CACT_MAIN_TOOLBAR_H__

/**
 * SECTION: cact_main_toolbar
 * @short_description: Main toolbar management.
 * @include: cact/cact-main-toolbar.h
 */

#include "cact-main-window.h"

G_BEGIN_DECLS

enum {
	MAIN_TOOLBAR_FILE_ID = 1,
	MAIN_TOOLBAR_EDIT_ID,
	MAIN_TOOLBAR_TOOLS_ID,
	MAIN_TOOLBAR_HELP_ID,
};

void cact_main_toolbar_init( BaseWindow *window, GtkActionGroup *group );
void cact_main_toolbar_activate( CactMainWindow *window, int toolbar_id, GtkUIManager *manager, gboolean active );

G_END_DECLS

#endif /* __CACT_MAIN_TOOLBAR_H__ */
