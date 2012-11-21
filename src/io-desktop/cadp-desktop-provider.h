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

#ifndef __CADP_DESKTOP_PROVIDER_H__
#define __CADP_DESKTOP_PROVIDER_H__

/**
 * SECTION: cadp_desktop_provider
 * @short_description: #CappDesktopProvider class definition.
 * @include: cadp-desktop-provider.h
 *
 * This class manages .desktop files I/O storage subsystem, or, in
 * other words, .desktop files as NAIIOProvider providers. As this, it
 * should only be used through the NAIIOProvider interface.
 */

#include <api/na-object-item.h>
#include <api/na-timeout.h>

#include "cadp-desktop-file.h"

G_BEGIN_DECLS

#define CADP_TYPE_DESKTOP_PROVIDER                ( cadp_desktop_provider_get_type())
#define CADP_DESKTOP_PROVIDER( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CADP_TYPE_DESKTOP_PROVIDER, CappDesktopProvider ))
#define CADP_DESKTOP_PROVIDER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CADP_TYPE_DESKTOP_PROVIDER, CappDesktopProviderClass ))
#define CADP_IS_DESKTOP_PROVIDER( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CADP_TYPE_DESKTOP_PROVIDER ))
#define CADP_IS_DESKTOP_PROVIDER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CADP_TYPE_DESKTOP_PROVIDER ))
#define CADP_DESKTOP_PROVIDER_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CADP_TYPE_DESKTOP_PROVIDER, CappDesktopProviderClass ))

/* private instance data
 */
typedef struct _CappDesktopProviderPrivate {
	/*< private >*/
	gboolean  dispose_has_run;
	GList    *monitors;
	NATimeout timeout;
}
	CappDesktopProviderPrivate;

typedef struct {
	/*< private >*/
	GObject                     parent;
	CappDesktopProviderPrivate *private;
}
	CappDesktopProvider;

typedef struct _CappDesktopProviderClassPrivate   CappDesktopProviderClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass                     parent;
	CappDesktopProviderClassPrivate *private;
}
	CappDesktopProviderClass;

/* this is a ':'-separated list of XDG_DATA_DIRS/subdirs searched for
 * menus or actions .desktop files.
 */
#define CADP_DESKTOP_PROVIDER_SUBDIRS	"file-manager/actions"

GType cadp_desktop_provider_get_type     ( void );
void  cadp_desktop_provider_register_type( GTypeModule *module );

void  cadp_desktop_provider_add_monitor     ( CappDesktopProvider *provider, const gchar *dir );
void  cadp_desktop_provider_on_monitor_event( CappDesktopProvider *provider );
void  cadp_desktop_provider_release_monitors( CappDesktopProvider *provider );

G_END_DECLS

#endif /* __CADP_DESKTOP_PROVIDER_H__ */
