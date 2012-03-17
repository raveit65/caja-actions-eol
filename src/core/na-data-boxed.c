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

#include <stdlib.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-mateconf-utils.h>
#include <api/na-data-def.h>
#include <api/na-data-types.h>
#include <api/na-data-boxed.h>

/* private class data
 */
struct NADataBoxedClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NADataBoxedPrivate {
	gboolean      dispose_has_run;
	NADataDef    *def ;
	union {
		gboolean  boolean;
		gchar    *string;
		GSList   *slist;
		void     *pointer;
		guint     uint;
	} u;
};

typedef struct {
	guint           type;
	GParamSpec * ( *spec )           ( const NADataDef * );
	void         ( *free )           ( const NADataBoxed * );
	void         ( *dump )           ( const NADataBoxed * );
	gboolean     ( *are_equal )      ( const NADataBoxed *, const NADataBoxed * );
	gboolean     ( *is_valid )       ( const NADataBoxed * );
	gboolean     ( *is_set )         ( const NADataBoxed * );
	gchar *      ( *get_as_string )  ( const NADataBoxed * );
	void *       ( *get_as_void )    ( const NADataBoxed * );
	void         ( *get_as_value )   ( const NADataBoxed *, GValue *value );
	void         ( *set_from_boxed ) ( NADataBoxed *, const NADataBoxed * );
	void         ( *set_from_string )( NADataBoxed *, const gchar *string );
	void         ( *set_from_value ) ( NADataBoxed *, const GValue *value );
	void         ( *set_from_void )  ( NADataBoxed *, const void *value );
}
	DataBoxedFn;

static GObjectClass *st_parent_class   = NULL;

static GType register_type( void );
static void  class_init( NADataBoxedClass *klass );
static void  instance_init( GTypeInstance *instance, gpointer klass );
static void  instance_dispose( GObject *object );
static void  instance_finalize( GObject *object );

static DataBoxedFn *get_data_boxed_fn( guint type );

static GParamSpec *string_spec( const NADataDef *idtype );
static void        string_free( const NADataBoxed *boxed );
static void        string_dump( const NADataBoxed *boxed );
static gboolean    string_are_equal( const NADataBoxed *a, const NADataBoxed *b );
static gboolean    string_is_valid( const NADataBoxed *boxed );
static gboolean    string_is_set( const NADataBoxed *boxed );
static gchar      *string_get_as_string( const NADataBoxed *boxed );
static void       *string_get_as_void( const NADataBoxed *boxed );
static void        string_get_as_value( const NADataBoxed *boxed, GValue *value );
static void        string_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source );
static void        string_set_from_string( NADataBoxed *boxed, const gchar *string );
static void        string_set_from_value( NADataBoxed *boxed, const GValue *value );
static void        string_set_from_void( NADataBoxed *boxed, const void *value );

static gboolean    locale_are_equal( const NADataBoxed *a, const NADataBoxed *b );
static gboolean    locale_is_valid( const NADataBoxed *boxed );
static gboolean    locale_is_set( const NADataBoxed *boxed );

static GParamSpec *slist_spec( const NADataDef *idtype );
static void        slist_free( const NADataBoxed *boxed );
static void        slist_dump( const NADataBoxed *boxed );
static gboolean    slist_are_equal( const NADataBoxed *a, const NADataBoxed *b );
static gboolean    slist_is_valid( const NADataBoxed *boxed );
static gboolean    slist_is_set( const NADataBoxed *boxed );
static gchar      *slist_get_as_string( const NADataBoxed *boxed );
static void       *slist_get_as_void( const NADataBoxed *boxed );
static void        slist_get_as_value( const NADataBoxed *boxed, GValue *value );
static void        slist_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source );
static void        slist_set_from_string( NADataBoxed *boxed, const gchar *string );
static void        slist_set_from_value( NADataBoxed *boxed, const GValue *value );
static void        slist_set_from_void( NADataBoxed *boxed, const void *value );

