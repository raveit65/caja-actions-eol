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

#include <glib/gi18n.h>
#include <libintl.h>

#include <api/na-core-utils.h>

#include <core/na-about.h>

#include "cact-application.h"
#include "cact-main-window.h"

/* private class data
 */
struct _CactApplicationClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CactApplicationPrivate {
	gboolean   dispose_has_run;
	NAUpdater *updater;
};

static const gchar *st_application_name	= N_( "Caja-Actions Configuration Tool" );
static const gchar *st_description		= N_( "A user interface to edit your own contextual actions" );
static const gchar *st_unique_name		= "org.mate.caja-actions.ConfigurationTool";

static gboolean     st_non_unique_opt = FALSE;
static gboolean     st_version_opt    = FALSE;

static GOptionEntry st_option_entries[] = {
	{ "non-unique", 'n', 0, G_OPTION_ARG_NONE, &st_non_unique_opt,
			N_( "Set it to run multiple instances of the program [unique]" ), NULL },
	{ "version"   , 'v', 0, G_OPTION_ARG_NONE, &st_version_opt,
			N_( "Output the version number, and exit gracefully [no]" ), NULL },
	{ NULL }
};

static BaseApplicationClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( CactApplicationClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *application );
static void     instance_finalize( GObject *application );

static gboolean appli_manage_options( BaseApplication *application );
static gboolean appli_init_application( BaseApplication *application );
static gboolean appli_create_windows( BaseApplication *application );

GType
cact_application_get_type( void )
{
	static GType application_type = 0;

	if( !application_type ){
		application_type = register_type();
	}

	return( application_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_application_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactApplicationClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactApplication ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_TYPE_APPLICATION, "CactApplication", &info, 0 );

	return( type );
}

static void
class_init( CactApplicationClass *klass )
{
	static const gchar *thisfn = "cact_application_class_init";
	GObjectClass *object_class;
	BaseApplicationClass *appli_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = BASE_APPLICATION_CLASS( g_type_class_peek_parent( klass ));

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactApplicationClassPrivate, 1 );

	appli_class = BASE_APPLICATION_CLASS( klass );
	appli_class->manage_options = appli_manage_options;
	appli_class->init_application = appli_init_application;
	appli_class->create_windows = appli_create_windows;
}

static void
instance_init( GTypeInstance *application, gpointer klass )
{
	static const gchar *thisfn = "cact_application_instance_init";
	CactApplication *self;

	g_return_if_fail( CACT_IS_APPLICATION( application ));

	g_debug( "%s: application=%p (%s), klass=%p",
			thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ), ( void * ) klass );

	self = CACT_APPLICATION( application );

	self->private = g_new0( CactApplicationPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *application )
{
	static const gchar *thisfn = "cact_application_instance_dispose";
	CactApplication *self;

	g_return_if_fail( CACT_IS_APPLICATION( application ));

	self = CACT_APPLICATION( application );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: application=%p (%s)", thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ));

		self->private->dispose_has_run = TRUE;

		if( self->private->updater ){
			g_object_unref( self->private->updater );
		}

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( application );
		}
	}
}

static void
instance_finalize( GObject *application )
{
	static const gchar *thisfn = "cact_application_instance_finalize";
	CactApplication *self;

	g_return_if_fail( CACT_IS_APPLICATION( application ));

	g_debug( "%s: application=%p (%s)", thisfn, ( void * ) application, G_OBJECT_TYPE_NAME( application ));

	self = CACT_APPLICATION( application );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( application );
	}
}

/**
 * cact_application_new:
 *
 * Returns: a newly allocated CactApplication object.
 */
CactApplication *
cact_application_new( void )
{
	CactApplication *application;

	application = g_object_new( CACT_TYPE_APPLICATION, NULL );

	g_object_set( G_OBJECT( application ),
			BASE_PROP_OPTIONS,          st_option_entries,
			BASE_PROP_APPLICATION_NAME, gettext( st_application_name ),
			BASE_PROP_DESCRIPTION,      gettext( st_description ),
			BASE_PROP_ICON_NAME,        na_about_get_icon_name(),
			BASE_PROP_UNIQUE_NAME,      st_unique_name,
			NULL );

	return( application );
}

/*
 * overridden to manage command-line options
 */
static gboolean
appli_manage_options( BaseApplication *application )
{
	static const gchar *thisfn = "cact_application_appli_manage_options";
	gboolean ret;

	g_return_val_if_fail( CACT_IS_APPLICATION( application ), FALSE );

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = TRUE;

	/* display the program version ?
	 * if yes, then stops here
	 */
	if( st_version_opt ){
		na_core_utils_print_version();
		ret = FALSE;
	}

	/* run the application as non-unique ?
	 */
	if( ret && st_non_unique_opt ){
		g_object_set( G_OBJECT( application ), BASE_PROP_UNIQUE_NAME, "", NULL );
	}

	/* call parent class */
	if( ret && BASE_APPLICATION_CLASS( st_parent_class )->manage_options ){
		ret = BASE_APPLICATION_CLASS( st_parent_class )->manage_options( application );
	}

	return( ret );
}

/*
 * initialize the application
 */
static gboolean
appli_init_application( BaseApplication *application )
{
	static const gchar *thisfn = "cact_application_appli_init_application";
	gboolean ret;
	CactApplicationPrivate *priv;

	g_return_val_if_fail( CACT_IS_APPLICATION( application ), FALSE );

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = TRUE;
	priv = CACT_APPLICATION( application )->private;

	/* create the NAPivot object (loading the plugins and so on)
	 * after having dealt with command-line arguments
	 */
	priv->updater = na_updater_new();
	na_pivot_set_loadable( NA_PIVOT( priv->updater ), PIVOT_LOAD_ALL );

	/* call parent class */
	if( ret && BASE_APPLICATION_CLASS( st_parent_class )->init_application ){
		ret = BASE_APPLICATION_CLASS( st_parent_class )->init_application( application );
	}

	return( ret );
}

/*
 * create application startup windows
 */
static gboolean
appli_create_windows( BaseApplication *application )
{
	static const gchar *thisfn = "cact_application_appli_create_windows";
	gboolean ret;
	CactMainWindow *window;

	g_return_val_if_fail( CACT_IS_APPLICATION( application ), FALSE );

	g_debug( "%s: application=%p", thisfn, ( void * ) application );

	ret = FALSE;

	/* creating main window
	 */
	window = cact_main_window_new( CACT_APPLICATION( application ));

	if( window ){
		g_return_val_if_fail( CACT_IS_MAIN_WINDOW( window ), FALSE );
		ret = TRUE;

	} else {
		g_object_set( G_OBJECT( application ), BASE_PROP_CODE, BASE_EXIT_CODE_INIT_WINDOW, NULL );
	}

	return( ret );
}

/**
 * cact_application_get_updater:
 * @application: this CactApplication object.
 *
 * Returns a pointer on the #NAUpdater object.
 *
 * The returned pointer is owned by the #CactApplication object.
 * It should not be g_free() not g_object_unref() by the caller.
 */
NAUpdater *
cact_application_get_updater( const CactApplication *application )
{
	NAUpdater *updater = NULL;

	g_return_val_if_fail( CACT_IS_APPLICATION( application ), NULL );

	if( !application->private->dispose_has_run ){

		updater = application->private->updater;
	}

	return( updater );
}
