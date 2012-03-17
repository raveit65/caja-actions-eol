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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_MENU_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_MENU_H__

/**
 * SECTION: na_object_menu
 * @short_description: #NAObjectMenu class definition.
 * @include: caja-actions/na-object-menu.h
 */

#include "na-object-item.h"

G_BEGIN_DECLS

#define NA_OBJECT_MENU_TYPE					( na_object_menu_get_type())
#define NA_OBJECT_MENU( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, NA_OBJECT_MENU_TYPE, NAObjectMenu ))
#define NA_OBJECT_MENU_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NA_OBJECT_MENU_TYPE, NAObjectMenuClass ))
#define NA_IS_OBJECT_MENU( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_OBJECT_MENU_TYPE ))
#define NA_IS_OBJECT_MENU_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_OBJECT_MENU_TYPE ))
#define NA_OBJECT_MENU_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_OBJECT_MENU_TYPE, NAObjectMenuClass ))

typedef struct NAObjectMenuPrivate      NAObjectMenuPrivate;

typedef struct {
	NAObjectItem         parent;
	NAObjectMenuPrivate *private;
}
	NAObjectMenu;

typedef struct NAObjectMenuClassPrivate NAObjectMenuClassPrivate;

typedef struct {
	NAObjectItemClass         parent;
	NAObjectMenuClassPrivate *private;
}
	NAObjectMenuClass;

GType         na_object_menu_get_type( void );

NAObjectMenu *na_object_menu_new( void );
NAObjectMenu *na_object_menu_new_with_defaults( void );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_MENU_H__ */
