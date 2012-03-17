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

#ifndef __TEST_IFACE_BASE_H__
#define __TEST_IFACE_BASE_H__

/**
 * SECTION: test_iface_base
 * @short_description: #TestBase class definition.
 * @include: test-iface-base.h
 *
 * A base class which implements TestIFace interface.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define TEST_BASE_TYPE					( test_base_get_type())
#define TEST_BASE( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, TEST_BASE_TYPE, TestBase ))
#define TEST_BASE_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, TEST_BASE_TYPE, TestBaseClass ))
#define TEST_IS_BASE( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, TEST_BASE_TYPE ))
#define TEST_IS_BASE_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), TEST_BASE_TYPE ))
#define TEST_BASE_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), TEST_BASE_TYPE, TestBaseClass ))

typedef struct TestBasePrivate TestBasePrivate;

typedef struct {
	GObject          parent;
	TestBasePrivate *private;
}
	TestBase;

typedef struct TestBaseClassPrivate TestBaseClassPrivate;

typedef struct {
	GObjectClass          parent;
	TestBaseClassPrivate *private;
}
	TestBaseClass;

GType     test_base_get_type( void );

TestBase *test_base_new( void );

G_END_DECLS

#endif /* __TEST_IFACE_BASE_H__ */
