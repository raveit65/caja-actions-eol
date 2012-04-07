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

#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "base-application.h"
#include "base-assistant.h"

/* private class data
 */
struct BaseAssistantClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct BaseAssistantPrivate {
	gboolean    dispose_has_run;
	gboolean    cancel_on_escape;
	gboolean    warn_on_escape;
	gboolean    warn_on_cancel;
	gboolean    apply_has_run;
	gboolean    escape_key_pressed;
};

/* instance properties
 */
enum {
	BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE_ID = 1,
	BASE_ASSISTANT_PROP_WARN_ON_ESCAPE_ID,
	BASE_ASSISTANT_PROP_WARN_ON_CANCEL_ID
};

static BaseWindowClass *st_parent_class = NULL;

static GType      register_type( void );
static void       class_init( BaseAssistantClass *klass );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void       instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void       instance_dispose( GObject *application );
static void       instance_finalize( GObject *application );

static void       v_assistant_apply( GtkAssistant *assistant, BaseAssistant *window );
static void       v_assistant_cancel( GtkAssistant *assistant, BaseAssistant *window );
static void       v_assistant_close( GtkAssistant *assistant, BaseAssistant *window );
static void       v_assistant_prepare( GtkAssistant *assistant, GtkWidget *page, BaseAssistant *window );

static void       on_apply_message( GtkAssistant *assistant, BaseAssistant *window );
static void       on_cancel_message( GtkAssistant *assistant, BaseAssistant *window );
static void       on_close_message( GtkAssistant *assistant, BaseAssistant *window );
static void       on_prepare_message( GtkAssistant *assistant, GtkWidget *page, BaseAssistant *window );

static void       on_initial_load( BaseAssistant *window, gpointer user_data );
static void       on_runtime_init( BaseAssistant *window, gpointer user_data );
static gboolean   on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseAssistant *assistant );
static void       assistant_do_apply( BaseAssistant *window, GtkAssistant *assistant );
static void       assistant_do_cancel( BaseAssistant *window, GtkAssistant *assistant );
static void       assistant_do_close( BaseAssistant *window, GtkAssistant *assistant );
static void       assistant_do_prepare( BaseAssistant *window, GtkAssistant *assistant, GtkWidget *page );

GType
base_assistant_get_type( void )
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
	static const gchar *thisfn = "base_assistant_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( BaseAssistantClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( BaseAssistant ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_WINDOW_TYPE, "BaseAssistant", &info, 0 );

	return( type );
}

static void
class_init( BaseAssistantClass *klass )
{
	static const gchar *thisfn = "base_assistant_class_init";
	GObjectClass *object_class;
	GParamSpec *spec;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;
	object_class->get_property = instance_get_property;
	object_class->set_property = instance_set_property;

	spec = g_param_spec_boolean(
			BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE,
			"Cancel on Escape",
			"Does the assistant should 'Cancel' when the user hits Escape ?", FALSE,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE_ID, spec );

	spec = g_param_spec_boolean(
			BASE_ASSISTANT_PROP_WARN_ON_ESCAPE,
			"Warn on Escape",
			"Does the user should confirm when exiting the assistant via Escape ?", FALSE,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_ASSISTANT_PROP_WARN_ON_ESCAPE_ID, spec );

	spec = g_param_spec_boolean(
			BASE_ASSISTANT_PROP_WARN_ON_CANCEL,
			"Warn on cancel",
			"Does the user should confirm when exiting the assistant via Cancel ?", FALSE,
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, BASE_ASSISTANT_PROP_WARN_ON_CANCEL_ID, spec );

	klass->private = g_new0( BaseAssistantClassPrivate, 1 );

	klass->apply = assistant_do_apply;
	klass->cancel = assistant_do_cancel;
	klass->close = assistant_do_close;
	klass->prepare = assistant_do_prepare;
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "base_assistant_instance_init";
	BaseAssistant *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( BASE_IS_ASSISTANT( instance ));
	self = BASE_ASSISTANT( instance );

	self->private = g_new0( BaseAssistantPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->cancel_on_escape = FALSE;
	self->private->warn_on_escape = FALSE;
	self->private->warn_on_cancel = FALSE;
	self->private->apply_has_run = FALSE;
	self->private->escape_key_pressed = FALSE;

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_INITIAL_LOAD,
			G_CALLBACK( on_initial_load ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_RUNTIME_INIT,
			G_CALLBACK( on_runtime_init ));
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	BaseAssistant *self;

	g_return_if_fail( BASE_IS_ASSISTANT( object ));
	self = BASE_ASSISTANT( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE_ID:
				g_value_set_boolean( value, self->private->cancel_on_escape );
				break;

			case BASE_ASSISTANT_PROP_WARN_ON_ESCAPE_ID:
				g_value_set_boolean( value, self->private->warn_on_escape );
				break;

			case BASE_ASSISTANT_PROP_WARN_ON_CANCEL_ID:
				g_value_set_boolean( value, self->private->warn_on_cancel );
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
	BaseAssistant *self;

	g_assert( BASE_IS_ASSISTANT( object ));
	self = BASE_ASSISTANT( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE_ID:
				self->private->cancel_on_escape = g_value_get_boolean( value );
				break;

			case BASE_ASSISTANT_PROP_WARN_ON_ESCAPE_ID:
				self->private->warn_on_escape = g_value_get_boolean( value );
				break;

			case BASE_ASSISTANT_PROP_WARN_ON_CANCEL_ID:
				self->private->warn_on_cancel = g_value_get_boolean( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_dispose( GObject *window )
{
	static const gchar *thisfn = "base_assistant_instance_dispose";
	BaseAssistant *self;

	g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window  ));
	g_return_if_fail( BASE_IS_ASSISTANT( window ));
	self = BASE_ASSISTANT( window );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( window );
		}
	}
}

static void
instance_finalize( GObject *window )
{
	static const gchar *thisfn = "base_assistant_instance_finalize";
	BaseAssistant *self;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );
	g_return_if_fail( BASE_IS_ASSISTANT( window ));
	self = BASE_ASSISTANT( window );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( window );
	}
}

