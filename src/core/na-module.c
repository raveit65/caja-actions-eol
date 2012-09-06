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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gmodule.h>

#include <api/na-core-utils.h>

#include "na-module.h"

/* private class data
 */
struct NAModuleClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAModulePrivate {
	gboolean  dispose_has_run;
	gchar    *path;						/* full pathname of the plugin */
	gchar    *name;						/* basename without the extension */
	GModule  *library;
	GList    *objects;

	/* api
	 */
	gboolean ( *startup )    ( GTypeModule *module );
	guint    ( *get_version )( void );
	gint     ( *list_types ) ( const GType **types );
	void     ( *shutdown )   ( void );
};

static GTypeModuleClass *st_parent_class = NULL;

static GType     register_type( void );
static void      class_init( NAModuleClass *klass );
static void      instance_init( GTypeInstance *instance, gpointer klass );
static void      instance_dispose( GObject *object );
static void      instance_finalize( GObject *object );

static NAModule *module_new( const gchar *filename );
static gboolean  module_load( GTypeModule *gmodule );
static gboolean  is_a_na_plugin( NAModule *module );
static gboolean  plugin_check( NAModule *module, const gchar *symbol, gpointer *pfn );
static void      register_module_types( NAModule *module );
static void      add_module_type( NAModule *module, GType type );
static void      object_weak_notify( NAModule *module, GObject *object );

static void      module_unload( GTypeModule *gmodule );

GType
na_module_get_type( void )
{
	static GType object_type = 0;

	if( !object_type ){
		object_type = register_type();
	}

	return( object_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_module_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAModuleClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAModule ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_TYPE_MODULE, "NAModule", &info, 0 );

	return( type );
}

