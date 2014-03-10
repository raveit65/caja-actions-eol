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

/* We want test here what is the exact behavior of virtual functions in
 * derived classes, whether or not base class has implemented them or
 * not.
 *
 * We define three classes, and some virtual functions :
 * class 'first': fn_a, fn_b, fn_c
 * class 'second', derived from 'first', implements fn_a, fn_b
 * class 'three', derived from 'second': implements fn_a, fn_c
 *
 * Public entry points are defined in class 'first': we check that calling
 * public entry points with an object of each class actually calls the
 * relevant virtual function.
 *
 * Also we check if calling the parent class is possible even if the
 * parent class has not explicitely defined the virtual function.
 *
 * NOTE: this only works if we do _not_ set the virtual method to NULL
 * in intermediate classes.
 */

#include <glib-object.h>
#include <glib.h>

#define PWI_TYPE_FIRST					( pwi_first_get_type())
#define PWI_FIRST( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, PWI_TYPE_FIRST, PwiFirst ))
#define PWI_FIRST_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, PWI_TYPE_FIRST, PwiFirstClass ))
#define PWI_IS_FIRST( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, PWI_TYPE_FIRST ))
#define PWI_IS_FIRST_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), PWI_TYPE_FIRST ))
#define PWI_FIRST_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), PWI_TYPE_FIRST, PwiFirstClass ))

typedef struct {
	void *empty;						/* so that gcc -pedantic is happy */
}
	PwiFirstPrivate;

typedef struct {
	GObject          parent;
	PwiFirstPrivate *private;
}
	PwiFirst;

typedef struct {
	void *empty;						/* so that gcc -pedantic is happy */
}
	PwiFirstClassPrivate;

typedef struct {
	GObjectClass          parent;
	PwiFirstClassPrivate *private;

	/* virtual functions */
	void ( *fn_a )( PwiFirst *instance );
	void ( *fn_b )( PwiFirst *instance );
	void ( *fn_c )( PwiFirst *instance );
}
	PwiFirstClass;

static GObjectClass *pwi_first_parent_class = NULL;
static GType pwi_first_get_type( void );
static void  pwi_first_class_init( PwiFirstClass *klass );
static void  pwi_first_init( PwiFirst *instance, gpointer klass );

static GType
pwi_first_register_type( void )
{
	static const gchar *thisfn = "first_register_type";

	static GTypeInfo info = {
		sizeof( PwiFirstClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) pwi_first_class_init,
		NULL,
		NULL,
		sizeof( PwiFirst ),
		0,
		( GInstanceInitFunc ) pwi_first_init
	};

	g_debug( "%s", thisfn );
	return( g_type_register_static( G_TYPE_OBJECT, "PwiFirst", &info, 0 ));
}

static GType
pwi_first_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = pwi_first_register_type();
	}

	return( type );
}

static void
do_first_fn_a( PwiFirst *instance )
{
	g_debug( "do_first_fn_a: instance=%p", ( void * ) instance );
}

static void
do_first_fn_b( PwiFirst *instance )
{
	g_debug( "do_first_fn_b: instance=%p", ( void * ) instance );
}

static void
do_first_fn_c( PwiFirst *instance )
{
	g_debug( "do_first_fn_c: instance=%p", ( void * ) instance );
}

static void
pwi_first_class_init( PwiFirstClass *klass )
{
	static const gchar *thisfn = "pwi_first_class_init";

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	pwi_first_parent_class = g_type_class_peek_parent( klass );

	klass->private = g_new0( PwiFirstClassPrivate, 1 );

	klass->fn_a = do_first_fn_a;
	klass->fn_b = do_first_fn_b;
	klass->fn_c = do_first_fn_c;
}

static void
pwi_first_init( PwiFirst *self, gpointer klass )
{
	static const gchar *thisfn = "pwi_first_init";

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) self, ( void * ) klass );

	self->private = g_new0( PwiFirstPrivate, 1 );
}

void pwi_first_fn_a( PwiFirst *instance );
void pwi_first_fn_b( PwiFirst *instance );
void pwi_first_fn_c( PwiFirst *instance );

void
pwi_first_fn_a( PwiFirst *instance )
{
	g_debug( "pwi_first_fn_a: instance=%p", ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));

	if( PWI_FIRST_GET_CLASS( instance )->fn_a ){
		PWI_FIRST_GET_CLASS( instance )->fn_a( instance );
	} else {
		g_debug( "default to invoke do_first_fn_a()" );
		do_first_fn_a( instance );
	}
}

