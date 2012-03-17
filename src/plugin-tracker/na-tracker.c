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

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libcaja-extension/caja-extension-types.h>
#include <libcaja-extension/caja-file-info.h>
#include <libcaja-extension/caja-menu-provider.h>

#include <api/na-dbus.h>

#include "na-tracker.h"
#include "na-tracker-dbus.h"

/* private class data
 */
struct NATrackerClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NATrackerPrivate {
	gboolean       dispose_has_run;
	NATrackerDBus *tracker;
};

static GObjectClass *st_parent_class = NULL;
static GType         st_module_type = 0;

static void           class_init( NATrackerClass *klass );
static void           instance_init( GTypeInstance *instance, gpointer klass );
static NATrackerDBus *initialize_dbus_connection( void );
static void           instance_dispose( GObject *object );
static void           instance_finalize( GObject *object );

static void           menu_provider_iface_init( CajaMenuProviderIface *iface );
static GList         *menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *folder );
static GList         *menu_provider_get_file_items( CajaMenuProvider *provider, GtkWidget *window, GList *files );

GType
na_tracker_get_type( void )
{
	g_assert( st_module_type );
	return( st_module_type );
}

void
na_tracker_register_type( GTypeModule *module )
{
	static const gchar *thisfn = "na_tracker_register_type";

	static const GTypeInfo info = {
		sizeof( NATrackerClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NATracker ),
		0,
		( GInstanceInitFunc ) instance_init,
	};

	static const GInterfaceInfo menu_provider_iface_info = {
		( GInterfaceInitFunc ) menu_provider_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s: module=%p", thisfn, ( void * ) module );
	g_assert( st_module_type == 0 );

	st_module_type = g_type_module_register_type( module, G_TYPE_OBJECT, "NATracker", &info, 0 );

	g_type_module_add_interface( module, st_module_type, CAJA_TYPE_MENU_PROVIDER, &menu_provider_iface_info );
}

static void
class_init( NATrackerClass *klass )
{
	static const gchar *thisfn = "na_tracker_class_init";
	GObjectClass *gobject_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	gobject_class = G_OBJECT_CLASS( klass );
	gobject_class->dispose = instance_dispose;
	gobject_class->finalize = instance_finalize;

	klass->private = g_new0( NATrackerClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_tracker_instance_init";
	NATracker *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_return_if_fail( NA_IS_TRACKER( instance ));

	self = NA_TRACKER( instance );

	self->private = g_new0( NATrackerPrivate, 1 );
	self->private->dispose_has_run = FALSE;
	self->private->tracker = initialize_dbus_connection();
}

/*
 * initialize the DBus connection at class init time
 * & instantiate the object which will do effective tracking
 */
static NATrackerDBus *
initialize_dbus_connection( void )
{
	static const gchar *thisfn = "na_tracker_initialize_dbus_connection";
	NATrackerDBus *tracker;
	DBusGConnection *connection;
	GError *error;
	DBusGProxy *proxy;
	guint32 request_name_ret;

	/* get a connection on session DBus
	 */
	tracker = NULL;
	error = NULL;
	connection = dbus_g_bus_get( DBUS_BUS_SESSION, &error );
	if( !connection ){
		g_warning( "%s: unable to get a connection on session DBus: %s", thisfn, error->message );
		g_error_free( error );
		return( NULL );
	}
	g_debug( "%s: connection is ok", thisfn );

	/* get a proxy for this connection
	 * this proxy let us request some standard DBus services
	 */
	proxy = dbus_g_proxy_new_for_name( connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS );
	if( !proxy ){
		g_warning( "%s: unable to get a proxy for the connection", thisfn );
		dbus_g_connection_unref( connection );
		return( NULL );
	}
	g_debug( "%s: proxy is ok", thisfn );

	/* try to register our service name as a unique 'well known' name
	 */
	if( !org_freedesktop_DBus_request_name(
			proxy, CAJA_ACTIONS_DBUS_SERVICE, 0, &request_name_ret, &error )){

		g_warning( "%s: unable to register %s as a 'well known' name on the bus: %s",
				thisfn, CAJA_ACTIONS_DBUS_SERVICE, error->message );
		g_error_free( error );
		dbus_g_connection_unref( connection );
		return( NULL );
	}
	g_debug( "%s: well known name registration is ok", thisfn );

	if( request_name_ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER ){
		g_warning("%s: got result code %u from requesting name (not the primary owner of the name)", thisfn, request_name_ret );
		dbus_g_connection_unref( connection );
		return( NULL );
	}
	g_debug( "%s: primary owner check is ok", thisfn );

	/* allocate the tracking object and register it
	 * instantiation takes care of installing introspection infos
	 */
	tracker = g_object_new( NA_TRACKER_DBUS_TYPE, NULL );
	dbus_g_connection_register_g_object( connection, NA_TRACKER_DBUS_TRACKER_PATH, G_OBJECT( tracker ));

	g_debug( "%s: registering tracker path is ok", thisfn );
	return( tracker );
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_tracker_instance_dispose";
	NATracker *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_TRACKER( object ));
	self = NA_TRACKER( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		g_object_unref( self->private->tracker );
		self->private->tracker = NULL;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_tracker_instance_finalize";
	NATracker *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_TRACKER( object ));
	self = NA_TRACKER( object );

	g_free( self->private );

	/* chain up to the parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
menu_provider_iface_init( CajaMenuProviderIface *iface )
{
	static const gchar *thisfn = "na_tracker_menu_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_background_items = menu_provider_get_background_items;
	iface->get_file_items = menu_provider_get_file_items;
	iface->get_toolbar_items = NULL;
}

static GList *
menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *folder )
{
	static const gchar *thisfn = "na_tracker_menu_provider_get_background_items";
	NATracker *self;
	gchar *uri;
	GList *selected;

	uri = caja_file_info_get_uri( folder );
	g_debug( "%s: provider=%p, window=%p, folder=%s", thisfn, ( void * ) provider, ( void * ) window, uri );
	g_free( uri );

	g_return_val_if_fail( NA_IS_TRACKER( provider ), NULL );
	self = NA_TRACKER( provider );

	if( !self->private->dispose_has_run && self->private->tracker ){

		selected = g_list_prepend( NULL, folder );
		na_tracker_dbus_set_uris( self->private->tracker, selected );
		g_list_free( selected );
	}

	return( NULL );
}

/*
 * this function is called each time the selection changed
 * menus items are available :
 * a) in Edit menu while the selection stays unchanged
 * b) in contextual menu while the selection stays unchanged
 */
static GList *
menu_provider_get_file_items( CajaMenuProvider *provider, GtkWidget *window, GList *files )
{
	static const gchar *thisfn = "na_tracker_menu_provider_get_file_items";
	NATracker *self;

	g_debug( "%s: provider=%p, window=%p, files=%p, count=%d",
			thisfn, ( void * ) provider, ( void * ) window, ( void * ) files, g_list_length( files ));

	g_return_val_if_fail( NA_IS_TRACKER( provider ), NULL );
	self = NA_TRACKER( provider );

	if( !self->private->dispose_has_run && self->private->tracker ){

		na_tracker_dbus_set_uris( self->private->tracker, files );
	}

	return( NULL );
}
