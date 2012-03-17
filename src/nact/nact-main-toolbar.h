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

#ifndef __NACT_MAIN_TOOLBAR_H__
#define __NACT_MAIN_TOOLBAR_H__

/**
 * SECTION: nact_main_toolbar
 * @short_description: Main toolbar management.
 * @include: nact/nact-main-toolbar.h
 */

#include "nact-main-window.h"

G_BEGIN_DECLS

enum {
	MAIN_TOOLBAR_FILE_ID = 1,
	MAIN_TOOLBAR_EDIT_ID,
	MAIN_TOOLBAR_TOOLS_ID,
	MAIN_TOOLBAR_HELP_ID,
};

void nact_main_toolbar_init( NactMainWindow *window, GtkActionGroup *group );
void nact_main_toolbar_activate( NactMainWindow *window, int toolbar_id, GtkUIManager *manager, gboolean active );

G_END_DECLS

#endif /* __NACT_MAIN_TOOLBAR_H__ */
