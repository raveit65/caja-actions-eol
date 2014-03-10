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
 * class A: fn1, fn2, fn3
 * class AB: implements fn1, fn2
 * class ABC: implements fn1, fn3
 *
 * Public entry points are defined in class A: we check that calling
 * public entry points with an object of each class actually calls the
 * relevant virtual function.
 *
 * Also we check that calling the parent class is possible even if the
 * parent class has not explicitely defined the virtual function.
 *
 * Same as test-virtuals.c, without the test for existance of function.
 */

#include <glib-object.h>
#include <glib.h>

#define PWI_FIRST_TYPE			( pwi_first_get_type())
#define PWI_FIRST( object )		( G_TYPE_CHECK_INSTANCE_CAST( object, PWI_FIRST_TYPE, PwiFirst ))
#define PWI_FIRST_CLASS( klass )	( G_TYPE_CHECK_CLASS_CAST( klass, PWI_FIRST_TYPE, PwiFirstClass ))
#define PWI_IS_FIRST( object )		( G_TYPE_CHECK_INSTANCE_TYPE( object, PWI_FIRST_TYPE ))
#define PWI_IS_FIRST_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), PWI_FIRST_TYPE ))
#define PWI_FIRST_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), PWI_FIRST_TYPE, PwiFirstClass ))

typedef struct PwiFirstPrivate PwiFirstPrivate;

typedef struct {
	GObject          parent;
	PwiFirstPrivate *private;
}
	PwiFirst;

typedef struct PwiFirstClassPrivate PwiFirstClassPrivate;

typedef struct {
	GObjectClass          parent;
	PwiFirstClassPrivate *private;

	/* virtual functions */
	void ( *fn_a )( PwiFirst *instance );
	void ( *fn_b )( PwiFirst *instance );
	void ( *fn_c )( PwiFirst *instance );
}
	PwiFirstClass;

GType pwi_first_get_type( void );

void pwi_first_fn_a( PwiFirst *instance );
void pwi_first_fn_b( PwiFirst *instance );
void pwi_first_fn_c( PwiFirst *instance );

#define PWI_FIRST_SECOND_TYPE			( pwi_first_second_get_type())
#define PWI_FIRST_SECOND( object )		( G_TYPE_CHECK_INSTANCE_CAST( object, PWI_FIRST_SECOND_TYPE, PwiFirstSecond ))
#define PWI_FIRST_SECOND_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, PWI_FIRST_SECOND_TYPE, PwiFirstSecondClass ))
#define PWI_IS_FIRST_SECOND( object )		( G_TYPE_CHECK_INSTANCE_TYPE( object, PWI_FIRST_SECOND_TYPE ))
#define PWI_IS_FIRST_SECOND_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), PWI_FIRST_SECOND_TYPE ))
#define PWI_FIRST_SECOND_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), PWI_FIRST_SECOND_TYPE, PwiFirstSecondClass ))

typedef struct PwiFirstSecondPrivate PwiFirstSecondPrivate;

typedef struct {
	PwiFirst               parent;
	PwiFirstSecondPrivate *private;
}
	PwiFirstSecond;

typedef struct PwiFirstSecondClassPrivate PwiFirstSecondClassPrivate;

typedef struct {
	PwiFirstClass               parent;
	PwiFirstSecondClassPrivate *private;
}
	PwiFirstSecondClass;

GType pwi_first_second_get_type( void );

#define PWI_FIRST_SECOND_THREE_TYPE			( pwi_first_second_three_get_type())
#define PWI_FIRST_SECOND_THREE( object )		( G_TYPE_CHECK_INSTANCE_CAST( object, PWI_FIRST_SECOND_THREE_TYPE, PwiFirstSecondThree ))
#define PWI_FIRST_SECOND_THREE_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, PWI_FIRST_SECOND_THREE_TYPE, PwiFirstSecondThreeClass ))
#define PWI_IS_FIRST_SECOND_THREE( object )		( G_TYPE_CHECK_INSTANCE_TYPE( object, PWI_FIRST_SECOND_THREE_TYPE ))
#define PWI_IS_FIRST_SECOND_THREE_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), PWI_FIRST_SECOND_THREE_TYPE ))
#define PWI_FIRST_SECOND_THREE_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), PWI_FIRST_SECOND_THREE_TYPE, PwiFirstSecondThreeClass ))

