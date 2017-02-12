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

#include <string.h>
#include <syslog.h>

#include <libcaja-extension/caja-extension-types.h>

#include "na-tracker.h"

static void set_log_handler( void );
static void log_handler( const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data );

static GLogFunc st_default_log_func = NULL;

/*
 * A caja extension must implement three functions :
 *
 * - caja_module_initialize
 * - caja_module_list_types
 * - caja_module_shutdown
 *
 * The first two functions are called at caja startup.
 *
 * The prototypes for these functions are defined in caja-extension-types.h
 */

void
caja_module_initialize( GTypeModule *module )
{
	static const gchar *thisfn = "caja_module_initialize";

	syslog( LOG_USER | LOG_INFO, "[N-A] %s Tracker %s initializing...", PACKAGE_NAME, PACKAGE_VERSION );

	set_log_handler();

	g_debug( "%s: module=%p", thisfn, ( void * ) module );

	g_type_module_set_name( module, PACKAGE_STRING );

	na_tracker_register_type( module );
}

void
caja_module_list_types( const GType **types, int *num_types )
{
	static const gchar *thisfn = "caja_module_list_types";
	static GType type_list[1];

	g_debug( "%s: types=%p, num_types=%p", thisfn, ( void * ) types, ( void * ) num_types );

	type_list[0] = NA_TYPE_TRACKER;
	*types = type_list;
	*num_types = 1;
}

void
caja_module_shutdown( void )
{
	static const gchar *thisfn = "caja_module_shutdown";

	g_debug( "%s", thisfn );

	/* remove the log handler
	 * almost useless as the process is nonetheless terminating at this time
	 * but this is the art of coding...
	 */
	if( st_default_log_func ){
		g_log_set_default_handler( st_default_log_func, NULL );
		st_default_log_func = NULL;
	}
}

/*
 * a log handler that we install when in development mode in order to be
 * able to log plugin runtime
 * TODO: the debug flag should be dynamic, so that an advanced user could
 * setup a given key and obtain a full log to send to Bugzilla..
 * For now, is always install when compiled in maintainer mode, never else
 */
static void
set_log_handler( void )
{
	st_default_log_func = g_log_set_default_handler(( GLogFunc ) log_handler, NULL );
}

/*
 * we used to install a log handler for each and every log domain used
 * in Caja-Actions ; this led to a fastidious enumeration
 * instead we install a default log handler which will receive all
 * debug messages, i.e. not only from N-A, but also from other code
 * in the Caja process
 */
static void
log_handler( const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data )
{
	gchar *tmp;

	tmp = g_strdup( "" );
	if( log_domain && strlen( log_domain )){
		g_free( tmp );
		tmp = g_strdup_printf( "[%s] ", log_domain );
	}

#ifdef NA_MAINTAINER_MODE
	/*( *st_default_log_func )( log_domain, log_level, message, user_data );*/
	syslog( LOG_USER | LOG_DEBUG, "%s%s", tmp, message );
#else
	if( g_getenv( CAJA_ACTIONS_DEBUG )){
		syslog( LOG_USER | LOG_DEBUG, "%s%s", tmp, message );
	}
#endif

	g_free( tmp );
}