static GParamSpec *bool_spec( const NADataDef *idtype );
static void        bool_free( const NADataBoxed *boxed );
static void        bool_dump( const NADataBoxed *boxed );
static gboolean    bool_are_equal( const NADataBoxed *a, const NADataBoxed *b );
static gboolean    bool_is_valid( const NADataBoxed *boxed );
static gboolean    bool_is_set( const NADataBoxed *boxed );
static gchar      *bool_get_as_string( const NADataBoxed *boxed );
static void       *bool_get_as_void( const NADataBoxed *boxed );
static void        bool_get_as_value( const NADataBoxed *boxed, GValue *value );
static void        bool_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source );
static void        bool_set_from_string( NADataBoxed *boxed, const gchar *string );
static void        bool_set_from_value( NADataBoxed *boxed, const GValue *value );
static void        bool_set_from_void( NADataBoxed *boxed, const void *value );

static GParamSpec *pointer_spec( const NADataDef *idtype );
static void        pointer_free( const NADataBoxed *boxed );
static void        pointer_dump( const NADataBoxed *boxed );
static gboolean    pointer_are_equal( const NADataBoxed *a, const NADataBoxed *b );
static gboolean    pointer_is_valid( const NADataBoxed *boxed );
static gboolean    pointer_is_set( const NADataBoxed *boxed );
static gchar      *pointer_get_as_string( const NADataBoxed *boxed );
static void       *pointer_get_as_void( const NADataBoxed *boxed );
static void        pointer_get_as_value( const NADataBoxed *boxed, GValue *value );
static void        pointer_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source );
static void        pointer_set_from_string( NADataBoxed *boxed, const gchar *string );
static void        pointer_set_from_value( NADataBoxed *boxed, const GValue *value );
static void        pointer_set_from_void( NADataBoxed *boxed, const void *value );

static GParamSpec *uint_spec( const NADataDef *idtype );
static void        uint_free( const NADataBoxed *boxed );
static void        uint_dump( const NADataBoxed *boxed );
static gboolean    uint_are_equal( const NADataBoxed *a, const NADataBoxed *b );
static gboolean    uint_is_valid( const NADataBoxed *boxed );
static gboolean    uint_is_set( const NADataBoxed *boxed );
static gchar      *uint_get_as_string( const NADataBoxed *boxed );
static void       *uint_get_as_void( const NADataBoxed *boxed );
static void        uint_get_as_value( const NADataBoxed *boxed, GValue *value );
static void        uint_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source );
static void        uint_set_from_string( NADataBoxed *boxed, const gchar *string );
static void        uint_set_from_value( NADataBoxed *boxed, const GValue *value );
static void        uint_set_from_void( NADataBoxed *boxed, const void *value );

static DataBoxedFn st_data_boxed_fn[] = {
		{ NAFD_TYPE_STRING,
				string_spec,
				string_free,
				string_dump,
				string_are_equal,
				string_is_valid,
				string_is_set,
				string_get_as_string,
				string_get_as_void,
				string_get_as_value,
				string_set_from_boxed,
				string_set_from_string,
				string_set_from_value,
				string_set_from_void
				},
		{ NAFD_TYPE_LOCALE_STRING,
				string_spec,
				string_free,
				string_dump,
				locale_are_equal,
				locale_is_valid,
				locale_is_set,
				string_get_as_string,
				string_get_as_void,
				string_get_as_value,
				string_set_from_boxed,
				string_set_from_string,
				string_set_from_value,
				string_set_from_void
				},
		{ NAFD_TYPE_STRING_LIST,
				slist_spec,
				slist_free,
				slist_dump,
				slist_are_equal,
				slist_is_valid,
				slist_is_set,
				slist_get_as_string,
				slist_get_as_void,
				slist_get_as_value,
				slist_set_from_boxed,
				slist_set_from_string,
				slist_set_from_value,
				slist_set_from_void
				},
		{ NAFD_TYPE_BOOLEAN,
				bool_spec,
				bool_free,
				bool_dump,
				bool_are_equal,
				bool_is_valid,
				bool_is_set,
				bool_get_as_string,
				bool_get_as_void,
				bool_get_as_value,
				bool_set_from_boxed,
				bool_set_from_string,
				bool_set_from_value,
				bool_set_from_void
				},
		{ NAFD_TYPE_POINTER,
				pointer_spec,
				pointer_free,
				pointer_dump,
				pointer_are_equal,
				pointer_is_valid,
				pointer_is_set,
				pointer_get_as_string,
				pointer_get_as_void,
				pointer_get_as_value,
				pointer_set_from_boxed,
				pointer_set_from_string,
				pointer_set_from_value,
				pointer_set_from_void
				},
		{ NAFD_TYPE_UINT,
				uint_spec,
				uint_free,
				uint_dump,
				uint_are_equal,
				uint_is_valid,
				uint_is_set,
				uint_get_as_string,
				uint_get_as_void,
				uint_get_as_value,
				uint_set_from_boxed,
				uint_set_from_string,
				uint_set_from_value,
				uint_set_from_void
				},
		{ 0 }
};

