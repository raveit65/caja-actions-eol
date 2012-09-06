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

#include <string.h>

#include <glib/gi18n.h>

#include <libcaja-extension/caja-extension-types.h>
#include <libcaja-extension/caja-file-info.h>
#include <libcaja-extension/caja-menu-provider.h>

#include <api/na-object-api.h>

#include <core/na-pivot.h>
#include <core/na-iabout.h>
#include <core/na-iprefs.h>
#include <core/na-ipivot-consumer.h>
#include <core/na-selected-info.h>

#include "caja-actions.h"

/* private class data
 */
struct CajaActionsClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct CajaActionsPrivate {
	gboolean dispose_has_run;
	NAPivot *pivot;
};

static GObjectClass *st_parent_class = NULL;
static GType         st_actions_type = 0;

static void              class_init( CajaActionsClass *klass );
static void              instance_init( GTypeInstance *instance, gpointer klass );
static void              instance_constructed( GObject *object );
static void              instance_dispose( GObject *object );
static void              instance_finalize( GObject *object );

static void              iabout_iface_init( NAIAboutInterface *iface );
static gchar            *iabout_get_application_name( NAIAbout *instance );

static void              ipivot_consumer_iface_init( NAIPivotConsumerInterface *iface );
static void              ipivot_consumer_items_changed( NAIPivotConsumer *instance, gpointer user_data );
static void              ipivot_consumer_create_root_menu_changed( NAIPivotConsumer *instance, gboolean enabled );
static void              ipivot_consumer_display_about_changed( NAIPivotConsumer *instance, gboolean enabled );
static void              ipivot_consumer_display_order_changed( NAIPivotConsumer *instance, gint order_mode );

static void              menu_provider_iface_init( CajaMenuProviderIface *iface );
static GList            *menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder );
static GList            *menu_provider_get_file_items( CajaMenuProvider *provider, GtkWidget *window, GList *files );
static GList            *menu_provider_get_toolbar_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder );

static GList            *get_file_or_background_items( CajaActions *plugin, guint target, void *selection );
static GList            *build_caja_menus( CajaActions *plugin, GList *tree, guint target, GList *files );
static NAObjectProfile  *get_candidate_profile( CajaActions *plugin, NAObjectAction *action, guint target, GList *files );
static CajaMenuItem *create_item_from_profile( NAObjectProfile *profile, guint target, GList *files );
static CajaMenuItem *create_item_from_menu( NAObjectMenu *menu, GList *subitems );
static CajaMenuItem *create_menu_item( NAObjectItem *item );
static void              attach_submenu_to_item( CajaMenuItem *item, GList *subitems );
static void              weak_notify_profile( NAObjectProfile *profile, CajaMenuItem *item );
static void              destroy_notify_file_list( GList *list);
static void              weak_notify_menu( NAObjectMenu *menu, CajaMenuItem *item );

static void              execute_action( CajaMenuItem *item, NAObjectProfile *profile );

static GList            *create_root_menu( CajaActions *plugin, GList *caja_menu );
static GList            *add_about_item( CajaActions *plugin, GList *caja_menu );
static void              execute_about( CajaMenuItem *item, CajaActions *plugin );

GType
caja_actions_get_type( void )
{
	g_assert( st_actions_type );
	return( st_actions_type );
}

