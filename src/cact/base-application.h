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

#ifndef __BASE_APPLICATION_H__
#define __BASE_APPLICATION_H__

/**
 * SECTION: base-application
 * @title: BaseApplication
 * @short_description: The Base Application application base class definition
 * @include: base-application.h
 *
 * #BaseApplication is the base class for the application part of Gtk programs.
 * It aims at providing all common services. It interacts with #BaseBuilder
 * and #BaseWindow classes.
 *
 * #BaseApplication is a pure virtual class. A Gtk program should derive
 * its own class from #BaseApplication, and instantiate it in its main()
 * program entry point.
 *
 * <example>
 *   <programlisting>
 *     #include "my-application.h"
 *
 *     int
 *     main( int argc, char **argv )
 *     {
 *         MyApplication *appli;
 *         int code;
 *
 *         appli = my_application_new();
 *         code = base_appliction_run_with_args( BASE_APPLICATION( appli ), argc, argv );
 *         g_object_unref( appli );
 *
 *         return( code );
 *     }
 *   </programlisting>
 * </example>
 *
 * main                 BaseApplication      CactApplication      BaseWindow           CactWindow
 * ===================  ===================  ===================  ===================  ===================
 * appli = cact_application_new()
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                                           appli = g_object_new()
 *                                           set properties
 *                                             application name
 *                                             icon name
 *                                             description
 *                                             command-line definitions
 *                                             unique name (if apply)
 * --------------------+--------------------+--------------------+--------------------+-------------------
 * ret = base_application_run_with_args( appli, argc, argv )
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                      init i18n
 *                      init application name
 *                      init gtk with command-line options
 *                      manage command-line options
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                                           manage specific command-line options
 *                                           calling parent class if ok to continue
 *                                           setting application code else
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                      init unique manager
 *                        unique app name must have been set at this time
 *                        application name should have been set at this time
 *                      init session manager
 *                      init icon name
 *                      create window(s)
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                                           foreach window to create
 *                                             create BaseWindow-derived window
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                                                                on class init
 *                                                                  init common builder
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                                                                                     init window
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                                                                load and init gtk toplevel
 *                                                                init window
 *                                                                show widgets
 * --------------------+--------------------+--------------------+--------------------+-------------------
 *                      enter the main loop
 *                        leaving to the main window the
 *                        responsability to gtk_main_quit()
 *                        after having set the application
 *                        exit code.
 * --------------------+--------------------+--------------------+--------------------+-------------------
 * g_object_unref( appli )
 * return( ret )
 * ===================  ===================  ===================  ===================  ===================
 * main                 BaseApplication      CactApplication      BaseWindow           CactWindow
 *
 * At any time, a function may preset the exit code of the application just by
 * setting the @BASE_PROP_CODE property. Note that unless it also asks to quit
 * immediately by returning %FALSE, another function may always set another exit
 * code after that.
 */

#include "glib-object.h"

G_BEGIN_DECLS

#define BASE_TYPE_APPLICATION                ( base_application_get_type())
#define BASE_APPLICATION( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, BASE_TYPE_APPLICATION, BaseApplication ))
#define BASE_APPLICATION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, BASE_TYPE_APPLICATION, BaseApplicationClass ))
#define BASE_IS_APPLICATION( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, BASE_TYPE_APPLICATION ))
#define BASE_IS_APPLICATION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), BASE_TYPE_APPLICATION ))
#define BASE_APPLICATION_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), BASE_TYPE_APPLICATION, BaseApplicationClass ))

typedef struct _BaseApplicationPrivate       BaseApplicationPrivate;

typedef struct {
	/*< private >*/
	GObject                 parent;
	BaseApplicationPrivate *private;
}
	BaseApplication;

typedef struct _BaseApplicationClassPrivate  BaseApplicationClassPrivate;

/**
 * BaseApplicationClass:
 * @manage_options:  manage the command-line arguments.
 * @main_window_new: open and run the first (main) window of the application.
 *
 * This defines the virtual method a derived class may, should or must implement.
 */
