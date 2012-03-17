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

#ifndef __BASE_BUILDER_H__
#define __BASE_BUILDER_H__

/**
 * SECTION: base_builder
 * @short_description: #BaseBuilder class definition.
 * @include: nact/base-builder.h
 *
 * This class is derived from GtkBuilder class. It adds to it a list
 * of already loaded files to be sure to not load them twice.
 *
 * #Basebuilder class is embedded as a convenience object in
 * #BaseApplication and, possibly, in #BaseWindow instances.
 */

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BASE_BUILDER_TYPE					( base_builder_get_type())
#define BASE_BUILDER( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, BASE_BUILDER_TYPE, BaseBuilder ))
#define BASE_BUILDER_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, BASE_BUILDER_TYPE, BaseBuilderClass ))
#define BASE_IS_BUILDER( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, BASE_BUILDER_TYPE ))
#define BASE_IS_BUILDER_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), BASE_BUILDER_TYPE ))
#define BASE_BUILDER_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), BASE_BUILDER_TYPE, BaseBuilderClass ))

typedef struct BaseBuilderPrivate      BaseBuilderPrivate;

typedef struct {
	GtkBuilder          parent;
	BaseBuilderPrivate *private;
}
	BaseBuilder;

typedef struct BaseBuilderClassPrivate BaseBuilderClassPrivate;

typedef struct {
	GtkBuilderClass          parent;
	BaseBuilderClassPrivate *private;
}
	BaseBuilderClass;

GType        base_builder_get_type( void );

BaseBuilder *base_builder_new( void );

gboolean     base_builder_add_from_file( BaseBuilder *builder, const gchar *filename, GError **error );

GtkWindow   *base_builder_load_named_toplevel( BaseBuilder *builder, const gchar *name );

G_END_DECLS

#endif /* __BASE_BUILDER_H__ */