void
caja_actions_register_type( GTypeModule *module )
{
	static const gchar *thisfn = "caja_actions_register_type";

	static const GTypeInfo info = {
		sizeof( CajaActionsClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CajaActions ),
		0,
		( GInstanceInitFunc ) instance_init,
	};

	static const GInterfaceInfo menu_provider_iface_info = {
		( GInterfaceInitFunc ) menu_provider_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo iabout_iface_info = {
		( GInterfaceInitFunc ) iabout_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ipivot_consumer_iface_info = {
		( GInterfaceInitFunc ) ipivot_consumer_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s: module=%p", thisfn, ( void * ) module );
	g_assert( st_actions_type == 0 );

	st_actions_type = g_type_module_register_type( module, G_TYPE_OBJECT, "CajaActions", &info, 0 );

	g_type_module_add_interface( module, st_actions_type, CAJA_TYPE_MENU_PROVIDER, &menu_provider_iface_info );

	g_type_module_add_interface( module, st_actions_type, NA_IABOUT_TYPE, &iabout_iface_info );

	g_type_module_add_interface( module, st_actions_type, NA_IPIVOT_CONSUMER_TYPE, &ipivot_consumer_iface_info );
}

static void
class_init( CajaActionsClass *klass )
{
	static const gchar *thisfn = "caja_actions_class_init";
	GObjectClass *gobject_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	gobject_class = G_OBJECT_CLASS( klass );
	gobject_class->constructed = instance_constructed;
	gobject_class->dispose = instance_dispose;
	gobject_class->finalize = instance_finalize;

	klass->private = g_new0( CajaActionsClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "caja_actions_instance_init";
	CajaActions *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_return_if_fail( CAJA_IS_ACTIONS( instance ));
	g_return_if_fail( NA_IS_IPIVOT_CONSUMER( instance ));

	self = CAJA_ACTIONS( instance );

	self->private = g_new0( CajaActionsPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_constructed( GObject *object )
{
	static const gchar *thisfn = "caja_actions_instance_constructed";
	CajaActions *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );

	g_return_if_fail( CAJA_IS_ACTIONS( object ));
	g_return_if_fail( NA_IS_IPIVOT_CONSUMER( object ));

	self = CAJA_ACTIONS( object );

	if( !self->private->dispose_has_run ){

		self->private->pivot = na_pivot_new();
		na_pivot_register_consumer( self->private->pivot, NA_IPIVOT_CONSUMER( self ));
		na_pivot_set_automatic_reload( self->private->pivot, TRUE );
		na_pivot_set_loadable( self->private->pivot, !PIVOT_LOAD_DISABLED & !PIVOT_LOAD_INVALID );
		na_pivot_load_items( self->private->pivot );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( object );
		}
	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "caja_actions_instance_dispose";
	CajaActions *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( CAJA_IS_ACTIONS( object ));
	self = CAJA_ACTIONS( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		g_object_unref( self->private->pivot );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "caja_actions_instance_finalize";
	CajaActions *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( CAJA_IS_ACTIONS( object ));
	self = CAJA_ACTIONS( object );

	g_free( self->private );

	/* chain up to the parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/*
 * This function notifies Caja file manager that the context menu
 * items may have changed, and that it should reload them.
 *
 * Patch has been provided by Frederic Ruaudel, the initial author of
 * Caja-Actions, and applied on Caja 2.15.4 development branch
 * on 2006-06-16. It was released with Caja 2.16.0
 */
#ifndef HAVE_CAJA_MENU_PROVIDER_EMIT_ITEMS_UPDATED_SIGNAL
static void caja_menu_provider_emit_items_updated_signal (CajaMenuProvider *provider)
{
	/* -> fake function for backward compatibility
	 * -> do nothing
	 */
}
#endif

static void
iabout_iface_init( NAIAboutInterface *iface )
{
	static const gchar *thisfn = "caja_actions_iabout_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_application_name = iabout_get_application_name;
}

static gchar *
iabout_get_application_name( NAIAbout *instance )
{
	/* i18n: title of the About dialog box, when seen from Caja file manager */
	return( g_strdup( _( "Caja Actions" )));
}

static void
ipivot_consumer_iface_init( NAIPivotConsumerInterface *iface )
{
	static const gchar *thisfn = "caja_actions_ipivot_consumer_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->on_items_changed = ipivot_consumer_items_changed;
	iface->on_create_root_menu_changed = ipivot_consumer_create_root_menu_changed;
	iface->on_display_about_changed = ipivot_consumer_display_about_changed;
	iface->on_display_order_changed = ipivot_consumer_display_order_changed;
	iface->on_mandatory_prefs_changed = NULL;
}

static void
ipivot_consumer_items_changed( NAIPivotConsumer *instance, gpointer user_data )
{
	static const gchar *thisfn = "caja_actions_ipivot_consumer_items_changed";
	CajaActions *self;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );
	g_return_if_fail( CAJA_IS_ACTIONS( instance ));
	self = CAJA_ACTIONS( instance );

	if( !self->private->dispose_has_run ){

		caja_menu_provider_emit_items_updated_signal( CAJA_MENU_PROVIDER( self ));
	}
}

static void
ipivot_consumer_create_root_menu_changed( NAIPivotConsumer *instance, gboolean enabled )
{
	static const gchar *thisfn = "caja_actions_ipivot_consumer_create_root_menu_changed";
	CajaActions *self;

	g_debug( "%s: instance=%p, enabled=%s", thisfn, ( void * ) instance, enabled ? "True":"False" );
	g_return_if_fail( CAJA_IS_ACTIONS( instance ));
	self = CAJA_ACTIONS( instance );

	if( !self->private->dispose_has_run ){

		caja_menu_provider_emit_items_updated_signal( CAJA_MENU_PROVIDER( self ));
	}
}

static void
ipivot_consumer_display_about_changed( NAIPivotConsumer *instance, gboolean enabled )
{
	static const gchar *thisfn = "caja_actions_ipivot_consumer_display_about_changed";
	CajaActions *self;

	g_debug( "%s: instance=%p, enabled=%s", thisfn, ( void * ) instance, enabled ? "True":"False" );
	g_return_if_fail( CAJA_IS_ACTIONS( instance ));
	self = CAJA_ACTIONS( instance );

	if( !self->private->dispose_has_run ){

		caja_menu_provider_emit_items_updated_signal( CAJA_MENU_PROVIDER( self ));
	}
}

static void
ipivot_consumer_display_order_changed( NAIPivotConsumer *instance, gint order_mode )
{
	static const gchar *thisfn = "caja_actions_ipivot_consumer_display_order_changed";
	CajaActions *self;

	g_debug( "%s: instance=%p, order_mode=%d", thisfn, ( void * ) instance, order_mode );
	g_return_if_fail( CAJA_IS_ACTIONS( instance ));
	self = CAJA_ACTIONS( instance );

	if( !self->private->dispose_has_run ){

		caja_menu_provider_emit_items_updated_signal( CAJA_MENU_PROVIDER( self ));
	}
}

static void
menu_provider_iface_init( CajaMenuProviderIface *iface )
{
	static const gchar *thisfn = "caja_actions_menu_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_file_items = menu_provider_get_file_items;
	iface->get_background_items = menu_provider_get_background_items;
	iface->get_toolbar_items = menu_provider_get_toolbar_items;
}

/*
 * this function is called when caja has to paint a folder background
 * one of the first calls is with current_folder = 'x-caja-desktop:///'
 * the menu items are available :
 * a) in File menu
 * b) in contextual menu of the folder if there is no current selection
 *
 * get_background_items is very similar, from the user point of view, to
 * get_file_items when:
 * - either there is zero item selected - current_folder should so be the
 *   folder currently displayed in the file manager view
 * - or when there is only one selected directory
 */
static GList *
menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder )
{
	static const gchar *thisfn = "caja_actions_menu_provider_get_background_items";
	GList *caja_menus_list = NULL;
	gchar *uri;

	if( !CAJA_ACTIONS( provider )->private->dispose_has_run ){

		uri = caja_file_info_get_uri( current_folder );
		g_debug( "%s: provider=%p, window=%p, current_folder=%p (%s)",
				thisfn, ( void * ) provider, ( void * ) window, ( void * ) current_folder, uri );
		g_free( uri );

		caja_menus_list = get_file_or_background_items(
				CAJA_ACTIONS( provider ), ITEM_TARGET_LOCATION, ( void * ) current_folder );
	}

	return( caja_menus_list );
}

/*
 * this function is called each time the selection changed
 * menus items are available :
 * a) in Edit menu while the selection stays unchanged
 * b) in contextual menu while the selection stays unchanged
 */
static GList *
menu_provider_get_file_items( CajaMenuProvider *provider, GtkWidget *window, GList *files )
{
	static const gchar *thisfn = "caja_actions_menu_provider_get_file_items";
	GList *caja_menus_list = NULL;

	g_debug( "%s: provider=%p, window=%p, files=%p, count=%d",
			thisfn, ( void * ) provider, ( void * ) window, ( void * ) files, g_list_length( files ));

	g_return_val_if_fail( CAJA_IS_ACTIONS( provider ), NULL );

	/* no need to go further if there is no files in the list */
	if( !g_list_length( files )){
		return(( GList * ) NULL );
	}

	if( !CAJA_ACTIONS( provider )->private->dispose_has_run ){

		caja_menus_list = get_file_or_background_items(
				CAJA_ACTIONS( provider ), ITEM_TARGET_SELECTION, ( void * ) files );
	}

	return( caja_menus_list );
}

/*
 * as of 2.26, this function is only called for folders, but for the
 * desktop (x-caja-desktop:///) which seems to be only called by
 * get_background_items ; also, only actions (not menus) are displayed
 */
static GList *
menu_provider_get_toolbar_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder )
{
	static const gchar *thisfn = "caja_actions_menu_provider_get_toolbar_items";
	GList *caja_menus_list = NULL;
	gchar *uri;
	GList *selected;
	GList *pivot_tree;

	uri = caja_file_info_get_uri( current_folder );
	g_debug( "%s: provider=%p, window=%p, current_folder=%p (%s)",
			thisfn, ( void * ) provider, ( void * ) window, ( void * ) current_folder, uri );
	g_free( uri );

	if( !CAJA_ACTIONS( provider )->private->dispose_has_run ){

		pivot_tree = na_pivot_get_items( CAJA_ACTIONS( provider )->private->pivot );

		selected = na_selected_info_get_list_from_item( current_folder );

		caja_menus_list = build_caja_menus(
				CAJA_ACTIONS( provider ), pivot_tree, ITEM_TARGET_TOOLBAR, selected );

		na_selected_info_free_list( selected );
	}

	return( caja_menus_list );
}

static GList *
get_file_or_background_items( CajaActions *plugin, guint target, void *selection )
{
	GList *menus_list;
	GList *pivot_tree;
	GList *selected;
	gboolean root_menu;
	gboolean add_about;

	g_return_val_if_fail( NA_IS_PIVOT( plugin->private->pivot ), NULL );

	pivot_tree = na_pivot_get_items( plugin->private->pivot );

	if( target == ITEM_TARGET_LOCATION ){
		g_return_val_if_fail( CAJA_IS_FILE_INFO( selection ), NULL );
		selected = na_selected_info_get_list_from_item( CAJA_FILE_INFO( selection ));

	} else {
		g_return_val_if_fail( target == ITEM_TARGET_SELECTION, NULL );
		selected = na_selected_info_get_list_from_list(( GList * ) selection );
	}

	menus_list = build_caja_menus( plugin, pivot_tree, target, selected );
	/*g_debug( "%s: menus has %d level zero items", thisfn, g_list_length( menus_list ));*/

	na_selected_info_free_list( selected );

	root_menu = na_iprefs_read_bool( NA_IPREFS( plugin->private->pivot ), IPREFS_CREATE_ROOT_MENU, FALSE );
	if( root_menu ){
		menus_list = create_root_menu( plugin, menus_list );
	}

	add_about = na_iprefs_read_bool( NA_IPREFS( plugin->private->pivot ), IPREFS_ADD_ABOUT_ITEM, TRUE );
	/*g_debug( "%s: add_about=%s", thisfn, add_about ? "True":"False" );*/
	if( add_about ){
		menus_list = add_about_item( plugin, menus_list );
	}

	return( menus_list );
}

/*
 * when building a menu for the toolbar, do not use menus hierarchy
 * files is a GList of NASelectedInfo items
 */
static GList *
build_caja_menus( CajaActions *plugin, GList *tree, guint target, GList *files )
{
	static const gchar *thisfn = "caja_actions_build_caja_menus";
	GList *menus_list = NULL;
	GList *subitems, *submenu;
	GList *it;
	NAObjectProfile *profile;
	CajaMenuItem *item;
	gchar *action_label;

	g_debug( "%s: plugin=%p, tree=%p, target=%d, files=%p (count=%d)",
			thisfn, ( void * ) plugin, ( void * ) tree, target,
			( void * ) files, g_list_length( files ));

	for( it = tree ; it ; it = it->next ){

		g_return_val_if_fail( NA_IS_OBJECT_ITEM( it->data ), NULL );

#ifdef NA_MAINTAINER_MODE
		/* check this here as a security though NAPivot should only have
		 * loaded valid and enabled items
		 */
		if( !na_object_is_enabled( it->data ) || !na_object_is_valid( it->data )){
			gchar *label = na_object_get_label( it->data );
			g_warning( "%s: '%s' item: enabled=%s, valid=%s", thisfn, label,
					na_object_is_enabled( it->data ) ? "True":"False",
					na_object_is_valid( it->data ) ? "True":"False" );
			g_free( label );
			continue;
		}
#endif

		/*if( !na_icontext_is_candidate( it->data, target, files )){
			continue;
		}*/

		/* recursively build sub-menus
		 */
		if( NA_IS_OBJECT_MENU( it->data )){
			subitems = na_object_get_items( it->data );
			submenu = build_caja_menus( plugin, subitems, target, files );
			/*g_debug( "%s: submenu has %d items", thisfn, g_list_length( submenu ));*/

			if( submenu ){
				if( target != ITEM_TARGET_TOOLBAR ){
					item = create_item_from_menu( NA_OBJECT_MENU( it->data ), submenu );
					menus_list = g_list_append( menus_list, item );

				} else {
					menus_list = g_list_concat( menus_list, submenu );
				}
			}
			continue;
		}

		g_return_val_if_fail( NA_IS_OBJECT_ACTION( it->data ), NULL );
		action_label = na_object_get_label( it->data );

		/* to be removed when NAObjectAction will implement NAIContext interface
		 */
		if( !na_object_action_is_candidate( it->data, target, files )){
			g_debug( "%s: action %s is not candidate", thisfn, action_label );
			g_free( action_label );
			continue;
		}

		g_debug( "%s: action %s is candidate", thisfn, action_label );
		profile = get_candidate_profile( plugin, NA_OBJECT_ACTION( it->data ), target, files );
		if( profile ){
			item = create_item_from_profile( profile, target, files );
			menus_list = g_list_append( menus_list, item );
		}

		g_free( action_label );
	}

	return( menus_list );
}

/*
 * could also be a NAObjectAction method - but this is not used elsewhere
 */
static NAObjectProfile *
get_candidate_profile( CajaActions *plugin, NAObjectAction *action, guint target, GList *files )
{
	static const gchar *thisfn = "caja_actions_get_candidate_profile";
	NAObjectProfile *candidate = NULL;
	gchar *action_label;
	gchar *profile_label;
	GList *profiles, *ip;

	action_label = na_object_get_label( action );
	profiles = na_object_get_items( action );

	for( ip = profiles ; ip && !candidate ; ip = ip->next ){
		NAObjectProfile *profile = NA_OBJECT_PROFILE( ip->data );

		if( na_icontext_is_candidate( NA_ICONTEXT( profile ), target, files )){
			profile_label = na_object_get_label( profile );
			g_debug( "%s: selecting %s (profile=%p '%s')", thisfn, action_label, ( void * ) profile, profile_label );
			g_free( profile_label );

			candidate = profile;
		}
	}

	g_free( action_label );

	return( candidate );
}

static CajaMenuItem *
create_item_from_profile( NAObjectProfile *profile, guint target, GList *files )
{
	CajaMenuItem *item;
	NAObjectAction *action;
	NAObjectProfile *duplicate;

	action = NA_OBJECT_ACTION( na_object_get_parent( profile ));
	duplicate = NA_OBJECT_PROFILE( na_object_duplicate( profile ));
	na_object_set_parent( duplicate, NULL );

	item = create_menu_item( NA_OBJECT_ITEM( action ));

	/* attach a weak ref on the Caja menu item: our profile will be
	 * unreffed in weak notify function
	 */
	g_signal_connect( item,
				"activate",
				G_CALLBACK( execute_action ),
				duplicate );

	g_object_weak_ref( G_OBJECT( item ), ( GWeakNotify ) weak_notify_profile, duplicate );

	g_object_set_data_full( G_OBJECT( item ),
			"caja-actions-files",
			na_selected_info_copy_list( files ),
			( GDestroyNotify ) destroy_notify_file_list );

	g_object_set_data( G_OBJECT( item ),
			"caja-actions-target",
			GUINT_TO_POINTER( target ));

	return( item );
}

static void
weak_notify_profile( NAObjectProfile *profile, CajaMenuItem *item )
{
	g_debug( "caja_actions_weak_notify_profile: profile=%p (ref_count=%d)",
			( void * ) profile, G_OBJECT( profile )->ref_count );
	g_object_unref( profile );
}

static void
destroy_notify_file_list( GList *list)
{
	g_debug( "caja_actions_destroy_notify_file_list" );
	na_selected_info_free_list( list );
}

/*
 * note that each appended CajaMenuItem is ref-ed by the CajaMenu
 * we can so safely release our own ref on subitems after this function
 */
static CajaMenuItem *
create_item_from_menu( NAObjectMenu *menu, GList *subitems )
{
	/*static const gchar *thisfn = "caja_actions_create_item_from_menu";*/
	CajaMenuItem *item;

	item = create_menu_item( NA_OBJECT_ITEM( menu ));
	g_object_weak_ref( G_OBJECT( item ), ( GWeakNotify ) weak_notify_menu, menu );

	attach_submenu_to_item( item, subitems );
	caja_menu_item_list_free( subitems );

	/*g_debug( "%s: returning item=%p", thisfn, ( void * ) item );*/
	return( item );
}

static void
weak_notify_menu( NAObjectMenu *menu, CajaMenuItem *item )
{
	g_debug( "caja_actions_weak_notify_menu: menu=%p (ref_count=%d)",
			( void * ) menu, G_OBJECT( menu )->ref_count );
	/*g_object_unref( menu );*/
}

static CajaMenuItem *
create_menu_item( NAObjectItem *item )
{
	CajaMenuItem *menu_item;
	gchar *id, *name, *label, *tooltip, *icon;

	id = na_object_get_id( item );
	name = g_strdup_printf( "%s-%s-%s", PACKAGE, G_OBJECT_TYPE_NAME( item ), id );
	label = na_object_get_label( item );
	/*g_debug( "caja_actions_create_menu_item: %s - %s", name, label );*/
	tooltip = na_object_get_tooltip( item );
	icon = na_object_get_icon( item );

	menu_item = caja_menu_item_new( name, label, tooltip, icon );

	g_free( icon );
 	g_free( tooltip );
 	g_free( label );
 	g_free( name );
 	g_free( id );

	return( menu_item );
}

static void
attach_submenu_to_item( CajaMenuItem *item, GList *subitems )
{
	CajaMenu *submenu;
	GList *it;

	submenu = caja_menu_new();
	caja_menu_item_set_submenu( item, submenu );

	for( it = subitems ; it ; it = it->next ){
		caja_menu_append_item( submenu, CAJA_MENU_ITEM( it->data ));
	}
}

static void
execute_action( CajaMenuItem *item, NAObjectProfile *profile )
{
	static const gchar *thisfn = "caja_actions_execute_action";
	GList *files;
	GString *cmd;
	gchar *param, *path;
	guint target;

	g_debug( "%s: item=%p, profile=%p", thisfn, ( void * ) item, ( void * ) profile );

	files = ( GList * ) g_object_get_data( G_OBJECT( item ), "caja-actions-files" );
	target = GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( item ), "caja-actions-target" ));

	path = na_object_get_path( profile );
	cmd = g_string_new( path );

	param = na_object_profile_parse_parameters( profile, target, files );

	if( param != NULL ){
		g_string_append_printf( cmd, " %s", param );
		g_free( param );
	}

	g_debug( "%s: executing '%s'", thisfn, cmd->str );
	g_spawn_command_line_async( cmd->str, NULL );

	g_string_free (cmd, TRUE);
	g_free( path );
}

/*
 * create a root submenu
 */
static GList *
create_root_menu( CajaActions *plugin, GList *menu )
{
	static const gchar *thisfn = "caja_actions_create_root_menu";
	GList *caja_menu;
	CajaMenuItem *root_item;
	gchar *icon;

	g_debug( "%s: plugin=%p, menu=%p (%d items)",
			thisfn, ( void * ) plugin, ( void * ) menu, g_list_length( menu ));

	if( !menu || !g_list_length( menu )){
		return( NULL );
	}

	icon = na_iabout_get_icon_name();
	root_item = caja_menu_item_new( "CajaActionsExtensions",
				/* i18n: label of an automagic root submenu */
				_( "Caja Actions" ),
				/* i18n: tooltip of an automagic root submenu */
				_( "A submenu which embeds the currently available Caja Actions extensions" ),
				icon );
	attach_submenu_to_item( root_item, menu );
	caja_menu = g_list_append( NULL, root_item );
	g_free( icon );

	return( caja_menu );
}

/*
 * if there is a root submenu,
 * then add the about item to the end of the first level of this menu
 */
static GList *
add_about_item( CajaActions *plugin, GList *menu )
{
	static const gchar *thisfn = "caja_actions_add_about_item";
	GList *caja_menu;
	gboolean have_root_menu;
	CajaMenuItem *root_item;
	CajaMenuItem *about_item;
	CajaMenu *first;
	gchar *icon;

	g_debug( "%s: plugin=%p, menu=%p (%d items)",
			thisfn, ( void * ) plugin, ( void * ) menu, g_list_length( menu ));

	if( !menu || !g_list_length( menu )){
		return( NULL );
	}

	have_root_menu = FALSE;
	caja_menu = menu;

	if( g_list_length( menu ) == 1 ){
		root_item = CAJA_MENU_ITEM( menu->data );
		g_object_get( G_OBJECT( root_item ), "menu", &first, NULL );
		if( first ){
			g_return_val_if_fail( CAJA_IS_MENU( first ), NULL );
			have_root_menu = TRUE;
		}
	}

	if( have_root_menu ){
		icon = na_iabout_get_icon_name();

		about_item = caja_menu_item_new( "AboutCajaActions",
					_( "About Caja Actions" ),
					_( "Display information about Caja Actions" ),
					icon );

		g_signal_connect_data( about_item,
					"activate",
					G_CALLBACK( execute_about ),
					plugin,
					NULL,
					0 );

		caja_menu_append_item( first, about_item );

		g_free( icon );
	}

	return( caja_menu );
}

static void
execute_about( CajaMenuItem *item, CajaActions *plugin )
{
	g_return_if_fail( CAJA_IS_ACTIONS( plugin ));
	g_return_if_fail( NA_IS_IABOUT( plugin ));

	na_iabout_display( NA_IABOUT( plugin ));
}
