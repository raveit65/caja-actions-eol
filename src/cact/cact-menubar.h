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

#ifndef __CACT_MENUBAR_H__
#define __CACT_MENUBAR_H__

/*
 * SECTION: cact-menubar
 * @title: CactMenubar
 * @short_description: The Menubar class definition
 * @include: cact-menubar.h
 *
 * This is a convenience class which embeds the menubar of the application.
 *
 * There is one object (because there is one menubar). It is created by
 * the main window at initialization time.
 *
 * Attaching the object to the window let us connect easily to all 'window'-
 * class messages, thus reducing the count of needed public functions.
 *
 * The #CactMenubar object connects to BASE_SIGNAL_INITIALIZE_WINDOW signal
 * at instanciation time. The caller (usually a #CactMainWindow) should take
 * care to connect itself to this signal before creating the #CactMenubar
 * object if it wants its callback be triggered first.
 *
 * The #CactMenubar object maintains as private data the indicators needed
 * in order to rightly update menu items sensitivity.
 * It is up to the application to update these indicators.
 * Each time an indicator is updated, it triggers an update of all relevant
 * menu items sensitivities.
 *
 * Toolbar and sort buttons are also driven by this menubar.
 */

#include "base-window.h"

G_BEGIN_DECLS

#define CACT_TYPE_MENUBAR                ( cact_menubar_get_type())
#define CACT_MENUBAR( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_MENUBAR, CactMenubar ))
#define CACT_MENUBAR_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_MENUBAR, CactMenubarClass ))
#define CACT_IS_MENUBAR( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_MENUBAR ))
#define CACT_IS_MENUBAR_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_MENUBAR ))
#define CACT_MENUBAR_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_MENUBAR, CactMenubarClass ))

typedef struct _CactMenubarPrivate       CactMenubarPrivate;

typedef struct {
	/*< private >*/
	GObject             parent;
	CactMenubarPrivate *private;
}
	CactMenubar;

typedef struct _CactMenubarClassPrivate  CactMenubarClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass             parent;
	CactMenubarClassPrivate *private;
}
	CactMenubarClass;

GType        cact_menubar_get_type  ( void );

CactMenubar *cact_menubar_new       ( BaseWindow *window );

void         cact_menubar_save_items( BaseWindow *window );

G_END_DECLS

#endif /* __CACT_MENUBAR_H__ */
