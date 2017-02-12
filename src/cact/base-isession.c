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

#include "base-isession.h"
#include "base-marshal.h"
#include "egg-sm-client.h"

/* private interface data
 */
struct _BaseISessionInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* pseudo-properties, set against the instance
 */
typedef struct {
	EggSMClient *sm_client;
	gulong       sm_client_quit_handler_id;
	gulong       sm_client_quit_requested_handler_id;
}
	ISessionData;

#define BASE_PROP_ISESSION_DATA			"base-prop-isession-data"

/* signals defined by the BaseISession interface
 */
enum {
	QUIT_REQUESTED,
	QUIT,
	LAST_SIGNAL
};

static guint st_initializations = 0;	/* interface initialisation count */
static gint  st_signals[ LAST_SIGNAL ] = { 0 };

static GType         register_type( void );
static void          interface_base_init( BaseISessionInterface *klass );
static void          interface_base_finalize( BaseISessionInterface *klass );

static ISessionData *get_isession_data( BaseISession *instance );
static void          on_instance_finalized( gpointer user_data, BaseISession *instance );
static void          client_quit_requested_cb( EggSMClient *client, BaseISession *instance );
static gboolean      on_quit_requested_class_handler( BaseISession *instance, void *user_data );
static gboolean      signal_accumulator_false_handled( GSignalInvocationHint *hint, GValue *return_accu, const GValue *handler_return, gpointer dummy);
static void          client_quit_cb( EggSMClient *client, BaseISession *instance );
static void          on_quit_class_handler( BaseISession *instance, void *user_data );

