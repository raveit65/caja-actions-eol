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

#include <api/na-core-utils.h>
#include <api/na-ifactory-provider.h>

#include "cadp-desktop-provider.h"
#include "cadp-formats.h"
#include "cadp-keys.h"
#include "cadp-monitor.h"
#include "cadp-reader.h"
#include "cadp-writer.h"

/* private class data
 */
struct _CappDesktopProviderClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static GType         st_module_type = 0;
static GObjectClass *st_parent_class = NULL;
static guint         st_burst_timeout = 100;		/* burst timeout in msec */

static void   class_init( CappDesktopProviderClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *object );
static void   instance_finalize( GObject *object );

static void   iio_provider_iface_init( NAIIOProviderInterface *iface );
static gchar *iio_provider_get_id( const NAIIOProvider *provider );
static gchar *iio_provider_get_name( const NAIIOProvider *provider );
static guint  iio_provider_get_version( const NAIIOProvider *provider );

static void   ifactory_provider_iface_init( NAIFactoryProviderInterface *iface );
static guint  ifactory_provider_get_version( const NAIFactoryProvider *reader );

static void   iimporter_iface_init( NAIImporterInterface *iface );
static guint  iimporter_get_version( const NAIImporter *importer );

static void   iexporter_iface_init( NAIExporterInterface *iface );
static guint  iexporter_get_version( const NAIExporter *exporter );
static gchar *iexporter_get_name( const NAIExporter *exporter );
static void  *iexporter_get_formats( const NAIExporter *exporter );
static void   iexporter_free_formats( const NAIExporter *exporter, GList *format_list );

static void   on_monitor_timeout( CappDesktopProvider *provider );

GType
cadp_desktop_provider_get_type( void )
{
	return( st_module_type );
}

