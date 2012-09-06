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

#ifndef __CACT_IACTION_TAB_H__
#define __CACT_IACTION_TAB_H__

/**
 * SECTION: cact_iaction_tab
 * @short_description: #CactIActionTab interface definition.
 * @include: cact/cact-iaction-tab.h
 *
 * This interface implements the "Caja Menu Item" tab of the notebook.
 *
 * Entry fields are enabled, as soon as an edited item has been set a a
 * property of the main window,
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CACT_IACTION_TAB_TYPE						( cact_iaction_tab_get_type())
#define CACT_IACTION_TAB( object )					( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_IACTION_TAB_TYPE, CactIActionTab ))
#define CACT_IS_IACTION_TAB( object )				( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_IACTION_TAB_TYPE ))
#define CACT_IACTION_TAB_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), CACT_IACTION_TAB_TYPE, CactIActionTabInterface ))

typedef struct CactIActionTab CactIActionTab;

typedef struct CactIActionTabInterfacePrivate CactIActionTabInterfacePrivate;

typedef struct {
	GTypeInterface                  parent;
	CactIActionTabInterfacePrivate *private;
}
	CactIActionTabInterface;

GType    cact_iaction_tab_get_type( void );

void     cact_iaction_tab_initial_load_toplevel( CactIActionTab *instance );
void     cact_iaction_tab_runtime_init_toplevel( CactIActionTab *instance );
void     cact_iaction_tab_all_widgets_showed( CactIActionTab *instance );
void     cact_iaction_tab_dispose( CactIActionTab *instance );

gboolean cact_iaction_tab_has_label( CactIActionTab *instance );

G_END_DECLS

#endif /* __CACT_IACTION_TAB_H__ */
