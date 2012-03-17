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

#ifndef __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_H__
#define __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_H__

/**
 * SECTION: na_ifactory_object
 * @short_description: #NAIFactoryObject interface definition.
 * @include: caja-actions/na-ifactory_object.h
 *
 * This interface must be implemented by #NAObject-derived objects which
 * should take advantage of data factory management system.
 *
 * A #NAObject-derived which should implement this #NAIFactoryObject
 * interface must meet following conditions:
 * - must accept an empty constructor
 *
 * Elementary data are implemented as a GList of NADataBoxed objects.
 *
 * Caja-Actions v 2.30 - API version:  1
 */

#include "na-data-def.h"
#include "na-data-boxed.h"
#include "na-ifactory-provider-provider.h"

G_BEGIN_DECLS

#define NA_IFACTORY_OBJECT_TYPE							( na_ifactory_object_get_type())
#define NA_IFACTORY_OBJECT( instance )					( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_IFACTORY_OBJECT_TYPE, NAIFactoryObject ))
#define NA_IS_IFACTORY_OBJECT( instance )				( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_IFACTORY_OBJECT_TYPE ))
#define NA_IFACTORY_OBJECT_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_IFACTORY_OBJECT_TYPE, NAIFactoryObjectInterface ))

typedef struct NAIFactoryObject                 NAIFactoryObject;

typedef struct NAIFactoryObjectInterfacePrivate NAIFactoryObjectInterfacePrivate;

typedef struct {
	GTypeInterface                    parent;
	NAIFactoryObjectInterfacePrivate *private;

	/**
	 * get_version:
	 * @instance: this #NAIFactoryObject instance.
	 *
	 * Returns: the version of this interface supported by @instance implementation.
	 *
	 * Defaults to 1.
	 */
	guint         ( *get_version )( const NAIFactoryObject *instance );

	/**
	 * get_groups:
	 * @instance: this #NAIFactoryObject instance.
	 *
	 * Returns: a pointer to the NADataGroup which defines this object.
	 */
	NADataGroup * ( *get_groups ) ( const NAIFactoryObject *instance );

	/**
	 * copy:
	 * @instance: the target #NAIFactoryObject instance.
	 * @source: the source #NAIFactoryObject instance.
	 *
	 * This function is triggered after having copied @source to
	 * @instance target. This later may take advantage of this call
	 * to do some particular copy tasks.
	 */
	void          ( *copy )       ( NAIFactoryObject *instance, const NAIFactoryObject *source );

	/**
	 * are_equal:
	 * @a: the first #NAIFactoryObject instance.
	 * @b: the second #NAIFactoryObject instance.
	 *
	 * Returns: %TRUE if @a is equal to @b.
	 *
	 * This function is triggered after all elementary data comparisons
	 * have been sucessfully made.
	 */
	gboolean      ( *are_equal )  ( const NAIFactoryObject *a, const NAIFactoryObject *b );

	/**
	 * is_valid:
	 * @object: the #NAIFactoryObject instance whose validity is to be checked.
	 *
	 * Returns: %TRUE if @object is valid.
	 *
	 * This function is triggered after all elementary data comparisons
	 * have been sucessfully made.
	 */
	gboolean      ( *is_valid )   ( const NAIFactoryObject *object );

	/**
	 * read_start:
	 * @instance: this #NAIFactoryObject instance.
	 * @reader: the instance which has provided read services.
	 * @reader_data: the data associated to @reader.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called just before the object is unserialized.
	 */
	void          ( *read_start ) ( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );

	/**
	 * read_done:
	 * @instance: this #NAIFactoryObject instance.
	 * @reader: the instance which has provided read services.
	 * @reader_data: the data associated to @reader.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called when the object has been unserialized.
	 */
	void          ( *read_done )  ( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );

	/**
	 * write_start:
	 * @instance: this #NAIFactoryObject instance.
	 * @writer: the instance which has provided writing services.
	 * @writer_data: the data associated to @writer.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called just before the object is serialized.
	 *
	 * Returns: a NAIIOProvider operation return code.
	 */
	guint         ( *write_start )( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );

	/**
	 * write_done:
	 * @instance: this #NAIFactoryObject instance.
	 * @writer: the instance which has provided writing services.
	 * @writer_data: the data associated to @writer.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called when the object has been serialized.
	 *
	 * Returns: a NAIIOProvider operation return code.
	 */
	guint         ( *write_done ) ( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );
}
	NAIFactoryObjectInterface;

GType        na_ifactory_object_get_type( void );

NADataGroup *na_ifactory_object_get_data_groups( const NAIFactoryObject *object );

NADataBoxed *na_ifactory_object_get_data_boxed ( const NAIFactoryObject *object, const gchar *name );

void        *na_ifactory_object_get_as_void    ( const NAIFactoryObject *object, const gchar *name );

void         na_ifactory_object_set_from_void  ( NAIFactoryObject *object, const gchar *name, const void *data );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_H__ */
