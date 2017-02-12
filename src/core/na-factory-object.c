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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-data-boxed.h>
#include <api/na-data-types.h>
#include <api/na-iio-provider.h>
#include <api/na-ifactory-provider.h>
#include <api/na-object-api.h>

#include "na-factory-object.h"
#include "na-factory-provider.h"

typedef gboolean ( *NADataDefIterFunc )( NADataDef *def, void *user_data );

enum {
	DATA_DEF_ITER_SET_PROPERTIES = 1,
	DATA_DEF_ITER_SET_DEFAULTS,
	DATA_DEF_ITER_IS_VALID,
	DATA_DEF_ITER_READ_ITEM,
};

/* while iterating on read item
 */
typedef struct {
	NAIFactoryObject   *object;
	NAIFactoryProvider *reader;
	void               *reader_data;
	GSList            **messages;
}
	NafoReadIter;

/* while iterating on write item
 */
typedef struct {
	NAIFactoryProvider *writer;
	void               *writer_data;
	GSList            **messages;
	guint               code;
}
	NafoWriteIter;

/* while iterating on is_valid
 */
typedef struct {
	NAIFactoryObject  *object;
	gboolean           is_valid;
}
	NafoValidIter;

/* while iterating on set defaults
 */
typedef struct {
	NAIFactoryObject *object;
}
	NafoDefaultIter;

extern gboolean                   ifactory_object_initialized;
extern gboolean                   ifactory_object_finalized;

static gboolean     define_class_properties_iter( const NADataDef *def, GObjectClass *class );
static gboolean     set_defaults_iter( NADataDef *def, NafoDefaultIter *data );
static gboolean     is_valid_mandatory_iter( const NADataDef *def, NafoValidIter *data );
static gboolean     read_data_iter( NADataDef *def, NafoReadIter *iter );
static gboolean     write_data_iter( const NAIFactoryObject *object, NADataBoxed *boxed, NafoWriteIter *iter );

