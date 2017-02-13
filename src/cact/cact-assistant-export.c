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
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-exporter.h>
#include <core/na-export-format.h>
#include <core/na-gtk-utils.h>
#include <core/na-ioptions-list.h>

#include "cact-application.h"
#include "cact-main-window.h"
#include "cact-assistant-export.h"
#include "cact-export-ask.h"
#include "cact-tree-view.h"

/* Export Assistant
 *
 * pos.  type     title
 * ---   -------  ------------------------------------
 *   0   Intro    Introduction
 *   1   Content  Selection of the actions
 *   2   Content  Selection of the target folder
 *   3   Content  Selection of the export format
 *   4   Confirm  Summary of the operations to be done
 *   5   Summary  Export is done: summary of the done operations
 */

enum {
	ASSIST_PAGE_INTRO = 0,
	ASSIST_PAGE_ACTIONS_SELECTION,
	ASSIST_PAGE_FOLDER_SELECTION,
	ASSIST_PAGE_FORMAT_SELECTION,
	ASSIST_PAGE_CONFIRM,
	ASSIST_PAGE_DONE
};

/* private class data
 */
struct _CactAssistantExportClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CactAssistantExportPrivate {
	gboolean      dispose_has_run;
	CactTreeView *items_view;
	gboolean      preferences_locked;
	gchar        *uri;
	GList        *selected_items;
	GList        *results;
};

typedef struct {
	NAObjectItem *item;
	GSList       *msg;
	gchar        *fname;
	gchar        *format;
}
	ExportStruct;

static const gchar        *st_xmlui_filename = PKGUIDIR "/cact-assistant-export.ui";
static const gchar        *st_toplevel_name  = "ExportAssistant";
static const gchar        *st_wsp_name       = NA_IPREFS_EXPORT_ASSISTANT_WSP;

static BaseAssistantClass *st_parent_class   = NULL;

static GType      register_type( void );
static void       class_init( CactAssistantExportClass *klass );
static void       ioptions_list_iface_init( NAIOptionsListInterface *iface, void *user_data );
static GList     *ioptions_list_get_formats( const NAIOptionsList *instance, GtkWidget *container );
static void       ioptions_list_free_formats( const NAIOptionsList *instance, GtkWidget *container, GList *formats );
static NAIOption *ioptions_list_get_ask_option( const NAIOptionsList *instance, GtkWidget *container );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_constructed( GObject *instance );
static void       instance_dispose( GObject *instance );
static void       instance_finalize( GObject *instance );
static void       on_base_initialize_gtk_toplevel( CactAssistantExport *window, GtkAssistant *toplevel, gpointer user_data );
static void       items_tree_view_initialize_gtk( CactAssistantExport *window, GtkAssistant *toplevel );
static void       folder_chooser_initialize_gtk( CactAssistantExport *window );
static void       format_tree_view_initialize_gtk( CactAssistantExport *window );
static void       on_base_initialize_base_window( CactAssistantExport *window, gpointer user_data );
static void       on_base_all_widgets_showed( CactAssistantExport *window, gpointer user_data );
static void       on_items_tree_view_selection_changed( CactAssistantExport *window, GList *selected_items, gpointer user_data );
static void       on_folder_chooser_selection_changed( GtkFileChooser *chooser, CactAssistantExport *window );
static void       assistant_prepare( BaseAssistant *window, GtkAssistant *assistant, GtkWidget *page );
static void       assist_prepare_confirm( CactAssistantExport *window, GtkAssistant *assistant, GtkWidget *page );
static void       assistant_apply( BaseAssistant *window, GtkAssistant *assistant );
static void       assist_prepare_exportdone( CactAssistantExport *window, GtkAssistant *assistant, GtkWidget *page );
static void       free_results( GList *list );

