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

#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <string.h>
#include <unique/unique.h>

#include "base-application.h"
#include "base-window.h"
#include "egg-sm-client.h"

/* private class data
 */
struct BaseApplicationClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct BaseApplicationPrivate {
	gboolean      dispose_has_run;
	int           argc;
	gpointer      argv;
	GOptionEntry *options;
	gboolean      is_gtk_initialized;
	UniqueApp    *unique_app_handle;
	int           exit_code;
	gchar        *exit_message1;
	gchar        *exit_message2;
	BaseBuilder  *builder;
	BaseWindow   *main_window;
	EggSMClient  *sm_client;
};

/* instance properties
 */
enum {
	BASE_APPLICATION_PROP_ARGC_ID = 1,
	BASE_APPLICATION_PROP_ARGV_ID,
	BASE_APPLICATION_PROP_OPTIONS_ID,
	BASE_APPLICATION_PROP_IS_GTK_INITIALIZED_ID,
	BASE_APPLICATION_PROP_UNIQUE_APP_HANDLE_ID,
	BASE_APPLICATION_PROP_EXIT_CODE_ID,
	BASE_APPLICATION_PROP_EXIT_MESSAGE1_ID,
	BASE_APPLICATION_PROP_EXIT_MESSAGE2_ID,
	BASE_APPLICATION_PROP_BUILDER_ID,
	BASE_APPLICATION_PROP_MAIN_WINDOW_ID
};

static GObjectClass *st_parent_class = NULL;

static GType          register_type( void );
static void           class_init( BaseApplicationClass *klass );
static void           instance_init( GTypeInstance *instance, gpointer klass );
static void           instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void           instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void           instance_dispose( GObject *application );
static void           instance_finalize( GObject *application );

static gboolean       v_initialize( BaseApplication *application );
static gboolean       v_initialize_i18n( BaseApplication *application );
static gboolean       v_initialize_gtk( BaseApplication *application );
static gboolean       v_manage_options( BaseApplication *application );
static gboolean       v_initialize_application_name( BaseApplication *application );
static gboolean       v_initialize_session_manager( BaseApplication *application );
static gboolean       v_initialize_unique_app( BaseApplication *application );
static gboolean       v_initialize_ui( BaseApplication *application );
static gboolean       v_initialize_default_icon( BaseApplication *application );
static gboolean       v_initialize_application( BaseApplication *application );

static int            application_do_run( BaseApplication *application );
static gboolean       application_do_initialize( BaseApplication *application );
static gboolean       application_do_initialize_i18n( BaseApplication *application );
static gboolean       application_do_initialize_gtk( BaseApplication *application );
static gboolean       application_do_manage_options( BaseApplication *application );
static gboolean       application_do_initialize_application_name( BaseApplication *application );
static gboolean       application_do_initialize_session_manager( BaseApplication *application );
static gboolean       application_do_initialize_unique_app( BaseApplication *application );
static gboolean       application_do_initialize_ui( BaseApplication *application );
static gboolean       application_do_initialize_default_icon( BaseApplication *application );
static gboolean       application_do_initialize_application( BaseApplication *application );

static gboolean       check_for_unique_app( BaseApplication *application );
/*static UniqueResponse on_unique_message_received( UniqueApp *app, UniqueCommand command, UniqueMessageData *message, guint time, gpointer user_data );*/

static void           client_quit_cb( EggSMClient *client, BaseApplication *application );
static void           client_quit_requested_cb( EggSMClient *client, BaseApplication *application );
static gint           display_dlg( BaseApplication *application, GtkMessageType type_message, GtkButtonsType type_buttons, const gchar *first, const gchar *second );
static void           display_error_message( BaseApplication *application );
static gboolean       init_with_args( BaseApplication *application, int *argc, char ***argv, GOptionEntry *entries );
static void           set_initialize_i18n_error( BaseApplication *application );
static void           set_initialize_gtk_error( BaseApplication *application );
static void           set_initialize_unique_app_error( BaseApplication *application );
static void           set_initialize_ui_get_fname_error( BaseApplication *application );
static void           set_initialize_ui_add_xml_error( BaseApplication *application, const gchar *filename, GError *error );
static void           set_initialize_default_icon_error( BaseApplication *application );
static void           set_initialize_application_error( BaseApplication *application );

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

	g_debug( "%s", thisfn );

	g_type_init();

	type = g_type_register_static( G_TYPE_OBJECT, "BaseApplication", &info, 0 );

	return( type );
}

