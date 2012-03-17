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

#ifndef __NADP_DESKTOP_PROVIDER_H__
#define __NADP_DESKTOP_PROVIDER_H__

/**
 * SECTION: nadp_desktop_provider
 * @short_description: #NadpDesktopProvider class definition.
 * @include: nadp-desktop-provider.h
 *
 * This class manages .desktop files I/O storage subsystem, or, in
 * other words, .desktop files as NAIIOProvider providers. As this, it
 * should only be used through the NAIIOProvider interface.
 */

#include <api/na-object-item.h>

#include "nadp-desktop-file.h"

G_BEGIN_DECLS

#define NADP_DESKTOP_PROVIDER_TYPE					( nadp_desktop_provider_get_type())
#define NADP_DESKTOP_PROVIDER( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NADP_DESKTOP_PROVIDER_TYPE, NadpDesktopProvider ))
#define NADP_DESKTOP_PROVIDER_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NADP_DESKTOP_PROVIDER_TYPE, NadpDesktopProviderClass ))
#define NADP_IS_DESKTOP_PROVIDER( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NADP_DESKTOP_PROVIDER_TYPE ))
#define NADP_IS_DESKTOP_PROVIDER_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NADP_DESKTOP_PROVIDER_TYPE ))
#define NADP_DESKTOP_PROVIDER_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NADP_DESKTOP_PROVIDER_TYPE, NadpDesktopProviderClass ))

typedef struct NadpDesktopProviderPrivate      NadpDesktopProviderPrivate;

/* private instance data
 */
struct NadpDesktopProviderPrivate {
	gboolean dispose_has_run;
};

typedef struct {
	GObject                     parent;
	NadpDesktopProviderPrivate *private;
}
	NadpDesktopProvider;

typedef struct NadpDesktopProviderClassPrivate NadpDesktopProviderClassPrivate;

typedef struct {
	GObjectClass                     parent;
	NadpDesktopProviderClassPrivate *private;
}
	NadpDesktopProviderClass;

/* this is a ':'-separated list of XDG_DATA_DIRS/subdirs searched for
 * menus or actions .desktop files.
 */
#define NADP_DESKTOP_PROVIDER_SUBDIRS		"file-manager/actions"

GType nadp_desktop_provider_get_type     ( void );

void  nadp_desktop_provider_register_type( GTypeModule *module );

G_END_DECLS

#endif /* __NADP_DESKTOP_PROVIDER_H__ */
