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

#ifndef __CAJA_ACTIONS_API_NA_ICONTEXT_H__
#define __CAJA_ACTIONS_API_NA_ICONTEXT_H__

/**
 * SECTION: na_icontext
 * @short_description: #NAIContext interface definition.
 * @include: caja-actions/na-icontext.h
 *
 * This interface is implemented by all #NAObject-derived objects
 * whose the display in the Caja context menu is subject to some
 * conditions.
 *
 * Implementors, typically actions, profiles and menus, host the required
 * data as #NADataBoxed in a dedicated NA_FACTORY_CONDITIONS_GROUP
 * data group.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_ICONTEXT_TYPE						( na_icontext_get_type())
#define NA_ICONTEXT( instance )					( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_ICONTEXT_TYPE, NAIContext ))
#define NA_IS_ICONTEXT( instance )				( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_ICONTEXT_TYPE ))
#define NA_ICONTEXT_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_ICONTEXT_TYPE, NAIContextInterface ))

typedef struct NAIContext                 NAIContext;

typedef struct NAIContextInterfacePrivate NAIContextInterfacePrivate;

typedef struct {
	GTypeInterface              parent;
	NAIContextInterfacePrivate *private;

	/**
	 * is_candidate:
	 * @object: this #NAIContext object.
	 * @target: the initial target which triggered this function's stack.
	 *  This target is defined in na-object-item.h.
	 * @selection: the current selection as a #GList of #CajaFileInfo.
	 *
	 * Returns: %TRUE if the @object may be a potential candidate, %FALSE
	 * else.
	 *
	 * The #NAIContext implementor may take advantage of this
	 * virtual function to check for its own specific data. Only if the
	 * implementor does return %TRUE (or just doesn't implement this
	 * virtual), the conditions themselves will be checked.
	 */
	gboolean ( *is_candidate )( NAIContext *object, guint target, GList *selection );
}
	NAIContextInterface;

GType    na_icontext_get_type( void );

gboolean na_icontext_is_candidate( const NAIContext *object, guint target, GList *selection );
gboolean na_icontext_is_valid    ( const NAIContext *object );

void     na_icontext_set_scheme    ( NAIContext *object, const gchar *scheme, gboolean selected );
void     na_icontext_replace_folder( NAIContext *object, const gchar *old, const gchar *new );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_ICONTEXT_H__ */
