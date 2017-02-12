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

#include <string.h>

#include "test-iface-derived.h"
#include "test-iface-iface.h"

/* private class data
 */
struct TestDerivedClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct TestDerivedPrivate {
	gboolean  dispose_has_run;
};

static TestBaseClass *st_parent_class = NULL;

static GType register_type( void );
static void  class_init( TestDerivedClass *klass );
static void  iface_iface_init( TestDerivedClass *klass );
static void  instance_init( GTypeInstance *instance, gpointer klass );
static void  instance_dispose( GObject *object );
static void  instance_finalize( GObject *object );

static void  iface_fna( TestIFace *object );
static void  iface_fnb( TestIFace *object );

GType
test_derived_get_type( void )
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
	static const gchar *thisfn = "test_iface_derived_register_type";

	static GTypeInfo info = {
		sizeof( TestDerivedClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( TestDerived ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	return( g_type_register_static( TEST_BASE_TYPE, "TestDerived", &info, 0 ));
}

static void
class_init( TestDerivedClass *klass )
{
	static const gchar *thisfn = "test_derived_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( TestDerivedClassPrivate, 1 );

	/* there is no error message but this has no effect
	 * event in a TestDerived object, these are TestBase functions which are called
	 */
	if( 0 ){
		(( TestIFaceInterface * ) klass )->fna = iface_fna;
		(( TestIFaceInterface * ) klass )->fnb = iface_fnb;
	}

	/* idem
	 */
	if( 0 ){
		(( TestIFaceInterface * ) st_parent_class )->fna = iface_fna;
		(( TestIFaceInterface * ) st_parent_class )->fnb = iface_fnb;
	}

	if( 1 ){
		iface_iface_init( klass );
	}
}

static void
iface_iface_init( TestDerivedClass *klass )
{
	static const gchar *thisfn = "test_iface_derived_iface_iface_init";

	/* this has the effect of totally overriding the interface api
	 * and both TestBase and TestDerived objects will only call TestDerived functions
	 */
	/*GTypeInterface *iface;
	iface = g_type_interface_peek( klass, TEST_IFACE_TYPE );
	g_debug( "%s: iface=%s at %p", thisfn, g_type_name( G_TYPE_FROM_INTERFACE( iface )), ( void * ) iface );
	(( TestIFaceInterface * ) iface )->fna = iface_fna;
	(( TestIFaceInterface * ) iface )->fnb = iface_fnb;*/

	/* this segfault
	 */
	/*
	GTypeInterface *iface;
	iface = g_type_interface_peek( klass, TEST_IFACE_TYPE );
	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );
	iface = g_type_interface_peek_parent( iface );
	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );
	g_debug( "%s: iface=%s at %p", thisfn, g_type_name( G_TYPE_FROM_INTERFACE( iface )), ( void * ) iface );
	(( TestIFaceInterface * ) iface )->fna = iface_fna;
	(( TestIFaceInterface * ) iface )->fnb = iface_fnb;*/

	/* this has the effect of totally overriding the interface api
	 * and both TestBase and TestDerived objects will only call TestDerived functions
	 */
	if( 0 ){
		GTypeInterface *iface;
		iface = g_type_interface_peek( st_parent_class, TEST_IFACE_TYPE );
		g_debug( "%s: iface=%s at %p", thisfn, g_type_name( G_TYPE_FROM_INTERFACE( iface )), ( void * ) iface );
		(( TestIFaceInterface * ) iface )->fna = iface_fna;
		(( TestIFaceInterface * ) iface )->fnb = iface_fnb;
	}
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	/*static const gchar *thisfn = "test_derived_instance_init";*/
	TestDerived *self;

	/*g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );*/
	g_assert( TEST_IS_DERIVED( instance ));
	self = TEST_DERIVED( instance );

	self->private = g_new0( TestDerivedPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	/*static const gchar *thisfn = "test_derived_instance_dispose";*/
	TestDerived *self;

	/*g_debug( "%s: object=%p", thisfn, ( void * ) object );*/
	g_assert( TEST_IS_DERIVED( object ));
	self = TEST_DERIVED( object );

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
	/*static const gchar *thisfn = "test_derived_instance_finalize";*/
	TestDerived *self;

	/*g_debug( "%s: object=%p", thisfn, (void * ) object );*/
	g_assert( TEST_IS_DERIVED( object ));
	self = ( TestDerived * ) object;

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

TestDerived *
test_derived_new( void )
{
	TestDerived *object = g_object_new( TEST_DERIVED_TYPE, NULL );

	return( object );
}

static void
iface_fna( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_derived_iface_fna";
	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
}

static void
iface_fnb( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_derived_iface_fnb";
	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
}
