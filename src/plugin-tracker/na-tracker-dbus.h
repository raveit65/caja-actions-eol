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

#ifndef __NA_TRACKER_DBUS_H__
#define __NA_TRACKER_DBUS_H__

/**
 * SECTION: na_tracker_dbus
 * @short_description: #NATrackerDBus class definition.
 * @include: tracker/na-tracker.h
 *
 * There is only one NATrackerDBus object in the process.
 *
 * As a Caja extension, it is initialized when the module is loaded
 * by the file manager at startup time.
 *
 * The NATrackerDBus object maintains the list of lastly selected items in
 * Caja UI.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_TRACKER_DBUS_TYPE				( na_tracker_dbus_get_type())
#define NA_TRACKER_DBUS( object )			( G_TYPE_CHECK_INSTANCE_CAST(( object ), NA_TRACKER_DBUS_TYPE, NATrackerDBus ))
#define NA_TRACKER_DBUS_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST(( klass ), NA_TRACKER_DBUS_TYPE, NATrackerDBusClass ))
#define NA_IS_TRACKER_DBUS( object )		( G_TYPE_CHECK_INSTANCE_TYPE(( object ), NA_TRACKER_DBUS_TYPE ))
#define NA_IS_TRACKER_DBUS_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TRACKER_DBUS_TYPE ))
#define NA_TRACKER_DBUS_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TRACKER_DBUS_TYPE, NATrackerDBusClass ))

typedef struct NATrackerDBusPrivate      NATrackerDBusPrivate;

typedef struct
{
	GObject               parent;
	NATrackerDBusPrivate *private;
}
	NATrackerDBus;

typedef struct NATrackerDBusClassPrivate NATrackerDBusClassPrivate;

typedef struct
{
	GObjectClass               parent;
	NATrackerDBusClassPrivate *private;
}
	NATrackerDBusClass;

GType    na_tracker_dbus_get_type( void );

void     na_tracker_dbus_set_uris( NATrackerDBus *tracker, GList *files );

gboolean na_tracker_dbus_get_selected_paths( NATrackerDBus *tracker, char ***paths, GError **error );

#define NA_TRACKER_DBUS_TRACKER_PATH		"/org/caja_actions/DBus/Tracker"
#define NA_TRACKER_DBUS_TRACKER_INTERFACE	"org.caja_actions.DBus.Tracker.Status"

G_END_DECLS

#endif /* __NA_TRACKER_DBUS_H__ */
