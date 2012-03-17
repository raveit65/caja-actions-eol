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

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-iprefs.h>

#include "base-iprefs.h"
#include "base-window.h"
#include "nact-gtk-utils.h"
#include "nact-iprefs.h"
#include "nact-application.h"
#include "nact-main-tab.h"
#include "nact-ifolders-tab.h"

/* private interface data
 */
struct NactIFoldersTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* column ordering
 */
enum {
	FOLDERS_PATH_COLUMN = 0,
	FOLDERS_N_COLUMN
};

#define IPREFS_FOLDERS_DIALOG		"ifolders-chooser"
#define IPREFS_FOLDERS_PATH			"ifolders-path"

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;
static gboolean st_on_selection_change = FALSE;

static GType        register_type( void );
static void         interface_base_init( NactIFoldersTabInterface *klass );
static void         interface_base_finalize( NactIFoldersTabInterface *klass );

static void         on_tab_updatable_selection_changed( NactIFoldersTab *instance, gint count_selected );
static void         on_tab_updatable_enable_tab( NactIFoldersTab *instance, NAObjectItem *item );
static gboolean     tab_set_sensitive( NactIFoldersTab *instance );

static gboolean     on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, NactIFoldersTab *instance );
static void         inline_edition( NactIFoldersTab *instance );
static void         insert_new_row( NactIFoldersTab *instance );
static void         delete_row( NactIFoldersTab *instance );

static void         add_row( NactIFoldersTab *instance, GtkTreeView *listview, const gchar *path );
static void         add_path_to_folders( NactIFoldersTab *instance, const gchar *path );
static GtkTreeView *get_folders_treeview( NactIFoldersTab *instance );
static void         on_folder_path_edited( GtkCellRendererText *renderer, const gchar *path, const gchar *text, NactIFoldersTab *instance );
static void         on_folders_selection_changed( GtkTreeSelection *selection, NactIFoldersTab *instance );
static void         on_add_folder_clicked( GtkButton *button, NactIFoldersTab *instance );
static void         on_remove_folder_clicked( GtkButton *button, NactIFoldersTab *instance );
static void         remove_path_from_folders( NactIFoldersTab *instance, const gchar *path );
static void         reset_folders( NactIFoldersTab *instance );
static void         setup_folders( NactIFoldersTab *instance );
static void         treeview_cell_edited( NactIFoldersTab *instance, const gchar *path_string, const gchar *text, gint column, gchar **old_text );

GType
nact_ifolders_tab_get_type( void )
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
	static const gchar *thisfn = "nact_ifolders_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NactIFoldersTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NactIFoldersTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_WINDOW_TYPE );

	return( type );
}

static void
interface_base_init( NactIFoldersTabInterface *klass )
{
	static const gchar *thisfn = "nact_ifolders_tab_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NactIFoldersTabInterfacePrivate, 1 );

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NactIFoldersTabInterface *klass )
{
	static const gchar *thisfn = "nact_ifolders_tab_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
}

void
nact_ifolders_tab_initial_load_toplevel( NactIFoldersTab *instance )
{
	static const gchar *thisfn = "nact_ifolders_tab_initial_load_toplevel";
	GtkTreeView *listview;
	GtkListStore *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *text_cell;
	GtkTreeSelection *selection;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IFOLDERS_TAB( instance ));

	if( st_initialized && !st_finalized ){

		model = gtk_list_store_new( FOLDERS_N_COLUMN, G_TYPE_STRING );
		gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( model ), FOLDERS_PATH_COLUMN, GTK_SORT_ASCENDING );
		listview = get_folders_treeview( instance );
		gtk_tree_view_set_model( listview, GTK_TREE_MODEL( model ));
		g_object_unref( model );

		text_cell = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
				"folder-path",
				text_cell,
				"text", FOLDERS_PATH_COLUMN,
				NULL );
		gtk_tree_view_append_column( listview, column );

		gtk_tree_view_set_headers_visible( listview, FALSE );

		selection = gtk_tree_view_get_selection( listview );
		gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
	}
}

void
nact_ifolders_tab_runtime_init_toplevel( NactIFoldersTab *instance )
{
	static const gchar *thisfn = "nact_ifolders_tab_runtime_init_toplevel";
	GtkTreeView *listview;
	GtkTreeViewColumn *column;
	GList *renderers;
	GtkWidget *add_button, *remove_button;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IFOLDERS_TAB( instance ));

	if( st_initialized && !st_finalized ){

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

		listview = get_folders_treeview( instance );
		column = gtk_tree_view_get_column( listview, FOLDERS_PATH_COLUMN );
		renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( renderers->data ),
				"edited",
				G_CALLBACK( on_folder_path_edited ));

		add_button = base_window_get_widget( BASE_WINDOW( instance ), "AddFolderButton");
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( add_button ),
				"clicked",
				G_CALLBACK( on_add_folder_clicked ));

		remove_button = base_window_get_widget( BASE_WINDOW( instance ), "RemoveFolderButton");
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( remove_button ),
				"clicked",
				G_CALLBACK( on_remove_folder_clicked ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( gtk_tree_view_get_selection( listview )),
				"changed",
				G_CALLBACK( on_folders_selection_changed ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( listview ),
				"key-press-event",
				G_CALLBACK( on_key_pressed_event ));
	}
}

