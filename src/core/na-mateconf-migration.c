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

#include "na-mateconf-migration.h"

#define MIGRATION_COMMAND				PKGLIBEXECDIR "/na-mateconf2key.sh -delete -nodummy -verbose"

/**
 * na_mateconf_migration_run:
 *
 * Migrate users actions and menus from MateConf to .desktop files.
 * Disable MateConf I/O provider both for reading and writing.
 * Migrate users preferences to NASettings.
 *
 * Since: 3.1
 */
void
na_mateconf_migration_run( void )
{
	static const gchar *thisfn = "na_mateconf_migration_run";
#ifdef HAVE_MATECONF
	gchar *out, *err;
	GError *error;

	g_debug( "%s: running %s", thisfn, MIGRATION_COMMAND );

	error = NULL;
	if( !g_spawn_command_line_sync( MIGRATION_COMMAND, &out, &err, NULL, &error )){
		g_warning( "%s: %s", thisfn, error->message );
		g_error_free( error );
		error = NULL;

	} else {
		g_debug( "%s: out=%s", thisfn, out );
		g_debug( "%s: err=%s", thisfn, err );
		g_free( out );
		g_free( err );
	}
#else
	g_debug( "%s: MateConf support is disabled, no migration", thisfn );
#endif /* HAVE_MATECONF */
}
