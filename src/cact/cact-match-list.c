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

#include <glib/gi18n.h>
#include <libintl.h>

#include <api/na-object-api.h>
#include <api/na-core-utils.h>

#include "base-keysyms.h"
#include "base-gtk-utils.h"
#include "cact-main-tab.h"
#include "cact-match-list.h"

/* column ordering
 */
enum {
	ITEM_COLUMN = 0,
	MUST_MATCH_COLUMN,
	MUST_NOT_MATCH_COLUMN,
	N_COLUMN
};

typedef struct {
	guint  header_id;
	gchar *header_label;
}
	ColumnHeaderStruct;

/* internal data set against the instance,
 * addressed with the tab name
 */
typedef struct {
	BaseWindow      *window;
	gchar           *tab_name;
	guint            tab_id;
	GtkTreeView     *listview;
	GtkWidget       *addbutton;
	GtkWidget       *removebutton;
	pget_filters     pget;
	pset_filters     pset;
	pon_add_cb       pon_add;
	pon_remove_cb    pon_remove;
	guint            match_header;
	gchar           *item_header;
	gboolean         editable_filter;
	/* dynamic data */
	gboolean         on_selection_change;
	gboolean         editable_item;
	guint            sort_column;
	guint            sort_order;
}
	MatchListData;

/* i18n: label of the header of the column which let the user select a positive filter
 */
static ColumnHeaderStruct st_match_headers[] = {
	{ MATCH_LIST_MUST_MATCH_ONE_OF, N_( "Must match one of" ) },
	{ MATCH_LIST_MUST_MATCH_ALL_OF, N_( "Must match all of" ) },
	{ 0 }
};

static void         initialize_pseudo_iface( MatchListData *data );
static void         create_tree_model( MatchListData *data );
static void         on_base_initialize_window( BaseWindow *window, MatchListData *data );
static void         on_main_selection_changed( BaseWindow *window, GList *selected_items, MatchListData *data );

static void         on_add_filter_clicked( GtkButton *button, MatchListData *data );
static void         on_filter_clicked( GtkTreeViewColumn *treeviewcolumn, MatchListData *data );
static void         on_filter_edited( GtkCellRendererText *renderer, const gchar *path, const gchar *text, MatchListData *data );
static gboolean     on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, MatchListData *data );
static void         on_must_match_clicked( GtkTreeViewColumn *treeviewcolumn, MatchListData *data );
static void         on_must_match_toggled( GtkCellRendererToggle *cell_renderer, gchar *path, MatchListData *data );
static void         on_must_not_match_clicked( GtkTreeViewColumn *treeviewcolumn, MatchListData *data );
static void         on_must_not_match_toggled( GtkCellRendererToggle *cell_renderer, gchar *path, MatchListData *data );
static void         on_remove_filter_clicked( GtkButton *button, MatchListData *data );
static void         on_selection_changed( GtkTreeSelection *selection, MatchListData *data );

static void         add_filter( MatchListData *data, const gchar *filter, const gchar *prefix );
static guint        count_filters( const gchar *filter, MatchListData *data );
static void         delete_current_row( MatchListData *data );
static void         delete_row_at_path( GtkTreeView *treeview, GtkTreeModel *model, GtkTreePath *path );
static void         dump_current_rows( MatchListData *data );
static void         edit_inline( MatchListData *data );
static gchar       *get_filter_from_path( const gchar *path_str, MatchListData *data );
static const gchar *get_must_match_header( guint id );
static gboolean     get_rows_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter* iter, GSList **filters );
static void         insert_new_row( MatchListData *data );
static void         insert_new_row_data( MatchListData *data, const gchar *filter, gboolean match, gboolean no_match );
static void         iter_for_setup( gchar *filter, GtkTreeModel *model );
static gchar       *search_for_unique_label( const gchar *propal, MatchListData *data );
static void         set_match_status( const gchar *path_str, gboolean must_match, gboolean must_not_match, MatchListData *data );
static void         sort_on_column( GtkTreeViewColumn *treeviewcolumn, MatchListData *data, guint colid );

static void         on_instance_finalized( MatchListData *data, BaseWindow *window );