GType
na_data_boxed_get_type( void )
{
	static GType item_type = 0;

	if( item_type == 0 ){
		item_type = register_type();
	}

	return( item_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_data_boxed_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NADataBoxedClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NADataBoxed ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NADataBoxed", &info, 0 );

	return( type );
}

static void
class_init( NADataBoxedClass *klass )
{
	static const gchar *thisfn = "na_data_boxed_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NADataBoxedClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_data_boxed_instance_init";
	NADataBoxed *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( NA_IS_DATA_BOXED( instance ));

	self = NA_DATA_BOXED( instance );

	self->private = g_new0( NADataBoxedPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_data_boxed_instance_dispose";
	NADataBoxed *self;

	g_debug( "%s: object=%p (%s), name=%s",
			thisfn,
			( void * ) object, G_OBJECT_TYPE_NAME( object ),
			NA_DATA_BOXED( object )->private->def->name );

	g_return_if_fail( NA_IS_DATA_BOXED( object ));

	self = NA_DATA_BOXED( object );

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
	NADataBoxed *self;

	g_return_if_fail( NA_IS_DATA_BOXED( object ));

	self = NA_DATA_BOXED( object );

	DataBoxedFn *fn = get_data_boxed_fn( self->private->def->type );
	if( fn->free ){
		( *fn->free )( self );
	}

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static DataBoxedFn *
get_data_boxed_fn( guint type )
{
	static const gchar *thisfn = "na_data_boxed_get_data_boxed_fn";
	int i;
	DataBoxedFn *fn;

	fn = NULL;

	for( i = 0 ; st_data_boxed_fn[i].type && !fn ; ++i ){
		if( st_data_boxed_fn[i].type == type ){
			fn = st_data_boxed_fn+i;
		}
	}

	if( !fn ){
		g_warning( "%s: unmanaged type=%d", thisfn, type );
	}

	return( fn );
}

/**
 * na_data_boxed_get_param_spec:
 * @def: a #NADataDef definition structure.
 *
 * Returns: a #GParamSpec structure.
 */
GParamSpec *
na_data_boxed_get_param_spec( const NADataDef *def )
{
	GParamSpec *spec;
	DataBoxedFn *fn;

	spec = NULL;
	fn = get_data_boxed_fn( def->type );

	if( fn ){
		if( fn->spec ){
			spec = ( *fn->spec )( def );
		}
	}

	return( spec );
}

/**
 * na_data_boxed_new:
 * @def: the #NADataDef definition structure for this boxed.
 *
 * Returns: a newly allocated #NADataBoxed.
 */
NADataBoxed *
na_data_boxed_new( const NADataDef *def )
{
	NADataBoxed *boxed;

	g_return_val_if_fail( def != NULL, NULL );

	boxed = g_object_new( NA_DATA_BOXED_TYPE, NULL );

	boxed->private->def = ( NADataDef * ) def;

	return( boxed );
}

/**
 * na_data_boxed_get_data_def:
 * @boxed: this #NADataBoxed object.
 *
 * Returns: a pointer to the #NADataDef structure attached to the object.
 * Should never be %NULL.
 */
NADataDef *
na_data_boxed_get_data_def( const NADataBoxed *boxed )
{
	NADataDef *def;

	g_return_val_if_fail( NA_IS_DATA_BOXED( boxed ), NULL );

	def = NULL;

	if( !boxed->private->dispose_has_run ){

		def = boxed->private->def;
	}

	return( def );
}

/**
 * na_data_boxed_are_equal:
 * @a: the first #NADataBoxed object.
 * @b: the second #NADataBoxed object.
 *
 * Returns: %TRUE if the two boxeds are equal, %FALSE else.
 */
gboolean
na_data_boxed_are_equal( const NADataBoxed *a, const NADataBoxed *b )
{
	DataBoxedFn *fn;
	gboolean are_equal;

	g_return_val_if_fail( NA_IS_DATA_BOXED( a ), FALSE );
	g_return_val_if_fail( NA_IS_DATA_BOXED( b ), FALSE );

	are_equal = FALSE;

	if( !a->private->dispose_has_run &&
		!b->private->dispose_has_run ){

		if( a->private->def->type == b->private->def->type ){

			fn = get_data_boxed_fn( a->private->def->type );

			if( fn ){
				if( fn->are_equal ){
					are_equal = ( *fn->are_equal )( a, b );
				}
			}
		}
	}

	return( are_equal );
}

/**
 * na_data_boxed_is_valid:
 * @object: the #NADataBoxed object whose validity is to be checked.
 *
 * Returns: %TRUE if the boxed is valid, %FALSE else.
 */
gboolean
na_data_boxed_is_valid( const NADataBoxed *boxed )
{
	DataBoxedFn *fn;
	gboolean is_valid;

	g_return_val_if_fail( NA_IS_DATA_BOXED( boxed ), FALSE );

	is_valid = FALSE;

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->is_valid ){
				is_valid = ( *fn->is_valid )( boxed );
			}
		}
	}

	return( is_valid );
}

