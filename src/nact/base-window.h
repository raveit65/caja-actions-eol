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

#ifndef __BASE_WINDOW_H__
#define __BASE_WINDOW_H__

/**
 * SECTION: base_window
 * @short_description: #BaseWindow public function declaration.
 * @include: nact/base-window.h
 *
 * This is a base class which encapsulates a Gtk+ windows.
 * It works together with the BaseApplication class to run a Gtk+
 * application.
 */

#include <gtk/gtk.h>

#include "base-application-class.h"
#include "base-window-class.h"

G_BEGIN_DECLS

/* instance properties
 */
#define BASE_WINDOW_PROP_PARENT						"base-window-parent"
#define BASE_WINDOW_PROP_APPLICATION				"base-window-application"
#define BASE_WINDOW_PROP_TOPLEVEL_NAME				"base-window-toplevel-name"
#define BASE_WINDOW_PROP_TOPLEVEL_WIDGET			"base-window-toplevel-widget"
#define BASE_WINDOW_PROP_INITIALIZED				"base-window-is-initialized"
#define BASE_WINDOW_PROP_SAVE_WINDOW_POSITION		"base-window-save-window-position"
#define BASE_WINDOW_PROP_HAS_OWN_BUILDER			"base-window-has-own-builder"
#define BASE_WINDOW_PROP_XML_UI_FILENAME			"base-window-xml-ui-filename"

/* signals defined in this class
 *
 * All signals of this class have the same behavior:
 * - the message is sent to all derived classes, which are free to
 *   connect to the signal in order to implement their own code
 * - finally, the default class handler points to a virtual function
 * - the virtual function actually tries to call the actual function
 *   as possibly implemented by the derived class
 * - if no derived class has implemented the virtual function, the call
 *   fall back to doing nothing at all
 */
#define BASE_WINDOW_SIGNAL_INITIAL_LOAD				"nact-base-window-initial-load"
#define BASE_WINDOW_SIGNAL_RUNTIME_INIT				"nact-base-window-runtime-init"
#define BASE_WINDOW_SIGNAL_ALL_WIDGETS_SHOWED		"nact-base-window-all-widgets-showed"

gboolean         base_window_init( BaseWindow *window );
gboolean         base_window_run( BaseWindow *window );

BaseApplication *base_window_get_application( const BaseWindow *window );
GtkWindow       *base_window_get_named_toplevel( const BaseWindow *window, const gchar *name );
BaseWindow      *base_window_get_parent( const BaseWindow *window );
GtkWindow       *base_window_get_toplevel( const BaseWindow *window );
GtkWidget       *base_window_get_widget( const BaseWindow *window, const gchar *name );
gboolean         base_window_is_willing_to_quit( const BaseWindow *window );

void             base_window_error_dlg( const BaseWindow *window, GtkMessageType type, const gchar *primary, const gchar *secondary );
gboolean         base_window_yesno_dlg( const BaseWindow *window, GtkMessageType type, const gchar *first, const gchar *second );

gulong           base_window_signal_connect( BaseWindow *window, GObject *instance, const gchar *signal, GCallback fn );
gulong           base_window_signal_connect_after( BaseWindow *window, GObject *instance, const gchar *signal, GCallback fn );
gulong           base_window_signal_connect_by_name( BaseWindow *window, const gchar *name, const gchar *signal, GCallback fn );

G_END_DECLS

#endif /* __BASE_WINDOW_H__ */
