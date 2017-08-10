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

#include <glib.h>
#include <string.h>

#include "na-gtk-utils.h"
#include "na-settings.h"

static void   int_list_to_position( GList *list, gint *x, gint *y, gint *width, gint *height );
static GList *position_to_int_list( gint x, gint y, gint width, gint height );
static void   free_int_list( GList *list );

/*
 * na_gtk_utils_find_widget_by_name:
 * @container: a #GtkContainer, usually the #GtkWindow toplevel.
 * @name: the name of the searched widget.
 *
 * Returns: the searched widget.
 */
GtkWidget *
na_gtk_utils_find_widget_by_name( GtkContainer *container, const gchar *name )
{
	GList *children = gtk_container_get_children( container );
	GList *ic;
	GtkWidget *found = NULL;
	GtkWidget *child;
	const gchar *child_name;

	for( ic = children ; ic && !found ; ic = ic->next ){

		if( GTK_IS_WIDGET( ic->data )){
			child = GTK_WIDGET( ic->data );
			child_name = gtk_buildable_get_name( GTK_BUILDABLE( child ));
			if( child_name && strlen( child_name ) && !g_ascii_strcasecmp( name, child_name )){
				found = child;
				break;
			}
			if( GTK_IS_CONTAINER( child )){
				found = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( child ), name );
			}
		}
	}

	g_list_free( children );
	return( found );
}

#ifdef NA_MAINTAINER_MODE
static void
dump_children( const gchar *thisfn, GtkContainer *container, int level )
{
	GList *children = gtk_container_get_children( container );
	GList *ic;
	GtkWidget *child;
	const gchar *child_name;
	GString *prefix;
	int i;

	prefix = g_string_new( "" );
	for( i = 0 ; i <= level ; ++i ){
		g_string_append_printf( prefix, "%s", "|  " );
	}

	for( ic = children ; ic ; ic = ic->next ){

		if( GTK_IS_WIDGET( ic->data )){
			child = GTK_WIDGET( ic->data );
			child_name = gtk_buildable_get_name( GTK_BUILDABLE( child ));
			g_debug( "%s: %s%s\t%p %s",
					thisfn, prefix->str, G_OBJECT_TYPE_NAME( child ), ( void * ) child, child_name );

			if( GTK_IS_CONTAINER( child )){
				dump_children( thisfn, GTK_CONTAINER( child ), level+1 );
			}
		}
	}

	g_list_free( children );
	g_string_free( prefix, TRUE );
}

void
na_gtk_utils_dump_children( GtkContainer *container )
{
	static const gchar *thisfn = "na_gtk_utils_dump_children";

	g_debug( "%s: container=%p", thisfn, container );

	dump_children( thisfn, container, 0 );
}
#endif

/**
 * na_gtk_utils_restore_position_window:
 * @toplevel: the #GtkWindow window.
 * @wsp_name: the string which handles the window size and position in user preferences.
 *
 * Position the specified window on the screen.
 *
 * A window position is stored as a list of integers "x,y,width,height".
 */
void
na_gtk_utils_restore_window_position( GtkWindow *toplevel, const gchar *wsp_name )
{
	static const gchar *thisfn = "na_gtk_utils_restore_window_position";
	GList *list;
	gint x=0, y=0, width=0, height=0;
	GdkDisplay *display;
	GdkScreen *screen;
	gint screen_width, screen_height;

	g_return_if_fail( GTK_IS_WINDOW( toplevel ));
	g_return_if_fail( wsp_name && strlen( wsp_name ));

	g_debug( "%s: toplevel=%p (%s), wsp_name=%s",
			thisfn, ( void * ) toplevel, G_OBJECT_TYPE_NAME( toplevel ), wsp_name );

	list = na_settings_get_uint_list( wsp_name, NULL, NULL );

	if( list ){
		int_list_to_position( list, &x, &y, &width, &height );
		g_debug( "%s: wsp_name=%s, x=%d, y=%d, width=%d, height=%d", thisfn, wsp_name, x, y, width, height );
		free_int_list( list );
	}

	x = MAX( 1, x );
	y = MAX( 1, y );
	width = MAX( 1, width );
	height = MAX( 1, height );

	/* bad hack for the first time we open the main window
	 * try to target an ideal size and position
	 */
	if( !strcmp( wsp_name, NA_IPREFS_MAIN_WINDOW_WSP )){
		if( x == 1 && y == 1 && width == 1 && height == 1 ){
			x = 50;
			y = 70;
			width = 1030;
			height = 560;

		} else {
			display = gdk_display_get_default();
			screen = gdk_display_get_default_screen( display );

			gdk_window_get_geometry (gdk_screen_get_root_window( screen ), NULL, NULL,
						 &screen_width, &screen_height);

			g_debug( "%s: screen=(%d,%d), DEFAULT_HEIGHT=%d",
					thisfn, screen_width, screen_height, DEFAULT_HEIGHT );

			screen_height -= 2*DEFAULT_HEIGHT;
			width = MIN( width, screen_width-x );
			height = MIN( height, screen_height-y );
		}
	}

	g_debug( "%s: wsp_name=%s, x=%d, y=%d, width=%d, height=%d",
			thisfn, wsp_name, x, y, width, height );

	gtk_window_move( toplevel, x, y );
	gtk_window_resize( toplevel, width, height );
}

