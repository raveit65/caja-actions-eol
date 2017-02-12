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

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "na-import-mode.h"
#include "na-ioption.h"

/* private class data
 */
struct _NAImportModeClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _NAImportModePrivate {
	gboolean   dispose_has_run;

	/* properties
	 */
	guint      id;
	gchar     *mode;
	gchar     *label;
	gchar     *description;
	GdkPixbuf *image;
};

/* properties
 */
enum {
	MAIN_PROP_0 = 0,

	NA_IMPORT_PROP_MODE_ID,
	NA_IMPORT_PROP_LABEL_ID,
	NA_IMPORT_PROP_DESCRIPTION_ID,
	NA_IMPORT_PROP_IMAGE_ID,

	NA_IMPORT_PROP_N_PROPERTIES
};

static GObjectClass *st_parent_class = NULL;

static GType      register_type( void );
static void       class_init( NAImportModeClass *klass );
static void       ioption_iface_init( NAIOptionInterface *iface, void *user_data );
static gchar     *ioption_get_id( const NAIOption *option );
static gchar     *ioption_get_label( const NAIOption *option );
static gchar     *ioption_get_description( const NAIOption *option );
static GdkPixbuf *ioption_get_pixbuf( const NAIOption *option );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void       instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void       instance_dispose( GObject *object );
static void       instance_finalize( GObject *object );

GType
na_import_mode_get_type( void )
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
	static const gchar *thisfn = "na_import_mode_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAImportModeClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAImportMode ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	static const GInterfaceInfo ioption_iface_info = {
		( GInterfaceInitFunc ) ioption_iface_init,
		NULL,
		NULL
	};

	type = g_type_register_static( G_TYPE_OBJECT, "NAImportMode", &info, 0 );

	g_type_add_interface_static( type, NA_TYPE_IOPTION, &ioption_iface_info );

	return( type );
}

