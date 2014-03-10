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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <string.h>

#include "base-application.h"
#include "base-isession.h"
#include "base-iunique.h"

/* private class data
 */
struct _BaseApplicationClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _BaseApplicationPrivate {
	gboolean      dispose_has_run;

	/* properties
	 */
	int           argc;
	GStrv         argv;
	GOptionEntry *options;
	gchar        *application_name;
	gchar        *description;
	gchar        *icon_name;
	gchar        *unique_app_name;
	int           code;
};

/* instance properties
 */
enum {
	BASE_PROP_0,

	BASE_PROP_ARGC_ID,
	BASE_PROP_ARGV_ID,
	BASE_PROP_OPTIONS_ID,
	BASE_PROP_APPLICATION_NAME_ID,
	BASE_PROP_DESCRIPTION_ID,
	BASE_PROP_ICON_NAME_ID,
	BASE_PROP_UNIQUE_NAME_ID,
	BASE_PROP_CODE_ID,

	BASE_PROP_N_PROPERTIES
};

static GObjectClass *st_parent_class = NULL;

static GType        register_type( void );
static void         class_init( BaseApplicationClass *klass );
static void         isession_iface_init( BaseISessionInterface *iface, void *user_data );
static void         iunique_iface_init( BaseIUniqueInterface *iface, void *user_data );
static const gchar *iunique_get_application_name( const BaseIUnique *instance );
static void         instance_init( GTypeInstance *instance, gpointer klass );
static void         instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void         instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void         instance_dispose( GObject *application );
static void         instance_finalize( GObject *application );

static gboolean     init_i18n( BaseApplication *application );
static gboolean     init_application_name( BaseApplication *application );
static gboolean     init_gtk( BaseApplication *application );
static gboolean     v_manage_options( BaseApplication *application );
static gboolean     init_unique_manager( BaseApplication *application );
static gboolean     init_session_manager( BaseApplication *application );
static gboolean     init_icon_name( BaseApplication *application );
static gboolean     v_init_application( BaseApplication *application );
static gboolean     v_create_windows( BaseApplication *application );

/*
 * the BaseISessionInterface interface is registered here because
 * the interface requires its implementation to be of BaseApplication
 * type. So we have to first register the type class before trying to
 * register the type interface.
 */
