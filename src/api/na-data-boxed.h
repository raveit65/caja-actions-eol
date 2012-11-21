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

#ifndef __CAJA_ACTIONS_API_NA_DATA_BOXED_H__
#define __CAJA_ACTIONS_API_NA_DATA_BOXED_H__

/**
 * SECTION: data-boxed
 * @title: NADataBoxed
 * @short_description: The Data Factory Element Class Definition
 * @include: caja-actions/na-data-boxed.h
 *
 * The object which encapsulates an elementary data of #NAIFactoryObject.
 * A #NADataBoxed object has a type and a value.
 *
 * #NADataBoxed class is derived from #NABoxed one, and implements the same
 * types that those defined in na-data-types.h.
 *
 * Additionally, #NADataBoxed class holds the #NADataDef data definition
 * suitable for a NAFactoryObject object. It such provides default value
 * and validity status.
 *
 * Since: 2.30
 */

#include <glib-object.h>

#include "na-boxed.h"
#include "na-data-def.h"

G_BEGIN_DECLS

#define NA_TYPE_DATA_BOXED                ( na_data_boxed_get_type())
#define NA_DATA_BOXED( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_DATA_BOXED, NADataBoxed ))
#define NA_DATA_BOXED_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_DATA_BOXED, NADataBoxedClass ))
#define NA_IS_DATA_BOXED( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_DATA_BOXED ))
#define NA_IS_DATA_BOXED_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_DATA_BOXED ))
#define NA_DATA_BOXED_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_DATA_BOXED, NADataBoxedClass ))

typedef struct _NADataBoxedPrivate        NADataBoxedPrivate;

typedef struct {
	/*< private >*/
	NABoxed             parent;
	NADataBoxedPrivate *private;
}
	NADataBoxed;

typedef struct _NADataBoxedClassPrivate   NADataBoxedClassPrivate;

typedef struct {
	/*< private >*/
	NABoxedClass             parent;
	NADataBoxedClassPrivate *private;
}
	NADataBoxedClass;

GType            na_data_boxed_get_type( void );

NADataBoxed     *na_data_boxed_new            ( const NADataDef *def );

const NADataDef *na_data_boxed_get_data_def   ( const NADataBoxed *boxed );
void             na_data_boxed_set_data_def   ( NADataBoxed *boxed, const NADataDef *def );

GParamSpec      *na_data_boxed_get_param_spec ( const NADataDef *def );

gboolean         na_data_boxed_is_default     ( const NADataBoxed *boxed );
gboolean         na_data_boxed_is_valid       ( const NADataBoxed *boxed );

/* These functions are deprecated starting with 3.1.0
 */
#ifdef NA_ENABLE_DEPRECATED
gboolean         na_data_boxed_are_equal      ( const NADataBoxed *a, const NADataBoxed *b );
void             na_data_boxed_dump           ( const NADataBoxed *boxed );
gchar           *na_data_boxed_get_as_string  ( const NADataBoxed *boxed );
void             na_data_boxed_get_as_value   ( const NADataBoxed *boxed, GValue *value );
void            *na_data_boxed_get_as_void    ( const NADataBoxed *boxed );
void             na_data_boxed_set_from_boxed ( NADataBoxed *boxed, const NADataBoxed *value );
void             na_data_boxed_set_from_string( NADataBoxed *boxed, const gchar *value );
void             na_data_boxed_set_from_value ( NADataBoxed *boxed, const GValue *value );
void             na_data_boxed_set_from_void  ( NADataBoxed *boxed, const void *value );
#endif /* NA_ENABLE_DEPRECATED */

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_DATA_BOXED_H__ */
