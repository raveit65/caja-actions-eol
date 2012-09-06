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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <api/na-mateconf-monitor.h>

/* private class data
 */
struct NAMateConfMonitorClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAMateConfMonitorPrivate {
	gboolean              dispose_has_run;
	MateConfClient          *mateconf;
	gchar                *path;
	gint                  preload;
	MateConfClientNotifyFunc handler;
	gpointer              user_data;
	guint                 monitor_id;
};

static GObjectClass *st_parent_class = NULL;

static GType register_type( void );
static void  class_init( NAMateConfMonitorClass *klass );
static void  instance_init( GTypeInstance *instance, gpointer klass );
static void  instance_dispose( GObject *object );
static void  instance_finalize( GObject *object );

static guint install_monitor( NAMateConfMonitor *monitor );
static void  release_monitor( NAMateConfMonitor *monitor );

GType
na_mateconf_monitor_get_type( void )
{
	static GType object_type = 0;

	if( !object_type ){
		object_type = register_type();
	}

	return( object_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_mateconf_monitor_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAMateConfMonitorClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAMateConfMonitor ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NAMateConfMonitor", &info, 0 );

	return( type );
}

static void
class_init( NAMateConfMonitorClass *klass )
{
	static const gchar *thisfn = "na_mateconf_monitor_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAMateConfMonitorClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_mateconf_monitor_instance_init";
	NAMateConfMonitor *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( NA_IS_MATECONF_MONITOR( instance ));
	self = NA_MATECONF_MONITOR( instance );

	self->private = g_new0( NAMateConfMonitorPrivate, 1 );

	self->private->mateconf = mateconf_client_get_default();

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_mateconf_monitor_instance_dispose";
	NAMateConfMonitor *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NA_IS_MATECONF_MONITOR( object ));
	self = NA_MATECONF_MONITOR( object );

	if( !self->private->dispose_has_run ){

		/* release the installed monitor before setting dispose_has_run */
		release_monitor( self );

		self->private->dispose_has_run = TRUE;

		g_object_unref( self->private->mateconf );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	NAMateConfMonitor *self;

	g_return_if_fail( NA_IS_MATECONF_MONITOR( object ));
	self = NA_MATECONF_MONITOR( object );

	g_free( self->private->path );
	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * na_mateconf_monitor_new:
 * @client: a #MateConfClient object already initialized by the caller.
 * @path: the absolute path to monitor.
 * @preload: a #MateConfClientPreloadType for this monitoring.
 * @handler: the function to be triggered by the monitor.
 * @user_data: data to pass to the @handler.
 *
 * Initializes the monitoring of a MateConf path.
 */
NAMateConfMonitor *
na_mateconf_monitor_new( const gchar *path, MateConfClientNotifyFunc handler, gpointer user_data )
{
	static const gchar *thisfn = "na_mateconf_monitor_new";
	NAMateConfMonitor *monitor;

	g_debug( "%s: path=%s, user_data=%p", thisfn, path, ( void * ) user_data );

	monitor = g_object_new( NA_MATECONF_MONITOR_TYPE, NULL );

	monitor->private->path = g_strdup( path );
	monitor->private->preload = MATECONF_CLIENT_PRELOAD_RECURSIVE;
	monitor->private->handler = handler;
	monitor->private->user_data = user_data;

	monitor->private->monitor_id = install_monitor( monitor );

	return( monitor );
}

static guint
install_monitor( NAMateConfMonitor *monitor )
{
	static const gchar *thisfn = "na_mateconf_monitor_install_monitor";
	GError *error = NULL;
	guint notify_id;

	g_return_val_if_fail( NA_IS_MATECONF_MONITOR( monitor ), 0 );
	g_return_val_if_fail( !monitor->private->dispose_has_run, 0 );

	mateconf_client_add_dir(
			monitor->private->mateconf,
			monitor->private->path,
			monitor->private->preload,
			&error );

	if( error ){
		g_warning( "%s[mateconf_client_add_dir] path=%s, error=%s", thisfn, monitor->private->path, error->message );
		g_error_free( error );
		return( 0 );
	}

	notify_id = mateconf_client_notify_add(
			monitor->private->mateconf,
			monitor->private->path,
			monitor->private->handler,
			monitor->private->user_data,
			NULL,
			&error );

	if( error ){
		g_warning( "%s[mateconf_client_notify_add] path=%s, error=%s", thisfn, monitor->private->path, error->message );
		g_error_free( error );
		return( 0 );
	}

	return( notify_id );
}

/**
 * na_mateconf_monitor_release_monitors:
 * @monitors: a list of #NAMateConfMonitors.
 *
 * Release allocated monitors.
 */
void
na_mateconf_monitor_release_monitors( GList *monitors )
{
	g_list_foreach( monitors, ( GFunc ) g_object_unref, NULL );
	g_list_free( monitors );
}

/*
 * this is called by instance_dispose, before setting dispose_has_run
 */
static void
release_monitor( NAMateConfMonitor *monitor )
{
	static const gchar *thisfn = "na_mateconf_monitor_release_monitor";
	GError *error = NULL;

	g_debug( "%s: monitor=%p", thisfn, ( void * ) monitor );
	g_return_if_fail( NA_IS_MATECONF_MONITOR( monitor ));

	if( !monitor->private->dispose_has_run ){

		if( monitor->private->monitor_id ){
			mateconf_client_notify_remove( monitor->private->mateconf, monitor->private->monitor_id );
		}

		mateconf_client_remove_dir( monitor->private->mateconf, monitor->private->path, &error );

		if( error ){
			g_warning( "%s: path=%s, error=%s", thisfn, monitor->private->path, error->message );
			g_error_free( error );
		}
	}
}
