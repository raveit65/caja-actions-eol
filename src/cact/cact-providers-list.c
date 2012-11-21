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

#include <glib/gi18n.h>

#include <api/na-core-utils.h>
#include <api/na-mateconf-utils.h>
#include <api/na-object-api.h>

#include <core/na-io-provider.h>

#include "cact-application.h"
#include "base-gtk-utils.h"
#include "cact-providers-list.h"

/* column ordering
 */
enum {
	PROVIDER_READABLE_COLUMN = 0,
	PROVIDER_READABLE_MANDATORY_COLUMN,
	PROVIDER_WRITABLE_COLUMN,
	PROVIDER_WRITABLE_MANDATORY_COLUMN,
	PROVIDER_LIBELLE_COLUMN,
	PROVIDER_ID_COLUMN,
	PROVIDER_PROVIDER_COLUMN,
	PROVIDER_N_COLUMN
};

/* data attached to the treeview widget on gtk toplevel initialization
 * at this time, only treeview is set
 * at base window initialization time, the current window is associated
 */
typedef struct {
	/* set when creating the model (on_initialize_gtk_toplevel)
	 */
	GtkTreeView        *treeview;

	/* set when initializing the view (on_initialize_base_window)
	 */
	BaseWindow         *window;
	gboolean            preferences_locked;
	gboolean            editable;
}
	ProvidersListData;

/* some data needed when saving the list store
 */
typedef struct {
	GSList     *order;
}
	ProvidersListSaveData;

#define PROVIDERS_LIST_DATA				"cact-providers-list-data"
#define PROVIDERS_LIST_TREEVIEW			"cact-providers-list-treeview"

static gboolean st_on_selection_change = FALSE;

static void               init_view_setup_providers( GtkTreeView *treeview, BaseWindow *window );
static void               init_view_connect_signals( GtkTreeView *treeview, BaseWindow *window );
static void               init_view_select_first_row( GtkTreeView *treeview );

static gboolean           providers_list_save_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter* iter, ProvidersListSaveData *plsd );

static void               on_selection_changed( GtkTreeSelection *selection, BaseWindow *window );
static void               on_readable_toggled( GtkCellRendererToggle *renderer, gchar *path, BaseWindow *window );
static void               on_writable_toggled( GtkCellRendererToggle *renderer, gchar *path, BaseWindow *window );
static void               on_up_clicked( GtkButton *button, BaseWindow *window );
static void               on_down_clicked( GtkButton *button, BaseWindow *window );

static void               display_label( GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, ProvidersListData *data );
static GtkButton         *get_up_button( BaseWindow *window );
static GtkButton         *get_down_button( BaseWindow *window );
static ProvidersListData *get_providers_list_data( GtkTreeView *treeview );

/**
 * cact_providers_list_create_model:
 * @treeview: the #GtkTreeView.
 *
 * Create the treeview model when initially loading the widget from
 * the UI manager.
 */
void
cact_providers_list_create_model( GtkTreeView *treeview )
{
	static const char *thisfn = "cact_providers_list_create_model";
	GtkListStore *model;
	GtkCellRenderer *toggled_cell;
	GtkTreeViewColumn *column;
	GtkCellRenderer *text_cell;
	GtkTreeSelection *selection;
	ProvidersListData *data;

	g_return_if_fail( GTK_IS_TREE_VIEW( treeview ));

	g_debug( "%s: treeview=%p", thisfn, ( void * ) treeview );

	data = get_providers_list_data( treeview );

	model = gtk_list_store_new( PROVIDER_N_COLUMN,
			G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_OBJECT );
	gtk_tree_view_set_model( treeview, GTK_TREE_MODEL( model ));
	g_object_unref( model );

	/* readable */
	toggled_cell = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes(
			_( "Readable" ),
			toggled_cell,
			"active", PROVIDER_READABLE_COLUMN,
			NULL );
	gtk_tree_view_append_column( treeview, column );

	/* readable mandatory */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column( treeview, column );
	gtk_tree_view_column_set_visible( column, FALSE );

	/* writable */
	toggled_cell = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes(
			_( "Writable" ),
			toggled_cell,
			"active", PROVIDER_WRITABLE_COLUMN,
			NULL );
	gtk_tree_view_append_column( treeview, column );

	/* writable mandatory */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column( treeview, column );
	gtk_tree_view_column_set_visible( column, FALSE );

	/* label */
	text_cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
			_( "I/O Provider" ),
			text_cell,
			"text", PROVIDER_LIBELLE_COLUMN,
			NULL );
	gtk_tree_view_column_set_cell_data_func( column, text_cell, ( GtkTreeCellDataFunc ) display_label, data, NULL );
	gtk_tree_view_append_column( treeview, column );

	/* id */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column( treeview, column );
	gtk_tree_view_column_set_visible( column, FALSE );

	/* provider */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column( treeview, column );
	gtk_tree_view_column_set_visible( column, FALSE );

	gtk_tree_view_set_headers_visible( treeview, TRUE );

	selection = gtk_tree_view_get_selection( treeview );
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
}

