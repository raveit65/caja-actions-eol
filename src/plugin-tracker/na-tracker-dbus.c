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

/*
 * pwi 2009-12-09 - French comments are from
 * http://www.unixgarden.com/index.php/programmation/decouvertes-et-experimentation-avec-d-bus
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dbus/dbus-glib.h>

#include <libcaja-extension/caja-file-info.h>

#include "na-tracker-dbus.h"
#include "na-tracker-dbus-glue.h"

/* private class data
 */
struct NATrackerDBusClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NATrackerDBusPrivate {
	gboolean  dispose_has_run;
	GList    *selected;
};

static GObjectClass *st_parent_class = NULL;

static GType  register_type( void );
static void   class_init( NATrackerDBusClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *object );
static void   instance_finalize( GObject *object );

static GList *free_selected( GList *selected );

GType
na_tracker_dbus_get_type( void )
{
	static GType tracker_type = 0;

	if( !tracker_type ){
		tracker_type = register_type();
	}

	return( tracker_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_tracker_dbus_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NATrackerDBusClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NATrackerDBus ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NATrackerDBus", &info, 0 );

	return( type );
}

static void
class_init( NATrackerDBusClass *klass )
{
	static const gchar *thisfn = "na_tracker_dbus_class_init";
	GObjectClass *gobject_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	gobject_class = G_OBJECT_CLASS( klass );
	gobject_class->dispose = instance_dispose;
	gobject_class->finalize = instance_finalize;

	klass->private = g_new0( NATrackerDBusClassPrivate, 1 );

	/* Installation du mécanisme d’introspection,
	 * permettant de faire l’appel de méthodes via un nom.
	 * - Le second paramètre de cette fonction (dbus_glib_na_tracker_dbus_object_info)
	 * est généré lors de l’appel à dbus-binding-tool, et sa définition
	 * peut être retrouvée dans le fichier na-tracker-dbus-glue.h
	 */
	dbus_g_object_type_install_info( NA_TRACKER_DBUS_TYPE, &dbus_glib_na_tracker_dbus_object_info );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_tracker_dbus_instance_init";
	NATrackerDBus *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_return_if_fail( NA_IS_TRACKER_DBUS( instance ));

	self = NA_TRACKER_DBUS( instance );

	self->private = g_new0( NATrackerDBusPrivate, 1 );
	self->private->dispose_has_run = FALSE;
	self->private->selected = NULL;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_tracker_dbus_instance_dispose";
	NATrackerDBus *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_TRACKER_DBUS( object ));
	self = NA_TRACKER_DBUS( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		self->private->selected = free_selected( self->private->selected );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_tracker_dbus_instance_finalize";
	NATrackerDBus *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_TRACKER_DBUS( object ));
	self = NA_TRACKER_DBUS( object );

	g_free( self->private );

	/* chain up to the parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * na_tracker_dbus_set_uris:
 * @tracker: this #NATrackerDBus instance.
 * @files: the list of currently selected items.
 *
 * Maintains our own list of uris.
 */
void
na_tracker_dbus_set_uris( NATrackerDBus *tracker, GList *files )
{
	if( !tracker->private->dispose_has_run ){

		tracker->private->selected = free_selected( tracker->private->selected );
		tracker->private->selected = caja_file_info_list_copy( files );
	}
}

/**
 * na_tracker_dbus_get_selected_paths:
 * @tracker: this #NATrackerDBus object.
 * @paths: the location in which copy the strings to be sent.
 * @error: the location of a GError.
 *
 * Sends on session DBus the list of currently selected items, as two strings
 * for each item :
 * - the uri
 * - the mimetype as returned by CajaFileInfo.
 *
 * This is required as some particular items are only known by Caja
 * (e.g. computer), and standard GLib functions are not able to retrieve
 * their mimetype.
 *
 * Exported as GetSelectedPaths method on Tracker.Status interface.
 */
gboolean
na_tracker_dbus_get_selected_paths( NATrackerDBus *tracker, char ***paths, GError **error )
{
	static const gchar *thisfn = "na_tracker_dbus_get_selected_paths";
	*error = NULL;
	GList *it;
	int count;
	gchar **iter;

	g_debug( "%s: object=%p, paths=%p, error=%p", thisfn, ( void * ) tracker, ( void * ) paths, ( void * ) error );

	*error = NULL;
	*paths = NULL;

	if( !tracker->private->dispose_has_run ){

		count = 2 * g_list_length( tracker->private->selected );
		*paths = ( char ** ) g_new0( gchar *, 1+count );
		iter = *paths;

		for( it = tracker->private->selected ; it ; it = it->next ){
			*iter = caja_file_info_get_uri(( CajaFileInfo * ) it->data );
			iter++;
			*iter = caja_file_info_get_mime_type(( CajaFileInfo * ) it->data );
			iter++;
		}
	}

	return( TRUE );
}

static GList *
free_selected( GList *selected )
{
	caja_file_info_list_free( selected );

	return( NULL );
}
