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

#ifndef __NACT_WINDOW_H__
#define __NACT_WINDOW_H__

/**
 * SECTION: nact_window
 * @short_description: #NactWindow class definition.
 * @include: nact/nact-window.h
 *
 * This class is derived from BaseWindow class.
 * It is a common base class for all Caja Actions window documents.
 */

#include <api/na-object-item.h>

#include "base-window.h"

G_BEGIN_DECLS

#define NACT_WINDOW_TYPE				( nact_window_get_type())
#define NACT_WINDOW( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_WINDOW_TYPE, NactWindow ))
#define NACT_WINDOW_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NACT_WINDOW_TYPE, NactWindowClass ))
#define NACT_IS_WINDOW( object )		( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_WINDOW_TYPE ))
#define NACT_IS_WINDOW_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NACT_WINDOW_TYPE ))
#define NACT_WINDOW_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NACT_WINDOW_TYPE, NactWindowClass ))

typedef struct NactWindowPrivate      NactWindowPrivate;

typedef struct {
	BaseWindow         parent;
	NactWindowPrivate *private;
}
	NactWindow;

typedef struct NactWindowClassPrivate NactWindowClassPrivate;

typedef struct {
	BaseWindowClass         parent;
	NactWindowClassPrivate *private;
}
	NactWindowClass;

GType      nact_window_get_type( void );

gboolean   nact_window_has_writable_providers( NactWindow *window );

gboolean   nact_window_is_item_writable( const NactWindow *window, const NAObjectItem *item, guint *reason );

gboolean   nact_window_save_item  ( NactWindow *window, NAObjectItem *item );
gboolean   nact_window_delete_item( NactWindow *window, const NAObjectItem *item );

void       nact_window_count_level_zero_items( GList *items, guint *actions, guint *profiles, guint *menus );

gboolean   nact_window_warn_modified( NactWindow *window );

G_END_DECLS

#endif /* __NACT_WINDOW_H__ */
