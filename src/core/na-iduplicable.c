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

#include <api/na-iduplicable.h>

/* private interface data
 */
struct NAIDuplicableInterfacePrivate {
	GList *consumers;
};

/* the data sructure set on each NAIDuplicable object
 */
typedef struct {
	NAIDuplicable *origin;
	gboolean       modified;
	gboolean       valid;
	gulong         status_changed_handler_id;
}
	DuplicableStr;

#define NA_IDUPLICABLE_DATA_DUPLICABLE			"na-iduplicable-data-duplicable"

/* signals emitted on NAIDuplicable when a status changes
 */
enum {
	STATUS_CHANGED,
	LAST_SIGNAL
};

static NAIDuplicableInterface *st_interface = NULL;
static gboolean                st_initialized = FALSE;
static gboolean                st_finalized = FALSE ;
static gint                    st_signals[ LAST_SIGNAL ] = { 0 };

static GType          register_type( void );
static void           interface_base_init( NAIDuplicableInterface *klass );
static void           interface_base_finalize( NAIDuplicableInterface *klass );

static void           v_copy( NAIDuplicable *target, const NAIDuplicable *source );
static gboolean       v_are_equal( const NAIDuplicable *a, const NAIDuplicable *b );
static gboolean       v_is_valid( const NAIDuplicable *object );

static DuplicableStr *get_duplicable_str( const NAIDuplicable *object );

static void           status_changed_handler( NAIDuplicable *instance, gpointer user_data );
static void           propagate_signal_to_consumers( const gchar *signal, NAIDuplicable *instance, gpointer user_data );
static void           release_signal_consumers( GList *consumers );

GType
na_iduplicable_get_type( void )
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
	static const gchar *thisfn = "na_iduplicable_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIDuplicableInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIDuplicable", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIDuplicableInterface *klass )
{
	static const gchar *thisfn = "na_iduplicable_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NAIDuplicableInterfacePrivate, 1 );

		klass->private->consumers = NULL;

		klass->copy = NULL;
		klass->are_equal = NULL;
		klass->is_valid = NULL;

		/**
		 * na-iduplicable-status-changed:
		 *
		 * This signal is emitted by NAIDuplicable when the modification
		 * or the validity status of an object has been modified.
		 */
		st_signals[ STATUS_CHANGED ] = g_signal_new(
				NA_IDUPLICABLE_SIGNAL_STATUS_CHANGED,
				G_TYPE_OBJECT,
				G_SIGNAL_RUN_LAST,
				0,						/* no default handler */
				NULL,
				NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE,
				1,
				G_TYPE_POINTER );

		st_interface = klass;

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NAIDuplicableInterface *klass )
{
	static const gchar *thisfn = "na_iduplicable_interface_base_finalize";

	if( !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		release_signal_consumers( klass->private->consumers );

		g_free( klass->private );
	}
}

/**
 * na_iduplicable_dispose:
 * @object: the #NAIDuplicable object to be initialized.
 *
 * Releases resources.
 */
void
na_iduplicable_dispose( const NAIDuplicable *object )
{
	DuplicableStr *str;

	g_return_if_fail( NA_IS_IDUPLICABLE( object ));

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );

		if( g_signal_handler_is_connected(( gpointer ) object, str->status_changed_handler_id )){
			g_signal_handler_disconnect(( gpointer ) object, str->status_changed_handler_id );
		}

		g_free( str );
	}
}

/**
 * na_iduplicable_dump:
 * @object: the #NAIDuplicable object to be dumped.
 *
 * Dumps via g_debug the properties of the object.
 *
 * We ouput here only the data we set ourselves againt the
 * #NAIDuplicable-implemented object.
 *
 * This function should be called by the implementation when it dumps
 * itself its own content.
 */
void
na_iduplicable_dump( const NAIDuplicable *object )
{
	static const gchar *thisfn = "na_iduplicable_dump";
	DuplicableStr *str;

	g_return_if_fail( NA_IS_IDUPLICABLE( object ));

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );

		g_debug( "%s:   origin=%p", thisfn, ( void * ) str->origin );
		g_debug( "%s: modified=%s", thisfn, str->modified ? "True" : "False" );
		g_debug( "%s:    valid=%s", thisfn, str->valid ? "True" : "False" );
	}
}

/**
 * na_iduplicable_duplicate:
 * @object: the #NAIDuplicable object to be duplicated.
 *
 * Exactly duplicates a #NAIDuplicable-implemented object, including
 * modification and validity status which are copied from @object to
 * the duplicated one.
 *
 * Though this function is not recursive by itself, it is widely supposed
 * everywhere in the program that recursivity is provided but #NAObject
 * implementation.
 *
 * +------------------------------------------------------------------------------+
 * | na_object_duplicate (aka na_iduplicable_duplicate) is definitively recursive |
 * +------------------------------------------------------------------------------+
 *
 * Returns: a new #NAIDuplicable.
 */