static void
class_init( BaseApplicationClass *klass )
{
	static const gchar *thisfn = "base_application_class_init";
	GObjectClass *object_class;
	GParamSpec *spec;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;
	object_class->get_property = instance_get_property;
	object_class->set_property = instance_set_property;

	spec = g_param_spec_int(
			BASE_APPLICATION_PROP_ARGC,
			"Command-line arguments count",
			"Command-line arguments count", 0, 65535, 0,
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_ARGC_ID, spec );

	spec = g_param_spec_pointer(
			BASE_APPLICATION_PROP_ARGV,
			"Command-line arguments",
			"A pointer to command-line arguments",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_ARGV_ID, spec );

	spec = g_param_spec_pointer(
			BASE_APPLICATION_PROP_OPTIONS,
			"Command-line options",
			"A pointer to command-line options",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_OPTIONS_ID, spec );

	spec = g_param_spec_boolean(
			BASE_APPLICATION_PROP_IS_GTK_INITIALIZED,
			"Gtk+ initialization flag",
			"Has Gtk+ be initialized ?", FALSE,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_IS_GTK_INITIALIZED_ID, spec );

	spec = g_param_spec_pointer(
			BASE_APPLICATION_PROP_UNIQUE_APP_HANDLE,
			"UniqueApp object pointer",
			"A reference to the UniqueApp object if any",
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_UNIQUE_APP_HANDLE_ID, spec );

	spec = g_param_spec_int(
			BASE_APPLICATION_PROP_EXIT_CODE,
			"Exit code",
			"Exit code of the application", 0, 65535, 0,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_EXIT_CODE_ID, spec );

	spec = g_param_spec_string(
			BASE_APPLICATION_PROP_EXIT_MESSAGE1,
			"Error message",
			"First line of the error message displayed when exit_code not nul", "",
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_EXIT_MESSAGE1_ID, spec );

	spec = g_param_spec_string(
			BASE_APPLICATION_PROP_EXIT_MESSAGE2,
			"Error message",
			"Second line of the error message displayed when exit_code not nul", "",
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_EXIT_MESSAGE2_ID, spec );

	spec = g_param_spec_pointer(
			BASE_APPLICATION_PROP_BUILDER,
			"UI object pointer",
			"A reference to the UI definition from GtkBuilder",
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_BUILDER_ID, spec );

	spec = g_param_spec_pointer(
			BASE_APPLICATION_PROP_MAIN_WINDOW,
			"Main BaseWindow object",
			"A reference to the main BaseWindow object",
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_APPLICATION_PROP_MAIN_WINDOW_ID, spec );

	klass->private = g_new0( BaseApplicationClassPrivate, 1 );

	klass->run = application_do_run;
	klass->initialize = application_do_initialize;
	klass->initialize_i18n = application_do_initialize_i18n;
	klass->initialize_gtk = application_do_initialize_gtk;
	klass->manage_options = application_do_manage_options;
	klass->initialize_application_name = application_do_initialize_application_name;
	klass->initialize_session_manager = application_do_initialize_session_manager;
	klass->initialize_unique_app = application_do_initialize_unique_app;
	klass->initialize_ui = application_do_initialize_ui;
	klass->initialize_default_icon = application_do_initialize_default_icon;
	klass->initialize_application = application_do_initialize_application;
	klass->get_application_name = NULL;
	klass->get_icon_name = NULL;
	klass->get_unique_app_name = NULL;
	klass->get_ui_filename = NULL;
	klass->get_main_window = NULL;
}