/**
 * na_data_boxed_is_set:
 * @boxed: this #NADataBoxed object.
 *
 * Returns: %TRUE if the #NADataBoxed is set,
 * %FALSE else, e.g. empty or equal to the default value.
 */
gboolean
na_data_boxed_is_set( const NADataBoxed *boxed )
{
	gboolean is_set;
	DataBoxedFn *fn;

	g_return_val_if_fail( NA_IS_DATA_BOXED( boxed ), FALSE );

	is_set = FALSE;

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->is_set ){
				is_set = ( *fn->is_set )( boxed );
			}
		}
	}

	return( is_set );
}

/**
 * na_data_boxed_dump:
 * @boxed: this #NADataBoxed object.
 *
 * Dump the content of @boxed.
 */
void
na_data_boxed_dump( const NADataBoxed *boxed )
{
	DataBoxedFn *fn;

	fn = get_data_boxed_fn( boxed->private->def->type );

	if( fn ){
		if( fn->dump ){
			( *fn->dump )( boxed );
		}
	}
}

/**
 * na_data_boxed_set_data_def:
 * @boxed: this #NADataBoxed object.
 * @def: the new #NADataDef to be set.
 *
 * Changes the #NADataDef a @boxed points to:
 * -> the new type must be the same that the previous one.
 * -> value is unchanged.
 */
void
na_data_boxed_set_data_def( NADataBoxed *boxed, const NADataDef *new_def )
{
	g_return_if_fail( NA_IS_DATA_BOXED( boxed ));
	g_return_if_fail( new_def != NULL );
	g_return_if_fail( new_def->type == boxed->private->def->type );

	if( !boxed->private->dispose_has_run ){

		boxed->private->def = ( NADataDef * ) new_def;
	}
}

/**
 * na_data_boxed_get_as_string:
 * @boxed: the #NADataBoxed whose value is to be set.
 *
 * Returns: the value of the @boxed, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
na_data_boxed_get_as_string( const NADataBoxed *boxed )
{
	DataBoxedFn *fn;
	gchar *value;

	g_return_val_if_fail( NA_IS_DATA_BOXED( boxed ), NULL );

	value = NULL;

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->get_as_string ){
				value = ( *fn->get_as_string )( boxed );
			}
		}
	}

	return( value );
}

/**
 * na_data_boxed_get_as_void:
 * @boxed: the #NADataBoxed whose value is to be set.
 *
 * Returns: the content of the @boxed.
 *
 * If of type NAFD_TYPE_STRING, NAFD_TYPE_LOCALE_STRING OR
 * NAFD_TYPE_STRING_LIST, then the content is returned in a newly
 * allocated value, which should be released by the caller.
 */
void *
na_data_boxed_get_as_void( const NADataBoxed *boxed )
{
	DataBoxedFn *fn;
	void *value;

	g_return_val_if_fail( NA_IS_DATA_BOXED( boxed ), NULL );

	value = NULL;

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->get_as_void ){
				value = ( *fn->get_as_void )( boxed );
			}
		}
	}

	return( value );
}

/**
 * na_data_boxed_get_as_value:
 * @boxed: the #NADataBoxed whose value is to be set.
 * @value: the string to be set.
 *
 * Setup @value with the content of the @boxed.
 */