GType
cact_assistant_export_get_type( void )
{
	static GType window_type = 0;

	if( !window_type ){
		window_type = register_type();
	}

	return( window_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_assistant_export_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactAssistantExportClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactAssistantExport ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo ioptions_list_iface_info = {
		( GInterfaceInitFunc ) ioptions_list_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_TYPE_ASSISTANT, "CactAssistantExport", &info, 0 );

	g_type_add_interface_static( type, NA_TYPE_IOPTIONS_LIST, &ioptions_list_iface_info );

	return( type );
}

static void
class_init( CactAssistantExportClass *klass )
{
	static const gchar *thisfn = "cact_assistant_export_class_init";
	GObjectClass *object_class;
	BaseAssistantClass *assist_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->constructed = instance_constructed;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactAssistantExportClassPrivate, 1 );

	assist_class = BASE_ASSISTANT_CLASS( klass );
	assist_class->apply = assistant_apply;
	assist_class->prepare = assistant_prepare;
}

static void
ioptions_list_iface_init( NAIOptionsListInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_assistant_export_ioptions_list_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->get_options = ioptions_list_get_formats;
	iface->free_options = ioptions_list_free_formats;
	iface->get_ask_option = ioptions_list_get_ask_option;
}

static GList *
ioptions_list_get_formats( const NAIOptionsList *instance, GtkWidget *container )
{
	CactAssistantExport *window;
	CactApplication *application;
	NAUpdater *updater;
	GList *formats;

	g_return_val_if_fail( CACT_IS_ASSISTANT_EXPORT( instance ), NULL );
	window = CACT_ASSISTANT_EXPORT( instance );

	application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = cact_application_get_updater( application );
	formats = na_exporter_get_formats( NA_PIVOT( updater ));

	return( formats );
}

static void
ioptions_list_free_formats( const NAIOptionsList *instance, GtkWidget *container, GList *formats )
{
	na_exporter_free_formats( formats );
}

static NAIOption *
ioptions_list_get_ask_option( const NAIOptionsList *instance, GtkWidget *container )
{
	return( na_exporter_get_ask_option());
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_assistant_export_instance_init";
	CactAssistantExport *self;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_ASSISTANT_EXPORT( instance );

	self->private = g_new0( CactAssistantExportPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_constructed( GObject *window )
{
	static const gchar *thisfn = "cact_assistant_export_instance_constructed";
	CactAssistantExportPrivate *priv;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	priv = CACT_ASSISTANT_EXPORT( window )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( window );
		}

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				BASE_SIGNAL_INITIALIZE_GTK,
				G_CALLBACK( on_base_initialize_gtk_toplevel ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				BASE_SIGNAL_INITIALIZE_WINDOW,
				G_CALLBACK( on_base_initialize_base_window ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				BASE_SIGNAL_SHOW_WIDGETS,
				G_CALLBACK( on_base_all_widgets_showed ));
	}
}

static void
instance_dispose( GObject *window )
{
	static const gchar *thisfn = "cact_assistant_export_instance_dispose";
	CactAssistantExport *self;
	GtkAssistant *assistant;
	GtkWidget *page;
	guint pos;
	GtkWidget *pane;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	self = CACT_ASSISTANT_EXPORT( window );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		self->private->dispose_has_run = TRUE;

		g_object_unref( self->private->items_view );

		if( self->private->selected_items ){
			self->private->selected_items = na_object_free_items( self->private->selected_items );
		}

		assistant = GTK_ASSISTANT( base_window_get_gtk_toplevel( BASE_WINDOW( window )));
		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_ACTIONS_SELECTION );
		pane = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p1-HPaned" );
		pos = gtk_paned_get_position( GTK_PANED( pane ));
		na_settings_set_uint( NA_IPREFS_EXPORT_ASSISTANT_PANED, pos );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( window );
		}
	}
}

static void
instance_finalize( GObject *window )
{
	static const gchar *thisfn = "cact_assistant_export_instance_finalize";
	CactAssistantExport *self;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

	self = CACT_ASSISTANT_EXPORT( window );

	free_results( self->private->results );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( window );
	}
}

/**
 * Run the assistant.
 *
 * @main: the main window of the application.
 */
