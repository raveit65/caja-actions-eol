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

#include <core/na-gtk-utils.h>
#include <core/na-updater.h>

#include "base-gtk-utils.h"

#define DEFAULT_WIDTH		22

typedef struct {
	GtkWidget *table;
	guint      rows;
	guint      ir;
	guint      columns;
	guint      ic;
	GtkWidget *grid;
}
	TableToGridData;

#if GTK_CHECK_VERSION( 3,0,0 )
static void table_to_grid_foreach_cb( GtkWidget *widget, TableToGridData *ttg );
#endif

/**
 * base_gtk_utils_position_window:
 * @window: this #BaseWindow-derived window.
 * @wsp_name: the string which handles the window size and position in user preferences.
 *
 * Position the specified window on the screen.
 *
 * A window position is stored as a list of integers "x,y,width,height".
 */
void
base_gtk_utils_restore_window_position( const BaseWindow *window, const gchar *wsp_name )
{
	GtkWindow *toplevel;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( wsp_name && strlen( wsp_name ));

	toplevel = base_window_get_gtk_toplevel( window );
	g_return_if_fail( GTK_IS_WINDOW( toplevel ));

	na_gtk_utils_restore_window_position( toplevel, wsp_name );
}

/**
 * base_gtk_utils_save_window_position:
 * @window: this #BaseWindow-derived window.
 * @wsp_name: the string which handles the window size and position in user preferences.
 *
 * Save the size and position of the specified window.
 */
void
base_gtk_utils_save_window_position( const BaseWindow *window, const gchar *wsp_name )
{
	GtkWindow *toplevel;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( wsp_name && strlen( wsp_name ));

	toplevel = base_window_get_gtk_toplevel( window );
	g_return_if_fail( GTK_IS_WINDOW( toplevel ));

	na_gtk_utils_save_window_position( toplevel, wsp_name );
}

/**
 * base_gtk_utils_set_editable:
 * @widget: the #GtkWdiget.
 * @editable: whether the @widget is editable or not.
 *
 * Try to set a visual indication of whether the @widget is editable or not.
 *
 * Having a GtkWidget should be enough, but we also deal with a GtkTreeViewColumn.
 * So the most-bottom common ancestor is just GObject (since GtkObject having been
 * deprecated in Gtk+-3.0)
 */
void
base_gtk_utils_set_editable( GObject *widget, gboolean editable )
{
	na_gtk_utils_set_editable( widget, editable );
}

/**
 * base_gtk_utils_radio_set_initial_state:
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
base_gtk_utils_radio_set_initial_state( GtkRadioButton *button,
		GCallback handler, void *user_data, gboolean editable, gboolean sensitive )
{
	na_gtk_utils_radio_set_initial_state( button, handler, user_data, editable, sensitive );
}

/**
 * base_gtk_utils_radio_reset_initial_state:
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
base_gtk_utils_radio_reset_initial_state( GtkRadioButton *button, GCallback handler )
{
	na_gtk_utils_radio_reset_initial_state( button, handler );
}

/**
 * base_gtk_utils_toggle_set_initial_state:
 * @button: the #GtkToggleButton button.
 * @handler: the corresponding "toggled" handler.
 * @window: the toplevel #BaseWindow which embeds the button;
 *  it will be passed as user_data when connecting the signal.
 * @active: whether the check button is initially active (checked).
 * @editable: whether this radio button group is editable.
 * @sensitive: whether this radio button group is sensitive.
 *
 * This function should be called for a check button which may happen to be
 * read-only..
 *
 * It does the following operations:
 * - connect the 'toggled' handler to the button
 * - set the button as active or icactive depending of @active
 * - set the button as editable or not depending of @editable
 * - set the button as sensitive or not depending of @sensitive
 * - explictely triggers the 'toggled' handler
 */
void
base_gtk_utils_toggle_set_initial_state( BaseWindow *window,
		const gchar *button_name, GCallback handler,
		gboolean active, gboolean editable, gboolean sensitive )
{
	typedef void ( *toggle_handler )( GtkToggleButton *, BaseWindow * );
	GtkToggleButton *button;

	button = GTK_TOGGLE_BUTTON( base_window_get_widget( window, button_name ));

	if( button ){
		base_window_signal_connect( window, G_OBJECT( button ), "toggled", handler );

		g_object_set_data( G_OBJECT( button ), NA_TOGGLE_DATA_HANDLER, handler );
		g_object_set_data( G_OBJECT( button ), NA_TOGGLE_DATA_USER_DATA, window );
		g_object_set_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE, GUINT_TO_POINTER( editable ));

		base_gtk_utils_set_editable( G_OBJECT( button ), editable );
		gtk_widget_set_sensitive( GTK_WIDGET( button ), sensitive );
		gtk_toggle_button_set_active( button, active );

		( *( toggle_handler ) handler )( button, window );
	}
}