/**
 * cact_providers_list_init_view:
 * @window: the parent #BaseWindow which embeds the view.
 * @treeview: the #GtkTreeView.
 *
 * Connects signals at runtime initialization of the widget, and setup
 * current default values.
 */
void
cact_providers_list_init_view( BaseWindow *window, GtkTreeView *treeview )
{
	static const gchar *thisfn = "cact_providers_list_init_view";
	ProvidersListData *data;
	CactApplication *application;
	NAUpdater *updater;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( GTK_IS_TREE_VIEW( treeview ));

	g_debug( "%s: treeview=%p, window=%p", thisfn, ( void * ) treeview, ( void * ) window );

	g_object_set_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW, treeview );

	data = get_providers_list_data( treeview );
	data->window = window;
	application = CACT_APPLICATION( base_window_get_application( window ));
	updater = cact_application_get_updater( application );
	data->preferences_locked = na_updater_are_preferences_locked( updater );

	init_view_setup_providers( treeview, window );
	init_view_connect_signals( treeview, window );

	init_view_select_first_row( treeview );
}

static void
init_view_setup_providers( GtkTreeView *treeview, BaseWindow *window )
{
	static const gchar *thisfn = "cact_providers_list_init_view_setup_providers";
	CactApplication *application;
	NAUpdater *updater;
	GtkListStore *model;
	const GList *providers;
	const GList *iter;
	GtkTreeIter row;
	gchar *id, *libelle;
	gboolean readable, readable_mandatory;
	gboolean writable, writable_mandatory;
	NAIOProvider *provider;

	model = GTK_LIST_STORE( gtk_tree_view_get_model( treeview ));

	application = CACT_APPLICATION( base_window_get_application( window ));
	updater = cact_application_get_updater( application );
	providers = na_io_provider_get_io_providers_list( NA_PIVOT( updater ));

	for( iter = providers ; iter ; iter = iter->next ){
		provider = NA_IO_PROVIDER( iter->data );
		id = na_io_provider_get_id( provider );
		libelle = na_io_provider_get_name( provider );
		readable = na_io_provider_is_conf_readable( provider, NA_PIVOT( updater ), &readable_mandatory );
		writable = na_io_provider_is_conf_writable( provider, NA_PIVOT( updater ), &writable_mandatory );

		g_debug( "%s: id=%s, readable=%s (mandatory=%s), writable=%s (mandatory=%s)",
				thisfn, id,
				readable ? "True":"False", readable_mandatory ? "True":"False",
				writable ? "True":"False", writable_mandatory ? "True":"False" );

		if( !libelle || !g_utf8_strlen( libelle, -1 )){
			g_free( libelle );

			if( na_io_provider_is_available( provider )){
				/* i18n: default name when the I/O providers doesn't provide one */
				libelle = g_strdup_printf( "<%s: %s>", id, _( "no name" ));

			} else {
				/* i18n: name displayed when the corresponding I/O provider is unavailable at runtime */
				libelle = g_strdup_printf( "<%s: %s>", id, _( "unavailable I/O provider" ));
			}
		}

		gtk_list_store_append( model, &row );
		gtk_list_store_set( model, &row,
				PROVIDER_READABLE_COLUMN, readable,
				PROVIDER_READABLE_MANDATORY_COLUMN, readable_mandatory,
				PROVIDER_WRITABLE_COLUMN, writable,
				PROVIDER_WRITABLE_MANDATORY_COLUMN, writable_mandatory,
				PROVIDER_LIBELLE_COLUMN, libelle,
				PROVIDER_ID_COLUMN, id,
				PROVIDER_PROVIDER_COLUMN, iter->data,
				-1 );

		g_free( libelle );
		g_free( id );
	}
}

