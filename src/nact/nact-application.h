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

#ifndef __NACT_APPLICATION_H__
#define __NACT_APPLICATION_H__

/**
 * SECTION: nact_application
 * @short_description: #NactApplication class definition.
 * @include: nact/nact-application.h
 *
 * This is the main class for caja-actions-config-tool program.
 */

#include <core/na-updater.h>

#include "base-application.h"

G_BEGIN_DECLS

#define NACT_APPLICATION_TYPE					( nact_application_get_type())
#define NACT_APPLICATION( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_APPLICATION_TYPE, NactApplication ))
#define NACT_APPLICATION_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, NACT_APPLICATION_TYPE, NactApplicationClass ))
#define NACT_IS_APPLICATION( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_APPLICATION_TYPE ))
#define NACT_IS_APPLICATION_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NACT_APPLICATION_TYPE ))
#define NACT_APPLICATION_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NACT_APPLICATION_TYPE, NactApplicationClass ))

typedef struct NactApplicationPrivate      NactApplicationPrivate;

typedef struct {
	BaseApplication         parent;
	NactApplicationPrivate *private;
}
	NactApplication;

typedef struct NactApplicationClassPrivate NactApplicationClassPrivate;

typedef struct {
	BaseApplicationClass         parent;
	NactApplicationClassPrivate *private;
}
	NactApplicationClass;

GType            nact_application_get_type( void );

NactApplication *nact_application_new_with_args( int argc, char **argv );

NAUpdater       *nact_application_get_updater( NactApplication *application );

G_END_DECLS

#endif /* __NACT_APPLICATION_H__ */