void
na_data_boxed_get_as_value( const NADataBoxed *boxed, GValue *value )
{
	DataBoxedFn *fn;

	g_return_if_fail( NA_IS_DATA_BOXED( boxed ));

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->get_as_value ){
				( *fn->get_as_value )( boxed, value );
			}
		}
	}
}

/**
 * na_data_boxed_set_from_boxed:
 * @boxed: the #NADataBoxed whose value is to be set.
 * @value: the source #NADataBoxed.
 *
 * Copy value from @value to @boxed.
 */
void
na_data_boxed_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *value )
{
	DataBoxedFn *fn;

	g_return_if_fail( NA_IS_DATA_BOXED( boxed ));
	g_return_if_fail( NA_IS_DATA_BOXED( value ));
	g_return_if_fail( boxed->private->def->type == value->private->def->type );

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->free ){
				( *fn->free )( boxed );
			}
			if( fn->set_from_boxed ){
				( *fn->set_from_boxed )( boxed, value );
			}
		}
	}
}

/**
 * na_data_boxed_set_from_string:
 * @boxed: the #NADataBoxed whose value is to be set.
 * @value: the string to be set.
 *
 * Evaluates the @value and set it to the @boxed.
 */
void
na_data_boxed_set_from_string( NADataBoxed *boxed, const gchar *value )
{
	DataBoxedFn *fn;

	g_return_if_fail( NA_IS_DATA_BOXED( boxed ));

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->free ){
				( *fn->free )( boxed );
			}
			if( fn->set_from_string ){
				( *fn->set_from_string )( boxed, value );
			}
		}
	}
}

/**
 * na_data_boxed_set_from_value:
 * @boxed: the #NADataBoxed whose value is to be set.
 * @value: the value whose content is to be got.
 *
 * Evaluates the @value and set it to the @boxed.
 */
void
na_data_boxed_set_from_value( NADataBoxed *boxed, const GValue *value )
{
	DataBoxedFn *fn;

	g_return_if_fail( NA_IS_DATA_BOXED( boxed ));

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->free ){
				( *fn->free )( boxed );
			}
			if( fn->set_from_value ){
				( *fn->set_from_value )( boxed, value );
			}
		}
	}
}

/**
 * na_data_boxed_set_from_void:
 * @boxed: the #NADataBoxed whose value is to be set.
 * @value: the value whose content is to be got.
 *
 * Evaluates the @value and set it to the @boxed.
 */
void
na_data_boxed_set_from_void( NADataBoxed *boxed, const void *value )
{
	DataBoxedFn *fn;

	g_return_if_fail( NA_IS_DATA_BOXED( boxed ));

	if( !boxed->private->dispose_has_run ){

		fn = get_data_boxed_fn( boxed->private->def->type );

		if( fn ){
			if( fn->free ){
				( *fn->free )( boxed );
			}
			if( fn->set_from_void ){
				( *fn->set_from_void )( boxed, value );
			}
		}
	}
}

static GParamSpec *
string_spec( const NADataDef *def )
{
	return( g_param_spec_string(
			def->name,
			def->short_label,
			def->long_label,
			def->default_value,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));
}

static void
string_free( const NADataBoxed *boxed )
{
	g_free( boxed->private->u.string );
	boxed->private->u.string = NULL;
}

static void
string_dump( const NADataBoxed *boxed )
{
	g_debug( "na-data-boxed: %s=%s", boxed->private->def->name, boxed->private->u.string );
}

static gboolean
string_are_equal( const NADataBoxed *a, const NADataBoxed *b )
{
	if( !a->private->u.string && !b->private->u.string ){
		return( TRUE );
	}
	if( !a->private->u.string || !b->private->u.string ){
		return( FALSE );
	}
	if( strcmp( a->private->u.string, b->private->u.string ) == 0 ){
		return( TRUE );
	}
	return( FALSE );
}

static gboolean
string_is_valid( const NADataBoxed *boxed )
{
	gboolean is_valid = TRUE;

	if( boxed->private->def->mandatory ){
		if( !boxed->private->u.string || !strlen( boxed->private->u.string )){
			g_debug( "na_data_boxed_string_is_valid: invalid %s: mandatory but empty or null", boxed->private->def->name );
			is_valid = FALSE;
		}
	}

	return( is_valid );
}

