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

#ifndef __CACT_ICAPABILITIES_TAB_H__
#define __CACT_ICAPABILITIES_TAB_H__

/**
 * SECTION: cact_icapabilities_tab
 * @short_description: #CactICapabilitiesTab interface declaration.
 * @include: cact/cact-icapabilities-tab.h
 *
 * This interface implements all the widgets which define the
 * conditions for the action.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CACT_TYPE_ICAPABILITIES_TAB                      ( cact_icapabilities_tab_get_type())
#define CACT_ICAPABILITIES_TAB( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, CACT_TYPE_ICAPABILITIES_TAB, CactICapabilitiesTab ))
#define CACT_IS_ICAPABILITIES_TAB( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, CACT_TYPE_ICAPABILITIES_TAB ))
#define CACT_ICAPABILITIES_TAB_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_TYPE_ICAPABILITIES_TAB, CactICapabilitiesTabInterface ))

typedef struct _CactICapabilitiesTab                     CactICapabilitiesTab;
typedef struct _CactICapabilitiesTabInterfacePrivate     CactICapabilitiesTabInterfacePrivate;

typedef struct {
	/*< private >*/
	GTypeInterface                        parent;
	CactICapabilitiesTabInterfacePrivate *private;
}
	CactICapabilitiesTabInterface;

GType cact_icapabilities_tab_get_type( void );

void  cact_icapabilities_tab_init    ( CactICapabilitiesTab *instance );

G_END_DECLS

#endif /* __CACT_ICAPABILITIES_TAB_H__ */
