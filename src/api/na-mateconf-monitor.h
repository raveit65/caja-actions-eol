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

#ifndef __CAJA_ACTIONS_API_NA_MATECONF_MONITOR_H__
#define __CAJA_ACTIONS_API_NA_MATECONF_MONITOR_H__

/**
 * SECTION: na_mateconf_monitor
 * @short_description: #NAMateConfMonitor class definition.
 * @include: caja-actions/na-mateconf-monitor.h
 *
 * This class manages the MateConf monitoring.
 * It is used to monitor both the MateConf provider and the MateConf runtime
 * preferences.
 */

#include <mateconf/mateconf-client.h>

G_BEGIN_DECLS

#define NA_MATECONF_MONITOR_TYPE					( na_mateconf_monitor_get_type())
#define NA_MATECONF_MONITOR( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_MATECONF_MONITOR_TYPE, NAMateConfMonitor ))
#define NA_MATECONF_MONITOR_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, NA_MATECONF_MONITOR_TYPE, NAMateConfMonitorClass ))
#define NA_IS_MATECONF_MONITOR( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_MATECONF_MONITOR_TYPE ))
#define NA_IS_MATECONF_MONITOR_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_MATECONF_MONITOR_TYPE ))
#define NA_MATECONF_MONITOR_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_MATECONF_MONITOR_TYPE, NAMateConfMonitorClass ))

typedef struct NAMateConfMonitorPrivate NAMateConfMonitorPrivate;

typedef struct {
	GObject                parent;
	NAMateConfMonitorPrivate *private;
}
	NAMateConfMonitor;

typedef struct NAMateConfMonitorClassPrivate NAMateConfMonitorClassPrivate;

typedef struct {
	GObjectClass                parent;
	NAMateConfMonitorClassPrivate *private;
}
	NAMateConfMonitorClass;

GType           na_mateconf_monitor_get_type( void );

NAMateConfMonitor *na_mateconf_monitor_new( const gchar *path, MateConfClientNotifyFunc handler, gpointer user_data );

void            na_mateconf_monitor_release_monitors( GList *monitors );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_MATECONF_MONITOR_H__ */
