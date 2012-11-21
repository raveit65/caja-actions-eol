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

#include <core/na-iprefs.h>
#include <core/na-updater.h>

#include "cact-application.h"
#include "cact-sort-buttons.h"
#include "cact-tree-view.h"

/* private class data
 */
struct _CactSortButtonsClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

struct _CactSortButtonsPrivate {
	gboolean    dispose_has_run;
	BaseWindow *window;
	NAUpdater  *updater;
	gboolean    toggling;
	gint        active;
	guint       count_items;
};

typedef struct {
	gchar           *btn_name;
	guint            order_mode;
	GtkToggleButton *button;
}
	ToggleGroup;

static ToggleGroup st_toggle_group [] = {
		{ "SortManualButton", IPREFS_ORDER_MANUAL,           NULL },
		{ "SortUpButton",     IPREFS_ORDER_ALPHA_ASCENDING,  NULL },
		{ "SortDownButton",   IPREFS_ORDER_ALPHA_DESCENDING, NULL },
		{ NULL }
};

#define WINDOW_DATA_SORT_BUTTONS				"window-data-sort-buttons"

static GObjectClass *st_parent_class = NULL;

static GType register_type( void );
static void  class_init( CactSortButtonsClass *klass );
static void  instance_init( GTypeInstance *instance, gpointer klass );
static void  instance_dispose( GObject *application );
static void  instance_finalize( GObject *application );

static void  on_base_initialize_buttons( BaseWindow *window, gpointer user_data );
static void  on_toggle_button_toggled( GtkToggleButton *button, BaseWindow *window );
static void  on_settings_order_mode_changed( const gchar *group, const gchar *key, gconstpointer new_value, gboolean mandatory, CactSortButtons *sort_buttons );
static void  on_tree_view_count_changed( BaseWindow *window, gboolean reset, gint menus_count, gint actions_count, gint profiles_count, gpointer user_data );

static void  enable_buttons( const CactSortButtons *sort_buttons, gboolean enabled );
static gint  toggle_group_get_from_mode( guint mode );
static gint  toggle_group_get_from_button( GtkToggleButton *toggled_button );

GType
cact_sort_buttons_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_sort_buttons_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactSortButtonsClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactSortButtons ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "CactSortButtons", &info, 0 );

	return( type );
}

static void
class_init( CactSortButtonsClass *klass )
{
	static const gchar *thisfn = "cact_sort_buttons_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactSortButtonsClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_sort_buttons_instance_init";
	CactSortButtons *self;

	g_return_if_fail( CACT_IS_SORT_BUTTONS( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_SORT_BUTTONS( instance );

	self->private = g_new0( CactSortButtonsPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->toggling = FALSE;
	self->private->active = -1;
	self->private->count_items = 0;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cact_sort_buttons_instance_dispose";
	CactSortButtons *self;

	g_return_if_fail( CACT_IS_SORT_BUTTONS( object ));

	self = CACT_SORT_BUTTONS( object );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *instance )
{
	static const gchar *thisfn = "cact_sort_buttons_instance_finalize";
	CactSortButtons *self;

	g_return_if_fail( CACT_IS_SORT_BUTTONS( instance ));

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	self = CACT_SORT_BUTTONS( instance );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( instance );
	}
}

/**
 * cact_sort_buttons_new:
 * @window: the main window.
 *
 * Returns: a new #CactSortButtons object.
 */
CactSortButtons *
cact_sort_buttons_new( BaseWindow *window )
{
	CactSortButtons *obj;
	CactApplication *application;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	obj = g_object_new( CACT_TYPE_SORT_BUTTONS, NULL );

	base_window_signal_connect( window,
			G_OBJECT( window ), BASE_SIGNAL_INITIALIZE_WINDOW, G_CALLBACK( on_base_initialize_buttons ));

	g_object_set_data( G_OBJECT( window ), WINDOW_DATA_SORT_BUTTONS, obj );

	obj->private->window = window;

	application = CACT_APPLICATION( base_window_get_application( window ));
	obj->private->updater = cact_application_get_updater( application );

	return( obj );
}

/*
 * on_base_initialize_buttons:
 * @window: the #BaseWindow.
 *
 * Initialization of the UI each time it is displayed.
 *
 * At end, buttons are all :
 * - all off,
 * - connected to a toggle handler,
 * - enabled (sensitive) if sort order mode is modifiable.
 */
static void
on_base_initialize_buttons( BaseWindow *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_sort_buttons_on_base_initialize_buttons";
	CactSortButtons *sort_buttons;
	gint i;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	g_debug( "%s: window=%p, user_data=%p", thisfn, ( void * ) window, ( void * ) user_data );

	base_window_signal_connect( window,
			G_OBJECT( window ), TREE_SIGNAL_COUNT_CHANGED, G_CALLBACK( on_tree_view_count_changed ));

	sort_buttons = CACT_SORT_BUTTONS( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_SORT_BUTTONS ));

	for( i = 0 ; st_toggle_group[i].btn_name ; ++i ){
		st_toggle_group[i].button =
				GTK_TOGGLE_BUTTON( base_window_get_widget( window, st_toggle_group[i].btn_name ));
		base_window_signal_connect( window,
				G_OBJECT( st_toggle_group[i].button ), "toggled", G_CALLBACK( on_toggle_button_toggled ));
	}

	na_settings_register_key_callback(
			NA_IPREFS_ITEMS_LIST_ORDER_MODE, G_CALLBACK( on_settings_order_mode_changed ), sort_buttons );

	/* for now, disable the sort buttons
	 * they will be enabled as soon as we receive the count of displayed items
	 */
	enable_buttons( sort_buttons, FALSE );
}

/*
 * if the user re-clicks on the already active buttons, reset it active
 */
static void
on_toggle_button_toggled( GtkToggleButton *toggled_button, BaseWindow *window )
{
	CactSortButtons *sort_buttons;
	gint i, ibtn;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	sort_buttons = CACT_SORT_BUTTONS( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_SORT_BUTTONS ));

	if( !sort_buttons->private->dispose_has_run ){
		if( !sort_buttons->private->toggling ){

			sort_buttons->private->toggling = TRUE;
			ibtn = toggle_group_get_from_button( toggled_button );

			/* the user re-clicks on the already active button
			 * do not let it becomes false, but keep it active
			 */
			if( ibtn == sort_buttons->private->active ){
				gtk_toggle_button_set_active( st_toggle_group[ibtn].button, TRUE );

			/* reset all buttons to false, then the clicked one to active
			 */
			} else {
				for( i = 0 ; st_toggle_group[i].btn_name ; ++i ){
					gtk_toggle_button_set_active( st_toggle_group[i].button, FALSE );
				}
				gtk_toggle_button_set_active( toggled_button, TRUE );
				sort_buttons->private->active = ibtn;
				na_iprefs_set_order_mode( st_toggle_group[ibtn].order_mode );
			}

			sort_buttons->private->toggling = FALSE;
		}
	}
}

