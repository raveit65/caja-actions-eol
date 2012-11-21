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
#include <stdlib.h>
#include <string.h>

#include <core/na-gtk-utils.h>

#include "base-application.h"
#include "base-builder.h"
#include "base-window.h"
#include "base-gtk-utils.h"
#include "base-marshal.h"

/* private class data
 */
struct _BaseWindowClassPrivate {
	BaseBuilder     *builder;			/* common builder */
};

/* private instance data
 */
struct _BaseWindowPrivate {
	gboolean         dispose_has_run;

	/* properties
	 */
	BaseWindow      *parent;
	BaseApplication *application;
	gchar           *xmlui_filename;
	gboolean         has_own_builder;
	gchar           *toplevel_name;
	gchar           *wsp_name;
	gboolean         destroy_on_dispose;

	/* internals
	 */
	GtkWindow       *gtk_toplevel;
	gboolean         initialized;
	GList           *signals;
	BaseBuilder     *builder;
};

/* instance properties
 */
enum {
	BASE_PROP_0,

	BASE_PROP_PARENT_ID,
	BASE_PROP_APPLICATION_ID,
	BASE_PROP_XMLUI_FILENAME_ID,
	BASE_PROP_HAS_OWN_BUILDER_ID,
	BASE_PROP_TOPLEVEL_NAME_ID,
	BASE_PROP_WSP_NAME_ID,
	BASE_PROP_DESTROY_ON_DISPOSE_ID,

	BASE_PROP_N_PROPERTIES
};

/* pseudo-properties set against the Gtk toplevel
 */
typedef struct {
	gboolean initialized;
}
	BaseGtkData;

#define BASE_PROP_GTK_DATA				"base-prop-window-gtk-data"

/* signals defined in BaseWindow, to be used in all derived classes
 */
enum {
	INITIALIZE_GTK,
	INITIALIZE_BASE,
	SHOW_WIDGETS,
	LAST_SIGNAL
};

/* connected signals, to be disconnected at BaseWindow::instance_dispose()
 */
typedef struct {
	gpointer instance;
	gulong   handler_id;
}
	RecordedSignal;

static GObjectClass *st_parent_class           = NULL;
static gint          st_signals[ LAST_SIGNAL ] = { 0 };
static gboolean      st_debug_signal_connect   = FALSE;

static GType        register_type( void );
static void         class_init( BaseWindowClass *klass );
static void         instance_init( GTypeInstance *instance, gpointer klass );
static void         instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void         instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void         instance_constructed( GObject *window );
static void         instance_dispose( GObject *window );
static void         instance_finalize( GObject *window );

/* initialization process
 */
static BaseGtkData *get_base_gtk_data( BaseWindow *window );
static void         on_gtk_toplevel_finalized( gpointer user_data, GtkWindow *toplevel );
static gboolean     init_gtk_toplevel( BaseWindow *window );
static void         on_initialize_gtk_toplevel_class_handler( BaseWindow *window, GtkWindow *toplevel );
static void         do_initialize_gtk_toplevel( BaseWindow *window, GtkWindow *toplevel );
static void         on_initialize_base_window_class_handler( BaseWindow *window );
static void         do_initialize_base_window( BaseWindow *window );
static void         on_show_widgets_class_handler( BaseWindow *window );
static void         do_show_widgets( BaseWindow *window );

/* misc
 */
static void         record_connected_signal( BaseWindow *window, GObject *instance, gulong handler_id );
static gint         display_dlg( const BaseWindow *parent, GtkMessageType type_message, GtkButtonsType type_buttons, const gchar *primary, const gchar *secondary );