void
pwi_first_fn_b( PwiFirst *instance )
{
	g_debug( "pwi_first_fn_b: instance=%p", ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));

	if( PWI_FIRST_GET_CLASS( instance )->fn_b ){
		PWI_FIRST_GET_CLASS( instance )->fn_b( instance );
	} else {
		g_debug( "default to invoke do_first_fn_b()" );
		do_first_fn_b( instance );
	}
}

void
pwi_first_fn_c( PwiFirst *instance )
{
	g_debug( "pwi_first_fn_c: instance=%p", ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));

	if( PWI_FIRST_GET_CLASS( instance )->fn_c ){
		PWI_FIRST_GET_CLASS( instance )->fn_c( instance );
	} else {
		g_debug( "default to invoke do_first_fn_c()" );
		do_first_fn_c( instance );
	}
}

#define PWI_TYPE_SECOND					( pwi_second_get_type())
#define PWI_SECOND( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, PWI_TYPE_SECOND, PwiSecond ))
#define PWI_SECOND_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, PWI_TYPE_SECOND, PwiSecondClass ))
#define PWI_IS_SECOND( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, PWI_TYPE_SECOND ))
#define PWI_IS_SECOND_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), PWI_TYPE_SECOND ))
#define PWI_SECOND_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), PWI_TYPE_SECOND, PwiSecondClass ))

typedef struct {
	void *empty;						/* so that gcc -pedantic is happy */
}
	PwiSecondPrivate;

typedef struct {
	PwiFirst          parent;
	PwiSecondPrivate *private;
}
	PwiSecond;

typedef struct {
	void *empty;						/* so that gcc -pedantic is happy */
}
	PwiSecondClassPrivate;

typedef struct {
	PwiFirstClass          parent;
	PwiSecondClassPrivate *private;
}
	PwiSecondClass;

static GObjectClass *pwi_second_parent_class = NULL;
static GType pwi_second_get_type( void );
static void  pwi_second_class_init( PwiSecondClass *klass );
static void  pwi_second_init( PwiSecond *instance, gpointer klass );

static GType
pwi_second_register_type( void )
{
	static const gchar *thisfn = "second_register_type";

	static GTypeInfo info = {
		sizeof( PwiSecondClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) pwi_second_class_init,
		NULL,
		NULL,
		sizeof( PwiSecond ),
		0,
		( GInstanceInitFunc ) pwi_second_init
	};

	g_debug( "%s", thisfn );
	return( g_type_register_static( PWI_TYPE_FIRST, "PwiSecond", &info, 0 ));
}

static GType
pwi_second_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = pwi_second_register_type();
	}

	return( type );
}

static void
do_second_fn_a( PwiFirst *instance )
{
	g_debug( "do_second_fn_a: instance=%p", ( void * ) instance );
	if( PWI_FIRST_CLASS( pwi_second_parent_class )->fn_a ){
		PWI_FIRST_CLASS( pwi_second_parent_class )->fn_a( instance );
	}
}

static void
do_second_fn_b( PwiFirst *instance )
{
	g_debug( "do_second_fn_b: instance=%p", ( void * ) instance );
	if( PWI_FIRST_CLASS( pwi_second_parent_class )->fn_b ){
		PWI_FIRST_CLASS( pwi_second_parent_class )->fn_b( instance );
	}
}

static void
pwi_second_class_init( PwiSecondClass *klass )
{
	static const gchar *thisfn = "pwi_second_class_init";
	PwiFirstClass *parent_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	pwi_second_parent_class = g_type_class_peek_parent( klass );

	klass->private = g_new0( PwiSecondClassPrivate, 1 );

	parent_class = PWI_FIRST_CLASS( klass );
	parent_class->fn_a = do_second_fn_a;
	parent_class->fn_b = do_second_fn_b;
	/*parent_class->fn_c = NULL;*/
}

static void
pwi_second_init( PwiSecond *self, gpointer klass )
{
	static const gchar *thisfn = "pwi_second_init";

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) self, ( void * ) klass );

	self->private = g_new0( PwiSecondPrivate, 1 );
}

#define PWI_TYPE_THREE					( pwi_three_get_type())
#define PWI_THREE( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, PWI_TYPE_THREE, PwiThree ))
#define PWI_THREE_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, PWI_TYPE_THREE, PwiThreeClass ))
#define PWI_IS_THREE( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, PWI_TYPE_THREE ))
#define PWI_IS_THREE_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), PWI_TYPE_THREE ))
#define PWI_THREE_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), PWI_TYPE_THREE, PwiThreeClass ))

typedef struct {
	void *empty;						/* so that gcc -pedantic is happy */
}
	PwiThreePrivate;

typedef struct {
	PwiSecond        parent;
	PwiThreePrivate *private;
}
	PwiThree;

