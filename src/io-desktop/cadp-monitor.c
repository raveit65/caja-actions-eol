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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>

#include "cadp-monitor.h"

/* private class data
 */
struct _CappMonitorClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CappMonitorPrivate {
	gboolean             dispose_has_run;
	CappDesktopProvider *provider;
	gchar               *name;
	GFile               *file;
	GFileMonitor        *monitor;
	gulong               handler;
};

static GObjectClass *st_parent_class = NULL;

static GType  register_type( void );
static void   class_init( CappMonitorClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *object );
static void   instance_finalize( GObject *object );

static void   on_monitor_changed( GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, CappMonitor *my_monitor );

GType
cadp_monitor_get_type( void )
{
	static GType class_type = 0;

	if( !class_type ){
		class_type = register_type();
	}

	return( class_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cadp_monitor_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CappMonitorClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CappMonitor ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "CappMonitor", &info, 0 );

	return( type );
}

static void
class_init( CappMonitorClass *klass )
{
	static const gchar *thisfn = "cadp_monitor_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CappMonitorClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cadp_monitor_instance_init";
	CappMonitor *self;

	g_return_if_fail( CADP_IS_MONITOR( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CADP_MONITOR( instance );

	self->private = g_new0( CappMonitorPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cadp_monitor_instance_dispose";
	CappMonitor *self;

	g_return_if_fail( CADP_IS_MONITOR( object ));

	self = CADP_MONITOR( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		if( self->private->handler ){
			g_signal_handler_disconnect( self->private->monitor, self->private->handler );
		}

		if( self->private->monitor ){
			g_object_unref( self->private->monitor );
		}

		if( self->private->file ){
			g_object_unref( self->private->file );
		}

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "cadp_monitor_instance_finalize";
	CappMonitor *self;

	g_return_if_fail( CADP_IS_MONITOR( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = CADP_MONITOR( object );

	g_free( self->private->name );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * cadp_monitor_new:
 * @provider: the #CappDesktopProvider instance.
 * @path: the path of a directory to be monitored.
 *
 * Installs a new monitor on the given directory.
 *
 * Returns: a new #CappMonitor instance.
 */
CappMonitor *
cadp_monitor_new( const CappDesktopProvider *provider, const gchar *path )
{
	static const gchar *thisfn = "cadp_monitor_new";
	CappMonitor *monitor;
	GFileMonitorFlags flags;
	GError *error;

	monitor = g_object_new( CADP_TYPE_MONITOR, NULL );

	monitor->private->provider = CADP_DESKTOP_PROVIDER( provider );
	monitor->private->name = g_strdup( path );
	monitor->private->file = g_file_new_for_path( path );

	error = NULL;
	flags = G_FILE_MONITOR_NONE;

	monitor->private->monitor = g_file_monitor_directory( monitor->private->file, flags, NULL, &error );
	if( error ){
		g_warning( "%s: g_file_monitor: %s", thisfn, error->message );
		g_error_free( error );
		error = NULL;
		g_object_unref( monitor );
		return( NULL );
	}

	g_return_val_if_fail( monitor->private->monitor, NULL );

	monitor->private->handler = g_signal_connect(
				monitor->private->monitor, "changed", G_CALLBACK( on_monitor_changed ), monitor );

	return( monitor );
}

/*
 * - an existing file is modified: n events on dir + m events on file
 * - an existing file is deleted: 1 event on file + 1 event on dir
 * - a new file is created: n events on the dir
 * - a new directory is created: 1 event of this new dir
 * - a directory is removed: 1 event of this new dir
 * - an existing file is renamed: 1 event on file + n events on dir
 * - the renamed file is modified: n events on dir
 */
static void
on_monitor_changed( GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, CappMonitor *my_monitor )
{
	cadp_desktop_provider_on_monitor_event( my_monitor->private->provider );
}