static NADataGroup *v_get_groups( const NAIFactoryObject *object );
static void         v_copy( NAIFactoryObject *target, const NAIFactoryObject *source );
static gboolean     v_are_equal( const NAIFactoryObject *a, const NAIFactoryObject *b );
static gboolean     v_is_valid( const NAIFactoryObject *object );
static void         v_read_start( NAIFactoryObject *serializable, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
static void         v_read_done( NAIFactoryObject *serializable, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
static guint        v_write_start( NAIFactoryObject *serializable, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
static guint        v_write_done( NAIFactoryObject *serializable, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );

static void         attach_boxed_to_object( NAIFactoryObject *object, NADataBoxed *boxed );
static void         free_data_boxed_list( NAIFactoryObject *object );
static void         iter_on_data_defs( const NADataGroup *idgroups, guint mode, NADataDefIterFunc pfn, void *user_data );

/*
 * na_factory_object_define_properties:
 * @class: the #GObjectClass.
 * @groups: the list of #NADataGroup structure which define the data of the class.
 *
 * Initializes all the properties for the class.
 */
void
na_factory_object_define_properties( GObjectClass *class, const NADataGroup *groups )
{
	static const gchar *thisfn = "na_factory_object_define_properties";

	g_return_if_fail( G_IS_OBJECT_CLASS( class ));

	g_debug( "%s: class=%p (%s)",
			thisfn, ( void * ) class, G_OBJECT_CLASS_NAME( class ));

	/* define class properties
	 */
	iter_on_data_defs( groups, DATA_DEF_ITER_SET_PROPERTIES, ( NADataDefIterFunc ) define_class_properties_iter, class );
}

static gboolean
define_class_properties_iter( const NADataDef *def, GObjectClass *class )
{
	static const gchar *thisfn = "na_factory_object_define_class_properties_iter";
	gboolean stop;
	GParamSpec *spec;

	g_debug( "%s: def=%p (%s)", thisfn, ( void * ) def, def->name );

	stop = FALSE;

	spec = na_data_boxed_get_param_spec( def );

	if( spec ){
		g_object_class_install_property( class, g_quark_from_string( def->name ), spec );

	} else {
		g_warning( "%s: type=%d: unable to get a spec", thisfn, def->type );
	}

	return( stop );
}

/*
 * na_factory_object_get_data_def:
 * @object: this #NAIFactoryObject object.
 * @name: the searched name.
 *
 * Returns: the #NADataDef structure which describes this @name, or %NULL.
 */
NADataDef *
na_factory_object_get_data_def( const NAIFactoryObject *object, const gchar *name )
{
	NADataDef *def;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	def = NULL;

	NADataGroup *groups = v_get_groups( object );
	while( groups->group ){

		NADataDef *def = groups->def;
		if( def ){
			while( def->name ){

				if( !strcmp( def->name, name )){
					return( def );
				}
				def++;
			}
		}
		groups++;
	}

	return( def );
}

/*
 * na_factory_object_get_data_groups:
 * @object: the #NAIFactoryObject instance.
 *
 * Returns: a pointer to the list of #NADataGroup which define the data.
 */
NADataGroup *
na_factory_object_get_data_groups( const NAIFactoryObject *object )
{
	NADataGroup *groups;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	groups = v_get_groups( object );

	return( groups );
}

/*
 * na_factory_object_iter_on_boxed:
 * @object: this #NAIFactoryObject object.
 * @pfn: the function to be called.
 * @user_data: data to be provided to the user function.
 *
 * Iterate on each #NADataBoxed attached to the @object.
 *
 * The @fn called function may return %TRUE to stop the iteration.
 */
void
na_factory_object_iter_on_boxed( const NAIFactoryObject *object, NAFactoryObjectIterBoxedFn pfn, void *user_data )
{
	GList *list, *ibox;
	gboolean stop;

	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	list = g_object_get_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA );
	/*g_debug( "list=%p (count=%u)", ( void * ) list, g_list_length( list ));*/
	stop = FALSE;

	for( ibox = list ; ibox && !stop ; ibox = ibox->next ){
		stop = ( *pfn )( object, NA_DATA_BOXED( ibox->data ), user_data );
	}
}

/*
 * na_factory_object_get_default:
 * @object: this #NAIFactoryObject object.
 * @name: the searched name.
 *
 * Returns: the default value for this @object, as a newly allocated
 * string which should be g_free() by the caller.
 */
gchar *
na_factory_object_get_default( NAIFactoryObject *object, const gchar *name )
{
	static const gchar *thisfn = "na_factory_object_set_defaults";
	gchar *value;
	NADataDef *def;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	value = NULL;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	def = na_factory_object_get_data_def( object, name );
	if( def ){
		value = g_strdup( def->default_value );
	}

	return( value );
}

/*
 * na_factory_object_set_defaults:
 * @object: this #NAIFactoryObject object.
 *
 * Implement default values in this new @object.
 */
void
na_factory_object_set_defaults( NAIFactoryObject *object )
{
	static const gchar *thisfn = "na_factory_object_set_defaults";
	NADataGroup *groups;
	NafoDefaultIter *iter_data;

	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	groups = v_get_groups( object );
	if( !groups ){
		g_warning( "%s: no NADataGroup found for %s", thisfn, G_OBJECT_TYPE_NAME( object ));

	} else {
		iter_data = g_new0( NafoDefaultIter, 1 );
		iter_data->object = object;

		iter_on_data_defs( groups, DATA_DEF_ITER_SET_DEFAULTS, ( NADataDefIterFunc ) set_defaults_iter, iter_data );

		g_free( iter_data );
	}
}

/*
 * because this function is called very early in the NAIFactoryObject life,
 * we assume here that if a NADataBoxed has been allocated, then this is
 * most probably because it is set. Thus a 'null' value is not considered
 * as an 'unset' value.
 */
static gboolean
set_defaults_iter( NADataDef *def, NafoDefaultIter *data )
{
	NADataBoxed *boxed = na_ifactory_object_get_data_boxed( data->object, def->name );

	if( !boxed ){
		boxed = na_data_boxed_new( def );
		attach_boxed_to_object( data->object, boxed );
		na_boxed_set_from_string( NA_BOXED( boxed ), def->default_value );
	}

	/* do not stop */
	return( FALSE );
}

/*
 * na_factory_object_move_boxed:
 * @target: the target #NAIFactoryObject instance.
 * @source: the source #NAIFactoryObject instance.
 * @boxed: a #NADataBoxed.
 *
 * Move the @boxed from @source to @target, detaching from @source list
 * to be attached to @target one.
 */
void
na_factory_object_move_boxed( NAIFactoryObject *target, const NAIFactoryObject *source, NADataBoxed *boxed )
{
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( target ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( source ));

	GList *src_list = g_object_get_data( G_OBJECT( source ), NA_IFACTORY_OBJECT_PROP_DATA );

	if( g_list_find( src_list, boxed )){
		src_list = g_list_remove( src_list, boxed );
		g_object_set_data( G_OBJECT( source ), NA_IFACTORY_OBJECT_PROP_DATA, src_list );

		attach_boxed_to_object( target, boxed );

		const NADataDef *src_def = na_data_boxed_get_data_def( boxed );
		NADataDef *tgt_def = na_factory_object_get_data_def( target, src_def->name );
		na_data_boxed_set_data_def( boxed, tgt_def );
	}
}

/*
 * na_factory_object_copy:
 * @target: the target #NAIFactoryObject instance.
 * @source: the source #NAIFactoryObject instance.
 *
 * Copies one instance to another.
 * Takes care of not overriding provider data.
 */
void
na_factory_object_copy( NAIFactoryObject *target, const NAIFactoryObject *source )
{
	static const gchar *thisfn = "na_factory_object_copy";
	GList *dest_list, *idest, *inext;
	GList *src_list, *isrc;
	NADataBoxed *boxed;
	const NADataDef *def;
	void *provider, *provider_data;

	g_return_if_fail( NA_IS_IFACTORY_OBJECT( target ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( source ));

	g_debug( "%s: target=%p (%s), source=%p (%s)",
			thisfn,
			( void * ) target, G_OBJECT_TYPE_NAME( target ),
			( void * ) source, G_OBJECT_TYPE_NAME( source ));

	/* first remove copyable data from target
	 */
	provider = na_object_get_provider( target );
	provider_data = na_object_get_provider_data( target );

	idest = dest_list = g_object_get_data( G_OBJECT( target ), NA_IFACTORY_OBJECT_PROP_DATA );
	while( idest ){
		boxed = NA_DATA_BOXED( idest->data );
		inext = idest->next;
		def = na_data_boxed_get_data_def( boxed );
		if( def->copyable ){
			dest_list = g_list_remove_link( dest_list, idest );
			g_object_unref( idest->data );
		}
		idest = inext;
	}
	g_object_set_data( G_OBJECT( target ), NA_IFACTORY_OBJECT_PROP_DATA, dest_list );

	/* only then copy copyable data from source
	 */
	src_list = g_object_get_data( G_OBJECT( source ), NA_IFACTORY_OBJECT_PROP_DATA );
	for( isrc = src_list ; isrc ; isrc = isrc->next ){
		boxed = NA_DATA_BOXED( isrc->data );
		def = na_data_boxed_get_data_def( boxed );
		if( def->copyable ){
			NADataBoxed *tgt_boxed = na_ifactory_object_get_data_boxed( target, def->name );
			if( !tgt_boxed ){
				tgt_boxed = na_data_boxed_new( def );
				attach_boxed_to_object( target, tgt_boxed );
			}
			na_boxed_set_from_boxed( NA_BOXED( tgt_boxed ), NA_BOXED( boxed ));
		}
	}

	if( provider ){
		na_object_set_provider( target, provider );
		if( provider_data ){
			na_object_set_provider_data( target, provider_data );
		}
	}

	v_copy( target, source );
}

/*
 * na_factory_object_are_equal:
 * @a: the first (original) #NAIFactoryObject instance.
 * @b: the second (current) #NAIFactoryObject isntance.
 *
 * Returns: %TRUE if @a is equal to @b, %FALSE else.
 */
gboolean
na_factory_object_are_equal( const NAIFactoryObject *a, const NAIFactoryObject *b )
{
	static const gchar *thisfn = "na_factory_object_are_equal";
	gboolean are_equal;
	GList *a_list, *b_list, *ia, *ib;

	are_equal = FALSE;

	a_list = g_object_get_data( G_OBJECT( a ), NA_IFACTORY_OBJECT_PROP_DATA );
	b_list = g_object_get_data( G_OBJECT( b ), NA_IFACTORY_OBJECT_PROP_DATA );

	g_debug( "%s: a=%p, b=%p", thisfn, ( void * ) a, ( void * ) b );

	are_equal = TRUE;
	for( ia = a_list ; ia && are_equal ; ia = ia->next ){

		NADataBoxed *a_boxed = NA_DATA_BOXED( ia->data );
		const NADataDef *a_def = na_data_boxed_get_data_def( a_boxed );
		if( a_def->comparable ){

			NADataBoxed *b_boxed = na_ifactory_object_get_data_boxed( b, a_def->name );
			if( b_boxed ){
				are_equal = na_boxed_are_equal( NA_BOXED( a_boxed ), NA_BOXED( b_boxed ));
				if( !are_equal ){
					g_debug( "%s: %s not equal as %s different", thisfn, G_OBJECT_TYPE_NAME( a ), a_def->name );
				}

			} else {
				are_equal = FALSE;
				g_debug( "%s: %s not equal as %s has disappeared", thisfn, G_OBJECT_TYPE_NAME( a ), a_def->name );
			}
		}
	}

	for( ib = b_list ; ib && are_equal ; ib = ib->next ){

		NADataBoxed *b_boxed = NA_DATA_BOXED( ib->data );
		const NADataDef *b_def = na_data_boxed_get_data_def( b_boxed );
		if( b_def->comparable ){

			NADataBoxed *a_boxed = na_ifactory_object_get_data_boxed( a, b_def->name );
			if( !a_boxed ){
				are_equal = FALSE;
				g_debug( "%s: %s not equal as %s was not set", thisfn, G_OBJECT_TYPE_NAME( a ), b_def->name );
			}
		}
	}

	are_equal &= v_are_equal( a, b );

	return( are_equal );
}

/*
 * na_factory_object_is_valid:
 * @object: the #NAIFactoryObject instance whose validity is to be checked.
 *
 * Returns: %TRUE if @object is valid, %FALSE else.
 */
gboolean
na_factory_object_is_valid( const NAIFactoryObject *object )
{
	static const gchar *thisfn = "na_factory_object_is_valid";
	gboolean is_valid;
	NADataGroup *groups;
	GList *list, *iv;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), FALSE );

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	list = g_object_get_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA );
	is_valid = TRUE;

	/* mandatory data must be set
	 */
	NafoValidIter iter_data;
	iter_data.object = ( NAIFactoryObject * ) object;
	iter_data.is_valid = TRUE;

	groups = v_get_groups( object );
	if( groups ){
		iter_on_data_defs( groups, DATA_DEF_ITER_IS_VALID, ( NADataDefIterFunc ) is_valid_mandatory_iter, &iter_data );
	}
	is_valid = iter_data.is_valid;

	for( iv = list ; iv && is_valid ; iv = iv->next ){
		is_valid = na_data_boxed_is_valid( NA_DATA_BOXED( iv->data ));
	}

	is_valid &= v_is_valid( object );

	return( is_valid );
}