GType
base_window_get_type( void )
{
	static GType window_type = 0;

	if( !window_type ){
		window_type = register_type();
	}

	return( window_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "base_window_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( BaseWindowClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( BaseWindow ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "BaseWindow", &info, 0 );

	return( type );
}

static void
class_init( BaseWindowClass *klass )
{
	static const gchar *thisfn = "base_window_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->get_property = instance_get_property;
	object_class->set_property = instance_set_property;
	object_class->constructed = instance_constructed;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	g_object_class_install_property( object_class, BASE_PROP_XMLUI_FILENAME_ID,
			g_param_spec_string(
					BASE_PROP_XMLUI_FILENAME,
					_( "XML UI filename" ),
					_( "The filename which contains the XML UI definition" ),
					"",
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_HAS_OWN_BUILDER_ID,
			g_param_spec_boolean(
					BASE_PROP_HAS_OWN_BUILDER,
					_( "Has its own GtkBuilder" ),
					_( "Whether this BaseWindow reallocates a new GtkBuilder each time it is opened" ),
					FALSE,
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_TOPLEVEL_NAME_ID,
			g_param_spec_string(
					BASE_PROP_TOPLEVEL_NAME,
					_( "Toplevel name" ),
					_( "The internal GtkBuildable name of the toplevel window" ),
					"",
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_APPLICATION_ID,
			g_param_spec_pointer(
					BASE_PROP_APPLICATION,
					_( "BaseApplication" ),
					_( "A pointer (not a reference) to the BaseApplication instance" ),
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_PARENT_ID,
			g_param_spec_pointer(
					BASE_PROP_PARENT,
					_( "Parent BaseWindow" ),
					_( "A pointer (not a reference) to the BaseWindow parent of this BaseWindow" ),
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_WSP_NAME_ID,
			g_param_spec_string(
					BASE_PROP_WSP_NAME,
					_( "WSP name" ),
					_( "The string which handles the window size and position in user preferences" ),
					"",
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_DESTROY_ON_DISPOSE_ID,
			g_param_spec_boolean(
					BASE_PROP_DESTROY_ON_DISPOSE,
					_( "Destroy the Gtk toplevel" ),
					_( "Whether the embedded Gtk Toplevel should be destroyed at dispose time" ),
					FALSE,
					G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	klass->private = g_new0( BaseWindowClassPrivate, 1 );

	klass->private->builder = base_builder_new();

	klass->initialize_gtk_toplevel = do_initialize_gtk_toplevel;
	klass->initialize_base_window = do_initialize_base_window;
	klass->show_widgets = do_show_widgets;
	klass->run = NULL;

	/**
	 * base-signal-window-initialize-gtk:
	 *
	 * The signal is emitted by and on the #BaseWindow instance after it
	 * has loaded for the first time the Gtk toplevel widget from the
	 * #BaseBuilder for this window.
	 *
	 * The Gtk toplevel is initialized only once, just after it has been
	 * loaded, even if several #BaseWindow instances embed it.
	 *
	 * The toplevel GtkWindow is passed as a parameter to this signal.
	 *
	 * The class handler calls the class initialize_gtk_toplevel() virtual
	 * method.
	 *
	 * The default virtual method just does nothing.
	 */
	st_signals[ INITIALIZE_GTK ] =
		g_signal_new_class_handler(
				BASE_SIGNAL_INITIALIZE_GTK,
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				G_CALLBACK( on_initialize_gtk_toplevel_class_handler ),
				NULL,
				NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE,
				1,
				G_TYPE_POINTER );

	/**
	 * base-signal-window-initialize-window:
	 *
	 * The signal is emitted by and on the #BaseWindow instance after the
	 * toplevel GtkWindow has been initialized, before actually displaying
	 * the window.
	 *
	 * Is is so time to initialize it with runtime values.
	 *
	 * The class handler calls the class initialize_base_window() virtual
	 * method.
	 *
	 * The default virtual method set transient state of the Gtk toplevel
	 * againts its parent, and manages its size and position on the desktop.
	 * It so should really be called by the derived class.
	 */
	st_signals[ INITIALIZE_BASE ] =
		g_signal_new_class_handler(
				BASE_SIGNAL_INITIALIZE_WINDOW,
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				G_CALLBACK( on_initialize_base_window_class_handler ),
				NULL,
				NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0 );

	/**
	 * base-signal-window-show-widgets:
	 *
	 * The signal is emitted by and on the #BaseWindow instance when the
	 * toplevel widget has been initialized with its runtime values,
	 * just before showing it and all its descendants.
	 *
	 * It is typically used by notebooks, to select the first visible
	 * page.
	 *
	 * The class handler calls the class all_widgets_showed() virtual
	 * method.
	 *
	 * The default virtual method calls gtk_widget_show_all().
	 * It so should really be called by the derived class.
	 */
	st_signals[ SHOW_WIDGETS ] =
		g_signal_new_class_handler(
				BASE_SIGNAL_SHOW_WIDGETS,
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				G_CALLBACK( on_show_widgets_class_handler ),
				NULL,
				NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE,
				0 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "base_window_instance_init";
	BaseWindow *self;

	g_return_if_fail( BASE_IS_WINDOW( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = BASE_WINDOW( instance );

	self->private = g_new0( BaseWindowPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->signals = NULL;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	BaseWindow *self;

	g_return_if_fail( BASE_IS_WINDOW( object ));
	self = BASE_WINDOW( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_PROP_PARENT_ID:
				g_value_set_pointer( value, self->private->parent );
				break;

			case BASE_PROP_APPLICATION_ID:
				g_value_set_pointer( value, self->private->application );
				break;

			case BASE_PROP_XMLUI_FILENAME_ID:
				g_value_set_string( value, self->private->xmlui_filename );
				break;

			case BASE_PROP_HAS_OWN_BUILDER_ID:
				g_value_set_boolean( value, self->private->has_own_builder );
				break;

			case BASE_PROP_TOPLEVEL_NAME_ID:
				g_value_set_string( value, self->private->toplevel_name );
				break;

			case BASE_PROP_WSP_NAME_ID:
				g_value_set_string( value, self->private->wsp_name );
				break;

			case BASE_PROP_DESTROY_ON_DISPOSE_ID:
				g_value_set_boolean( value, self->private->destroy_on_dispose );
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
	BaseWindow *self;

	g_return_if_fail( BASE_IS_WINDOW( object ));
	self = BASE_WINDOW( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_PROP_PARENT_ID:
				self->private->parent = g_value_get_pointer( value );
				break;

			case BASE_PROP_APPLICATION_ID:
				self->private->application = g_value_get_pointer( value );
				break;

			case BASE_PROP_XMLUI_FILENAME_ID:
				g_free( self->private->xmlui_filename );
				self->private->xmlui_filename = g_value_dup_string( value );
				break;

			case BASE_PROP_HAS_OWN_BUILDER_ID:
				self->private->has_own_builder = g_value_get_boolean( value );
				break;

			case BASE_PROP_TOPLEVEL_NAME_ID:
				g_free( self->private->toplevel_name );
				self->private->toplevel_name = g_value_dup_string( value );
				break;

			case BASE_PROP_WSP_NAME_ID:
				g_free( self->private->wsp_name );
				self->private->wsp_name = g_value_dup_string( value );
				break;

			case BASE_PROP_DESTROY_ON_DISPOSE_ID:
				self->private->destroy_on_dispose = g_value_get_boolean( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

/*
 * it is time here to initialize the Gtk toplevel if this has not already
 * been done - We do this early in the build process, and this may trigger
 * some error conditions (mainly if the toplevel name is not found in the
 * xml ui filename)
 */
static void
instance_constructed( GObject *window )
{
	static const gchar *thisfn = "base_window_instance_constructed";
	BaseWindowPrivate *priv;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	priv = BASE_WINDOW( window )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( window );
		}

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		/* at least the BaseWindow parent or the BaseApplication application
		 * must have been provided at instanciation time
		 */
		if( !priv->application ){
			g_return_if_fail( priv->parent );
			g_return_if_fail( BASE_IS_WINDOW( priv->parent ));

			priv->application = priv->parent->private->application;
		}

		g_return_if_fail( BASE_IS_APPLICATION( priv->application ));
	}
}

static void
instance_dispose( GObject *window )
{
	static const gchar *thisfn = "base_window_instance_dispose";
	BaseWindowPrivate *priv;
	GList *is;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	priv = BASE_WINDOW( window )->private;

	if( !priv->dispose_has_run ){

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		if( priv->wsp_name && strlen( priv->wsp_name )){
			base_gtk_utils_save_window_position( BASE_WINDOW( window ), priv->wsp_name );
		}

		/* signals must be deconnected before quitting main loop
		 * (if objects are still alive)
		 */
		for( is = priv->signals ; is ; is = is->next ){
			RecordedSignal *str = ( RecordedSignal * ) is->data;
			g_debug( "%s: str=%p instance=%p", thisfn, ( void * ) str, ( void * ) str->instance );
			if( G_IS_OBJECT( str->instance )){
				if( g_signal_handler_is_connected( str->instance, str->handler_id )){
					g_signal_handler_disconnect( str->instance, str->handler_id );
					if( st_debug_signal_connect ){
						g_debug( "%s: disconnecting signal handler %p:%lu", thisfn, str->instance, str->handler_id );
					}
				}
			}
			g_free( str );
		}
		g_list_free( priv->signals );

		/* at least the main window should have this property set
		 */
		if( priv->destroy_on_dispose ){
			gtk_widget_destroy( GTK_WIDGET( priv->gtk_toplevel ));
		}

#if 0
		if( is_main_window( BASE_WINDOW( window ))){
			g_debug( "%s: quitting main window", thisfn );
			gtk_main_quit ();
			gtk_widget_destroy( GTK_WIDGET( priv->gtk_toplevel ));

		} else
			if( GTK_IS_ASSISTANT( priv->gtk_toplevel )){
			g_debug( "%s: quitting assistant", thisfn );
			gtk_main_quit();
			if( is_gtk_toplevel_initialized( BASE_WINDOW( window ), priv->gtk_toplevel )){
				gtk_widget_hide( GTK_WIDGET( priv->gtk_toplevel ));
			}

		} else {
			g_debug( "%s: quitting dialog", thisfn );
			if( is_gtk_toplevel_initialized( BASE_WINDOW( window ), priv->gtk_toplevel )){
				gtk_widget_hide( GTK_WIDGET( priv->gtk_toplevel ));
			}
		}
#endif

		/* must dispose _after_ quitting the loop
		 */
		priv->dispose_has_run = TRUE;

		/* release the Gtkbuilder, if any
		 */
		if( priv->has_own_builder ){
			if( BASE_IS_BUILDER( priv->builder )){
				g_object_unref( priv->builder );
			}
		} else {
			if( GTK_IS_WINDOW( priv->gtk_toplevel )){
				gtk_widget_hide( GTK_WIDGET( priv->gtk_toplevel ));
			}
		}

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( window );
		}
	}
}

static void
instance_finalize( GObject *window )
{
	static const gchar *thisfn = "base_window_instance_finalize";
	BaseWindow *self;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

	self = BASE_WINDOW( window );

	g_free( self->private->toplevel_name );
	g_free( self->private->xmlui_filename );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( window );
	}
}

static BaseGtkData *
get_base_gtk_data( BaseWindow *window )
{
	BaseGtkData *data;
	BaseWindowPrivate *priv;

	priv = window->private;

	g_return_val_if_fail( GTK_IS_WINDOW( priv->gtk_toplevel ), NULL );

	data = ( BaseGtkData * ) g_object_get_data( G_OBJECT( priv->gtk_toplevel ), BASE_PROP_GTK_DATA );

	if( !data ){
		data = g_new0( BaseGtkData, 1 );
		g_object_set_data( G_OBJECT( priv->gtk_toplevel ), BASE_PROP_GTK_DATA, data );

		g_object_weak_ref( G_OBJECT( priv->gtk_toplevel ), ( GWeakNotify ) on_gtk_toplevel_finalized, NULL );
	}

	return( data );
}

static void
on_gtk_toplevel_finalized( gpointer user_data, GtkWindow *toplevel )
{
	static const gchar *thisfn = "base_window_on_gtk_toplevel_finalized";
	BaseGtkData *data;

	g_debug( "%s: toplevel=%p, user_data=%p", thisfn, ( void * ) toplevel, ( void * ) user_data );

	data = ( BaseGtkData * ) g_object_get_data( G_OBJECT( toplevel ), BASE_PROP_GTK_DATA );

	g_free( data );
}

/**
 * base_window_init:
 * @window: this #BaseWindow object.
 *
 * Initialize the Gtk toplevel if needed, initialize the window at runtime,
 * and show it.
 *
 * Returns: %TRUE if the window has been successfully loaded and initialized
 * and all widgets showed, %FALSE else.
 */
gboolean
base_window_init( BaseWindow *window )
{
	static const gchar *thisfn = "base_window_init";
	BaseWindowPrivate *priv;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), FALSE );

	priv = window->private;

	if( priv->dispose_has_run ){
		return( FALSE );
	}

	if( !priv->initialized ){

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		/* allocate a dedicated BaseBuilder or use the common one
		 */
		g_debug( "%s: has_own_builder=%s", thisfn, priv->has_own_builder ? "True":"False" );

		if( priv->has_own_builder ){
			priv->builder = base_builder_new();
		} else {
			priv->builder = BASE_WINDOW_GET_CLASS( window )->private->builder;
		}

		g_return_val_if_fail( BASE_IS_BUILDER( priv->builder ), FALSE );

		/* having a builder, we load in it the XML UI definition file
		 * (if it has not been already done), and ask it to build (or retrieve)
		 * the toplevel Gtk window => this may trigger an error
		 */
		if( !init_gtk_toplevel( window )){
			return( FALSE );
		}

		g_return_val_if_fail( GTK_IS_WINDOW( priv->gtk_toplevel ), FALSE );

		g_signal_emit_by_name( window, BASE_SIGNAL_INITIALIZE_WINDOW );

		g_signal_emit_by_name( window, BASE_SIGNAL_SHOW_WIDGETS );

		priv->initialized = TRUE;
	}

	return( TRUE );
}

static gboolean
init_gtk_toplevel( BaseWindow *window )
{
	static const gchar *thisfn = "base_window_init_gtk_toplevel";
	BaseWindowPrivate *priv;
	gboolean ret;
	GError *error;
	gchar *msg;
	BaseGtkData *data;

	ret = FALSE;
	priv = window->private;
	priv->gtk_toplevel = NULL;
	error = NULL;

	/* load the XML definition from the UI file
	 * if this has not been already done
	 */
	g_debug( "%s: xmlui_filename=%s", thisfn, priv->xmlui_filename );

	if( priv->xmlui_filename &&
		g_utf8_strlen( priv->xmlui_filename, -1 ) &&
		!base_builder_add_from_file( priv->builder, priv->xmlui_filename, &error )){

			msg = g_strdup_printf(
					_( "Unable to load %s UI XML definition: %s" ),
					priv->xmlui_filename, error->message );
			base_window_display_error_dlg( NULL, thisfn, msg );
			g_free( msg );
			g_error_free( error );

	/* then build (or retrieve) the toplevel widget
	 */
	} else if( priv->toplevel_name && strlen( priv->toplevel_name )){

		priv->gtk_toplevel = base_builder_get_toplevel_by_name(
					priv->builder, priv->toplevel_name );

		if( !priv->gtk_toplevel ){
			msg = g_strdup_printf(
					_( "Unable to load %s dialog definition." ),
					priv->toplevel_name );
			base_window_display_error_dlg( NULL, msg, NULL );
			g_free( msg );

		/* eventually initialize the toplevel Gtk window
		 */
		} else {
			g_return_val_if_fail( GTK_IS_WINDOW( priv->gtk_toplevel ), FALSE );

			data = get_base_gtk_data( window );

			if( !data->initialized ){

				g_signal_emit_by_name(
						window,
						BASE_SIGNAL_INITIALIZE_GTK,
						priv->gtk_toplevel );

				data->initialized = TRUE;
			}

			ret = TRUE;
		}
	}

	return( ret );
}

/*
 * default class handler for BASE_SIGNAL_INITIALIZE_GTK signal
 *
 * let the virtual methods finalize the Gtk initialization management
 */
static void
on_initialize_gtk_toplevel_class_handler( BaseWindow *window, GtkWindow *toplevel )
{
	static const gchar *thisfn = "base_window_on_initialize_gtk_toplevel_class_handler";

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( GTK_IS_WINDOW( toplevel ));
	g_return_if_fail( toplevel == window->private->gtk_toplevel );

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p (%s), toplevel=%p (%s)",
				thisfn,
				( void * ) window, G_OBJECT_TYPE_NAME( window ),
				( void * ) toplevel, G_OBJECT_TYPE_NAME( toplevel ));

		if( BASE_WINDOW_GET_CLASS( window )->initialize_gtk_toplevel ){
			BASE_WINDOW_GET_CLASS( window )->initialize_gtk_toplevel( window, toplevel );
		}
	}
#ifdef NA_MAINTAINER_MODE
	base_window_dump_children( window );
#endif
}

static void
do_initialize_gtk_toplevel( BaseWindow *window, GtkWindow *toplevel )
{
	static const gchar *thisfn = "base_window_do_initialize_gtk_toplevel";

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( GTK_IS_WINDOW( toplevel ));
	g_return_if_fail( toplevel == window->private->gtk_toplevel );

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p (%s), toplevel=%p (%s)",
				thisfn,
				( void * ) window, G_OBJECT_TYPE_NAME( window ),
				( void * ) toplevel, G_OBJECT_TYPE_NAME( toplevel ));
	}
}

/*
 * default class handler for BASE_SIGNAL_INITIALIZE_WINDOW message
 *
 * let the virtual methods finalize the runtime initialization management
 */
static void
on_initialize_base_window_class_handler( BaseWindow *window )
{
	static const gchar *thisfn = "base_window_on_initialize_base_window_class_handler";

	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		if( BASE_WINDOW_GET_CLASS( window )->initialize_base_window ){
			BASE_WINDOW_GET_CLASS( window )->initialize_base_window( window );
		}
	}
}

static void
do_initialize_base_window( BaseWindow *window )
{
	static const gchar *thisfn = "base_window_do_initialize_base_window";
	BaseWindowPrivate *priv;
	GtkWindow *parent_toplevel;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	priv = window->private;

	if( !priv->dispose_has_run ){

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		if( priv->parent ){
			g_return_if_fail( BASE_IS_WINDOW( priv->parent ));
			parent_toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( priv->parent ));
			gtk_window_set_transient_for( priv->gtk_toplevel, parent_toplevel );
		}

		if( priv->wsp_name && strlen( priv->wsp_name )){
			base_gtk_utils_restore_window_position( window, priv->wsp_name );
		}
	}
}

/*
 * default class handler for BASE_SIGNAL_SHOW_WIDGETS message
 *
 * let the virtual methods finalize the display management
 */
static void
on_show_widgets_class_handler( BaseWindow *window )
{
	static const gchar *thisfn = "base_window_on_show_widgets_class_handler";

	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( !window->private->dispose_has_run ){
		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		if( BASE_WINDOW_GET_CLASS( window )->show_widgets ){
			BASE_WINDOW_GET_CLASS( window )->show_widgets( window );
		}
	}
}

static void
do_show_widgets( BaseWindow *window )
{
	static const gchar *thisfn = "base_window_do_show_widgets";

	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		gtk_widget_show_all( GTK_WIDGET( window->private->gtk_toplevel ));
	}
}

/**
 * base_window_run:
 * @window: this #BaseWindow object.
 *
 * Runs the window.
 *
 * This function calls the run() virtual method, and does nothing else.
 * If the derived class does not rely on the main loop, it really should
 * implement the run() virtual method.
 *
 * Returns: the exit code as set by the derived class, or:
 * - %BASE_EXIT_CODE_PROGRAM if the window has already been disposed,
 * - %BASE_EXIT_CODE_INIT_WINDOW if the window was not and cannot be
 *   loaded and initialized.
 */
int
base_window_run( BaseWindow *window )
{
	static const gchar *thisfn = "base_window_run";
	int code = BASE_EXIT_CODE_PROGRAM;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), code );

	if( !window->private->dispose_has_run ){

		if( !base_window_init( window )){
			g_debug( "%s: base_window_init() returns False", thisfn );
			code = BASE_EXIT_CODE_INIT_WINDOW;

		} else {
			g_return_val_if_fail( GTK_IS_WINDOW( window->private->gtk_toplevel ), BASE_EXIT_CODE_PROGRAM );
			g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

			code = BASE_EXIT_CODE_OK;

			if( BASE_WINDOW_GET_CLASS( window )->run ){
				code = BASE_WINDOW_GET_CLASS( window )->run( window );
			}
		}
	}

	return( code );
}

#ifdef NA_MAINTAINER_MODE
/*
 * base_window_dump_children:
 * @window: this #BaseWindow instance.
 *
 * Displays the known children of this window.
 */
void
base_window_dump_children( const BaseWindow *window )
{
	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		na_gtk_utils_dump_children( GTK_CONTAINER( window->private->gtk_toplevel ));
	}
}
#endif

/**
 * base_window_get_application:
 * @window: this #BaseWindow object.
 *
 * Returns: a pointer on the #BaseApplication object.
 *
 * The returned pointer is owned by the primary allocator of the
 * application ; it should not be g_free() nor g_object_unref() by the
 * caller.
 */
BaseApplication *
base_window_get_application( const BaseWindow *window )
{
	BaseApplication *application = NULL;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	if( !window->private->dispose_has_run ){

		application = window->private->application;
	}

	return( application );
}

/**
 * base_window_get_parent:
 * @window: this #BaseWindow instance..
 *
 * Returns the #BaseWindow parent of @window.
 *
 * The returned object is owned by @window, and should not be freed.
 */
BaseWindow *
base_window_get_parent( const BaseWindow *window )
{
	BaseWindow *parent = NULL;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	if( !window->private->dispose_has_run ){

		parent = window->private->parent;
	}

	return( parent );
}

/**
 * base_window_get_gtk_toplevel:
 * @window: this #BaseWindow instance..
 *
 * Returns the top-level GtkWindow attached to this BaseWindow object.
 *
 * The caller may close the window by g_object_unref()-ing the returned
 * #GtkWindow.
 */
GtkWindow *
base_window_get_gtk_toplevel( const BaseWindow *window )
{
	GtkWindow *gtk_toplevel = NULL;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	if( !window->private->dispose_has_run ){

		gtk_toplevel = window->private->gtk_toplevel;
	}

	return( gtk_toplevel );
}

/**
 * base_window_get_gtk_toplevel_by_name:
 * @window: this #BaseWindow instance.
 * @name: the name of the searched GtkWindow.
 *
 * Returns: the named top-level GtkWindow.
 *
 * The function searches for the toplevel first in the private builder,
 * and then in the class common builder (if not found and is not the
 * same).
 *
 * This is just a convenience function to be able to open quickly a
 * window (e.g. Legend dialog).
 *
 * The caller may close the window by g_object_unref()-ing the returned
 * #GtkWindow.
 */
GtkWindow *
base_window_get_gtk_toplevel_by_name( const BaseWindow *window, const gchar *name )
{
	GtkWindow *gtk_toplevel = NULL;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	if( !window->private->dispose_has_run ){

		gtk_toplevel = base_builder_get_toplevel_by_name( window->private->builder, name );

		if( !gtk_toplevel ){
			if( window->private->has_own_builder ){
				gtk_toplevel = base_builder_get_toplevel_by_name(
						BASE_WINDOW_GET_CLASS( window )->private->builder, name );
			}
		}
	}

	return( gtk_toplevel );
}

/**
 * base_window_get_widget:
 * @window: this #BaseWindow instance.
 * @name: the name of the searched child.
 *
 * Returns a pointer to the named widget which is a child of the
 * toplevel #GtkWindow associated with @window.
 *
 * Returns: a pointer to the searched widget, or NULL.
 * This pointer is owned by GtkBuilder instance, and must not be
 * g_free() nor g_object_unref() by the caller.
 */
GtkWidget *
base_window_get_widget( const BaseWindow *window, const gchar *name )
{
	GtkWidget *widget = NULL;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	if( !window->private->dispose_has_run ){

		widget = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( window->private->gtk_toplevel ), name );
	}

	return( widget );
}

