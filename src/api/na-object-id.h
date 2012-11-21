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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_ID_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_ID_H__

/**
 * SECTION: object-id
 * @title: NAObjectId
 * @short_description: The Identified Object Base Class Definition
 * @include: caja-actions/na-object-id.h
 *
 * This is a pure virtual class, i.e. not an instantiatable one.
 * It serves as the base class for #NAObject -derived object which have
 * a unique Id, i.e. for #NAObjectItem and #NAObjectProfile.
 */

#include "na-object.h"

G_BEGIN_DECLS

#define NA_TYPE_OBJECT_ID                ( na_object_id_get_type())
#define NA_OBJECT_ID( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_OBJECT_ID, NAObjectId ))
#define NA_OBJECT_ID_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_OBJECT_ID, NAObjectIdClass ))
#define NA_IS_OBJECT_ID( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_OBJECT_ID ))
#define NA_IS_OBJECT_ID_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_OBJECT_ID ))
#define NA_OBJECT_ID_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_OBJECT_ID, NAObjectIdClass ))

typedef struct _NAObjectIdPrivate        NAObjectIdPrivate;

typedef struct {
	/*< private >*/
	NAObject           parent;
	NAObjectIdPrivate *private;
}
	NAObjectId;

typedef struct _NAObjectIdClassPrivate   NAObjectIdClassPrivate;

/**
 * NAObjectIdClass:
 * @new_id: Allocate a new id to an existing NAObjectId.
 *
 * The #NAObjectIdClass defines some methods available to derived classes.
 */
typedef struct {
	/*< private >*/
	NAObjectClass           parent;
	NAObjectIdClassPrivate *private;

	/*< public >*/
	/**
	 * new_id:
	 * @object: a NAObjectId object.
	 * @new_parent: possibly the new NAObjectId parent, or NULL.
	 * If not NULL, this should actually be a NAObjectItem.
	 *
	 * If @object is a NAObjectProfile, then @new_parent must be a
	 * not null NAObjectAction. This function ensures that the new
	 * profile name does not already exist in the given @new_parent.
	 *
	 * This is a pure virtual function which should be implemented by
	 * the actual class. Actually, we asks for the most-derived class
	 * which implements this function.
	 *
	 * Returns: a new id suitable for this @object.
	 *
	 * Since: 2.30
	 */
	gchar * ( *new_id )( const NAObjectId *object, const NAObjectId *new_parent );
}
	NAObjectIdClass;

GType  na_object_id_get_type( void );

gint   na_object_id_sort_alpha_asc   ( const NAObjectId *a, const NAObjectId *b );
gint   na_object_id_sort_alpha_desc  ( const NAObjectId *a, const NAObjectId *b );

void   na_object_id_prepare_for_paste( NAObjectId *object, gboolean relabel, gboolean renumber, NAObjectId *parent );
void   na_object_id_set_copy_of_label( NAObjectId *object );
void   na_object_id_set_new_id       ( NAObjectId *object, const NAObjectId *new_parent );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_ID_H__ */
