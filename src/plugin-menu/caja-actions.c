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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>

#include <libcaja-extension/caja-extension-types.h>
#include <libcaja-extension/caja-file-info.h>
#include <libcaja-extension/caja-menu-provider.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>
#include <api/na-timeout.h>

#include <core/na-pivot.h>
#include <core/na-about.h>
#include <core/na-selected-info.h>
#include <core/na-tokens.h>

#include "caja-actions.h"

/* private class data
 */
struct _CajaActionsClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CajaActionsPrivate {
	gboolean  dispose_has_run;
	NAPivot  *pivot;
	gulong    items_changed_handler;
	gulong    settings_changed_handler;
	NATimeout change_timeout;
};

static GObjectClass *st_parent_class  = NULL;
static GType         st_actions_type  = 0;
static gint          st_burst_timeout = 100;		/* burst timeout in msec */

static void              class_init( CajaActionsClass *klass );
static void              instance_init( GTypeInstance *instance, gpointer klass );
static void              instance_constructed( GObject *object );
static void              instance_dispose( GObject *object );
static void              instance_finalize( GObject *object );

static void              menu_provider_iface_init( CajaMenuProviderIface *iface );
static GList            *menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder );
static GList            *menu_provider_get_file_items( CajaMenuProvider *provider, GtkWidget *window, GList *files );

#ifdef HAVE_CAJA_MENU_PROVIDER_GET_TOOLBAR_ITEMS
static GList            *menu_provider_get_toolbar_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder );
#endif

static GList            *build_caja_menu( CajaActions *plugin, guint target, GList *selection );
static GList            *build_caja_menu_rec( GList *tree, guint target, GList *selection, NATokens *tokens );
static NAObjectItem     *expand_tokens_item( const NAObjectItem *item, NATokens *tokens );
static void              expand_tokens_context( NAIContext *context, NATokens *tokens );
static NAObjectProfile  *get_candidate_profile( NAObjectAction *action, guint target, GList *files );
static CajaMenuItem *create_item_from_profile( NAObjectProfile *profile, guint target, GList *files, NATokens *tokens );
static CajaMenuItem *create_item_from_menu( NAObjectMenu *menu, GList *subitems, guint target );
static CajaMenuItem *create_menu_item( const NAObjectItem *item, guint target );
static void              weak_notify_menu_item( void *user_data /* =NULL */, CajaMenuItem *item );
static void              attach_submenu_to_item( CajaMenuItem *item, GList *subitems );
static void              weak_notify_profile( NAObjectProfile *profile, CajaMenuItem *item );

static void              execute_action( CajaMenuItem *item, NAObjectProfile *profile );

static GList            *create_root_menu( CajaActions *plugin, GList *caja_menu );
static GList            *add_about_item( CajaActions *plugin, GList *caja_menu );
static void              execute_about( CajaMenuItem *item, CajaActions *plugin );