static gboolean
string_is_set( const NADataBoxed *boxed )
{
	gboolean is_set = FALSE;

	if( boxed->private->u.string && strlen( boxed->private->u.string )){
		if( boxed->private->def->default_value && strlen( boxed->private->def->default_value )){
			is_set = ( strcmp( boxed->private->u.string, boxed->private->def->default_value ) != 0 );
		} else {
			is_set = TRUE;
		}
	}

	return( is_set );
}

static gchar *
string_get_as_string( const NADataBoxed *boxed )
{
	return( g_strdup( boxed->private->u.string ));
}

static void *
string_get_as_void( const NADataBoxed *boxed )
{
	void *value = NULL;

	if( boxed->private->u.string ){
		value = g_strdup( boxed->private->u.string );
	}

	return( value );
}

static void
string_get_as_value( const NADataBoxed *boxed, GValue *value )
{
	g_value_set_string( value, boxed->private->u.string );
}

static void
string_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source )
{
	boxed->private->u.string = g_strdup( source->private->u.string );
}

static void
string_set_from_string( NADataBoxed *boxed, const gchar *string )
{
	if( string ){
		boxed->private->u.string = g_strdup( string );
	}
}

static void
string_set_from_value( NADataBoxed *boxed, const GValue *value )
{
	if( g_value_get_string( value )){
		boxed->private->u.string = g_value_dup_string( value );
	}
}

static void
string_set_from_void( NADataBoxed *boxed, const void *value )
{
	if( value ){
		boxed->private->u.string = g_strdup(( const gchar * ) value );
	}
}

static gboolean
locale_are_equal( const NADataBoxed *a, const NADataBoxed *b )
{
	if( !a->private->u.string && !b->private->u.string ){
		return( TRUE );
	}
	if( !a->private->u.string || !b->private->u.string ){
		return( FALSE );
	}
	return( na_core_utils_str_collate( a->private->u.string, b->private->u.string ) == 0 );
}

static gboolean
locale_is_valid( const NADataBoxed *boxed )
{
	gboolean is_valid = TRUE;

	if( boxed->private->def->mandatory ){
		if( !boxed->private->u.string || !g_utf8_strlen( boxed->private->u.string, -1 )){
			g_debug( "na_data_boxed_string_is_valid: invalid %s: mandatory but empty or null", boxed->private->def->name );
			is_valid = FALSE;
		}
	}

	return( is_valid );
}

static gboolean
locale_is_set( const NADataBoxed *boxed )
{
	gboolean is_set = FALSE;

	if( boxed->private->u.string && g_utf8_strlen( boxed->private->u.string, -1 )){
		if( boxed->private->def->default_value && g_utf8_strlen( boxed->private->def->default_value, -1 )){
			is_set = ( na_core_utils_str_collate( boxed->private->u.string, boxed->private->def->default_value ) != 0 );
		} else {
			is_set = TRUE;
		}
	}

	return( is_set );
}

static GParamSpec *
slist_spec( const NADataDef *def )
{
	return( g_param_spec_pointer(
			def->name,
			def->short_label,
			def->long_label,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));
}

static void
slist_free( const NADataBoxed *boxed )
{
	na_core_utils_slist_free( boxed->private->u.slist );
	boxed->private->u.slist = NULL;
}

static void
slist_dump( const NADataBoxed *boxed )
{
	g_debug( "na-data-boxed: %s=", boxed->private->def->name );
	na_core_utils_slist_dump( boxed->private->u.slist );
}

static gboolean
slist_are_equal( const NADataBoxed *a, const NADataBoxed *b )
{
	if( !a->private->u.slist && !b->private->u.slist ){
		return( TRUE );
	}
	if( !a->private->u.slist || !b->private->u.slist ){
		return( FALSE );
	}
	return( na_core_utils_slist_are_equal( a->private->u.slist, b->private->u.slist ));
}

static gboolean
slist_is_valid( const NADataBoxed *boxed )
{
	gboolean is_valid = TRUE;

	if( boxed->private->def->mandatory ){
		if( !boxed->private->u.slist || !g_slist_length( boxed->private->u.slist )){
			g_debug( "na_data_boxed_string_is_valid: invalid %s: mandatory but empty or null", boxed->private->def->name );
			is_valid = FALSE;
		}
	}

	return( is_valid );
}

