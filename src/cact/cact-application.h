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

#ifndef __CACT_APPLICATION_H__
#define __CACT_APPLICATION_H__

/**
 * SECTION: cact_application
 * @short_description: #CactApplication class definition.
 * @include: cact/cact-application.h
 *
 * This is the main class for caja-actions-config-tool program.
 */

#include <core/na-updater.h>

#include "base-application.h"

G_BEGIN_DECLS

#define CACT_APPLICATION_TYPE					( cact_application_get_type())
#define CACT_APPLICATION( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_APPLICATION_TYPE, CactApplication ))
#define CACT_APPLICATION_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, CACT_APPLICATION_TYPE, CactApplicationClass ))
#define CACT_IS_APPLICATION( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_APPLICATION_TYPE ))
#define CACT_IS_APPLICATION_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_APPLICATION_TYPE ))
#define CACT_APPLICATION_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_APPLICATION_TYPE, CactApplicationClass ))

typedef struct CactApplicationPrivate      CactApplicationPrivate;

typedef struct {
	BaseApplication         parent;
	CactApplicationPrivate *private;
}
	CactApplication;

typedef struct CactApplicationClassPrivate CactApplicationClassPrivate;

typedef struct {
	BaseApplicationClass         parent;
	CactApplicationClassPrivate *private;
}
	CactApplicationClass;

GType            cact_application_get_type( void );

CactApplication *cact_application_new_with_args( int argc, char **argv );

NAUpdater       *cact_application_get_updater( CactApplication *application );

G_END_DECLS

#endif /* __CACT_APPLICATION_H__ */
