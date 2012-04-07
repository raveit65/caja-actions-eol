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

#ifndef __CACT_ICONDITIONS_TAB_H__
#define __CACT_ICONDITIONS_TAB_H__

/**
 * SECTION: cact_iconditionstab
 * @short_description: #CactIConditionsTab interface declaration.
 * @include: cact/cact-iconditions-tab.h
 *
 * This interface implements all the widgets which define the
 * conditions for the action.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CACT_ICONDITIONS_TAB_TYPE						( cact_iconditions_tab_get_type())
#define CACT_ICONDITIONS_TAB( object )					( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_ICONDITIONS_TAB_TYPE, CactIConditionsTab ))
#define CACT_IS_ICONDITIONS_TAB( object )				( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_ICONDITIONS_TAB_TYPE ))
#define CACT_ICONDITIONS_TAB_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_ICONDITIONS_TAB_TYPE, CactIConditionsTabInterface ))

typedef struct CactIConditionsTab CactIConditionsTab;

typedef struct CactIConditionsTabInterfacePrivate CactIConditionsTabInterfacePrivate;

typedef struct {
	GTypeInterface                      parent;
	CactIConditionsTabInterfacePrivate *private;
}
	CactIConditionsTabInterface;

GType    cact_iconditions_tab_get_type( void );

void     cact_iconditions_tab_initial_load_toplevel( CactIConditionsTab *instance );
void     cact_iconditions_tab_runtime_init_toplevel( CactIConditionsTab *instance );
void     cact_iconditions_tab_all_widgets_showed( CactIConditionsTab *instance );
void     cact_iconditions_tab_dispose( CactIConditionsTab *instance );

void     cact_iconditions_tab_get_isfiledir( CactIConditionsTab *instance, gboolean *isfile, gboolean *isdir );
gboolean cact_iconditions_tab_get_multiple( CactIConditionsTab *instance );

G_END_DECLS

#endif /* __CACT_ICONDITIONS_TAB_H__ */