/**
 * base_gtk_utils_toggle_reset_initial_state:
 * @button: the #GtkToggleButton check button.
 *
 * When clicking on a read-only check button, this function ensures that
 * the check button is not modified. It may be called whether the button
 * is editable or not (does nothing if button is actually editable).
 */
void
base_gtk_utils_toggle_reset_initial_state( GtkToggleButton *button )
{
	gboolean editable;
	GCallback handler;
	gpointer user_data;
	gboolean active;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( !editable ){
		active = gtk_toggle_button_get_active( button );
		handler = G_CALLBACK( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_HANDLER ));
		user_data = g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_USER_DATA );

		g_signal_handlers_block_by_func(( gpointer ) button, handler, user_data );
		gtk_toggle_button_set_active( button, !active );
		g_signal_handlers_unblock_by_func(( gpointer ) button, handler, user_data );
	}
}

/**
 * base_gtk_utils_get_pixbuf:
 * @name: the name of the file or an icon.
 * @widget: the widget on which the image should be rendered.
 * @size: the desired size.
 *
 * Returns a pixbuf for the given widget.
 */
GdkPixbuf *
base_gtk_utils_get_pixbuf( const gchar *name, GtkWidget *widget, GtkIconSize size )
{
	static const gchar *thisfn = "base_gtk_utils_get_pixbuf";
	GdkPixbuf* pixbuf;
	GError *error;
	gint width, height;
	GtkIconTheme *icon_theme;

	error = NULL;
	pixbuf = NULL;

	if( !gtk_icon_size_lookup( size, &width, &height )){
		width = DEFAULT_WIDTH;
		height = DEFAULT_HEIGHT;
	}

	if( name && strlen( name )){
		if( g_path_is_absolute( name )){
			pixbuf = gdk_pixbuf_new_from_file_at_size( name, width, height, &error );
			if( error ){
				if( error->code != G_FILE_ERROR_NOENT ){
					g_warning( "%s: gdk_pixbuf_new_from_file_at_size: name=%s, error=%s (%d)", thisfn, name, error->message, error->code );
				}
				g_error_free( error );
				error = NULL;
				pixbuf = NULL;
			}

		} else {
/* gtk_widget_render_icon() is deprecated since Gtk+ 3.0
 * see http://library.gnome.org/devel/gtk/unstable/GtkWidget.html#gtk-widget-render-icon
 * and http://git.gnome.org/browse/gtk+/commit/?id=07eeae15825403037b7df139acf9bfa104d5559d
 */
#if GTK_CHECK_VERSION( 2, 91, 7 )
			pixbuf = gtk_widget_render_icon_pixbuf( widget, name, size );
#else
			pixbuf = gtk_widget_render_icon( widget, name, size, NULL );
#endif
			if( !pixbuf ){
				icon_theme = gtk_icon_theme_get_default();
				pixbuf = gtk_icon_theme_load_icon(
								icon_theme, name, width, GTK_ICON_LOOKUP_GENERIC_FALLBACK, &error );
				if( error ){
					/* it happens that the message "Icon 'xxxx' not present in theme"
					 * is generated with a domain of 'gtk-icon-theme-error-quark' and
					 * an error code of zero - it seems difficult to just test zero
					 * so does not display warning, but just debug
					 */
					g_debug( "%s: %s (%s:%d)",
							thisfn, error->message, g_quark_to_string( error->domain ), error->code );
					g_error_free( error );
				}
			}
		}
	}

	if( !pixbuf ){
		g_debug( "%s: null pixbuf, loading transparent image", thisfn );
		pixbuf = gdk_pixbuf_new_from_file_at_size( PKGUIDIR "/transparent.png", width, height, NULL );
	}

	return( pixbuf );
}

/**
 * base_gtk_utils_render:
 * @name: the name of the file or an icon, or %NULL.
 * @widget: the widget on which the image should be rendered.
 * @size: the desired size.
 *
 * Displays the (maybe themed) image on the given widget.
 */
void
base_gtk_utils_render( const gchar *name, GtkImage *widget, GtkIconSize size )
{
	static const gchar *thisfn = "base_gtk_utils_render";
	GdkPixbuf* pixbuf;
	gint width, height;

	g_debug( "%s: name=%s, widget=%p, size=%d", thisfn, name, ( void * ) widget, size );

	if( name ){
		pixbuf = base_gtk_utils_get_pixbuf( name, GTK_WIDGET( widget ), size );

	} else {
		if( !gtk_icon_size_lookup( size, &width, &height )){
			width = DEFAULT_WIDTH;
			height = DEFAULT_HEIGHT;
		}
		pixbuf = gdk_pixbuf_new_from_file_at_size( PKGUIDIR "/transparent.png", width, height, NULL );
	}

	if( pixbuf ){
		gtk_image_set_from_pixbuf( widget, pixbuf );
		g_object_unref( pixbuf );
	}
}

