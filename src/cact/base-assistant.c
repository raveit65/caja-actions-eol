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

#include <glib.h>
#include <glib/gi18n.h>

#include "base-application.h"
#include "base-assistant.h"
#include "base-keysyms.h"

/* private class data
 */
struct _BaseAssistantClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _BaseAssistantPrivate {
	gboolean dispose_has_run;

	/* properties
	 */
	gboolean quit_on_escape;
	gboolean warn_on_escape;

	/* internals
	 */
	gboolean escape_key_pressed;
};

/* instance properties
 */
enum {
	BASE_PROP_0,

	BASE_PROP_QUIT_ON_ESCAPE_ID,
	BASE_PROP_WARN_ON_ESCAPE_ID,

	BASE_PROP_N_PROPERTIES
};

static BaseWindowClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( BaseAssistantClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void     instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void     instance_constructed( GObject *window );
static void     instance_dispose( GObject *window );
static void     instance_finalize( GObject *window );

static void     on_initialize_base_window( BaseAssistant *window );
static int      do_run( BaseWindow *window );
static gboolean on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseAssistant *assistant );
static void     on_apply( GtkAssistant *assistant, BaseAssistant *window );
static void     on_prepare( GtkAssistant *assistant, GtkWidget *page, BaseAssistant *window );
static void     on_cancel( GtkAssistant *assistant, BaseAssistant *window );
static void     on_close( GtkAssistant *assistant, BaseAssistant *window );
static void     do_close( BaseAssistant *window, GtkAssistant *assistant );

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

	type = g_type_register_static( BASE_TYPE_WINDOW, "BaseAssistant", &info, 0 );

	return( type );
}

