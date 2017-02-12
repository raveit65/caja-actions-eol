/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
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

#ifndef __CACT_ISCHEMES_TAB_H__
#define __CACT_ISCHEMES_TAB_H__

/**
 * SECTION: cact_ischemes_tab
 * @short_description: #CactISchemesTab interface declaration.
 * @include: cact/cact-ischemes-tab.h
 *
 * This interface implements all the widgets which define the
 * conditions for the action.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CACT_TYPE_ISCHEMES_TAB                      ( cact_ischemes_tab_get_type())
#define CACT_ISCHEMES_TAB( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, CACT_TYPE_ISCHEMES_TAB, CactISchemesTab ))
#define CACT_IS_ISCHEMES_TAB( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, CACT_TYPE_ISCHEMES_TAB ))
#define CACT_ISCHEMES_TAB_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_TYPE_ISCHEMES_TAB, CactISchemesTabInterface ))

typedef struct _CactISchemesTab                     CactISchemesTab;
typedef struct _CactISchemesTabInterfacePrivate     CactISchemesTabInterfacePrivate;

typedef struct {
	/*< private >*/
	GTypeInterface                   parent;
	CactISchemesTabInterfacePrivate *private;
}
	CactISchemesTabInterface;

GType cact_ischemes_tab_get_type( void );

void  cact_ischemes_tab_init    ( CactISchemesTab *instance );

G_END_DECLS

#endif /* __CACT_ISCHEMES_TAB_H__ */
