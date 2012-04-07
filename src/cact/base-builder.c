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

#include <api/na-core-utils.h>

#include "base-builder.h"

/* private class data
 */
struct BaseBuilderClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct BaseBuilderPrivate {
	gboolean  dispose_has_run;
	GSList   *fnames;
};

static GtkBuilderClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( BaseBuilderClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *application );
static void     instance_finalize( GObject *application );

static gboolean already_loaded( BaseBuilder *builder, const gchar *filename );

GType
base_builder_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "base_builder_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( BaseBuilderClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( BaseBuilder ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( GTK_TYPE_BUILDER, "BaseBuilder", &info, 0 );

	return( type );
}

static void
class_init( BaseBuilderClass *klass )
{
	static const gchar *thisfn = "base_builder_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( BaseBuilderClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "base_builder_instance_init";
	BaseBuilder *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( BASE_IS_BUILDER( instance ));
	self = BASE_BUILDER( instance );

	self->private = g_new0( BaseBuilderPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *instance )
{
	static const gchar *thisfn = "base_builder_instance_dispose";
	BaseBuilder *self;

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));
	g_return_if_fail( BASE_IS_BUILDER( instance ));
	self = BASE_BUILDER( instance );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( instance );
		}
	}
}

static void
instance_finalize( GObject *window )
{
	static const gchar *thisfn = "base_builder_instance_finalize";
	BaseBuilder *self;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );
	g_return_if_fail( BASE_IS_BUILDER( window ));
	self = BASE_BUILDER( window );

	na_core_utils_slist_free( self->private->fnames );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( window );
	}
}

/**
 * base_builder_new:
 *
 * Returns: a newly allocated #BaseBuilder object.
 */
BaseBuilder *
base_builder_new( void )
{
	BaseBuilder *builder;

	builder = g_object_new( BASE_BUILDER_TYPE, NULL );

	return( builder );
}

/**
 * base_builder_add_from_file:
 * @builder: this #BaseBuilder object.
 * @filename: the filename to load.
 * @error: a #GError whilch will be allocated if an error occurs.
 *
 * Loads the file into the GtkBuilder, taking care of not loading it
 * twice.
 *
 * Returns: %TRUE if filename has been successfully loaded, or were
 * already loaded, %FALSE else.
 */
gboolean
base_builder_add_from_file( BaseBuilder *builder, const gchar *filename, GError **error )
{
	static const gchar *thisfn = "base_builder_add_from_file";
	gboolean ret = FALSE;
	guint retint;

	g_return_val_if_fail( BASE_IS_BUILDER( builder ), FALSE );

	if( !builder->private->dispose_has_run ){

		if( already_loaded( builder, filename )){
			g_debug( "%s: %s already loaded", thisfn, filename );
			ret = TRUE;

		} else {
			retint = gtk_builder_add_from_file( GTK_BUILDER( builder ), filename, error );
			if( retint > 0 && !( *error )){
				builder->private->fnames = g_slist_prepend( builder->private->fnames, g_strdup( filename ));
				ret = TRUE;
			}
		}
	}

	return( ret );
}

/**
 * base_builder_load_named_toplevel:
 * @builder: this #BaseBuilder object.
 * @name: the name of the searched toplevel window.
 *
 * This function provides a pointer to the toplevel dialog associated
 * with the specified #BaseWindow.
 *
 * Returns: a pointer to the named dialog, or NULL.
 * This pointer is owned by GtkBuilder instance, and must not be
 * g_free() nor g_object_unref() by the caller.
 */
GtkWindow *
base_builder_load_named_toplevel( BaseBuilder *builder, const gchar *name )
{
	GtkWindow *toplevel = NULL;

	g_return_val_if_fail( BASE_IS_BUILDER( builder ), NULL );
	g_return_val_if_fail( name, NULL );
	g_return_val_if_fail( g_utf8_strlen( name, -1 ), NULL );

	if( !builder->private->dispose_has_run ){

		toplevel = GTK_WINDOW( gtk_builder_get_object( GTK_BUILDER( builder ), name ));

		if( toplevel ){
			g_return_val_if_fail( GTK_IS_WINDOW( toplevel ), NULL );
		}
	}

	return( toplevel );
}

static gboolean
already_loaded( BaseBuilder *builder, const gchar *filename )
{
	gboolean loaded = FALSE;
	GSList *it;

	for( it = builder->private->fnames ; it && !loaded ; it = it->next ){
		if( !na_core_utils_str_collate(( const gchar * ) it->data, filename )){
			loaded = TRUE;
		}
	}

	return( loaded );
}