NAIDuplicable *
na_iduplicable_duplicate( const NAIDuplicable *object )
{
	static const gchar *thisfn = "na_iduplicable_duplicate";
	NAIDuplicable *dup;
	DuplicableStr *dup_str, *obj_str;

	g_debug( "%s: object=%p (%s)",
			thisfn,
			( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_val_if_fail( NA_IS_IDUPLICABLE( object ), NULL );

	dup = NULL;

	if( st_initialized && !st_finalized ){

		dup = g_object_new( G_OBJECT_TYPE( object ), NULL );

		v_copy( dup, object );

		dup_str = get_duplicable_str( dup );
		obj_str = get_duplicable_str( object );

		dup_str->origin = ( NAIDuplicable * ) object;
		dup_str->modified = obj_str->modified;
		dup_str->valid = obj_str->valid;
	}

	return( dup );
}

/**
 * na_iduplicable_check_status:
 * @object: the #NAIDuplicable object to be checked.
 *
 * Checks the edition status of the #NAIDuplicable object, and set up
 * the corresponding properties.
 *
 * This function is supposed to be called each time the object may have
 * been modified in order to set the corresponding properties. Helper
 * functions na_iduplicable_is_modified() and na_iduplicable_is_valid()
 * will then only return the current value of the properties.
 *
 * #na_iduplicable_check_status() is not, as itself, recursive.
 * That is, the modification and validity status are only set on the
 * specified object.
 * #NAObject implementation has choosen to handle itself the recursivity:
 * #na_object_check_status() so first check status for childs, before
 * calling this function.
 */
void
na_iduplicable_check_status( const NAIDuplicable *object )
{
	static const gchar *thisfn = "na_iduplicable_check_status";
	DuplicableStr *str;
	gboolean was_modified, was_valid;
	gboolean changed;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NA_IS_IDUPLICABLE( object ));

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );

		was_modified = str->modified;
		was_valid = str->valid;
		changed = FALSE;

		if( str->origin ){
			str->modified = !v_are_equal( str->origin, object );
		} else {
			str->modified = TRUE;
		}

		str->valid = v_is_valid( object );

		if( was_modified && !str->modified ){
			g_debug( "%s: %p (%s) status changed to non-modified",
					thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
			changed = TRUE;
		} else if ( !was_modified && str->modified ){
			g_debug( "%s: %p (%s) status changed to modified",
					thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
			changed = TRUE;
		}

		if( was_valid && !str->valid ){
			g_debug( "%s: %p (%s) status changed to non-valid",
					thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
			changed = TRUE;
		} else if( !was_valid && str->valid ){
			g_debug( "%s: %p (%s) status changed to valid",
					thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
			changed = TRUE;
		}

		if( changed ){
			g_signal_emit_by_name( G_OBJECT( object ), NA_IDUPLICABLE_SIGNAL_STATUS_CHANGED, object );
		}

#if 0
		g_debug( "%s: object=%p (%s), modified=%s, valid=%s", thisfn,
				( void * ) object, G_OBJECT_TYPE_NAME( object ),
				modified ? "True":"False", valid ? "True":"False" );
#endif
	}
}

/**
 * na_iduplicable_get_origin:
 * @object: the #NAIDuplicable object whose origin is to be returned.
 *
 * Returns the origin of a duplicated #NAIDuplicable.
 *
 * Returns: the original #NAIDuplicable, or NULL.
 */
NAIDuplicable *
na_iduplicable_get_origin( const NAIDuplicable *object )
{
	NAIDuplicable *origin;
	DuplicableStr *str;

	g_return_val_if_fail( NA_IS_IDUPLICABLE( object ), NULL );

	origin = NULL;

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );
		origin = str->origin;
	}

	return( origin );
}

/**
 * na_iduplicable_is_valid:
 * @object: the #NAIDuplicable object whose status is to be returned.
 *
 * Returns the current value of the relevant property
 * without rechecking the edition status itself.
 *
 * Returns: %TRUE is the provided object is valid.
 */
gboolean
na_iduplicable_is_valid( const NAIDuplicable *object )
{
	gboolean is_valid;
	DuplicableStr *str;

	g_return_val_if_fail( NA_IS_IDUPLICABLE( object ), FALSE );

	is_valid = FALSE;

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );
		is_valid = str->valid;
	}

	return( is_valid );
}

/**
 * na_iduplicable_is_modified:
 * @object: the #NAIDuplicable object whose status is to be returned.
 *
 * Returns the current value of the %PROP_IDUPLICABLE_ISMODIFIED
 * property without rechecking the edition status itself.
 *
 * Returns: %TRUE is the provided object has been modified regarding of
 * the original one.
 */
gboolean
na_iduplicable_is_modified( const NAIDuplicable *object )
{
	gboolean is_modified;
	DuplicableStr *str;

	g_return_val_if_fail( NA_IS_IDUPLICABLE( object ), FALSE );

	is_modified = FALSE;

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );
		is_modified = str->modified;
	}

	return( is_modified );
}

