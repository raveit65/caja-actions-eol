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

#include <api/na-core-utils.h>

#include "base-builder.h"

/* private class data
 */
struct _BaseBuilderClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _BaseBuilderPrivate {
	gboolean  dispose_has_run;
	GSList   *fnames;
};

static GtkBuilderClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( BaseBuilderClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *application );
static void     free_built_object( GObject *object, gpointer user_data );
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

	g_return_if_fail( BASE_IS_BUILDER( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = BASE_BUILDER( instance );

	self->private = g_new0( BaseBuilderPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *instance )
{
	static const gchar *thisfn = "base_builder_instance_dispose";
	BaseBuilder *self;
	GSList *objects;

	g_return_if_fail( BASE_IS_BUILDER( instance ));

	self = BASE_BUILDER( instance );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		objects = gtk_builder_get_objects( GTK_BUILDER( instance ));
		g_slist_foreach( objects, ( GFunc ) free_built_object, NULL );
		g_slist_free( objects );

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( instance );
		}
	}
}

static void
free_built_object( GObject *object, gpointer user_data )
{
	static const gchar *thisfn = "base_builder_free_built_object";

	if( GTK_IS_WIDGET( object )){
		if( gtk_widget_is_toplevel( GTK_WIDGET( object ))){

			g_debug( "%s: object=%p (%s) %s", thisfn,
					( void * ) object, G_OBJECT_TYPE_NAME( object ),
					gtk_buildable_get_name( GTK_BUILDABLE( object )));

			gtk_widget_destroy( GTK_WIDGET( object ));
		}
	}
}

static void
instance_finalize( GObject *instance )
{
	static const gchar *thisfn = "base_builder_instance_finalize";
	BaseBuilder *self;

	g_return_if_fail( BASE_IS_BUILDER( instance ));

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	self = BASE_BUILDER( instance );

	na_core_utils_slist_free( self->private->fnames );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( instance );
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

	builder = g_object_new( BASE_TYPE_BUILDER, NULL );

	return( builder );
}

/**
 * base_builder_add_from_file:
 * @builder: this #BaseBuilder object.
 * @filename: the filename to load.
 * @error: a #GError which will be allocated if an error occurs.
 *
 * Loads the file into the GtkBuilder, taking care of not loading it
 * twice.
 *
 * Returns: %TRUE if filename has been successfully loaded, or was
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

/**
 * base_builder_get_toplevel_by_name:
 * @builder: this #BaseBuilder object.
 * @name: the name of the searched toplevel window.
 *
 * This function provides a pointer to the toplevel window associated
 * with the specified @name.
 *
 * Returns: a pointer to the named window, or %NULL.
 * This pointer is owned by GtkBuilder instance, and must not be
 * g_free() nor g_object_unref() by the caller.
 */
GtkWindow *
base_builder_get_toplevel_by_name( const BaseBuilder *builder, const gchar *name )
{
	static const gchar *thisfn = "base_builder_get_toplevel_by_name";
	GtkWindow *toplevel = NULL;

	g_return_val_if_fail( BASE_IS_BUILDER( builder ), NULL );
	g_return_val_if_fail( name, NULL );
	g_return_val_if_fail( g_utf8_strlen( name, -1 ), NULL );

	if( !builder->private->dispose_has_run ){

		toplevel = GTK_WINDOW( gtk_builder_get_object( GTK_BUILDER( builder ), name ));

		if( toplevel ){
			g_return_val_if_fail( GTK_IS_WINDOW( toplevel ), NULL );
		} else {
			na_core_utils_slist_dump( thisfn, builder->private->fnames );
		}
	}

	return( toplevel );
}