void
cact_assistant_export_run( BaseWindow *main_window )
{
	CactAssistantExport *assistant;
	gboolean esc_quit, esc_confirm;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( main_window ));

	esc_quit = na_settings_get_boolean( NA_IPREFS_ASSISTANT_ESC_QUIT, NULL, NULL );
	esc_confirm = na_settings_get_boolean( NA_IPREFS_ASSISTANT_ESC_CONFIRM, NULL, NULL );

	assistant = g_object_new( CACT_TYPE_ASSISTANT_EXPORT,
			BASE_PROP_PARENT,          main_window,
			BASE_PROP_HAS_OWN_BUILDER, TRUE,
			BASE_PROP_XMLUI_FILENAME,  st_xmlui_filename,
			BASE_PROP_TOPLEVEL_NAME,   st_toplevel_name,
			BASE_PROP_WSP_NAME,        st_wsp_name,
			BASE_PROP_QUIT_ON_ESCAPE,  esc_quit,
			BASE_PROP_WARN_ON_ESCAPE,  esc_confirm,
			NULL );

	base_window_run( BASE_WINDOW( assistant ));
}

static void
on_base_initialize_gtk_toplevel( CactAssistantExport *window, GtkAssistant *assistant, gpointer user_data )
{
	static const gchar *thisfn = "cact_assistant_export_on_base_initialize_gtk_toplevel";
	gboolean are_locked, mandatory;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	if( !window->private->dispose_has_run ){
		g_debug( "%s: window=%p, assistant=%p, user_data=%p",
				thisfn, ( void * ) window, ( void * ) assistant, ( void * ) user_data );

		items_tree_view_initialize_gtk( window, assistant );
		folder_chooser_initialize_gtk( window );
		format_tree_view_initialize_gtk( window );

		are_locked = na_settings_get_boolean( NA_IPREFS_ADMIN_PREFERENCES_LOCKED, NULL, &mandatory );
		window->private->preferences_locked = are_locked && mandatory;

	}
}

static void
items_tree_view_initialize_gtk( CactAssistantExport *window, GtkAssistant *assistant )
{
	static const gchar *thisfn = "cact_assistant_export_items_tree_view_initialize_gtk";
	GtkWidget *page;

	g_debug( "%s: window=%p, assistant=%p", thisfn, ( void * ) window, ( void * ) assistant );

	page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_ACTIONS_SELECTION );

	window->private->items_view =
			cact_tree_view_new(
					BASE_WINDOW( window ),
					GTK_CONTAINER( page ),
					"ActionsList",
					TREE_MODE_SELECTION );
}

static void
folder_chooser_initialize_gtk( CactAssistantExport *window )
{
	static const gchar *thisfn = "cact_assistant_export_folder_chooser_initialize_gtk";
	GtkAssistant *assistant;
	GtkWidget *page, *chooser;
	gchar *uri;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	assistant = GTK_ASSISTANT( base_window_get_gtk_toplevel( BASE_WINDOW( window )));
	page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_FOLDER_SELECTION );
	chooser = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p2-ExportFolderChooser" );

	gtk_file_chooser_set_action( GTK_FILE_CHOOSER( chooser ), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER );
	gtk_file_chooser_set_create_folders( GTK_FILE_CHOOSER( chooser ), TRUE );
	gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( chooser ), FALSE );

	uri = na_settings_get_string( NA_IPREFS_EXPORT_ASSISTANT_URI, NULL, NULL );
	if( uri && strlen( uri )){
		gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( chooser ), uri );
	}
	g_free( uri );

	base_window_signal_connect(
			BASE_WINDOW( window ),
			G_OBJECT( chooser ),
			"selection-changed",
			G_CALLBACK( on_folder_chooser_selection_changed ));
}

static void
format_tree_view_initialize_gtk( CactAssistantExport *window )
{
	static const gchar *thisfn = "cact_assistant_export_on_format_tree_view_realized";
	GtkAssistant *assistant;
	GtkWidget *page, *tree_view;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	assistant = GTK_ASSISTANT( base_window_get_gtk_toplevel( BASE_WINDOW( window )));
	page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_FORMAT_SELECTION );
	tree_view = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p3-ExportFormatTreeView" );

	na_ioptions_list_gtk_init( NA_IOPTIONS_LIST( window ), tree_view, TRUE );

	gtk_assistant_set_page_complete( assistant, page, TRUE );
}