/**
 * base_window_display_error_dlg:
 * @parent: the #BaseWindow parent, may be %NULL.
 * @primary: the primary message.
 * @secondary: the secondary message.
 *
 * Display an error dialog box, with a 'OK' button only.
 *
 * if @secondary is not null, then @primary is displayed as a bold title.
 */
void
base_window_display_error_dlg( const BaseWindow *parent, const gchar *primary, const gchar *secondary )
{
	display_dlg( parent, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, primary, secondary );
}

/**
 * base_window_display_yesno_dlg:
 * @parent: the #BaseWindow parent, may be %NULL.
 * @primary: the primary message.
 * @secondary: the secondary message.
 *
 * Display a warning dialog box, with a 'OK' button only.
 *
 * if @secondary is not null, then @primary is displayed as a bold title.
 *
 * Returns: %TRUE if the user has clicked 'Yes', %FALSE else.
 */
gboolean
base_window_display_yesno_dlg( const BaseWindow *parent, const gchar *primary, const gchar *secondary )
{
	gint result;

	result = display_dlg( parent, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, primary, secondary );

	return( result == GTK_RESPONSE_YES );
}

/**
 * base_window_display_message_dlg:
 * @parent: the #BaseWindow parent, may be %NULL.
 * @message: the message to be displayed.
 *
 * Displays an information dialog with only an OK button.
 */