static gboolean
slist_is_set( const NADataBoxed *boxed )
{
	gboolean is_set = FALSE;
	GSList *default_value;

	if( boxed->private->u.slist && g_slist_length( boxed->private->u.slist )){
		if( boxed->private->def->default_value ){
			default_value = na_mateconf_utils_slist_from_string( boxed->private->def->default_value );
			is_set = !na_core_utils_slist_are_equal( default_value, boxed->private->u.slist );
			na_core_utils_slist_free( default_value );

		} else {
			is_set = TRUE;
		}
	}

	return( is_set );
}

static gchar *
slist_get_as_string( const NADataBoxed *boxed )
{
	return( na_mateconf_utils_slist_to_string( boxed->private->u.slist ));
}

static void *
slist_get_as_void( const NADataBoxed *boxed )
{
	void *value = NULL;

	if( boxed->private->u.slist ){
		value = na_core_utils_slist_duplicate( boxed->private->u.slist );
	}

	return( value );
}

static void
slist_get_as_value( const NADataBoxed *boxed, GValue *value )
{
	g_value_set_pointer( value, na_core_utils_slist_duplicate( boxed->private->u.slist ));
}

static void
slist_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source )
{
	boxed->private->u.slist = na_core_utils_slist_duplicate( source->private->u.slist );
}

static void
slist_set_from_string( NADataBoxed *boxed, const gchar *string )
{
	GSList *slist;

	if( string ){

		/* if it is a string list which comes from MateConf
		 */
		slist = na_mateconf_utils_slist_from_string( string );

		if( slist ){
			boxed->private->u.slist = slist;

		} else {
			boxed->private->u.slist = g_slist_append( NULL, g_strdup( string ));
		}
	}
}

static void
slist_set_from_value( NADataBoxed *boxed, const GValue *value )
{
	if( g_value_get_pointer( value )){
		boxed->private->u.slist = na_core_utils_slist_duplicate( g_value_get_pointer( value ));
	}
}

static void
slist_set_from_void( NADataBoxed *boxed, const void *value )
{
	if( value ){
		boxed->private->u.slist = na_core_utils_slist_duplicate(( GSList * ) value );
	}
}

static GParamSpec *
bool_spec( const NADataDef *def )
{
	return( g_param_spec_boolean(
			def->name,
			def->short_label,
			def->long_label,
			na_core_utils_boolean_from_string( def->default_value ),
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));
}

static void
bool_free( const NADataBoxed *boxed )
{
	/* n/a */
}

static void
bool_dump( const NADataBoxed *boxed )
{
	g_debug( "na-data-boxed: %s=%s",
			boxed->private->def->name, boxed->private->u.boolean ? "True":"False" );
}

static gboolean
bool_are_equal( const NADataBoxed *a, const NADataBoxed *b )
{
	return( a->private->u.boolean == b->private->u.boolean );
}

static gboolean
bool_is_valid( const NADataBoxed *boxed )
{
	return( TRUE );
}

static gboolean
bool_is_set( const NADataBoxed *boxed )
{
	gboolean is_set = TRUE;
	gboolean default_value;

	if( boxed->private->def->default_value && strlen( boxed->private->def->default_value )){
		default_value = na_core_utils_boolean_from_string( boxed->private->def->default_value );
		is_set = ( default_value != boxed->private->u.boolean );
	}

	return( is_set );
}

static gchar *
bool_get_as_string( const NADataBoxed *boxed )
{
	return( g_strdup_printf( "%s", boxed->private->u.boolean ? "True":"False" ));
}

static void *
bool_get_as_void( const NADataBoxed *boxed )
{
	return( GUINT_TO_POINTER( boxed->private->u.boolean ));
}

static void
bool_get_as_value( const NADataBoxed *boxed, GValue *value )
{
	g_value_set_boolean( value, boxed->private->u.boolean );
}

static void
bool_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source )
{
	boxed->private->u.boolean = source->private->u.boolean;
}

static void
bool_set_from_string( NADataBoxed *boxed, const gchar *string )
{
	boxed->private->u.boolean = na_core_utils_boolean_from_string( string );
}

static void
bool_set_from_value( NADataBoxed *boxed, const GValue *value )
{
	boxed->private->u.boolean = g_value_get_boolean( value );
}

static void
bool_set_from_void( NADataBoxed *boxed, const void *value )
{
	boxed->private->u.boolean = GPOINTER_TO_UINT( value );
}

static GParamSpec *
pointer_spec( const NADataDef *def )
{
	return( g_param_spec_pointer(
			def->name,
			def->short_label,
			def->long_label,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));
}