static void
on_base_initialize_base_window( CactAssistantExport *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_assistant_export_on_base_initialize_base_window";
	GtkAssistant *assistant;
	GtkWidget *page;
	guint pos;
	GtkWidget *pane;
	gchar *format;
	gboolean mandatory;
	GtkWidget *tree_view;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	if( !window->private->dispose_has_run ){
		g_debug( "%s: window=%p, user_data=%p", thisfn, ( void * ) window, ( void * ) user_data );

		assistant = GTK_ASSISTANT( base_window_get_gtk_toplevel( BASE_WINDOW( window )));

		/* intro page is always true
		 */
		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_INTRO );
		gtk_assistant_set_page_complete( assistant, page, TRUE );

		/* set the slider position of the item selection page
		 */
		pos = na_settings_get_uint( NA_IPREFS_EXPORT_ASSISTANT_PANED, NULL, NULL );
		if( pos ){
			page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_ACTIONS_SELECTION );
			pane = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p1-HPaned" );
			gtk_paned_set_position( GTK_PANED( pane ), pos );
		}

		/* initialize the export format tree view
		 */
		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_FORMAT_SELECTION );
		tree_view = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p3-ExportFormatTreeView" );
		format = na_settings_get_string( NA_IPREFS_EXPORT_PREFERRED_FORMAT, NULL, &mandatory );
		na_ioptions_list_set_editable(
				NA_IOPTIONS_LIST( window ), tree_view,
				!mandatory && !window->private->preferences_locked );
		na_ioptions_list_set_default(
				NA_IOPTIONS_LIST( window ), tree_view, format );
		g_free( format );
	}
}

static void
on_base_all_widgets_showed( CactAssistantExport *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_assistant_export_on_base_all_widgets_showed";
	GtkAssistant *assistant;
	GtkWidget *page;
	CactMainWindow *main_window;
	CactTreeView *main_items_view;
	GList *items;
	GtkTreePath *path;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	if( !window->private->dispose_has_run ){
		g_debug( "%s: window=%p, user_data=%p", thisfn, ( void * ) window, ( void * ) user_data );

		assistant = GTK_ASSISTANT( base_window_get_gtk_toplevel( BASE_WINDOW( window )));

		/* fill up the items tree view
		 */
		main_window = CACT_MAIN_WINDOW( base_window_get_parent( BASE_WINDOW( window )));
		main_items_view = cact_main_window_get_items_view( main_window );
		items = cact_tree_view_get_items( main_items_view );
		cact_tree_view_fill( window->private->items_view, items );

		/* connect to the 'selection-changed' signal emitted by CactTreeView
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				MAIN_SIGNAL_SELECTION_CHANGED,
				G_CALLBACK( on_items_tree_view_selection_changed ));

		/* select the first row
		 */
		path = gtk_tree_path_new_from_string( "0" );
		cact_tree_view_select_row_at_path( window->private->items_view, path );
		gtk_tree_path_free( path );

		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_ACTIONS_SELECTION );
		gtk_widget_show_all( page );

		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_FOLDER_SELECTION );
		gtk_widget_show_all( page );

		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_FORMAT_SELECTION );
		gtk_widget_show_all( page );
	}
}

static void
on_items_tree_view_selection_changed( CactAssistantExport *window, GList *selected_items, gpointer user_data )
{
	static const gchar *thisfn = "cact_assistant_export_on_items_tree_view_selection_changed";
	GtkAssistant *assistant;
	GtkWidget *page;
	gboolean enabled;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p, selected_items=%p (count=%d), user_data=%p",
			thisfn, ( void * ) window,
			( void * ) selected_items, g_list_length( selected_items ), ( void * ) user_data );

		if( window->private->selected_items ){
			window->private->selected_items = na_object_free_items( window->private->selected_items );
		}

		enabled = ( g_list_length( selected_items ) > 0 );
		window->private->selected_items = na_object_copyref_items( selected_items );

		assistant = GTK_ASSISTANT( base_window_get_gtk_toplevel( BASE_WINDOW( window )));
		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_ACTIONS_SELECTION );
		gtk_assistant_set_page_complete( assistant, page, enabled );
	}
}