/**
 * base_gtk_utils_select_file:
 * @window: the #BaseWindow which will be the parent of the dialog box.
 * @title: the title of the dialog box.
 * @wsp_name: the name of the dialog box in Preferences to read/write
 *  its size and position.
 * @entry: the #GtkEntry which is associated with the selected file.
 * @entry_name: the name of the entry in Preferences to be read/written.
 *
 * Opens a #GtkFileChooserDialog and let the user choose an existing file
 * -> choose and display an existing file name
 * -> record the dirname URI.
 *
 * If the user validates its selection, the chosen file pathname will be
 * written in the @entry #GtkEntry, while the corresponding dirname
 * URI will be written as @entry_name in Preferences.
 */
void
base_gtk_utils_select_file( BaseWindow *window,
				const gchar *title, const gchar *wsp_name,
				GtkWidget *entry, const gchar *entry_name )
{
	base_gtk_utils_select_file_with_preview(
			window, title, wsp_name, entry, entry_name, NULL );
}

/**
 * base_gtk_utils_select_file_with_preview:
 * @window: the #BaseWindow which will be the parent of the dialog box.
 * @title: the title of the dialog box.
 * @wsp_name: the name of the dialog box in Preferences to read/write
 *  its size and position.
 * @entry: the #GtkEntry which is associated with the selected file.
 * @entry_name: the name of the entry in Preferences to be read/written.
 * @update_preview_cb: the callback function in charge of updating the
 *  preview widget. May be NULL.
 *
 * Opens a #GtkFileChooserDialog and let the user choose an existing file
 * -> choose and display an existing file name
 * -> record the dirname URI.
 *
 * If the user validates its selection, the chosen file pathname will be
 * written in the @entry #GtkEntry, while the corresponding dirname
 * URI will be written as @entry_name in Preferences.
 */
void
base_gtk_utils_select_file_with_preview( BaseWindow *window,
				const gchar *title, const gchar *wsp_name,
				GtkWidget *entry, const gchar *entry_name,
				GCallback update_preview_cb )
{
	GtkWindow *toplevel;
	GtkWidget *dialog;
	const gchar *text;
	gchar *filename, *uri;
	GtkWidget *preview;

	toplevel = base_window_get_gtk_toplevel( window );

	dialog = gtk_file_chooser_dialog_new(
			title,
			toplevel,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL
			);

	if( update_preview_cb ){
		preview = gtk_image_new();
		gtk_file_chooser_set_preview_widget( GTK_FILE_CHOOSER( dialog ), preview );
		g_signal_connect( dialog, "update-preview", update_preview_cb, preview );
	}

	base_gtk_utils_restore_window_position( window, wsp_name );

	text = gtk_entry_get_text( GTK_ENTRY( entry ));

	if( text && strlen( text )){
		gtk_file_chooser_set_filename( GTK_FILE_CHOOSER( dialog ), text );

	} else {
		uri = na_settings_get_string( entry_name, NULL, NULL );
		if( uri ){
			gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( dialog ), uri );
			g_free( uri );
		}
	}

	if( gtk_dialog_run( GTK_DIALOG( dialog )) == GTK_RESPONSE_ACCEPT ){
		filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ));
		gtk_entry_set_text( GTK_ENTRY( entry ), filename );
	    g_free( filename );
	  }

	uri = gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( dialog ));
	na_settings_set_string( entry_name, uri );
	g_free( uri );

	base_gtk_utils_save_window_position( window, wsp_name );

	gtk_widget_destroy( dialog );
}

/**
 * base_gtk_utils_select_dir:
 * @window: the #BaseWindow which will be the parent of the dialog box.
 * @title: the title of the dialog box.
 * @wsp_name: the name of the dialog box in Preferences to read/write
 *  its size and position.
 * @entry: the #GtkEntry which is associated with the field.
 * @entry_name: the name of the entry in Preferences to be read/written.
 * @default_dir_uri: the URI of the directory which should be set in there is
 *  not yet any preference (see @entry_name)
 *
 * Opens a #GtkFileChooserDialog and let the user choose an existing directory
 * -> choose and display an existing dir name
 * -> record the dirname URI of this dir name.
 *
 * If the user validates its selection, the chosen file pathname will be
 * written in the @entry #GtkEntry, while the corresponding dirname
 * URI will be written as @entry_name in Preferences.
 */
