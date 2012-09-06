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

#ifndef __PLUGIN_MENU_CAJA_ACTIONS_H__
#define __PLUGIN_MENU_CAJA_ACTIONS_H__

/**
 * SECTION: caja_actions
 * @short_description: #CajaActions class definition.
 * @include: plugin/caja-actions.h
 *
 * There is only one CajaActions object in the process.
 *
 * As a Caja extension, it is initialized when the module is loaded
 * by the file manager at startup time.
 *
 * In the caja-actions-config UI, it is initialized when the program
 * is loaded.
 *
 * The CajaActions object maintains the list of currently defined
 * actions in its private area.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CAJA_ACTIONS_TYPE					( caja_actions_get_type())
#define CAJA_ACTIONS( object )				( G_TYPE_CHECK_INSTANCE_CAST(( object ), CAJA_ACTIONS_TYPE, CajaActions ))
#define CAJA_ACTIONS_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST(( klass ), CAJA_ACTIONS_TYPE, CajaActionsClass ))
#define CAJA_IS_ACTIONS( object )			( G_TYPE_CHECK_INSTANCE_TYPE(( object ), CAJA_ACTIONS_TYPE ))
#define CAJA_IS_ACTIONS_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), CAJA_ACTIONS_TYPE ))
#define CAJA_ACTIONS_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), CAJA_ACTIONS_TYPE, CajaActionsClass ))

typedef struct CajaActionsPrivate      CajaActionsPrivate;

typedef struct
{
	GObject                 parent;
	CajaActionsPrivate *private;
}
	CajaActions;

typedef struct CajaActionsClassPrivate CajaActionsClassPrivate;

typedef struct
{
	GObjectClass                 parent;
	CajaActionsClassPrivate *private;
}
	CajaActionsClass;

GType caja_actions_get_type     ( void );
void  caja_actions_register_type( GTypeModule *module );

G_END_DECLS

#endif /* __PLUGIN_MENU_CAJA_ACTIONS_H__ */