void
cadp_desktop_provider_register_type( GTypeModule *module )
{
	static const gchar *thisfn = "cadp_desktop_provider_register_type";

	static GTypeInfo info = {
		sizeof( CappDesktopProviderClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CappDesktopProvider ),
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

	static const GInterfaceInfo iimporter_iface_info = {
		( GInterfaceInitFunc ) iimporter_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo iexporter_iface_info = {
		( GInterfaceInitFunc ) iexporter_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	st_module_type = g_type_module_register_type( module, G_TYPE_OBJECT, "CappDesktopProvider", &info, 0 );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IIO_PROVIDER, &iio_provider_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IFACTORY_PROVIDER, &ifactory_provider_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IIMPORTER, &iimporter_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IEXPORTER, &iexporter_iface_info );
}

static void
class_init( CappDesktopProviderClass *klass )
{
	static const gchar *thisfn = "cadp_desktop_provider_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CappDesktopProviderClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cadp_desktop_provider_instance_init";
	CappDesktopProvider *self;

	g_return_if_fail( CADP_IS_DESKTOP_PROVIDER( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CADP_DESKTOP_PROVIDER( instance );

	self->private = g_new0( CappDesktopProviderPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->monitors = NULL;
	self->private->timeout.timeout = st_burst_timeout;
	self->private->timeout.handler = ( NATimeoutFunc ) on_monitor_timeout;
	self->private->timeout.user_data = self;
	self->private->timeout.source_id = 0;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cadp_desktop_provider_instance_dispose";
	CappDesktopProvider *self;

	g_return_if_fail( CADP_IS_DESKTOP_PROVIDER( object ));

	self = CADP_DESKTOP_PROVIDER( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		cadp_desktop_provider_release_monitors( self );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "cadp_desktop_provider_instance_finalize";
	CappDesktopProvider *self;

	g_return_if_fail( CADP_IS_DESKTOP_PROVIDER( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = CADP_DESKTOP_PROVIDER( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
iio_provider_iface_init( NAIIOProviderInterface *iface )
{
	static const gchar *thisfn = "cadp_desktop_provider_iio_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = iio_provider_get_version;
	iface->get_id = iio_provider_get_id;
	iface->get_name = iio_provider_get_name;
	iface->read_items = cadp_iio_provider_read_items;
	iface->is_willing_to_write = cadp_iio_provider_is_willing_to_write;
	iface->is_able_to_write = cadp_iio_provider_is_able_to_write;
	iface->write_item = cadp_iio_provider_write_item;
	iface->delete_item = cadp_iio_provider_delete_item;
	iface->duplicate_data = cadp_iio_provider_duplicate_data;
}

static guint
iio_provider_get_version( const NAIIOProvider *provider )
{
	return( 1 );
}

static gchar *
iio_provider_get_id( const NAIIOProvider *provider )
{
	return( g_strdup( PROVIDER_ID ));
}

static gchar *
iio_provider_get_name( const NAIIOProvider *provider )
{
	return( g_strdup( _( "Caja-Actions Desktop I/O Provider" )));
}

static void
ifactory_provider_iface_init( NAIFactoryProviderInterface *iface )
{
	static const gchar *thisfn = "cadp_desktop_provider_ifactory_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = ifactory_provider_get_version;
	iface->read_start = cadp_reader_ifactory_provider_read_start;
	iface->read_data = cadp_reader_ifactory_provider_read_data;
	iface->read_done = cadp_reader_ifactory_provider_read_done;
	iface->write_start = cadp_writer_ifactory_provider_write_start;
	iface->write_data = cadp_writer_ifactory_provider_write_data;
	iface->write_done = cadp_writer_ifactory_provider_write_done;
}

static guint
ifactory_provider_get_version( const NAIFactoryProvider *reader )
{
	return( 1 );
}

static void
iimporter_iface_init( NAIImporterInterface *iface )
{
	static const gchar *thisfn = "cadp_desktop_provider_iimporter_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = iimporter_get_version;
	iface->import_from_uri = cadp_reader_iimporter_import_from_uri;
}

static guint
iimporter_get_version( const NAIImporter *importer )
{
	return( 2 );
}

static void
iexporter_iface_init( NAIExporterInterface *iface )
{
	static const gchar *thisfn = "cadp_desktop_iexporter_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = iexporter_get_version;
	iface->get_name = iexporter_get_name;
	iface->get_formats = iexporter_get_formats;
	iface->free_formats = iexporter_free_formats;
	iface->to_file = cadp_writer_iexporter_export_to_file;
	iface->to_buffer = cadp_writer_iexporter_export_to_buffer;
}

static guint
iexporter_get_version( const NAIExporter *exporter )
{
	return( 2 );
}

static gchar *
iexporter_get_name( const NAIExporter *exporter )
{
	return( g_strdup( "NA Desktop Exporter" ));
}

static void *
iexporter_get_formats( const NAIExporter *exporter )
{
	return(( void * ) cadp_formats_get_formats( exporter ));
}

static void
iexporter_free_formats( const NAIExporter *exporter, GList *format_list )
{
	cadp_formats_free_formats( format_list );
}

/**
 * cadp_desktop_provider_add_monitor:
 * @provider: this #CappDesktopProvider object.
 * @dir: the path to the directory to be monitored. May not exist.
 *
 * Installs a GIO monitor on the given directory.
 */
void
cadp_desktop_provider_add_monitor( CappDesktopProvider *provider, const gchar *dir )
{
	CappMonitor *monitor;

	g_return_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ));

	if( !provider->private->dispose_has_run ){

		monitor = cadp_monitor_new( provider, dir );
		provider->private->monitors = g_list_prepend( provider->private->monitors, monitor );
	}
}

/**
 * cadp_desktop_provider_on_monitor_event:
 * @provider: this #CappDesktopProvider object.
 *
 * Factorize events received from GIO when monitoring desktop directories.
 */
void
cadp_desktop_provider_on_monitor_event( CappDesktopProvider *provider )
{
	g_return_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ));

	if( !provider->private->dispose_has_run ){

		na_timeout_event( &provider->private->timeout );
	}
}

/**
 * cadp_desktop_provider_release_monitors:
 * @provider: this #CappDesktopProvider object.
 *
 * Release previously set desktop monitors.
 */
void
cadp_desktop_provider_release_monitors( CappDesktopProvider *provider )
{
	g_return_if_fail( CADP_IS_DESKTOP_PROVIDER( provider ));

	if( provider->private->monitors ){

		g_list_foreach( provider->private->monitors, ( GFunc ) g_object_unref, NULL );
		g_list_free( provider->private->monitors );
		provider->private->monitors = NULL;
	}
}

static void
on_monitor_timeout( CappDesktopProvider *provider )
{
	static const gchar *thisfn = "cadp_desktop_provider_on_monitor_timeout";

	/* last individual notification is older that the st_burst_timeout
	 * so triggers the NAIIOProvider interface and destroys this timeout
	 */
	g_debug( "%s: triggering NAIIOProvider interface for provider=%p (%s)",
			thisfn, ( void * ) provider, G_OBJECT_TYPE_NAME( provider ));

	na_iio_provider_item_changed( NA_IIO_PROVIDER( provider ));
}