/*
 * we check the selected uri for writability
 * this is always subject to become invalid before actually writing
 * but this is better than nothing, doesn't ?
 *
 * NB: if the GtkFileChooser widget automatically opens on another mode that
 * 'file system' (e.g. search or recently used), it may be because of
 * ~/.config/gtk-2.0/gtkfilechooser.ini file -> to be checked
 * (maybe removed)
 */
static void
on_folder_chooser_selection_changed( GtkFileChooser *chooser, CactAssistantExport *window )
{
	static const gchar *thisfn = "cact_assistant_export_on_folder_chooser_selection_changed";
	GtkAssistant *assistant;
	GtkWidget *page;
	gchar *uri;
	gboolean enabled;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( window ));

	if( !window->private->dispose_has_run ){
		g_debug( "%s: chooser=%p, window=%p", thisfn, ( void * ) chooser, ( void * ) window );

		uri = gtk_file_chooser_get_current_folder_uri( chooser );
		g_debug( "%s: uri=%s", thisfn, uri );
		enabled = ( uri && strlen( uri ) && na_core_utils_dir_is_writable_uri( uri ));

		if( enabled ){
			g_free( window->private->uri );
			window->private->uri = g_strdup( uri );
			na_settings_set_string( NA_IPREFS_EXPORT_ASSISTANT_URI, uri );
		}

		g_free( uri );

		assistant = GTK_ASSISTANT( base_window_get_gtk_toplevel( BASE_WINDOW( window )));
		page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_FOLDER_SELECTION );
		gtk_assistant_set_page_complete( assistant, page, enabled );
	}
}

static void
assistant_prepare( BaseAssistant *window, GtkAssistant *assistant, GtkWidget *page )
{
	static const gchar *thisfn = "cact_assistant_export_on_prepare";

	g_debug( "%s: window=%p, assistant=%p, page=%p", thisfn, window, assistant, page );

	GtkAssistantPageType type = gtk_assistant_get_page_type( assistant, page );

	switch( type ){
		case GTK_ASSISTANT_PAGE_CONFIRM:
			assist_prepare_confirm( CACT_ASSISTANT_EXPORT( window ), assistant, page );
			break;

		case GTK_ASSISTANT_PAGE_SUMMARY:
			assist_prepare_exportdone( CACT_ASSISTANT_EXPORT( window ), assistant, page );
			break;

		default:
			break;
	}
}