GType
base_isession_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = register_type();
	}

	return( iface_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "base_isession_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( BaseISessionInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_INTERFACE, "BaseISession", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( BaseISessionInterface *klass )
{
	static const gchar *thisfn = "base_isession_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		/**
		 * base-signal-isession-quit-requested:
		 *
		 * The signal is emitted when the session is about to terminate,
		 * to determine if all implementations are actually willing to quit.
		 *
		 * If the implementation is not willing to quit, then the user handler
		 * should return %FALSE to stop the signal emission.
		 *
		 * Returning %TRUE will let the signal be emitted to other connected
		 * handlers, eventually authorizing the application to terminate.
		 *
		 * Notes about GLib signals.
		 *
		 * If the signal is defined as G_SIGNAL_RUN_CLEANUP, then the object
		 * handler does not participate to the return value of the signal
		 * (accumulator is not called). More this object handler is triggered
		 * unconditionnally, event if a user handler has returned %FALSE to
		 * stop the emission.
		 *
		 * Contrarily, if the signal is defined as G_SIGNAL_RUN_LAST, then the
		 * object handler returned value is taken into account by the accumulator,
		 * and can participate to the return value for the signal. If a user
		 * handler returns FALSE to stop the emission, then the object handler
		 * will not be triggered.
		 */
		st_signals[ QUIT_REQUESTED ] =
			g_signal_new_class_handler(
					BASE_SIGNAL_QUIT_REQUESTED,
					G_TYPE_FROM_CLASS( klass ),
					G_SIGNAL_RUN_LAST,
					G_CALLBACK( on_quit_requested_class_handler ),
					( GSignalAccumulator ) signal_accumulator_false_handled,
					NULL,
					base_cclosure_marshal_BOOLEAN__VOID,
					G_TYPE_BOOLEAN,
					0 );

		/**
		 * base-signal-isession-quit:
		 *
		 * The signal is emitted when the session is about to terminate,
		 * and no application has refused to abort then end of session
		 * (all have accepted the end). It is time for the application
		 * to terminate cleanly.
		 */
		st_signals[ QUIT ] =
			g_signal_new_class_handler(
					BASE_SIGNAL_QUIT,
					G_TYPE_FROM_CLASS( klass ),
					G_SIGNAL_RUN_LAST,
					G_CALLBACK( on_quit_class_handler ),
					NULL,
					NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0 );

		klass->private = g_new0( BaseISessionInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( BaseISessionInterface *klass )
{
	static const gchar *thisfn = "base_isession_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

static ISessionData *
get_isession_data( BaseISession *instance )
{
	ISessionData *data;

	data = ( ISessionData * ) g_object_get_data( G_OBJECT( instance ), BASE_PROP_ISESSION_DATA );

	if( !data ){
		data = g_new0( ISessionData, 1 );
		g_object_set_data( G_OBJECT( instance ), BASE_PROP_ISESSION_DATA, data );
		g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, BaseISession *instance )
{
	static const gchar *thisfn = "base_isession_on_instance_finalized";
	ISessionData *data;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );

	data = get_isession_data( instance );

	if( data->sm_client_quit_handler_id &&
		g_signal_handler_is_connected( data->sm_client, data->sm_client_quit_handler_id )){
			g_signal_handler_disconnect( data->sm_client, data->sm_client_quit_handler_id  );
	}

	if( data->sm_client_quit_requested_handler_id &&
		g_signal_handler_is_connected( data->sm_client, data->sm_client_quit_requested_handler_id )){
			g_signal_handler_disconnect( data->sm_client, data->sm_client_quit_requested_handler_id  );
	}

	if( data->sm_client ){
		g_object_unref( data->sm_client );
	}

	g_free( data );
}

void
base_isession_init( BaseISession *instance )
{
	static const gchar *thisfn = "base_isession_init";
	ISessionData *data;

	g_return_if_fail( BASE_IS_ISESSION( instance ));

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );

	data = get_isession_data( instance );

	/* initialize the session manager
	 */
	egg_sm_client_set_mode( EGG_SM_CLIENT_MODE_NO_RESTART );
	data->sm_client = egg_sm_client_get();
	egg_sm_client_startup();
	g_debug( "%s: sm_client=%p", thisfn, ( void * ) data->sm_client );

	data->sm_client_quit_handler_id =
			g_signal_connect(
					data->sm_client,
					"quit-requested",
					G_CALLBACK( client_quit_requested_cb ),
					instance );

	data->sm_client_quit_requested_handler_id =
			g_signal_connect(
					data->sm_client,
					"quit",
					G_CALLBACK( client_quit_cb ),
					instance );
}

/*
 * base_isession_is_willing_to_quit:
 * @instance: this #BaseISession instance.
 *
 * Returns: %TRUE if the implementation is willing to quit, %FALSE else.
 *
 * From the session management strict point of view, this function may be
 * just static and reserved for the own internal use of the interface.
 * We make it public because it is also useful for the application itself,
 * and we are so able to reuse both the signal and its handler mechanisms.
 */
gboolean
base_isession_is_willing_to_quit( const BaseISession *instance )
{
	static const gchar *thisfn = "base_isession_is_willing_to_quit";
	GValue instance_params = {0};
	GValue return_value = {0};

	g_return_val_if_fail( BASE_IS_ISESSION( instance ), TRUE );

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	g_value_init( &instance_params, G_TYPE_FROM_INSTANCE( instance ));
	g_value_set_instance( &instance_params, ( gpointer ) instance );

	g_value_init( &return_value, G_TYPE_BOOLEAN );
	g_value_set_boolean( &return_value, TRUE );

	g_signal_emitv( &instance_params, st_signals[ QUIT_REQUESTED ], 0, &return_value );

	return( g_value_get_boolean( &return_value ));
}

/*
 * the session manager advertises us that the session is about to exit
 *
 * Returns: %TRUE if the application is willing to quit, %FALSE else.
 *
 * This function is called when the session manager detects the end of
 * session and thus asks its clients if they are willing to quit.
 */
static void
client_quit_requested_cb( EggSMClient *client, BaseISession *instance )
{
	static const gchar *thisfn = "base_isession_client_quit_requested_cb";
	gboolean willing_to;

	g_debug( "%s: client=%p, instance=%p (%s)",
			thisfn,
			( void * ) client,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	willing_to = base_isession_is_willing_to_quit( instance );

	egg_sm_client_will_quit( client, willing_to );
}

/*
 * Handler of BASE_SIGNAL_QUIT_REQUESTED message
 *
 * Application should handle this signal in order to trap application
 * termination, returning %FALSE if it is not willing to quit.
 */
static gboolean
on_quit_requested_class_handler( BaseISession *instance, void *user_data )
{
	static const gchar *thisfn = "base_isession_on_quit_requested_class_handler";

	g_return_val_if_fail( BASE_IS_ISESSION( instance ), TRUE );

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	return( TRUE );
}

/*
 * the first handler which returns FALSE stops the emission
 * this is used on BASE_SIGNAL_QUIT_REQUESTED signal
 */
static gboolean
signal_accumulator_false_handled(
		GSignalInvocationHint *hint, GValue *return_accu, const GValue *handler_return, gpointer dummy)
{
	static const gchar *thisfn = "base_isession_signal_accumulator_false_handled";
	gboolean continue_emission;
	gboolean willing_to_quit;

	willing_to_quit = g_value_get_boolean( handler_return );
	g_value_set_boolean( return_accu, willing_to_quit );
	continue_emission = willing_to_quit;

	g_debug( "%s: willing_to handler returns %s", thisfn, willing_to_quit ? "True":"False" );
	return( continue_emission );
}

/*
 * cleanly terminate the application when exiting the session
 * -> triggers the implementation
 */
static void
client_quit_cb( EggSMClient *client, BaseISession *instance )
{
	static const gchar *thisfn = "base_isession_client_quit_cb";

	g_return_if_fail( BASE_IS_ISESSION( instance ));

	g_debug( "%s: client=%p, instance=%p", thisfn, ( void * ) client, ( void * ) instance );

	g_signal_emit_by_name( G_OBJECT( instance ), BASE_SIGNAL_QUIT, NULL );
}

static void
on_quit_class_handler( BaseISession *instance, void *user_data )
{
	static const gchar *thisfn = "base_isession_on_quit_class_handler";

	g_return_if_fail( BASE_IS_ISESSION( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );
}
