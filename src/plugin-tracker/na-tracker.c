/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
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
#include "na-tracker-gdbus.h"

#include <libcaja-extension/caja-extension-types.h>
#include <libcaja-extension/caja-file-info.h>
#include <libcaja-extension/caja-menu-provider.h>

#include <api/na-dbus.h>

#include "na-tracker.h"

/* private class data
 */
struct _NATrackerClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _NATrackerPrivate {
	gboolean                  dispose_has_run;
	guint                     owner_id;	/* the identifier returns by g_bus_own_name */
	GDBusObjectManagerServer *manager;
	GList                    *selected;
};

static GObjectClass *st_parent_class = NULL;
static GType         st_module_type = 0;

static void    class_init( NATrackerClass *klass );
static void    instance_init( GTypeInstance *instance, gpointer klass );
static void    initialize_dbus_connection( NATracker *tracker );
static void    on_bus_acquired( GDBusConnection *connection, const gchar *name, NATracker *tracker );
static void    on_name_acquired( GDBusConnection *connection, const gchar *name, NATracker *tracker );
static void    on_name_lost( GDBusConnection *connection, const gchar *name, NATracker *tracker );
static gboolean on_properties1_get_selected_paths( NATrackerProperties1 *tracker_properties, GDBusMethodInvocation *invocation, NATracker *tracker );
static void    instance_dispose( GObject *object );
static void    instance_finalize( GObject *object );

static void    menu_provider_iface_init( CajaMenuProviderIface *iface );
static GList  *menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *folder );
static GList  *menu_provider_get_file_items( CajaMenuProvider *provider, GtkWidget *window, GList *files );

static void    set_uris( NATracker *tracker, GList *files );
static gchar **get_selected_paths( NATracker *tracker );
static GList  *free_selected( GList *selected );

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

	initialize_dbus_connection( self );
}

/*
 * initialize the DBus connection at instanciation time
 * & instantiate the object which will do effective tracking
 */
static void
initialize_dbus_connection( NATracker *tracker )
{
	NATrackerPrivate *priv = tracker->private;

	priv->owner_id = g_bus_own_name(
			G_BUS_TYPE_SESSION,
			CAJA_ACTIONS_DBUS_SERVICE,
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			( GBusAcquiredCallback ) on_bus_acquired,
			( GBusNameAcquiredCallback ) on_name_acquired,
			( GBusNameLostCallback ) on_name_lost,
			tracker,
			NULL );
}

static void
on_bus_acquired( GDBusConnection *connection, const gchar *name, NATracker *tracker )
{
	static const gchar *thisfn = "na_tracker_on_bus_acquired";
	NATrackerObjectSkeleton *tracker_object;
	NATrackerProperties1 *tracker_properties1;

	/*NATrackerDBus *tracker_object;*/

	g_debug( "%s: connection=%p, name=%s, tracker=%p",
			thisfn,
			( void * ) connection,
			name,
			( void * ) tracker );

	/* create a new org.freedesktop.DBus.ObjectManager rooted at
	 *  /org/caja_actions/DBus/Tracker
	 */
	tracker->private->manager = g_dbus_object_manager_server_new( CAJA_ACTIONS_DBUS_TRACKER_PATH );

	/* create a new D-Bus object at the path
	 *  /org/caja_actions/DBus/Tracker
	 *  (which must be same or below than that of object manager server)
	 */
	tracker_object = na_tracker_object_skeleton_new( CAJA_ACTIONS_DBUS_TRACKER_PATH "/0" );

	/* make a newly created object export the interface
	 *  org.caja_actions.DBus.Tracker.Properties1
	 *  and attach it to the D-Bus object, which takes its own reference on it
	 */
	tracker_properties1 = na_tracker_properties1_skeleton_new();
	na_tracker_object_skeleton_set_properties1( tracker_object, tracker_properties1 );
	g_object_unref( tracker_properties1 );

	/* handle GetSelectedPaths method invocation on the .Properties1 interface
	 */
	g_signal_connect(
			tracker_properties1,
			"handle-get-selected-paths",
			G_CALLBACK( on_properties1_get_selected_paths ),
			tracker );

	/* and export the DBus object on the object manager server
	 * (which takes its own reference on it)
	 */
	g_dbus_object_manager_server_export( tracker->private->manager, G_DBUS_OBJECT_SKELETON( tracker_object ));
	g_object_unref( tracker_object );

	/* and connect the object manager server to the D-Bus session
	 * exporting all attached objects
	 */
	g_dbus_object_manager_server_set_connection( tracker->private->manager, connection );
}