static void
instance_init( GTypeInstance *application, gpointer klass )
{
	static const gchar *thisfn = "base_application_instance_init";
	BaseApplication *self;

	g_debug( "%s: application=%p (%s), klass=%p",
			thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ), ( void * ) klass );

	g_return_if_fail( BASE_IS_APPLICATION( application ));
	self = BASE_APPLICATION( application );

	self->private = g_new0( BaseApplicationPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->exit_code = 0;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	BaseApplication *self;

	g_return_if_fail( BASE_IS_APPLICATION( object ));
	self = BASE_APPLICATION( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_APPLICATION_PROP_ARGC_ID:
				g_value_set_int( value, self->private->argc );
				break;

			case BASE_APPLICATION_PROP_ARGV_ID:
				g_value_set_pointer( value, self->private->argv );
				break;

			case BASE_APPLICATION_PROP_OPTIONS_ID:
				g_value_set_pointer( value, self->private->options );
				break;

			case BASE_APPLICATION_PROP_IS_GTK_INITIALIZED_ID:
				g_value_set_boolean( value, self->private->is_gtk_initialized );
				break;

			case BASE_APPLICATION_PROP_UNIQUE_APP_HANDLE_ID:
				g_value_set_pointer( value, self->private->unique_app_handle );
				break;

			case BASE_APPLICATION_PROP_EXIT_CODE_ID:
				g_value_set_int( value, self->private->exit_code );
				break;

			case BASE_APPLICATION_PROP_EXIT_MESSAGE1_ID:
				g_value_set_string( value, self->private->exit_message1 );
				break;

			case BASE_APPLICATION_PROP_EXIT_MESSAGE2_ID:
				g_value_set_string( value, self->private->exit_message2 );
				break;

			case BASE_APPLICATION_PROP_BUILDER_ID:
				g_value_set_pointer( value, self->private->builder );
				break;

			case BASE_APPLICATION_PROP_MAIN_WINDOW_ID:
				g_value_set_pointer( value, self->private->main_window );
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
			case BASE_APPLICATION_PROP_ARGC_ID:
				self->private->argc = g_value_get_int( value );
				break;

			case BASE_APPLICATION_PROP_ARGV_ID:
				self->private->argv = g_value_get_pointer( value );
				break;

			case BASE_APPLICATION_PROP_OPTIONS_ID:
				self->private->options = g_value_get_pointer( value );
				break;

			case BASE_APPLICATION_PROP_IS_GTK_INITIALIZED_ID:
				self->private->is_gtk_initialized = g_value_get_boolean( value );
				break;

			case BASE_APPLICATION_PROP_UNIQUE_APP_HANDLE_ID:
				self->private->unique_app_handle = g_value_get_pointer( value );
				break;

			case BASE_APPLICATION_PROP_EXIT_CODE_ID:
				self->private->exit_code = g_value_get_int( value );
				break;

			case BASE_APPLICATION_PROP_EXIT_MESSAGE1_ID:
				g_free( self->private->exit_message1 );
				self->private->exit_message1 = g_value_dup_string( value );
				break;

			case BASE_APPLICATION_PROP_EXIT_MESSAGE2_ID:
				g_free( self->private->exit_message2 );
				self->private->exit_message2 = g_value_dup_string( value );
				break;

			case BASE_APPLICATION_PROP_BUILDER_ID:
				self->private->builder = g_value_get_pointer( value );
				break;

			case BASE_APPLICATION_PROP_MAIN_WINDOW_ID:
				self->private->main_window = g_value_get_pointer( value );
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

	g_debug( "%s: application=%p (%s)", thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ));
	g_return_if_fail( BASE_IS_APPLICATION( application ));
	self = BASE_APPLICATION( application );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		if( UNIQUE_IS_APP( self->private->unique_app_handle )){
			g_object_unref( self->private->unique_app_handle );
		}

		if( GTK_IS_BUILDER( self->private->builder )){
			g_object_unref( self->private->builder );
		}

		if( self->private->sm_client ){
			g_object_unref( self->private->sm_client );
		}

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

	g_debug( "%s: application=%p", thisfn, ( void * ) application );
	g_return_if_fail( BASE_IS_APPLICATION( application ));
	self = BASE_APPLICATION( application );

	g_free( self->private->exit_message1 );
	g_free( self->private->exit_message2 );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( application );
	}
}