static gboolean
is_valid_mandatory_iter( const NADataDef *def, NafoValidIter *data )
{
	NADataBoxed *boxed;

	if( def->mandatory ){
		boxed = na_ifactory_object_get_data_boxed( data->object, def->name );
		if( !boxed ){
			g_debug( "na_factory_object_is_valid_mandatory_iter: invalid %s: mandatory but not set", def->name );
			data->is_valid = FALSE;
		}
	}

	/* do not stop iteration while valid */
	return( !data->is_valid );
}

/*
 * na_factory_object_dump:
 * @object: this #NAIFactoryObject instance.
 *
 * Dumps the content of @object.
 */
void
na_factory_object_dump( const NAIFactoryObject *object )
{
	static const gchar *thisfn = "na_factory_object_dump";
	static const gchar *prefix = "na-factory-data-";
	GList *list, *it;
	guint length;
	guint l_prefix;

	length = 0;
	l_prefix = strlen( prefix );
	list = g_object_get_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA );

	for( it = list ; it ; it = it->next ){
		NADataBoxed *boxed = NA_DATA_BOXED( it->data );
		const NADataDef *def = na_data_boxed_get_data_def( boxed );
		length = MAX( length, strlen( def->name ));
	}

	length -= l_prefix;
	length += 1;

	for( it = list ; it ; it = it->next ){
		NADataBoxed *boxed = NA_DATA_BOXED( it->data );
		const NADataDef *def = na_data_boxed_get_data_def( boxed );
		gchar *value = na_boxed_get_string( NA_BOXED( boxed ));
		g_debug( "| %s: %*s=%s", thisfn, length, def->name+l_prefix, value );
		g_free( value );
	}
}