/**
 * na_gtk_utils_save_window_position:
 * @toplevel: the #GtkWindow window.
 * @wsp_name: the string which handles the window size and position in user preferences.
 *
 * Save the size and position of the specified window.
 */
void
na_gtk_utils_save_window_position( GtkWindow *toplevel, const gchar *wsp_name )
{
	static const gchar *thisfn = "na_gtk_utils_save_window_position";
	gint x, y, width, height;
	GList *list;

	g_return_if_fail( GTK_IS_WINDOW( toplevel ));
	g_return_if_fail( wsp_name && strlen( wsp_name ));

	gtk_window_get_position( toplevel, &x, &y );
	gtk_window_get_size( toplevel, &width, &height );
	g_debug( "%s: wsp_name=%s, x=%d, y=%d, width=%d, height=%d", thisfn, wsp_name, x, y, width, height );

	list = position_to_int_list( x, y, width, height );
	na_settings_set_uint_list( wsp_name, list );
	free_int_list( list );
}

/*
 * extract the position of the window from the list of unsigned integers
 */
static void
int_list_to_position( GList *list, gint *x, gint *y, gint *width, gint *height )
{
	GList *it;
	int i;

	g_assert( x );
	g_assert( y );
	g_assert( width );
	g_assert( height );

	for( it=list, i=0 ; it ; it=it->next, i+=1 ){
		switch( i ){
			case 0:
				*x = GPOINTER_TO_UINT( it->data );
				break;
			case 1:
				*y = GPOINTER_TO_UINT( it->data );
				break;
			case 2:
				*width = GPOINTER_TO_UINT( it->data );
				break;
			case 3:
				*height = GPOINTER_TO_UINT( it->data );
				break;
		}
	}
}

static GList *
position_to_int_list( gint x, gint y, gint width, gint height )
{
	GList *list = NULL;

	list = g_list_append( list, GUINT_TO_POINTER( x ));
	list = g_list_append( list, GUINT_TO_POINTER( y ));
	list = g_list_append( list, GUINT_TO_POINTER( width ));
	list = g_list_append( list, GUINT_TO_POINTER( height ));

	return( list );
}

/*
 * free the list of int
 */
static void
free_int_list( GList *list )
{
	g_list_free( list );
}

/**
 * na_gtk_utils_set_editable:
 * @widget: the #GtkWdiget.
 * @editable: whether the @widget is editable or not.
 *
 * Try to set a visual indication of whether the @widget is editable or not.
 *
 * Having a GtkWidget should be enough, but we also deal with a GtkTreeViewColumn.
 * So the most-bottom common ancestor is just GObject (since GtkObject having been
 * deprecated in Gtk+-3.0)
 *
 * Note that using 'sensitivity' property is just a work-around because the
 * two things have distinct semantics:
 * - editable: whether we are allowed to modify the value (is not read-only)
 * - sensitive: whether the value is relevant (has a sense in this context)
 */