static void
init_view_connect_signals( GtkTreeView *treeview, BaseWindow *window )
{
	GtkTreeViewColumn *column;
	GList *renderers;
	GtkButton *up_button, *down_button;

	column = gtk_tree_view_get_column( treeview, PROVIDER_READABLE_COLUMN );
	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
	base_window_signal_connect(
			window,
			G_OBJECT( renderers->data ),
			"toggled",
			G_CALLBACK( on_readable_toggled ));

	column = gtk_tree_view_get_column( treeview, PROVIDER_WRITABLE_COLUMN );
	renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
	base_window_signal_connect(
			window,
			G_OBJECT( renderers->data ),
			"toggled",
			G_CALLBACK( on_writable_toggled ));

	up_button = get_up_button( window );
	base_window_signal_connect(
			window,
			G_OBJECT( up_button ),
			"clicked",
			G_CALLBACK( on_up_clicked ));

	down_button = get_down_button( window );
	base_window_signal_connect(
			window,
			G_OBJECT( down_button ),
			"clicked",
			G_CALLBACK( on_down_clicked ));

	base_window_signal_connect(
			window,
			G_OBJECT( gtk_tree_view_get_selection( treeview )),
			"changed",
			G_CALLBACK( on_selection_changed ));
}

static void
init_view_select_first_row( GtkTreeView *treeview )
{
	GtkTreeSelection *selection;
	GtkTreePath *path;

	path = gtk_tree_path_new_first();
	if( path ){
		selection = gtk_tree_view_get_selection( treeview );
		gtk_tree_selection_select_path( selection, path );
		gtk_tree_path_free( path );
	}
}

/**
 * cact_providers_list_save:
 * @window: the #BaseWindow which embeds this treeview.
 *
 * Save the I/O provider status as a user preference.
 */
void
cact_providers_list_save( BaseWindow *window )
{
	static const gchar *thisfn = "cact_providers_list_save";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	ProvidersListSaveData *plsd;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	plsd = g_new0( ProvidersListSaveData, 1 );
	plsd->order = NULL;

	treeview = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW ));
	model = gtk_tree_view_get_model( treeview );
	gtk_tree_model_foreach( model, ( GtkTreeModelForeachFunc ) providers_list_save_iter, plsd );

	plsd->order = g_slist_reverse( plsd->order );
	na_settings_set_string_list( NA_IPREFS_IO_PROVIDERS_WRITE_ORDER, plsd->order );

	na_core_utils_slist_free( plsd->order );
	g_free( plsd );
}

static gboolean
providers_list_save_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter* iter, ProvidersListSaveData *plsd )
{
	gchar *id;
	gboolean readable, writable;
	NAIOProvider *provider;
	gchar *group;

	gtk_tree_model_get( model, iter,
			PROVIDER_ID_COLUMN, &id,
			PROVIDER_READABLE_COLUMN, &readable,
			PROVIDER_WRITABLE_COLUMN, &writable,
			PROVIDER_PROVIDER_COLUMN, &provider,
			-1 );

	group = g_strdup_printf( "%s %s", NA_IPREFS_IO_PROVIDER_GROUP, id );
	na_settings_set_boolean_ex( group, NA_IPREFS_IO_PROVIDER_READABLE, readable );
	na_settings_set_boolean_ex( group, NA_IPREFS_IO_PROVIDER_WRITABLE, writable );
	g_free( group );

	plsd->order = g_slist_prepend( plsd->order, g_strdup( id ));

	g_object_unref( provider );
	g_free( id );

	return( FALSE ); /* don't stop looping */
}

/**
 * cact_providers_list_dispose:
 * @treeview: the #GtkTreeView.
 *
 * Release the content of the page when we are closing the Preferences dialog.
 */
void
cact_providers_list_dispose( BaseWindow *window )
{
	static const gchar *thisfn = "cact_providers_list_dispose";
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreeSelection *selection;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	treeview = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW ));
	model = gtk_tree_view_get_model( treeview );
	selection = gtk_tree_view_get_selection( treeview );

	gtk_tree_selection_unselect_all( selection );
	gtk_list_store_clear( GTK_LIST_STORE( model ));
}

