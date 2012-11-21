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

#ifndef __CADP_MONITOR_H__
#define __CADP_MONITOR_H__

/**
 * SECTION: cadp_monitor
 * @short_description: #CappMonitor class definition.
 * @include: cadp-monitor.h
 *
 * This class manages monitoring on .desktop files and directories.
 * We also put a monitor on directories which do not exist, to be
 * triggered when a file is dropped there.
 *
 * During tests of GIO monitoring, we don't have found any case where a
 * file monitor would be triggered without the parent directory monitor
 * has been itself triggered. We, so only monitor directories (not files).
 * More, as several events may be triggered for one user modification,
 * we try to factorize all monitor events before advertizing NAPivot.
 */

#include "cadp-desktop-provider.h"

G_BEGIN_DECLS

#define CADP_TYPE_MONITOR                ( cadp_monitor_get_type())
#define CADP_MONITOR( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CADP_TYPE_MONITOR, CappMonitor ))
#define CADP_MONITOR_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CADP_TYPE_MONITOR, CappMonitorClass ))
#define CADP_IS_MONITOR( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CADP_TYPE_MONITOR ))
#define CADP_IS_MONITOR_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CADP_TYPE_MONITOR ))
#define CADP_MONITOR_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CADP_TYPE_MONITOR, CappMonitorClass ))

typedef struct _CappMonitorPrivate       CappMonitorPrivate;

typedef struct {
	/*< private >*/
	GObject             parent;
	CappMonitorPrivate *private;
}
	CappMonitor;

typedef struct _CappMonitorClassPrivate  CappMonitorClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass             parent;
	CappMonitorClassPrivate *private;
}
	CappMonitorClass;

GType        cadp_monitor_get_type( void );

CappMonitor *cadp_monitor_new( const CappDesktopProvider *provider, const gchar *path );

G_END_DECLS

#endif /* __CADP_MONITOR_H__ */
