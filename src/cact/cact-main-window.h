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

#ifndef __CACT_MAIN_WINDOW_H__
#define __CACT_MAIN_WINDOW_H__

/**
 * SECTION: cact_main_window
 * @short_description: #CactMainWindow class definition.
 * @include: cact/cact-main-window.h
 *
 * This class is derived from BaseWindow and manages the main window.
 */

#include <api/na-object-item.h>

#include "cact-clipboard.h"
#include "cact-window.h"

G_BEGIN_DECLS

#define CACT_MAIN_WINDOW_TYPE					( cact_main_window_get_type())
#define CACT_MAIN_WINDOW( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_MAIN_WINDOW_TYPE, CactMainWindow ))
#define CACT_MAIN_WINDOW_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, CACT_MAIN_WINDOW_TYPE, CactMainWindowClass ))
#define CACT_IS_MAIN_WINDOW( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_MAIN_WINDOW_TYPE ))
#define CACT_IS_MAIN_WINDOW_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_MAIN_WINDOW_TYPE ))
#define CACT_MAIN_WINDOW_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_MAIN_WINDOW_TYPE, CactMainWindowClass ))

typedef struct CactMainWindowPrivate      CactMainWindowPrivate;

typedef struct {
	CactWindow             parent;
	CactMainWindowPrivate *private;
}
	CactMainWindow;

typedef struct CactMainWindowClassPrivate CactMainWindowClassPrivate;

typedef struct {
	CactWindowClass             parent;
	CactMainWindowClassPrivate *private;
}
	CactMainWindowClass;

#define MAIN_WINDOW_SIGNAL_SELECTION_CHANGED			"main-window-selection-changed"
#define MAIN_WINDOW_SIGNAL_UPDATE_ACTION_SENSITIVITIES	"main-window-update-sensitivities"
#define MAIN_WINDOW_SIGNAL_LEVEL_ZERO_ORDER_CHANGED		"main-window-level-zero-order-changed"

GType           cact_main_window_get_type( void );

CactMainWindow *cact_main_window_new( BaseApplication *application );

CactClipboard  *cact_main_window_get_clipboard     ( const CactMainWindow *window );
NAObjectItem   *cact_main_window_get_item          ( const CactMainWindow *window, const gchar *id );
gboolean        cact_main_window_has_modified_items( const CactMainWindow *window );
void            cact_main_window_move_to_deleted   ( CactMainWindow *window, GList *items );
void            cact_main_window_reload            ( CactMainWindow *window );
void            cact_main_window_remove_deleted    ( CactMainWindow *window );

G_END_DECLS

#endif /* __CACT_MAIN_WINDOW_H__ */