static void
pointer_free( const NADataBoxed *boxed )
{
	boxed->private->u.pointer = NULL;
}

static void
pointer_dump( const NADataBoxed *boxed )
{
	g_debug( "na-data-boxed: %s=%p",
			boxed->private->def->name, ( void * ) boxed->private->u.pointer );
}

static gboolean
pointer_are_equal( const NADataBoxed *a, const NADataBoxed *b )
{
	return( a->private->u.pointer == b->private->u.pointer );
}

static gboolean
pointer_is_valid( const NADataBoxed *boxed )
{
	gboolean is_valid = TRUE;

	if( boxed->private->def->mandatory ){
		if( !boxed->private->u.pointer ){
			g_debug( "na_data_boxed_string_is_valid: invalid %s: mandatory but null", boxed->private->def->name );
			is_valid = FALSE;
		}
	}

	return( is_valid );
}

static gboolean
pointer_is_set( const NADataBoxed *boxed )
{
	gboolean is_set = FALSE;

	if( boxed->private->u.pointer ){
		is_set = TRUE;
	}

	return( is_set );
}

static gchar *
pointer_get_as_string( const NADataBoxed *boxed )
{
	return( g_strdup_printf( "%p", boxed->private->u.pointer ));
}

static void *
pointer_get_as_void( const NADataBoxed *boxed )
{
	return( boxed->private->u.pointer );
}

static void
pointer_get_as_value( const NADataBoxed *boxed, GValue *value )
{
	g_value_set_pointer( value, boxed->private->u.pointer );
}

static void
pointer_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source )
{
	boxed->private->u.pointer = source->private->u.pointer;
}

static void
pointer_set_from_string( NADataBoxed *boxed, const gchar *pointer )
{
	g_warning( "na_data_boxed_pointer_set_from_string: unrelevant function call" );
}

static void
pointer_set_from_value( NADataBoxed *boxed, const GValue *value )
{
	boxed->private->u.pointer = g_value_get_pointer( value );
}

static void
pointer_set_from_void( NADataBoxed *boxed, const void *value )
{
	boxed->private->u.pointer = ( void * ) value;
}

static GParamSpec *
uint_spec( const NADataDef *def )
{
	return( g_param_spec_uint(
			def->name,
			def->short_label,
			def->long_label,
			0,
			UINT_MAX,
			atoi( def->default_value ),
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));
}

static void
uint_free( const NADataBoxed *boxed )
{
	boxed->private->u.uint = 0;
}

static void
uint_dump( const NADataBoxed *boxed )
{
	g_debug( "na-data-boxed: %s=%d",
			boxed->private->def->name, boxed->private->u.uint );
}

static gboolean
uint_are_equal( const NADataBoxed *a, const NADataBoxed *b )
{
	return( a->private->u.uint == b->private->u.uint );
}

static gboolean
uint_is_valid( const NADataBoxed *boxed )
{
	return( TRUE );
}

static gboolean
uint_is_set( const NADataBoxed *boxed )
{
	gboolean is_set = FALSE;
	guint default_value;

	if( boxed->private->def->default_value ){
		default_value = atoi( boxed->private->def->default_value );
		is_set = ( boxed->private->u.uint != default_value );

	} else {
		is_set = TRUE;
	}

	return( is_set );
}

static gchar *
uint_get_as_string( const NADataBoxed *boxed )
{
	return( g_strdup_printf( "%u", boxed->private->u.uint ));
}

static void *
uint_get_as_void( const NADataBoxed *boxed )
{
	return( GUINT_TO_POINTER( boxed->private->u.uint ));
}

static void
uint_get_as_value( const NADataBoxed *boxed, GValue *value )
{
	g_value_set_uint( value, boxed->private->u.uint );
}

static void
uint_set_from_boxed( NADataBoxed *boxed, const NADataBoxed *source )
{
	boxed->private->u.uint = source->private->u.uint;
}

static void
uint_set_from_string( NADataBoxed *boxed, const gchar *string )
{
	boxed->private->u.uint = atoi( string );
}

static void
uint_set_from_value( NADataBoxed *boxed, const GValue *value )
{
	boxed->private->u.uint = g_value_get_uint( value );
}

static void
uint_set_from_void( NADataBoxed *boxed, const void *value )
{
	boxed->private->u.uint = GPOINTER_TO_UINT( value );
}
