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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_ACTION_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_ACTION_H__

/**
 * SECTION: object-action
 * @title: NAObjectAction
 * @short_description: The Action Class Definition
 * @include: caja-actions/na-object-action.h
 *
 * This is the class which maintains data and properties of a &prodname;
 * action.
 *
 * <note>
 *   <formalpara>
 *     <title>Edition status</title>
 *     <para>
 *       As a particular rule for a #NAObjectItem -derived class,
 *       a #NAObjectAction is considered modified as soon as any of
 *       its profiles has been modified itself
 *       (because they are saved as a whole).
 *     </para>
 *   </formalpara>
 * </note>
 */

#include "na-object-item.h"
#include "na-object-profile.h"

G_BEGIN_DECLS

#define NA_TYPE_OBJECT_ACTION                ( na_object_action_get_type())
#define NA_OBJECT_ACTION( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_OBJECT_ACTION, NAObjectAction ))
#define NA_OBJECT_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_OBJECT_ACTION, NAObjectActionClass ))
#define NA_IS_OBJECT_ACTION( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_OBJECT_ACTION ))
#define NA_IS_OBJECT_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_OBJECT_ACTION ))
#define NA_OBJECT_ACTION_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_OBJECT_ACTION, NAObjectActionClass ))

typedef struct _NAObjectActionPrivate        NAObjectActionPrivate;

typedef struct {
	/*< private >*/
	NAObjectItem           parent;
	NAObjectActionPrivate *private;
}
	NAObjectAction;

typedef struct _NAObjectActionClassPrivate   NAObjectActionClassPrivate;

typedef struct {
	/*< private >*/
	NAObjectItemClass           parent;
	NAObjectActionClassPrivate *private;
}
	NAObjectActionClass;

GType na_object_action_get_type( void );

NAObjectAction *na_object_action_new( void );
NAObjectAction *na_object_action_new_with_profile( void );
NAObjectAction *na_object_action_new_with_defaults( void );

gchar          *na_object_action_get_new_profile_name( const NAObjectAction *action );

void            na_object_action_attach_profile( NAObjectAction *action, NAObjectProfile *profile );
void            na_object_action_set_last_version( NAObjectAction *action );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_ACTION_H__ */