static void
assist_prepare_confirm( CactAssistantExport *window, GtkAssistant *assistant, GtkWidget *page )
{
	static const gchar *thisfn = "cact_assistant_export_prepare_confirm";
	gchar *text, *tmp;
	gchar *label_item;
	gchar *format_label, *format_label2;
	gchar *format_description, *format_description2;
	gchar *format_id;
	GtkWidget *label;
	NAIOption *format;
	GList *it;
	GtkWidget *format_page, *tree_view;

	g_debug( "%s: window=%p, assistant=%p, page=%p",
			thisfn, ( void * ) window, ( void * ) assistant, ( void * ) page );

	/* display the items to be exported
	 */
	text = NULL;
	for( it = window->private->selected_items ; it ; it = it->next ){
		label_item = na_object_get_label( it->data );
		if( text ){
			tmp = g_strdup_printf( "%s\n%s", text, label_item );
			g_free( text );
			text = tmp;
		} else {
			text = g_strdup( label_item );
		}
		g_free( label_item );
	}
	label = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p4-ConfirmItemsList" );
	g_return_if_fail( GTK_IS_LABEL( label ));
	gtk_label_set_text( GTK_LABEL( label ), text );
	g_free( text );

	/* display the target folder
	 */
	g_return_if_fail( window->private->uri && strlen( window->private->uri ));
	label = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p4-ConfirmTargetFolder" );
	g_return_if_fail( GTK_IS_LABEL( label ));
	gtk_label_set_text( GTK_LABEL( label ), window->private->uri );

	/* display the export format and its description
	 */
	format_page = gtk_assistant_get_nth_page( assistant, ASSIST_PAGE_FORMAT_SELECTION );
	tree_view = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( format_page ), "p3-ExportFormatTreeView" );
	g_return_if_fail( GTK_IS_TREE_VIEW( tree_view ));
	format = na_ioptions_list_get_selected( NA_IOPTIONS_LIST( window ), tree_view );
	g_return_if_fail( NA_IS_EXPORT_FORMAT( format ));

	format_label = na_ioption_get_label( format );
	format_label2 = na_core_utils_str_remove_char( format_label, "_" );
	text = g_strdup_printf( "%s:", format_label2 );
	label = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p4-ConfirmExportFormat" );
	g_return_if_fail( GTK_IS_LABEL( label ));
	gtk_label_set_text( GTK_LABEL( label ), text );
	g_free( format_label );
	g_free( format_label2 );
	g_free( text );

	format_description = na_ioption_get_description( format );
	format_description2 = na_core_utils_str_remove_char( format_description, "_" );
	label = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p4-ConfirmExportTooltip" );
	g_return_if_fail( GTK_IS_LABEL( label ));
	gtk_label_set_text( GTK_LABEL( label ), format_description2 );
	g_free( format_description );
	g_free( format_description2 );

	format_id = na_ioption_get_id( format );
	na_settings_set_string( NA_IPREFS_EXPORT_PREFERRED_FORMAT, format_id );
	g_free( format_id );

	gtk_assistant_set_page_complete( assistant, page, TRUE );
}

/*
 * As of 1.11, cact_mateconf_writer doesn't return any error message.
 * An error is simply indicated by returning a null filename.
 * So we provide a general error message.
 */
static void
assistant_apply( BaseAssistant *wnd, GtkAssistant *assistant )
{
	static const gchar *thisfn = "cact_assistant_export_on_apply";
	CactAssistantExport *window;
	GList *ia;
	ExportStruct *str;
	CactApplication *application;
	NAUpdater *updater;
	gboolean first;

	g_return_if_fail( CACT_IS_ASSISTANT_EXPORT( wnd ));

	g_debug( "%s: window=%p, assistant=%p", thisfn, ( void * ) wnd, ( void * ) assistant );

	window = CACT_ASSISTANT_EXPORT( wnd );

	application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = cact_application_get_updater( application );
	first = TRUE;

	g_return_if_fail( window->private->uri && strlen( window->private->uri ));

	for( ia = window->private->selected_items ; ia ; ia = ia->next ){
		str = g_new0( ExportStruct, 1 );
		window->private->results = g_list_append( window->private->results, str );

		str->item = NA_OBJECT_ITEM( na_object_get_origin( NA_IDUPLICABLE( ia->data )));
		str->format = na_settings_get_string( NA_IPREFS_EXPORT_PREFERRED_FORMAT, NULL, NULL );
		g_return_if_fail( str->format && strlen( str->format ));

		if( !strcmp( str->format, EXPORTER_FORMAT_ASK )){
			g_free( str->format );
			str->format = cact_export_ask_user( BASE_WINDOW( wnd ), str->item, first );
			g_return_if_fail( str->format && strlen( str->format ));

			if( !str->format || !strcmp( str->format, EXPORTER_FORMAT_NOEXPORT )){
				str->msg = g_slist_append( NULL, g_strdup( _( "Export canceled due to user action." )));
			}
		}

		if( strcmp( str->format, EXPORTER_FORMAT_NOEXPORT ) != 0 ){
			str->fname = na_exporter_to_file( NA_PIVOT( updater ), str->item, window->private->uri, str->format, &str->msg );
		}

		g_free( str->format );
		first = FALSE;
	}
}

