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

#ifndef __CAJA_ACTIONS_API_NA_ICONTEXT_H__
#define __CAJA_ACTIONS_API_NA_ICONTEXT_H__

/**
 * SECTION: icontext
 * @title: NAIContext
 * @short_description: The Contextual Interface
 * @include: caja-actions/na-icontext.h
 *
 * This interface is implemented by all #NAObject -derived objects
 * whose the display in the Caja context menu is subject to some
 * conditions.
 *
 * Implementors, typically #NAObjectAction, #NAObjectProfile and
 * #NAObjectMenu, host the required data as #NADataBoxed in a dedicated
 * NA_FACTORY_CONDITIONS_GROUP data group.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_TYPE_ICONTEXT                      ( na_icontext_get_type())
#define NA_ICONTEXT( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_TYPE_ICONTEXT, NAIContext ))
#define NA_IS_ICONTEXT( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_TYPE_ICONTEXT ))
#define NA_ICONTEXT_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_TYPE_ICONTEXT, NAIContextInterface ))

typedef struct _NAIContext                    NAIContext;
typedef struct _NAIContextInterfacePrivate    NAIContextInterfacePrivate;

/**
 * NAIContextInterface:
 * @is_candidate: determines if the given NAObject-derived object is
 *                candidate to display in Caja.
 *
 * This interface manages all conditions relevant to a displayable status
 * in Caja.
 */
typedef struct {
	/*< private >*/
	GTypeInterface              parent;
	NAIContextInterfacePrivate *private;

	/*< public >*/
	/**
	 * is_candidate:
	 * @object: this NAIContext object.
	 * @target: the initial target which triggered this function's stack.
	 *  This target is defined in na-object-item.h.
	 * @selection: the current selection as a GList of CajaFileInfo.
	 *
	 * The NAIContext implementor may take advantage of this
	 * virtual function to check for its own specific data. Only if the
	 * implementor does return %TRUE (or just doesn't implement this
	 * virtual), the conditions themselves will be checked.
	 *
	 * Returns: %TRUE if the @object may be a potential candidate, %FALSE
	 * else.
	 *
	 * Since: 2.30
	 */
	gboolean ( *is_candidate )( NAIContext *object, guint target, GList *selection );
}
	NAIContextInterface;

GType    na_icontext_get_type( void );

gboolean na_icontext_are_equal       ( const NAIContext *a, const NAIContext *b );
gboolean na_icontext_is_candidate    ( const NAIContext *context, guint target, GList *selection );
gboolean na_icontext_is_valid        ( const NAIContext *context );

void     na_icontext_check_mimetypes ( const NAIContext *context );

void     na_icontext_copy            ( NAIContext *context, const NAIContext *source );
void     na_icontext_read_done       ( NAIContext *context );
void     na_icontext_set_scheme      ( NAIContext *context, const gchar *scheme, gboolean selected );
void     na_icontext_set_only_desktop( NAIContext *context, const gchar *desktop, gboolean selected );
void     na_icontext_set_not_desktop ( NAIContext *context, const gchar *desktop, gboolean selected );
void     na_icontext_replace_folder  ( NAIContext *context, const gchar *old, const gchar *new );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_ICONTEXT_H__ */
