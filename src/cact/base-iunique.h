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

#ifndef __BASE_IUNIQUE_H__
#define __BASE_IUNIQUE_H__

/**
 * SECTION: base_iunique
 * @short_description: #BaseIUnique interface definition.
 * @include: cact/base-iunique.h
 *
 * This interface implements the features to make an application
 * unique, i.e. check that we run only one instance of it.
 *
 * This interface is implemented by the BaseApplication class.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define BASE_TYPE_IUNIQUE                      ( base_iunique_get_type())
#define BASE_IUNIQUE( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, BASE_TYPE_IUNIQUE, BaseIUnique ))
#define BASE_IS_IUNIQUE( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, BASE_TYPE_IUNIQUE ))
#define BASE_IUNIQUE_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), BASE_TYPE_IUNIQUE, BaseIUniqueInterface ))

typedef struct _BaseIUnique                    BaseIUnique;
typedef struct _BaseIUniqueInterfacePrivate    BaseIUniqueInterfacePrivate;

typedef struct {
	/*< private >*/
	GTypeInterface               parent;
	BaseIUniqueInterfacePrivate *private;

	/*< public >*/
	/**
	 * get_application_name:
	 * @instance: this #NAIExporter instance.
	 *
	 * Returns: the 'display' name of the application.
	 */
	const gchar * ( *get_application_name )( const BaseIUnique *instance );
}
	BaseIUniqueInterface;

GType    base_iunique_get_type      ( void );

gboolean base_iunique_init_with_name( BaseIUnique *instance, const gchar *unique_app_name );

G_END_DECLS

#endif /* __BASE_IUNIQUE_H__ */
