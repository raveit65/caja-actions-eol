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

#include <string.h>

#include <api/na-ifactory-object.h>

#include "na-factory-object.h"

/* private interface data
 */
struct _NAIFactoryObjectInterfacePrivate {
	void *empty;					/* so that gcc -pedantic is happy */
};

static guint st_initializations = 0;	/* interface initialization count */

static GType register_type( void );
static void  interface_base_init( NAIFactoryObjectInterface *klass );
static void  interface_base_finalize( NAIFactoryObjectInterface *klass );

static guint ifactory_object_get_version( const NAIFactoryObject *instance );

/*
 * Registers the GType of this interface.
 */
GType
na_ifactory_object_get_type( void )
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
	static const gchar *thisfn = "na_ifactory_object_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIFactoryObjectInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIFactoryObject", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIFactoryObjectInterface *klass )
{
	static const gchar *thisfn = "na_ifactory_object_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p (%s)", thisfn, ( void * ) klass, G_OBJECT_CLASS_NAME( klass ));

		klass->private = g_new0( NAIFactoryObjectInterfacePrivate, 1 );

		klass->get_version = ifactory_object_get_version;
		klass->get_groups = NULL;
		klass->copy = NULL;
		klass->are_equal = NULL;
		klass->is_valid = NULL;
		klass->read_start = NULL;
		klass->read_done = NULL;
		klass->write_start = NULL;
		klass->write_done = NULL;
	}

	st_initializations += 1;
}

static void
interface_base_finalize( NAIFactoryObjectInterface *klass )
{
	static const gchar *thisfn = "na_ifactory_object_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

static guint
ifactory_object_get_version( const NAIFactoryObject *instance )
{
	return( 1 );
}

/**
 * na_ifactory_object_get_data_boxed:
 * @object: a #NAIFactoryObject object.
 * @name: the name of the elementary data we are searching for.
 *
 * The returned #NADataBoxed is owned by #NAIFactoryObject @object, and
 * should not be released by the caller.
 *
 * Returns: The #NADataBoxed object which contains the specified data,
 * or %NULL.
 *
 * Since: 2.30
 */
NADataBoxed *
na_ifactory_object_get_data_boxed( const NAIFactoryObject *object, const gchar *name )
{
	GList *list, *ip;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	list = g_object_get_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA );
	/*g_debug( "list=%p (count=%u)", ( void * ) list, g_list_length( list ));*/

	for( ip = list ; ip ; ip = ip->next ){
		NADataBoxed *boxed = NA_DATA_BOXED( ip->data );
		const NADataDef *def = na_data_boxed_get_data_def( boxed );

		if( !strcmp( def->name, name )){
			return( boxed );
		}
	}

	return( NULL );
}

/**
 * na_ifactory_object_get_data_groups:
 * @object: a #NAIFactoryObject object.
 *
 * The returned #NADataGroup is owned by the #NAIFactoryObject @object,
 * and should not be released by the caller.
 *
 * Returns: The #NADataGroup groups definition, or %NULL.
 *
 * Since: 2.30
 */
NADataGroup *
na_ifactory_object_get_data_groups( const NAIFactoryObject *object )
{
	NADataGroup *groups;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	groups = NULL;

	if( NA_IFACTORY_OBJECT_GET_INTERFACE( object )->get_groups ){
		groups = NA_IFACTORY_OBJECT_GET_INTERFACE( object )->get_groups( object );
	}

	return( groups );
}

/**
 * na_ifactory_object_get_as_void:
 * @object: this #NAIFactoryObject instance.
 * @name: the elementary data whose value is to be got.
 *
 * If the type of the value is %NA_DATA_TYPE_STRING, %NA_DATA_TYPE_LOCALE_STRING,
 * or %NA_DATA_TYPE_STRING_LIST, then the returned value is a newly allocated
 * one and should be g_free() (resp. na_core_utils_slist_free()) by the
 * caller.
 *
 * Returns: the searched value.
 *
 * Since: 2.30
 */
void *
na_ifactory_object_get_as_void( const NAIFactoryObject *object, const gchar *name )
{
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	return( na_factory_object_get_as_void( object, name ));
}

/**
 * na_ifactory_object_set_from_void:
 * @object: this #NAIFactoryObject instance.
 * @name: the name of the elementary data whose value is to be set.
 * @data: the value to set.
 *
 * Set the elementary data with the given value.
 *
 * Since: 2.30
 */
void
na_ifactory_object_set_from_void( NAIFactoryObject *object, const gchar *name, const void *data )
{
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	na_factory_object_set_from_void( object, name, data );
}
