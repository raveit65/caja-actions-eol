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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_ACTION_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_ACTION_H__

/**
 * SECTION: na_object_action
 * @short_description: #NAObjectAction class definition.
 * @include: caja-actions/na-object-action.h
 *
 * This is the class which maintains data and properties of a Caja
 * action.
 *
 * Note about edition status:
 * As a particular rule of #NAItem derived class, a #NAObjectAction is
 * considered modified as soon as any of its profiles has been modified
 * itself (because they are saved as a whole).
 */

#include "na-object-item.h"
#include "na-object-profile.h"

G_BEGIN_DECLS

#define NA_OBJECT_ACTION_TYPE					( na_object_action_get_type())
#define NA_OBJECT_ACTION( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_OBJECT_ACTION_TYPE, NAObjectAction ))
#define NA_OBJECT_ACTION_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, NA_OBJECT_ACTION_TYPE, NAObjectActionClass ))
#define NA_IS_OBJECT_ACTION( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_OBJECT_ACTION_TYPE ))
#define NA_IS_OBJECT_ACTION_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_OBJECT_ACTION_TYPE ))
#define NA_OBJECT_ACTION_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_OBJECT_ACTION_TYPE, NAObjectActionClass ))

typedef struct NAObjectActionPrivate      NAObjectActionPrivate;

typedef struct {
	NAObjectItem           parent;
	NAObjectActionPrivate *private;
}
	NAObjectAction;

typedef struct NAObjectActionClassPrivate NAObjectActionClassPrivate;

typedef struct {
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

gboolean        na_object_action_is_candidate( const NAObjectAction *action, guint target, GList *selection );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_ACTION_H__ */