/**
 * cact_match_list_init_with_args:
 * @window: the #BaseWindow window which contains the view.
 * @tab_name: a string constant which identifies this page.
 * @tab_id: our id for this page.
 * @listview: the #GtkTreeView widget.
 * @addbutton: the #GtkButton widget.
 * @removebutton: the #GtkButton widget.
 * @pget: a pointer to the function to get the list of filters.
 * @pset: a pointer to the function to set the list of filters.
 * @pon_add: an optional pointer to a function which handles the Add button.
 * @pon_remove: an optional pointer to a function which handles the Remove button.
 * @item_header: the title of the item header.
 *
 * Initialize this pseudo-interface, and creates the tree model.
 *
 * This function can only be called when (and so should be called immediately
 * after) the Gtk toplevel has been first initialized. This is because we need
 * here pointers to GtkTreeView and GtkButton widgets.
 */
void
cact_match_list_init_with_args( BaseWindow *window, const gchar *tab_name,
				guint tab_id,
				GtkWidget *listview,
				GtkWidget *addbutton,
				GtkWidget *removebutton,
				pget_filters pget,
				pset_filters pset,
				pon_add_cb pon_add,
				pon_remove_cb pon_remove,
				guint match_header,
				const gchar *item_header,
				gboolean editable_filter )
{
	static const gchar *thisfn = "cact_match_list_init_with_args";
	MatchListData *data;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	g_debug( "%s: window=%p, tab_name=%s", thisfn, ( void * ) window, tab_name );

	data = g_new0( MatchListData, 1 );

	/* parameters
	 */
	data->window = window;
	data->tab_name = g_strdup( tab_name );
	data->tab_id = tab_id;
	data->listview = GTK_TREE_VIEW( listview );
	data->addbutton = addbutton;
	data->removebutton = removebutton;
	data->pget = pget;
	data->pset = pset;
	data->pon_add = pon_add;
	data->pon_remove = pon_remove;
	data->match_header = match_header;
	data->item_header = g_strdup( item_header );
	data->editable_filter = editable_filter;

	/* dynamic data
	 */
	data->on_selection_change = FALSE;
	data->editable_item = FALSE;
	data->sort_column = 0;
	data->sort_order = 0;

	g_object_set_data( G_OBJECT( window ), tab_name, data );

	initialize_pseudo_iface( data );
	create_tree_model( data );
}

static void
initialize_pseudo_iface( MatchListData *data )
{
	base_window_signal_connect_with_data(
			data->window,
			G_OBJECT( data->window ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_base_initialize_window ),
			data );

	g_object_weak_ref( G_OBJECT( data->window ), ( GWeakNotify ) on_instance_finalized, data );
}

static void
create_tree_model( MatchListData *data )
{
	GtkListStore *model;
	GtkCellRenderer *text_cell, *radio_cell;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	model = gtk_list_store_new( N_COLUMN, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN );
	gtk_tree_view_set_model( data->listview, GTK_TREE_MODEL( model ));
	g_object_unref( model );

	text_cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
			data->item_header,
			text_cell,
			"text", ITEM_COLUMN,
			NULL );
	gtk_tree_view_append_column( data->listview, column );

	radio_cell = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio( GTK_CELL_RENDERER_TOGGLE( radio_cell ), TRUE );
	column = gtk_tree_view_column_new_with_attributes(
			get_must_match_header( data->match_header ),
			radio_cell,
			"active", MUST_MATCH_COLUMN,
			NULL );
	gtk_tree_view_append_column( data->listview, column );

	radio_cell = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio( GTK_CELL_RENDERER_TOGGLE( radio_cell ), TRUE );
	column = gtk_tree_view_column_new_with_attributes(
			/* i18n: label of the header of a column which let the user select a negative filter */
			_( "Must not match any of" ),
			radio_cell,
			"active", MUST_NOT_MATCH_COLUMN,
			NULL );
	gtk_tree_view_append_column( data->listview, column );

	/* an empty column to fill out the view
	 */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column( data->listview, column );

	gtk_tree_view_set_headers_visible( data->listview, TRUE );
	gtk_tree_view_set_headers_clickable( data->listview, TRUE );

	selection = gtk_tree_view_get_selection( data->listview );
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
}

/*
 *  Initializes the tab widget at each time the widget will be displayed.
 * Connect signals.
 */
