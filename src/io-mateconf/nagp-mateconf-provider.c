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

#include <glib/gi18n.h>
#include <string.h>

#include <api/na-ifactory-provider.h>
#include <api/na-iio-provider.h>
#include <api/na-mateconf-monitor.h>

#include "nagp-mateconf-provider.h"
#include "nagp-reader.h"
#include "nagp-writer.h"
#include "nagp-keys.h"

/* private class data
 */
struct _NagpMateConfProviderClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static GType         st_module_type = 0;
static GObjectClass *st_parent_class = NULL;

#ifdef NA_ENABLE_DEPRECATED
static gint          st_burst_timeout = 100;		/* burst timeout in msec */
#endif

static void     class_init( NagpMateConfProviderClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *object );
static void     instance_finalize( GObject *object );

static void     iio_provider_iface_init( NAIIOProviderInterface *iface );
static gchar   *iio_provider_get_id( const NAIIOProvider *provider );
static gchar   *iio_provider_get_name( const NAIIOProvider *provider );
static guint    iio_provider_get_version( const NAIIOProvider *provider );

static void     ifactory_provider_iface_init( NAIFactoryProviderInterface *iface );
static guint    ifactory_provider_get_version( const NAIFactoryProvider *provider );

#ifdef NA_ENABLE_DEPRECATED
static GList   *install_monitors( NagpMateConfProvider *provider );
static void     config_path_changed_cb( MateConfClient *client, guint cnxn_id, MateConfEntry *entry, NagpMateConfProvider *provider );
static gboolean config_path_changed_trigger_interface( NagpMateConfProvider *provider );
static gulong   time_val_diff( const GTimeVal *recent, const GTimeVal *old );
#endif

GType
nagp_mateconf_provider_get_type( void )
{
	return( st_module_type );
}