void
base_window_display_message_dlg( const BaseWindow *parent, GSList *msg )
{
	GString *string;
	GSList *im;

	string = g_string_new( "" );
	for( im = msg ; im ; im = im->next ){
		if( g_utf8_strlen( string->str, -1 )){
			string = g_string_append( string, "\n" );
		}
		string = g_string_append( string, ( gchar * ) im->data );
	}
	display_dlg( parent, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, string->str, NULL );

	g_string_free( string, TRUE );
}

static gint
display_dlg( const BaseWindow *parent, GtkMessageType type_message, GtkButtonsType type_buttons, const gchar *primary, const gchar *secondary )
{
	GtkWindow *gtk_parent;
	GtkWidget *dialog;
	gint result;

	gtk_parent = NULL;
	if( parent ){
		gtk_parent = base_window_get_gtk_toplevel( parent );
	}

	dialog = gtk_message_dialog_new( gtk_parent, GTK_DIALOG_MODAL, type_message, type_buttons, "%s", primary );

	if( secondary && g_utf8_strlen( secondary, -1 )){
		gtk_message_dialog_format_secondary_text( GTK_MESSAGE_DIALOG( dialog ), "%s", secondary );
	}

	g_object_set( G_OBJECT( dialog ) , "title", g_get_application_name(), NULL );

	result = gtk_dialog_run( GTK_DIALOG( dialog ));

	gtk_widget_destroy( dialog );

	return( result );
}