/*
 * na_factory_object_finalize:
 * @object: the #NAIFactoryObject being finalized.
 *
 * Clears all data associated with this @object.
 */
void
na_factory_object_finalize( NAIFactoryObject *object )
{
	free_data_boxed_list( object );
}

/*
 * na_factory_object_read_item:
 * @object: this #NAIFactoryObject instance.
 * @reader: the #NAIFactoryProvider which is at the origin of this read.
 * @reader_data: reader data.
 * @messages: a pointer to a #GSList list of strings; the implementation
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * Unserializes the object.
 */
void
na_factory_object_read_item( NAIFactoryObject *object, const NAIFactoryProvider *reader, void *reader_data, GSList **messages )
{
	static const gchar *thisfn = "na_factory_object_read_item";

	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));
	g_return_if_fail( NA_IS_IFACTORY_PROVIDER( reader ));

	NADataGroup *groups = v_get_groups( object );

	if( groups ){
		v_read_start( object, reader, reader_data, messages );

		NafoReadIter *iter = g_new0( NafoReadIter, 1 );
		iter->object = object;
		iter->reader = ( NAIFactoryProvider * ) reader;
		iter->reader_data = reader_data;
		iter->messages = messages;

		iter_on_data_defs( groups, DATA_DEF_ITER_READ_ITEM, ( NADataDefIterFunc ) read_data_iter, iter );

		g_free( iter );

		v_read_done( object, reader, reader_data, messages );

	} else {
		g_warning( "%s: class %s doesn't return any NADataGroup structure",
				thisfn, G_OBJECT_TYPE_NAME( object ));
	}
}