void
nact_ifolders_tab_all_widgets_showed( NactIFoldersTab *instance )
{
	static const gchar *thisfn = "nact_ifolders_tab_all_widgets_showed";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IFOLDERS_TAB( instance ));

	if( st_initialized && !st_finalized ){
	}
}

void
nact_ifolders_tab_dispose( NactIFoldersTab *instance )
{
	static const gchar *thisfn = "nact_ifolders_tab_dispose";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IFOLDERS_TAB( instance ));

	if( st_initialized && !st_finalized ){

		reset_folders( instance );
	}
}

static void
on_tab_updatable_selection_changed( NactIFoldersTab *instance, gint count_selected )
{
	static const gchar *thisfn = "nact_ifolders_tab_on_tab_updatable_selection_changed";
	NAObjectItem *item;
	NAObjectProfile *profile;
	gboolean enable_tab;
	gboolean editable;
	GtkTreeView *treeview;
	GtkTreeViewColumn *column;
	GtkWidget *widget;

	g_debug( "%s: instance=%p, count_selected=%d", thisfn, ( void * ) instance, count_selected );
	g_return_if_fail( NACT_IS_IFOLDERS_TAB( instance ));

	if( st_initialized && !st_finalized ){

		st_on_selection_change = TRUE;

		reset_folders( instance );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &item,
				TAB_UPDATABLE_PROP_EDITED_PROFILE, &profile,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		g_return_if_fail( !item || NA_IS_OBJECT_ITEM( item ));

		enable_tab = tab_set_sensitive( instance );

		if( item && NA_IS_OBJECT_ACTION( item )){
			setup_folders( instance );
		}

		treeview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( instance ), "FoldersTreeView" ));
		gtk_widget_set_sensitive( GTK_WIDGET( treeview ), profile != NULL );
		column = gtk_tree_view_get_column( treeview, FOLDERS_PATH_COLUMN );
		nact_gtk_utils_set_editable( GTK_OBJECT( column ), editable );

		widget = base_window_get_widget( BASE_WINDOW( instance ), "AddFolderButton" );
		gtk_widget_set_sensitive( widget, profile != NULL );
		nact_gtk_utils_set_editable( GTK_OBJECT( widget ), editable );

		widget = base_window_get_widget( BASE_WINDOW( instance ), "RemoveFolderButton" );
		gtk_widget_set_sensitive( widget, profile != NULL );
		nact_gtk_utils_set_editable( GTK_OBJECT( widget ), editable );

		st_on_selection_change = FALSE;
	}
}

static void
on_tab_updatable_enable_tab( NactIFoldersTab *instance, NAObjectItem *item )
{
	static const gchar *thisfn = "nact_ifolders_tab_on_tab_updatable_enable_tab";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p, item=%p", thisfn, ( void * ) instance, ( void * ) item );
		g_return_if_fail( NACT_IS_IFOLDERS_TAB( instance ));

		tab_set_sensitive( instance );
	}
}

static gboolean
tab_set_sensitive( NactIFoldersTab *instance )
{
	NAObjectAction *action;
	NAObjectProfile *profile;
	gboolean enable_tab;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &action,
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &profile,
			NULL );

	enable_tab = ( profile != NULL );
	nact_main_tab_enable_page( NACT_MAIN_WINDOW( instance ), TAB_FOLDERS, enable_tab );

	return( enable_tab );
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, NactIFoldersTab *instance )
{
	gboolean stop;
	gboolean editable;

	stop = FALSE;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITABLE, &editable,
			NULL );

	if( editable ){

		if( event->keyval == GDK_F2 ){
			inline_edition( instance );
			stop = TRUE;
		}

		if( event->keyval == GDK_Insert || event->keyval == GDK_KP_Insert ){
			insert_new_row( instance );
			stop = TRUE;
		}

		if( event->keyval == GDK_Delete || event->keyval == GDK_KP_Delete ){
			delete_row( instance );
			stop = TRUE;
		}
	}

	return( stop );
}