static void
on_base_initialize_window( BaseWindow *window, MatchListData *data )
{
	GtkTreeViewColumn *column;
	GList *renderers;
	GtkTreeModel *model;

	g_return_if_fail( data != NULL );

	column = gtk_tree_view_get_column( data->listview, ITEM_COLUMN );
	base_window_signal_connect_with_data(
			window,
			G_OBJECT( column ),
			"clicked",
			G_CALLBACK( on_filter_clicked ),
			data );

	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
	base_window_signal_connect_with_data(
			window,
			G_OBJECT( renderers->data ),
			"edited",
			G_CALLBACK( on_filter_edited ),
			data );

	column = gtk_tree_view_get_column( data->listview, MUST_MATCH_COLUMN );
	base_window_signal_connect_with_data(
			window,
			G_OBJECT( column ),
			"clicked",
			G_CALLBACK( on_must_match_clicked ),
			data );

	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
	base_window_signal_connect_with_data(
			window,
			G_OBJECT( renderers->data ),
			"toggled",
			G_CALLBACK( on_must_match_toggled ),
			data );

	column = gtk_tree_view_get_column( data->listview, MUST_NOT_MATCH_COLUMN );
	base_window_signal_connect_with_data(
			window,
			G_OBJECT( column ),
			"clicked",
			G_CALLBACK( on_must_not_match_clicked ),
			data );

	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
	base_window_signal_connect_with_data(
			window,
			G_OBJECT( renderers->data ),
			"toggled",
			G_CALLBACK( on_must_not_match_toggled ),
			data );

	if( data->pon_add ){
		base_window_signal_connect(
				window,
				G_OBJECT( data->addbutton ),
				"clicked",
				G_CALLBACK( data->pon_add ));
	} else {
		base_window_signal_connect_with_data(
				window,
				G_OBJECT( data->addbutton ),
				"clicked",
				G_CALLBACK( on_add_filter_clicked ),
				data );
	}

	if( data->pon_remove ){
		base_window_signal_connect(
				window,
				G_OBJECT( data->removebutton ),
				"clicked",
				G_CALLBACK( data->pon_remove ));
	} else {
		base_window_signal_connect_with_data(
				window,
				G_OBJECT( data->removebutton ),
				"clicked",
				G_CALLBACK( on_remove_filter_clicked ),
				data );
	}

	base_window_signal_connect_with_data(
			window,
			G_OBJECT( gtk_tree_view_get_selection( data->listview )),
			"changed",
			G_CALLBACK( on_selection_changed ),
			data );

	base_window_signal_connect_with_data(
			window,
			G_OBJECT( data->listview ),
			"key-press-event",
			G_CALLBACK( on_key_pressed_event ),
			data );

	model = gtk_tree_view_get_model( data->listview );
	gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( model ), ITEM_COLUMN, GTK_SORT_ASCENDING );
	data->sort_column = ITEM_COLUMN;
	data->sort_order = GTK_SORT_ASCENDING;

	column = gtk_tree_view_get_column( data->listview, ITEM_COLUMN );
	sort_on_column( column, data, ITEM_COLUMN );

	base_window_signal_connect_with_data(
			window,
			G_OBJECT( window ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ),
			data );
}

/*
 * Called each time the selection changes in the Actions tree view.
 *
 * Basically we are using here a rather common scheme:
 * - object has a GSList of strings, each of one being a filter description,
 *   which may be negated
 * - the list is displayed in a listview with radio toggle buttons
 *   so that a user cannot have both positive and negative assertions
 *   for the same basename filter
 * - update the object with a summary of the listbox contents
 */
