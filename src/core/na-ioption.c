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

#include "na-ioption.h"

/* private interface data
 */
struct _NAIOptionInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* data set against the instance
 *
 * Initialization here mainly means setting the weak ref against the instance.
 */
typedef struct {
	gboolean initialized;
}
	IOptionData;

#define IOPTION_PROP_DATA				"na-prop-ioption-data"

static guint st_initializations = 0;	/* interface initialization count */

static GType        register_type( void );
static void         interface_base_init( NAIOptionInterface *iface );
static void         interface_base_finalize( NAIOptionInterface *iface );

static guint        ioption_get_version( const NAIOption *instance );
static IOptionData *get_ioption_data( NAIOption *instance );
static void         on_instance_finalized( gpointer user_data, NAIOption *instance );

/**
 * na_ioption_get_type:
 *
 * Returns: the #GType type of this interface.
 */
GType
na_ioption_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

/*
 * na_ioption_register_type:
 *
 * Registers this interface.
 */
static GType
register_type( void )
{
	static const gchar *thisfn = "na_ioption_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIOptionInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIOption", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIOptionInterface *iface )
{
	static const gchar *thisfn = "na_ioption_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: iface=%p (%s)", thisfn, ( void * ) iface, G_OBJECT_CLASS_NAME( iface ));

		iface->private = g_new0( NAIOptionInterfacePrivate, 1 );

		iface->get_version = ioption_get_version;
	}

	st_initializations += 1;
}

static void
interface_base_finalize( NAIOptionInterface *iface )
{
	static const gchar *thisfn = "na_ioption_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

		g_free( iface->private );
	}
}

static guint
ioption_get_version( const NAIOption *instance )
{
	return( 1 );
}

static IOptionData *
get_ioption_data( NAIOption *instance )
{
	IOptionData *data;

	data = ( IOptionData * ) g_object_get_data( G_OBJECT( instance ), IOPTION_PROP_DATA );

	if( !data ){
		data = g_new0( IOptionData, 1 );
		g_object_set_data( G_OBJECT( instance ), IOPTION_PROP_DATA, data );
		g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );

		data->initialized = TRUE;
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, NAIOption *instance )
{
	static const gchar *thisfn = "na_ioption_on_instance_finalized";
	IOptionData *data;

	g_debug( "%s: user_data=%p, instance=%p", thisfn, ( void * ) user_data, ( void * ) instance );

	data = get_ioption_data( instance );

	g_free( data );
}

/*
 * na_ioption_get_id:
 * @option: this #NAIOption instance.
 *
 * Returns: the string identifier of the format, as a newly
 * allocated string which should be g_free() by the caller.
 */
gchar *
na_ioption_get_id( const NAIOption *option )
{
	gchar *id;

	g_return_val_if_fail( NA_IS_IOPTION( option ), NULL );

	get_ioption_data( NA_IOPTION( option ));
	id = NULL;

	if( NA_IOPTION_GET_INTERFACE( option )->get_id ){
		id = NA_IOPTION_GET_INTERFACE( option )->get_id( option );
	}

	return( id );
}

/*
 * na_ioption_get_label:
 * @option: this #NAIOption instance.
 *
 * Returns: the UTF-8 localizable label of the format, as a newly
 * allocated string which should be g_free() by the caller.
 */
gchar *
na_ioption_get_label( const NAIOption *option )
{
	gchar *label;

	g_return_val_if_fail( NA_IS_IOPTION( option ), NULL );

	get_ioption_data( NA_IOPTION( option ));
	label = NULL;

	if( NA_IOPTION_GET_INTERFACE( option )->get_label ){
		label = NA_IOPTION_GET_INTERFACE( option )->get_label( option );
	}

	return( label );
}

/*
 * na_ioption_get_description:
 * @format: this #NAExportFormat object.
 *
 * Returns: the UTF-8 localizable description of the format, as a newly
 * allocated string which should be g_free() by the caller.
 */
gchar *
na_ioption_get_description( const NAIOption *option )
{
	gchar *description;

	g_return_val_if_fail( NA_IS_IOPTION( option ), NULL );

	get_ioption_data( NA_IOPTION( option ));
	description = NULL;

	if( NA_IOPTION_GET_INTERFACE( option )->get_description ){
		description = NA_IOPTION_GET_INTERFACE( option )->get_description( option );
	}

	return( description );
}

/*
 * na_ioption_get_pixbuf:
 * @option: this #NAIOption instance.
 *
 * Returns: a new reference to the #GdkPixbuf image associated with this format,
 * or %NULL.
 */
GdkPixbuf *
na_ioption_get_pixbuf( const NAIOption *option )
{
	GdkPixbuf *pixbuf;

	g_return_val_if_fail( NA_IS_IOPTION( option ), NULL );

	get_ioption_data( NA_IOPTION( option ));
	pixbuf = NULL;

	if( NA_IOPTION_GET_INTERFACE( option )->get_pixbuf ){
		pixbuf = NA_IOPTION_GET_INTERFACE( option )->get_pixbuf( option );
	}

	return( pixbuf );
}