/**
 * na_iduplicable_set_origin:
 * @object: the #NAIDuplicable object whose origin is to be set.
 * @origin: the new original #NAIDuplicable.
 *
 * Sets the new origin of a duplicated #NAIDuplicable.
 */
void
na_iduplicable_set_origin( NAIDuplicable *object, const NAIDuplicable *origin )
{
	DuplicableStr *str;

	g_return_if_fail( NA_IS_IDUPLICABLE( object ));
	g_return_if_fail( NA_IS_IDUPLICABLE( origin ) || !origin );

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );
		str->origin = ( NAIDuplicable * ) origin;
	}
}

/**
 * na_iduplicable_set_modified:
 * @object: the #NAIDuplicable object whose modification status is to be set.
 * @modified: the new modification status #NAIDuplicable.
 *
 * Sets the new modified of a duplicated #NAIDuplicable.
 */
void
na_iduplicable_set_modified( NAIDuplicable *object, gboolean modified )
{
	DuplicableStr *str;

	g_return_if_fail( NA_IS_IDUPLICABLE( object ));

	if( st_initialized && !st_finalized ){

		str = get_duplicable_str( object );

		if( str->modified != modified ){
			str->modified = modified;
			g_signal_emit_by_name( G_OBJECT( object ), NA_IDUPLICABLE_SIGNAL_STATUS_CHANGED, object );
		}
	}
}

static void
v_copy( NAIDuplicable *target, const NAIDuplicable *source )
{
	if( NA_IDUPLICABLE_GET_INTERFACE( target )->copy ){
		NA_IDUPLICABLE_GET_INTERFACE( target )->copy( target, source );
	}
}

static gboolean
v_are_equal( const NAIDuplicable *a, const NAIDuplicable *b )
{
	if( NA_IDUPLICABLE_GET_INTERFACE( a )->are_equal ){
		return( NA_IDUPLICABLE_GET_INTERFACE( a )->are_equal( a, b ));
	}

	return( FALSE );
}

static gboolean
v_is_valid( const NAIDuplicable *object )
{
	if( NA_IDUPLICABLE_GET_INTERFACE( object )->is_valid ){
		return( NA_IDUPLICABLE_GET_INTERFACE( object )->is_valid( object ));
	}

	return( FALSE );
}

/**
 * na_iduplicable_register_consumer:
 * @consumer: the target instance.
 *
 * This function registers a consumer, i.e. an instance to which edition
 * status signals will be propagated.
 */
void
na_iduplicable_register_consumer( GObject *consumer )
{
	if( st_initialized && !st_finalized ){

		g_return_if_fail( st_interface );
		g_debug( "na_iduplicable_register_consumer: consumer=%p", ( void * ) consumer );
		st_interface->private->consumers = g_list_prepend( st_interface->private->consumers, consumer );
	}
}

static void
status_changed_handler( NAIDuplicable *instance, gpointer user_data )
{
	/*g_debug( "na_iduplicable_propagate_modified_changed: instance=%p (%s), user_data=%p (%s)",
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data, G_OBJECT_TYPE_NAME( user_data ));*/

	propagate_signal_to_consumers( NA_IDUPLICABLE_SIGNAL_STATUS_CHANGED, instance, user_data );
}

/*
 * note that propagating the signal to consumers re-triggers
 */
static void
propagate_signal_to_consumers( const gchar *signal, NAIDuplicable *instance, gpointer user_data )
{
	static const gchar *thisfn = "na_iduplicable_propagate_signals_to_consumers";
	GList *ic;

	g_return_if_fail( st_interface );

	if( st_initialized && !st_finalized ){

		g_debug( "%s: signal=%s, instance=%p, user_data=%p", thisfn, signal, ( void * ) instance, ( void * ) user_data );

		for( ic = st_interface->private->consumers ; ic ; ic = ic->next ){
			g_signal_emit_by_name( ic->data, signal, user_data );
		}
	}
}

static void
release_signal_consumers( GList *consumers )
{
	g_list_free( consumers );
}

static DuplicableStr *
get_duplicable_str( const NAIDuplicable *object )
{
	DuplicableStr *str;

	str = ( DuplicableStr * ) g_object_get_data( G_OBJECT( object ), NA_IDUPLICABLE_DATA_DUPLICABLE );

	if( !str ){
		str = g_new0( DuplicableStr, 1 );

		str->origin = NULL;
		str->modified = FALSE;
		str->valid = TRUE;

		str->status_changed_handler_id = g_signal_connect(
				G_OBJECT( object ),
				NA_IDUPLICABLE_SIGNAL_STATUS_CHANGED,
				G_CALLBACK( status_changed_handler ),
				( gpointer ) object );

		g_object_set_data( G_OBJECT( object ), NA_IDUPLICABLE_DATA_DUPLICABLE, str );
	}

	return( str );
}