static void
on_main_selection_changed( BaseWindow *window, GList *selected_items, MatchListData *data )
{
	static const gchar *thisfn = "cact_match_list_on_main_selection_changed";
	NAIContext *context;
	gboolean enable_tab;
	GSList *filters;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkTreePath *path;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( data != NULL );

	g_object_get( G_OBJECT( window ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &data->editable_item,
			NULL );

	enable_tab = ( context != NULL );
	cact_main_tab_enable_page( CACT_MAIN_WINDOW( data->window ), data->tab_id, enable_tab );

	data->on_selection_change = TRUE;

	filters = context ? ( *data->pget )( context ) : NULL;
	g_debug( "%s: filters=%p (count=%d)", thisfn, ( void * ) filters, filters ? g_slist_length( filters ) : -1 );

	model = gtk_tree_view_get_model( data->listview );
	selection = gtk_tree_view_get_selection( data->listview );
	gtk_tree_selection_unselect_all( selection );
	gtk_list_store_clear( GTK_LIST_STORE( model ));

	if( filters ){
		na_core_utils_slist_dump( thisfn, filters );
		g_slist_foreach( filters, ( GFunc ) iter_for_setup, model );
	}

	column = gtk_tree_view_get_column( data->listview, ITEM_COLUMN );
	base_gtk_utils_set_editable( G_OBJECT( column ), data->editable_item && data->editable_filter );

	base_gtk_utils_set_editable( G_OBJECT( data->addbutton ), data->editable_item );
	base_gtk_utils_set_editable( G_OBJECT( data->removebutton ), data->editable_item );
	gtk_widget_set_sensitive( data->removebutton, FALSE );

	data->on_selection_change = FALSE;

	path = gtk_tree_path_new_first();
	if( path ){
		selection = gtk_tree_view_get_selection( data->listview );
		gtk_tree_selection_select_path( selection, path );
		gtk_tree_path_free( path );
	}
}

/**
 * cact_match_list_insert_row:
 * @window: the #BaseWindow window which contains the view.
 * @tab_name: a string constant which identifies this page.
 * @filter: the item to add.
 * @match: whether the 'must match' column is checked.
 * @not_match: whether the 'must not match' column is checked.
 *
 * Add a new row to the list view.
 */
void
cact_match_list_insert_row( BaseWindow *window, const gchar *tab_name, const gchar *filter, gboolean match, gboolean not_match )
{
	MatchListData *data;

	data = ( MatchListData * ) g_object_get_data( G_OBJECT( window ), tab_name );
	g_return_if_fail( data != NULL );

	insert_new_row_data( data, filter, match, not_match );
}

/**
 * cact_match_list_get_rows:
 * @window: the #BaseWindow window which contains the view.
 * @tab_name: a string constant which identifies this page.
 *
 * Returns the list of rows as a newly allocated string list which should
 * be na_core_utils_slist_free() by the caller.
 */
GSList *
cact_match_list_get_rows( BaseWindow *window, const gchar *tab_name )
{
	GSList *filters;
	MatchListData *data;
	GtkTreeModel *model;

	data = ( MatchListData * ) g_object_get_data( G_OBJECT( window ), tab_name );
	g_return_val_if_fail( data != NULL, NULL );

	model = gtk_tree_view_get_model( data->listview );
	filters = NULL;
	gtk_tree_model_foreach( model, ( GtkTreeModelForeachFunc ) get_rows_iter, &filters );

	return( filters );
}

/* callback function called when the user clicks on "Add" button
 * and no application callback has been provided by the caller
 * at initialization time
 */
static void
on_add_filter_clicked( GtkButton *button, MatchListData *data )
{
	insert_new_row( data );
}

static void
on_filter_clicked( GtkTreeViewColumn *treeviewcolumn, MatchListData *data )
{
	sort_on_column( treeviewcolumn, data, ITEM_COLUMN );
}

static void
on_filter_edited( GtkCellRendererText *renderer, const gchar *path_str, const gchar *text, MatchListData *data )
{
	static const gchar *thisfn = "cact_match_list_on_filter_edited";
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	gchar *old_text;
	NAIContext *context;
	gboolean must_match, must_not_match;
	gchar *to_add, *to_remove;
	GSList *filters;
	GtkWidget *dialog;

	g_return_if_fail( data->editable_filter );

	g_object_get( G_OBJECT( data->window ), MAIN_PROP_CONTEXT, &context, NULL );
	g_return_if_fail( NA_IS_ICONTEXT( context ));

	model = gtk_tree_view_get_model( data->listview );
	path = gtk_tree_path_new_from_string( path_str );
	gtk_tree_model_get_iter( model, &iter, path );
	gtk_tree_path_free( path );

	gtk_tree_model_get( model, &iter, ITEM_COLUMN, &old_text, -1 );

	if( strcmp( text, old_text ) == 0 ){
		return;
	}

	dump_current_rows( data );
	g_debug( "%s: new filter=%s, count=%d", thisfn, text, count_filters( text, data ));

	if( count_filters( text, data ) >= 1 ){
		dialog = gtk_message_dialog_new(
				base_window_get_gtk_toplevel( BASE_WINDOW( data->window )),
				GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
				_( "'%s' filter already exists in the list.\nPlease provide another one." ), text );
		gtk_dialog_run( GTK_DIALOG( dialog ));
		gtk_widget_destroy( dialog );

		return;
	}

	gtk_tree_model_get( model, &iter,
			MUST_MATCH_COLUMN, &must_match,
			MUST_NOT_MATCH_COLUMN, &must_not_match,
			-1 );

	gtk_list_store_set( GTK_LIST_STORE( model ), &iter, ITEM_COLUMN, text, -1 );

	filters = ( *data->pget )( context );

	if( filters ){
		to_remove = g_strdup( old_text );
		filters = na_core_utils_slist_remove_ascii( filters, to_remove );
		g_free( to_remove );
		to_remove = g_strdup_printf( "!%s", old_text );
		filters = na_core_utils_slist_remove_ascii( filters, to_remove );
		g_free( to_remove );
	}

	if( must_match ){
		filters = g_slist_prepend( filters, g_strdup( text ));

	} else if( must_not_match ){
		to_add = g_strdup_printf( "!%s", text );
		filters = g_slist_prepend( filters, to_add );
	}

	( *data->pset )( context, filters );
	na_core_utils_slist_free( filters );
	g_free( old_text );

	g_signal_emit_by_name( G_OBJECT( data->window ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, MatchListData *data )
{
	gboolean stop;

	stop = FALSE;

	if( event->keyval == CACT_KEY_F2 ){
		if( data->editable_filter ){
			edit_inline( data );
			stop = TRUE;
		}
	}

	if( event->keyval == CACT_KEY_Insert || event->keyval == CACT_KEY_KP_Insert ){
		if( data->editable_item ){
			insert_new_row( data );
			stop = TRUE;
		}
	}

	if( event->keyval == CACT_KEY_Delete || event->keyval == CACT_KEY_KP_Delete ){
		if( data->editable_item ){
			delete_current_row( data );
			stop = TRUE;
		}
	}

	return( stop );
}

static void
on_must_match_clicked( GtkTreeViewColumn *treeviewcolumn, MatchListData *data )
{
	sort_on_column( treeviewcolumn, data, MUST_MATCH_COLUMN );
}

/*
 * clicking on an already active toggle button has no effect
 * clicking on an icactive toggle button has a double effect:
 * - the other toggle button becomes icactive
 * - this toggle button becomes active
 * the corresponding strings must be respectively removed/added to the
 * filters list
 */
static void
on_must_match_toggled( GtkCellRendererToggle *cell_renderer, gchar *path_str, MatchListData *data )
{
	/*static const gchar *thisfn = "cact_match_list_on_must_match_toggled";*/
	gchar *filter;
	NAIContext *context;
	GSList *filters;
	gchar *to_remove;
	gboolean active;

	/*gboolean is_active = gtk_cell_renderer_toggle_get_active( cell_renderer );
	g_debug( "%s: is_active=%s", thisfn, is_active ? "True":"False" );*/

	active = gtk_cell_renderer_toggle_get_active( cell_renderer );

	if( data->editable_item ){
		if( !active ){
			g_object_get( G_OBJECT( data->window ), MAIN_PROP_CONTEXT, &context, NULL );
			g_return_if_fail( NA_IS_ICONTEXT( context ));

			set_match_status( path_str, TRUE, FALSE, data );

			filter = get_filter_from_path( path_str, data );
			filters = ( *data->pget )( context );

			if( filters ){
				to_remove = g_strdup_printf( "!%s", filter );
				filters = na_core_utils_slist_remove_ascii( filters, to_remove );
				g_free( to_remove );
			}

			filters = g_slist_prepend( filters, g_strdup( filter ));
			( *data->pset )( context, filters );

			na_core_utils_slist_free( filters );
			g_free( filter );

			g_signal_emit_by_name( G_OBJECT( data->window ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
		}
	} else {
		g_signal_handlers_block_by_func(( gpointer ) cell_renderer, on_must_match_toggled, data );
		gtk_cell_renderer_toggle_set_active( cell_renderer, !active );
		g_signal_handlers_unblock_by_func(( gpointer ) cell_renderer, on_must_match_toggled, data );
	}
}

static void
on_must_not_match_clicked( GtkTreeViewColumn *treeviewcolumn, MatchListData *data )
{
	sort_on_column( treeviewcolumn, data, MUST_NOT_MATCH_COLUMN );
}

static void
on_must_not_match_toggled( GtkCellRendererToggle *cell_renderer, gchar *path_str, MatchListData *data )
{
	/*static const gchar *thisfn = "cact_match_list_on_must_not_match_toggled";*/
	gchar *filter;
	NAIContext *context;
	GSList *filters;
	gchar *to_add;
	gboolean active;

	/*gboolean is_active = gtk_cell_renderer_toggle_get_active( cell_renderer );
	g_debug( "%s: is_active=%s", thisfn, is_active ? "True":"False" );*/

	active = gtk_cell_renderer_toggle_get_active( cell_renderer );

	if( data->editable_item ){
		if( !active ){
			g_object_get( G_OBJECT( data->window ), MAIN_PROP_CONTEXT, &context, NULL );
			g_return_if_fail( NA_IS_ICONTEXT( context ));

			set_match_status( path_str, FALSE, TRUE, data );

			filter = get_filter_from_path( path_str, data );
			filters = ( *data->pget )( context );

			if( filters ){
				filters = na_core_utils_slist_remove_ascii( filters, filter );
			}

			to_add = g_strdup_printf( "!%s", filter );
			filters = g_slist_prepend( filters, to_add );
			( *data->pset )( context, filters );

			na_core_utils_slist_free( filters );
			g_free( filter );

			g_signal_emit_by_name( G_OBJECT( data->window ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
		}
	} else {
		g_signal_handlers_block_by_func(( gpointer ) cell_renderer, on_must_not_match_toggled, data );
		gtk_cell_renderer_toggle_set_active( cell_renderer, !active );
		g_signal_handlers_unblock_by_func(( gpointer ) cell_renderer, on_must_not_match_toggled, data );
	}
}

static void
on_remove_filter_clicked( GtkButton *button, MatchListData *data )
{
	delete_current_row( data );
}

static void
on_selection_changed( GtkTreeSelection *selection, MatchListData *data )
{
	gtk_widget_set_sensitive( data->removebutton,
			data->editable_item && gtk_tree_selection_count_selected_rows( selection ) > 0 );
}

static void
add_filter( MatchListData *data, const gchar *filter, const gchar *prefix )
{
	NAIContext *context;
	GSList *filters;
	gchar *to_add;

	g_object_get( G_OBJECT( data->window ), MAIN_PROP_CONTEXT, &context, NULL );

	if( context ){
		filters = ( *data->pget )( context );
		to_add = g_strdup_printf( "%s%s", prefix, filter );
		filters = g_slist_prepend( filters, to_add );
		( *data->pset )( context, filters );
		na_core_utils_slist_free( filters );

		g_signal_emit_by_name( G_OBJECT( data->window ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
	}
}

static guint
count_filters( const gchar *filter, MatchListData *data )
{
	guint count;
	GtkTreeModel *model;
	GSList *filters;

	model = gtk_tree_view_get_model( data->listview );
	filters = NULL;
	gtk_tree_model_foreach( model, ( GtkTreeModelForeachFunc ) get_rows_iter, &filters );
	count = na_core_utils_slist_count( filters, filter );
	na_core_utils_slist_free( filters );

	return( count );
}

static void
delete_current_row( MatchListData *data )
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *rows;
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *filter;
	NAIContext *context;
	GSList *filters;
	gchar *to_remove;

	selection = gtk_tree_view_get_selection( data->listview );
	model = gtk_tree_view_get_model( data->listview );
	rows = gtk_tree_selection_get_selected_rows( selection, NULL );

	if( g_list_length( rows ) == 1 ){
		path = ( GtkTreePath * ) rows->data;
		gtk_tree_model_get_iter( model, &iter, path );
		gtk_tree_model_get( model, &iter, ITEM_COLUMN, &filter, -1 );

		delete_row_at_path( data->listview, model, path );

		g_object_get( G_OBJECT( data->window ), MAIN_PROP_CONTEXT, &context, NULL );

		if( context ){
			filters = ( *data->pget )( context );

			if( filters ){
				to_remove = g_strdup_printf( "!%s", filter );
				filters = na_core_utils_slist_remove_ascii( filters, to_remove );
				g_free( to_remove );
				filters = na_core_utils_slist_remove_ascii( filters, filter );
				( *data->pset )( context, filters );
				na_core_utils_slist_free( filters );

				g_signal_emit_by_name( G_OBJECT( data->window ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );
			}
		}

		g_free( filter );
	}

	g_list_foreach( rows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( rows );
}

static void
delete_row_at_path( GtkTreeView *treeview, GtkTreeModel *model, GtkTreePath *path )
{
	GtkTreeIter iter;

	if( gtk_tree_model_get_iter( model, &iter, path )){
		gtk_list_store_remove( GTK_LIST_STORE( model ), &iter );

		if( gtk_tree_model_get_iter( model, &iter, path ) || gtk_tree_path_prev( path )){
			gtk_tree_view_set_cursor( treeview, path, NULL, FALSE );
		}
	}
}

static void
dump_current_rows( MatchListData *data )
{
#ifdef NA_MAINTAINER_MODE
	GtkTreeModel *model;
	GSList *filters;

	model = gtk_tree_view_get_model( data->listview );
	filters = NULL;
	gtk_tree_model_foreach( model, ( GtkTreeModelForeachFunc ) get_rows_iter, &filters );
	na_core_utils_slist_dump( "cact_match_list_dump_current_rows", filters );
	na_core_utils_slist_free( filters );
#endif
}

static void
edit_inline( MatchListData *data )
{
	GtkTreeSelection *selection;
	GList *rows;
	GtkTreePath *path;
	GtkTreeViewColumn *column;

	selection = gtk_tree_view_get_selection( data->listview );
	rows = gtk_tree_selection_get_selected_rows( selection, NULL );

	if( g_list_length( rows ) == 1 ){
		gtk_tree_view_get_cursor( data->listview, &path, &column );
		gtk_tree_view_set_cursor( data->listview, path, column, TRUE );
		gtk_tree_path_free( path );
	}

	g_list_foreach( rows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( rows );
}

static gchar *
get_filter_from_path( const gchar *path_str, MatchListData *data )
{
	gchar *filter;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;

	filter = NULL;

	model = gtk_tree_view_get_model( data->listview );
	path = gtk_tree_path_new_from_string( path_str );
	gtk_tree_model_get_iter( model, &iter, path );
	gtk_tree_path_free( path );

	gtk_tree_model_get( model, &iter, ITEM_COLUMN, &filter, -1 );

	return( filter );
}

static const gchar *
get_must_match_header( guint id )
{
	guint i;

	for( i = 0 ; st_match_headers[i].header_id ; ++i ){
		if( st_match_headers[i].header_id == id ){
			return( gettext( st_match_headers[i].header_label ));
		}
	}

	return( "" );
}

static gboolean
get_rows_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter* iter, GSList **filters )
{
	gchar *keyword;

	gtk_tree_model_get( model, iter, ITEM_COLUMN, &keyword, -1 );
	*filters = g_slist_prepend( *filters, keyword );

	return( FALSE ); /* don't stop looping */
}

static void
insert_new_row( MatchListData *data )
{
	/* i18n notes : new filter for a new row in a match/no match list */
	static const gchar *filter_label = N_( "new-filter" );
	gchar *label;

	label = search_for_unique_label( gettext( filter_label ), data );
	insert_new_row_data( data, label, TRUE, FALSE );
	g_free( label );
}

static void
insert_new_row_data( MatchListData *data, const gchar *filter, gboolean match, gboolean not_match )
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *column;

	g_return_if_fail( !( match && not_match ));

	model = gtk_tree_view_get_model( data->listview );

	gtk_list_store_insert_with_values( GTK_LIST_STORE( model ), &iter, 0,
			ITEM_COLUMN, filter,
			MUST_MATCH_COLUMN, match,
			MUST_NOT_MATCH_COLUMN, not_match,
			-1 );

	path = gtk_tree_model_get_path( model, &iter );
	column = gtk_tree_view_get_column( data->listview, ITEM_COLUMN );
	gtk_tree_view_set_cursor( data->listview, path, column, TRUE );
	gtk_tree_path_free( path );

	if( match ){
		add_filter( data, filter, "" );
	}

	if( not_match ){
		add_filter( data, filter, "!" );
	}
}

static void
iter_for_setup( gchar *filter_orig, GtkTreeModel *model )
{
	GtkTreeIter iter;
	gchar *tmp, *filter;
	gboolean positive;
	gboolean negative;

	filter = g_strstrip( g_strdup( filter_orig ));
	positive = FALSE;
	negative = FALSE;

	if( filter[0] == '!' ){
		tmp = g_strstrip( g_strdup( filter+1 ));
		g_free( filter );
		filter = tmp;
		negative = TRUE;

	} else {
		positive = TRUE;
	}

	gtk_list_store_append( GTK_LIST_STORE( model ), &iter );
	gtk_list_store_set(
			GTK_LIST_STORE( model ),
			&iter,
			ITEM_COLUMN, filter,
			MUST_MATCH_COLUMN, positive,
			MUST_NOT_MATCH_COLUMN, negative,
			-1 );

	g_free( filter );
}

static gchar *
search_for_unique_label( const gchar *propal, MatchListData *data )
{
	gchar *label;
	guint count;

	label = g_strdup( propal );
	count = 1;

	while( count_filters( label, data ) >= 1 ){
		g_free( label );
		label = g_strdup_printf( "%s-%d", propal, ++count );
	}

	return( label );
}

static void
set_match_status( const gchar *path_str, gboolean must_match, gboolean must_not_match, MatchListData *data )
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model( data->listview );
	path = gtk_tree_path_new_from_string( path_str );
	gtk_tree_model_get_iter( model, &iter, path );
	gtk_tree_path_free( path );

	gtk_list_store_set( GTK_LIST_STORE( model ), &iter,
			MUST_MATCH_COLUMN, must_match,
			MUST_NOT_MATCH_COLUMN, must_not_match,
			-1 );
}

static void
sort_on_column( GtkTreeViewColumn *treeviewcolumn, MatchListData *data, guint new_col_id )
{
	guint prev_col_id;
	guint prev_order, new_order;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;

	prev_col_id = data->sort_column;
	prev_order = data->sort_order;

	column = gtk_tree_view_get_column( data->listview, prev_col_id );
	gtk_tree_view_column_set_sort_indicator( column, FALSE );

	if( new_col_id == prev_col_id ){
		new_order = ( prev_order == GTK_SORT_ASCENDING ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING );
	} else {
		new_order = GTK_SORT_ASCENDING;
	}

	data->sort_column = new_col_id;
	data->sort_order = new_order;

	gtk_tree_view_column_set_sort_indicator( treeviewcolumn, TRUE );
	gtk_tree_view_column_set_sort_order( treeviewcolumn, new_order );

	model = gtk_tree_view_get_model( data->listview );
	gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( model ), new_col_id, new_order );
}

static void
on_instance_finalized( MatchListData *data, BaseWindow *window )
{
	static const gchar *thisfn = "cact_match_list_on_instance_finalized";

	g_return_if_fail( data != NULL );

	g_debug( "%s: window=%p, user_data=%p, tab_name=%s",
			thisfn,
			( void * ) window,
			( void * ) data, data->tab_name );

	g_object_set_data( G_OBJECT( window ), data->tab_name, NULL );

	/* This function is called when the CactMainWindow is about to be finalized.
	 * At this time, the CactTreeModel has already been finalized.
	 * It is so too late to try to clear it...
	 */
#if 0
	GtkTreeModel *model = gtk_tree_view_get_model( data->listview );
	GtkTreeSelection *selection = gtk_tree_view_get_selection( data->listview );
	gtk_tree_selection_unselect_all( selection );
	gtk_list_store_clear( GTK_LIST_STORE( model ));
#endif

	g_free( data->tab_name );
	g_free( data->item_header );

	g_free( data );
}
