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

#ifndef __BASE_APPLICATION_H__
#define __BASE_APPLICATION_H__

/**
 * SECTION: base_application
 * @short_description: #BaseApplication public function declarations.
 * @include: cact/base-application.h
 */

#include <gtk/gtk.h>

#include "base-application-class.h"
#include "base-builder.h"
#include "base-window-class.h"

G_BEGIN_DECLS

enum {
	BASE_APPLICATION_ERROR_I18N = 1,		/* i18n initialization error */
	BASE_APPLICATION_ERROR_GTK,				/* gtk+ initialization error */
	BASE_APPLICATION_ERROR_MAIN_WINDOW,		/* unable to obtain the main window */
	BASE_APPLICATION_ERROR_UNIQUE_APP,		/* another instance is running */
	BASE_APPLICATION_ERROR_UI_FNAME,		/* empty XML filename */
	BASE_APPLICATION_ERROR_UI_LOAD,			/* unable to load the XML definition of the UI */
	BASE_APPLICATION_ERROR_DEFAULT_ICON		/* unable to set default icon */
};

/**
 * @BASE_APPLICATION_PROP_ARGC: count of arguments in command-line.
 * @BASE_APPLICATION_PROP_ARGV: list of command-line arguments
 *
 * These two variables must be provided before running the
 * initialization process ; they are required in order to correctly
 * initialize the Gtk+ user interface.
 */
#define BASE_APPLICATION_PROP_ARGC					"base-application-argc"
#define BASE_APPLICATION_PROP_ARGV					"base-application-argv"

/**
 * @BASE_APPLICATION_PROP_OPTIONS: command-line options.
 *
 * Can be provided at instanciation time only.
 */
#define BASE_APPLICATION_PROP_OPTIONS				"base-application-options"

/**
 * @BASE_APPLICATION_PROP_IS_GTK_INITIALIZED: set to %TRUE after
 * successfully returning from the application_initialize_gtk() virtual
 * function.
 *
 * While this flag is not %TRUE, error messages are printed to
 * stdout. When %TRUE, error messages are displayed with a dialog
 * box.
 */
#define BASE_APPLICATION_PROP_IS_GTK_INITIALIZED	"base-application-is-gtk-initialized"

/**
 * @BASE_APPLICATION_PROP_UNIQUE_APP_HANDLE: the UniqueApp object allocated
 * if the derived-class has provided a UniqueApp name (see
 * #application_get_unique_app_name). Rather for internal use.
 */
#define BASE_APPLICATION_PROP_UNIQUE_APP_HANDLE		"base-application-unique-app-handle"

/**
 * @BASE_APPLICATION_PROP_EXIT_CODE: the code which will be returned by the
 * program to the operating system.
 * @BASE_APPLICATION_PROP_EXIT_MESSAGE1:
 * @BASE_APPLICATION_PROP_EXIT_MESSAGE2: the message which will be displayed
 * at program terminaison if @BASE_APPLICATION_PROP_EXIT_CODE is not zero.
 * When in graphical mode, the first line is displayed as bold.
 *
 * See @BASE_APPLICATION_PROP_IS_GTK_INITIALIZED for how the
 * @BASE_APPLICATION_PROP_EXIT_MESSAGE is actually displayed.
 */
#define BASE_APPLICATION_PROP_EXIT_CODE				"base-application-exit-code"
#define BASE_APPLICATION_PROP_EXIT_MESSAGE1			"base-application-exit-message1"
#define BASE_APPLICATION_PROP_EXIT_MESSAGE2			"base-application-exit-message2"

/**
 * @BASE_APPLICATION_PROP_BUILDER: the #BaseBuilder object allocated to
 * handle the user interface XML definition. Rather for internal use.
 */
#define BASE_APPLICATION_PROP_BUILDER				"base-application-builder"

/**
 * @BASE_APPLICATION_PROP_MAIN_WINDOW: as its name says: a pointer to the
 * #BaseWindow-derived main window of the application.
 */
#define BASE_APPLICATION_PROP_MAIN_WINDOW			"base-application-main-window"

int          base_application_run( BaseApplication *application );
gchar       *base_application_get_application_name( BaseApplication *application );
gchar       *base_application_get_icon_name( BaseApplication *application );
gchar       *base_application_get_unique_app_name( BaseApplication *application );
gchar       *base_application_get_ui_filename( BaseApplication *application );
BaseBuilder *base_application_get_builder( BaseApplication *application );
BaseWindow  *base_application_get_main_window( BaseApplication *application );

void         base_application_message_dlg( BaseApplication *application, GSList *message );
void         base_application_error_dlg( BaseApplication *application, GtkMessageType type, const gchar *first, const gchar *second );
gboolean     base_application_yesno_dlg( BaseApplication *application, GtkMessageType type, const gchar *first, const gchar *second );

G_END_DECLS

#endif /* __BASE_APPLICATION_H__ */
