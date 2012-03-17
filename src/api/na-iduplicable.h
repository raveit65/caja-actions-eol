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

#ifndef __CAJA_ACTIONS_API_NA_IDUPLICABLE_H__
#define __CAJA_ACTIONS_API_NA_IDUPLICABLE_H__

/**
 * SECTION: na_iduplicable
 * @short_description: #NAIDuplicable interface.
 * @include: caja-actions/private/na-iduplicable.h
 *
 * This interface is implemented by #NAObject in order to let
 * #NAObject-derived instance duplication be easily tracked. This works
 * by keeping a pointer on the original object at duplication time, and
 * then only checking edition status when explicitely required.
 *
 * As the reference count of the original object is not incremented
 * here, the caller has to garantee itself that the original object
 * will stay in life at least as long as the duplicated one.
 *
 * Modification status in Caja-Actions configuration tool.
 *
 * - Objects whose origin is NULL are considered as modified ; this is
 *   in particular the case of new, pasted, imported and dropped
 *   objects.
 *
 * - when a new object, whether is is really new or it has been pasted,
 *   imported or dropped, is inserted somewhere in the tree, its
 *   immediate parent is also marked as modified.
 *
 * - Check for edition status, which positions modification and validity
 *   status, is not recursive ; it is the responsability of the
 *   implementation to check for edition status of childs of object..
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_IDUPLICABLE_TYPE							( na_iduplicable_get_type())
#define NA_IDUPLICABLE( instance )					( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_IDUPLICABLE_TYPE, NAIDuplicable ))
#define NA_IS_IDUPLICABLE( instance )				( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_IDUPLICABLE_TYPE ))
#define NA_IDUPLICABLE_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_IDUPLICABLE_TYPE, NAIDuplicableInterface ))

typedef struct NAIDuplicable                 NAIDuplicable;

typedef struct NAIDuplicableInterfacePrivate NAIDuplicableInterfacePrivate;

typedef struct {
	GTypeInterface                 parent;
	NAIDuplicableInterfacePrivate *private;

	/**
	 * copy:
	 * @target: the #NAIDuplicable target of the copy.
	 * @source: the #NAIDuplicable source of the copy
	 *
	 * Copies data from @source to @ŧarget, so that @target becomes an
	 * exact copy of @source.
	 *
	 * Each derived class of the implementation should define this
	 * function to copy its own data. The implementation should take
	 * care itself of calling each function in the class hierarchy,
	 * from topmost base class to most-derived one.
	 */
	void     ( *copy )     ( NAIDuplicable *target, const NAIDuplicable *source );

	/**
	 * are_equal:
	 * @a: a first #NAIDuplicable object.
	 * @b: a second #NAIDuplicable object to be compared to the first
	 * one.
	 *
	 * Compares the two objects.
	 *
	 * Returns: %TRUE if @a and @b are identical, %FALSE else.
	 *
	 * Each derived class of the implementation should define this
	 * function to compare its own data. The implementation should take
	 * care itself of calling each function in the class hierarchy,
	 * from topmost base class to most-derived one.
	 */
	gboolean ( *are_equal )( const NAIDuplicable *a, const NAIDuplicable *b );

	/**
	 * is_valid:
	 * @object: the #NAIDuplicable object to be checked.
	 *
	 * Checks @object for validity.
	 *
	 * Returns: %TRUE if @object is valid, %FALSE else.
	 *
	 * Each derived class of the implementation should define this
	 * function to compare its own data. The implementation should take
	 * care itself of calling each function in the class hierarchy,
	 * from topmost base class to most-derived one.
	 */
	gboolean ( *is_valid )   ( const NAIDuplicable *object );
}
	NAIDuplicableInterface;

#define NA_IDUPLICABLE_SIGNAL_STATUS_CHANGED	"na-iduplicable-status-changed"

GType          na_iduplicable_get_type( void );

void           na_iduplicable_dispose     ( const NAIDuplicable *object );
void           na_iduplicable_dump        ( const NAIDuplicable *object );
NAIDuplicable *na_iduplicable_duplicate   ( const NAIDuplicable *object );
void           na_iduplicable_check_status( const NAIDuplicable *object );

NAIDuplicable *na_iduplicable_get_origin  ( const NAIDuplicable *object );
gboolean       na_iduplicable_is_valid    ( const NAIDuplicable *object );
gboolean       na_iduplicable_is_modified ( const NAIDuplicable *object );

void           na_iduplicable_set_origin  ( NAIDuplicable *object, const NAIDuplicable *origin );
void           na_iduplicable_set_modified( NAIDuplicable *object, gboolean is_modified );

void           na_iduplicable_register_consumer( GObject *consumer );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IDUPLICABLE_H__ */
