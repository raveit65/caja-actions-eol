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

#include <api/na-iimporter.h>

#include "na-importer-ask.h"

/* private interface data
 */
struct NAIImporterInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

gboolean iimporter_initialized = FALSE;
gboolean iimporter_finalized   = FALSE;

static GType  register_type( void );
static void   interface_base_init( NAIImporterInterface *klass );
static void   interface_base_finalize( NAIImporterInterface *klass );

static guint  iimporter_get_version( const NAIImporter *instance );

/**
 * na_iimporter_get_type:
 *
 * Returns: the #GType type of this interface.
 */
GType
na_iimporter_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

/*
 * na_iimporter_register_type:
 *
 * Registers this interface.
 */
static GType
register_type( void )
{
	static const gchar *thisfn = "na_iimporter_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIImporterInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIImporter", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIImporterInterface *klass )
{
	static const gchar *thisfn = "na_iimporter_interface_base_init";

	if( !iimporter_initialized ){

		g_debug( "%s: klass%p (%s)", thisfn, ( void * ) klass, G_OBJECT_CLASS_NAME( klass ));

		klass->private = g_new0( NAIImporterInterfacePrivate, 1 );

		klass->get_version = iimporter_get_version;
		klass->from_uri = NULL;

		iimporter_initialized = TRUE;
	}
}

static void
interface_base_finalize( NAIImporterInterface *klass )
{
	static const gchar *thisfn = "na_iimporter_interface_base_finalize";

	if( iimporter_initialized && !iimporter_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		iimporter_finalized = TRUE;

		g_free( klass->private );
	}
}

static guint
iimporter_get_version( const NAIImporter *instance )
{
	return( 1 );
}

/**
 * na_iimporter_ask_user:
 * @importer: this #NAIImporter instance.
 * @parms: a #NAIImporterUriParms structure.
 * @existing: the #NAObjectItem-derived already existing object.
 *
 * Ask the user for what to do when an imported item has the same ID
 * that an already existing one.
 *
 * Returns: the definitive import mode.
 */
guint
na_iimporter_ask_user( const NAIImporter *importer, const NAIImporterUriParms *parms, const NAObjectItem *existing )
{
	guint mode;

	mode = na_importer_ask_user( parms, existing );

	return( mode );
}
