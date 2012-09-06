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

#ifndef __CACT_MAIN_MENUBAR_VIEW_H__
#define __CACT_MAIN_MENUBAR_VIEW_H__

/**
 * SECTION: cact_main_menubar
 * @short_description: Main menubar View menu management.
 * @include: cact/cact-main-menubar-view.h
 */

#include <gtk/gtk.h>

#include "cact-main-menubar.h"

G_BEGIN_DECLS

void cact_main_menubar_view_on_update_sensitivities( CactMainWindow *window, gpointer user_data, MenubarIndicatorsStruct *mis );

void cact_main_menubar_view_on_expand_all   ( GtkAction *action, CactMainWindow *window );
void cact_main_menubar_view_on_collapse_all ( GtkAction *action, CactMainWindow *window );
void cact_main_menubar_view_on_toolbar_file ( GtkToggleAction *action, CactMainWindow *window );
void cact_main_menubar_view_on_toolbar_edit ( GtkToggleAction *action, CactMainWindow *window );
void cact_main_menubar_view_on_toolbar_tools( GtkToggleAction *action, CactMainWindow *window );
void cact_main_menubar_view_on_toolbar_help ( GtkToggleAction *action, CactMainWindow *window );

G_END_DECLS

#endif /* __CACT_CACT_MENUBAR_VIEW_H__ */
