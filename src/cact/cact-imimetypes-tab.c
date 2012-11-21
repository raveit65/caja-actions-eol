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

#include <api/na-object-api.h>

#include "cact-main-tab.h"
#include "cact-match-list.h"
#include "cact-imimetypes-tab.h"

/* private interface data
 */
struct _CactIMimetypesTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* the identifier of this notebook page in the Match dialog
 */
#define ITAB_NAME						"mimetypes"

static guint st_initializations = 0;	/* interface initialization count */

static GType   register_type( void );
static void    interface_base_init( CactIMimetypesTabInterface *klass );
static void    interface_base_finalize( CactIMimetypesTabInterface *klass );

static void    on_base_initialize_gtk( CactIMimetypesTab *instance, GtkWindow *toplevel, gpointer user_data );
static void    on_base_initialize_window( CactIMimetypesTab *instance, gpointer user_data );

static void    on_main_selection_changed( CactIMimetypesTab *instance, GList *selected_items, gpointer user_data );

static GSList *get_mimetypes( void *context );
static void    set_mimetypes( void *context, GSList *filters );

static void    on_instance_finalized( gpointer user_data, CactIMimetypesTab *instance );

GType
cact_imimetypes_tab_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = register_type();
	}

	return( iface_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_imimetypes_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactIMimetypesTabInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_INTERFACE, "CactIMimetypesTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_TYPE_WINDOW );

	return( type );
}

static void
interface_base_init( CactIMimetypesTabInterface *klass )
{
	static const gchar *thisfn = "cact_imimetypes_tab_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactIMimetypesTabInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( CactIMimetypesTabInterface *klass )
{
	static const gchar *thisfn = "cact_imimetypes_tab_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/*
 * cact_imimetypes_tab_init:
 * @instance: this #CactIMimetypesTab instance.
 *
 * Initialize the interface
 * Connect to #BaseWindow signals
 */
void
cact_imimetypes_tab_init( CactIMimetypesTab *instance )
{
	static const gchar *thisfn = "cact_imimetypes_tab_init";

	g_return_if_fail( CACT_IS_IMIMETYPES_TAB( instance ));

	g_debug( "%s: instance=%p (%s)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_GTK,
			G_CALLBACK( on_base_initialize_gtk ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_base_initialize_window ));

	cact_main_tab_init( CACT_MAIN_WINDOW( instance ), TAB_MIMETYPES );

	g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
}

/*
 * on_base_initialize_gtk:
 * @window: this #CactIMimetypesTab instance.
 *
 * Initializes the tab widget at initial load.
 */
static void
on_base_initialize_gtk( CactIMimetypesTab *instance, GtkWindow *toplevel, void *user_data )
{
	static const gchar *thisfn = "cact_imimetypes_tab_on_base_initialize_gtk";

	g_return_if_fail( CACT_IS_IMIMETYPES_TAB( instance ));

	g_debug( "%s: instance=%p (%s), toplevel=%p, user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) toplevel,
			( void * ) user_data );

	cact_match_list_init_with_args(
			BASE_WINDOW( instance ),
			ITAB_NAME,
			TAB_MIMETYPES,
			base_window_get_widget( BASE_WINDOW( instance ), "MimetypesTreeView" ),
			base_window_get_widget( BASE_WINDOW( instance ), "AddMimetypeButton" ),
			base_window_get_widget( BASE_WINDOW( instance ), "RemoveMimetypeButton" ),
			( pget_filters ) get_mimetypes,
			( pset_filters ) set_mimetypes,
			NULL,
			NULL,
			MATCH_LIST_MUST_MATCH_ONE_OF,
			_( "Mimetype filter" ),
			TRUE );
}

/*
 * on_base_initialize_window:
 * @window: this #CactIMimetypesTab instance.
 *
 * Initializes the tab widget at each time the widget will be displayed.
 * Connect signals and setup runtime values.
 */
static void
on_base_initialize_window( CactIMimetypesTab *instance, void *user_data )
{
	static const gchar *thisfn = "cact_imimetypes_tab_on_base_initialize_window";

	g_return_if_fail( CACT_IS_IMIMETYPES_TAB( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));
}

static void
on_main_selection_changed( CactIMimetypesTab *instance, GList *selected_items, gpointer user_data )
{
	NAIContext *context;
	gboolean editable;
	gboolean enable_tab;

	g_object_get( G_OBJECT( instance ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
			NULL );

	enable_tab = ( context != NULL );
	cact_main_tab_enable_page( CACT_MAIN_WINDOW( instance ), TAB_MIMETYPES, enable_tab );
}

static GSList *
get_mimetypes( void *context )
{
	return( na_object_get_mimetypes( context ));
}

static void
set_mimetypes( void *context, GSList *filters )
{
	na_object_set_mimetypes( context, filters );
}

static void
on_instance_finalized( gpointer user_data, CactIMimetypesTab *instance )
{
	static const gchar *thisfn = "cact_imimetypes_tab_on_instance_finalized";

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );
}