/**
 * Records a connected signal, to be disconnected at CactWindow dispose.
 */
gulong
base_window_signal_connect( BaseWindow *window, GObject *instance, const gchar *signal, GCallback fn )
{
	gulong handler_id = 0;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), ( gulong ) 0 );

	if( !window->private->dispose_has_run ){

		handler_id = g_signal_connect( instance, signal, fn, window );
		record_connected_signal( window, instance, handler_id );
	}

	return( handler_id );
}

gulong
base_window_signal_connect_after( BaseWindow *window, GObject *instance, const gchar *signal, GCallback fn )
{
	gulong handler_id = 0;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), ( gulong ) 0 );

	if( !window->private->dispose_has_run ){

		handler_id = g_signal_connect_after( instance, signal, fn, window );
		record_connected_signal( window, instance, handler_id );
	}

	return( handler_id );
}

gulong
base_window_signal_connect_by_name( BaseWindow *window, const gchar *name, const gchar *signal, GCallback fn )
{
	gulong handler_id = 0;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), ( gulong ) 0 );

	if( !window->private->dispose_has_run ){

		GtkWidget *widget = base_window_get_widget( window, name );
		if( GTK_IS_WIDGET( widget )){

			handler_id = base_window_signal_connect( window, G_OBJECT( widget ), signal, fn );
		}
	}

	return( handler_id );
}