void
base_gtk_utils_select_dir( BaseWindow *window,
				const gchar *title, const gchar *wsp_name,
				GtkWidget *entry, const gchar *entry_name )
{
	GtkWindow *toplevel;
	GtkWidget *dialog;
	const gchar *text;
	gchar *dir, *uri;

	toplevel = base_window_get_gtk_toplevel( window );

	dialog = gtk_file_chooser_dialog_new(
			title,
			toplevel,
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL
			);

	base_gtk_utils_restore_window_position( window, wsp_name );

	text = gtk_entry_get_text( GTK_ENTRY( entry ));

	if( text && strlen( text )){
		gtk_file_chooser_set_filename( GTK_FILE_CHOOSER( dialog ), text );

	} else {
		uri = na_settings_get_string( entry_name, NULL, NULL );
		if( uri ){
			gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( dialog ), uri );
			g_free( uri );
		}
	}

	if( gtk_dialog_run( GTK_DIALOG( dialog )) == GTK_RESPONSE_ACCEPT ){
		dir = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ));
		gtk_entry_set_text( GTK_ENTRY( entry ), dir );
	    g_free( dir );
	  }

	uri = gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( dialog ));
	na_settings_set_string( entry_name, uri );
	g_free( uri );

	base_gtk_utils_save_window_position( window, wsp_name );

	gtk_widget_destroy( dialog );
}

/*
 * base_gtk_utils_table_to_grid:
 * @window: the #BaseWindow container.
 * @table_name: the name of the #GtkTable to be replaced.
 *
 * Dynamically replaces a GtkTable with a GtkGrid, doing its best in order
 * to preserve order and name of all children.
 *
 * The caller has to take care of calling this function for Gtk 3.x, only
 * replacing valuable GtkTables.
 *
 * This function should be called from on_base_initialize_gtk().
 */
void
base_gtk_utils_table_to_grid( BaseWindow *window, const gchar *table_name )
{
#if GTK_CHECK_VERSION( 3,0,0 )
	static const gchar *thisfn = "base_gtk_utils_table_to_grid";
	TableToGridData ttg;
	GtkWidget *parent;
	guint col_spacing, row_spacing;

	memset( &ttg, '\0', sizeof( TableToGridData ));

	ttg.table = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( base_window_get_gtk_toplevel( window )), table_name );
	g_return_if_fail( ttg.table );
	g_return_if_fail( GTK_IS_TABLE( ttg.table ));
	g_debug( "%s: table=%p (%s)", thisfn, ( void * ) ttg.table, gtk_buildable_get_name( GTK_BUILDABLE( ttg.table )));

	parent = gtk_widget_get_parent( ttg.table );

#ifdef NA_MAINTAINER_MODE
	na_gtk_utils_dump_children( GTK_CONTAINER( parent ));
#endif

#if !GTK_CHECK_VERSION( 3,4,0 )
	gtk_table_get_size( GTK_TABLE( ttg.table ), &ttg.rows, &ttg.columns );
	col_spacing = gtk_table_get_default_col_spacing( GTK_TABLE( ttg.table ));
	row_spacing = gtk_table_get_default_row_spacing( GTK_TABLE( ttg.table ));
#else
	col_spacing = 6;
	row_spacing = 6;
#endif

	ttg.grid = gtk_grid_new();
	gtk_grid_set_column_spacing( GTK_GRID( ttg.grid ), col_spacing );
	gtk_grid_set_row_spacing( GTK_GRID( ttg.grid ), row_spacing );

	gtk_container_foreach( GTK_CONTAINER( ttg.table ), ( GtkCallback ) table_to_grid_foreach_cb, &ttg );
	/*gtk_widget_unparent( ttg.table );*/

	if( GTK_IS_ALIGNMENT( parent )){
		gtk_container_remove( GTK_CONTAINER( parent ), ttg.table );
		gtk_container_add( GTK_CONTAINER( parent ), ttg.grid );
	} else {
		g_warning( "%s: untreated parent of class %s", thisfn, G_OBJECT_TYPE_NAME( parent ));
	}

#ifdef NA_MAINTAINER_MODE
	na_gtk_utils_dump_children( GTK_CONTAINER( parent ));
#endif
#endif
}

#if GTK_CHECK_VERSION( 3,0,0 )
static void
table_to_grid_foreach_cb( GtkWidget *widget, TableToGridData *ttg )
{
	static const gchar *thisfn = "base_gtk_utils_table_to_grid_foreach_cb";
	guint left, top, x_options;

	g_debug( "%s: widget=%p (%s)", thisfn, ( void * ) widget, gtk_buildable_get_name( GTK_BUILDABLE( widget )));

	gtk_container_child_get( GTK_CONTAINER( ttg->table ), widget,
			"left-attach", &left, "top-attach", &top, "x-options", &x_options, NULL );
	gtk_widget_unparent( widget );
	gtk_grid_attach( GTK_GRID( ttg->grid ), widget, left, top, 1, 1 );
	gtk_widget_set_hexpand( widget, x_options & GTK_EXPAND );
}
#endif
