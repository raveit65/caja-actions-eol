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

#include <glib-object.h>
#include <glib/gprintf.h>
#include <stdlib.h>

#include <core/na-mate-vfs-uri.h>

static const gchar *uris[] = {
		"http://robert:azerty01@mon.domain.com/path/to/a/document?query#anchor",
		"ssh://pwi.dyndns.biz:2207",
		"sftp://kde.org:1234/pub/kde",
		"/usr/bin/caja-actions-config-tool",
		"file:///home/pierre/data/eclipse/caja-actions/AUTHORS",
		NULL
};

int
main( int argc, char** argv )
{
	int i;

	g_type_init();
	g_printf( "URIs parsing test.\n\n" );

	for( i = 0 ; uris[i] ; ++i ){
		NAMateVFSURI *vfs = g_new0( NAMateVFSURI, 1 );
		na_mate_vfs_uri_parse( vfs, uris[i] );
		g_printf( "original  uri=%s\n", uris[i] );
		g_printf( "vfs      path=%s\n", vfs->path );
		g_printf( "vfs    scheme=%s\n", vfs->scheme );
		g_printf( "vfs host_name=%s\n", vfs->host_name );
		g_printf( "vfs host_port=%d\n", vfs->host_port );
		g_printf( "vfs user_name=%s\n", vfs->user_name );
		g_printf( "vfs  password=%s\n", vfs->password );
		g_printf( "\n" );
	}

	return( EXIT_SUCCESS );
}