void
na_gtk_utils_set_editable( GObject *widget, gboolean editable )
{
	GList *renderers, *irender;

/* GtkComboBoxEntry is deprecated from Gtk+3
 * see. http://git.gnome.org/browse/gtk+/commit/?id=9612c648176378bf237ad0e1a8c6c995b0ca7c61
 * while 'has_entry' property exists since 2.24
 */
	if( GTK_IS_COMBO_BOX( widget ) && gtk_combo_box_get_has_entry( GTK_COMBO_BOX( widget ))){
		/* idem as GtkEntry */
		gtk_editable_set_editable( GTK_EDITABLE( gtk_bin_get_child( GTK_BIN( widget ))), editable );
		g_object_set( G_OBJECT( gtk_bin_get_child( GTK_BIN( widget ))), "can-focus", editable, NULL );
		/* disable the listbox button itself */
		gtk_combo_box_set_button_sensitivity( GTK_COMBO_BOX( widget ), editable ? GTK_SENSITIVITY_ON : GTK_SENSITIVITY_OFF );

	} else if( GTK_IS_COMBO_BOX( widget )){
		/* disable the listbox button itself */
		gtk_combo_box_set_button_sensitivity( GTK_COMBO_BOX( widget ), editable ? GTK_SENSITIVITY_ON : GTK_SENSITIVITY_OFF );

	} else if( GTK_IS_ENTRY( widget )){
		gtk_editable_set_editable( GTK_EDITABLE( widget ), editable );
		/* removing the frame leads to a disturbing modification of the
		 * height of the control */
		/*g_object_set( G_OBJECT( widget ), "has-frame", editable, NULL );*/
		/* this prevents the caret to be displayed when we click in the entry */
		g_object_set( G_OBJECT( widget ), "can-focus", editable, NULL );

	} else if( GTK_IS_TEXT_VIEW( widget )){
		g_object_set( G_OBJECT( widget ), "can-focus", editable, NULL );
		gtk_text_view_set_editable( GTK_TEXT_VIEW( widget ), editable );

	} else if( GTK_IS_TOGGLE_BUTTON( widget )){
		/* transforms to a quasi standard GtkButton */
		/*g_object_set( G_OBJECT( widget ), "draw-indicator", editable, NULL );*/
		/* this at least prevent the keyboard focus to go to the button
		 * (which is better than nothing) */
		g_object_set( G_OBJECT( widget ), "can-focus", editable, NULL );

	} else if( GTK_IS_TREE_VIEW_COLUMN( widget )){
		renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( GTK_TREE_VIEW_COLUMN( widget )));
		for( irender = renderers ; irender ; irender = irender->next ){
			if( GTK_IS_CELL_RENDERER_TEXT( irender->data )){
				g_object_set( G_OBJECT( irender->data ), "editable", editable, "editable-set", TRUE, NULL );
			}
		}
		g_list_free( renderers );

	} else if( GTK_IS_BUTTON( widget )){
		gtk_widget_set_sensitive( GTK_WIDGET( widget ), editable );
	}
}

/**
 * na_gtk_utils_radio_set_initial_state:
 * @button: the #GtkRadioButton button which is initially active.
 * @handler: the corresponding "toggled" handler.
 * @user_data: the user data associated to the handler.
 * @editable: whether this radio button group is editable.
 * @sensitive: whether this radio button group is sensitive.
 *
 * This function should be called for the button which is initially active
 * inside of a radio button group when the radio group may happen to not be
 * editable.
 * This function should be called only once for the radio button group.
 *
 * It does the following operations:
 * - set the button as active
 * - set other buttons of the radio button group as icactive
 * - set all buttons of radio button group as @editable
 *
 * The initially active @button, along with its @handler, are recorded
 * as properties of the radio button group (actually as properties of each
 * radio button of the group), so that they can later be used to reset the
 * initial state.
 */
void
na_gtk_utils_radio_set_initial_state( GtkRadioButton *button,
		GCallback handler, void *user_data, gboolean editable, gboolean sensitive )
{
	GSList *group, *ig;
	GtkRadioButton *other;

	group = gtk_radio_button_get_group( button );

	for( ig = group ; ig ; ig = ig->next ){
		other = GTK_RADIO_BUTTON( ig->data );
		g_object_set_data( G_OBJECT( other ), NA_TOGGLE_DATA_BUTTON, button );
		g_object_set_data( G_OBJECT( other ), NA_TOGGLE_DATA_HANDLER, handler );
		g_object_set_data( G_OBJECT( other ), NA_TOGGLE_DATA_USER_DATA, user_data );
		g_object_set_data( G_OBJECT( other ), NA_TOGGLE_DATA_EDITABLE, GUINT_TO_POINTER( editable ));
		na_gtk_utils_set_editable( G_OBJECT( other ), editable );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( other ), FALSE );
		gtk_widget_set_sensitive( GTK_WIDGET( other ), sensitive );
	}

	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );
}

/**
 * na_gtk_utils_radio_reset_initial_state:
 * @button: the #GtkRadioButton being toggled.
 * @handler: the corresponding "toggled" handler.
 * @data: data associated with the @handler callback.
 *
 * When clicking on a read-only radio button, this function ensures that
 * the radio button is not modified. It may be called whether the radio
 * button group is editable or not (does nothing if group is actually
 * editable).
 */
void
na_gtk_utils_radio_reset_initial_state( GtkRadioButton *button, GCallback handler )
{
	GtkToggleButton *initial_button;
	GCallback initial_handler;
	gboolean active;
	gboolean editable;
	gpointer user_data;

	active = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( active && !editable ){
		initial_button = GTK_TOGGLE_BUTTON( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_BUTTON ));
		initial_handler = G_CALLBACK( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_HANDLER ));
		user_data = g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_USER_DATA );

		if( handler ){
			g_signal_handlers_block_by_func(( gpointer ) button, handler, user_data );
		}
		g_signal_handlers_block_by_func(( gpointer ) initial_button, initial_handler, user_data );

		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), FALSE );
		gtk_toggle_button_set_active( initial_button, TRUE );

		g_signal_handlers_unblock_by_func(( gpointer ) initial_button, initial_handler, user_data );
		if( handler ){
			g_signal_handlers_unblock_by_func(( gpointer ) button, handler, user_data );
		}
	}
}