static gboolean
read_data_iter( NADataDef *def, NafoReadIter *iter )
{
	gboolean stop;

	stop = FALSE;

	NADataBoxed *boxed = na_factory_provider_read_data( iter->reader, iter->reader_data, iter->object, def, iter->messages );

	if( boxed ){
		NADataBoxed *exist = na_ifactory_object_get_data_boxed( iter->object, def->name );

		if( exist ){
			na_boxed_set_from_boxed( NA_BOXED( exist ), NA_BOXED( boxed ));
			g_object_unref( boxed );

		} else {
			attach_boxed_to_object( iter->object, boxed );
		}
	}

	return( stop );
}

/*
 * na_factory_object_write_item:
 * @object: this #NAIFactoryObject instance.
 * @writer: the #NAIFactoryProvider which is at the origin of this write.
 * @writer_data: writer data.
 * @messages: a pointer to a #GSList list of strings; the implementation
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * Serializes the object down to the @writer.
 *
 * Returns: a NAIIOProvider operation return code.
 */
guint
na_factory_object_write_item( NAIFactoryObject *object, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	static const gchar *thisfn = "na_factory_object_write_item";
	guint code;
	NADataGroup *groups;
	gchar *msg;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( writer ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );

	code = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	groups = v_get_groups( object );

	if( groups ){
		code = v_write_start( object, writer, writer_data, messages );

	} else {
		msg = g_strdup_printf( "%s: class %s doesn't return any NADataGroup structure",
				thisfn, G_OBJECT_TYPE_NAME( object ));
		g_warning( "%s", msg );
		*messages = g_slist_append( *messages, msg );
	}

	if( code == NA_IIO_PROVIDER_CODE_OK ){

		NafoWriteIter *iter = g_new0( NafoWriteIter, 1 );
		iter->writer = ( NAIFactoryProvider * ) writer;
		iter->writer_data = writer_data;
		iter->messages = messages;
		iter->code = code;

		na_factory_object_iter_on_boxed( object, ( NAFactoryObjectIterBoxedFn ) write_data_iter, iter );

		code = iter->code;
		g_free( iter );
	}

	if( code == NA_IIO_PROVIDER_CODE_OK ){
		code = v_write_done( object, writer, writer_data, messages );
	}

	return( code );
}

