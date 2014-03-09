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

#ifndef __CAJA_ACTIONS_API_NA_MATECONF_MONITOR_H__
#define __CAJA_ACTIONS_API_NA_MATECONF_MONITOR_H__

#ifdef HAVE_MATECONF
#ifdef NA_ENABLE_DEPRECATED
/**
 * SECTION: mateconf-monitor
 * @title: NAMateConfMonitor
 * @short_description: The MateConf Monitoring Class Definition
 * @include: caja-actions/na-mateconf-monitor.h
 *
 * This class manages the MateConf monitoring.
 * It is used to monitor both the MateConf provider and the MateConf runtime
 * preferences.
 *
 * Starting with Caja-Actions 3.1.0, MateConf, whether it is used as a
 * preference storage subsystem or as an I/O provider, is deprecated.
 */

#include <mateconf/mateconf-client.h>

G_BEGIN_DECLS

#define NA_MATECONF_MONITOR( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_MATECONF_MONITOR_TYPE, NAMateConfMonitor ))
#define NA_MATECONF_MONITOR_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_MATECONF_MONITOR_TYPE, NAMateConfMonitorClass ))
#define NA_IS_MATECONF_MONITOR( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_MATECONF_MONITOR_TYPE ))
#define NA_IS_MATECONF_MONITOR_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_MATECONF_MONITOR_TYPE ))
#define NA_MATECONF_MONITOR_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_MATECONF_MONITOR_TYPE, NAMateConfMonitorClass ))

typedef struct _NAMateConfMonitorPrivate        NAMateConfMonitorPrivate;

typedef struct {
	/*< private >*/
	GObject                parent;
	NAMateConfMonitorPrivate *private;
}
	NAMateConfMonitor;

typedef struct _NAMateConfMonitorClassPrivate   NAMateConfMonitorClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass                parent;
	NAMateConfMonitorClassPrivate *private;
}
	NAMateConfMonitorClass;

NAMateConfMonitor *na_mateconf_monitor_new( const gchar *path, MateConfClientNotifyFunc handler, gpointer user_data );

void            na_mateconf_monitor_release_monitors( GList *monitors );

G_END_DECLS

#endif /* NA_ENABLE_DEPRECATED */
#endif /* HAVE_MATECONF */
#endif /* __CAJA_ACTIONS_API_NA_MATECONF_MONITOR_H__ */
