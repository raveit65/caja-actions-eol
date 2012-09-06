/*
 * Caja Actions
 * A Caja extension which offers configurable context menu pivots.
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

#ifndef __CORE_NA_UPDATER_H__
#define __CORE_NA_UPDATER_H__

/**
 * SECTION: na_updater
 * @short_description: #NAUpdater class definition.
 * @include: core/na-updater.h
 *
 * #NAUpdater is a #NAPivot-derived class which allows its clients
 * to update actions and menus.
 */

#include "na-pivot.h"

G_BEGIN_DECLS

#define NA_UPDATER_TYPE					( na_updater_get_type())
#define NA_UPDATER( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, NA_UPDATER_TYPE, NAUpdater ))
#define NA_UPDATER_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NA_UPDATER_TYPE, NAUpdaterClass ))
#define NA_IS_UPDATER( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_UPDATER_TYPE ))
#define NA_IS_UPDATER_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_UPDATER_TYPE ))
#define NA_UPDATER_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_UPDATER_TYPE, NAUpdaterClass ))

typedef struct NAUpdaterPrivate      NAUpdaterPrivate;

typedef struct {
	NAPivot           parent;
	NAUpdaterPrivate *private;
}
	NAUpdater;

typedef struct NAUpdaterClassPrivate NAUpdaterClassPrivate;

typedef struct {
	NAPivotClass           parent;
	NAUpdaterClassPrivate *private;
}
	NAUpdaterClass;

GType      na_updater_get_type( void );

NAUpdater *na_updater_new( void );

/* update the tree in memory
 */
void       na_updater_append_item( NAUpdater *updater, NAObjectItem *item );
void       na_updater_insert_item( NAUpdater *updater, NAObjectItem *item, const gchar *parent_id, gint pos );
void       na_updater_remove_item( NAUpdater *updater, NAObject *item );

/* read from / write to the physical storage subsystem
 */
gboolean   na_updater_is_item_writable( const NAUpdater *updater, const NAObjectItem *item, gint *reason );
guint      na_updater_write_item      ( const NAUpdater *updater, NAObjectItem *item, GSList **messages );
guint      na_updater_delete_item     ( const NAUpdater *updater, const NAObjectItem *item, GSList **messages );

G_END_DECLS

#endif /* __CORE_NA_UPDATER_H__ */
