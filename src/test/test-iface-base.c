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

#include "test-iface-base.h"
#include "test-iface-iface.h"

/* private class data
 */
struct TestBaseClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct TestBasePrivate {
	gboolean dispose_has_run;
};

static GObjectClass *st_parent_class = NULL;

static GType register_type( void );
static void  class_init( TestBaseClass *klass );
static void  instance_init( GTypeInstance *instance, gpointer klass );
static void  instance_dispose( GObject *object );
static void  instance_finalize( GObject *object );

static void  iface_iface_init( TestIFaceInterface *iface, void *user_data );
static void  iface_fna( TestIFace *object );
static void  iface_fnb( TestIFace *object );

GType
test_base_get_type( void )
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
	static const gchar *thisfn = "test_iface_base_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( TestBaseClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( TestBase ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo iface_iface_info = {
		( GInterfaceInitFunc ) iface_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "TestBase", &info, 0 );

	g_type_add_interface_static( type, TEST_IFACE_TYPE, &iface_iface_info );

	return( type );
}

static void
class_init( TestBaseClass *klass )
{
	static const gchar *thisfn = "test_iface_base_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( TestBaseClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "test_iface_base_instance_init";
	TestBase *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_assert( TEST_IS_BASE( instance ));
	self = TEST_BASE( instance );

	self->private = g_new0( TestBasePrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "test_iface_base_instance_dispose";
	TestBase *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_assert( TEST_IS_BASE( object ));
	self = TEST_BASE( object );

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
	TestBase *self;

	g_assert( TEST_IS_BASE( object ));
	self = ( TestBase * ) object;

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

TestBase *
test_base_new( void )
{
	TestBase *object = g_object_new( TEST_BASE_TYPE, NULL );

	return( object );
}

static void
iface_iface_init( TestIFaceInterface *iface, void *user_data )
{
	static const gchar *thisfn = "test_iface_base_iface_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->fna = iface_fna;
	iface->fnb = iface_fnb;
}

static void
iface_fna( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_base_iface_fna";
	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
}

static void
iface_fnb( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_base_iface_fnb";
	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
}
