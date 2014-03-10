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

/* A class with implements an interface
 * This interface itself requiring the class...
 */

#include <glib.h>
#include <glib-object.h>

/* ********************************************************************
 * Declaring the interface
 * ********************************************************************/
#define TEST_IFACE_TYPE						( test_iface_get_type())
#define TEST_IFACE( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, TEST_IFACE_TYPE, TestIFace ))
#define TEST_IS_IFACE( object )				( G_TYPE_CHECK_INSTANCE_TYPE( object, TEST_IFACE_TYPE ))
#define TEST_IFACE_GET_INTERFACE( instance )( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), TEST_IFACE_TYPE, TestIFaceInterface ))

typedef struct TestIFace TestIFace;

typedef struct TestIFaceInterfacePrivate TestIFaceInterfacePrivate;

typedef struct {
	GTypeInterface             parent;
	TestIFaceInterfacePrivate *private;
}
	TestIFaceInterface;

GType test_iface_get_type( void );

/* ********************************************************************
 * Declaring the class
 * ********************************************************************/
#define TEST_BASE_TYPE					( test_base_get_type())
#define TEST_BASE( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, TEST_BASE_TYPE, TestBase ))
#define TEST_BASE_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, TEST_BASE_TYPE, TestBaseClass ))
#define TEST_IS_BASE( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, TEST_BASE_TYPE ))
#define TEST_IS_BASE_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), TEST_BASE_TYPE ))
#define TEST_BASE_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), TEST_BASE_TYPE, TestBaseClass ))

typedef struct TestBasePrivate TestBasePrivate;

typedef struct {
	GObject          parent;
	TestBasePrivate *private;
}
	TestBase;

typedef struct TestBaseClassPrivate TestBaseClassPrivate;

typedef struct {
	GObjectClass          parent;
	TestBaseClassPrivate *private;
}
	TestBaseClass;

GType     test_base_get_type( void );

static TestBase *test_base_new( void );

/* ********************************************************************
 * Implementing the interface
 * ********************************************************************/

/* private interface data
 */
struct TestIFaceInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static guint st_initializations = 0;

static GType iface_register_type( void );
static void  interface_base_init( TestIFaceInterface *klass );
static void  interface_base_finalize( TestIFaceInterface *klass );

GType
test_iface_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = iface_register_type();
	}

	return( iface_type );
}

static GType
iface_register_type( void )
{
	static const gchar *thisfn = "test_iface_iface_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( TestIFaceInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "TestIFace", &info, 0 );

	g_type_interface_add_prerequisite( type, TEST_BASE_TYPE );

	return( type );
}

static void
interface_base_init( TestIFaceInterface *klass )
{
	static const gchar *thisfn = "test_iface_iface_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( TestIFaceInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( TestIFaceInterface *klass )
{
	static const gchar *thisfn = "test_iface_iface_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/* ********************************************************************
 * Implementing the class
 * ********************************************************************/

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

static GType base_register_type( void );
static void  class_init( TestBaseClass *klass );
static void  instance_init( GTypeInstance *instance, gpointer klass );
static void  instance_dispose( GObject *object );
static void  instance_finalize( GObject *object );

static void  iface_iface_init( TestIFaceInterface *iface, void *user_data );

GType
test_base_get_type( void )
{
	static GType object_type = 0;

	static const GInterfaceInfo iface_iface_info = {
		( GInterfaceInitFunc ) iface_iface_init,
		NULL,
		NULL
	};

	if( !object_type ){
		object_type = base_register_type();
		g_type_add_interface_static( object_type, TEST_IFACE_TYPE, &iface_iface_info );
	}

	return( object_type );
}

static GType
base_register_type( void )
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

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "TestBase", &info, 0 );

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

	g_return_if_fail( TEST_IS_BASE( instance ));

	self = TEST_BASE( instance );

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );

	self->private = g_new0( TestBasePrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "test_iface_base_instance_dispose";
	TestBase *self;

	g_return_if_fail( TEST_IS_BASE( object ));

	self = TEST_BASE( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p", thisfn, ( void * ) object );

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
	static const gchar *thisfn = "test_iface_base_instance_finalize";
	TestBase *self;

	g_return_if_fail( TEST_IS_BASE( object ));

	self = ( TestBase * ) object;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static TestBase *
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
}

/* ********************************************************************
 * main
 * ********************************************************************/

int
main( int argc, char **argv )
{
	TestBase *base;

#if !GLIB_CHECK_VERSION( 2,36, 0 )
	g_type_init();
#endif

	g_debug( "allocating TestBase -------------------------------------" );
	base = test_base_new();

	g_debug( "unreffing TestBase ------------------------------------" );
	g_object_unref( base );

	g_debug( "end -----------------------------------------------------" );

	return( 0 );
}