static void
class_init( NAImportModeClass *klass )
{
	static const gchar *thisfn = "na_import_mode_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->set_property = instance_set_property;
	object_class->get_property = instance_get_property;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAImportModeClassPrivate, 1 );

	g_object_class_install_property( object_class, NA_IMPORT_PROP_MODE_ID,
			g_param_spec_string(
					NA_IMPORT_PROP_MODE,
					"Import mode",
					"The string identifier of the import mode, stored in user's preferences",
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, NA_IMPORT_PROP_LABEL_ID,
			g_param_spec_string(
					NA_IMPORT_PROP_LABEL,
					"Import label",
					"The label associated to the import mode",
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, NA_IMPORT_PROP_DESCRIPTION_ID,
			g_param_spec_string(
					NA_IMPORT_PROP_DESCRIPTION,
					"Import mode description",
					"The description associated to the import mode",
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, NA_IMPORT_PROP_IMAGE_ID,
			g_param_spec_pointer(
					NA_IMPORT_PROP_IMAGE,
					"Import mode image",
					"The image associated to the import mode, as a GdkPixbuf",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));
}

static void
ioption_iface_init( NAIOptionInterface *iface, void *user_data )
{
	static const gchar *thisfn = "na_import_mode_ioption_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->get_id = ioption_get_id;
	iface->get_label = ioption_get_label;
	iface->get_description = ioption_get_description;
	iface->get_pixbuf = ioption_get_pixbuf;
}

/*
 * ioption_get_id:
 * @option: this #NAIOption instance.
 *
 * Returns: the ASCII id of the @option, as a newly allocated string which
 * should be g_free() by the caller.
 */
static gchar *
ioption_get_id( const NAIOption *option )
{
	gchar *id;
	NAImportMode *mode;

	g_return_val_if_fail( NA_IS_IMPORT_MODE( option ), NULL );
	mode = NA_IMPORT_MODE( option );
	id = NULL;

	if( !mode->private->dispose_has_run ){

		id = g_strdup( mode->private->mode );
	}

	return( id );
}

/*
 * ioption_get_label:
 * @option: this #NAIOption instance.
 *
 * Returns: the label associated to @option, as a newly allocated string
 * which should be g_free() by the caller.
 */
static gchar *
ioption_get_label( const NAIOption *option )
{
	gchar *label;
	NAImportMode *mode;

	g_return_val_if_fail( NA_IS_IMPORT_MODE( option ), NULL );
	mode = NA_IMPORT_MODE( option );
	label = NULL;

	if( !mode->private->dispose_has_run ){

		label = g_strdup( mode->private->label );
	}

	return( label );
}

/*
 * ioption_get_description:
 * @option: this #NAIOption instance.
 *
 * Returns: the description associated to @option, as a newly allocated string
 * which should be g_free() by the caller.
 */
static gchar *
ioption_get_description( const NAIOption *option )
{
	gchar *description;
	NAImportMode *mode;

	g_return_val_if_fail( NA_IS_IMPORT_MODE( option ), NULL );
	mode = NA_IMPORT_MODE( option );
	description = NULL;

	if( !mode->private->dispose_has_run ){

		description = g_strdup( mode->private->description );
	}

	return( description );
}

/*
 * ioption_get_pixbuf:
 * @option: this #NAIOption instance.
 *
 * Returns: a new reference to the pixbuf associated to @option;
 * which should later be g_object_unref() by the caller.
 */
static GdkPixbuf *
ioption_get_pixbuf( const NAIOption *option )
{
	GdkPixbuf *pixbuf;
	NAImportMode *mode;

	g_return_val_if_fail( NA_IS_IMPORT_MODE( option ), NULL );
	mode = NA_IMPORT_MODE( option );
	pixbuf = NULL;

	if( !mode->private->dispose_has_run ){

		pixbuf = mode->private->image ? g_object_ref( mode->private->image ) : NULL;
	}

	return( pixbuf );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_import_mode_instance_init";
	NAImportMode *self;

	g_return_if_fail( NA_IS_IMPORT_MODE( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	self = NA_IMPORT_MODE( instance );

	self->private = g_new0( NAImportModePrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	NAImportMode *self;

	g_return_if_fail( NA_IS_IMPORT_MODE( object ));
	self = NA_IMPORT_MODE( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case NA_IMPORT_PROP_MODE_ID:
				g_value_set_string( value, self->private->mode );
				break;

			case NA_IMPORT_PROP_LABEL_ID:
				g_value_set_string( value, self->private->label );
				break;

			case NA_IMPORT_PROP_DESCRIPTION_ID:
				g_value_set_string( value, self->private->description );
				break;

			case NA_IMPORT_PROP_IMAGE_ID:
				g_value_set_pointer( value, self->private->image );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec )
{
	NAImportMode *self;

	g_return_if_fail( NA_IS_IMPORT_MODE( object ));
	self = NA_IMPORT_MODE( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case NA_IMPORT_PROP_MODE_ID:
				g_free( self->private->mode );
				self->private->mode = g_value_dup_string( value );
				break;

			case NA_IMPORT_PROP_LABEL_ID:
				g_free( self->private->label );
				self->private->label = g_value_dup_string( value );
				break;

			case NA_IMPORT_PROP_DESCRIPTION_ID:
				g_free( self->private->description );
				self->private->description = g_value_dup_string( value );
				break;

			case NA_IMPORT_PROP_IMAGE_ID:
				self->private->image = g_value_get_pointer( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_import_mode_instance_dispose";
	NAImportMode *self;

	g_return_if_fail( NA_IS_IMPORT_MODE( object ));

	self = NA_IMPORT_MODE( object );

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
	static const gchar *thisfn = "na_import_mode_instance_finalize";
	NAImportMode *self;

	g_return_if_fail( NA_IS_IMPORT_MODE( object ));

	g_debug( "%s: object=%p", thisfn, ( void * ) object );

	self = NA_IMPORT_MODE( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/*
 * na_import_mode_new:
 * @mode_id: the internal identifier of the import mode.
 *
 * Returns: a newly allocated #NAImportMode object.
 *
 * Since: 3.2
 */
NAImportMode *
na_import_mode_new( guint mode_id )
{
	NAImportMode *mode;

	mode = g_object_new( NA_TYPE_IMPORT_MODE, NULL );

	mode->private->id = mode_id;

	return( mode );
}

/*
 * na_import_mode_get_id:
 * @mode: a #NAImportMode object.
 *
 * Returns: the internal identifier of the import mode.
 *
 * Since: 3.2
 */
guint
na_import_mode_get_id( const NAImportMode *mode )
{
	guint id;

	g_return_val_if_fail( NA_IS_IMPORT_MODE( mode ), 0 );

	id = 0;

	if( !mode->private->dispose_has_run ){

		id = mode->private->id;
	}

	return( id );
}
