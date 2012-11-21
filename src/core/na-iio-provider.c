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

#include <api/na-iio-provider.h>

#include "na-io-provider.h"

/* private interface data
 */
struct _NAIIOProviderInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* signals
 */
enum {
	ITEM_CHANGED,
	LAST_SIGNAL
};

static guint st_initializations = 0;	/* interface initialization count */
static gint  st_signals[ LAST_SIGNAL ] = { 0 };

static GType    register_type( void );
static void     interface_base_init( NAIIOProviderInterface *klass );
static void     interface_base_finalize( NAIIOProviderInterface *klass );

static gboolean do_is_willing_to_write( const NAIIOProvider *instance );
static gboolean do_is_able_to_write( const NAIIOProvider *instance );

/**
 * na_iio_provider_get_type:
 *
 * Returns: the #GType type of this interface.
 */
GType
na_iio_provider_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

/*
 * na_iio_provider_register_type:
 *
 * Registers this interface.
 */
static GType
register_type( void )
{
	static const gchar *thisfn = "na_iio_provider_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIIOProviderInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIIOProvider", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIIOProviderInterface *klass )
{
	static const gchar *thisfn = "na_iio_provider_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass%p (%s)", thisfn, ( void * ) klass, G_OBJECT_CLASS_NAME( klass ));

		klass->private = g_new0( NAIIOProviderInterfacePrivate, 1 );

		klass->get_id = NULL;
		klass->get_version = NULL;
		klass->read_items = NULL;
		klass->is_willing_to_write = do_is_willing_to_write;
		klass->is_able_to_write = do_is_able_to_write;
		klass->write_item = NULL;
		klass->delete_item = NULL;
		klass->duplicate_data = NULL;

		/**
		 * NAIIOProvider::io-provider-item-changed:
		 * @provider: the #NAIIOProvider which has called the
		 *  na_iio_provider_item_changed() function.
		 *
		 * This signal is registered without any default handler.
		 *
		 * This signal is not meant to be directly sent by a plugin.
		 * Instead, the plugin should call the na_iio_provider_item_changed()
		 * function.
		 *
		 * See also na_iio_provider_item_changed().
		 */
		st_signals[ ITEM_CHANGED ] = g_signal_new(
					IO_PROVIDER_SIGNAL_ITEM_CHANGED,
					NA_TYPE_IIO_PROVIDER,
					G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
					0,									/* class offset */
					NULL,								/* accumulator */
					NULL,								/* accumulator data */
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( NAIIOProviderInterface *klass )
{
	static const gchar *thisfn = "na_iio_provider_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

static gboolean
do_is_willing_to_write( const NAIIOProvider *instance )
{
	return( FALSE );
}

static gboolean
do_is_able_to_write( const NAIIOProvider *instance )
{
	return( FALSE );
}

/**
 * na_iio_provider_item_changed:
 * @instance: the calling #NAIIOProvider.
 *
 * Informs &prodname; that this #NAIIOProvider @instance has
 * detected a modification in (at least) one of its items (menu
 * or action).
 *
 * This function may be triggered for each and every
 * #NAObjectItem -derived modified object, and should, at least, be
 * triggered once for a coherent set of updates.
 *
 * When receiving this signal, the currently running program may just
 * want to immediately reload the current list of items, menus and actions
 * (this is for example what &caja; plugin does); it may also choose
 * to ask the user if he is willing to reload such a current list (and
 * this is the way &cact; has chosen to deal with this message).
 *
 * Note that application NAPivot/NAUpdater pivot is typically the only
 * object connected to this signal. It acts so as a filtering proxy,
 * re-emitting its own 'items-changed' signal for a whole set of detected
 * underlying modifications.
 *
 * Since: 2.30
 */
void
na_iio_provider_item_changed( const NAIIOProvider *instance )
{
	static const gchar *thisfn = "na_iio_provider_item_changed";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );

	g_signal_emit_by_name(( gpointer ) instance, IO_PROVIDER_SIGNAL_ITEM_CHANGED );
}
