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

#include <api/na-extension.h>

#include "nagp-mateconf-provider.h"

/* the count of GType types provided by this extension
 * each new GType type must
 * - be registered in na_extension_startup()
 * - be addressed in na_extension_list_types().
 */
#define NAGP_TYPES_COUNT	1

/*
 * na_extension_startup:
 *
 * mandatory starting with API v. 1.
 */
gboolean
na_extension_startup( GTypeModule *module )
{
	static const gchar *thisfn = "nagp_module_na_extension_startup";

	g_debug( "%s: module=%p", thisfn, ( void * ) module );

	nagp_mateconf_provider_register_type( module );

	return( TRUE );
}

/*
 * na_extension_get_version:
 *
 * optional, defaults to 1.
 */
guint
na_extension_get_version( void )
{
	static const gchar *thisfn = "nagp_module_na_extension_get_version";
	guint version;

	version = 1;

	g_debug( "%s: version=%d", thisfn, version );

	return( version );
}

/*
 * na_extension_list_types:
 *
 * mandatory starting with v. 1.
 */
guint
na_extension_list_types( const GType **types )
{
	static const gchar *thisfn = "nagp_module_na_extension_list_types";
	static GType types_list [1+NAGP_TYPES_COUNT];

	g_debug( "%s: types=%p", thisfn, ( void * ) types );

	types_list[0] = NAGP_MATECONF_PROVIDER_TYPE;

	types_list[NAGP_TYPES_COUNT] = 0;
	*types = types_list;

	return( NAGP_TYPES_COUNT );
}

/*
 * na_extension_shutdown:
 *
 * mandatory starting with v. 1.
 */
void
na_extension_shutdown( void )
{
	static const gchar *thisfn = "nagp_module_na_extension_shutdown";

	g_debug( "%s", thisfn );
}