typedef struct {
	void *empty;						/* so that gcc -pedantic is happy */
}
	PwiThreeClassPrivate;

typedef struct {
	PwiSecondClass        parent;
	PwiThreeClassPrivate *private;
}
	PwiThreeClass;

static GObjectClass *pwi_three_parent_class = NULL;
static GType pwi_three_get_type( void );
static void  pwi_three_class_init( PwiThreeClass *klass );
static void  pwi_three_init( PwiThree *instance, gpointer klass );

static GType
pwi_three_register_type( void )
{
	static const gchar *thisfn = "three_register_type";

	static GTypeInfo info = {
		sizeof( PwiThreeClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) pwi_three_class_init,
		NULL,
		NULL,
		sizeof( PwiThree ),
		0,
		( GInstanceInitFunc ) pwi_three_init
	};

	g_debug( "%s", thisfn );
	return( g_type_register_static( PWI_TYPE_SECOND, "PwiThree", &info, 0 ));
}

static GType
pwi_three_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = pwi_three_register_type();
	}

	return( type );
}

static void
do_three_fn_a( PwiFirst *instance )
{
	g_debug( "do_three_fn_a: instance=%p", ( void * ) instance );
	if( PWI_FIRST_CLASS( pwi_three_parent_class )->fn_a ){
		PWI_FIRST_CLASS( pwi_three_parent_class )->fn_a( instance );
	}
}

static void
do_three_fn_c( PwiFirst *instance )
{
	g_debug( "do_three_fn_c: instance=%p", ( void * ) instance );
	if( PWI_FIRST_CLASS( pwi_three_parent_class )->fn_c ){
		PWI_FIRST_CLASS( pwi_three_parent_class )->fn_c( instance );
	}
}

static void
pwi_three_class_init( PwiThreeClass *klass )
{
	static const gchar *thisfn = "pwi_three_class_init";
	PwiFirstClass *parent_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	pwi_three_parent_class = g_type_class_peek_parent( klass );

	klass->private = g_new0( PwiThreeClassPrivate, 1 );

	parent_class = PWI_FIRST_CLASS( klass );
	parent_class->fn_a = do_three_fn_a;
	/*parent_class->fn_b = NULL;*/
	parent_class->fn_c = do_three_fn_c;
}

static void
pwi_three_init( PwiThree *self, gpointer klass )
{
	static const gchar *thisfn = "pwi_three_init";

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) self, ( void * ) klass );

	self->private = g_new0( PwiThreePrivate, 1 );
}

int
main( int argc, char **argv )
{
	PwiFirst *a;
	PwiSecond *b;
	PwiThree *c;

#if !GLIB_CHECK_VERSION( 2,36, 0 )
	g_type_init();
#endif

	a = g_object_new( PWI_TYPE_FIRST, NULL );
	b = g_object_new( PWI_TYPE_SECOND, NULL );
	c = g_object_new( PWI_TYPE_THREE, NULL );

	g_debug( "%s", "" );

	g_debug( "expected pwi_first_fn_a, do_first_fn_a" );
	pwi_first_fn_a( PWI_FIRST( a ));
	g_debug( "expected pwi_first_fn_a, do_second_fn_a, do_first_fn_a" );
	pwi_first_fn_a( PWI_FIRST( b ));
	g_debug( "expected pwi_first_fn_a, do_three_fn_a, do_second_fn_a, do_first_fn_a" );
	pwi_first_fn_a( PWI_FIRST( c ));

	g_debug( "%s", "" );

	g_debug( "expected pwi_first_fn_b, do_first_fn_b" );
	pwi_first_fn_b( PWI_FIRST( a ));
	g_debug( "expected pwi_first_fn_b, do_second_fn_b, do_first_fn_b" );
	pwi_first_fn_b( PWI_FIRST( b ));
	g_debug( "expected pwi_first_fn_b, do_second_fn_b, do_first_fn_b" );
	/* NOT OK
	 * result is pwi_first_fn_b, default to do_first_fn_b */
	pwi_first_fn_b( PWI_FIRST( c ));

	g_debug( "%s", "" );

	g_debug( "expected pwi_first_fn_c, do_first_fn_c" );
	pwi_first_fn_c( PWI_FIRST( a ));
	g_debug( "expected pwi_first_fn_c, do_first_fn_c" );
	pwi_first_fn_c( PWI_FIRST( b ));
	g_debug( "expected pwi_first_fn_c, do_three_fn_c, do_first_fn_c" );
	/* NOT OK
	 * result is pwi_first_fn_c, do_three_fn_c */
	pwi_first_fn_c( PWI_FIRST( c ));

	return( 0 );
}