GType
base_application_get_type( void )
{
	static GType application_type = 0;

	if( !application_type ){
		application_type = register_type();
	}

	return( application_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "base_application_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( BaseApplicationClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( BaseApplication ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo isession_iface_info = {
		( GInterfaceInitFunc ) isession_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo iunique_iface_info = {
		( GInterfaceInitFunc ) iunique_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

#if !GLIB_CHECK_VERSION( 2,36, 0 )
	g_type_init();
#endif

	type = g_type_register_static( G_TYPE_OBJECT, "BaseApplication", &info, 0 );

	g_type_add_interface_static( type, BASE_TYPE_ISESSION, &isession_iface_info );

	g_type_add_interface_static( type, BASE_TYPE_IUNIQUE, &iunique_iface_info );

	return( type );
}

static void
class_init( BaseApplicationClass *klass )
{
	static const gchar *thisfn = "base_application_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;
	object_class->get_property = instance_get_property;
	object_class->set_property = instance_set_property;

	g_object_class_install_property( object_class, BASE_PROP_ARGC_ID,
			g_param_spec_int(
					BASE_PROP_ARGC,
					_( "Arguments count" ),
					_( "The count of command-line arguments" ),
					0, 65535, 0,
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_ARGV_ID,
			g_param_spec_boxed(
					BASE_PROP_ARGV,
					_( "Arguments" ),
					_( "The array of command-line arguments" ),
					G_TYPE_STRV,
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_OPTIONS_ID,
			g_param_spec_pointer(
					BASE_PROP_OPTIONS,
					_( "Option entries" ),
					_( "The array of command-line option definitions" ),
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_APPLICATION_NAME_ID,
			g_param_spec_string(
					BASE_PROP_APPLICATION_NAME,
					_( "Application name" ),
					_( "The name of the application" ),
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_DESCRIPTION_ID,
			g_param_spec_string(
					BASE_PROP_DESCRIPTION,
					_( "Description" ),
					_( "A short description to be displayed in the first line of --help output" ),
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_ICON_NAME_ID,
			g_param_spec_string(
					BASE_PROP_ICON_NAME,
					_( "Icon name" ),
					_( "The name of the icon of the application" ),
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_UNIQUE_NAME_ID,
			g_param_spec_string(
					BASE_PROP_UNIQUE_NAME,
					_( "UniqueApp name" ),
					_( "The Unique name of the application" ),
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_CODE_ID,
			g_param_spec_int(
					BASE_PROP_CODE,
					_( "Return code" ),
					_( "The return code of the application" ),
					-127, 127, 0,
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	klass->private = g_new0( BaseApplicationClassPrivate, 1 );

	klass->manage_options = NULL;
	klass->init_application = NULL;
	klass->create_windows = NULL;
}

static void
isession_iface_init( BaseISessionInterface *iface, void *user_data )
{
	static const gchar *thisfn = "base_application_isession_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
iunique_iface_init( BaseIUniqueInterface *iface, void *user_data )
{
	static const gchar *thisfn = "base_application_iunique_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->get_application_name = iunique_get_application_name;
}

static const gchar *
iunique_get_application_name( const BaseIUnique *instance )
{
	g_return_val_if_fail( BASE_IS_IUNIQUE( instance ), NULL );
	g_return_val_if_fail( BASE_IS_APPLICATION( instance ), NULL );

	return( BASE_APPLICATION( instance )->private->application_name );
}

static void
instance_init( GTypeInstance *application, gpointer klass )
{
	static const gchar *thisfn = "base_application_instance_init";
	BaseApplication *self;

	g_return_if_fail( BASE_IS_APPLICATION( application ));

	g_debug( "%s: application=%p (%s), klass=%p",
			thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ), ( void * ) klass );

	self = BASE_APPLICATION( application );

	self->private = g_new0( BaseApplicationPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	BaseApplication *self;

	g_return_if_fail( BASE_IS_APPLICATION( object ));
	self = BASE_APPLICATION( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_PROP_ARGC_ID:
				g_value_set_int( value, self->private->argc );
				break;

			case BASE_PROP_ARGV_ID:
				g_value_set_boxed( value, self->private->argv );
				break;

			case BASE_PROP_OPTIONS_ID:
				g_value_set_pointer( value, self->private->options );
				break;

			case BASE_PROP_APPLICATION_NAME_ID:
				g_value_set_string( value, self->private->application_name );
				break;

			case BASE_PROP_DESCRIPTION_ID:
				g_value_set_string( value, self->private->description );
				break;

			case BASE_PROP_ICON_NAME_ID:
				g_value_set_string( value, self->private->icon_name );
				break;

			case BASE_PROP_UNIQUE_NAME_ID:
				g_value_set_string( value, self->private->unique_app_name );
				break;

			case BASE_PROP_CODE_ID:
				g_value_set_int( value, self->private->code );
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
	BaseApplication *self;

	g_return_if_fail( BASE_IS_APPLICATION( object ));
	self = BASE_APPLICATION( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_PROP_ARGC_ID:
				self->private->argc = g_value_get_int( value );
				break;

			case BASE_PROP_ARGV_ID:
				if( self->private->argv ){
					g_boxed_free( G_TYPE_STRV, self->private->argv );
				}
				self->private->argv = g_value_dup_boxed( value );
				break;

			case BASE_PROP_OPTIONS_ID:
				self->private->options = g_value_get_pointer( value );
				break;

			case BASE_PROP_APPLICATION_NAME_ID:
				g_free( self->private->application_name );
				self->private->application_name = g_value_dup_string( value );
				break;

			case BASE_PROP_DESCRIPTION_ID:
				g_free( self->private->description );
				self->private->description = g_value_dup_string( value );
				break;

			case BASE_PROP_ICON_NAME_ID:
				g_free( self->private->icon_name );
				self->private->icon_name = g_value_dup_string( value );
				break;

			case BASE_PROP_UNIQUE_NAME_ID:
				g_free( self->private->unique_app_name );
				self->private->unique_app_name = g_value_dup_string( value );
				break;

			case BASE_PROP_CODE_ID:
				self->private->code = g_value_get_int( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_dispose( GObject *application )
{
	static const gchar *thisfn = "base_application_instance_dispose";
	BaseApplication *self;

	g_return_if_fail( BASE_IS_APPLICATION( application ));

	self = BASE_APPLICATION( application );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: application=%p (%s)", thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ));

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( application );
		}
	}
}

static void
instance_finalize( GObject *application )
{
	static const gchar *thisfn = "base_application_instance_finalize";
	BaseApplication *self;

	g_return_if_fail( BASE_IS_APPLICATION( application ));

	g_debug( "%s: application=%p (%s)", thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ));

	self = BASE_APPLICATION( application );

	g_free( self->private->application_name );
	g_free( self->private->description );
	g_free( self->private->icon_name );
	g_free( self->private->unique_app_name );
	g_strfreev( self->private->argv );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( application );
	}
}

/**
 * base_application_run_with_args:
 * @application: this #BaseApplication -derived instance.
 *
 * Starts and runs the application.
 * Takes care of creating, initializing, and running the main window.
 *
 * All steps are implemented as virtual functions which provide some
 * suitable defaults, and may be overriden by a derived class.
 *
 * Returns: an %int code suitable as an exit code for the program.
 *
 * Though it is defined as a virtual function itself, it should be very
 * seldomly needed to override this in a derived class.
 */
int
base_application_run_with_args( BaseApplication *application, int argc, GStrv argv )
{
	static const gchar *thisfn = "base_application_run_with_args";
	BaseApplicationPrivate *priv;

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), BASE_EXIT_CODE_PROGRAM );

	priv = application->private;

	if( !priv->dispose_has_run ){

		g_debug( "%s: application=%p (%s), argc=%d",
				thisfn,
				( void * ) application, G_OBJECT_TYPE_NAME( application ),
				argc );

		priv->argc = argc;
		priv->argv = g_strdupv( argv );
		priv->code = BASE_EXIT_CODE_OK;

		if( init_i18n( application ) &&
			init_application_name( application ) &&
			init_gtk( application ) &&
			v_manage_options( application ) &&
			init_unique_manager( application ) &&
			init_session_manager( application ) &&
			init_icon_name( application ) &&
			v_init_application( application ) &&
			v_create_windows( application )){

			g_debug( "%s: entering gtk_main", thisfn );
			gtk_main();
		}
	}

	return( priv->code );
}

/*
 * i18n initialization
 *
 * Returns: %TRUE to continue the execution, %FALSE to terminate the program.
 * The program exit code will be taken from @code.
 */
static gboolean
init_i18n( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_init_i18n";

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

#ifdef ENABLE_NLS
	bindtextdomain( GETTEXT_PACKAGE, MATELOCALEDIR );
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
# endif
	textdomain( GETTEXT_PACKAGE );
#endif

	return( TRUE );
}

/*
 * From GLib Reference Manual:
 * Sets a human-readable name for the application.
 * This name should be localized if possible, and is intended for display to the user.
 *
 * This application name is supposed to have been set as a property when
 * the BaseApplication-derived has been instanciated.
 */
static gboolean
init_application_name( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_init_application_name";
	gchar *name;
	gboolean ret;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = TRUE;

	/* setup default Gtk+ application name
	 * must have been set at instanciation time by the derived class
	 */
	name = base_application_get_application_name( application );
	if( name && g_utf8_strlen( name, -1 )){
		g_set_application_name( name );

	} else {
		application->private->code = BASE_EXIT_CODE_APPLICATION_NAME;
		ret = FALSE;
	}
	g_free( name );

	return( ret );
}

/*
 * pre-gtk initialization
 */
static gboolean
init_gtk( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_init_gtk";
	gboolean ret;
	char *parameter_string;
	GError *error;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	/* manage command-line arguments
	 */
	if( application->private->options ){
		parameter_string = g_strdup( g_get_application_name());
		error = NULL;
		ret = gtk_init_with_args(
				&application->private->argc,
				( char *** ) &application->private->argv,
				parameter_string,
				application->private->options,
				GETTEXT_PACKAGE,
				&error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );
			ret = FALSE;
			application->private->code = BASE_EXIT_CODE_ARGS;
		}
		g_free( parameter_string );

	} else {
		ret = gtk_init_check(
				&application->private->argc,
				( char *** ) &application->private->argv );
		if( !ret ){
			g_warning( "%s", _( "Unable to interpret command-line arguments" ));
			application->private->code = BASE_EXIT_CODE_ARGS;
		}
	}

	return( ret );
}

static gboolean
v_manage_options( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_manage_options";
	gboolean ret;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = TRUE;

	if( BASE_APPLICATION_GET_CLASS( application )->manage_options ){
		ret = BASE_APPLICATION_GET_CLASS( application )->manage_options( application );
	}

	return( ret );
}

/*
 * Initialize BaseIUnique interface for the instance
 */
static gboolean
init_unique_manager( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_init_unique_manager";
	gboolean ret;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = base_iunique_init_with_name(
			BASE_IUNIQUE( application ),
			application->private->unique_app_name );

	if( !ret ){
		application->private->code = BASE_EXIT_CODE_UNIQUE_APP;
	}

	return( ret );
}

/*
 * Relying on session manager to have a chance to save the modifications
 * before exiting a session
 */
static gboolean
init_session_manager( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_init_session_manager";

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	base_isession_init( BASE_ISESSION( application ));

	return( TRUE );
}

/*
 * From GTK+ Reference Manual:
 * Sets an icon to be used as fallback for windows that haven't had
 * gtk_window_set_icon_list() called on them from a named themed icon.
 *
 * This icon name is supposed to have been set as a property when
 * the BaseApplication-derived has been instanciated.
 */
static gboolean
init_icon_name( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_init_icon_name";

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	/* setup default application icon
	 */
	if( application->private->icon_name && g_utf8_strlen( application->private->icon_name, -1 )){
			gtk_window_set_default_icon_name( application->private->icon_name );

	} else {
		g_warning( "%s: no default icon name", thisfn );
	}

	return( TRUE );
}

static gboolean
v_init_application( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_init_application";
	gboolean ret;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = TRUE;

	if( BASE_APPLICATION_GET_CLASS( application )->init_application ){
		ret = BASE_APPLICATION_GET_CLASS( application )->init_application( application );
	}

	return( ret );
}

static gboolean
v_create_windows( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_create_windows";
	gboolean ret;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = TRUE;

	if( BASE_APPLICATION_GET_CLASS( application )->create_windows ){
		ret = BASE_APPLICATION_GET_CLASS( application )->create_windows( application );
	}

	return( ret );
}

/**
 * base_application_get_application_name:
 * @application: this #BaseApplication instance.
 *
 * Returns: the application name as a newly allocated string which should
 * be be g_free() by the caller.
 */
gchar *
base_application_get_application_name( const BaseApplication *application )
{
	gchar *name = NULL;

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){

		name = g_strdup( application->private->application_name );
	}

	return( name );
}

/**
 * base_application_is_willing_to_quit:
 * @application: this #BaseApplication instance.
 *
 * Returns: %TRUE if the application is willing to quit, %FALSE else.
 *
 * This function is typically called from the derived application, when
 * the user has required the termination, in order to let it handle
 * unsaved modifications.
 */
gboolean
base_application_is_willing_to_quit( const BaseApplication *application )
{
	g_return_val_if_fail( BASE_IS_APPLICATION( application ), TRUE );
	g_return_val_if_fail( BASE_IS_ISESSION( application ), TRUE );

	return( base_isession_is_willing_to_quit( BASE_ISESSION( application )));
}
