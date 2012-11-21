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

#ifndef __CACT_IPROPERTIES_TAB_H__
#define __CACT_IPROPERTIES_TAB_H__

/**
 * SECTION: cact_iproperties_tab
 * @short_description: #CactIPropertiesTab interface definition.
 * @include: cact/cact-iproperties-tab.h
 *
 * This interface implements the "Properties" tab of the notebook.
 *
 * Entry fields are enabled, as soon as an edited item has been set a a
 * property of the main window,
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CACT_TYPE_IPROPERTIES_TAB                      ( cact_iproperties_tab_get_type())
#define CACT_IPROPERTIES_TAB( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, CACT_TYPE_IPROPERTIES_TAB, CactIPropertiesTab ))
#define CACT_IS_IPROPERTIES_TAB( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, CACT_TYPE_IPROPERTIES_TAB ))
#define CACT_IPROPERTIES_TAB_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_TYPE_IPROPERTIES_TAB, CactIPropertiesTabInterface ))

typedef struct _CactIPropertiesTab                     CactIPropertiesTab;
typedef struct _CactIPropertiesTabInterfacePrivate     CactIPropertiesTabInterfacePrivate;

typedef struct {
	/*< private >*/
	GTypeInterface                      parent;
	CactIPropertiesTabInterfacePrivate *private;
}
	CactIPropertiesTabInterface;

GType cact_iproperties_tab_get_type( void );

void  cact_iproperties_tab_init    ( CactIPropertiesTab *instance );

G_END_DECLS

#endif /* __CACT_IPROPERTIES_TAB_H__ */
