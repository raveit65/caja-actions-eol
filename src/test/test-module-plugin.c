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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>
#include <gmodule.h>

/* this is a plugin, i.e. a dynamically loaded resource
 */

G_MODULE_EXPORT const gchar *g_module_check_init( GModule *module );
G_MODULE_EXPORT void         g_module_unload( GModule *module );

#if 0
/* this is first version
 * the module is loaded, the function called, and then the module is unloaded
 * note that we do not define any GType here, so do not provide any sort of
 * GInterface implementation
 *
	(process:23911): NA-test-DEBUG: in g_module_check_init: module=0xf97f20
	(process:23911): NA-test-DEBUG: say_hello: module=0xf97f20
	(process:23911): NA-test-DEBUG: in g_module_unload: module=0xf97f20
 *
 */

G_MODULE_EXPORT void         say_hello( GModule *module );

/* automatically called when the module is loaded
 */
G_MODULE_EXPORT const gchar *
g_module_check_init( GModule *module )
{
	const gchar *error;

	error = NULL;
	g_debug( "in g_module_check_init: module=%p", ( void * ) module );

	return( error );
}

/* automatically called when the module is unloaded
 */
G_MODULE_EXPORT void
g_module_unload( GModule *module )
{
	g_debug( "in g_module_unload: module=%p", ( void * ) module );
}

/* a function dynamically called from the program
 */
G_MODULE_EXPORT void
say_hello( GModule *module )
{
	g_debug( "say_hello: module=%p", ( void * ) module );
}
#endif

/* version 2
 * define the module as a GTypeModule-derived one
 */
#define TEST_MODULE_PLUGIN_TYPE                 ( na_module_plugin_get_type())
#define TEST_MODULE_PLUGIN( object )            ( G_TYPE_CHECK_INSTANCE_CAST( object, TEST_MODULE_PLUGIN_TYPE, NAModulePlugin ))
#define TEST_MODULE_PLUGIN_CLASS( klass )       ( G_TYPE_CHECK_CLASS_CAST( klass, TEST_MODULE_PLUGIN_TYPE, NAModulePluginClass ))
#define TEST_IS_MODULE_PLUGIN( object )         ( G_TYPE_CHECK_INSTANCE_TYPE( object, TEST_MODULE_PLUGIN_TYPE ))
#define TEST_IS_MODULE_PLUGIN_CLASS( klass )    ( G_TYPE_CHECK_CLASS_TYPE(( klass ), TEST_MODULE_PLUGIN_TYPE ))
#define TEST_MODULE_PLUGIN_GET_CLASS( object )  ( G_TYPE_INSTANCE_GET_CLASS(( object ), TEST_MODULE_PLUGIN_TYPE, NAModulePluginClass ))

typedef struct _NAModulePluginPrivate           NAModulePluginPrivate;
typedef struct _NAModulePluginClassPrivate      NAModulePluginClassPrivate;

typedef struct {
	GObject                parent;
	NAModulePluginPrivate *private;
}
	NAModulePlugin;

typedef struct {
	GObjectClass                parent;
	NAModulePluginClassPrivate *private;
}
	NAModulePluginClass;

GType na_module_plugin_get_type( void );

/* private instance data
 */
struct _NAModulePluginPrivate {
	gboolean dispose_has_run;
};

/* private class data
 */
struct _NAModulePluginClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static GType             st_module_type = 0;
static GTypeModuleClass *st_parent_class = NULL;

static void           na_module_plugin_register_type( GTypeModule *module );
static void class_init( NAModulePluginClass *klass );
static void instance_init( GTypeInstance *instance, gpointer klass );
static void instance_dispose( GObject *object );
static void instance_finalize( GObject *object );

G_MODULE_EXPORT void plugin_init( GTypeModule *module );

GType
na_module_plugin_get_type( void )
{
	return( st_module_type );
}

static void
na_module_plugin_register_type( GTypeModule *module )
{
	static const gchar *thisfn = "na_module_plugin_register_type";

	static GTypeInfo info = {
		sizeof( NAModulePluginClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAModulePlugin ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	/*static const GInterfaceInfo iio_provider_iface_info = {
		( GInterfaceInitFunc ) iio_provider_iface_init,
		NULL,
		NULL
	};*/

	g_debug( "%s", thisfn );

	st_module_type = g_type_module_register_type( module, G_TYPE_OBJECT, "NAModulePlugin", &info, 0 );

	/*g_type_module_add_interface( module, st_module_type, NA_TYPE_IIO_PROVIDER, &iio_provider_iface_info );*/
}

static void
class_init( NAModulePluginClass *klass )
{
	static const gchar *thisfn = "na_module_plugin_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAModulePluginClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_module_plugin_instance_init";
	NAModulePlugin *self;

	g_return_if_fail( TEST_IS_MODULE_PLUGIN( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = TEST_MODULE_PLUGIN( instance );

	self->private = g_new0( NAModulePluginPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_module_plugin_instance_dispose";
	NAModulePlugin *self;

	g_return_if_fail( TEST_IS_MODULE_PLUGIN( object ));

	self = TEST_MODULE_PLUGIN( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_test_module_plugin_instance_finalize";
	NAModulePlugin *self;

	g_return_if_fail( TEST_IS_MODULE_PLUGIN( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = TEST_MODULE_PLUGIN( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/* automatically called when the module is loaded
 * this is very less useful in this version as we need a GTypeModule to register
 * our dynamic types
 */
G_MODULE_EXPORT const gchar *
g_module_check_init( GModule *module )
{
	g_debug( "in g_module_check_init: module=%p", ( void * ) module );
	return( NULL );
}

/* automatically called when the module is unloaded
 */
G_MODULE_EXPORT void
g_module_unload( GModule *module )
{
	g_debug( "in g_module_unload: module=%p", ( void * ) module );
}

/* module is actually a NAModule, but we do not care of that
 * registering the type - but do not yet allocate an object
 */
G_MODULE_EXPORT void
plugin_init( GTypeModule *module )
{
	g_debug( "plugin_init: module=%p", ( void * ) module );

	na_module_plugin_register_type( module );
}