/**
 * base_assistant_set_cancel_on_esc:
 * @window: this #BaseAssistant instance.
 * @cancel: whether hitting 'Escape' key triggers the 'Cancel' action.
 *
 * Set 'cancel on escape' property.
 */
void
base_assistant_set_cancel_on_esc( BaseAssistant *window, gboolean cancel )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	g_object_set( G_OBJECT( window ), BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE, cancel, NULL );
}

/**
 * base_assistant_set_warn_on_esc:
 * @window: this #BaseAssistant instance.
 * @warn: whether the 'Cancel' action, when triggered by 'Escape' key,
 * should emit a warning.
 *
 * Set 'warn on escape' property.
 */
void
base_assistant_set_warn_on_esc( BaseAssistant *window, gboolean warn )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	g_object_set( G_OBJECT( window ), BASE_ASSISTANT_PROP_WARN_ON_ESCAPE, warn, NULL );
}

/**
 * base_assistant_set_warn_on_cancel:
 * @window: this #BaseAssistant instance.
 * @warn: whether the 'Cancel' action should emit a warning.
 *
 * Set 'warn on close' property.
 */
void
base_assistant_set_warn_on_cancel( BaseAssistant *window, gboolean warn )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	g_object_set( G_OBJECT( window ), BASE_ASSISTANT_PROP_WARN_ON_CANCEL, warn, NULL );
}

static void
v_assistant_apply( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( BASE_ASSISTANT_GET_CLASS( window )->apply ){
		BASE_ASSISTANT_GET_CLASS( window )->apply( window, assistant );

	} else {
		assistant_do_apply( window, assistant );
	}

	window->private->apply_has_run = TRUE;
}

static void
v_assistant_cancel( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( BASE_ASSISTANT_GET_CLASS( window )->cancel ){
		BASE_ASSISTANT_GET_CLASS( window )->cancel( window, assistant );

	} else {
		assistant_do_cancel( window, assistant );
	}
}

static void
v_assistant_close( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( BASE_ASSISTANT_GET_CLASS( window )->close ){
		BASE_ASSISTANT_GET_CLASS( window )->close( window, assistant );

	} else {
		assistant_do_close( window, assistant );
	}
}

static void
v_assistant_prepare( GtkAssistant *assistant, GtkWidget *page, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( BASE_ASSISTANT_GET_CLASS( window )->prepare ){
		BASE_ASSISTANT_GET_CLASS( window )->prepare( window, assistant, page );

	} else {
		assistant_do_prepare( window, assistant, page );
	}
}

/*
 * starting with Gtk+ 2.18, this work-around will become useless
 * so message handlers could safely be the v_xxx functions
 */
static void
on_apply_message( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( !window->private->apply_has_run ){
		v_assistant_apply( assistant, window );
	}
}

static void
on_cancel_message( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	v_assistant_cancel( assistant, window );
}

static void
on_close_message( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	v_assistant_close( assistant, window );
}

