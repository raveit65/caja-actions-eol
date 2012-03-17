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

#include <api/na-iexporter.h>

/* private interface data
 */
struct NAIExporterInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

gboolean iexporter_initialized = FALSE;
gboolean iexporter_finalized   = FALSE;

static GType register_type( void );
static void  interface_base_init( NAIExporterInterface *klass );
static void  interface_base_finalize( NAIExporterInterface *klass );

static guint iexporter_get_version( const NAIExporter *instance );

/**
 * na_iexporter_get_type:
 *
 * Returns: the #GType type of this interface.
 */
GType
na_iexporter_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

/*
 * na_iexporter_register_type:
 *
 * Registers this interface.
 */
static GType
register_type( void )
{
	static const gchar *thisfn = "na_iexporter_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIExporterInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIExporter", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIExporterInterface *klass )
{
	static const gchar *thisfn = "na_iexporter_interface_base_init";

	if( !iexporter_initialized ){

		g_debug( "%s: klass%p (%s)", thisfn, ( void * ) klass, G_OBJECT_CLASS_NAME( klass ));

		klass->private = g_new0( NAIExporterInterfacePrivate, 1 );

		klass->get_version = iexporter_get_version;
		klass->get_name = NULL;
		klass->get_formats = NULL;
		klass->to_file = NULL;
		klass->to_buffer = NULL;

		iexporter_initialized = TRUE;
	}
}

static void
interface_base_finalize( NAIExporterInterface *klass )
{
	static const gchar *thisfn = "na_iexporter_interface_base_finalize";

	if( iexporter_initialized && !iexporter_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		iexporter_finalized = TRUE;

		g_free( klass->private );
	}
}

static guint
iexporter_get_version( const NAIExporter *instance )
{
	return( 1 );
}
