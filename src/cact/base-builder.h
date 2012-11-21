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

#ifndef __BASE_BUILDER_H__
#define __BASE_BUILDER_H__

/**
 * SECTION: base_builder
 * @short_description: #BaseBuilder class definition.
 * @include: cact/base-builder.h
 *
 * This class is derived from GtkBuilder class. It adds to it two
 * features:
 *
 * - do not load twice the .xml files;
 * - destroy Gtk toplevels at dispose time.
 *
 * A common #Basebuilder object is instanciated at #BaseWindow class level.
 * Each #Basewindow -derived object may later use this common #BaseBuilder
 * object, or allocate its own.
 */

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BASE_TYPE_BUILDER                ( base_builder_get_type())
#define BASE_BUILDER( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, BASE_TYPE_BUILDER, BaseBuilder ))
#define BASE_BUILDER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, BASE_TYPE_BUILDER, BaseBuilderClass ))
#define BASE_IS_BUILDER( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, BASE_TYPE_BUILDER ))
#define BASE_IS_BUILDER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), BASE_TYPE_BUILDER ))
#define BASE_BUILDER_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), BASE_TYPE_BUILDER, BaseBuilderClass ))

typedef struct _BaseBuilderPrivate       BaseBuilderPrivate;

typedef struct {
	/*< private >*/
	GtkBuilder          parent;
	BaseBuilderPrivate *private;
}
	BaseBuilder;

typedef struct _BaseBuilderClassPrivate  BaseBuilderClassPrivate;

typedef struct {
	/*< private >*/
	GtkBuilderClass          parent;
	BaseBuilderClassPrivate *private;
}
	BaseBuilderClass;

GType        base_builder_get_type            ( void );

BaseBuilder *base_builder_new                 ( void );

gboolean     base_builder_add_from_file       ( BaseBuilder *builder, const gchar *filename, GError **error );

GtkWindow   *base_builder_get_toplevel_by_name( const BaseBuilder *builder, const gchar *name );

G_END_DECLS

#endif /* __BASE_BUILDER_H__ */
