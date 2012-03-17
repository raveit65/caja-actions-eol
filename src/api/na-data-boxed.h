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

#ifndef __CAJA_ACTIONS_API_NA_DATA_BOXED_H__
#define __CAJA_ACTIONS_API_NA_DATA_BOXED_H__

/**
 * SECTION: na_data_boxed
 * @short_description: #NADataBoxed class definition.
 * @include: core/na-data-boxed.h
 *
 * The object which encapsulates an elementary data of #NAIFactoryObject.
 * A #NADataBoxed object has a type and a value.
 */

#include <glib-object.h>

#include "na-data-def.h"

G_BEGIN_DECLS

#define NA_DATA_BOXED_TYPE					( na_data_boxed_get_type())
#define NA_DATA_BOXED( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_DATA_BOXED_TYPE, NADataBoxed ))
#define NA_DATA_BOXED_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NA_DATA_BOXED_TYPE, NADataBoxedClass ))
#define NA_IS_DATA_BOXED( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_DATA_BOXED_TYPE ))
#define NA_IS_DATA_BOXED_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_DATA_BOXED_TYPE ))
#define NA_DATA_BOXED_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_DATA_BOXED_TYPE, NADataBoxedClass ))

typedef struct NADataBoxedPrivate      NADataBoxedPrivate;

typedef struct {
	GObject             parent;
	NADataBoxedPrivate *private;
}
	NADataBoxed;

typedef struct NADataBoxedClassPrivate NADataBoxedClassPrivate;

typedef struct {
	GObjectClass             parent;
	NADataBoxedClassPrivate *private;
}
	NADataBoxedClass;

GType        na_data_boxed_get_type( void );

GParamSpec  *na_data_boxed_get_param_spec ( const NADataDef *def );

NADataBoxed *na_data_boxed_new            ( const NADataDef *def );

NADataDef   *na_data_boxed_get_data_def   ( const NADataBoxed *boxed );
gboolean     na_data_boxed_are_equal      ( const NADataBoxed *a, const NADataBoxed *b );
gboolean     na_data_boxed_is_valid       ( const NADataBoxed *boxed );
gboolean     na_data_boxed_is_set         ( const NADataBoxed *boxed );
void         na_data_boxed_dump           ( const NADataBoxed *boxed );

void         na_data_boxed_set_data_def   ( NADataBoxed *boxed, const NADataDef *def );

gchar       *na_data_boxed_get_as_string  ( const NADataBoxed *boxed );
void        *na_data_boxed_get_as_void    ( const NADataBoxed *boxed );
void         na_data_boxed_get_as_value   ( const NADataBoxed *boxed, GValue *value );

void         na_data_boxed_set_from_boxed ( NADataBoxed *boxed, const NADataBoxed *value );
void         na_data_boxed_set_from_string( NADataBoxed *boxed, const gchar *value );
void         na_data_boxed_set_from_value ( NADataBoxed *boxed, const GValue *value );
void         na_data_boxed_set_from_void  ( NADataBoxed *boxed, const void *value );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_DATA_BOXED_H__ */
