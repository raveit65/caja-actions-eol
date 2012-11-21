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

#ifndef __CACT_IFOLDERS_TAB_H__
#define __CACT_IFOLDERS_TAB_H__

/**
 * SECTION: cact_ifolders_tab
 * @short_description: #CactIFoldersTab interface declaration.
 * @include: cact/cact-ifolders-tab.h
 *
 * This interface implements all the widgets which are relevant for
 * items which are applied to backgrounds.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CACT_TYPE_IFOLDERS_TAB                      ( cact_ifolders_tab_get_type())
#define CACT_IFOLDERS_TAB( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, CACT_TYPE_IFOLDERS_TAB, CactIFoldersTab ))
#define CACT_IS_IFOLDERS_TAB( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, CACT_TYPE_IFOLDERS_TAB ))
#define CACT_IFOLDERS_TAB_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_TYPE_IFOLDERS_TAB, CactIFoldersTabInterface ))

typedef struct _CactIFoldersTab                     CactIFoldersTab;
typedef struct _CactIFoldersTabInterfacePrivate     CactIFoldersTabInterfacePrivate;

typedef struct {
	/*< private >*/
	GTypeInterface                   parent;
	CactIFoldersTabInterfacePrivate *private;
}
	CactIFoldersTabInterface;

GType cact_ifolders_tab_get_type( void );

void  cact_ifolders_tab_init    ( CactIFoldersTab *instance );

G_END_DECLS

#endif /* __CACT_IFOLDERS_TAB_H__ */
