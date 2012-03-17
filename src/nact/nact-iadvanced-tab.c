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

#include <api/na-object-api.h>

#include "base-window.h"
#include "nact-main-tab.h"
#include "nact-schemes-list.h"
#include "nact-iadvanced-tab.h"

/* private interface data
 */
struct NactIAdvancedTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;

static GType         register_type( void );
static void          interface_base_init( NactIAdvancedTabInterface *klass );
static void          interface_base_finalize( NactIAdvancedTabInterface *klass );

static void          runtime_init_connect_signals( NactIAdvancedTab *instance, GtkTreeView *listview );
static void          on_tab_updatable_selection_changed( NactIAdvancedTab *instance, gint count_selected );
static void          on_tab_updatable_enable_tab( NactIAdvancedTab *instance, NAObjectItem *item );
static gboolean      tab_set_sensitive( NactIAdvancedTab *instance );
static GtkTreeView  *get_schemes_tree_view( NactIAdvancedTab *instance );

GType
nact_iadvanced_tab_get_type( void )
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
	static const gchar *thisfn = "nact_iadvanced_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NactIAdvancedTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NactIAdvancedTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_WINDOW_TYPE );

	return( type );
}

static void
interface_base_init( NactIAdvancedTabInterface *klass )
{
	static const gchar *thisfn = "nact_iadvanced_tab_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NactIAdvancedTabInterfacePrivate, 1 );

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NactIAdvancedTabInterface *klass )
{
	static const gchar *thisfn = "nact_iadvanced_tab_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
}

void
nact_iadvanced_tab_initial_load_toplevel( NactIAdvancedTab *instance )
{
	static const gchar *thisfn = "nact_iadvanced_tab_initial_load_toplevel";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
		g_return_if_fail( NACT_IS_IADVANCED_TAB( instance ));

		nact_schemes_list_create_model( get_schemes_tree_view( instance ), TRUE );
	}
}

void
nact_iadvanced_tab_runtime_init_toplevel( NactIAdvancedTab *instance )
{
	static const gchar *thisfn = "nact_iadvanced_tab_runtime_init_toplevel";
	GtkTreeView *listview;

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
		g_return_if_fail( NACT_IS_IADVANCED_TAB( instance ));

		listview = get_schemes_tree_view( instance );
		runtime_init_connect_signals( instance, listview );
		nact_schemes_list_init_view( listview, BASE_WINDOW( instance ));
	}
}

static void
runtime_init_connect_signals( NactIAdvancedTab *instance, GtkTreeView *listview )
{
	static const gchar *thisfn = "nact_iadvanced_tab_runtime_init_connect_signals";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p, listview=%p", thisfn, ( void * ) instance, ( void * ) listview );
		g_return_if_fail( NACT_IS_IADVANCED_TAB( instance ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				MAIN_WINDOW_SIGNAL_SELECTION_CHANGED,
				G_CALLBACK( on_tab_updatable_selection_changed ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				TAB_UPDATABLE_SIGNAL_ENABLE_TAB,
				G_CALLBACK( on_tab_updatable_enable_tab ));
	}
}

void
nact_iadvanced_tab_all_widgets_showed( NactIAdvancedTab *instance )
{
	static const gchar *thisfn = "nact_iadvanced_tab_all_widgets_showed";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
		g_return_if_fail( NACT_IS_IADVANCED_TAB( instance ));
	}
}

void
nact_iadvanced_tab_dispose( NactIAdvancedTab *instance )
{
	static const gchar *thisfn = "nact_iadvanced_tab_dispose";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
		g_return_if_fail( NACT_IS_IADVANCED_TAB( instance ));

		nact_schemes_list_dispose( BASE_WINDOW( instance ));
	}
}

/**
 * Returns selected schemes as a list of strings.
 * The caller should call na_core_utils_slist_free() after use.
 */
GSList *
nact_iadvanced_tab_get_schemes( NactIAdvancedTab *instance )
{
	GSList *list;

	list = NULL;
	g_return_val_if_fail( NACT_IS_IADVANCED_TAB( instance ), list );

	if( st_initialized && !st_finalized ){

		list = nact_schemes_list_get_schemes( get_schemes_tree_view( instance ));
	}

	return( list );
}

static void
on_tab_updatable_selection_changed( NactIAdvancedTab *instance, gint count_selected )
{
	static const gchar *thisfn = "nact_iadvanced_tab_on_tab_updatable_selection_changed";
	NAObjectItem *item;
	NAObjectProfile *profile;
	gboolean enable_tab;
	GSList *schemes;
	gboolean editable;

	schemes = NULL;
	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p, count_selected=%d", thisfn, ( void * ) instance, count_selected );
		g_return_if_fail( NACT_IS_IADVANCED_TAB( instance ));

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &item,
				TAB_UPDATABLE_PROP_EDITED_PROFILE, &profile,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		enable_tab = tab_set_sensitive( instance );

		if( profile ){
			schemes = na_object_get_schemes( profile );
		}

		nact_schemes_list_setup_values(
				get_schemes_tree_view( instance ),
				BASE_WINDOW( instance ),
				schemes,
				item && NA_IS_OBJECT_ACTION( item ),
				editable );
	}
}

static void
on_tab_updatable_enable_tab( NactIAdvancedTab *instance, NAObjectItem *item )
{
	static const gchar *thisfn = "nact_iadvanced_tab_on_tab_updatable_enable_tab";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p, item=%p", thisfn, ( void * ) instance, ( void * ) item );
		g_return_if_fail( NACT_IS_IADVANCED_TAB( instance ));

		tab_set_sensitive( instance );
	}
}

static gboolean
tab_set_sensitive( NactIAdvancedTab *instance )
{
	NAObjectItem *item;
	NAObjectProfile *profile;
	gboolean enable_tab;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &item,
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &profile,
			NULL );

	enable_tab = ( profile != NULL && na_object_is_target_selection( NA_OBJECT_ACTION( item )));
	nact_main_tab_enable_page( NACT_MAIN_WINDOW( instance ), TAB_ADVANCED, enable_tab );

	return( enable_tab );
}

static GtkTreeView *
get_schemes_tree_view( NactIAdvancedTab *instance )
{
	GtkWidget *treeview;

	treeview = base_window_get_widget( BASE_WINDOW( instance ), "SchemesTreeView" );
	g_assert( GTK_IS_TREE_VIEW( treeview ));

	return( GTK_TREE_VIEW( treeview ));
}
