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

#ifndef __CACT_SORT_BUTTONS_H__
#define __CACT_SORT_BUTTONS_H__

/**
 * SECTION: cact_sort_buttons
 * @short_description: Manage the sort buttons in the main window.
 * @include: cact/cact-sort-buttons.h
 */

#include "cact-main-window.h"

G_BEGIN_DECLS

void  cact_sort_buttons_initial_load                 ( CactMainWindow *window );
void  cact_sort_buttons_runtime_init                 ( CactMainWindow *window );
void  cact_sort_buttons_all_widgets_showed           ( CactMainWindow *window );
void  cact_sort_buttons_dispose                      ( CactMainWindow *window );
void  cact_sort_buttons_display_order_change         ( CactMainWindow *window, guint order_mode );
void  cact_sort_buttons_level_zero_writability_change( CactMainWindow *window );

G_END_DECLS

#endif /* __CACT_SORT_BUTTONS_H__ */
