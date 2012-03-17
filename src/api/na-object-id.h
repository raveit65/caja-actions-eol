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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_ID_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_ID_H__

/**
 * SECTION: na_object_id
 * @short_description: #NAObjectId class definition.
 * @include: caja-actions/na-object-id.h
 *
 * This is a pure virtual class, i.e. not an instantiatable one.
 * It serves as the base class for #NAObject-derived object which have
 * a unique Id, i.e. for #NAObjectItem and #NAObjectProfile.
 */

#include "na-object.h"

G_BEGIN_DECLS

#define NA_OBJECT_ID_TYPE					( na_object_id_get_type())
#define NA_OBJECT_ID( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_OBJECT_ID_TYPE, NAObjectId ))
#define NA_OBJECT_ID_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, NA_OBJECT_ID_TYPE, NAObjectIdClass ))
#define NA_IS_OBJECT_ID( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_OBJECT_ID_TYPE ))
#define NA_IS_OBJECT_ID_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_OBJECT_ID_TYPE ))
#define NA_OBJECT_ID_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_OBJECT_ID_TYPE, NAObjectIdClass ))

typedef struct NAObjectIdPrivate      NAObjectIdPrivate;

typedef struct {
	NAObject           parent;
	NAObjectIdPrivate *private;
}
	NAObjectId;

typedef struct NAObjectIdClassPrivate NAObjectIdClassPrivate;

typedef struct {
	NAObjectClass           parent;
	NAObjectIdClassPrivate *private;

	/**
	 * new_id:
	 * @object: a #NAObjectId object.
	 * @new_parent: possibly the new #NAObjectId parent, or NULL.
	 * If not NULL, this should actually be a #NAObjectItem.
	 *
	 * Returns: a new id suitable for this @object.
	 *
	 * If @object is a #NAObjectProfile, then @new_parent must be a
	 * not null #NAObjectAction. This function ensures that the new
	 * profile name does not already exist in the given @new_parent.
	 *
	 * This is a pure virtual function which should be implemented by
	 * the actual class. Actually, we asks for the most-derived class
	 * which implements this function.
	 */
	gchar * ( *new_id )( const NAObjectId *object, const NAObjectId *new_parent );
}
	NAObjectIdClass;

GType  na_object_id_get_type( void );

gint   na_object_id_sort_alpha_asc ( const NAObjectId *a, const NAObjectId *b );
gint   na_object_id_sort_alpha_desc( const NAObjectId *a, const NAObjectId *b );

void   na_object_id_prepare_for_paste( NAObjectId *object, gboolean relabel, gboolean renumber, NAObjectId *action );
void   na_object_id_set_copy_of_label( NAObjectId *object );
void   na_object_id_set_new_id       ( NAObjectId *object, const NAObjectId *new_parent );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_ID_H__ */