void
nagp_mateconf_provider_register_type( GTypeModule *module )
{
	static const gchar *thisfn = "nagp_mateconf_provider_register_type";

	static GTypeInfo info = {
		sizeof( NagpMateConfProviderClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NagpMateConfProvider ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo iio_provider_iface_info = {
		( GInterfaceInitFunc ) iio_provider_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ifactory_provider_iface_info = {
		( GInterfaceInitFunc ) ifactory_provider_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	st_module_type = g_type_module_register_type( module, G_TYPE_OBJECT, "NagpMateConfProvider", &info, 0 );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IIO_PROVIDER, &iio_provider_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IFACTORY_PROVIDER, &ifactory_provider_iface_info );
}

static void
class_init( NagpMateConfProviderClass *klass )
{
	static const gchar *thisfn = "nagp_mateconf_provider_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NagpMateConfProviderClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "nagp_mateconf_provider_instance_init";
	NagpMateConfProvider *self;

	g_return_if_fail( NAGP_IS_MATECONF_PROVIDER( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = NAGP_MATECONF_PROVIDER( instance );

	self->private = g_new0( NagpMateConfProviderPrivate, 1 );

	self->private->dispose_has_run = FALSE;

	self->private->mateconf = mateconf_client_get_default();

#ifdef NA_ENABLE_DEPRECATED
	self->private->monitors = install_monitors( self );
#endif
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "nagp_mateconf_provider_instance_dispose";
	NagpMateConfProvider *self;

	g_return_if_fail( NAGP_IS_MATECONF_PROVIDER( object ));

	self = NAGP_MATECONF_PROVIDER( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

#ifdef NA_ENABLE_DEPRECATED
		/* release the MateConf monitoring */
		na_mateconf_monitor_release_monitors( self->private->monitors );
#endif

		/* release the MateConf connexion */
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
	static const gchar *thisfn =" nagp_mateconf_provider_instance_finalize";
	NagpMateConfProvider *self;

	g_return_if_fail( NAGP_IS_MATECONF_PROVIDER( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = NAGP_MATECONF_PROVIDER( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
iio_provider_iface_init( NAIIOProviderInterface *iface )
{
	static const gchar *thisfn = "nagp_mateconf_provider_iio_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_id = iio_provider_get_id;
	iface->get_name = iio_provider_get_name;
	iface->get_version = iio_provider_get_version;
	iface->read_items = nagp_iio_provider_read_items;
	iface->is_willing_to_write = nagp_iio_provider_is_willing_to_write;
	iface->is_able_to_write = nagp_iio_provider_is_able_to_write;
#ifdef NA_ENABLE_DEPRECATED
	iface->write_item = nagp_iio_provider_write_item;
	iface->delete_item = nagp_iio_provider_delete_item;
#else
	iface->write_item = NULL;
	iface->delete_item = NULL;
#endif
	iface->duplicate_data = NULL;
}

static gchar *
iio_provider_get_id( const NAIIOProvider *provider )
{
	return( g_strdup( "na-mateconf" ));
}

static gchar *
iio_provider_get_name( const NAIIOProvider *provider )
{
	return( g_strdup( _( "Caja-Actions MateConf I/O Provider" )));
}

static guint
iio_provider_get_version( const NAIIOProvider *provider )
{
	return( 1 );
}

static void
ifactory_provider_iface_init( NAIFactoryProviderInterface *iface )
{
	static const gchar *thisfn = "nagp_mateconf_provider_ifactory_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = ifactory_provider_get_version;
	iface->read_start = nagp_reader_read_start;
	iface->read_data = nagp_reader_read_data;
	iface->read_done = nagp_reader_read_done;
#ifdef NA_ENABLE_DEPRECATED
	iface->write_start = nagp_writer_write_start;
	iface->write_data = nagp_writer_write_data;
	iface->write_done = nagp_writer_write_done;
#else
	iface->write_start = NULL;
	iface->write_data = NULL;
	iface->write_done = NULL;
#endif
}

static guint
ifactory_provider_get_version( const NAIFactoryProvider *provider )
{
	return( 1 );
}

#ifdef NA_ENABLE_DEPRECATED
static GList *
install_monitors( NagpMateConfProvider *provider )
{
	GList *list = NULL;

	g_return_val_if_fail( NAGP_IS_MATECONF_PROVIDER( provider ), NULL );
	g_return_val_if_fail( NA_IS_IIO_PROVIDER( provider ), NULL );
	g_return_val_if_fail( !provider->private->dispose_has_run, NULL );

	/* monitor the configurations/ directory which contains all menus,
	 * actions and profiles definitions
	 */
	list = g_list_prepend( list,
			na_mateconf_monitor_new(
					NAGP_CONFIGURATIONS_PATH,
					( MateConfClientNotifyFunc ) config_path_changed_cb,
					provider ));

	list = g_list_prepend( list,
			na_mateconf_monitor_new(
					NAGP_SCHEMAS_PATH,
					( MateConfClientNotifyFunc ) config_path_changed_cb,
					provider ));

	return( list );
}

/*
 * this callback is triggered each time a value is changed under our
 * configurations/ directory ; as each object has several entries which
 * describe it, this callback is triggered several times for each object
 * update
 *
 * up to and including 1.10.1, the user interface took care of writing
 * a special key in MateConf at the end of each update operations ;
 * as MateConf monitored only this special key, it triggered this callback
 * once for each global update operation
 *
 * this special key was of the form xxx:yyyyyyyy-yyyy-yyyy-..., where :
 *    xxx was a sequential number (inside of the ui session)
 *    yyyyyyyy-yyyy-yyyy-... was the uuid of the involved action
 *
 * this was a sort of hack which simplified a lot the notification
 * system, but didn't take into account any modification which might
 * come from outside of the ui
 *
 * if the modification is made elsewhere (an action is imported as a
 * xml file in mateconf, or mateconf is directly edited), we'd have to rely
 * only on the standard monitor (MateConf watch) mechanism
 *
 * this is what we do below, in three phases:
 * - first, MateConf underlying subsystem advertises us, through the watch
 *   mechanism, of each and every modification ; this leads us to be
 *   triggered for each new/modified/deleted _entry_
 * - as we want trigger the NAIIOProvider interface only once for each
 *   update operation (i.e. once for each flow of individual notifications),
 *   then we install a timer in order to wait for all
 *   entries have been modified
 * - when a [burst_timeout] reasonable delay has elapsed without having
 *   received any new individual notification, then we can assume that
 *   we have reached the end of the flow and that we can now trigger
 *   the NAIIOProvider interface
 *
 * Note that we used to try to send one notification per modified object.
 * This cannot work as we are not sure at all that we will received
 * individual notifications themselves grouped by object.
 */
static void
config_path_changed_cb( MateConfClient *client, guint cnxn_id, MateConfEntry *entry, NagpMateConfProvider *provider )
{
	g_return_if_fail( NAGP_IS_MATECONF_PROVIDER( provider ));
	g_return_if_fail( NA_IS_IIO_PROVIDER( provider ));

	if( !provider->private->dispose_has_run ){

		g_get_current_time( &provider->private->last_event );

		if( !provider->private->event_source_id ){
			provider->private->event_source_id =
				g_timeout_add(
						st_burst_timeout,
						( GSourceFunc ) config_path_changed_trigger_interface,
						provider );
		}
	}
}

/*
 * this timer is set when we receive the first event of a serie
 * we continue to loop until last event is older that the st_burst_timeout
 * delay (in msec)
 * there is no race condition here as we are not multithreaded
 * or .. is there ?
 */
static gboolean
config_path_changed_trigger_interface( NagpMateConfProvider *provider )
{
	static const gchar *thisfn = "nagp_mateconf_provider_config_path_changed_trigger_interface";
	GTimeVal now;
	gulong diff;
	gulong timeout_usec = 1000*st_burst_timeout;

	g_get_current_time( &now );
	diff = time_val_diff( &now, &provider->private->last_event );
	if( diff < timeout_usec ){
		return( TRUE );
	}

	/* last individual notification is older that the st_burst_timeout
	 * so triggers the NAIIOProvider interface and destroys this timeout
	 */
	g_debug( "%s: triggering NAIIOProvider interface for provider=%p (%s)",
			thisfn, ( void * ) provider, G_OBJECT_TYPE_NAME( provider ));

	na_iio_provider_item_changed( NA_IIO_PROVIDER( provider ));
	provider->private->event_source_id = 0;
	return( FALSE );
}

/*
 * returns the difference in microseconds.
 */
static gulong
time_val_diff( const GTimeVal *recent, const GTimeVal *old )
{
	gulong microsec = 1000000 * ( recent->tv_sec - old->tv_sec );
	microsec += recent->tv_usec  - old->tv_usec;
	return( microsec );
}
#endif /* NA_ENABLE_DEPRECATED */