static gboolean
write_data_iter( const NAIFactoryObject *object, NADataBoxed *boxed, NafoWriteIter *iter )
{
	const NADataDef *def = na_data_boxed_get_data_def( boxed );

	if( def->writable ){
		iter->code = na_factory_provider_write_data( iter->writer, iter->writer_data, object, boxed, iter->messages );
	}

	/* iter while code is ok */
	return( iter->code != NA_IIO_PROVIDER_CODE_OK );
}

/*
 * na_factory_object_get_as_value:
 * @object: this #NAIFactoryObject instance.
 * @name: the elementary data id.
 * @value: the #GValue to be set.
 *
 * Set the @value with the current content of the #NADataBoxed attached
 * to @name.
 *
 * This is to be read as "set value from data element".
 */
void
na_factory_object_get_as_value( const NAIFactoryObject *object, const gchar *name, GValue *value )
{
	NADataBoxed *boxed;

	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	g_value_unset( value );

	boxed = na_ifactory_object_get_data_boxed( object, name );
	if( boxed ){
		na_boxed_get_as_value( NA_BOXED( boxed ), value );
	}
}

/*
 * na_factory_object_get_as_void:
 * @object: this #NAIFactoryObject instance.
 * @name: the elementary data whose value is to be got.
 *
 * Returns: the searched value.
 *
 * If the type of the value is NA_DATA_TYPE_STRING or NA_DATA_TYPE_LOCALE_STRING
 * (resp. NA_DATA_TYPE_STRING_LIST), then the returned value is a newly allocated
 * string (resp. GSList) and should be g_free() (resp. na_core_utils_slist_free())
 * by the caller.
 */
void *
na_factory_object_get_as_void( const NAIFactoryObject *object, const gchar *name )
{
	void *value;
	NADataBoxed *boxed;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	value = NULL;

	boxed = na_ifactory_object_get_data_boxed( object, name );
	if( boxed ){
		value = na_boxed_get_as_void( NA_BOXED( boxed ));
	}

	return( value );
}

/*
 * na_factory_object_is_set:
 * @object: this #NAIFactoryObject instance.
 * @name: the elementary data whose value is to be tested.
 *
 * Returns: %TRUE if the value is set (may be %NULL), %FALSE else.
 */
gboolean
na_factory_object_is_set( const NAIFactoryObject *object, const gchar *name )
{
	NADataBoxed *boxed;

	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), FALSE );

	boxed = na_ifactory_object_get_data_boxed( object, name );

	return( boxed != NULL );
}

/*
 * na_factory_object_set_from_value:
 * @object: this #NAIFactoryObject instance.
 * @name: the elementary data id.
 * @value: the #GValue whose content is to be got.
 *
 * Get from the @value the content to be set in the #NADataBoxed
 * attached to @property_id.
 */
void
na_factory_object_set_from_value( NAIFactoryObject *object, const gchar *name, const GValue *value )
{
	static const gchar *thisfn = "na_factory_object_set_from_value";

	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	NADataBoxed *boxed = na_ifactory_object_get_data_boxed( object, name );
	if( boxed ){
		na_boxed_set_from_value( NA_BOXED( boxed ), value );

	} else {
		NADataDef *def = na_factory_object_get_data_def( object, name );
		if( !def ){
			g_warning( "%s: unknown NADataDef %s", thisfn, name );

		} else {
			boxed = na_data_boxed_new( def );
			na_boxed_set_from_value( NA_BOXED( boxed ), value );
			attach_boxed_to_object( object, boxed );
		}
	}
}

/*
 * na_factory_object_set_from_void:
 * @object: this #NAIFactoryObject instance.
 * @name: the elementary data whose value is to be set.
 * @data: the value to set.
 *
 * Set the elementary data with the given value.
 */
void
na_factory_object_set_from_void( NAIFactoryObject *object, const gchar *name, const void *data )
{
	static const gchar *thisfn = "na_factory_object_set_from_void";

	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	NADataBoxed *boxed = na_ifactory_object_get_data_boxed( object, name );
	if( boxed ){
		na_boxed_set_from_void( NA_BOXED( boxed ), data );

	} else {
		NADataDef *def = na_factory_object_get_data_def( object, name );
		if( !def ){
			g_warning( "%s: unknown NADataDef %s for %s", thisfn, name, G_OBJECT_TYPE_NAME( object ));

		} else {
			boxed = na_data_boxed_new( def );
			na_boxed_set_from_void( NA_BOXED( boxed ), data );
			attach_boxed_to_object( object, boxed );
		}
	}
}

