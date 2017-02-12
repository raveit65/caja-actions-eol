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

#ifndef __CACT_IBASENAMES_TAB_H__
#define __CACT_IBASENAMES_TAB_H__

/**
 * SECTION: cact_ibasenames_tab
 * @short_description: #CactIBasenamesTab interface declaration.
 * @include: cact/cact-ibasenames-tab.h
 *
 * This interface implements all the widgets which define the
 * basenames-based conditions.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CACT_TYPE_IBASENAMES_TAB                      ( cact_ibasenames_tab_get_type())
#define CACT_IBASENAMES_TAB( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, CACT_TYPE_IBASENAMES_TAB, CactIBasenamesTab ))
#define CACT_IS_IBASENAMES_TAB( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, CACT_TYPE_IBASENAMES_TAB ))
#define CACT_IBASENAMES_TAB_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_TYPE_IBASENAMES_TAB, CactIBasenamesTabInterface ))

typedef struct _CactIBasenamesTab                     CactIBasenamesTab;
typedef struct _CactIBasenamesTabInterfacePrivate     CactIBasenamesTabInterfacePrivate;

typedef struct {
	/*< private >*/
	GTypeInterface                     parent;
	CactIBasenamesTabInterfacePrivate *private;
}
	CactIBasenamesTabInterface;

GType cact_ibasenames_tab_get_type( void );

void  cact_ibasenames_tab_init    ( CactIBasenamesTab *instance );

G_END_DECLS

#endif /* __CACT_IBASENAMES_TAB_H__ */