static void
on_prepare_message( GtkAssistant *assistant, GtkWidget *page, BaseAssistant *window )
{
	static const gchar *thisfn = "base_assistant_on_prepare_message";
	GtkAssistantPageType type;

	g_debug( "%s: assistant=%p, page=%p, window=%p",
			thisfn, ( void * ) assistant, ( void * ) page, ( void * ) window );
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	type = gtk_assistant_get_page_type( assistant, page );

	switch( type ){
		case GTK_ASSISTANT_PAGE_SUMMARY:
			if( !window->private->apply_has_run ){
				v_assistant_apply( assistant, window );
			}
			break;

		default:
			break;
	}

	v_assistant_prepare( assistant, page, window );
}

static void
on_initial_load( BaseAssistant *window, gpointer user_data )
{
	static const gchar *thisfn = "base_assistant_on_initial_load";

	g_debug( "%s: window=%p, user_data=%p", thisfn, ( void * ) window, ( void * ) user_data );
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( !window->private->dispose_has_run ){

		base_assistant_set_cancel_on_esc( window, FALSE );
		base_assistant_set_warn_on_esc( window, FALSE );
		base_assistant_set_warn_on_cancel( window, FALSE );
	}
}

static void
on_runtime_init( BaseAssistant *window, gpointer user_data )
{
	static const gchar *thisfn = "base_assistant_on_runtime_init";
	GtkWindow *toplevel;

	g_debug( "%s: window=%p, user_data=%p", thisfn, ( void * ) window, ( void * ) user_data );
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( !window->private->dispose_has_run ){

		toplevel = base_window_get_toplevel( BASE_WINDOW( window ));
		g_assert( GTK_IS_ASSISTANT( toplevel ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"key-press-event",
				G_CALLBACK( on_key_pressed_event ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"apply",
				G_CALLBACK( on_apply_message ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"cancel",
				G_CALLBACK( on_cancel_message ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"close",
				G_CALLBACK( on_close_message ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"prepare",
				G_CALLBACK( on_prepare_message ));
	}
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseAssistant *assistant )
{
	/*static const gchar *thisfn = "base_assistant_on_key_pressed_event";
	g_debug( "%s: widget=%p, event=%p, user_data=%p", thisfn, widget, event, user_data );*/
	gboolean stop = FALSE;
	GtkWindow *toplevel;

	g_return_val_if_fail( BASE_IS_ASSISTANT( assistant ), FALSE );

	if( !assistant->private->dispose_has_run ){

		if( event->keyval == GDK_Escape &&
			assistant->private->cancel_on_escape ){

				assistant->private->escape_key_pressed = TRUE;
				toplevel = base_window_get_toplevel( BASE_WINDOW( assistant ));
				g_signal_emit_by_name( toplevel, "cancel", toplevel );
				stop = TRUE;
		}
	}

	return( stop );
}

static void
assistant_do_apply( BaseAssistant *window, GtkAssistant *assistant )
{
	static const gchar *thisfn = "base_assistant_assistant_do_apply";

	g_debug( "%s: window=%p, assistant=%p", thisfn, ( void * ) window, ( void * ) assistant );
}

/*
 * the 'Cancel' button is clicked
 */
static void
assistant_do_cancel( BaseAssistant *window, GtkAssistant *assistant )
{
	static const gchar *thisfn = "base_assistant_assistant_do_cancel";
	gboolean ok = TRUE;
	gchar *first;

	g_debug( "%s: window=%p, assistant=%p", thisfn, ( void * ) window, ( void * ) assistant );

	if( window->private->warn_on_cancel ||
		( window->private->warn_on_escape && window->private->escape_key_pressed )){

			first = g_strdup( _( "Are you sure you want to quit this assistant ?" ));
			ok = base_window_yesno_dlg( BASE_WINDOW( window ), GTK_MESSAGE_QUESTION, first, NULL );
			g_free( first );
	}

	window->private->escape_key_pressed = FALSE;

	if( ok ){
		assistant_do_close( window, assistant );
	}
}

static void
assistant_do_close( BaseAssistant *window, GtkAssistant *assistant )
{
	static const gchar *thisfn = "base_assistant_assistant_do_close";

	g_debug( "%s: window=%p, assistant=%p", thisfn, ( void * ) window, ( void * ) assistant );

	g_object_unref( window );
}

static void
assistant_do_prepare( BaseAssistant *window, GtkAssistant *assistant, GtkWidget *page )
{
	static const gchar *thisfn = "base_assistant_assistant_do_prepare";

	g_debug( "%s: window=%p, assistant=%p, page=%p",
			thisfn, ( void * ) window, ( void * ) assistant, ( void * ) page );
}
