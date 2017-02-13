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

#include "cact-application.h"
#include "cact-main-toolbar.h"

typedef struct {
	int      id;
	gchar   *prefs_key;
	gboolean displayed_per_default;
	gchar   *ui_item;
	gchar   *ui_path;
}
	ToolbarProps;

static ToolbarProps toolbar_props[] = {
		{ MAIN_TOOLBAR_FILE_ID , NA_IPREFS_MAIN_TOOLBAR_FILE_DISPLAY,   TRUE, "ViewFileToolbarItem" , "/ui/FileToolbar" },
		{ MAIN_TOOLBAR_EDIT_ID , NA_IPREFS_MAIN_TOOLBAR_EDIT_DISPLAY,  FALSE, "ViewEditToolbarItem" , "/ui/EditToolbar" },
		{ MAIN_TOOLBAR_TOOLS_ID, NA_IPREFS_MAIN_TOOLBAR_TOOLS_DISPLAY, FALSE, "ViewToolsToolbarItem", "/ui/ToolsToolbar" },
		{ MAIN_TOOLBAR_HELP_ID , NA_IPREFS_MAIN_TOOLBAR_HELP_DISPLAY,   TRUE, "ViewHelpToolbarItem" , "/ui/HelpToolbar" }
};

/* defines the relative position of the main toolbars
 * that is: they are listed here in the order they should be displayed
 */
static int toolbar_pos[] = {
		MAIN_TOOLBAR_FILE_ID,
		MAIN_TOOLBAR_EDIT_ID,
		MAIN_TOOLBAR_TOOLS_ID,
		MAIN_TOOLBAR_HELP_ID
};

static void          init_toolbar( BaseWindow *window, GtkActionGroup *group, int toolbar_id );
static void          reorder_toolbars( GtkWidget *hbox, int toolbar_id, GtkWidget *handle );
static ToolbarProps *get_toolbar_properties( int toolbar_id );

/**
 * cact_main_toolbar_init:
 * @window: this #CactMainWindow window.
 *
 * Setup the initial display of the standard main toolbars.
 *
 * This actually only setup the initial state of the toggle options in
 * View > Toolbars menu; when an option is activated, this will trigger
 * the 'on_view_toolbar_activated()' which will actually display the
 * toolbar.
 */
void
cact_main_toolbar_init( BaseWindow *window, GtkActionGroup *group )
{
	static const gchar *thisfn = "cact_main_toolbar_init";
	int i;

	g_debug( "%s: window=%p, group=%p", thisfn, ( void * ) window, ( void * ) group );

	for( i = 0 ; i < G_N_ELEMENTS( toolbar_pos ) ; ++i ){
		init_toolbar( window, group, toolbar_pos[i] );
	}
}

static void
init_toolbar( BaseWindow *window, GtkActionGroup *group, int toolbar_id )
{
	ToolbarProps *props;
	gboolean is_active;
	GtkToggleAction *action;

	props = get_toolbar_properties( toolbar_id );
	if( props ){
		is_active = na_settings_get_boolean( props->prefs_key, NULL, NULL );
		if( is_active ){
			action = GTK_TOGGLE_ACTION( gtk_action_group_get_action( group, props->ui_item ));
			gtk_toggle_action_set_active( action, TRUE );
		}
	}
}

/**
 * cact_main_toolbar_activate:
 * @window: this #CactMainWindow.
 * @toolbar_id: the id of the activated toolbar.
 * @ui_manager: the #GtkUIManager.
 * @is_active: whether this toolbar is activated or not.
 *
 * Activate or desactivate the toolbar.
 *
 * pwi 2013-09-07
 * GtkHandleBox has been deprecated starting with Gtk 3.4 and there is
 * no replacement (see https://developer.gnome.org/gtk3/stable/GtkHandleBox.html).
 * So exit floating toolbars :(
 */
void
cact_main_toolbar_activate( CactMainWindow *window, int toolbar_id, GtkUIManager *ui_manager, gboolean is_active )
{
	static const gchar *thisfn = "cact_main_toolbar_activate";
	ToolbarProps *props;
	GtkWidget *toolbar, *hbox;

	props = get_toolbar_properties( toolbar_id );
	if( !props ){
		return;
	}

	toolbar = gtk_ui_manager_get_widget( ui_manager, props->ui_path );
	g_debug( "%s: toolbar=%p, path=%s, ref_count=%d", thisfn, ( void * ) toolbar, props->ui_path, G_OBJECT( toolbar )->ref_count );
	hbox = base_window_get_widget( BASE_WINDOW( window ), "ToolbarHBox" );

	if( is_active ){
		gtk_container_add( GTK_CONTAINER( hbox ), toolbar );
		reorder_toolbars( hbox, toolbar_id, toolbar );
		gtk_widget_show_all( toolbar );

	} else {
		gtk_container_remove( GTK_CONTAINER( hbox ), toolbar );
	}
	na_settings_set_boolean( props->prefs_key, is_active );
}

/*
 * @hbox: the GtkHBox container
 * @toolbar_id: toolbar identifier
 * @handle: hbox child, which used to be a GtkHandleBox (for moveable
 *  toolbars), and becomes the GtkToolbar itself starting with Gtk 3.4
 *
 * reposition the newly activated toolbar in handle
 * so that the relative positions of toolbars are respected in hbox
 */
static void
reorder_toolbars( GtkWidget *hbox, int toolbar_id, GtkWidget *handle )
{
	int this_canonic_rel_pos;
	int i;
	GList *children, *ic;
	int pos;
	int canonic_pos;

	this_canonic_rel_pos = 0;
	for( i = 0 ; i < G_N_ELEMENTS( toolbar_pos ); ++ i ){
		if( toolbar_pos[i] == toolbar_id ){
			this_canonic_rel_pos = i;
			break;
		}
	}
	g_object_set_data( G_OBJECT( handle ), "toolbar-canonic-pos", GINT_TO_POINTER( this_canonic_rel_pos ));

	pos = 0;
	children = gtk_container_get_children( GTK_CONTAINER( hbox ));
	for( ic = children ; ic ; ic = ic->next ){
		canonic_pos = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( ic->data ), "toolbar-canonic-pos" ));
		if( canonic_pos >= this_canonic_rel_pos ){
			break;
		}
		pos += 1;
	}

	gtk_box_reorder_child( GTK_BOX( hbox ), handle, pos );
}

static ToolbarProps *
get_toolbar_properties( int toolbar_id )
{
	static const gchar *thisfn = "cact_main_toolbar_get_toolbar_properties";
	ToolbarProps *props;
	int i;

	props = NULL;

	for( i = 0 ; i < G_N_ELEMENTS( toolbar_props ) && props == NULL ; ++i ){
		if( toolbar_props[i].id == toolbar_id ){
			props = &toolbar_props[i];
		}
	}

	if( !props ){
		g_warning( "%s: unable to find toolbar properties for id=%d", thisfn, toolbar_id );
	}

	return( props );
}