/**
 * base_application_run:
 * @application: this #BaseApplication instance.
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
base_application_run( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_run";
	int code = -1;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), -1 );

	if( !application->private->dispose_has_run ){
		code = BASE_APPLICATION_GET_CLASS( application )->run( application );
	}

	return( code );
}

/**
 * base_application_get_application_name:
 * @application: this #BaseApplication instance.
 *
 * Asks the #BaseApplication-derived class for its localized
 * application name.
 *
 * Defaults to empty.
 *
 * Returns: a newly allocated string to be g_free() by the caller.
 */
gchar *
base_application_get_application_name( BaseApplication *application )
{
	/*static const gchar *thisfn = "base_application_get_application_name";
	g_debug( "%s: application=%p", thisfn, application );*/
	gchar *name = NULL;

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){
		if( BASE_APPLICATION_GET_CLASS( application )->get_application_name ){
			name = BASE_APPLICATION_GET_CLASS( application )->get_application_name( application );

		} else {
			name = g_strdup( "" );
		}
	}

	return( name );
}

/**
 * base_application_get_builder:
 * @application: this #BaseApplication instance.
 *
 * Returns: the default #BaseBuilder object for the application.
 */
BaseBuilder *
base_application_get_builder( BaseApplication *application )
{
	/*static const gchar *thisfn = "base_application_get_application_name";
	g_debug( "%s: application=%p", thisfn, application );*/
	BaseBuilder *builder = NULL;

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){
		builder = application->private->builder;
	}

	return( builder );
}

/**
 * base_application_get_icon_name:
 * @application: this #BaseApplication instance.
 *
 * Asks the #BaseApplication-derived class for its default icon name.
 *
 * Defaults to empty.
 *
 * Returns: a newly allocated string to be g_free() by the caller.
 */
gchar *
base_application_get_icon_name( BaseApplication *application )
{
	/*static const gchar *thisfn = "base_application_get_icon_name";
	g_debug( "%s: icon=%p", thisfn, application );*/
	gchar *name = NULL;

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){
		if( BASE_APPLICATION_GET_CLASS( application )->get_icon_name ){
			name = BASE_APPLICATION_GET_CLASS( application )->get_icon_name( application );

		} else {
			name = g_strdup( "" );
		}
	}

	return( name );
}

/**
 * base_application_get_unique_app_name:
 * @application: this #BaseApplication instance.
 *
 * Asks the #BaseApplication-derived class for its UniqueApp name if any.
 *
 * Defaults to empty.
 *
 * Returns: a newly allocated string to be g_free() by the caller.
 */
gchar *
base_application_get_unique_app_name( BaseApplication *application )
{
	/*static const gchar *thisfn = "base_application_get_unique_app_name";
	g_debug( "%s: icon=%p", thisfn, application );*/
	gchar *name = NULL;

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){
		if( BASE_APPLICATION_GET_CLASS( application )->get_unique_app_name ){
			name = BASE_APPLICATION_GET_CLASS( application )->get_unique_app_name( application );

		} else {
			name = g_strdup( "" );
		}
	}

	return( name );
}

/**
 * base_application_get_ui_filename:
 * @application: this #BaseApplication instance.
 *
 * Asks the #BaseApplication-derived class for the filename of the file
 * which contains the XML definition of the user interface.
 *
 * Defaults to empty.
 *
 * Returns: a newly allocated string to be g_free() by the caller.
 */