static void
class_init( BaseAssistantClass *klass )
{
	static const gchar *thisfn = "base_assistant_class_init";
	GObjectClass *object_class;
	BaseWindowClass *base_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->constructed = instance_constructed;
	object_class->get_property = instance_get_property;
	object_class->set_property = instance_set_property;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	g_object_class_install_property( object_class, BASE_PROP_QUIT_ON_ESCAPE_ID,
			g_param_spec_boolean(
					BASE_PROP_QUIT_ON_ESCAPE,
					_( "Quit on Escape" ),
					_( "Should the assistant 'Quit' when the user hits Escape ?" ),
					FALSE,
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, BASE_PROP_WARN_ON_ESCAPE_ID,
			g_param_spec_boolean(
					BASE_PROP_WARN_ON_ESCAPE,
					_( "Warn on Escape" ),
					_( "Should the user be asked to confirm when exiting the assistant via Escape ?" ),
					FALSE,
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	base_class = BASE_WINDOW_CLASS( klass );
	base_class->run = do_run;

	klass->private = g_new0( BaseAssistantClassPrivate, 1 );
	klass->apply = NULL;
	klass->prepare = NULL;
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "base_assistant_instance_init";
	BaseAssistant *self;

	g_return_if_fail( BASE_IS_ASSISTANT( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = BASE_ASSISTANT( instance );

	self->private = g_new0( BaseAssistantPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->quit_on_escape = FALSE;
	self->private->warn_on_escape = FALSE;
	self->private->escape_key_pressed = FALSE;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	BaseAssistant *self;

	g_return_if_fail( BASE_IS_ASSISTANT( object ));

	self = BASE_ASSISTANT( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_PROP_QUIT_ON_ESCAPE_ID:
				g_value_set_boolean( value, self->private->quit_on_escape );
				break;

			case BASE_PROP_WARN_ON_ESCAPE_ID:
				g_value_set_boolean( value, self->private->warn_on_escape );
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

	g_return_if_fail( BASE_IS_ASSISTANT( object ));

	self = BASE_ASSISTANT( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case BASE_PROP_QUIT_ON_ESCAPE_ID:
				self->private->quit_on_escape = g_value_get_boolean( value );
				break;

			case BASE_PROP_WARN_ON_ESCAPE_ID:
				self->private->warn_on_escape = g_value_get_boolean( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_constructed( GObject *window )
{
	static const gchar *thisfn = "base_assistant_instance_constructed";
	BaseAssistantPrivate *priv;

	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	priv = BASE_ASSISTANT( window )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( window );
		}

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				BASE_SIGNAL_INITIALIZE_WINDOW,
				G_CALLBACK( on_initialize_base_window ));
	}
}

static void
instance_dispose( GObject *window )
{
	static const gchar *thisfn = "base_assistant_instance_dispose";
	BaseAssistantPrivate *priv;

	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	priv = BASE_ASSISTANT( window )->private;

	if( !priv->dispose_has_run ){
		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window  ));

		priv->dispose_has_run = TRUE;

		gtk_main_quit();

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
	BaseAssistantPrivate *priv;

	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window  ));

	priv = BASE_ASSISTANT( window )->private;

	g_free( priv );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( window );
	}
}

static void
on_initialize_base_window( BaseAssistant *window )
{
	static const gchar *thisfn = "base_assistant_on_initialize_base_window";
	GtkWindow *toplevel;

	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

	if( !window->private->dispose_has_run ){

		toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( window ));
		g_return_if_fail( GTK_IS_ASSISTANT( toplevel ));

		/* deals with 'Esc' key
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"key-press-event",
				G_CALLBACK( on_key_pressed_event ));

		/* transforms 'prepare' and 'apply' messages into virtual methods
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"prepare",
				G_CALLBACK( on_prepare ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"apply",
				G_CALLBACK( on_apply ));

		/* close the assistant
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"cancel",
				G_CALLBACK( on_cancel ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( toplevel ),
				"close",
				G_CALLBACK( on_close ));
	}
}

/*
 * run the assistant in a main event loop, which will be quitted
 * at dispose time
 */
static int
do_run( BaseWindow *window )
{
	static const gchar *thisfn = "base_assistant_do_run";
	int code;

	g_return_val_if_fail( BASE_IS_ASSISTANT( window ), BASE_EXIT_CODE_PROGRAM );

	code = BASE_EXIT_CODE_INIT_WINDOW;

	if( !BASE_ASSISTANT( window )->private->dispose_has_run ){

		g_debug( "%s: window=%p (%s), starting gtk_main",
				thisfn,
				( void * ) window, G_OBJECT_TYPE_NAME( window ));

		gtk_main();

		code = BASE_EXIT_CODE_OK;
	}

	return( code );
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseAssistant *assistant )
{
	gboolean stop = FALSE;
	GtkWindow *toplevel;

	g_return_val_if_fail( BASE_IS_ASSISTANT( assistant ), FALSE );

	if( !assistant->private->dispose_has_run ){

		if( event->keyval == CACT_KEY_Escape && assistant->private->quit_on_escape ){

				assistant->private->escape_key_pressed = TRUE;
				toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( assistant ));
				g_signal_emit_by_name( toplevel, "cancel", toplevel );
				stop = TRUE;
		}
	}

	return( stop );
}

static void
on_prepare( GtkAssistant *assistant, GtkWidget *page, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( BASE_ASSISTANT_GET_CLASS( window )->prepare ){
		BASE_ASSISTANT_GET_CLASS( window )->prepare( window, assistant, page );
	}
}

static void
on_apply( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	if( BASE_ASSISTANT_GET_CLASS( window )->apply ){
		BASE_ASSISTANT_GET_CLASS( window )->apply( window, assistant );
	}
}

/*
 * either the 'Cancel' button has been clicked
 * or the 'cancel' message has been emitted on the toplevel GtkAssistant
 * due to the 'Escape' key being pressed and 'quit-on-cancel' property
 * is true
 */
static void
on_cancel( GtkAssistant *assistant, BaseAssistant *window )
{
	static const gchar *thisfn = "base_assistant_on_cancel";
	gboolean ok = TRUE;
	gchar *msg;

	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	g_debug( "%s: window=%p, assistant=%p", thisfn, ( void * ) window, ( void * ) assistant );

	if( window->private->warn_on_escape && window->private->escape_key_pressed ){

		msg = g_strdup( _( "Are you sure you want to quit this assistant ?" ));
		ok = base_window_display_yesno_dlg( BASE_WINDOW( window ), msg, NULL );
		g_free( msg );
	}

	window->private->escape_key_pressed = FALSE;

	if( ok ){
		do_close( window, assistant );
	}
}

static void
on_close( GtkAssistant *assistant, BaseAssistant *window )
{
	g_return_if_fail( BASE_IS_ASSISTANT( window ));

	do_close( window, assistant );
}

static void
do_close( BaseAssistant *window, GtkAssistant *assistant )
{
	static const gchar *thisfn = "base_assistant_do_close";

	g_debug( "%s: window=%p, assistant=%p", thisfn, ( void * ) window, ( void * ) assistant );

	g_object_unref( window );
}
