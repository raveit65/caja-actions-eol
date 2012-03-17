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

#include "na-export-format.h"

/* private class data
 */
struct NAExportFormatClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAExportFormatPrivate {
	gboolean           dispose_has_run;
	GQuark             id;
	NAIExporterFormat *str;
	NAIExporter       *exporter;
};

static GObjectClass *st_parent_class = NULL;

static GType  register_type( void );
static void   class_init( NAExportFormatClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *object );
static void   instance_finalize( GObject *object );

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

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NAExportFormat", &info, 0 );

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
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_export_format_instance_init";
	NAExportFormat *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( NA_IS_EXPORT_FORMAT( instance ));
	self = NA_EXPORT_FORMAT( instance );

	self->private = g_new0( NAExportFormatPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_export_format_instance_dispose";
	NAExportFormat *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NA_IS_EXPORT_FORMAT( object ));
	self = NA_EXPORT_FORMAT( object );

	if( !self->private->dispose_has_run ){

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
	static const gchar *thisfn = "na_export_format_instance_finalize";
	NAExportFormat *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_EXPORT_FORMAT( object ));
	self = NA_EXPORT_FORMAT( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * na_export_format_new:
 * @str: a #NAIExporterFormat which describes an export format.
 * @exporter: the #NAIExporter which provides this export format.
 *
 * Returns: a newly allocated #NAExportFormat object.
 */
NAExportFormat *
na_export_format_new( const NAIExporterFormat *str, const NAIExporter *exporter )
{
	NAExportFormat *format;

	format = g_object_new( NA_EXPORT_FORMAT_TYPE, NULL );

	format->private->id = g_quark_from_string( str->format );
	format->private->str = ( NAIExporterFormat * ) str;
	format->private->exporter = ( NAIExporter * ) exporter;

	return( format );
}

/**
 * na_export_format_get_quark:
 * @format: this #NAExportFormat object.
 *
 * Returns: the #GQuark associated with this format.
 */
GQuark
na_export_format_get_quark( const NAExportFormat *format )
{
	GQuark id;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( format ), 0 );

	id = 0;

	if( !format->private->dispose_has_run ){

		id = format->private->id;
	}

	return( id );
}

/**
 * na_export_format_get_id:
 * @format: this #NAExportFormat object.
 *
 * Returns: the ASCII id of the format, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
na_export_format_get_id( const NAExportFormat *format )
{
	gchar *id;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( format ), NULL );

	id = NULL;

	if( !format->private->dispose_has_run ){

		id = g_strdup( format->private->str->format );
	}

	return( id );
}

/**
 * na_export_format_get_label:
 * @format: this #NAExportFormat object.
 *
 * Returns: the UTF-8 localizable label of the format, as a newly
 * allocated string which should be g_free() by the caller.
 */
gchar *
na_export_format_get_label( const NAExportFormat *format )
{
	gchar *label;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( format ), NULL );

	label = NULL;

	if( !format->private->dispose_has_run ){

		label = g_strdup( format->private->str->label );
	}

	return( label );
}

/**
 * na_export_format_get_description:
 * @format: this #NAExportFormat object.
 *
 * Returns: the UTF-8 localizable description of the format, as a newly
 * allocated string which should be g_free() by the caller.
 */
gchar *
na_export_format_get_description( const NAExportFormat *format )
{
	gchar *description;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( format ), NULL );

	description = NULL;

	if( !format->private->dispose_has_run ){

		description = g_strdup( format->private->str->description );
	}

	return( description );
}

/**
 * na_export_format_get_exporter:
 * @format: this #NAExportFormat object.
 *
 * Returns: a pointer to the #NAIExporter which providers this format.
 */
NAIExporter *
na_export_format_get_exporter( const NAExportFormat *format )
{
	NAIExporter *exporter;

	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( format ), NULL );

	exporter = NULL;

	if( !format->private->dispose_has_run ){

		exporter = format->private->exporter;
	}

	return( exporter );
}
