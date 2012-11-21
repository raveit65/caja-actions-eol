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

#include <api/na-ifactory-provider.h>
#include <api/na-iexporter.h>
#include <api/na-iimporter.h>

#include "caxml-provider.h"
#include "caxml-formats.h"
#include "caxml-reader.h"
#include "caxml-writer.h"

/* private class data
 */
struct _CAXMLProviderClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CAXMLProviderPrivate {
	gboolean dispose_has_run;
};

static GType         st_module_type = 0;
static GObjectClass *st_parent_class = NULL;

static void   class_init( CAXMLProviderClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *object );
static void   instance_finalize( GObject *object );

static void   iimporter_iface_init( NAIImporterInterface *iface );
static guint  iimporter_get_version( const NAIImporter *importer );

static void   iexporter_iface_init( NAIExporterInterface *iface );
static guint  iexporter_get_version( const NAIExporter *exporter );
static gchar *iexporter_get_name( const NAIExporter *exporter );
static void  *iexporter_get_formats( const NAIExporter *exporter );
static void   iexporter_free_formats( const NAIExporter *exporter, GList *format_list );

static void   ifactory_provider_iface_init( NAIFactoryProviderInterface *iface );
static guint  ifactory_provider_get_version( const NAIFactoryProvider *factory );

GType
caxml_provider_get_type( void )
{
	return( st_module_type );
}

void
caxml_provider_register_type( GTypeModule *module )
{
	static const gchar *thisfn = "caxml_provider_register_type";

	static GTypeInfo info = {
		sizeof( CAXMLProviderClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CAXMLProvider ),
		0,
		( GInstanceInitFunc ) instance_init
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

	static const GInterfaceInfo ifactory_provider_iface_info = {
		( GInterfaceInitFunc ) ifactory_provider_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	st_module_type = g_type_module_register_type( module, G_TYPE_OBJECT, "CAXMLProvider", &info, 0 );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IIMPORTER, &iimporter_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IEXPORTER, &iexporter_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IFACTORY_PROVIDER, &ifactory_provider_iface_info );
}

static void
class_init( CAXMLProviderClass *klass )
{
	static const gchar *thisfn = "caxml_provider_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CAXMLProviderClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "caxml_provider_instance_init";
	CAXMLProvider *self;

	g_return_if_fail( NA_IS_XML_PROVIDER( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CAXML_PROVIDER( instance );

	self->private = g_new0( CAXMLProviderPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "caxml_provider_instance_dispose";
	CAXMLProvider *self;

	g_return_if_fail( NA_IS_XML_PROVIDER( object ));

	self = CAXML_PROVIDER( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

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
	static const gchar *thisfn = "caxml_provider_instance_finalize";
	CAXMLProvider *self;

	g_return_if_fail( NA_IS_XML_PROVIDER( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = CAXML_PROVIDER( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
iimporter_iface_init( NAIImporterInterface *iface )
{
	static const gchar *thisfn = "caxml_provider_iimporter_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = iimporter_get_version;
	iface->import_from_uri = caxml_reader_import_from_uri;
}

static guint
iimporter_get_version( const NAIImporter *importer )
{
	return( 2 );
}

static void
iexporter_iface_init( NAIExporterInterface *iface )
{
	static const gchar *thisfn = "caxml_provider_iexporter_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = iexporter_get_version;
	iface->get_name = iexporter_get_name;
	iface->get_formats = iexporter_get_formats;
	iface->free_formats = iexporter_free_formats;
	iface->to_file = caxml_writer_export_to_file;
	iface->to_buffer = caxml_writer_export_to_buffer;
}

static guint
iexporter_get_version( const NAIExporter *exporter )
{
	return( 2 );
}

static gchar *
iexporter_get_name( const NAIExporter *exporter )
{
	return( g_strdup( "CAXML Exporter" ));
}

static void *
iexporter_get_formats( const NAIExporter *exporter )
{
	return(( void * ) caxml_formats_get_formats( exporter ));
}

static void
iexporter_free_formats( const NAIExporter *exporter, GList *format_list )
{
	caxml_formats_free_formats( format_list );
}

static void
ifactory_provider_iface_init( NAIFactoryProviderInterface *iface )
{
	static const gchar *thisfn = "caxml_provider_ifactory_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = ifactory_provider_get_version;
	iface->read_start = caxml_reader_read_start;
	iface->read_data = caxml_reader_read_data;
	iface->read_done = caxml_reader_read_done;
	iface->write_start = caxml_writer_write_start;
	iface->write_data = caxml_writer_write_data;
	iface->write_done = caxml_writer_write_done;
}

static guint
ifactory_provider_get_version( const NAIFactoryProvider *factory )
{
	return( 1 );
}