gchar *
base_application_get_ui_filename( BaseApplication *application )
{
	/*static const gchar *thisfn = "base_application_get_ui_filename";
	g_debug( "%s: icon=%p", thisfn, application );*/
	gchar *name = NULL;

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){
		if( BASE_APPLICATION_GET_CLASS( application )->get_ui_filename ){
			name = BASE_APPLICATION_GET_CLASS( application )->get_ui_filename( application );

		} else {
			name = g_strdup( "" );
		}
	}

	return( name );
}

/**
 * base_application_get_main_window:
 * @application: this #BaseApplication instance.
 *
 * Returns: a pointer to the #BaseWindow-derived object which serves as
 * the main window of the application.
 *
 * The returned pointer is owned by @application, and thus should not be
 * g_free() nor g_object_unref() by the caller.
 *
 * When first called, #BaseApplication asks for its derived class to
 * allocate a new object. This same object is then returned on
 * subsequent calls.
 */
BaseWindow *
base_application_get_main_window( BaseApplication *application )
{
	/*static const gchar *thisfn = "base_application_get_main_window";
	g_debug( "%s: application=%p", thisfn, application );*/

	g_return_val_if_fail( BASE_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){
		if( !application->private->main_window &&
			BASE_APPLICATION_GET_CLASS( application )->get_main_window ){
				application->private->main_window = BASE_WINDOW( BASE_APPLICATION_GET_CLASS( application )->get_main_window( application ));
		}
	}

	return( application->private->main_window );
}

/**
 * base_application_message_dlg:
 * @application: this #BaseApplication instance.
 * @message: the message to be displayed.
 *
 * Displays a dialog with only an OK button.
 */
void
base_application_message_dlg( BaseApplication *application, GSList *msg )
{
	GString *string;
	GSList *im;

	if( !application->private->dispose_has_run ){

		string = g_string_new( "" );
		for( im = msg ; im ; im = im->next ){
			if( g_utf8_strlen( string->str, -1 )){
				string = g_string_append( string, "\n" );
			}
			string = g_string_append( string, ( gchar * ) im->data );
		}
		display_dlg( application, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, string->str, NULL );

		g_string_free( string, TRUE );
	}
}

/**
 * base_application_error_dlg:
 * @application: this #BaseApplication instance.
 * @type:
 * @primary: a first message.
 * @secondaru: a second message.
 *
 * Displays an error dialog with only an OK button.
 */
void
base_application_error_dlg( BaseApplication *application,
							GtkMessageType type,
							const gchar *first,
							const gchar *second )
{
	if( !application->private->dispose_has_run ){
		display_dlg( application, type, GTK_BUTTONS_OK, first, second );
	}
}

/**
 * base_application_yesno_dlg:
 * @application: this #BaseApplication instance.
 * @type:
 * @primary: a first message.
 * @secondaru: a second message.
 *
 * Displays a choice dialog, with Yes and No buttons.
 * No button is the default.
 *
 * Returns: %TRUE if user has clicked on Yes button, %FALSE else.
 */
gboolean
base_application_yesno_dlg( BaseApplication *application, GtkMessageType type, const gchar *first, const gchar *second )
{
	gboolean ret = FALSE;
	gint result;

	if( !application->private->dispose_has_run ){

		result = display_dlg( application, type, GTK_BUTTONS_YES_NO, first, second );
		ret = ( result == GTK_RESPONSE_YES );
	}

	return( ret );
}

static gboolean
v_initialize( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize";

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	return( BASE_APPLICATION_GET_CLASS( application )->initialize( application ));
}

static gboolean
v_initialize_i18n( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_i18n";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->initialize_i18n( application );

	if( !ok ){
		set_initialize_i18n_error( application );
	}

	return( ok );
}

static gboolean
v_initialize_gtk( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_gtk";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->initialize_gtk( application );

	if( ok ){
		application->private->is_gtk_initialized = TRUE;

	} else {
		set_initialize_gtk_error( application );
	}

	return( ok );
}

static gboolean
v_manage_options( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_manage_options";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->manage_options( application );

	return( ok );
}