static NADataGroup *
v_get_groups( const NAIFactoryObject *object )
{
	if( NA_IFACTORY_OBJECT_GET_INTERFACE( object )->get_groups ){
		return( NA_IFACTORY_OBJECT_GET_INTERFACE( object )->get_groups( object ));
	}

	return( NULL );
}

static void
v_copy( NAIFactoryObject *target, const NAIFactoryObject *source )
{
	if( NA_IFACTORY_OBJECT_GET_INTERFACE( target )->copy ){
		NA_IFACTORY_OBJECT_GET_INTERFACE( target )->copy( target, source );
	}
}

static gboolean
v_are_equal( const NAIFactoryObject *a, const NAIFactoryObject *b )
{
	if( NA_IFACTORY_OBJECT_GET_INTERFACE( a )->are_equal ){
		return( NA_IFACTORY_OBJECT_GET_INTERFACE( a )->are_equal( a, b ));
	}

	return( TRUE );
}

static gboolean
v_is_valid( const NAIFactoryObject *object )
{
	if( NA_IFACTORY_OBJECT_GET_INTERFACE( object )->is_valid ){
		return( NA_IFACTORY_OBJECT_GET_INTERFACE( object )->is_valid( object ));
	}

	return( TRUE );
}

static void
v_read_start( NAIFactoryObject *serializable, const NAIFactoryProvider *reader, void *reader_data, GSList **messages )
{
	if( NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->read_start ){
		NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->read_start( serializable, reader, reader_data, messages );
	}
}

static void
v_read_done( NAIFactoryObject *serializable, const NAIFactoryProvider *reader, void *reader_data, GSList **messages )
{
	if( NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->read_done ){
		NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->read_done( serializable, reader, reader_data, messages );
	}
}

static guint
v_write_start( NAIFactoryObject *serializable, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	guint code = NA_IIO_PROVIDER_CODE_OK;

	if( NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->write_start ){
		code = NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->write_start( serializable, writer, writer_data, messages );
	}

	return( code );
}

static guint
v_write_done( NAIFactoryObject *serializable, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	guint code = NA_IIO_PROVIDER_CODE_OK;

	if( NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->write_done ){
		code = NA_IFACTORY_OBJECT_GET_INTERFACE( serializable )->write_done( serializable, writer, writer_data, messages );
	}

	return( code );
}

static void
attach_boxed_to_object( NAIFactoryObject *object, NADataBoxed *boxed )
{
	GList *list = g_object_get_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA );
	list = g_list_prepend( list, boxed );
	g_object_set_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA, list );
}

static void
free_data_boxed_list( NAIFactoryObject *object )
{
	GList *list;

	list = g_object_get_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA );

	g_list_foreach( list, ( GFunc ) g_object_unref, NULL );
	g_list_free( list );

	g_object_set_data( G_OBJECT( object ), NA_IFACTORY_OBJECT_PROP_DATA, NULL );
}

/*
 * the iter function must return TRUE to stops the enumeration
 */
static void
iter_on_data_defs( const NADataGroup *groups, guint mode, NADataDefIterFunc pfn, void *user_data )
{
	static const gchar *thisfn = "na_factory_object_iter_on_data_defs";
	NADataDef *def;
	gboolean stop;

	stop = FALSE;

	while( groups->group && !stop ){

		if( groups->def ){

			def = groups->def;
			while( def->name && !stop ){

				/*g_debug( "serializable_only=%s, def->serializable=%s",
						serializable_only ? "True":"False", def->serializable ? "True":"False" );*/

				switch( mode ){
					case DATA_DEF_ITER_SET_PROPERTIES:
						if( def->has_property ){
							stop = ( *pfn )( def, user_data );
						}
						break;

					case DATA_DEF_ITER_SET_DEFAULTS:
						if( def->default_value ){
							stop = ( *pfn )( def, user_data );
						}
						break;

					case DATA_DEF_ITER_IS_VALID:
						stop = ( *pfn )( def, user_data );
						break;

					case DATA_DEF_ITER_READ_ITEM:
						if( def->readable ){
							stop = ( *pfn )( def, user_data );
						}
						break;

					default:
						g_warning( "%s: unknown mode=%d", thisfn, mode );
				}

				def++;
			}
		}

		groups++;
	}
}