typedef struct PwiFirstSecondThreePrivate PwiFirstSecondThreePrivate;

typedef struct {
	PwiFirstSecond              parent;
	PwiFirstSecondThreePrivate *private;
}
	PwiFirstSecondThree;

typedef struct PwiFirstSecondThreeClassPrivate PwiFirstSecondThreeClassPrivate;

typedef struct {
	PwiFirstSecondClass              parent;
	PwiFirstSecondThreeClassPrivate *private;
}
	PwiFirstSecondThreeClass;

GType pwi_first_second_three_get_type( void );

struct PwiFirstClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

struct PwiFirstPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static GObjectClass *st_first_parent_class = NULL;

static GType first_register_type( void );
static void  first_class_init( PwiFirstClass *klass );
static void  first_instance_init( GTypeInstance *instance, gpointer klass );
static void  first_instance_dispose( GObject *application );
static void  first_instance_finalize( GObject *application );

static void  do_first_fn_a( PwiFirst *instance );
static void  do_first_fn_b( PwiFirst *instance );
static void  do_first_fn_c( PwiFirst *instance );

GType
pwi_first_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = first_register_type();
	}

	return( type );
}

static GType
first_register_type( void )
{
	static const gchar *thisfn = "first_register_type";

	static GTypeInfo info = {
		sizeof( PwiFirstClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) first_class_init,
		NULL,
		NULL,
		sizeof( PwiFirst ),
		0,
		( GInstanceInitFunc ) first_instance_init
	};

	g_debug( "%s", thisfn );
	return( g_type_register_static( G_TYPE_OBJECT, "PwiFirst", &info, 0 ));
}

static void
first_class_init( PwiFirstClass *klass )
{
	static const gchar *thisfn = "first_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_first_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = first_instance_dispose;
	object_class->finalize = first_instance_finalize;

	klass->private = g_new0( PwiFirstClassPrivate, 1 );

	klass->fn_a = do_first_fn_a;
	klass->fn_b = do_first_fn_b;
	klass->fn_c = do_first_fn_c;
}

static void
first_instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "first_instance_init";
	PwiFirst *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_assert( PWI_IS_FIRST( instance ));
	self = PWI_FIRST( instance );

	self->private = g_new0( PwiFirstPrivate, 1 );
}

static void
first_instance_dispose( GObject *instance )
{
	static const gchar *thisfn = "first_instance_dispose";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));

	/* chain up to the parent class */
	G_OBJECT_CLASS( st_first_parent_class )->dispose( instance );
}

static void
first_instance_finalize( GObject *instance )
{
	static const gchar *thisfn = "first_instance_finalize";
	PwiFirst *self;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));
	self = PWI_FIRST( instance );

	g_free( self->private );

	/* chain call to parent class */
	G_OBJECT_CLASS( st_first_parent_class )->finalize( instance );
}

void
pwi_first_fn_a( PwiFirst *instance )
{
	g_debug( "pwi_first_fn_a: instance=%p", ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));

	PWI_FIRST_GET_CLASS( instance )->fn_a( instance );
}

static void
do_first_fn_a( PwiFirst *instance )
{
	g_debug( "do_first_fn_a: instance=%p", ( void * ) instance );
}

void
pwi_first_fn_b( PwiFirst *instance )
{
	g_debug( "pwi_first_fn_b: instance=%p", ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));

	PWI_FIRST_GET_CLASS( instance )->fn_b( instance );
}

static void
do_first_fn_b( PwiFirst *instance )
{
	g_debug( "do_first_fn_b: instance=%p", ( void * ) instance );
}

void
pwi_first_fn_c( PwiFirst *instance )
{
	g_debug( "pwi_first_fn_c: instance=%p", ( void * ) instance );
	g_assert( PWI_IS_FIRST( instance ));

	PWI_FIRST_GET_CLASS( instance )->fn_c( instance );
}

static void
do_first_fn_c( PwiFirst *instance )
{
	g_debug( "do_first_fn_c: instance=%p", ( void * ) instance );
}

struct PwiFirstSecondClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

struct PwiFirstSecondPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static PwiFirstClass *st_first_second_parent_class = NULL;

static GType first_second_register_type( void );
static void  first_second_class_init( PwiFirstSecondClass *klass );
static void  first_second_instance_init( GTypeInstance *instance, gpointer klass );
static void  first_second_instance_dispose( GObject *application );
static void  first_second_instance_finalize( GObject *application );

