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

#include <core/na-about.h>

#include "cact-menubar-priv.h"

/**
 * cact_menubar_help_on_update_sensitivities:
 * @bar: this #CactMenubar object.
 *
 * Update sensitivities on the Help menu.
 */
void
cact_menubar_help_on_update_sensitivities( const CactMenubar *bar )
{
	cact_menubar_enable_item( bar, "HelpItem", TRUE );
	/* about always enabled */
}

/**
 * cact_menubar_help_on_help:
 * @action: the #GtkAction of the item.
 * @window: the #BaseWindow main application window.
 *
 * Triggers the "Help/Help" item.
 */
void
cact_menubar_help_on_help( GtkAction *action, BaseWindow *window )
{
	static const gchar *thisfn = "cact_menubar_help_on_help";
	GError *error;

	error = NULL;
	gtk_show_uri( NULL, "ghelp:caja-actions-config-tool", GDK_CURRENT_TIME, &error );
	if( error ){
		g_warning( "%s: %s", thisfn, error->message );
		g_error_free( error );
	}
}

/**
 * cact_menubar_help_on_about:
 * @action: the #GtkAction of the item.
 * @window: the #BaseWindow main application window.
 *
 * Triggers the "Help/About" item.
 */
void
cact_menubar_help_on_about( GtkAction *action, BaseWindow *window )
{
	na_about_display( base_window_get_gtk_toplevel( window ));
}