static void
assist_prepare_exportdone( CactAssistantExport *window, GtkAssistant *assistant, GtkWidget *page )
{
	static const gchar *thisfn = "cact_assistant_export_prepare_exportdone";
	gint errors;
	guint width;
	GtkWidget *vbox;
	GtkWidget *item_vbox;
	const gchar *color;
	gchar *item_label;
	gchar *text, *tmp;
	GtkWidget *label;
	GSList *is;
	GList *ir;
	ExportStruct *str;

	g_debug( "%s: window=%p, assistant=%p, page=%p",
			thisfn, ( void * ) window, ( void * ) assistant, ( void * ) page );

	errors = 0;
	width = 15;
	vbox = na_gtk_utils_find_widget_by_name( GTK_CONTAINER( page ), "p5-SummaryVBox" );
	g_return_if_fail( GTK_IS_BOX( vbox ));

	/* for each item:
	 * - display the item label
	 * - display the export filename
	 */
	for( ir = window->private->results ; ir ; ir = ir->next ){

		item_vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 4 );
		gtk_box_pack_start( GTK_BOX( vbox ), item_vbox, FALSE, FALSE, 0 );

		/* display the item label
		 */
		str = ( ExportStruct * ) ir->data;
		color = str->fname ? "blue" : "red";
		item_label = na_object_get_label( str->item );
		text = g_markup_printf_escaped( "<span foreground=\"%s\">%s</span>", color, item_label );
		label = gtk_label_new( NULL );
		gtk_label_set_markup( GTK_LABEL( label ), text );
		g_free( item_label );
		g_free( text );
		g_object_set( G_OBJECT( label ), "xalign", 0, NULL );
		g_object_set( G_OBJECT( label ), "xpad", width, NULL );
		gtk_box_pack_start( GTK_BOX( item_vbox ), label, FALSE, FALSE, 0 );

		/* display the process log
		 */
		text = NULL;
		if( str->fname ){
			/* i18n: action as been successfully exported to <filename> */
			text = g_strdup_printf( "%s %s", _( "Successfully exported as" ), str->fname );

		} else if( strcmp( str->format, EXPORTER_FORMAT_NOEXPORT ) != 0 ){
			errors += 1;
		}

		/* add messages if any
		 */
		for( is = str->msg ; is ; is = is->next ){
			if( text ){
				tmp = g_strdup_printf( "%s\n%s", text, ( gchar * ) is->data );
				g_free( text );
				text = tmp;
			} else {
				text = g_strdup(( gchar * ) is->data );
			}
		}

		label = gtk_label_new( text );
		g_free( text );
		gtk_label_set_line_wrap( GTK_LABEL( label ), TRUE );
		gtk_label_set_line_wrap_mode( GTK_LABEL( label ), PANGO_WRAP_WORD );
		g_object_set( G_OBJECT( label ), "xalign", 0, NULL );
		g_object_set( G_OBJECT( label ), "xpad", 2*width, NULL );
		gtk_box_pack_start( GTK_BOX( item_vbox ), label, FALSE, FALSE, 0 );
	}

	if( errors ){
		text = g_strdup_printf( "%s",
				_( "You may not have write permissions on selected folder." ));
		label = gtk_label_new( text );
		g_free( text );
		gtk_label_set_line_wrap( GTK_LABEL( label ), TRUE );
		gtk_label_set_line_wrap_mode( GTK_LABEL( label ), PANGO_WRAP_WORD );
		g_object_set( G_OBJECT( label ), "xalign", 0, NULL );
		g_object_set( G_OBJECT( label ), "xpad", width, NULL );
		gtk_box_pack_start( GTK_BOX( item_vbox ), label, FALSE, FALSE, 0 );
	}

	gtk_assistant_set_page_complete( assistant, page, TRUE );
	g_object_set( G_OBJECT( window ), BASE_PROP_WARN_ON_ESCAPE, FALSE, NULL );
	gtk_widget_show_all( page );
}

static void
free_results( GList *list )
{
	GList *ir;
	ExportStruct *str;

	for( ir = list ; ir ; ir = ir->next ){
		str = ( ExportStruct * ) ir->data;
		g_free( str->fname );
		na_core_utils_slist_free( str->msg );
	}

	g_list_free( list );
}