static void
on_name_acquired( GDBusConnection *connection, const gchar *name, NATracker *tracker )
{
	static const gchar *thisfn = "na_tracker_on_name_acquired";

	g_debug( "%s: connection=%p, name=%s, tracker=%p",
			thisfn,
			( void * ) connection,
			name,
			( void * ) tracker );
}

static void
on_name_lost( GDBusConnection *connection, const gchar *name, NATracker *tracker )
{
	static const gchar *thisfn = "na_tracker_on_name_lost";

	g_debug( "%s: connection=%p, name=%s, tracker=%p",
			thisfn,
			( void * ) connection,
			name,
			( void * ) tracker );
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_tracker_instance_dispose";
	NATrackerPrivate *priv;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_TRACKER( object ));

	priv = NA_TRACKER( object )->private;

	if( !priv->dispose_has_run ){

		priv->dispose_has_run = TRUE;

		if( priv->owner_id ){
			g_bus_unown_name( priv->owner_id );
		}
		if( priv->manager ){
			g_object_unref( priv->manager );
		}

		priv->selected = free_selected( priv->selected );

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
}

static GList *
menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *folder )
{
	static const gchar *thisfn = "na_tracker_menu_provider_get_background_items";
	NATracker *tracker;
	gchar *uri;
	GList *selected;

	g_return_val_if_fail( NA_IS_TRACKER( provider ), NULL );

	tracker = NA_TRACKER( provider );

	if( !tracker->private->dispose_has_run ){

		uri = caja_file_info_get_uri( folder );
		g_debug( "%s: provider=%p, window=%p, folder=%s",
				thisfn,
				( void * ) provider,
				( void * ) window,
				uri );
		g_free( uri );

		selected = g_list_prepend( NULL, folder );
		set_uris( tracker, selected );
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
	NATracker *tracker;

	g_return_val_if_fail( NA_IS_TRACKER( provider ), NULL );

	tracker = NA_TRACKER( provider );

	if( !tracker->private->dispose_has_run ){

		g_debug( "%s: provider=%p, window=%p, files=%p, count=%d",
				thisfn,
				( void * ) provider,
				( void * ) window,
				( void * ) files, g_list_length( files ));

		set_uris( tracker, files );
	}

	return( NULL );
}

/*
 * set_uris:
 * @tracker: this #NATracker instance.
 * @files: the list of currently selected items.
 *
 * Maintains our own list of uris.
 */
static void
set_uris( NATracker *tracker, GList *files )
{
	NATrackerPrivate *priv;

	priv = tracker->private;

	priv->selected = free_selected( tracker->private->selected );
	priv->selected = caja_file_info_list_copy( files );
}

/*
 * Returns: %TRUE if the method has been handled.
 */
static gboolean
on_properties1_get_selected_paths( NATrackerProperties1 *tracker_properties, GDBusMethodInvocation *invocation, NATracker *tracker )
{
	gchar **paths;

	g_return_val_if_fail( NA_IS_TRACKER( tracker ), FALSE );

	paths = get_selected_paths( tracker );

	na_tracker_properties1_complete_get_selected_paths(
			tracker_properties,
			invocation,
			( const gchar * const * ) paths );

	return( TRUE );
}

/*
 * get_selected_paths:
 * @tracker: this #NATracker object.
 *
 * Sends on session D-Bus the list of currently selected items, as two
 * strings for each item :
 * - the uri
 * - the mimetype as returned by CajaFileInfo.
 *
 * This is required as some particular items are only known by Caja
 * (e.g. computer), and standard GLib functions are not able to retrieve
 * their mimetype.
 *
 * Exported as GetSelectedPaths method on Tracker.Properties1 interface.
 */
static gchar **
get_selected_paths( NATracker *tracker )
{
	static const gchar *thisfn = "na_tracker_get_selected_paths";
	NATrackerPrivate *priv;
	gchar **paths;
	GList *it;
	int count;
	gchar **iter;

	paths = NULL;
	priv = tracker->private;

	g_debug( "%s: tracker=%p", thisfn, ( void * ) tracker );

	count = 2 * g_list_length( priv->selected );
	paths = ( char ** ) g_new0( gchar *, 1+count );
	iter = paths;

	for( it = priv->selected ; it ; it = it->next ){
		*iter = caja_file_info_get_uri(( CajaFileInfo * ) it->data );
		iter++;
		*iter = caja_file_info_get_mime_type(( CajaFileInfo * ) it->data );
		iter++;
	}

	return( paths );
}

static GList *
free_selected( GList *selected )
{
	caja_file_info_list_free( selected );

	return( NULL );
}
