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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "nact-main-tab.h"

/**
 * nact_main_tab_enable_page:
 * @window: the #NactMainWindow.
 * @num_page: the page number, starting from zero.
 * @enabled: whether the tab should be set sensitive or not.
 *
 * Set the sensitivity of the tab.
 */
void
nact_main_tab_enable_page( NactMainWindow *window, gint num_page, gboolean enabled )
{
	GtkNotebook *notebook;
	GtkWidget *page, *label;

	notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
	page = gtk_notebook_get_nth_page( notebook, num_page );
	gtk_widget_set_sensitive( page, enabled );

	label = gtk_notebook_get_tab_label( notebook, page );
	gtk_widget_set_sensitive( label, enabled );
}

/**
 * nact_main_tab_is_page_enabled:
 * @window: the #NactMainWindow.
 * @num_page: the page number, starting from zero.
 *
 * Returns: %TRUE if the tab is sensitive, %FALSE else.
 */
gboolean
nact_main_tab_is_page_enabled( NactMainWindow *window, gint num_page )
{
	gboolean is_sensitive;
	GtkNotebook *notebook;
	GtkWidget *page;

	notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
	page = gtk_notebook_get_nth_page( notebook, num_page );

#if(( GTK_MAJOR_VERSION > 2 ) || ( GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION > 18 ))
	is_sensitive = gtk_widget_is_sensitive( page );
#else
	is_sensitive = GTK_WIDGET_IS_SENSITIVE( page );
#endif

	g_debug( "nact_main_tab_is_page_enabled: num_page=%d, is_sensitive=%s", num_page, is_sensitive ? "True":"False" );

	return( is_sensitive );
}
