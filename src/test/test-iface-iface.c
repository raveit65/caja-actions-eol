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

#include "test-iface-iface.h"
#include "test-iface-base.h"

/* private interface data
 */
struct TestIFaceInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static GType register_type( void );
static void  interface_base_init( TestIFaceInterface *klass );
static void  interface_base_finalize( TestIFaceInterface *klass );

static void  v_fna( TestIFace *object );

static void  do_fna( TestIFace *object );

GType
test_iface_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = register_type();
	}

	return( iface_type );
}

static GType
register_type( void )
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

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( TestIFaceInterface *klass )
{
	static const gchar *thisfn = "test_iface_iface_interface_base_init";
	static gboolean initialized = FALSE;

	g_debug( "%s: klass=%p, initialized=%s",
			thisfn, ( void * ) klass, initialized ? "True":"False" );

	if( !initialized ){

		klass->private = g_new0( TestIFaceInterfacePrivate, 1 );

		initialized = TRUE;
	}
}

static void
interface_base_finalize( TestIFaceInterface *klass )
{
	static const gchar *thisfn = "test_iface_iface_interface_base_finalize";
	static gboolean finalized = FALSE ;

	g_debug( "%s: klass=%p, finalized=%s",
			thisfn, ( void * ) klass, finalized ? "True":"False" );

	if( !finalized ){

		finalized = TRUE;

		g_free( klass->private );
	}
}

/*
 * only call the implementation of the most-derived class (if any)
 * if the most-derived class has not implemented the function,
 * then fallback to local default
 */
void
test_iface_fna( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_iface_fna";
	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
	v_fna( object );
}

static void
v_fna( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_iface_v_fna";
	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
	if( TEST_IFACE_GET_INTERFACE( object )->fna ){
		TEST_IFACE_GET_INTERFACE( object )->fna( object );
	} else {
		do_fna( object );
	}
}

static void
do_fna( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_iface_do_fna";
	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
}

/*
 * successively call the implementation (if any) of each derived class
 * in the hierarchy order, from topmost base class to most-derived class
 * if any of class in the hierarchy has not implemented the function, the
 * do nothing and go to next class
 */
void
test_iface_fnb( TestIFace *object )
{
	static const gchar *thisfn = "test_iface_iface_fnb";
	GSList *hierarchy;
	GObjectClass *class;
	GType type;
	GType base_type;
	GSList *ic;
	GTypeInterface *iface;

	g_debug( "%s: %s at %p", thisfn, G_OBJECT_TYPE_NAME( object ), ( void * ) object );
	g_debug( "%s: g_type_from_instance=%d", thisfn, ( gint ) G_TYPE_FROM_INSTANCE( object ));
	g_debug( "%s: g_type_from_interface=%d", thisfn, ( gint ) G_TYPE_FROM_INTERFACE( object ));

	hierarchy = NULL;
	base_type = TEST_BASE_TYPE;
	type = G_OBJECT_TYPE( object );
	g_debug( "%s: type=%d %s", thisfn, ( gint ) type, G_OBJECT_TYPE_NAME( object ));
	while( TRUE ){
		/*hierarchy = g_slist_prepend( hierarchy, class );*/
		hierarchy = g_slist_prepend( hierarchy, GINT_TO_POINTER( type ));
		/*type = G_TYPE_FROM_CLASS( class );*/
		if( type == base_type ){
			break;
		}
		type = g_type_parent( type );
		if( !type ){
			g_debug( "%s: GOT ZERO TYPE", thisfn );
			break;
		}
	}

	for( ic = hierarchy ; ic ; ic = ic->next ){
		type = GPOINTER_TO_INT( ic->data );
		g_debug( "%s: iterating on %d type", thisfn, ( gint ) type );
		class = g_type_class_peek_static( type );
		g_debug( "%s: class is %s at %p", thisfn, G_OBJECT_CLASS_NAME( class ), ( void * ) class );
		iface = g_type_interface_peek( class, TEST_IFACE_TYPE );
		g_debug( "%s: iface at %p", thisfn, ( void * ) iface );
		if( (( TestIFaceInterface * ) iface )->fnb ){
			(( TestIFaceInterface * ) iface )->fnb( object );
		}
	}
}