static gboolean
v_initialize_application_name( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_application_name";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->initialize_application_name( application );

	return( ok );
}

static gboolean
v_initialize_session_manager( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_session_manager";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->initialize_session_manager( application );

	return( ok );
}

static gboolean
v_initialize_unique_app( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_unique_app";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->initialize_unique_app( application );

	if( !ok ){
		set_initialize_unique_app_error( application );
	}

	return( ok );
}

static gboolean
v_initialize_ui( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_ui";

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	return( BASE_APPLICATION_GET_CLASS( application )->initialize_ui( application ));
}

static gboolean
v_initialize_default_icon( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_default_icon";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->initialize_default_icon( application );

	if( !ok ){
		set_initialize_default_icon_error( application );
	}

	return( ok );
}

static gboolean
v_initialize_application( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_v_initialize_application";
	gboolean ok;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ok = BASE_APPLICATION_GET_CLASS( application )->initialize_application( application );

	if( !ok ){
		set_initialize_application_error( application );
	}

	return( ok );
}

static int
application_do_run( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_run";
	GtkWindow *wnd;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	if( v_initialize( application )){

		g_return_val_if_fail( application->private->main_window, -1 );
		g_return_val_if_fail( BASE_IS_WINDOW( application->private->main_window ), -1 );

		if( base_window_init( application->private->main_window )){

			wnd = base_window_get_toplevel( application->private->main_window );
			g_assert( wnd );
			g_assert( GTK_IS_WINDOW( wnd ));

			if( application->private->unique_app_handle ){
				unique_app_watch_window( application->private->unique_app_handle, wnd );
			}

			base_window_run( application->private->main_window );
		}
	}

	display_error_message( application );

	return( application->private->exit_code );
}

static gboolean
application_do_initialize( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize";

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	return(
			v_initialize_i18n( application ) &&
			v_initialize_application_name( application ) &&
			v_initialize_gtk( application ) &&
			v_manage_options( application ) &&
			v_initialize_session_manager( application ) &&
			v_initialize_unique_app( application ) &&
			v_initialize_ui( application ) &&
			v_initialize_default_icon( application ) &&
			v_initialize_application( application )
	);
}