/*
 * NASettings callback for a change on NA_IPREFS_ITEMS_LIST_ORDER_MODE key
 *
 * activate the button corresponding to the new sort order
 * desactivate the previous button
 * do nothing if new button and previous button are the sames
 *
 * testing 'toggling' is useless here because NASettings slightly delay the
 * notifications: when we toggle a button, and update the settings, then
 * we already have reset 'toggling' to FALSE when we are coming here
 */
static void
on_settings_order_mode_changed( const gchar *group, const gchar *key, gconstpointer new_value, gboolean mandatory, CactSortButtons *sort_buttons )
{
	static const gchar *thisfn = "cact_sort_buttons_on_settings_order_mode_changed";
	const gchar *order_mode_str;
	guint order_mode;
	gint ibtn;

	g_return_if_fail( CACT_IS_SORT_BUTTONS( sort_buttons ));

	if( !sort_buttons->private->dispose_has_run ){

		order_mode_str = ( const gchar * ) new_value;
		order_mode = na_iprefs_get_order_mode_by_label( order_mode_str );

		g_debug( "%s: group=%s, key=%s, order_mode=%u (%s), mandatory=%s, sort_buttons=%p (%s)",
				thisfn, group, key, order_mode, order_mode_str,
				mandatory ? "True":"False", ( void * ) sort_buttons, G_OBJECT_TYPE_NAME( sort_buttons ));

		ibtn = toggle_group_get_from_mode( order_mode );
		g_return_if_fail( ibtn >= 0 );

		if( sort_buttons->private->active == -1 || ibtn != sort_buttons->private->active ){
			sort_buttons->private->active = ibtn;
			gtk_toggle_button_set_active( st_toggle_group[ibtn].button, TRUE );
		}
	}
}

static void
on_tree_view_count_changed( BaseWindow *window, gboolean reset, gint menus_count, gint actions_count, gint profiles_count, gpointer user_data )
{
	static const gchar *thisfn = "cact_sort_buttons_on_tree_view_count_changed";
	CactSortButtons *sort_buttons;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	sort_buttons = CACT_SORT_BUTTONS( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_SORT_BUTTONS ));

	if( !sort_buttons->private->dispose_has_run ){
		g_debug( "%s: window=%p, reset=%s, nb_menus=%d, nb_actions=%d, nb_profiles=%d, user_data=%p",
				thisfn, ( void * ) window, reset ? "True":"False",
						menus_count, actions_count, profiles_count, ( void * ) user_data );

		if( reset ){
			sort_buttons->private->count_items = menus_count + actions_count;
		} else {
			sort_buttons->private->count_items += menus_count + actions_count;
		}

		enable_buttons( sort_buttons, sort_buttons->private->count_items > 0 );
	}

}

static void
enable_buttons( const CactSortButtons *sort_buttons, gboolean enabled )
{
	gboolean level_zero_writable;
	gboolean preferences_locked;
	gboolean finally_enabled;
	gint i;
	guint order_mode;

	level_zero_writable = na_updater_is_level_zero_writable( sort_buttons->private->updater );
	preferences_locked = na_updater_are_preferences_locked( sort_buttons->private->updater );
	finally_enabled = level_zero_writable && !preferences_locked && enabled;

	for( i=0 ; st_toggle_group[i].btn_name ; ++i ){
		gtk_widget_set_sensitive( GTK_WIDGET( st_toggle_group[i].button ), finally_enabled );
	}

	if( finally_enabled && sort_buttons->private->active == -1 ){
		order_mode = na_iprefs_get_order_mode( NULL );
		i = toggle_group_get_from_mode( order_mode );
		gtk_toggle_button_set_active( st_toggle_group[i].button, TRUE );
	}
}

/*
 * returns the index of the button for the given order mode
 * or -1 if not found
 */
static gint
toggle_group_get_from_mode( guint mode )
{
	guint i;

	for( i = 0 ; st_toggle_group[i].btn_name ; ++ i ){
		if( st_toggle_group[i].order_mode == mode ){
			return( i );
		}
	}

	return( -1 );
}

/*
 * returns the index of the toggle button, or -1
 */
static gint
toggle_group_get_from_button( GtkToggleButton *toggled_button )
{
	gint i;

	for( i = 0 ; st_toggle_group[i].btn_name ; ++i ){
		if( st_toggle_group[i].button == toggled_button ){
			return( i );
		}
	}

	return( -1 );
}