static void
inline_edition( NactIFoldersTab *instance )
{
	GtkTreeView *listview;
	GtkTreeSelection *selection;
	GList *listrows;
	GtkTreePath *path;
	GtkTreeViewColumn *column;

	listview = get_folders_treeview( instance );
	selection = gtk_tree_view_get_selection( listview );
	listrows = gtk_tree_selection_get_selected_rows( selection, NULL );

	if( g_list_length( listrows ) == 1 ){
		path = ( GtkTreePath * ) listrows->data;
		column = gtk_tree_view_get_column( listview, FOLDERS_PATH_COLUMN );
		gtk_tree_view_set_cursor( listview, path, column, TRUE );
	}

	g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( listrows );
}

/*
 * the list is sorted on path: it is no worth to try to insert a row
 * before currently selected item...
 */
static void
insert_new_row( NactIFoldersTab *instance )
{
	GtkTreeView *listview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	const gchar *folder_path = "/";
	GtkTreePath *path;
	GtkTreeViewColumn *column;

	listview = get_folders_treeview( instance );
	model = gtk_tree_view_get_model( listview );

	gtk_list_store_append( GTK_LIST_STORE( model ), &iter );
	gtk_list_store_set( GTK_LIST_STORE( model ), &iter, FOLDERS_PATH_COLUMN, folder_path, -1 );
	add_path_to_folders( instance, folder_path );

	path = gtk_tree_model_get_path( model, &iter );
	column = gtk_tree_view_get_column( listview, FOLDERS_PATH_COLUMN );
	gtk_tree_view_set_cursor( listview, path, column, TRUE );
	gtk_tree_path_free( path );
}

static void
delete_row( NactIFoldersTab *instance )
{
	GtkTreeView *listview;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *rows;
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *folder_path;

	listview = get_folders_treeview( instance );
	model = gtk_tree_view_get_model( listview );
	selection = gtk_tree_view_get_selection( listview );
	rows = gtk_tree_selection_get_selected_rows( selection, NULL );

	if( g_list_length( rows ) == 1 ){
		path = ( GtkTreePath * ) rows->data;
		gtk_tree_model_get_iter( model, &iter, path );
		gtk_tree_model_get( model, &iter, FOLDERS_PATH_COLUMN, &folder_path, -1 );

		gtk_list_store_remove( GTK_LIST_STORE( model ), &iter );
		remove_path_from_folders( instance, folder_path );
		g_free( folder_path );

		if( gtk_tree_model_get_iter( model, &iter, path ) ||
			gtk_tree_path_prev( path )){

			gtk_tree_view_set_cursor( listview, path, NULL, FALSE );
		}
	}

	g_list_foreach( rows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( rows );
}

static void
add_row( NactIFoldersTab *instance, GtkTreeView *listview, const gchar *path )
{
	GtkTreeModel *model;
	GtkTreeIter row;

	model = gtk_tree_view_get_model( listview );

	gtk_list_store_append(
			GTK_LIST_STORE( model ),
			&row );

	gtk_list_store_set(
			GTK_LIST_STORE( model ),
			&row,
			FOLDERS_PATH_COLUMN, path,
			-1 );
}

static void
add_path_to_folders( NactIFoldersTab *instance, const gchar *path )
{
	NAObjectAction *action;
	NAObjectProfile *edited;
	GSList *folders;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &action,
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
			NULL );

	folders = na_object_get_folders( edited );
	folders = g_slist_prepend( folders, ( gpointer ) g_strdup( path ));
	na_object_set_folders( edited, folders );
	na_core_utils_slist_free( folders );

	g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
}

static GtkTreeView *
get_folders_treeview( NactIFoldersTab *instance )
{
	GtkWidget *treeview;

	treeview = base_window_get_widget( BASE_WINDOW( instance ), "FoldersTreeview" );
	g_assert( GTK_IS_TREE_VIEW( treeview ));

	return( GTK_TREE_VIEW( treeview ));
}

static void
on_add_folder_clicked( GtkButton *button, NactIFoldersTab *instance )
{
#if 0
	/* this is the code I sent to gtk-app-devel list
	 * to know why one is not able to just enter '/' in the location entry
	 */
	GtkWidget *dialog;
	gchar *path;

	dialog = gtk_file_chooser_dialog_new( _( "Select a folder" ),
			NULL,
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL );

	gtk_file_chooser_set_filename( GTK_FILE_CHOOSER( dialog ), "/" );

	if( gtk_dialog_run( GTK_DIALOG( dialog )) == GTK_RESPONSE_ACCEPT ){
		path = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ));
		g_debug( "nact_ifolders_tab_on_add_folder_clicked: path=%s", path );
		g_free( path );
	}

	gtk_widget_destroy( dialog );
