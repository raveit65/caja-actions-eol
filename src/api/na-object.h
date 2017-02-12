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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_H__

/**
 * SECTION: object
 * @title: NAObject
 * @short_description: The Deepest Base Class Definition
 * @include: caja-actions/na-object.h
 *
 * This is the base class of all our data object hierarchy. #NAObject is
 * supposed to be used as a pure virtual base class, i.e. should only be
 * derived.
 *
 * All the API described here is rather private. External code should
 * use the API described in <filename>caja-actions/na-object-api.h</filename>.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_TYPE_OBJECT                ( na_object_object_get_type())
#define NA_OBJECT( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_OBJECT, NAObject ))
#define NA_OBJECT_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_OBJECT, NAObjectClass ))
#define NA_IS_OBJECT( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_OBJECT ))
#define NA_IS_OBJECT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_OBJECT ))
#define NA_OBJECT_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_OBJECT, NAObjectClass ))

typedef struct _NAObjectPrivate       NAObjectPrivate;

typedef struct {
	/*< private >*/
	GObject          parent;
	NAObjectPrivate *private;
}
	NAObject;

typedef struct _NAObjectClassPrivate  NAObjectClassPrivate;

/**
 * NAObjectClass:
 * @dump:      Dumps the #NAObject -part of the #NAObject -derived object.
 * @copy:      Copies a #NAObject to another.
 * @are_equal: Tests if two #NAObject are equal.
 * @is_valid:  Tests if a #NAObject is valid.
 *
 * The #NAObjectClass defines some methods available to derived classes.
 */
typedef struct {
	/*< private >*/
	GObjectClass          parent;
	NAObjectClassPrivate *private;

	/*< public >*/
	/**
	 * dump:
	 * @object: the NAObject-derived object to be dumped.
	 *
	 * Dumps via g_debug the content of the object.
	 *
	 * The derived class should call its parent class at the end of the
	 * dump of its own datas.
	 *
	 * Since: 2.30
	 */
	void     ( *dump )     ( const NAObject *object );

	/**
	 * copy:
	 * @target: the NAObject-derived object which will receive data.
	 * @source: the NAObject-derived object which provides data.
	 * @mode: the copy mode.
	 *
	 * Copies data and properties from @source to @target.
	 *
	 * The derived class should call its parent class at the end of the
	 * copy of its own datas.
	 *
	 * Since: 2.30
	 */
	void     ( *copy )     ( NAObject *target, const NAObject *source, guint mode );

	/**
	 * are_equal:
	 * @a: a first NAObject object.
	 * @b: a second NAObject object to be compared to the first one.
	 *
	 * Compares the two objects.
	 *
	 * When testing for the modification status of an object, @a stands for
	 * the original object, while @b stands for the duplicated one.
	 *
	 * As long as no difference is detected, the derived class should call
	 * its parent class at the end of its comparison.
	 * As soon as a difference is detected, the calling sequence should
	 * be stopped, and the result returned.
	 *
	 * Returns: TRUE if @a and @b are identical, FALSE else.
	 *
	 * Since: 2.30
	 */
	gboolean ( *are_equal )( const NAObject *a, const NAObject *b );

	/**
	 * is_valid:
	 * @object: the NAObject object to be checked.
	 *
	 * Checks @object for validity.
	 *
	 * A NAObject is valid if its internal identifier is set.
	 *
	 * As long as the item is valid, the derived class should call its parent
	 * at the end of its checks.
	 * As soon as an error is detected, the calling sequence should be stopped,
	 * and the result returned.
	 *
	 * Returns: TRUE if @object is valid, FALSE else.
	 *
	 * Since: 2.30
	 */
	gboolean ( *is_valid ) ( const NAObject *object );
}
	NAObjectClass;

GType     na_object_object_get_type( void );

void      na_object_object_check_status_rec( const NAObject *object );

void      na_object_object_reset_origin   ( NAObject *object, const NAObject *origin );

NAObject *na_object_object_ref  ( NAObject *object );
void      na_object_object_unref( NAObject *object );

void      na_object_object_dump      ( const NAObject *object );
void      na_object_object_dump_norec( const NAObject *object );
void      na_object_object_dump_tree ( GList *tree );

#ifdef NA_ENABLE_DEPRECATED
GList    *na_object_get_hierarchy( const NAObject *object );
void      na_object_free_hierarchy( GList *hierarchy );
#endif

void      na_object_object_debug_invalid( const NAObject *object, const gchar *reason );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_H__ */
