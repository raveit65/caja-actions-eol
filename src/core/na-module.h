/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu modules.
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

#ifndef __CORE_NA_MODULE_H__
#define __CORE_NA_MODULE_H__

/* @title: NAModule
 * @short_description: The #NAModule Class Definition
 * @include: core/na-module.h
 *
 * The NAModule class manages Caja-Actions extensions as dynamically
 * loadable modules (plugins).
 *
 * NAModule
 *  +- is derived from GTypeModule
 *      +- which itself implements GTypePlugin
 *
 * Each NAModule physically corresponds to a dynamically loadable library
 * (i.e. a plugin). A NAModule implements one or more interfaces, and/or
 * provides one or more services.
 *
 * Interfaces (resp. services) are implemented (resp. provided) by GObjects
 * which are dynamically instantiated at plugin initial-load time.
 *
 * So the dynamic is as follows:
 * - NAPivot scans for the PKGLIBDIR directory, trying to dynamically
 *   load all found libraries
 * - to be considered as a N-A plugin, a library must implement some
 *   functions (see api/na-api.h)
 * - for each found plugin, NAPivot calls na_api_list_types() which
 *   returns the type of GObjects implemented in the plugin
 * - NAPivot dynamically instantiates a GObject for each returned GType.
 *
 * After that, when NAPivot wants to access, say to NAIIOProvider
 * interfaces, it asks each module for its list of objects which implement
 * this given interface.
 * Interface API is then called against the returned GObject.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_TYPE_MODULE                ( na_module_get_type())
#define NA_MODULE( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_MODULE, NAModule ))
#define NA_MODULE_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_MODULE, NAModuleClass ))
#define NA_IS_MODULE( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_MODULE ))
#define NA_IS_MODULE_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_MODULE ))
#define NA_MODULE_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_MODULE, NAModuleClass ))

typedef struct _NAModulePrivate       NAModulePrivate;

typedef struct {
	/*< private >*/
	GTypeModule      parent;
	NAModulePrivate *private;
}
	NAModule;

typedef struct _NAModuleClassPrivate  NAModuleClassPrivate;

typedef struct {
	/*< private >*/
	GTypeModuleClass      parent;
	NAModuleClassPrivate *private;
}
	NAModuleClass;

GType    na_module_get_type               ( void );

void     na_module_dump                   ( const NAModule *module );
GList   *na_module_load_modules           ( void );

GList   *na_module_get_extensions_for_type( GList *modules, GType type );
void     na_module_free_extensions_list   ( GList *extensions );

gboolean na_module_has_id                 ( NAModule *module, const gchar *id );

void     na_module_release_modules        ( GList *modules );

G_END_DECLS

#endif /* __CORE_NA_MODULE_H__ */
