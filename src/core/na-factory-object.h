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

#ifndef __CORE_NA_FACTORY_OBJECT_H__
#define __CORE_NA_FACTORY_OBJECT_H__

/* @title: NAIFactoryObject
 * @short_description: The #NAIFactoryObject Internal Functions
 * @include: core/na-factory-object.h
 *
 * Declare the function only accessed from core library, i.e. not
 * published as API.
 */

#include <api/na-ifactory-provider.h>

G_BEGIN_DECLS

typedef gboolean ( *NAFactoryObjectIterBoxedFn )( const NAIFactoryObject *object, NADataBoxed *boxed, void *data );

#define NA_IFACTORY_OBJECT_PROP_DATA			"na-ifactory-object-prop-data"

void         na_factory_object_define_properties( GObjectClass *class, const NADataGroup *groups );
NADataDef   *na_factory_object_get_data_def     ( const NAIFactoryObject *object, const gchar *name );
NADataGroup *na_factory_object_get_data_groups  ( const NAIFactoryObject *object );
void         na_factory_object_iter_on_boxed    ( const NAIFactoryObject *object, NAFactoryObjectIterBoxedFn pfn, void *user_data );

gchar       *na_factory_object_get_default      ( NAIFactoryObject *object, const gchar *name );
void         na_factory_object_set_defaults     ( NAIFactoryObject *object );

void         na_factory_object_move_boxed       ( NAIFactoryObject *target, const NAIFactoryObject *source, NADataBoxed *boxed );

void         na_factory_object_copy             ( NAIFactoryObject *target, const NAIFactoryObject *source );
gboolean     na_factory_object_are_equal        ( const NAIFactoryObject *a, const NAIFactoryObject *b );
gboolean     na_factory_object_is_valid         ( const NAIFactoryObject *object );
void         na_factory_object_dump             ( const NAIFactoryObject *object );
void         na_factory_object_finalize         ( NAIFactoryObject *object );

void         na_factory_object_read_item        ( NAIFactoryObject *object, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
guint        na_factory_object_write_item       ( NAIFactoryObject *object, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );

void        *na_factory_object_get_as_void      ( const NAIFactoryObject *object, const gchar *name );
void         na_factory_object_get_as_value     ( const NAIFactoryObject *object, const gchar *name, GValue *value );
gboolean     na_factory_object_is_set           ( const NAIFactoryObject *object, const gchar *name );

void         na_factory_object_set_from_value   ( NAIFactoryObject *object, const gchar *name, const GValue *value );
void         na_factory_object_set_from_void    ( NAIFactoryObject *object, const gchar *name, const void *data );

G_END_DECLS

#endif /* __CORE_NA_FACTORY_OBJECT_H__ */
