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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_PROFILE_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_PROFILE_H__

/**
 * SECTION: object-profile
 * @title: NAObjectProfile
 * @short_description: The Action Profile Class Definition
 * @include: caja-actions/na-object-item.h
 */

#include "na-object-id.h"

G_BEGIN_DECLS

#define NA_TYPE_OBJECT_PROFILE                ( na_object_profile_get_type())
#define NA_OBJECT_PROFILE( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_OBJECT_PROFILE, NAObjectProfile ))
#define NA_OBJECT_PROFILE_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_OBJECT_PROFILE, NAObjectProfileClass ))
#define NA_IS_OBJECT_PROFILE( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_OBJECT_PROFILE ))
#define NA_IS_OBJECT_PROFILE_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_OBJECT_PROFILE ))
#define NA_OBJECT_PROFILE_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_OBJECT_PROFILE, NAObjectProfileClass ))

typedef struct _NAObjectProfilePrivate        NAObjectProfilePrivate;

typedef struct {
	/*< private >*/
	NAObjectId              parent;
	NAObjectProfilePrivate *private;
}
	NAObjectProfile;

typedef struct _NAObjectProfileClassPrivate   NAObjectProfileClassPrivate;

typedef struct {
	/*< private >*/
	NAObjectIdClass              parent;
	NAObjectProfileClassPrivate *private;
}
	NAObjectProfileClass;

enum {
	NA_EXECUTION_MODE_NORMAL = 1,
	NA_EXECUTION_MODE_TERMINAL,
	NA_EXECUTION_MODE_EMBEDDED,
	NA_EXECUTION_MODE_DISPLAY_OUTPUT
};

GType            na_object_profile_get_type( void );

NAObjectProfile *na_object_profile_new( void );
NAObjectProfile *na_object_profile_new_with_defaults( void );

void             na_object_profile_convert_v2_to_last( NAObjectProfile *profile );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_PROFILE_H__ */
