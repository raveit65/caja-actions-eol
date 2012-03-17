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

#ifndef __NACT_MAIN_MENUBAR_EDIT_H__
#define __NACT_MAIN_MENUBAR_EDIT_H__

/**
 * SECTION: nact_main_menubar
 * @short_description: Main menubar Edit menu management.
 * @include: nact/nact-main-menubar-edit.h
 */

#include <gtk/gtk.h>

#include "nact-main-menubar.h"

G_BEGIN_DECLS

void     nact_main_menubar_edit_on_update_sensitivities( NactMainWindow *window, gpointer user_data, MenubarIndicatorsStruct *mis );

void     nact_main_menubar_edit_on_cut          ( GtkAction *action, NactMainWindow *window );
void     nact_main_menubar_edit_on_copy         ( GtkAction *action, NactMainWindow *window );
void     nact_main_menubar_edit_on_paste        ( GtkAction *action, NactMainWindow *window );
void     nact_main_menubar_edit_on_paste_into   ( GtkAction *action, NactMainWindow *window );
void     nact_main_menubar_edit_on_duplicate    ( GtkAction *action, NactMainWindow *window );
void     nact_main_menubar_edit_on_delete       ( GtkAction *action, NactMainWindow *window );
void     nact_main_menubar_edit_on_reload       ( GtkAction *action, NactMainWindow *window );
void     nact_main_menubar_edit_on_prefererences( GtkAction *action, NactMainWindow *window );

gboolean nact_main_menubar_edit_is_pasted_object_relabeled( NAObject *object, NAPivot *pivot );

G_END_DECLS

#endif /* __NACT_NACT_MENUBAR_EDIT_H__ */