gulong
base_window_signal_connect_with_data( BaseWindow *window, GObject *instance, const gchar *signal, GCallback fn, void *user_data )
{
	gulong handler_id = 0;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), ( gulong ) 0 );

	if( !window->private->dispose_has_run ){

		handler_id = g_signal_connect( instance, signal, fn, user_data );
		record_connected_signal( window, instance, handler_id );
	}

	return( handler_id );
}

static void
record_connected_signal( BaseWindow *window, GObject *instance, gulong handler_id )
{
	static const gchar *thisfn = "base_window_record_connected_signal";

	RecordedSignal *str = g_new0( RecordedSignal, 1 );
	str->instance = instance;
	str->handler_id = handler_id;
	window->private->signals = g_list_prepend( window->private->signals, str );

	if( st_debug_signal_connect ){
		g_debug( "%s: connecting signal handler %p:%lu", thisfn, ( void * ) instance, handler_id );
	}
}

void
base_window_signal_disconnect( BaseWindow *window, gulong handler_id )
{
	GList *it;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		for( it = window->private->signals ; it ; it = it->next ){
			RecordedSignal *str = ( RecordedSignal * ) it->data;

			if( str->handler_id == handler_id ){
				g_signal_handler_disconnect( str->instance, str->handler_id );
				window->private->signals = g_list_delete_link( window->private->signals, it );
				g_free( str );
			}
		}
	}
}
