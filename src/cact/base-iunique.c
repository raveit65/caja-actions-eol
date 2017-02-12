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

#include <glib/gi18n.h>
#include <string.h>
#include <gio/gio.h>

#include "base-iunique.h"
#include "base-window.h"

/* private interface data
 */
struct _BaseIUniqueInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* pseudo-properties, set against the instance
 */
typedef struct {
	gchar     *unique_app_name;
	GApplication *handle;
}
	IUniqueData;

#define BASE_PROP_IUNIQUE_DATA			"base-prop-iunique-data"

static guint st_initializations = 0;	/* interface initialisation count */

static GType        register_type( void );
static void         interface_base_init( BaseIUniqueInterface *klass );
static void         interface_base_finalize( BaseIUniqueInterface *klass );

static IUniqueData *get_iunique_data( BaseIUnique *instance );
static void         on_instance_finalized( gpointer user_data, BaseIUnique *instance );

static const gchar *m_get_application_name( const BaseIUnique *instance );

GType
base_iunique_get_type( void )
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
	static const gchar *thisfn = "base_iunique_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( BaseIUniqueInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "BaseIUnique", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( BaseIUniqueInterface *klass )
{
	static const gchar *thisfn = "base_iunique_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( BaseIUniqueInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( BaseIUniqueInterface *klass )
{
	static const gchar *thisfn = "base_iunique_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

static IUniqueData *
get_iunique_data( BaseIUnique *instance )
{
	IUniqueData *data;

	data = ( IUniqueData * ) g_object_get_data( G_OBJECT( instance ), BASE_PROP_IUNIQUE_DATA );

	if( !data ){
		data = g_new0( IUniqueData, 1 );
		g_object_set_data( G_OBJECT( instance ), BASE_PROP_IUNIQUE_DATA, data );
		g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, BaseIUnique *instance )
{
	static const gchar *thisfn = "base_iunique_on_instance_finalized";
	IUniqueData *data;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );

	data = get_iunique_data( instance );

	if( data->handle ){
		g_object_unref( data->handle );
	}

	g_free( data->unique_app_name );
	g_free( data );
}

gboolean
base_iunique_init_with_name( BaseIUnique *instance, const gchar *unique_app_name )
{
	static const gchar *thisfn = "base_iunique_init_with_name";
	IUniqueData *data;
	gboolean ret;
	gboolean is_first;
	gchar *msg;
	GError *error = NULL;

	g_return_val_if_fail( BASE_IS_IUNIQUE( instance ), FALSE );

	g_debug( "%s: instance=%p, unique_app_name=%s", thisfn, ( void * ) instance, unique_app_name );

	ret = TRUE;
	data = get_iunique_data( instance );

	if( unique_app_name && strlen( unique_app_name )){

			data->handle = g_application_new( unique_app_name, G_APPLICATION_FLAGS_NONE );

			if (!g_application_register (data->handle, NULL, &error))
			{
				g_warning ("%s", error->message);
				g_error_free (error);
				return 1;
			}

			is_first = !g_application_get_is_remote( data->handle );

			if( !is_first ){
				/* i18n: application name */
				msg = g_strdup_printf(
						_( "Another instance of %s is already running.\n"
							"Please switch back to it." ),
						m_get_application_name( instance ));
				base_window_display_error_dlg( NULL, _( "The application is not unique" ), msg );
				g_free( msg );
				ret = FALSE;
			} else {
				data->unique_app_name = g_strdup( unique_app_name );
			}
	}

	return( ret );
}

static const gchar *
m_get_application_name( const BaseIUnique *instance )
{
	return( BASE_IUNIQUE_GET_INTERFACE( instance )->get_application_name( instance ));
}
