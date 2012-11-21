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

#include <api/na-object-profile.h>

#include "cact-main-tab.h"

static void on_tab_initialize_window( CactMainWindow *window, gpointer p_page );

/**
 * cact_main_tab_init:
 * @window: the #CactMainWindow.
 * @num_page: the page number, starting from zero.
 *
 * Common initialization of each page of the notebook.
 */
void
cact_main_tab_init( CactMainWindow *window, gint num_page )
{
	base_window_signal_connect_with_data(
			BASE_WINDOW( window ),
			G_OBJECT( window ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_tab_initialize_window ),
			GUINT_TO_POINTER( num_page ));
}

/**
 * cact_main_tab_enable_page:
 * @window: the #CactMainWindow.
 * @num_page: the page number, starting from zero.
 * @enabled: whether the tab should be set sensitive or not.
 *
 * Set the sensitivity of the tab.
 */
void
cact_main_tab_enable_page( CactMainWindow *window, gint num_page, gboolean enabled )
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
 * cact_main_tab_is_page_enabled:
 * @window: the #CactMainWindow.
 * @num_page: the page number, starting from zero.
 *
 * Returns: %TRUE if the tab is sensitive, %FALSE else.
 */
gboolean
cact_main_tab_is_page_enabled( CactMainWindow *window, gint num_page )
{
	gboolean is_sensitive;
	GtkNotebook *notebook;
	GtkWidget *page;

	notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
	page = gtk_notebook_get_nth_page( notebook, num_page );

	is_sensitive = gtk_widget_is_sensitive( page );

	g_debug( "cact_main_tab_is_page_enabled: num_page=%d, is_sensitive=%s", num_page, is_sensitive ? "True":"False" );

	return( is_sensitive );
}

/*
 * a commoon initialization for each page of the notebook
 * (provided that the page has itself called cact_main_tab_init()
 * *before* the BASE_SIGNAL_INITIALIZE_WINDOW has been emitted)
 */
static void
on_tab_initialize_window( CactMainWindow *window, gpointer p_page )
{
	GtkNotebook *notebook;
	GtkWidget *page;
	const gchar *text;
	guint num_page;

	notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
	num_page = GPOINTER_TO_UINT( p_page );
	page = gtk_notebook_get_nth_page( notebook, num_page );

	/* popup menu is enabled in CactMainWindow::on_base_initialize_window()
	 * but the displayed labels default to be those of the tab, i.e. embed
	 * an underscore as an accelerator - so get ride of this
	 */
	text = gtk_notebook_get_tab_label_text( notebook, page );
	gtk_notebook_set_menu_label_text( notebook, page, text );
}
