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

#ifndef __CORE_NA_IABOUT_H__
#define __CORE_NA_IABOUT_H__

/**
 * SECTION: na_iabout
 * @short_description: NAIAbout interface definition.
 * @include: runtime/na-iabout.h
 *
 * This interface displays the 'About Caja Actions' dialog box.
 * The application name may be provided by the implementor ; else,
 * the name of the application will be displayed.
 */

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define NA_IABOUT_TYPE						( na_iabout_get_type())
#define NA_IABOUT( object )					( G_TYPE_CHECK_INSTANCE_CAST( object, NA_IABOUT_TYPE, NAIAbout ))
#define NA_IS_IABOUT( object )				( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_IABOUT_TYPE ))
#define NA_IABOUT_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_IABOUT_TYPE, NAIAboutInterface ))

typedef struct NAIAbout                 NAIAbout;

typedef struct NAIAboutInterfacePrivate NAIAboutInterfacePrivate;

typedef struct {
	GTypeInterface            parent;
	NAIAboutInterfacePrivate *private;

	/**
	 * get_application_name:
	 * @iabout: this #NAIAbout implementor.
	 *
	 * Returns the application name as a newly allocated string.
	 *
	 * The application name will be g_free() by the interface.
	 */
	gchar *     ( *get_application_name )( NAIAbout *instance );

	/**
	 * get_toplevel:
	 * @iabout: this #NAIAbout implementor.
	 *
	 * Returns the toplevel parent of the displayed dialog box.
	 */
	GtkWindow * ( *get_toplevel )        ( NAIAbout *instance );
}
	NAIAboutInterface;

GType  na_iabout_get_type( void );

void   na_iabout_display( NAIAbout *instance );

gchar *na_iabout_get_icon_name( void );
gchar *na_iabout_get_copyright( gboolean console );

G_END_DECLS

#endif /* __CORE_NA_IABOUT_H__ */