static void
class_init( NAModuleClass *klass )
{
	static const gchar *thisfn = "na_module_class_init";
	GObjectClass *object_class;
	GTypeModuleClass *module_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	module_class = G_TYPE_MODULE_CLASS( klass );
	module_class->load = module_load;
	module_class->unload = module_unload;

	klass->private = g_new0( NAModuleClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_module_instance_init";
	NAModule *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( NA_IS_MODULE( instance ));
	self = NA_MODULE( instance );

	self->private = g_new0( NAModulePrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_module_instance_dispose";
	NAModule *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NA_IS_MODULE( object ));
	self = NA_MODULE( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_module_instance_finalize";
	NAModule *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_MODULE( object ));
	self = NA_MODULE( object );

	g_free( self->private->path );
	g_free( self->private->name );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * na_module_dump:
 * @module: this #NAModule instance.
 *
 * Dumps the content of the module.
 */
void
na_module_dump( const NAModule *module )
{
	static const gchar *thisfn = "na_module_dump";
	GList *iobj;

	g_debug( "%s:    path=%s", thisfn, module->private->path );
	g_debug( "%s:    name=%s", thisfn, module->private->name );
	g_debug( "%s: library=%p", thisfn, ( void * ) module->private->library );
	g_debug( "%s: objects=%p (count=%d)", thisfn, ( void * ) module->private->objects, g_list_length( module->private->objects ));
	for( iobj = module->private->objects ; iobj ; iobj = iobj->next ){
		g_debug( "%s:    iobj=%p (%s)", thisfn, ( void * ) iobj->data, G_OBJECT_TYPE_NAME( iobj->data ));
	}
}

/**
 * na_module_load_modules:
 *
 * Load availables dynamically loadable extension libraries (plugins).
 *
 * Returns: a #GList of #NAModule, each object representing a dynamically
 * loaded library.
 */
GList *
na_module_load_modules( void )
{
	static const gchar *thisfn = "na_module_load_modules";
	const gchar *dirname = PKGLIBDIR;
	const gchar *suffix = ".so";
	GList *modules;
	GDir *api_dir;
	GError *error;
	const gchar *entry;
	gchar *fname;
	NAModule *module;

	g_debug( "%s", thisfn );

	modules = NULL;
	error = NULL;

	api_dir = g_dir_open( dirname, 0, &error );
	if( error ){
		g_warning( "%s: g_dir_open: %s", thisfn, error->message );
		g_error_free( error );
		error = NULL;

	} else {
		while(( entry = g_dir_read_name( api_dir )) != NULL ){
			if( g_str_has_suffix( entry, suffix )){
				fname = g_build_filename( dirname, entry, NULL );
				module = module_new( fname );
				if( module ){
					module->private->name = na_core_utils_str_remove_suffix( entry, suffix );
					modules = g_list_prepend( modules, module );
					g_debug( "%s: module %s successfully loaded", thisfn, entry );
				}
				g_free( fname );
			}
		}
		g_dir_close( api_dir );
		modules = g_list_reverse( modules );
	}

	return( modules );
}

/*
 * @fname: full pathname of the being-loaded dynamic library.
 */
static NAModule *
module_new( const gchar *fname )
{
	NAModule *module;

	module = g_object_new( NA_MODULE_TYPE, NULL );
	module->private->path = g_strdup( fname );

	if( !g_type_module_use( G_TYPE_MODULE( module )) || !is_a_na_plugin( module )){

		g_object_unref( module );
		return( NULL );
	}

	register_module_types( module );

	return( module );
}

/*
 * triggered by GTypeModule base class when first loading the library,
 * which is itself triggered by module_new:g_type_module_use()
 *
 * returns: %TRUE if the module is successfully loaded
 */
static gboolean
module_load( GTypeModule *gmodule )
{
	static const gchar *thisfn = "na_module_module_load";
	NAModule *module;
	gboolean loaded;

	g_debug( "%s: gmodule=%p", thisfn, ( void * ) gmodule );
	g_return_val_if_fail( G_IS_TYPE_MODULE( gmodule ), FALSE );

	loaded = FALSE;
	module = NA_MODULE( gmodule );

	module->private->library = g_module_open( module->private->path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL );

	if( !module->private->library ){
		g_warning( "%s: g_module_open: path=%s, error=%s", thisfn, module->private->path, g_module_error());
	} else {
		loaded = TRUE;
	}

	return( loaded );
}

/*
 * the module has been successfully loaded
 * is it a Caja-Action plugin ?
 * if ok, we ask the plugin to initialize itself
 */
static gboolean
is_a_na_plugin( NAModule *module )
{
	static const gchar *thisfn = "na_module_is_a_na_plugin";
	gboolean ok;

	ok =
		plugin_check( module, "na_extension_startup"    , ( gpointer * ) &module->private->startup) &&
		plugin_check( module, "na_extension_list_types" , ( gpointer * ) &module->private->list_types ) &&
		module->private->startup( G_TYPE_MODULE( module ));

	if( ok ){
		g_debug( "%s: %s: ok", thisfn, module->private->path );
	}

	return( ok );
}

static gboolean
plugin_check( NAModule *module, const gchar *symbol, gpointer *pfn )
{
	static const gchar *thisfn = "na_module_plugin_check";
	gboolean ok;

	ok = g_module_symbol( module->private->library, symbol, pfn );

	if( !ok ){
		g_debug("%s: %s: %s: symbol not found", thisfn, module->private->path, symbol );
	}

	return( ok );
}

/*
 * the 'na_extension_startup' function of the plugin has been already
 * called ; the GType types the plugin provides have so already been
 * registered in the GType system
 *
 * we ask here the plugin to give us a list of these GTypes
 * for each GType, we allocate a new object of the given class
 * and keep this object in the module's list
 */
static void
register_module_types( NAModule *module )
{
	const GType *types;
	guint count, i;

	count = module->private->list_types( &types );
	module->private->objects = NULL;

	for( i = 0 ; i < count ; i++ ){
		if( types[i] ){
			add_module_type( module, types[i] );
		}
	}

	module->private->objects = g_list_reverse( module->private->objects );
}

static void
add_module_type( NAModule *module, GType type )
{
	GObject *object;

	object = g_object_new( type, NULL );
	g_debug( "na_module_add_module_type: allocating object=%p (%s)", ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_object_weak_ref( object, ( GWeakNotify ) object_weak_notify, module );

	module->private->objects = g_list_prepend( module->private->objects, object );
}

static void
object_weak_notify( NAModule *module, GObject *object )
{
	static const gchar *thisfn = "na_module_object_weak_notify";

	g_debug( "%s: module=%p, object=%p (%s)",
			thisfn, ( void * ) module, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	module->private->objects = g_list_remove( module->private->objects, object );
}

/*
 * 'unload' is triggered by the last 'unuse' call
 * which is itself called in na_module::instance_dispose
 */
static void
module_unload( GTypeModule *gmodule )
{
	static const gchar *thisfn = "na_module_module_unload";
	NAModule *module;

	g_debug( "%s: gmodule=%p", thisfn, ( void * ) gmodule );
	g_return_if_fail( G_IS_TYPE_MODULE( gmodule ));

	module = NA_MODULE( gmodule );

	if( module->private->shutdown ){
		module->private->shutdown();
	}

	if( module->private->library ){
		g_module_close( module->private->library );
	}

	module->private->startup = NULL;
	module->private->get_version = NULL;
	module->private->list_types = NULL;
	module->private->shutdown = NULL;
}

/**
 * na_module_get_extensions_for_type:
 * @type: the serched GType.
 *
 * Returns: a list of loaded modules willing to deal with requested @type.
 */
GList *
na_module_get_extensions_for_type( GList *modules, GType type )
{
	GList *willing_to, *im, *io;
	NAModule *a_modul;

	willing_to = NULL;

	for( im = modules; im ; im = im->next ){
		a_modul = NA_MODULE( im->data );
		for( io = a_modul->private->objects ; io ; io = io->next ){
			if( G_TYPE_CHECK_INSTANCE_TYPE( G_OBJECT( io->data ), type )){
				willing_to = g_list_prepend( willing_to, g_object_ref( io->data ));
			}
		}
	}

	return( g_list_reverse( willing_to ));
}

/**
 * na_module_free_extensions_list:
 * @extensions: a #GList as returned by #na_module_get_extensions_for_type().
 *
 * Free the previously returned list.
 */
void
na_module_free_extensions_list( GList *extensions )
{
	g_list_foreach( extensions, ( GFunc ) g_object_unref, NULL );
	g_list_free( extensions );
}

/**
 * na_module_has_id:
 * @module: this #NAModule object.
 * @id: the searched id.
 *
 * Returns: %TRUE if one of the interfaces advertised by the module has
 * the given id, %FALSE else.
 */
gboolean
na_module_has_id( NAModule *module, const gchar *id )
{
	gboolean id_ok;
	GList *iobj;

	id_ok = FALSE;
	for( iobj = module->private->objects ; iobj && !id_ok ; iobj = iobj->next ){
		g_debug( "na_module_has_id: object=%s", G_OBJECT_TYPE_NAME( iobj->data ));
	}

	return( id_ok );
}

/**
 * na_module_release_modules:
 * @modules: the list of loaded modules.
 *
 * Release resources allocated to the loaded modules on #NAPivot dispose.
 */
void
na_module_release_modules( GList *modules )
{
	GList *imod;
	GList *iobj;

	for( imod = modules ; imod ; imod = imod->next ){

		for( iobj = NA_MODULE( imod->data )->private->objects ; iobj ; iobj = iobj->next ){
			g_object_unref( iobj->data );
		}
	}

	g_list_free( modules );
}
