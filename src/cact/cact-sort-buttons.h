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

#ifndef __CACT_SORT_BUTTONS_H__
#define __CACT_SORT_BUTTONS_H__

/**
 * SECTION: cact-sort-buttons
 * @title: CactSortButtons
 * @short_description: The Sort Buttons class definition
 * @include: cact-sort-buttons.h
 *
 * A convenience class to manager sort buttons in the user interface.
 *
 * The sort order mode is monitored, so that buttons automatically display
 * the right order mode if it is modified by another way (e.g. from
 * Preferences editor).
 *
 * Modifying the sort order mode requires that:
 * - level zero is writable (see NAUpdater)
 * - preferences are not locked (see NAUpdater)
 * - sort order mode is not a mandatory preference.
 */

#include "base-window.h"

G_BEGIN_DECLS

#define CACT_TYPE_SORT_BUTTONS                ( cact_sort_buttons_get_type())
#define CACT_SORT_BUTTONS( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_SORT_BUTTONS, CactSortButtons ))
#define CACT_SORT_BUTTONS_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_SORT_BUTTONS, CactSortButtonsClass ))
#define CACT_IS_SORT_BUTTONS( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_SORT_BUTTONS ))
#define CACT_IS_SORT_BUTTONS_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_SORT_BUTTONS ))
#define CACT_SORT_BUTTONS_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_SORT_BUTTONS, CactSortButtonsClass ))

typedef struct _CactSortButtonsPrivate        CactSortButtonsPrivate;

typedef struct {
	/*< private >*/
	GObject                 parent;
	CactSortButtonsPrivate *private;
}
	CactSortButtons;

typedef struct _CactSortButtonsClassPrivate   CactSortButtonsClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass                 parent;
	CactSortButtonsClassPrivate *private;
}
	CactSortButtonsClass;

GType            cact_sort_buttons_get_type( void );
CactSortButtons *cact_sort_buttons_new     ( BaseWindow *window );

G_END_DECLS

#endif /* __CACT_SORT_BUTTONS_H__ */