typedef struct {
	/*< private >*/
	GObjectClass                 parent;
	BaseApplicationClassPrivate *private;

	/*< public >*/
	/**
	 * manage_options:
	 * @appli: this #BaseApplication -derived instance.
	 *
	 * This is invoked by the BaseApplication base class, after arguments
	 * in the command-line have been processed by gtk_init_with_args()
	 * function.
	 *
	 * This let the derived class an opportunity to manage command-line
	 * arguments. Unless it decides to stop the execution of the program,
	 * the derived class should call the parent class method (would it be
	 * defined) in order to let it manage its own options.
	 *
	 * The derived class may set the exit code of the application by
	 * setting the @BASE_PROP_CODE property of @appli.
	 *
	 * Returns: %TRUE to continue execution, %FALSE to stop it.
	 */
	gboolean  ( *manage_options ) ( BaseApplication *appli );

	/**
	 * init_application:
	 * @appli: this #BaseApplication -derived instance.
	 *
	 * This is invoked by the BaseApplication base class to let the derived
	 * class do its own initializations.
	 *
	 * Versus initializations which occur at instanciation time, this method
	 * let the application terminate its initializatin process, after command-line
	 * arguments have been managed, and before creating any window.
	 *
	 * Unless it decides to stop the execution of the program,
	 * the derived class should call the parent class method (would it be
	 * defined) in order to let it manage its own options.
	 *
	 * The derived class may set the exit code of the application by
	 * setting the @BASE_PROP_CODE property of @appli.
	 *
	 * Returns: %TRUE to continue execution, %FALSE to stop it.
	 */
	gboolean ( *init_application )( BaseApplication *appli );

	/**
	 * create_windows:
	 * @appli: this #BaseApplication -derived instance.
	 *
	 * This is invoked by the BaseApplication base class to let the derived
	 * class create its startup windows. This may include a splash window,
	 * a main window, some secondary or toolbox windows, and so on.
	 *
	 * Each created window should initialize and show itself at this time
	 * by calling base_window_init(). base_window_init() will return
	 * %FALSE if the Gtk toplevel cannot have been initialized.
	 *
	 * This is a pure virtual method. Only the most derived class
	 * create_windows() method is invoked.
	 *
	 * The derived class may set the exit code of the application by
	 * setting the @BASE_PROP_CODE property of @appli.
	 *
	 * Returns: %TRUE to continue execution, %FALSE to stop it.
	 *
	 * Only if this method returns %TRUE, the #BaseApplication class will
	 * enter in main loop, and stay in it until gtk_main_quit() is called.
	 *
	 * It is usually the responsability of main application window of calling
	 * gtk_main_quit() when it is closed, either as a menu action or if the
	 * user destroys it.
	 */
	gboolean ( *create_windows )( BaseApplication *appli );
}
	BaseApplicationClass;

/**
 * Properties defined by the BaseApplication class.
 * They may be provided at object instantiation time, either in the derived-
 * application constructor, or in the main() function, but in all cases
 * before calling base_application_run_with_args().
 *
 * @BASE_PROP_ARGC:             count of arguments in command-line.
 * @BASE_PROP_ARGV:             array of command-line arguments.
 * @BASE_PROP_OPTIONS:          array of command-line options descriptions.
 * @BASE_PROP_APPLICATION_NAME: application name.
 * @BASE_PROP_DESCRIPTION:      short description.
 * @BASE_PROP_ICON_NAME:        icon name.
 * @BASE_PROP_UNIQUE_NAME:      unique name of the application (if not empty)
 * @BASE_PROP_CODE:             return code of the application
 */
#define BASE_PROP_ARGC						"base-prop-application-argc"
#define BASE_PROP_ARGV						"base-prop-application-argv"
#define BASE_PROP_OPTIONS					"base-prop-application-options"
#define BASE_PROP_APPLICATION_NAME			"base-prop-application-name"
#define BASE_PROP_DESCRIPTION				"base-prop-application-description"
#define BASE_PROP_ICON_NAME					"base-prop-application-icon-name"
#define BASE_PROP_UNIQUE_NAME				"base-prop-application-unique-name"
#define BASE_PROP_CODE						"base-prop-application-code"

/**
 * BaseExitCode:
 *
 * The code returned by the application.
 *
 * The BaseApplication -derived class may define its own codes, starting
 * them with @BASE_EXIT_CODE_USER_APP.
 *
 * @BASE_EXIT_CODE_PROGRAM = -1:         this is a program error code.
 * @BASE_EXIT_CODE_OK = 0:               the program has successfully run, and returns zero.
 * @BASE_EXIT_CODE_APPLICATION_NAME = 1: no application name has been set by the derived class
 * @BASE_EXIT_CODE_ARGS = 2:             unable to interpret command-line options
 * @BASE_EXIT_CODE_UNIQUE_APP = 3:       another instance is already running
 * @BASE_EXIT_CODE_INIT_WINDOW = 4:      unable to create a startup window
 *
 * @BASE_EXIT_CODE_USER_APP = 32:        BaseApplication -derived class may use program return codes
 *                                       starting with this value
 */
typedef enum {
	BASE_EXIT_CODE_PROGRAM = -1,
	BASE_EXIT_CODE_OK = 0,
	BASE_EXIT_CODE_APPLICATION_NAME,
	BASE_EXIT_CODE_ARGS,
	BASE_EXIT_CODE_UNIQUE_APP,
	BASE_EXIT_CODE_INIT_WINDOW,

	BASE_EXIT_CODE_USER_APP = 32
}
	BaseExitCode;

GType    base_application_get_type            ( void );

int      base_application_run_with_args       ( BaseApplication *application, int argc, GStrv argv );

gchar   *base_application_get_application_name( const BaseApplication *application );

gboolean base_application_is_willing_to_quit  ( const BaseApplication *application );

G_END_DECLS

#endif /* __BASE_APPLICATION_H__ */
