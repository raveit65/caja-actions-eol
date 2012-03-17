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

#include <api/na-core-utils.h>

#include "nadp-xdg-dirs.h"

/**
 * nadp_xdg_dirs_get_data_dirs:
 *
 * Returns: the ordered list of data directories, most important first,
 * as a GSList of newly allocated strings.
 *
 * The returned list, along with the pointed out strings, should be
 * freed by the caller.
 */
GSList *
nadp_xdg_dirs_get_data_dirs( void )
{
	GSList *listdirs;
	gchar *userdir;
	GSList *datadirs;

	userdir = nadp_xdg_dirs_get_user_data_dir();
	listdirs = g_slist_prepend( NULL, userdir );

	datadirs = nadp_xdg_dirs_get_system_data_dirs();
	listdirs = g_slist_concat( listdirs, datadirs );

	return( listdirs );
}

/**
 * nadp_xdg_dirs_get_user_data_dir:
 *
 * Returns: the path to the single base directory relative to which
 * user-specific data files should be written, as a newly allocated
 * string.
 *
 * This directory is defined by the environment variable XDG_DATA_HOME.
 * It defaults to ~/.local/share.
 * cf. http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
 *
 * The returned string should be g_free() by the caller.
 */
gchar *
nadp_xdg_dirs_get_user_data_dir( void )
{
	gchar *dir;

	dir = g_strdup( g_get_user_data_dir());

	return( dir );
}

/**
 * nadp_xdg_dirs_get_system_data_dirs:
 *
 * Returns: the set of preference ordered base directories relative to
 * which data files should be written, as a GSList of newly allocated
 * strings.
 *
 * This set of directories is defined by the environment variable
 * XDG_DATA_DIRS. It defaults to /usr/local/share:/usr/share.
 *
 * source: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html
 *
 * The returned list, along with the pointed out strings, should be freed
 * by the caller.
 */
GSList *
nadp_xdg_dirs_get_system_data_dirs( void )
{
	const gchar **dirs;
	GSList *paths;

	dirs = ( const gchar ** ) g_get_system_data_dirs();

	paths = na_core_utils_slist_from_array( dirs );

	return( paths );
}
