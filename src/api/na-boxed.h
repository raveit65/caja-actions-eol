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

#ifndef __CAJA_ACTIONS_API_NA_BOXED_H__
#define __CAJA_ACTIONS_API_NA_BOXED_H__

/**
 * SECTION: boxed
 * @title: NABoxed
 * @short_description: The NABoxed Structure
 * @include: caja-actions/na-boxed.h
 *
 * The NABoxed structure is a way of handling various types of data in an
 * opaque structure.
 *
 * Since: 3.1
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_TYPE_BOXED                ( na_boxed_get_type())
#define NA_BOXED( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_BOXED, NABoxed ))
#define NA_BOXED_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_BOXED, NABoxedClass ))
#define NA_IS_BOXED( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_BOXED ))
#define NA_IS_BOXED_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_BOXED ))
#define NA_BOXED_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_BOXED, NABoxedClass ))

typedef struct _NABoxedPrivate       NABoxedPrivate;

typedef struct {
	/*< private >*/
	GObject         parent;
	NABoxedPrivate *private;
}
	NABoxed;

typedef struct _NABoxedClassPrivate  NABoxedClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass         parent;
	NABoxedClassPrivate *private;
}
	NABoxedClass;

GType         na_boxed_get_type       ( void );
void          na_boxed_set_type       ( NABoxed *boxed, guint type );

gboolean      na_boxed_are_equal      ( const NABoxed *a, const NABoxed *b );
NABoxed      *na_boxed_copy           ( const NABoxed *boxed );
void          na_boxed_dump           ( const NABoxed *boxed );
NABoxed      *na_boxed_new_from_string( guint type, const gchar *string );

gboolean      na_boxed_get_boolean    ( const NABoxed *boxed );
gconstpointer na_boxed_get_pointer    ( const NABoxed *boxed );
gchar        *na_boxed_get_string     ( const NABoxed *boxed );
GSList       *na_boxed_get_string_list( const NABoxed *boxed );
guint         na_boxed_get_uint       ( const NABoxed *boxed );
GList        *na_boxed_get_uint_list  ( const NABoxed *boxed );
void          na_boxed_get_as_value   ( const NABoxed *boxed, GValue *value );
void         *na_boxed_get_as_void    ( const NABoxed *boxed );

void          na_boxed_set_from_boxed ( NABoxed *boxed, const NABoxed *value );
void          na_boxed_set_from_string( NABoxed *boxed, const gchar *value );
void          na_boxed_set_from_value ( NABoxed *boxed, const GValue *value );
void          na_boxed_set_from_void  ( NABoxed *boxed, const void *value );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_BOXED_H__ */
