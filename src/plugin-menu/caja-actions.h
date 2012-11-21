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

#ifndef __PLUGIN_MENU_CAJA_ACTIONS_H__
#define __PLUGIN_MENU_CAJA_ACTIONS_H__

/**
 * SECTION: caja-actions
 * @title: CajaActions
 * @short_description: The CajaActions plugin class definition
 * @include: plugin-menu/caja-actions.h
 *
 * This is the class which handles the file manager menu plugin.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CAJA_ACTIONS_TYPE                ( caja_actions_get_type())
#define CAJA_ACTIONS( object )           ( G_TYPE_CHECK_INSTANCE_CAST(( object ), CAJA_ACTIONS_TYPE, CajaActions ))
#define CAJA_ACTIONS_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST(( klass ), CAJA_ACTIONS_TYPE, CajaActionsClass ))
#define CAJA_IS_ACTIONS( object )        ( G_TYPE_CHECK_INSTANCE_TYPE(( object ), CAJA_ACTIONS_TYPE ))
#define CAJA_IS_ACTIONS_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CAJA_ACTIONS_TYPE ))
#define CAJA_ACTIONS_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CAJA_ACTIONS_TYPE, CajaActionsClass ))

typedef struct _CajaActionsPrivate       CajaActionsPrivate;

typedef struct {
	/*< private >*/
	GObject                 parent;
	CajaActionsPrivate *private;
}
	CajaActions;

typedef struct _CajaActionsClassPrivate  CajaActionsClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass                 parent;
	CajaActionsClassPrivate *private;
}
	CajaActionsClass;

GType    caja_actions_get_type     ( void );
void     caja_actions_register_type( GTypeModule *module );

G_END_DECLS

#endif /* __PLUGIN_MENU_CAJA_ACTIONS_H__ */
