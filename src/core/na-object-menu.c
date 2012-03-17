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

#include <glib/gi18n.h>
#include <string.h>

#include <api/na-iio-provider.h>
#include <api/na-ifactory-object.h>
#include <api/na-object-api.h>

#include "na-factory-provider.h"
#include "na-factory-object.h"

/* private class data
 */
struct NAObjectMenuClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAObjectMenuPrivate {
	gboolean dispose_has_run;
};

/* i18n: default label for a new menu */
#define NEW_CAJA_MENU				N_( "New Caja menu" )

extern NADataGroup menu_data_groups [];			/* defined in na-item-menu-factory.c */

static NAObjectItemClass *st_parent_class = NULL;

static GType        register_type( void );
static void         class_init( NAObjectMenuClass *klass );
static void         instance_init( GTypeInstance *instance, gpointer klass );
static void         instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void         instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void         instance_dispose( GObject *object );
static void         instance_finalize( GObject *object );

static void         object_copy( NAObject *target, const NAObject *source, gboolean recursive );
static gboolean     object_is_valid( const NAObject *object );

static void         ifactory_object_iface_init( NAIFactoryObjectInterface *iface );
static guint        ifactory_object_get_version( const NAIFactoryObject *instance );
static NADataGroup *ifactory_object_get_groups( const NAIFactoryObject *instance );
static gboolean     ifactory_object_are_equal( const NAIFactoryObject *a, const NAIFactoryObject *b );
static gboolean     ifactory_object_is_valid( const NAIFactoryObject *object );
static void         ifactory_object_read_start( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
static void         ifactory_object_read_done( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
static guint        ifactory_object_write_start( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );
static guint        ifactory_object_write_done( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );

static gboolean     menu_is_valid( const NAObjectMenu *menu );
static gboolean     is_valid_label( const NAObjectMenu *menu );

GType
na_object_menu_get_type( void )
{
	static GType menu_type = 0;

	if( menu_type == 0 ){

		menu_type = register_type();
	}

	return( menu_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_object_menu_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAObjectMenuClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAObjectMenu ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo ifactory_object_iface_info = {
		( GInterfaceInitFunc ) ifactory_object_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( NA_OBJECT_ITEM_TYPE, "NAObjectMenu", &info, 0 );

	g_type_add_interface_static( type, NA_IFACTORY_OBJECT_TYPE, &ifactory_object_iface_info );

	return( type );
}

static void
class_init( NAObjectMenuClass *klass )
{
	static const gchar *thisfn = "na_object_menu_class_init";
	GObjectClass *object_class;
	NAObjectClass *naobject_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->set_property = instance_set_property;
	object_class->get_property = instance_get_property;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	naobject_class = NA_OBJECT_CLASS( klass );
	naobject_class->dump = NULL;
	naobject_class->copy = object_copy;
	naobject_class->are_equal = NULL;
	naobject_class->is_valid = object_is_valid;

	klass->private = g_new0( NAObjectMenuClassPrivate, 1 );

	na_factory_object_define_properties( object_class, menu_data_groups );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_object_menu_instance_init";
	NAObjectMenu *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( NA_IS_OBJECT_MENU( instance ));

	self = NA_OBJECT_MENU( instance );

	self->private = g_new0( NAObjectMenuPrivate, 1 );
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	g_return_if_fail( NA_IS_OBJECT_MENU( object ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NA_OBJECT_MENU( object )->private->dispose_has_run ){

		na_factory_object_get_as_value( NA_IFACTORY_OBJECT( object ), g_quark_to_string( property_id ), value );
	}
}

static void
instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec )
{
	g_return_if_fail( NA_IS_OBJECT_MENU( object ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NA_OBJECT_MENU( object )->private->dispose_has_run ){

		na_factory_object_set_from_value( NA_IFACTORY_OBJECT( object ), g_quark_to_string( property_id ), value );
	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_object_menu_instance_dispose";
	NAObjectMenu *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_if_fail( NA_IS_OBJECT_MENU( object ));

	self = NA_OBJECT_MENU( object );

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
	static const gchar *thisfn = "na_object_menu_instance_finalize";
	NAObjectMenu *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_if_fail( NA_IS_OBJECT_MENU( object ));

	self = NA_OBJECT_MENU( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
object_copy( NAObject *target, const NAObject *source, gboolean recursive )
{
	g_return_if_fail( NA_IS_OBJECT_MENU( target ));
	g_return_if_fail( NA_IS_OBJECT_MENU( source ));

	if( !NA_OBJECT_MENU( target )->private->dispose_has_run &&
		!NA_OBJECT_MENU( source )->private->dispose_has_run ){

		na_factory_object_copy( NA_IFACTORY_OBJECT( target ), NA_IFACTORY_OBJECT( source ));
	}
}

static gboolean
object_is_valid( const NAObject *object )
{
	g_return_val_if_fail( NA_IS_OBJECT_MENU( object ), FALSE );

	return( menu_is_valid( NA_OBJECT_MENU( object )));
}

static void
ifactory_object_iface_init( NAIFactoryObjectInterface *iface )
{
	static const gchar *thisfn = "na_object_menu_ifactory_object_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = ifactory_object_get_version;
	iface->get_groups = ifactory_object_get_groups;
	iface->copy = NULL;
	iface->are_equal = ifactory_object_are_equal;
	iface->is_valid = ifactory_object_is_valid;
	iface->read_start = ifactory_object_read_start;
	iface->read_done = ifactory_object_read_done;
	iface->write_start = ifactory_object_write_start;
	iface->write_done = ifactory_object_write_done;
}

static guint
ifactory_object_get_version( const NAIFactoryObject *instance )
{
	return( 1 );
}

static NADataGroup *
ifactory_object_get_groups( const NAIFactoryObject *instance )
{
	return( menu_data_groups );
}

static gboolean
ifactory_object_are_equal( const NAIFactoryObject *a, const NAIFactoryObject *b )
{
	return( na_object_item_are_equal( NA_OBJECT_ITEM( a ), NA_OBJECT_ITEM( b )));
}

static gboolean
ifactory_object_is_valid( const NAIFactoryObject *object )
{
	g_return_val_if_fail( NA_IS_OBJECT_MENU( object ), FALSE );

	return( menu_is_valid( NA_OBJECT_MENU( object )));
}

static void
ifactory_object_read_start( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages )
{
}

static void
ifactory_object_read_done( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages )
{
	na_factory_object_set_defaults( instance );
}

static guint
ifactory_object_write_start( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	na_object_item_factory_write_start( NA_OBJECT_ITEM( instance ));

	return( NA_IIO_PROVIDER_CODE_OK );
}

static guint
ifactory_object_write_done( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	return( NA_IIO_PROVIDER_CODE_OK );
}

static gboolean
menu_is_valid( const NAObjectMenu *menu )
{
	gboolean is_valid;
	gint valid_subitems;
	GList *subitems, *ip;

	is_valid = FALSE;

	if( !menu->private->dispose_has_run ){

		is_valid = TRUE;

		if( is_valid ){
			is_valid = is_valid_label( menu );
		}

		if( is_valid ){
			valid_subitems = 0;
			subitems = na_object_get_items( menu );
			for( ip = subitems ; ip && !valid_subitems ; ip = ip->next ){
				if( na_object_is_valid( ip->data )){
					valid_subitems += 1;
				}
			}
			is_valid = ( valid_subitems > 0 );
			if( !is_valid ){
				na_object_debug_invalid( menu, "no valid subitem" );
			}
		}
	}

	return( is_valid );
}

static gboolean
is_valid_label( const NAObjectMenu *menu )
{
	gboolean is_valid;
	gchar *label;

	label = na_object_get_label( menu );
	is_valid = ( label && g_utf8_strlen( label, -1 ) > 0 );
	g_free( label );

	if( !is_valid ){
		na_object_debug_invalid( menu, "label" );
	}

	return( is_valid );
}

/**
 * na_object_menu_new:
 *
 * Allocates a new #NAObjectMenu object.
 *
 * Returns: the newly allocated #NAObjectMenu object.
 */
NAObjectMenu *
na_object_menu_new( void )
{
	NAObjectMenu *menu;

	menu = g_object_new( NA_OBJECT_MENU_TYPE, NULL );

	return( menu );
}

/**
 * na_object_menu_new_with_defaults:
 *
 * Allocates a new #NAObjectMenu object, and setup default values.
 *
 * Returns: the newly allocated #NAObjectMenu object.
 */
NAObjectMenu *
na_object_menu_new_with_defaults( void )
{
	NAObjectMenu *menu = na_object_menu_new();
	na_object_set_new_id( menu, NULL );
	na_object_set_label( menu, NEW_CAJA_MENU );
	na_factory_object_set_defaults( NA_IFACTORY_OBJECT( menu ));

	return( menu );
}
