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

#include "na-export-format.h"
#include "na-ioption.h"

/* private class data
 */
struct _NAExportFormatClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _NAExportFormatPrivate {
	gboolean     dispose_has_run;
	gchar       *format;
	gchar       *label;
	gchar       *description;
	GdkPixbuf   *pixbuf;
	NAIExporter *provider;
};

static GObjectClass *st_parent_class = NULL;

static GType      register_type( void );
static void       class_init( NAExportFormatClass *klass );
static void       ioption_iface_init( NAIOptionInterface *iface, void *user_data );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_dispose( GObject *object );
static void       instance_finalize( GObject *object );
static gchar     *ioption_get_id( const NAIOption *option );
static gchar     *ioption_get_label( const NAIOption *option );
static gchar     *ioption_get_description( const NAIOption *option );
static GdkPixbuf *ioption_get_pixbuf( const NAIOption *option );

GType
na_export_format_get_type( void )
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
	static const gchar *thisfn = "na_export_format_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAExportFormatClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAExportFormat ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo ioption_iface_info = {
		( GInterfaceInitFunc ) ioption_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NAExportFormat", &info, 0 );

	g_type_add_interface_static( type, NA_TYPE_IOPTION, &ioption_iface_info );

	return( type );
}

static void
class_init( NAExportFormatClass *klass )
{
	static const gchar *thisfn = "na_export_format_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAExportFormatClassPrivate, 1 );
}

static void
ioption_iface_init( NAIOptionInterface *iface, void *user_data )
{
	static const gchar *thisfn = "na_export_format_ioption_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->get_id = ioption_get_id;
	iface->get_label = ioption_get_label;
	iface->get_description = ioption_get_description;
	iface->get_pixbuf = ioption_get_pixbuf;
}

/*
 * ioption_get_id:
 * @option: this #NAIOption instance.
 *
 * Returns: the ASCII id of the @option, as a newly allocated string which
 * should be g_free() by the caller.
 */
static gchar *
ioption_get_id( const NAIOption *option )
{
	gchar *id;
	NAExportFormat *format;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( option ), NULL );
	format = NA_EXPORT_FORMAT( option );
	id = NULL;

	if( !format->private->dispose_has_run ){

		id = g_strdup( format->private->format );
	}

	return( id );
}

/*
 * ioption_get_label:
 * @option: this #NAIOption instance.
 *
 * Returns: the label associated to @option, as a newly allocated string
 * which should be g_free() by the caller.
 */
static gchar *
ioption_get_label( const NAIOption *option )
{
	gchar *label;
	NAExportFormat *format;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( option ), NULL );
	format = NA_EXPORT_FORMAT( option );
	label = NULL;

	if( !format->private->dispose_has_run ){

		label = g_strdup( format->private->label );
	}

	return( label );
}

/*
 * ioption_get_description:
 * @option: this #NAIOption instance.
 *
 * Returns: the description associated to @option, as a newly allocated string
 * which should be g_free() by the caller.
 */
static gchar *
ioption_get_description( const NAIOption *option )
{
	gchar *description;
	NAExportFormat *format;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( option ), NULL );
	format = NA_EXPORT_FORMAT( option );
	description = NULL;

	if( !format->private->dispose_has_run ){

		description = g_strdup( format->private->description );
	}

	return( description );
}

/*
 * ioption_get_pixbuf:
 * @option: this #NAIOption instance.
 *
 * Returns: a new reference to the pixbuf associated to @option;
 * which should later be g_object_unref() by the caller.
 */
static GdkPixbuf *
ioption_get_pixbuf( const NAIOption *option )
{
	GdkPixbuf *pixbuf;
	NAExportFormat *format;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( option ), NULL );
	format = NA_EXPORT_FORMAT( option );
	pixbuf = NULL;

	if( !format->private->dispose_has_run ){

		pixbuf = format->private->pixbuf ? g_object_ref( format->private->pixbuf ) : NULL;
	}

	return( pixbuf );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_export_format_instance_init";
	NAExportFormat *self;

	g_return_if_fail( NA_IS_EXPORT_FORMAT( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	self = NA_EXPORT_FORMAT( instance );

	self->private = g_new0( NAExportFormatPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_export_format_instance_dispose";
	NAExportFormat *self;

	g_return_if_fail( NA_IS_EXPORT_FORMAT( object ));

	self = NA_EXPORT_FORMAT( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		if( self->private->pixbuf ){
			g_debug( "%s: pixbuf=%p (%s) ref_count=%d",
					thisfn,
					( void * ) self->private->pixbuf,
					G_OBJECT_TYPE_NAME( self->private->pixbuf ),
					G_OBJECT( self->private->pixbuf )->ref_count );
			g_object_unref( self->private->pixbuf );
			self->private->pixbuf = NULL;
		}

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_export_format_instance_finalize";
	NAExportFormat *self;

	g_return_if_fail( NA_IS_EXPORT_FORMAT( object ));

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	self = NA_EXPORT_FORMAT( object );

	g_free( self->private->format );
	g_free( self->private->label );
	g_free( self->private->description );
	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/*
 * na_export_format_new:
 * @exporter_format: a #NAIExporterFormatv2 which describes an export format.
 *
 * Returns: a newly allocated #NAExportFormat object.
 */
NAExportFormat *
na_export_format_new( const NAIExporterFormatv2 *exporter_format )
{
	NAExportFormat *format;

	format = g_object_new( NA_TYPE_EXPORT_FORMAT, NULL );

	format->private->format = g_strdup( exporter_format->format );
	format->private->label = g_strdup( exporter_format->label );
	format->private->description = g_strdup( exporter_format->description );
	format->private->pixbuf = exporter_format->pixbuf ? g_object_ref( exporter_format->pixbuf ) : NULL;
	format->private->provider = exporter_format->provider;

	return( format );
}

/*
 * na_export_format_get_provider:
 * @format: this #NAExportFormat object.
 *
 * Returns: a pointer to the #NAIExporter which provides this format.
 *
 * The pointer is owned by NAEportFormat class, and should not be released
 * by the caller.
 */
NAIExporter *
na_export_format_get_provider( const NAExportFormat *format )
{
	NAIExporter *exporter;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( format ), NULL );

	exporter = NULL;

	if( !format->private->dispose_has_run ){

		exporter = format->private->provider;
	}

	return( exporter );
}
