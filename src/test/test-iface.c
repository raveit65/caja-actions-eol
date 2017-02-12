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

/* We verify here that derived class can also use interfaces implemented
 * in base class.
 */

#include <glib.h>

#include "test-iface-derived.h"
#include "test-iface-iface.h"

int
main( int argc, char **argv )
{
	TestBase *base, *base2;
	TestDerived *derived;

#if !GLIB_CHECK_VERSION( 2,36, 0 )
	g_type_init();
#endif

	g_debug( "allocating TestBase -------------------------------------" );
	base = test_base_new();
	g_debug( "calling test_iface_fna on Base object -------------------" );
	test_iface_fna( TEST_IFACE( base ));
	g_debug( "calling test_iface_fnb on Base object -------------------" );
	test_iface_fnb( TEST_IFACE( base ));

	g_debug( "allocating TestDerived ----------------------------------" );
	derived = test_derived_new();
	if( TEST_IS_IFACE( derived )){
		g_debug( "Derived is also an IFace" );
	} else {
		g_debug( "Derived is NOT an IFace" );
	}
	g_debug( "calling test_iface_fna on Derived object ----------------" );
	test_iface_fna( TEST_IFACE( derived ));
	g_debug( "calling test_iface_fnb on Derived object ----------------" );
	test_iface_fnb( TEST_IFACE( derived ));

	g_debug( "allocating TestBase -------------------------------------" );
	base2 = test_base_new();
	g_debug( "calling test_iface_fna on another Base object -------------------" );
	test_iface_fna( TEST_IFACE( base2 ));
	g_debug( "calling test_iface_fnb on another Base object -------------------" );
	test_iface_fnb( TEST_IFACE( base2 ));

	g_debug( "unreffing TestDerived ------------------------------------" );
	g_object_unref( derived );

	g_debug( "unreffing TestBase ------------------------------------" );
	g_object_unref( base );

	g_debug( "end -----------------------------------------------------" );

	return( 0 );
}
