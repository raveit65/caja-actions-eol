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

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include "base-gtk-utils.h"
#include "cact-main-tab.h"
#include "cact-match-list.h"
#include "cact-add-scheme-dialog.h"
#include "cact-ischemes-tab.h"

/* private interface data
 */
struct _CactISchemesTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* the identifier of this notebook page in the Match dialog
 */
#define ITAB_NAME						"schemes"

static guint st_initializations = 0;	/* interface initialization count */

static GType   register_type( void );
static void    interface_base_init( CactISchemesTabInterface *klass );
static void    interface_base_finalize( CactISchemesTabInterface *klass );

static void    on_base_initialize_gtk( CactISchemesTab *instance, GtkWindow *toplevel, gpointer user_data );
static void    on_base_initialize_window( CactISchemesTab *instance, gpointer user_data );

static void    on_main_selection_changed( CactISchemesTab *instance, GList *selected_items, gpointer user_data );

static void    on_add_from_defaults( GtkButton *button, BaseWindow *window );
static GSList *get_schemes( void *context );
static void    set_schemes( void *context, GSList *filters );

static void    on_instance_finalized( gpointer user_data, CactISchemesTab *instance );

GType
cact_ischemes_tab_get_type( void )
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
	static const gchar *thisfn = "cact_ischemes_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactISchemesTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "CactISchemesTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_TYPE_WINDOW );

	return( type );
}

static void
interface_base_init( CactISchemesTabInterface *klass )
{
	static const gchar *thisfn = "cact_ischemes_tab_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactISchemesTabInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( CactISchemesTabInterface *klass )
{
	static const gchar *thisfn = "cact_ischemes_tab_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/**
 * cact_ischemes_tab_init:
 * @instance: this #CactISchemesTab instance.
 *
 * Initialize the interface
 * Connect to #BaseWindow signals
 */
void
cact_ischemes_tab_init( CactISchemesTab *instance )
{
	static const gchar *thisfn = "cact_ischemes_tab_init";

	g_return_if_fail( CACT_IS_ISCHEMES_TAB( instance ));

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

	cact_main_tab_init( CACT_MAIN_WINDOW( instance ), TAB_SCHEMES );

	g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
}

static void
on_base_initialize_gtk( CactISchemesTab *instance, GtkWindow *toplevel, void *user_data )
{
	static const gchar *thisfn = "cact_ischemes_tab_on_base_initialize_gtk";

	g_return_if_fail( CACT_IS_ISCHEMES_TAB( instance ));

	g_debug( "%s: instance=%p (%s), toplevel=%p, user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) toplevel,
			( void * ) user_data );

	cact_match_list_init_with_args(
			BASE_WINDOW( instance ),
			ITAB_NAME,
			TAB_SCHEMES,
			base_window_get_widget( BASE_WINDOW( instance ), "SchemesTreeView" ),
			base_window_get_widget( BASE_WINDOW( instance ), "AddSchemeButton" ),
			base_window_get_widget( BASE_WINDOW( instance ), "RemoveSchemeButton" ),
			( pget_filters ) get_schemes,
			( pset_filters ) set_schemes,
			NULL,
			NULL,
			MATCH_LIST_MUST_MATCH_ONE_OF,
			_( "Scheme filter" ),
			TRUE );
}

static void
on_base_initialize_window( CactISchemesTab *instance, void *user_data )
{
	static const gchar *thisfn = "cact_ischemes_tab_on_base_initialize_window";
	GtkWidget *button;

	g_return_if_fail( CACT_IS_ISCHEMES_TAB( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));

	button = base_window_get_widget( BASE_WINDOW( instance ), "AddFromDefaultButton" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( button ),
			"clicked",
			G_CALLBACK( on_add_from_defaults ));
}

static void
on_main_selection_changed( CactISchemesTab *instance, GList *selected_items, gpointer user_data )
{
	NAIContext *context;
	gboolean editable;
	gboolean enable_tab;
	GtkWidget *button;

	g_object_get( G_OBJECT( instance ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
			NULL );

	enable_tab = ( context != NULL );
	cact_main_tab_enable_page( CACT_MAIN_WINDOW( instance ), TAB_SCHEMES, enable_tab );

	button = base_window_get_widget( BASE_WINDOW( instance ), "AddFromDefaultButton" );
	base_gtk_utils_set_editable( G_OBJECT( button ), editable );
}

static void
on_add_from_defaults( GtkButton *button, BaseWindow *window )
{
	GSList *schemes;
	gchar *new_scheme;
	NAIContext *context;

	g_object_get( G_OBJECT( window ), MAIN_PROP_CONTEXT, &context, NULL );
	g_return_if_fail( context );

	schemes = cact_match_list_get_rows( window, ITAB_NAME );
	new_scheme = cact_add_scheme_dialog_run( window, schemes );
	na_core_utils_slist_free( schemes );

	if( new_scheme ){
		cact_match_list_insert_row( window, ITAB_NAME, new_scheme, FALSE, FALSE );
		g_free( new_scheme );
	}
}

static GSList *
get_schemes( void *context )
{
	return( na_object_get_schemes( context ));
}

static void
set_schemes( void *context, GSList *filters )
{
	na_object_set_schemes( context, filters );
}

static void
on_instance_finalized( gpointer user_data, CactISchemesTab *instance )
{
	static const gchar *thisfn = "cact_ischemes_tab_on_instance_finalized";

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );
}