static gboolean
application_do_initialize_i18n( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_i18n";

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

static gboolean
application_do_initialize_gtk( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_gtk";
	int argc;
	gpointer argv;
	gboolean ret;
	GOptionEntry *options;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	g_object_get( G_OBJECT( application ),
			BASE_APPLICATION_PROP_ARGC, &argc,
			BASE_APPLICATION_PROP_ARGV, &argv,
			BASE_APPLICATION_PROP_OPTIONS, &options,
			NULL );

	if( options ){
		ret = init_with_args( application, &argc, ( char *** ) &argv, options );
	} else {
		ret = gtk_init_check( &argc, ( char *** ) &argv );
	}

	if( ret ){
		application->private->argc = argc;
		application->private->argv = argv;
	}

	return( ret );
}

static gboolean
application_do_manage_options( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_manage_options";

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	return( TRUE );
}

static gboolean
application_do_initialize_application_name( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_application_name";
	gchar *name;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	name = base_application_get_application_name( application );
	if( name && g_utf8_strlen( name, -1 )){
		g_set_application_name( name );
	}
	g_free( name );

	return( TRUE );
}

static gboolean
application_do_initialize_session_manager( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_session_manager";
	gboolean ret = TRUE;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	egg_sm_client_set_mode( EGG_SM_CLIENT_MODE_NO_RESTART );
	application->private->sm_client = egg_sm_client_get();
	egg_sm_client_startup();
	g_debug( "%s: sm_client=%p", thisfn, ( void * ) application->private->sm_client );

	g_signal_connect(
			application->private->sm_client,
	        "quit-requested",
	        G_CALLBACK( client_quit_requested_cb ),
	        application );

	g_signal_connect(
			application->private->sm_client,
	        "quit",
	        G_CALLBACK( client_quit_cb ),
	        application );

	return( ret );
}

static gboolean
application_do_initialize_unique_app( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_unique_app";
	gboolean ret = TRUE;
	gchar *name;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	name = base_application_get_unique_app_name( application );

	if( name && strlen( name )){
		application->private->unique_app_handle = unique_app_new( name, NULL );
		ret = check_for_unique_app( application );
	}

	g_free( name );
	return( ret );
}

static gboolean
application_do_initialize_ui( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_ui";
	gboolean ret = TRUE;
	GError *error = NULL;
	gchar *name;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	name = base_application_get_ui_filename( application );

	if( !name || !strlen( name )){
		ret = FALSE;
		set_initialize_ui_get_fname_error( application );

	} else {
		application->private->builder = base_builder_new();
		if( !base_builder_add_from_file( application->private->builder, name, &error )){
			ret = FALSE;
			set_initialize_ui_add_xml_error( application, name, error );
			if( error ){
				g_error_free( error );
			}
		}
	}

	g_free( name );
	return( ret );
}

static gboolean
application_do_initialize_default_icon( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_default_icon";
	gchar *name;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	name = base_application_get_icon_name( application );

	if( name && strlen( name )){
		gtk_window_set_default_icon_name( name );
	}

	g_free( name );

	return( TRUE );
}

static gboolean
application_do_initialize_application( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_do_initialize_application";
	BaseWindow *window;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	window = base_application_get_main_window( application );

	return( window != NULL );
}

static gboolean
check_for_unique_app( BaseApplication *application )
{
	static const gchar *thisfn = "base_application_check_for_unique_app";
	gboolean is_first = TRUE;

	g_debug( "%s: application=%p", thisfn, ( void * ) application );
	g_assert( BASE_IS_APPLICATION( application ));

	if( unique_app_is_running( application->private->unique_app_handle )){

		is_first = FALSE;

		unique_app_send_message( application->private->unique_app_handle, UNIQUE_ACTIVATE, NULL );

	/* default from libunique is actually to activate the first window
	 * so we rely on the default..
	 */
	/*} else {
		g_signal_connect(
				application->private->unique,
				"message-received",
				G_CALLBACK( on_unique_message_received ),
				application
		);*/
	}

	return( is_first );
}

/*static UniqueResponse
on_unique_message_received(
		UniqueApp *app, UniqueCommand command, UniqueMessageData *message, guint time, gpointer user_data )
{
	static const gchar *thisfn = "base_application_check_for_unique_app";
	UniqueResponse resp = UNIQUE_RESPONSE_OK;

	switch( command ){
		case UNIQUE_ACTIVATE:
			g_debug( "%s: received message UNIQUE_ACTIVATE", thisfn );
			break;
		default:
			resp = UNIQUE_RESPONSE_PASSTHROUGH;
			break;
	}

	return( resp );
}*/

static void
client_quit_cb( EggSMClient *client, BaseApplication *application )
{
	static const gchar *thisfn = "base_application_client_quit_cb";

	g_debug( "%s: client=%p, application=%p", thisfn, ( void * ) client, ( void * ) application );

	if( BASE_IS_WINDOW( application->private->main_window )){
		g_object_unref( application->private->main_window );
		application->private->main_window = NULL;
	}
}

static void
client_quit_requested_cb( EggSMClient *client, BaseApplication *application )
{
	static const gchar *thisfn = "base_application_client_quit_requested_cb";
	gboolean willing_to = TRUE;

	g_debug( "%s: client=%p, application=%p", thisfn, ( void * ) client, ( void * ) application );

	if( BASE_IS_WINDOW( application->private->main_window )){
		willing_to = base_window_is_willing_to_quit( application->private->main_window );
	}

	egg_sm_client_will_quit( client, willing_to );
}

static gint
display_dlg( BaseApplication *application, GtkMessageType type_message, GtkButtonsType type_buttons, const gchar *first, const gchar *second )
{
	GtkWidget *dialog;
	const gchar *name;
	gint result;

	g_assert( BASE_IS_APPLICATION( application ));

	dialog = gtk_message_dialog_new( NULL, GTK_DIALOG_MODAL, type_message, type_buttons, "%s", first );

	if( second && g_utf8_strlen( second, -1 )){
		gtk_message_dialog_format_secondary_text( GTK_MESSAGE_DIALOG( dialog ), "%s", second );
	}

	name = g_get_application_name();

	g_object_set( G_OBJECT( dialog ) , "title", name, NULL );

	result = gtk_dialog_run( GTK_DIALOG( dialog ));

	gtk_widget_destroy( dialog );

	return( result );
}

static void
display_error_message( BaseApplication *application )
{
	if( application->private->exit_message1 && g_utf8_strlen( application->private->exit_message1, -1 )){

		if( application->private->is_gtk_initialized ){
			base_application_error_dlg(
					application,
					GTK_MESSAGE_INFO,
					application->private->exit_message1,
					application->private->exit_message2 );

		} else {
			g_printf( "%s\n", application->private->exit_message1 );
			if( application->private->exit_message2 && g_utf8_strlen( application->private->exit_message2, -1 )){
				g_printf( "%s\n", application->private->exit_message2 );
			}
		}
	}
}

static gboolean
init_with_args( BaseApplication *application, int *argc, char ***argv, GOptionEntry *entries )
{
	static const gchar *thisfn = "base_application_init_with_args";
	gboolean ret;
	char *parameter_string;
	GError *error;

	parameter_string = g_strdup( g_get_application_name());
	error = NULL;

	ret = gtk_init_with_args( argc, argv, parameter_string, entries, GETTEXT_PACKAGE, &error );
	if( error ){
		g_warning( "%s: %s", thisfn, error->message );
		g_error_free( error );
		ret = FALSE;
	}

	g_free( parameter_string );

	return( ret );
}

static void
set_initialize_i18n_error( BaseApplication *application )
{
	application->private->exit_code = BASE_APPLICATION_ERROR_I18N;

	application->private->exit_message1 =
		g_strdup( _( "Unable to initialize the internationalization environment." ));
}

static void
set_initialize_gtk_error( BaseApplication *application )
{
	application->private->exit_code = BASE_APPLICATION_ERROR_GTK;

	application->private->exit_message1 =
		g_strdup( _( "Unable to initialize the Gtk+ user interface." ));
}

static void
set_initialize_unique_app_error( BaseApplication *application )
{
	application->private->exit_code = BASE_APPLICATION_ERROR_UNIQUE_APP;

	application->private->exit_message1 =
		g_strdup( _( "Another instance of the application is already running." ));
}

static void
set_initialize_ui_get_fname_error( BaseApplication *application )
{
	application->private->exit_code = BASE_APPLICATION_ERROR_UI_FNAME;

	application->private->exit_message1 =
		g_strdup( _( "No filename provided for the UI XML definition." ));
}

static void
set_initialize_ui_add_xml_error( BaseApplication *application, const gchar *filename, GError *error )
{
	application->private->exit_code = BASE_APPLICATION_ERROR_UI_LOAD;

	application->private->exit_message1 =
		/* i18n: Unable to load the XML definition from <filename> */
		g_strdup_printf( _( "Unable to load the XML definition from %s." ), filename );

	if( error && error->message ){
		application->private->exit_message2 = g_strdup( error->message );
	}
}

static void
set_initialize_default_icon_error( BaseApplication *application )
{
	application->private->exit_code = BASE_APPLICATION_ERROR_DEFAULT_ICON;

	application->private->exit_message1 =
		g_strdup( _( "Unable to set the default icon for the application." ));
}

static void
set_initialize_application_error( BaseApplication *application )
{
	application->private->exit_code = BASE_APPLICATION_ERROR_MAIN_WINDOW;

	application->private->exit_message1 =
		g_strdup( _( "Unable to get the main window of the application." ));
}