/*
 * may up/down if:
 * - preferences are not locked
 * - i/o providers order is not mandatory
 */
static void
on_selection_changed( GtkTreeSelection *selection, BaseWindow *window )
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkButton *button;
	GtkTreePath *path;
	gboolean may_up, may_down;
	gboolean order_mandatory;
	ProvidersListData *data;
	GtkTreeView *treeview;

	g_debug( "cact_providers_list_on_selection_changed: selection=%p, window=%p (%s)",
			( void * ) selection, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

	may_up = FALSE;
	may_down = FALSE;
	/* TODO */
	order_mandatory = FALSE;

	treeview = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW ));
	data = get_providers_list_data( treeview );

	if( !data->preferences_locked &&
		!order_mandatory &&
		gtk_tree_selection_get_selected( selection, &model, &iter )){

			path = gtk_tree_model_get_path( model, &iter );
			may_up = gtk_tree_path_prev( path );
			gtk_tree_path_free( path );
			may_down = gtk_tree_model_iter_next( model, &iter );
	}

	button = get_up_button( window );
	gtk_widget_set_sensitive( GTK_WIDGET( button ), may_up );

	button = get_down_button( window );
	gtk_widget_set_sensitive( GTK_WIDGET( button ), may_down );
}

/*
 * may toggle if
 * - preferences are not locked
 * - readable status is not mandatory
 */
static void
on_readable_toggled( GtkCellRendererToggle *renderer, gchar *path_string, BaseWindow *window )
{
	static const gchar *thisfn = "cact_providers_list_on_readable_toggled";
	ProvidersListData *data;
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean state;
	gboolean readable_mandatory;
	gchar *id;

	if( !st_on_selection_change ){

		treeview = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW ));
		model = gtk_tree_view_get_model( treeview );
		if( gtk_tree_model_get_iter_from_string( model, &iter, path_string )){
			gtk_tree_model_get( model, &iter,
					PROVIDER_READABLE_COLUMN, &state,
					PROVIDER_ID_COLUMN, &id,
					PROVIDER_READABLE_MANDATORY_COLUMN, &readable_mandatory,
					-1 );

			g_debug( "%s: id=%s, readable=%s (mandatory=%s)", thisfn, id,
					state ? "True":"False", readable_mandatory ? "True":"False" );

			data = get_providers_list_data( treeview );

			if( readable_mandatory || data->preferences_locked ){
				g_signal_handlers_block_by_func(( gpointer ) renderer, on_readable_toggled, window );
				state = gtk_cell_renderer_toggle_get_active( renderer );
				gtk_cell_renderer_toggle_set_active( renderer, !state );
				g_signal_handlers_unblock_by_func(( gpointer ) renderer, on_readable_toggled, window );

			} else {
				gtk_list_store_set( GTK_LIST_STORE( model ), &iter, PROVIDER_READABLE_COLUMN, !state, -1 );
			}

			g_free( id );
		}
	}
}

static void
on_writable_toggled( GtkCellRendererToggle *renderer, gchar *path_string, BaseWindow *window )
{
	static const gchar *thisfn = "cact_providers_list_on_writable_toggled";
	GtkTreeView *treeview;
	ProvidersListData *data;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean state;
	gboolean writable_mandatory;
	gchar *id;

	if( !st_on_selection_change ){

		treeview = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW ));
		model = gtk_tree_view_get_model( treeview );
		if( gtk_tree_model_get_iter_from_string( model, &iter, path_string )){
			gtk_tree_model_get( model, &iter,
					PROVIDER_WRITABLE_COLUMN, &state,
					PROVIDER_ID_COLUMN, &id,
					PROVIDER_WRITABLE_MANDATORY_COLUMN, &writable_mandatory,
					-1 );

			g_debug( "%s: id=%s, writable=%s (mandatory=%s)", thisfn, id,
					state ? "True":"False", writable_mandatory ? "True":"False" );

			data = get_providers_list_data( treeview );

			if( writable_mandatory || data->preferences_locked ){
				g_signal_handlers_block_by_func(( gpointer ) renderer, on_writable_toggled, window );
				state = gtk_cell_renderer_toggle_get_active( renderer );
				gtk_cell_renderer_toggle_set_active( renderer, !state );
				g_signal_handlers_unblock_by_func(( gpointer ) renderer, on_writable_toggled, window );

			} else {
				gtk_list_store_set( GTK_LIST_STORE( model ), &iter, PROVIDER_WRITABLE_COLUMN, !state, -1 );
			}

			g_free( id );
		}
	}
}

