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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_H__

/**
 * SECTION: na_object
 * @short_description: #NAObject class definition.
 * @include: caja-actions/na-object.h
 *
 * This is the base class of all our data object hierarchy.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_OBJECT_TYPE					( na_object_object_get_type())
#define NA_OBJECT( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_OBJECT_TYPE, NAObject ))
#define NA_OBJECT_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NA_OBJECT_TYPE, NAObjectClass ))
#define NA_IS_OBJECT( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_OBJECT_TYPE ))
#define NA_IS_OBJECT_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_OBJECT_TYPE ))
#define NA_OBJECT_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_OBJECT_TYPE, NAObjectClass ))

typedef struct NAObjectPrivate      NAObjectPrivate;

typedef struct {
	GObject          parent;
	NAObjectPrivate *private;
}
	NAObject;

typedef struct NAObjectClassPrivate NAObjectClassPrivate;

typedef struct {
	GObjectClass          parent;
	NAObjectClassPrivate *private;

	/**
	 * dump:
	 * @object: the #NAObject-derived object to be dumped.
	 *
	 * Dumps via g_debug the content of the object.
	 *
	 * #NAObject class takes care of calling this function for each
	 * derived class, starting from topmost base class up to most-
	 * derived one. Each derived class has so only to take care of
	 * dumping its own data.
	 */
	void     ( *dump )     ( const NAObject *object );

	/**
	 * copy:
	 * @target: the #NAObject-derived object which will receive data.
	 * @source: the #NAObject-derived object which will provide data.
	 * @recursive: whether children should be recursively copied.
	 *
	 * Copies data and properties from @source to @target.
	 *
	 * Each derived class should take care of implementing this function
	 * when relevant. #NAObject class will take care of calling this
	 * function for each class of the hierarchy, starting from topmost
	 * base class up to the most-derived one. Each class has so only to
	 * take care of dumping its own data.
	 */
	void     ( *copy )     ( NAObject *target, const NAObject *source, gboolean recursive );

	/**
	 * are_equal:
	 * @a: a first #NAObject object.
	 * @b: a second #NAObject object to be compared to the first one.
	 *
	 * Compares the two objects.
	 *
	 * Returns: %TRUE if @a and @b are identical, %FALSE else.
	 *
	 * Each derived class should take care of implementing this function
	 * when relevant. #NAObject class will take care of calling this
	 * function for each class of the hierarchy, starting from topmost
	 * base class up to the most-derived one, at least while result
	 * stays at %TRUE.
	 * As soon as a difference is detected, the calling sequence will
	 * be stopped, and the result returned.
	 */
	gboolean ( *are_equal )( const NAObject *a, const NAObject *b );

	/**
	 * is_valid:
	 * @object: the #NAObject object to be checked.
	 *
	 * Checks @object for validity.
	 *
	 * Returns: %TRUE if @object is valid, %FALSE else.
	 *
	 * A #NAObject is valid if its internal identifiant is set.
	 *
	 * Each derived class should take care of implementing this function
	 * when relevant. #NAObject class will take care of calling this
	 * function for each class of the hierarchy, starting from topmost
	 * base class up to the most-derived one, at least while result
	 * stays at %TRUE.
	 * As soon as a difference is detected, the calling sequence will
	 * be stopped, and the result returned.
	 */
	gboolean ( *is_valid ) ( const NAObject *object );
}
	NAObjectClass;

GType     na_object_object_get_type( void );

void      na_object_object_check_status   ( const NAObject *object );
gboolean  na_object_object_check_status_up( const NAObject *object );

void      na_object_object_reset_origin   ( NAObject *object, const NAObject *origin );

NAObject *na_object_object_ref  ( NAObject *object );
void      na_object_object_unref( NAObject *object );

void      na_object_object_copy      ( NAObject *target, const NAObject *source, gboolean recursive );

void      na_object_object_dump      ( const NAObject *object );
void      na_object_object_dump_norec( const NAObject *object );
void      na_object_object_dump_tree ( GList *tree );

GList    *na_object_object_get_hierarchy( const NAObject *object );
void      na_object_free_hierarchy( GList *hierarchy );

void      na_object_object_debug_invalid( const NAObject *object, const gchar *reason );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_H__ */
