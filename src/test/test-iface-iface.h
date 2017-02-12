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

#ifndef __TEST_IFACE_IFACE_H__
#define __TEST_IFACE_IFACE_H__

/**
 * SECTION: test_iface
 * @short_description: #TestIFace interface definition.
 * @include: test-iface-iface.h
 *
 * Test to see if a derived class can directly beneficiate of the
 * interface implemented by its base class, or if we have to implement
 * virtual functions in the base class.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define TEST_IFACE_TYPE						( test_iface_get_type())
#define TEST_IFACE( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, TEST_IFACE_TYPE, TestIFace ))
#define TEST_IS_IFACE( object )				( G_TYPE_CHECK_INSTANCE_TYPE( object, TEST_IFACE_TYPE ))
#define TEST_IFACE_GET_INTERFACE( instance )( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), TEST_IFACE_TYPE, TestIFaceInterface ))

typedef struct TestIFace TestIFace;

typedef struct TestIFaceInterfacePrivate TestIFaceInterfacePrivate;

typedef struct {
	GTypeInterface             parent;
	TestIFaceInterfacePrivate *private;

	/**
	 * fna:
	 * @target: the #TestIFace target of the copy.
	 * @source: the #TestIFace source of the copy
	 *
	 * Copies data from @source to @ŧarget, so that @target becomes an
	 * exact copy of @source.
	 */
	void ( *fna )( TestIFace *object );
	void ( *fnb )( TestIFace *object );
}
	TestIFaceInterface;

GType test_iface_get_type( void );

void  test_iface_fna( TestIFace *object );

void  test_iface_fnb( TestIFace *object );

G_END_DECLS

#endif /* __TEST_IFACE_IFACE_H__ */