static void  do_first_second_fn_a( PwiFirst *instance );
static void  do_first_second_fn_b( PwiFirst *instance );

GType
pwi_first_second_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = first_second_register_type();
	}

	return( type );
}

static GType
first_second_register_type( void )
{
	static const gchar *thisfn = "first_second_register_type";

	static GTypeInfo info = {
		sizeof( PwiFirstSecondClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) first_second_class_init,
		NULL,
		NULL,
		sizeof( PwiFirstSecond ),
		0,
		( GInstanceInitFunc ) first_second_instance_init
	};

	g_debug( "%s", thisfn );
	return( g_type_register_static( PWI_FIRST_TYPE, "PwiFirstSecond", &info, 0 ));
}

static void
first_second_class_init( PwiFirstSecondClass *klass )
{
	static const gchar *thisfn = "first_second_class_init";
	GObjectClass *object_class;
	PwiFirstClass *first_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_first_second_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = first_second_instance_dispose;
	object_class->finalize = first_second_instance_finalize;

	klass->private = g_new0( PwiFirstSecondClassPrivate, 1 );

	first_class = PWI_FIRST_CLASS( klass );
	first_class->fn_a = do_first_second_fn_a;
	first_class->fn_b = do_first_second_fn_b;
	first_class->fn_c = NULL;
}

static void
first_second_instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "first_second_instance_init";
	PwiFirstSecond *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_assert( PWI_IS_FIRST_SECOND( instance ));
	self = PWI_FIRST_SECOND( instance );

	self->private = g_new0( PwiFirstSecondPrivate, 1 );
}

static void
first_second_instance_dispose( GObject *instance )
{
	static const gchar *thisfn = "first_second_instance_dispose";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_assert( PWI_IS_FIRST_SECOND( instance ));

	/* chain up to the parent class */
	G_OBJECT_CLASS( st_first_second_parent_class )->dispose( instance );
}

static void
first_second_instance_finalize( GObject *instance )
{
	static const gchar *thisfn = "first_second_instance_finalize";
	PwiFirstSecond *self;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_assert( PWI_IS_FIRST_SECOND( instance ));
	self = PWI_FIRST_SECOND( instance );

	g_free( self->private );

	/* chain call to parent class */
	G_OBJECT_CLASS( st_first_second_parent_class )->finalize( instance );
}

static void
do_first_second_fn_a( PwiFirst *instance )
{
	g_debug( "do_first_second_fn_a: instance=%p", ( void * ) instance );
	PWI_FIRST_CLASS( st_first_second_parent_class )->fn_a( instance );
}

static void
do_first_second_fn_b( PwiFirst *instance )
{
	g_debug( "do_first_second_fn_b: instance=%p", ( void * ) instance );
	PWI_FIRST_CLASS( st_first_second_parent_class )->fn_b( instance );
}

struct PwiFirstSecondThreeClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

struct PwiFirstSecondThreePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static PwiFirstSecondClass *st_first_second_three_parent_class = NULL;

static GType first_second_three_register_type( void );
static void  first_second_three_class_init( PwiFirstSecondThreeClass *klass );
static void  first_second_three_instance_init( GTypeInstance *instance, gpointer klass );
static void  first_second_three_instance_dispose( GObject *application );
static void  first_second_three_instance_finalize( GObject *application );

static void  do_first_second_three_fn_a( PwiFirst *instance );
static void  do_first_second_three_fn_c( PwiFirst *instance );

GType
pwi_first_second_three_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = first_second_three_register_type();
	}

	return( type );
}

static GType
first_second_three_register_type( void )
{
	static const gchar *thisfn = "first_second_three_register_type";

	static GTypeInfo info = {
		sizeof( PwiFirstSecondThreeClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) first_second_three_class_init,
		NULL,
		NULL,
		sizeof( PwiFirstSecondThree ),
		0,
		( GInstanceInitFunc ) first_second_three_instance_init
	};

	g_debug( "%s", thisfn );
	return( g_type_register_static( PWI_FIRST_SECOND_TYPE, "PwiFirstSecondThree", &info, 0 ));
}