#endif
	GtkWidget *dialog;
	GtkWindow *toplevel;
	NactApplication *application;
	NAUpdater *updater;
	gchar *path;
	GtkTreeView *listview;

	path = NULL;
	listview = get_folders_treeview( instance );
	toplevel = base_window_get_toplevel( BASE_WINDOW( instance ));

	/* i18n: title of the FileChoose dialog when selecting an URI which
	 * will be compare to Caja 'current_folder'
	 */
	dialog = gtk_file_chooser_dialog_new( _( "Select a folder" ),
			toplevel,
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL );

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( instance )));
	updater = nact_application_get_updater( application );

	base_iprefs_position_named_window( BASE_WINDOW( instance ), GTK_WINDOW( dialog ), IPREFS_FOLDERS_DIALOG );

	path = na_iprefs_read_string( NA_IPREFS( updater ), IPREFS_FOLDERS_PATH, "/" );
	if( path && g_utf8_strlen( path, -1 )){
		gtk_file_chooser_set_filename( GTK_FILE_CHOOSER( dialog ), path );
	}
	g_free( path );

	if( gtk_dialog_run( GTK_DIALOG( dialog )) == GTK_RESPONSE_ACCEPT ){
		path = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ));
		nact_iprefs_write_string( BASE_WINDOW( instance ), IPREFS_FOLDERS_PATH, path );
		add_row( instance, listview, path );
		add_path_to_folders( instance, path );
		g_free( path );
	}

	base_iprefs_save_named_window_position( BASE_WINDOW( instance ), GTK_WINDOW( dialog ), IPREFS_FOLDERS_DIALOG );

	gtk_widget_destroy( dialog );
}

static void
on_folder_path_edited( GtkCellRendererText *renderer, const gchar *path, const gchar *text, NactIFoldersTab *instance )
{
	treeview_cell_edited( instance, path, text, FOLDERS_PATH_COLUMN, NULL );
}

static void
on_folders_selection_changed( GtkTreeSelection *selection, NactIFoldersTab *instance )
{
	gboolean editable;
	GtkWidget *button;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITABLE, &editable,
			NULL );

	if( editable ){
		button = base_window_get_widget( BASE_WINDOW( instance ), "RemoveFolderButton");
		gtk_widget_set_sensitive( button, gtk_tree_selection_count_selected_rows( selection ) > 0 );
	}
}

static void
on_remove_folder_clicked( GtkButton *button, NactIFoldersTab *instance )
{
	delete_row( instance );
}

static void
remove_path_from_folders( NactIFoldersTab *instance, const gchar *path )
{
	NAObjectProfile *edited;
	GSList *folders;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
			NULL );

	folders = na_object_get_folders( edited );
	folders = na_core_utils_slist_remove_utf8( folders, path );
	na_object_set_folders( edited, folders );

	na_core_utils_slist_free( folders );

	g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
}

static void
reset_folders( NactIFoldersTab *instance )
{
	GtkTreeView *listview;
	GtkTreeModel *model;

	listview = get_folders_treeview( instance );
	model = gtk_tree_view_get_model( listview );
	gtk_list_store_clear( GTK_LIST_STORE( model ));
}

static void
setup_folders( NactIFoldersTab *instance )
{
	NAObjectProfile *edited;
	GSList *folders, *is;
	GtkTreeView *listview;

	listview = get_folders_treeview( instance );

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
			NULL );

	if( edited ){
		folders = na_object_get_folders( edited );
		for( is = folders ; is ; is = is->next ){
			add_row( instance, listview, ( const gchar * ) is->data );
		}
		na_core_utils_slist_free( folders );
	}
}

static void
treeview_cell_edited( NactIFoldersTab *instance, const gchar *path_string, const gchar *text, gint column, gchar **old_text )
{
	static const gchar *thisfn = "nact_ifolders_tab_treeview_cell_edited";
	GtkTreeView *listview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	NAObjectAction *action;
	NAObjectProfile *edited;
	gchar *previous_text;

	g_debug( "%s: instance=%p, path_string=%s, text=%s, column=%d",
			thisfn, ( void * ) instance, path_string, text, column );

	listview = get_folders_treeview( instance );
	model = gtk_tree_view_get_model( listview );
	path = gtk_tree_path_new_from_string( path_string );
	gtk_tree_model_get_iter( model, &iter, path );
	gtk_tree_path_free( path );

	gtk_tree_model_get( model, &iter, FOLDERS_PATH_COLUMN, &previous_text, -1 );

	gtk_list_store_set( GTK_LIST_STORE( model ), &iter, column, text, -1 );

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &action,
			TAB_UPDATABLE_PROP_EDITED_PROFILE, &edited,
			NULL );

	na_object_replace_folder( edited, previous_text, text );

	if( old_text ){
		*old_text = g_strdup( previous_text );
	}
	g_free( previous_text );

	g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
}
