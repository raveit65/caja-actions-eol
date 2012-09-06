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

#include <core/na-iprefs.h>

#include "cact-application.h"
#include "cact-iprefs.h"
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
		{ MAIN_TOOLBAR_FILE_ID , "main-file-toolbar" ,  TRUE, "ViewFileToolbarItem" , "/ui/FileToolbar" },
		{ MAIN_TOOLBAR_EDIT_ID , "main-edit-toolbar" , FALSE, "ViewEditToolbarItem" , "/ui/EditToolbar" },
		{ MAIN_TOOLBAR_TOOLS_ID, "main-tools-toolbar", FALSE, "ViewToolsToolbarItem", "/ui/ToolsToolbar" },
		{ MAIN_TOOLBAR_HELP_ID , "main-help-toolbar" ,  TRUE, "ViewHelpToolbarItem" , "/ui/HelpToolbar" }
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

static void          init_toolbar( CactMainWindow *window, GtkActionGroup *group, int toolbar_id );
static void          reorder_toolbars( GtkHBox *hbox, int toolbar_id, GtkWidget *handle );
static void          on_handle_finalize( gpointer data, GObject *handle );
static void          on_attach_toolbar( GtkHandleBox *handle, GtkToolbar *toolbar, CactMainWindow *window );
static void          on_detach_toolbar( GtkHandleBox *handle, GtkToolbar *toolbar, CactMainWindow *window );
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
cact_main_toolbar_init( CactMainWindow *window, GtkActionGroup *group )
{
	static const gchar *thisfn = "cact_main_toolbar_init";
	int i;

	g_debug( "%s: window=%p, group=%p", thisfn, ( void * ) window, ( void * ) group );

	for( i = 0 ; i < G_N_ELEMENTS( toolbar_pos ) ; ++i ){
		init_toolbar( window, group, toolbar_pos[i] );
	}
}

static void
init_toolbar( CactMainWindow *window, GtkActionGroup *group, int toolbar_id )
{
	CactApplication *application;
	NAUpdater *updater;
	ToolbarProps *props;
	gboolean is_active;
	GtkToggleAction *action;

	application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = cact_application_get_updater( application );
	props = get_toolbar_properties( toolbar_id );
	if( props ){
		is_active = na_iprefs_read_bool( NA_IPREFS( updater ), props->prefs_key, props->displayed_per_default );
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
 */
void
cact_main_toolbar_activate( CactMainWindow *window, int toolbar_id, GtkUIManager *ui_manager, gboolean is_active )
{
	static const gchar *thisfn = "cact_main_toolbar_activate";
	ToolbarProps *props;
	GtkWidget *toolbar, *hbox, *handle;
	gulong attach_id, detach_id;

	props = get_toolbar_properties( toolbar_id );
	if( !props ){
		return;
	}

	toolbar = gtk_ui_manager_get_widget( ui_manager, props->ui_path );
	g_debug( "%s: toolbar=%p, path=%s, ref=%d", thisfn, ( void * ) toolbar, props->ui_path, G_OBJECT( toolbar )->ref_count );
	hbox = base_window_get_widget( BASE_WINDOW( window ), "ToolbarHBox" );

	if( is_active ){
		handle = gtk_handle_box_new();
		gtk_handle_box_set_snap_edge( GTK_HANDLE_BOX( handle ), GTK_POS_LEFT );
		g_object_set_data( G_OBJECT( toolbar ), "cact-main-toolbar-handle", handle );
		attach_id = g_signal_connect( handle, "child-attached", (GCallback ) on_attach_toolbar, window );
		g_object_set_data( G_OBJECT( handle ), "cact-handle-attach-id", ( gpointer ) attach_id );
		detach_id = g_signal_connect( handle, "child-detached", (GCallback ) on_detach_toolbar, window );
		g_object_set_data( G_OBJECT( handle ), "cact-handle-detach-id", ( gpointer ) detach_id );
		g_object_weak_ref( G_OBJECT( handle ), ( GWeakNotify ) on_handle_finalize, NULL );
		gtk_container_add( GTK_CONTAINER( handle ), toolbar );
		gtk_container_add( GTK_CONTAINER( hbox ), handle );
		reorder_toolbars( GTK_HBOX( hbox ), toolbar_id, handle );
		gtk_widget_show_all( handle );
		g_debug( "%s: ref=%d", thisfn, G_OBJECT( toolbar )->ref_count );

	} else {
		handle = ( GtkWidget * ) g_object_get_data( G_OBJECT( toolbar ), "cact-main-toolbar-handle" );
		detach_id = ( gulong ) g_object_get_data( G_OBJECT( handle ), "cact-handle-detach-id" );
		g_signal_handler_disconnect( handle, detach_id );
		attach_id = ( gulong ) g_object_get_data( G_OBJECT( handle ), "cact-handle-attach-id" );
		g_signal_handler_disconnect( handle, attach_id );
		gtk_container_remove( GTK_CONTAINER( handle ), toolbar );
		gtk_container_remove( GTK_CONTAINER( hbox ), handle );
		g_debug( "%s: ref=%d", thisfn, G_OBJECT( toolbar )->ref_count );
	}

	cact_iprefs_write_bool( BASE_WINDOW( window ), props->prefs_key, is_active );
}

/*
 * reposition the newly activated toolbar in handle
 * so that the relative positions of toolbars are respected in hbox
 */
static void
reorder_toolbars( GtkHBox *hbox, int toolbar_id, GtkWidget *handle )
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

static void
on_handle_finalize( gpointer data, GObject *handle )
{
	g_debug( "cact_main_toolbar_on_handle_finalize: handle=%p", ( void * ) handle );
}

static void
on_attach_toolbar( GtkHandleBox *handle, GtkToolbar *toolbar, CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_toolbar_on_attach_toolbar";

	g_debug( "%s: handle=%p, toolbar=%p, window=%p", thisfn, ( void * ) handle, ( void * ) toolbar, ( void * ) window );

	gtk_toolbar_set_show_arrow( toolbar, TRUE );
}

static void
on_detach_toolbar( GtkHandleBox *handle, GtkToolbar *toolbar, CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_toolbar_on_detach_toolbar";

	g_debug( "%s: handle=%p, toolbar=%p, window=%p", thisfn, ( void * ) handle, ( void * ) toolbar, ( void * ) window );

	gtk_toolbar_set_show_arrow( toolbar, FALSE );
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
