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

#ifndef __CORE_API_NA_GTK_UTILS_H__
#define __CORE_API_NA_GTK_UTILS_H__

/* @title: GTK+
 * @short_description: The Gtk+ Library Utilities.
 * @include: core/na-gtk-utils.h
 */

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* widget hierarchy
 */
GtkWidget *na_gtk_utils_find_widget_by_name( GtkContainer *container, const gchar *name );

#ifdef NA_MAINTAINER_MODE
void       na_gtk_utils_dump_children          ( GtkContainer *container );
#endif

/* window size and position
 */
void       na_gtk_utils_restore_window_position( GtkWindow *toplevel, const gchar *wsp_name );
void       na_gtk_utils_save_window_position   ( GtkWindow *toplevel, const gchar *wsp_name );

/* widget status
 */
void       na_gtk_utils_set_editable( GObject *widget, gboolean editable );

void       na_gtk_utils_radio_set_initial_state  ( GtkRadioButton *button,
				GCallback toggled_handler, void *user_data,
				gboolean editable, gboolean sensitive );

void       na_gtk_utils_radio_reset_initial_state( GtkRadioButton *button, GCallback toggled_handler );

/* default height of a panel bar (dirty hack!)
 */
#define DEFAULT_HEIGHT		22

#define NA_TOGGLE_DATA_EDITABLE			"na-toggle-data-editable"
#define NA_TOGGLE_DATA_BUTTON			"na-toggle-data-button"
#define NA_TOGGLE_DATA_HANDLER			"na-toggle-data-handler"
#define NA_TOGGLE_DATA_USER_DATA		"na-toggle-data-user-data"

G_END_DECLS

#endif /* __CORE_API_NA_GTK_UTILS_H__ */