static void              on_pivot_items_changed_handler( NAPivot *pivot, CajaActions *plugin );
static void              on_settings_key_changed_handler( const gchar *group, const gchar *key, gconstpointer new_value, gboolean mandatory, CajaActions *plugin );
static void              on_change_event_timeout( CajaActions *plugin );

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

	g_assert( st_actions_type == 0 );

	g_debug( "%s: module=%p", thisfn, ( void * ) module );

	st_actions_type = g_type_module_register_type( module, G_TYPE_OBJECT, "CajaActions", &info, 0 );

	g_type_module_add_interface( module, st_actions_type, CAJA_TYPE_MENU_PROVIDER, &menu_provider_iface_info );
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

	g_return_if_fail( CAJA_IS_ACTIONS( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CAJA_ACTIONS( instance );

	self->private = g_new0( CajaActionsPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->change_timeout.timeout = st_burst_timeout;
	self->private->change_timeout.handler = ( NATimeoutFunc ) on_change_event_timeout;
	self->private->change_timeout.user_data = self;
	self->private->change_timeout.source_id = 0;
}

/*
 * Runtime modification management:
 * We have to react to some runtime environment modifications:
 *
 * - whether the items list has changed (we have to reload a new pivot)
 *   > registering for notifications against NAPivot
 *
 * - whether to add the 'About Caja-Actions' item
 * - whether to create a 'Caja-Actions actions' root menu
 *   > registering for notifications against NASettings
 */
static void
instance_constructed( GObject *object )
{
	static const gchar *thisfn = "caja_actions_instance_constructed";
	CajaActionsPrivate *priv;

	g_return_if_fail( CAJA_IS_ACTIONS( object ));

	priv = CAJA_ACTIONS( object )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( object );
		}

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		priv->pivot = na_pivot_new();

		/* setup NAPivot properties before loading items
		 */
		na_pivot_set_loadable( priv->pivot, !PIVOT_LOAD_DISABLED & !PIVOT_LOAD_INVALID );
		na_pivot_load_items( priv->pivot );

		/* register against NAPivot to be notified of items changes
		 */
		priv->items_changed_handler =
				g_signal_connect( priv->pivot,
						PIVOT_SIGNAL_ITEMS_CHANGED,
						G_CALLBACK( on_pivot_items_changed_handler ),
						object );

		/* register against NASettings to be notified of changes on
		 *  our runtime preferences
		 * because we only monitor here a few runtime keys, we prefer the
		 * callback way that the signal one
		 */
		na_settings_register_key_callback(
				NA_IPREFS_IO_PROVIDERS_READ_STATUS,
				G_CALLBACK( on_settings_key_changed_handler ),
				object );

		na_settings_register_key_callback(
				NA_IPREFS_ITEMS_ADD_ABOUT_ITEM,
				G_CALLBACK( on_settings_key_changed_handler ),
				object );

		na_settings_register_key_callback(
				NA_IPREFS_ITEMS_CREATE_ROOT_MENU,
				G_CALLBACK( on_settings_key_changed_handler ),
				object );

		na_settings_register_key_callback(
				NA_IPREFS_ITEMS_LEVEL_ZERO_ORDER,
				G_CALLBACK( on_settings_key_changed_handler ),
				object );

		na_settings_register_key_callback(
				NA_IPREFS_ITEMS_LIST_ORDER_MODE,
				G_CALLBACK( on_settings_key_changed_handler ),
				object );
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

		if( self->private->items_changed_handler ){
			g_signal_handler_disconnect( self->private->pivot, self->private->items_changed_handler );
		}
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

static void
menu_provider_iface_init( CajaMenuProviderIface *iface )
{
	static const gchar *thisfn = "caja_actions_menu_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_file_items = menu_provider_get_file_items;
	iface->get_background_items = menu_provider_get_background_items;

#ifdef HAVE_CAJA_MENU_PROVIDER_GET_TOOLBAR_ITEMS
	iface->get_toolbar_items = menu_provider_get_toolbar_items;
#endif
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
 *
 * Note that 'x-caja-desktop:///' cannot be interpreted by
 * #NASelectedInfo::query_file_attributes() function. It so never participate
 * to the display of actions.
 */
static GList *
menu_provider_get_background_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder )
{
	static const gchar *thisfn = "caja_actions_menu_provider_get_background_items";
	GList *caja_menus_list = NULL;
	gchar *uri;
	GList *selected;

	g_return_val_if_fail( CAJA_IS_ACTIONS( provider ), NULL );

	if( !CAJA_ACTIONS( provider )->private->dispose_has_run ){

		selected = na_selected_info_get_list_from_item( current_folder );

		if( selected ){
			uri = caja_file_info_get_uri( current_folder );
			g_debug( "%s: provider=%p, window=%p, current_folder=%p (%s)",
					thisfn,
					( void * ) provider,
					( void * ) window,
					( void * ) current_folder, uri );
			g_free( uri );

			caja_menus_list = build_caja_menu(
					CAJA_ACTIONS( provider ),
					ITEM_TARGET_LOCATION,
					selected );

			na_selected_info_free_list( selected );
		}
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
	GList *selected;

	g_return_val_if_fail( CAJA_IS_ACTIONS( provider ), NULL );

	if( !CAJA_ACTIONS( provider )->private->dispose_has_run ){

		/* no need to go further if there is no files in the list */
		if( !g_list_length( files )){
			return(( GList * ) NULL );
		}

		selected = na_selected_info_get_list_from_list(( GList * ) files );

		if( selected ){
			g_debug( "%s: provider=%p, window=%p, files=%p, count=%d",
					thisfn,
					( void * ) provider,
					( void * ) window,
					( void * ) files, g_list_length( files ));

#ifdef NA_MAINTAINER_MODE
			GList *im;
			for( im = files ; im ; im = im->next ){
				gchar *uri = caja_file_info_get_uri( CAJA_FILE_INFO( im->data ));
				gchar *mimetype = caja_file_info_get_mime_type( CAJA_FILE_INFO( im->data ));
				g_debug( "%s: uri='%s', mimetype='%s'", thisfn, uri, mimetype );
				g_free( mimetype );
				g_free( uri );
			}
#endif

			caja_menus_list = build_caja_menu(
					CAJA_ACTIONS( provider ),
					ITEM_TARGET_SELECTION,
					selected );

			na_selected_info_free_list( selected );
		}
	}

	return( caja_menus_list );
}

#ifdef HAVE_CAJA_MENU_PROVIDER_GET_TOOLBAR_ITEMS
/*
 * as of 2.26, this function is only called for folders, but for the
 * desktop (x-caja-desktop:///) which seems to be only called by
 * get_background_items ; also, only actions (not menus) are displayed
 *
 * the API is removed starting with Caja 2.91.90
 */
static GList *
menu_provider_get_toolbar_items( CajaMenuProvider *provider, GtkWidget *window, CajaFileInfo *current_folder )
{
	static const gchar *thisfn = "caja_actions_menu_provider_get_toolbar_items";
	GList *caja_menus_list = NULL;
	gchar *uri;
	GList *selected;

	g_return_val_if_fail( CAJA_IS_ACTIONS( provider ), NULL );

	if( !CAJA_ACTIONS( provider )->private->dispose_has_run ){

		selected = na_selected_info_get_list_from_item( current_folder );

		if( selected ){
			uri = caja_file_info_get_uri( current_folder );
			g_debug( "%s: provider=%p, window=%p, current_folder=%p (%s)",
					thisfn,
					( void * ) provider,
					( void * ) window,
					( void * ) current_folder, uri );
			g_free( uri );

			caja_menus_list = build_caja_menu(
					CAJA_ACTIONS( provider ),
					ITEM_TARGET_TOOLBAR,
					selected );

			na_selected_info_free_list( selected );
		}
	}

	return( caja_menus_list );
}
#endif

/*
 * build_caja_menu:
 * @target: whether the menu targets a location (a folder) or a selection
 *  (the list of currently selected items in the file manager)
 * @selection: a list of NASelectedInfo, with:
 *  - only one item if a location
 *  - one item by selected file manager item, if a selection.
 *  Note: a NASelectedInfo is just a sort of CajaFileInfo, with
 *  some added APIs.
 *
 * Build the Caja menu as a list of CajaMenuItem items
 *
 * Returns: the Caja menu
 */
static GList *
build_caja_menu( CajaActions *plugin, guint target, GList *selection )
{
	GList *caja_menu;
	NATokens *tokens;
	GList *tree;
	gboolean items_add_about_item;
	gboolean items_create_root_menu;

	g_return_val_if_fail( NA_IS_PIVOT( plugin->private->pivot ), NULL );

	tokens = na_tokens_new_from_selection( selection );

	tree = na_pivot_get_items( plugin->private->pivot );

	caja_menu = build_caja_menu_rec( tree, target, selection, tokens );

	/* the NATokens object has been attached (and reffed) by each found
	 * candidate profile, so it will be actually finalized only on actual
	 * CajaMenu finalization itself
	 */
	g_object_unref( tokens );

	if( target != ITEM_TARGET_TOOLBAR && caja_menu && g_list_length( caja_menu )){

		items_create_root_menu = na_settings_get_boolean( NA_IPREFS_ITEMS_CREATE_ROOT_MENU, NULL, NULL );
		if( items_create_root_menu ){
			caja_menu = create_root_menu( plugin, caja_menu );

			items_add_about_item = na_settings_get_boolean( NA_IPREFS_ITEMS_ADD_ABOUT_ITEM, NULL, NULL );
			if( items_add_about_item ){
				caja_menu = add_about_item( plugin, caja_menu );
			}
		}
	}

	return( caja_menu );
}

static GList *
build_caja_menu_rec( GList *tree, guint target, GList *selection, NATokens *tokens )
{
	static const gchar *thisfn = "caja_actions_build_caja_menu_rec";
	GList *caja_menu;
	GList *it;
	GList *subitems;
	NAObjectItem *item;
	GList *submenu;
	NAObjectProfile *profile;
	CajaMenuItem *menu_item;
	gchar *label;

	caja_menu = NULL;

	for( it = tree ; it ; it = it->next ){

		g_return_val_if_fail( NA_IS_OBJECT_ITEM( it->data ), NULL );
		label = na_object_get_label( it->data );
		g_debug( "%s: examining %s", thisfn, label );

		if( !na_icontext_is_candidate( NA_ICONTEXT( it->data ), target, selection )){
			g_debug( "%s: is not candidate (NAIContext): %s", thisfn, label );
			g_free( label );
			continue;
		}

		item = expand_tokens_item( NA_OBJECT_ITEM( it->data ), tokens );

		/* but we have to re-check for validity as a label may become
		 * dynamically empty - thus the NAObjectItem invalid :(
		 */
		if( !na_object_is_valid( item )){
			g_debug( "%s: item %s becomes invalid after tokens expansion", thisfn, label );
			g_object_unref( item );
			g_free( label );
			continue;
		}

		/* recursively build sub-menus
		 * the 'submenu' menu of cajaMenuItem's is attached to the returned
		 * 'item'
		 */
		if( NA_IS_OBJECT_MENU( it->data )){

			subitems = na_object_get_items( NA_OBJECT( it->data ));
			g_debug( "%s: menu has %d items", thisfn, g_list_length( subitems ));

			submenu = build_caja_menu_rec( subitems, target, selection, tokens );
			g_debug( "%s: submenu has %d items", thisfn, g_list_length( submenu ));

			if( submenu ){
				if( target == ITEM_TARGET_TOOLBAR ){
					caja_menu = g_list_concat( caja_menu, submenu );

				} else {
					menu_item = create_item_from_menu( NA_OBJECT_MENU( item ), submenu, target );
					caja_menu = g_list_append( caja_menu, menu_item );
				}
			}
			g_object_unref( item );
			g_free( label );
			continue;
		}

		g_return_val_if_fail( NA_IS_OBJECT_ACTION( item ), NULL );

		/* if we have an action, searches for a candidate profile
		 */
		profile = get_candidate_profile( NA_OBJECT_ACTION( item ), target, selection );
		if( profile ){
			menu_item = create_item_from_profile( profile, target, selection, tokens );
			caja_menu = g_list_append( caja_menu, menu_item );

		} else {
			g_debug( "%s: %s does not have any valid candidate profile", thisfn, label );
		}

		g_object_unref( item );
		g_free( label );
	}

	return( caja_menu );
}

/*
 * expand_tokens_item:
 * @item: a NAObjectItem read from the NAPivot.
 * @tokens: the NATokens object which holds current selection data
 *  (uris, basenames, mimetypes, etc.)
 *
 * Updates the @item, replacing parameters with the corresponding token.
 *
 * This function is not recursive, but works for the plain item:
 * - the menu (itself)
 * - the action and its profiles
 *
 * Returns: a duplicated object which has to be g_object_unref() by the caller.
 */
static NAObjectItem *
expand_tokens_item( const NAObjectItem *src, NATokens *tokens )
{
	gchar *old, *new;
	GSList *subitems_slist, *its, *new_slist;
	GList *subitems, *it;
	NAObjectItem *item;

	item = NA_OBJECT_ITEM( na_object_duplicate( src, DUPLICATE_OBJECT ));

	/* label, tooltip and icon name
	 * plus the toolbar label if this is an action
	 */
	old = na_object_get_label( item );
	new = na_tokens_parse_for_display( tokens, old, TRUE );
	na_object_set_label( item, new );
	g_free( old );
	g_free( new );

	old = na_object_get_tooltip( item );
	new = na_tokens_parse_for_display( tokens, old, TRUE );
	na_object_set_tooltip( item, new );
	g_free( old );
	g_free( new );

	old = na_object_get_icon( item );
	new = na_tokens_parse_for_display( tokens, old, TRUE );
	na_object_set_icon( item, new );
	g_free( old );
	g_free( new );

	if( NA_IS_OBJECT_ACTION( item )){
		old = na_object_get_toolbar_label( item );
		new = na_tokens_parse_for_display( tokens, old, TRUE );
		na_object_set_toolbar_label( item, new );
		g_free( old );
		g_free( new );
	}

	/* A NAObjectItem, whether it is an action or a menu, is also a NAIContext
	 */
	expand_tokens_context( NA_ICONTEXT( item ), tokens );

	/* subitems lists, whether this is the profiles list of an action
	 * or the items list of a menu, may be dynamic and embed a command;
	 * this command itself may embed parameters
	 */
	subitems_slist = na_object_get_items_slist( item );
	new_slist = NULL;
	for( its = subitems_slist ; its ; its = its->next ){
		old = ( gchar * ) its->data;
		if( old[0] == '[' && old[strlen(old)-1] == ']' ){
			new = na_tokens_parse_for_display( tokens, old, FALSE );
		} else {
			new = g_strdup( old );
		}
		new_slist = g_slist_prepend( new_slist, new );
	}
	na_object_set_items_slist( item, new_slist );
	na_core_utils_slist_free( subitems_slist );
	na_core_utils_slist_free( new_slist );

	/* last, deal with profiles of an action
	 */
	if( NA_IS_OBJECT_ACTION( item )){

		subitems = na_object_get_items( item );

		for( it = subitems ; it ; it = it->next ){

			/* desktop Exec key = MateConf path+parameters
			 * do not touch them here
			 */
			old = na_object_get_working_dir( it->data );
			new = na_tokens_parse_for_display( tokens, old, FALSE );
			na_object_set_working_dir( it->data, new );
			g_free( old );
			g_free( new );

			/* a NAObjectProfile is also a NAIContext
			 */
			expand_tokens_context( NA_ICONTEXT( it->data ), tokens );
		}
	}

	return( item );
}

static void
expand_tokens_context( NAIContext *context, NATokens *tokens )
{
	gchar *old, *new;

	old = na_object_get_try_exec( context );
	new = na_tokens_parse_for_display( tokens, old, FALSE );
	na_object_set_try_exec( context, new );
	g_free( old );
	g_free( new );

	old = na_object_get_show_if_registered( context );
	new = na_tokens_parse_for_display( tokens, old, FALSE );
	na_object_set_show_if_registered( context, new );
	g_free( old );
	g_free( new );

	old = na_object_get_show_if_true( context );
	new = na_tokens_parse_for_display( tokens, old, FALSE );
	na_object_set_show_if_true( context, new );
	g_free( old );
	g_free( new );

	old = na_object_get_show_if_running( context );
	new = na_tokens_parse_for_display( tokens, old, FALSE );
	na_object_set_show_if_running( context, new );
	g_free( old );
	g_free( new );
}

/*
 * could also be a NAObjectAction method - but this is not used elsewhere
 */
static NAObjectProfile *
get_candidate_profile( NAObjectAction *action, guint target, GList *files )
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
create_item_from_profile( NAObjectProfile *profile, guint target, GList *files, NATokens *tokens )
{
	CajaMenuItem *item;
	NAObjectAction *action;
	NAObjectProfile *duplicate;

	action = NA_OBJECT_ACTION( na_object_get_parent( profile ));
	duplicate = NA_OBJECT_PROFILE( na_object_duplicate( profile, DUPLICATE_ONLY ));
	na_object_set_parent( duplicate, NULL );

	item = create_menu_item( NA_OBJECT_ITEM( action ), target );

	g_signal_connect( item,
				"activate",
				G_CALLBACK( execute_action ),
				duplicate );

	/* unref the duplicated profile on menu item finalization
	 */
	g_object_weak_ref( G_OBJECT( item ), ( GWeakNotify ) weak_notify_profile, duplicate );

	g_object_set_data_full( G_OBJECT( item ),
			"caja-actions-tokens",
			g_object_ref( tokens ),
			( GDestroyNotify ) g_object_unref );

	return( item );
}

/*
 * called _after_ the CajaMenuItem has been finalized
 */
static void
weak_notify_profile( NAObjectProfile *profile, CajaMenuItem *item )
{
	g_debug( "caja_actions_weak_notify_profile: profile=%p (ref_count=%d)",
			( void * ) profile, G_OBJECT( profile )->ref_count );

	g_object_unref( profile );
}

/*
 * note that each appended CajaMenuItem is ref-ed by the CajaMenu
 * we can so safely release our own ref on subitems after having attached
 * the submenu
 */
static CajaMenuItem *
create_item_from_menu( NAObjectMenu *menu, GList *subitems, guint target )
{
	/*static const gchar *thisfn = "caja_actions_create_item_from_menu";*/
	CajaMenuItem *item;

	item = create_menu_item( NA_OBJECT_ITEM( menu ), target );

	attach_submenu_to_item( item, subitems );

	caja_menu_item_list_free( subitems );

	/*g_debug( "%s: returning item=%p", thisfn, ( void * ) item );*/
	return( item );
}

/*
 * Creates a CajaMenuItem
 *
 * We attach a weak notify function to the created item in order to be able
 * to check for instanciation/finalization cycles
 */
static CajaMenuItem *
create_menu_item( const NAObjectItem *item, guint target )
{
	CajaMenuItem *menu_item;
	gchar *id, *name, *label, *tooltip, *icon;

	id = na_object_get_id( item );
	name = g_strdup_printf( "%s-%s-%s-%d", PACKAGE, G_OBJECT_TYPE_NAME( item ), id, target );
	label = na_object_get_label( item );
	tooltip = na_object_get_tooltip( item );
	icon = na_object_get_icon( item );

	menu_item = caja_menu_item_new( name, label, tooltip, icon );

	g_object_weak_ref( G_OBJECT( menu_item ), ( GWeakNotify ) weak_notify_menu_item, NULL );

	g_free( icon );
 	g_free( tooltip );
 	g_free( label );
 	g_free( name );
 	g_free( id );

	return( menu_item );
}

/*
 * called _after_ the CajaMenuItem has been finalized
 */
static void
weak_notify_menu_item( void *user_data /* =NULL */, CajaMenuItem *item )
{
	g_debug( "caja_actions_weak_notify_menu_item: item=%p", ( void * ) item );
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

/*
 * callback triggered when an item is activated
 * path and parameters must yet been parsed against tokens
 *
 * note that if first parameter if of singular form, then we have to loop
 * againt the selected, each time replacing the singular parameters with
 * the current item of the selection
 */
static void
execute_action( CajaMenuItem *item, NAObjectProfile *profile )
{
	static const gchar *thisfn = "caja_actions_execute_action";
	NATokens *tokens;

	g_debug( "%s: item=%p, profile=%p", thisfn, ( void * ) item, ( void * ) profile );

	tokens = NA_TOKENS( g_object_get_data( G_OBJECT( item ), "caja-actions-tokens" ));
	na_tokens_execute_action( tokens, profile );
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

	g_debug( "%s: plugin=%p, menu=%p (%d items)",
			thisfn, ( void * ) plugin, ( void * ) menu, g_list_length( menu ));

	if( !menu || !g_list_length( menu )){
		return( NULL );
	}

	root_item = caja_menu_item_new(
			"CajaActionsExtensions",
			/* i18n: label of an automagic root submenu */
			_( "Caja-Actions actions" ),
			/* i18n: tooltip of an automagic root submenu */
			_( "A submenu which embeds the currently available Caja-Actions actions and menus" ),
			na_about_get_icon_name());
	attach_submenu_to_item( root_item, menu );
	caja_menu = g_list_append( NULL, root_item );

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
		about_item = caja_menu_item_new(
				"AboutCajaActions",
				_( "About Caja-Actions" ),
				_( "Display some information about Caja-Actions" ),
				na_about_get_icon_name());

		g_signal_connect_data(
				about_item,
				"activate",
				G_CALLBACK( execute_about ),
				plugin,
				NULL,
				0 );

		caja_menu_append_item( first, about_item );
	}

	return( caja_menu );
}

static void
execute_about( CajaMenuItem *item, CajaActions *plugin )
{
	g_return_if_fail( CAJA_IS_ACTIONS( plugin ));

	na_about_display( NULL );
}

/*
 * Not only the items list itself, but also several runtime preferences have
 * an effect on the display of items in file manager context menu.
 *
 * We of course monitor here all these information; only asking NAPivot
 * for reloading its items when we detect the end of a burst of changes.
 *
 * Only when NAPivot has finished with reloading its items list, then we
 * inform the file manager that its items list has changed.
 */

/* signal emitted by NAPivot at the end of a burst of 'item-changed' signals
 * from i/o providers
 */
static void
on_pivot_items_changed_handler( NAPivot *pivot, CajaActions *plugin )
{
	g_return_if_fail( NA_IS_PIVOT( pivot ));
	g_return_if_fail( CAJA_IS_ACTIONS( plugin ));

	if( !plugin->private->dispose_has_run ){

		na_timeout_event( &plugin->private->change_timeout );
	}
}

/* callback triggered by NASettings at the end of a burst of 'changed' signals
 * on runtime preferences which may affect the way file manager displays
 * its context menus
 */
static void
on_settings_key_changed_handler( const gchar *group, const gchar *key, gconstpointer new_value, gboolean mandatory, CajaActions *plugin )
{
	g_return_if_fail( CAJA_IS_ACTIONS( plugin ));

	if( !plugin->private->dispose_has_run ){

		na_timeout_event( &plugin->private->change_timeout );
	}
}

/*
 * automatically reloads the items, then signal the file manager.
 */
static void
on_change_event_timeout( CajaActions *plugin )
{
	static const gchar *thisfn = "caja_actions_on_change_event_timeout";
	g_debug( "%s: timeout expired", thisfn );

	na_pivot_load_items( plugin->private->pivot );
	caja_menu_provider_emit_items_updated_signal( CAJA_MENU_PROVIDER( plugin ));
}