static void
first_second_three_class_init( PwiFirstSecondThreeClass *klass )
{
	static const gchar *thisfn = "first_second_three_class_init";
	GObjectClass *object_class;
	PwiFirstClass *first_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_first_second_three_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = first_second_three_instance_dispose;
	object_class->finalize = first_second_three_instance_finalize;

	klass->private = g_new0( PwiFirstSecondThreeClassPrivate, 1 );

	first_class = PWI_FIRST_CLASS( klass );
	first_class->fn_a = do_first_second_three_fn_a;
	first_class->fn_b = NULL;
	first_class->fn_c = do_first_second_three_fn_c;
}

static void
first_second_three_instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "first_second_three_instance_init";
	PwiFirstSecondThree *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_assert( PWI_IS_FIRST_SECOND_THREE( instance ));
	self = PWI_FIRST_SECOND_THREE( instance );

	self->private = g_new0( PwiFirstSecondThreePrivate, 1 );
}

static void
first_second_three_instance_dispose( GObject *instance )
{
	static const gchar *thisfn = "first_second_three_instance_dispose";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_assert( PWI_IS_FIRST_SECOND_THREE( instance ));

	/* chain up to the parent class */
	G_OBJECT_CLASS( st_first_second_three_parent_class )->dispose( instance );
}

static void
first_second_three_instance_finalize( GObject *instance )
{
	static const gchar *thisfn = "first_second_three_instance_finalize";
	PwiFirstSecondThree *self;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_assert( PWI_IS_FIRST_SECOND_THREE( instance ));
	self = PWI_FIRST_SECOND_THREE( instance );

	g_free( self->private );

	/* chain call to parent class */
	G_OBJECT_CLASS( st_first_second_three_parent_class )->finalize( instance );
}

static void
do_first_second_three_fn_a( PwiFirst *instance )
{
	g_debug( "do_first_second_three_fn_a: instance=%p", ( void * ) instance );
	PWI_FIRST_CLASS( st_first_second_three_parent_class )->fn_a( instance );
}

static void
do_first_second_three_fn_c( PwiFirst *instance )
{
	g_debug( "do_first_second_three_fn_c: instance=%p", ( void * ) instance );
	PWI_FIRST_CLASS( st_first_second_three_parent_class )->fn_c( instance );
}

int
main( int argc, char **argv )
{
	PwiFirst *a;
	PwiFirstSecond *b;
	PwiFirstSecondThree *c;

#if !GLIB_CHECK_VERSION( 2,36, 0 )
	g_type_init();
#endif

	a = g_object_new( PWI_FIRST_TYPE, NULL );
	b = g_object_new( PWI_FIRST_SECOND_TYPE, NULL );
	c = g_object_new( PWI_FIRST_SECOND_THREE_TYPE, NULL );

	g_debug( "expected pwi_first_fn_a, do_first_fn_a" );
	pwi_first_fn_a( PWI_FIRST( a ));
	g_debug( "expected pwi_first_fn_a, do_first_second_fn_a, do_first_fn_a" );
	pwi_first_fn_a( PWI_FIRST( b ));
	g_debug( "expected pwi_first_fn_a, do_first_second_three_fn_a, do_first_second_fn_a, do_first_fn_a" );
	pwi_first_fn_a( PWI_FIRST( c ));

	g_debug( "%s", "" );

	g_debug( "expected pwi_first_fn_b, do_first_fn_b" );
	pwi_first_fn_b( PWI_FIRST( a ));
	g_debug( "expected pwi_first_fn_b, do_first_second_fn_b, do_first_fn_b" );
	pwi_first_fn_b( PWI_FIRST( b ));
	g_debug( "expected pwi_first_fn_b, do_first_second_fn_b, do_first_fn_b" );
	/* NOT OK
	 * segmentation fault after pwi_first_fn_b */
	pwi_first_fn_b( PWI_FIRST( c ));

	g_debug( "%s", "" );

	g_debug( "expected pwi_first_fn_c, do_first_fn_c" );
	pwi_first_fn_c( PWI_FIRST( a ));
	g_debug( "expected pwi_first_fn_c, do_first_fn_c" );
	pwi_first_fn_c( PWI_FIRST( b ));
	g_debug( "expected pwi_first_fn_c, do_first_second_three_fn_c, do_first_fn_c" );
	pwi_first_fn_c( PWI_FIRST( c ));

	return( 0 );
}
