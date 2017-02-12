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

#include <api/na-iio-provider.h>
#include <api/na-ifactory-provider.h>

#include "na-factory-object.h"
#include "na-factory-provider.h"

/* private interface data
 */
struct _NAIFactoryProviderInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static guint st_initializations = 0;	/* interface initialization count */

static GType register_type( void );
static void  interface_base_init( NAIFactoryProviderInterface *klass );
static void  interface_base_finalize( NAIFactoryProviderInterface *klass );

static guint ifactory_provider_get_version( const NAIFactoryProvider *instance );

static void  v_factory_provider_read_start( const NAIFactoryProvider *reader, void *reader_data, NAIFactoryObject *serializable, GSList **messages );
static void  v_factory_provider_read_done( const NAIFactoryProvider *reader, void *reader_data, NAIFactoryObject *serializable, GSList **messages );
static guint v_factory_provider_write_start( const NAIFactoryProvider *writer, void *writer_data, NAIFactoryObject *serializable, GSList **messages );
static guint v_factory_provider_write_done( const NAIFactoryProvider *writer, void *writer_data, NAIFactoryObject *serializable, GSList **messages );

/**
 * na_ifactory_provider_get_type:
 *
 * Registers the GType of this interface.
 *
 * Returns: the #NAIFactoryProvider #GType.
 */
GType
na_ifactory_provider_get_type( void )
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
	static const gchar *thisfn = "na_ifactory_provider_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIFactoryProviderInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIFactoryProvider", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIFactoryProviderInterface *klass )
{
	static const gchar *thisfn = "na_ifactory_provider_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p (%s)", thisfn, ( void * ) klass, G_OBJECT_CLASS_NAME( klass ));

		klass->private = g_new0( NAIFactoryProviderInterfacePrivate, 1 );

		klass->get_version = ifactory_provider_get_version;
		klass->read_start = NULL;
		klass->read_data = NULL;
		klass->read_done = NULL;
		klass->write_start = NULL;
		klass->write_data = NULL;
		klass->write_done = NULL;
	}

	st_initializations += 1;
}

static void
interface_base_finalize( NAIFactoryProviderInterface *klass )
{
	static const gchar *thisfn = "na_ifactory_provider_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

static guint
ifactory_provider_get_version( const NAIFactoryProvider *instance )
{
	return( 1 );
}

/**
 * na_ifactory_provider_read_item:
 * @reader: the instance which implements this #NAIFactoryProvider interface.
 * @reader_data: instance data which will be provided back to the interface
 *  methods
 * @object: the #NAIFactoryObject object to be unserialilzed.
 * @messages: a pointer to a #GSList list of strings; the implementation
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * This function is to be called by a #NAIIOProvider which would wish read
 * its items. The function takes care of collecting and structuring data,
 * while the callback interface methods #NAIFactoryProviderInterface.read_start(),
 * #NAIFactoryProviderInterface.read_data() and #NAIFactoryProviderInterface.read_done()
 * just have to fill a given #NADataBoxed with the ad-hoc data type.
 *
 * <example>
 *   <programlisting>
 *     &lcomment;
 *      * allocate the object to be read
 *      &rcomment;
 *     NAObjectItem *item = NA_OBJECT_ITEM( na_object_action_new());
 *     &lcomment;
 *      * some data we may need to have access to in callback methods
 *      &rcomment;
 *     void *data;
 *     &lcomment;
 *      * now call interface function
 *      &rcomment;
 *     na_ifactory_provider_read_item(
 *         NA_IFACTORY_PROVIDER( provider ),
 *         data,
 *         NA_IFACTORY_OBJECT( item ),
 *         messages );
 *   </programlisting>
 * </example>
 *
 * Since: 2.30
 */
void
na_ifactory_provider_read_item( const NAIFactoryProvider *reader, void *reader_data, NAIFactoryObject *object, GSList **messages )
{
	g_return_if_fail( NA_IS_IFACTORY_PROVIDER( reader ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	v_factory_provider_read_start( reader, reader_data, object, messages );
	na_factory_object_read_item( object, reader, reader_data, messages );
	v_factory_provider_read_done( reader, reader_data, object, messages );
}

/**
 * na_ifactory_provider_write_item:
 * @writer: the instance which implements this #NAIFactoryProvider interface.
 * @writer_data: instance data.
 * @object: the #NAIFactoryObject derived object to be serialized.
 * @messages: a pointer to a #GSList list of strings; the implementation
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * This function is to be called by a #NAIIOProvider which would wish write
 * an item. The function takes care of collecting and writing elementary data.
 *
 * Returns: a NAIIOProvider operation return code.
 *
 * Since: 2.30
 */
guint
na_ifactory_provider_write_item( const NAIFactoryProvider *writer, void *writer_data, NAIFactoryObject *object, GSList **messages )
{
	static const gchar *thisfn = "na_ifactory_provider_write_item";
	guint code;

	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( writer ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );

	g_debug( "%s: writer=%p, writer_data=%p, object=%p (%s)",
			thisfn, ( void * ) writer, ( void * ) writer_data, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	code = v_factory_provider_write_start( writer, writer_data, object, messages );

	if( code == NA_IIO_PROVIDER_CODE_OK ){
		code = na_factory_object_write_item( object, writer, writer_data, messages );
	}

	if( code == NA_IIO_PROVIDER_CODE_OK ){
		code = v_factory_provider_write_done( writer, writer_data, object, messages );
	}

	return( code );
}

static void
v_factory_provider_read_start( const NAIFactoryProvider *reader, void *reader_data, NAIFactoryObject *serializable, GSList **messages )
{
	if( NA_IFACTORY_PROVIDER_GET_INTERFACE( reader )->read_start ){
		NA_IFACTORY_PROVIDER_GET_INTERFACE( reader )->read_start( reader, reader_data, serializable, messages );
	}
}

static void
v_factory_provider_read_done( const NAIFactoryProvider *reader, void *reader_data, NAIFactoryObject *serializable, GSList **messages )
{
	if( NA_IFACTORY_PROVIDER_GET_INTERFACE( reader )->read_done ){
		NA_IFACTORY_PROVIDER_GET_INTERFACE( reader )->read_done( reader, reader_data, serializable, messages );
	}
}

static guint
v_factory_provider_write_start( const NAIFactoryProvider *writer, void *writer_data, NAIFactoryObject *serializable, GSList **messages )
{
	guint code = NA_IIO_PROVIDER_CODE_OK;

	if( NA_IFACTORY_PROVIDER_GET_INTERFACE( writer )->write_start ){
		code = NA_IFACTORY_PROVIDER_GET_INTERFACE( writer )->write_start( writer, writer_data, serializable, messages );
	}

	return( code );
}

static guint
v_factory_provider_write_done( const NAIFactoryProvider *writer, void *writer_data, NAIFactoryObject *serializable, GSList **messages )
{
	guint code = NA_IIO_PROVIDER_CODE_OK;

	if( NA_IFACTORY_PROVIDER_GET_INTERFACE( writer )->write_done ){
		code = NA_IFACTORY_PROVIDER_GET_INTERFACE( writer )->write_done( writer, writer_data, serializable, messages );
	}

	return( code );
}