/*
 * may up/down if:
 * - preferences are not locked
 * - i/o providers order is not mandatory
 */
static void
on_up_clicked( GtkButton *button, BaseWindow *window )
{
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter_selected;
	GtkTreePath *path_prev;
	GtkTreeIter iter_prev;

	treeview = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW ));
	selection = gtk_tree_view_get_selection( treeview );
	if( gtk_tree_selection_get_selected( selection, &model, &iter_selected )){
		path_prev = gtk_tree_model_get_path( model, &iter_selected );
		if( gtk_tree_path_prev( path_prev )){
			if( gtk_tree_model_get_iter( model, &iter_prev, path_prev )){
				gtk_list_store_move_before( GTK_LIST_STORE( model ), &iter_selected, &iter_prev );
				gtk_tree_selection_unselect_all( selection );
				gtk_tree_selection_select_path( selection, path_prev );
			}
		}
		gtk_tree_path_free( path_prev );
	}
}

static void
on_down_clicked( GtkButton *button, BaseWindow *window )
{
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter_selected;
	GtkTreeIter *iter_next;
	GtkTreePath *path_next;

	treeview = GTK_TREE_VIEW( g_object_get_data( G_OBJECT( window ), PROVIDERS_LIST_TREEVIEW ));
	selection = gtk_tree_view_get_selection( treeview );
	if( gtk_tree_selection_get_selected( selection, &model, &iter_selected )){
		iter_next = gtk_tree_iter_copy( &iter_selected );
		if( gtk_tree_model_iter_next( model, iter_next )){
			path_next = gtk_tree_model_get_path( model, iter_next );
			gtk_list_store_move_after( GTK_LIST_STORE( model ), &iter_selected, iter_next );
			gtk_tree_selection_unselect_all( selection );
			gtk_tree_selection_select_path( selection, path_next );
			gtk_tree_path_free( path_next );
		}
		gtk_tree_iter_free( iter_next );
	}
}

/*
 * unavailable provider: italic grey
 */
static void
display_label( GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, ProvidersListData *data )
{
	NAIOProvider *provider;
	gchar *name;

	gtk_tree_model_get( model, iter, PROVIDER_LIBELLE_COLUMN, &name, PROVIDER_PROVIDER_COLUMN, &provider, -1 );

	g_object_set( cell, "style-set", FALSE, NULL );
	g_object_set( cell, "foreground-set", FALSE, NULL );

	if( !na_io_provider_is_available( provider )){
		g_object_set( cell, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL );
		g_object_set( cell, "foreground", "grey", "foreground-set", TRUE, NULL );
	}

	g_object_unref( provider );

	g_object_set( cell, "text", name, NULL );

	if( data->preferences_locked ){
		g_object_set( cell, "foreground", "grey", "foreground-set", TRUE, NULL );
	}

	g_free( name );
}

static GtkButton *
get_up_button( BaseWindow *window )
{
	GtkButton *button;

	button = GTK_BUTTON( base_window_get_widget( window, "ProviderButtonUp" ));

	return( button );
}

static GtkButton *
get_down_button( BaseWindow *window )
{
	GtkButton *button;

	button = GTK_BUTTON( base_window_get_widget( window, "ProviderButtonDown" ));

	return( button );
}

static ProvidersListData *
get_providers_list_data( GtkTreeView *treeview )
{
	ProvidersListData *data;

	data = ( ProvidersListData * ) g_object_get_data( G_OBJECT( treeview ), PROVIDERS_LIST_DATA );

	if( data == NULL ){
		data = g_new0( ProvidersListData, 1 );
		g_object_set_data( G_OBJECT( treeview ), PROVIDERS_LIST_DATA, data );
		data->treeview = treeview;
	}

	return( data );
}
