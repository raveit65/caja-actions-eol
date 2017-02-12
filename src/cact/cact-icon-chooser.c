/*
 * Caja Actions
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

#include <core/na-gtk-utils.h>

#include "base-keysyms.h"
#include "cact-application.h"
#include "base-gtk-utils.h"
#include "cact-icon-chooser.h"

/* private class data
 */
struct _CactIconChooserClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CactIconChooserPrivate {
	gboolean      dispose_has_run;
	BaseWindow   *main_window;
	const gchar  *initial_icon;
	gchar        *current_icon;
	GtkWidget    *path_preview;
};

#define VIEW_ICON_SIZE					GTK_ICON_SIZE_DND
#define VIEW_ICON_DEFAULT_WIDTH			32	/* width of the GTK_ICON_SIZE_DND icon size */
#define PREVIEW_ICON_SIZE				GTK_ICON_SIZE_DIALOG
#define PREVIEW_ICON_WIDTH				64
#define CURRENT_ICON_SIZE				GTK_ICON_SIZE_DIALOG

/* column ordering in the Stock model
 */
enum {
	STOCK_NAME_COLUMN = 0,
	STOCK_LABEL_COLUMN,
	STOCK_PIXBUF_COLUMN,
	STOCK_N_COLUMN
};

/* column ordering in the ThemeContext model
 * this is the list store on the left which lets the user select the context
 */
enum {
	THEME_CONTEXT_LABEL_COLUMN = 0,
	THEME_CONTEXT_STORE_COLUMN,
	THEME_CONTEXT_LAST_SELECTED_COLUMN,
	THEME_CONTEXT_N_COLUMN
};

/* column ordering in the ThemeIconView model
 * foreach selected context, we display in the icon view the list of
 * corresponding icons
 */
enum {
	THEME_ICON_LABEL_COLUMN = 0,
	THEME_ICON_PIXBUF_COLUMN,
	THEME_ICON_N_COLUMN
};

static const gchar     *st_xmlui_filename = PKGUIDIR "/cact-icon-chooser.ui";
static const gchar     *st_toplevel_name  = "IconChooserDialog";
static const gchar     *st_wsp_name       = NA_IPREFS_ICON_CHOOSER_WSP;

static BaseDialogClass *st_parent_class   = NULL;

static GType         register_type( void );
static void          class_init( CactIconChooserClass *klass );
static void          instance_init( GTypeInstance *instance, gpointer klass );
static void          instance_constructed( GObject *dialog );
static void          instance_dispose( GObject *dialog );
static void          instance_finalize( GObject *dialog );

static void          on_base_initialize_gtk( CactIconChooser *editor, GtkDialog *toplevel, gpointer user_data );
static void          do_initialize_themed_icons( CactIconChooser *editor );
static void          do_initialize_icons_by_path( CactIconChooser *editor );
static void          on_base_initialize_window( CactIconChooser *editor, gpointer user_data );
static void          fillup_themed_icons( CactIconChooser *editor );
static void          fillup_icons_by_path( CactIconChooser *editor );
static void          on_base_show_widgets( CactIconChooser *editor, gpointer user_data );

static void          on_cancel_clicked( GtkButton *button, CactIconChooser *editor );
static void          on_ok_clicked( GtkButton *button, CactIconChooser *editor );
static void          on_dialog_cancel( BaseDialog *dialog );

static void          on_current_icon_changed( const CactIconChooser *editor );
static gboolean      on_destroy( GtkWidget *widget, GdkEvent *event, void *foo );
static gboolean      on_icon_view_button_press_event( GtkWidget *widget, GdkEventButton *event, CactIconChooser *editor );
static gboolean      on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, CactIconChooser *editor );
static void          on_themed_context_changed( GtkTreeSelection *selection, CactIconChooser *editor );
static void          on_themed_icon_changed( GtkIconView *icon_view, CactIconChooser *editor );
static void          on_themed_apply_button_clicked( GtkButton *button, CactIconChooser *editor );
static void          on_themed_apply_triggered( CactIconChooser *editor );
static void          on_path_selection_changed( GtkFileChooser *chooser, CactIconChooser *editor );
static void          on_path_update_preview( GtkFileChooser *chooser, CactIconChooser *editor );
static void          on_path_apply_button_clicked( GtkButton *button, CactIconChooser *editor );
static GtkListStore *theme_context_load_icons( CactIconChooser *editor, const gchar *context );

GType
cact_icon_chooser_get_type( void )
{
	static GType dialog_type = 0;

	if( !dialog_type ){
		dialog_type = register_type();
	}

	return( dialog_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_icon_chooser_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactIconChooserClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactIconChooser ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_TYPE_DIALOG, "CactIconChooser", &info, 0 );

	return( type );
}

static void
class_init( CactIconChooserClass *klass )
{
	static const gchar *thisfn = "cact_icon_chooser_class_init";
	GObjectClass *object_class;
	BaseDialogClass *dialog_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->constructed = instance_constructed;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactIconChooserClassPrivate, 1 );

	dialog_class = BASE_DIALOG_CLASS( klass );
	dialog_class->cancel = on_dialog_cancel;
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_icon_chooser_instance_init";
	CactIconChooser *self;

	g_return_if_fail( CACT_IS_ICON_CHOOSER( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_ICON_CHOOSER( instance );

	self->private = g_new0( CactIconChooserPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_constructed( GObject *dialog )
{
	static const gchar *thisfn = "cact_icon_chooser_instance_constructed";
	CactIconChooserPrivate *priv;

	g_return_if_fail( CACT_IS_ICON_CHOOSER( dialog ));

	priv = CACT_ICON_CHOOSER( dialog )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( dialog );
		}

		g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

		base_window_signal_connect(
				BASE_WINDOW( dialog ),
				G_OBJECT( dialog ),
				BASE_SIGNAL_INITIALIZE_GTK,
				G_CALLBACK( on_base_initialize_gtk ));

		base_window_signal_connect(
				BASE_WINDOW( dialog ),
				G_OBJECT( dialog ),
				BASE_SIGNAL_INITIALIZE_WINDOW,
				G_CALLBACK( on_base_initialize_window ));

		base_window_signal_connect(
				BASE_WINDOW( dialog ),
				G_OBJECT( dialog ),
				BASE_SIGNAL_SHOW_WIDGETS,
				G_CALLBACK( on_base_show_widgets ));
	}
}

static void
instance_dispose( GObject *dialog )
{
	static const gchar *thisfn = "cact_icon_chooser_instance_dispose";
	CactIconChooser *self;
	guint pos;
	GtkWidget *paned;

	g_return_if_fail( CACT_IS_ICON_CHOOSER( dialog ));

	self = CACT_ICON_CHOOSER( dialog );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

		self->private->dispose_has_run = TRUE;

		paned = base_window_get_widget( BASE_WINDOW( self ), "IconPaned" );
		pos = gtk_paned_get_position( GTK_PANED( paned ));
		na_settings_set_uint( NA_IPREFS_ICON_CHOOSER_PANED, pos );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( dialog );
		}
	}
}

static void
instance_finalize( GObject *dialog )
{
	static const gchar *thisfn = "cact_icon_chooser_instance_finalize";
	CactIconChooser *self;

	g_return_if_fail( CACT_IS_ICON_CHOOSER( dialog ));

	g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

	self = CACT_ICON_CHOOSER( dialog );

	g_free( self->private->current_icon );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( dialog );
	}
}

/**
 * cact_icon_chooser_choose_icon:
 * @parent: the #BaseWindow parent of this dialog.
 * @icon_name: the current icon at startup.
 *
 * Initializes and runs the dialog.
 *
 * This dialog lets the user choose an icon, either as the name of a
 * themed icon, or as the path of an image.
 *
 * Returns: the selected icon, as a new string which should be g_free()
 * by the caller.
 */
gchar *
cact_icon_chooser_choose_icon( BaseWindow *parent, const gchar *icon_name )
{
	static const gchar *thisfn = "cact_icon_chooser_choose_icon";
	CactIconChooser *editor;
	gchar *new_name;

	g_return_val_if_fail( BASE_IS_WINDOW( parent ), NULL );

	g_debug( "%s: parent=%p, icon_name=%s", thisfn, ( void * ) parent, icon_name );

	editor = g_object_new( CACT_TYPE_ICON_CHOOSER,
			BASE_PROP_PARENT,         parent,
			BASE_PROP_XMLUI_FILENAME, st_xmlui_filename,
			BASE_PROP_TOPLEVEL_NAME,  st_toplevel_name,
			BASE_PROP_WSP_NAME,       st_wsp_name,
			NULL );

	editor->private->main_window = parent;
	editor->private->initial_icon = icon_name;

	new_name = g_strdup( editor->private->initial_icon );

	if( base_window_run( BASE_WINDOW( editor )) == GTK_RESPONSE_OK ){
		g_free( new_name );
		new_name = g_strdup( editor->private->current_icon );
	}

	g_object_unref( editor );

	return( new_name );
}

static void
on_base_initialize_gtk( CactIconChooser *editor, GtkDialog *toplevel, gpointer user_data )
{
	static const gchar *thisfn = "cact_icon_chooser_on_base_initialize_gtk";

	g_return_if_fail( CACT_IS_ICON_CHOOSER( editor ));

	if( !editor->private->dispose_has_run ){

		g_debug( "%s: dialog=%p, toplevel=%p, user_data=%p",
				thisfn, ( void * ) editor, ( void * ) toplevel, ( void * ) user_data );

		/* initialize the notebook
		 */
		do_initialize_themed_icons( editor );
		do_initialize_icons_by_path( editor );

		/* destroy event
		 * this is here that we are going to release our stores
		 */
		GtkDialog *dialog = GTK_DIALOG( base_window_get_gtk_toplevel( BASE_WINDOW( editor )));
		g_signal_connect( G_OBJECT( dialog ), "destroy", G_CALLBACK( on_destroy ), NULL );

#if !GTK_CHECK_VERSION( 2,22,0 )
		gtk_dialog_set_has_separator( toplevel, FALSE );
#endif
	}
}

/*
 * initialize the themed icon tab
 * first, the listview which handles the context list
 * each context carries a list store which handles the corresponding icons
 * this store is initialized the first time the context is selected
 */
static void
do_initialize_themed_icons( CactIconChooser *editor )
{
	GtkTreeView *context_view;
	GtkTreeModel *context_model;
	GtkCellRenderer *text_cell;
	GtkTreeViewColumn *column;
	GtkIconView *icon_view;
	GtkTreeSelection *selection;
	GtkIconTheme *icon_theme;
	GList *theme_contexts, *it;
	const gchar *context_label;
	GtkTreeIter iter;

	context_view = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ThemedTreeView" ));
	context_model = GTK_TREE_MODEL(
			gtk_list_store_new( THEME_CONTEXT_N_COLUMN,
					G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_STRING ));
	gtk_tree_view_set_model( context_view, context_model );
	gtk_tree_view_set_headers_visible( context_view, FALSE );

	text_cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
			"theme-context",
			text_cell,
			"text", THEME_CONTEXT_LABEL_COLUMN,
			NULL );
	gtk_tree_view_append_column( context_view, column );

	icon_view = GTK_ICON_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconView" ));
	gtk_icon_view_set_text_column( icon_view, THEME_ICON_LABEL_COLUMN );
	gtk_icon_view_set_pixbuf_column( icon_view, THEME_ICON_PIXBUF_COLUMN );
	gtk_icon_view_set_selection_mode( icon_view, GTK_SELECTION_BROWSE );

	selection = gtk_tree_view_get_selection( context_view );
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );

	icon_theme = gtk_icon_theme_get_default();
	theme_contexts = g_list_sort(
			gtk_icon_theme_list_contexts( icon_theme ), ( GCompareFunc ) g_utf8_collate );

	for( it = theme_contexts ; it ; it = it->next ){
		context_label = ( const gchar *) it->data;
		gtk_list_store_append( GTK_LIST_STORE( context_model ), &iter );
		gtk_list_store_set( GTK_LIST_STORE( context_model ), &iter,
				THEME_CONTEXT_LABEL_COLUMN, context_label,
				THEME_CONTEXT_STORE_COLUMN, NULL,
				-1 );
	}
	g_list_foreach( theme_contexts, ( GFunc ) g_free, NULL );
	g_list_free( theme_contexts );

	g_object_unref( context_model );
}

static void
do_initialize_icons_by_path( CactIconChooser *editor )
{
	GtkFileChooser *file_chooser;

	file_chooser = GTK_FILE_CHOOSER( base_window_get_widget( BASE_WINDOW( editor ), "FileChooser" ));
	gtk_file_chooser_set_action( file_chooser, GTK_FILE_CHOOSER_ACTION_OPEN );
	gtk_file_chooser_set_select_multiple( file_chooser, FALSE );
}

static void
on_base_initialize_window( CactIconChooser *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_icon_chooser_on_base_initialize_window";
	guint pos;
	GtkWidget *paned;

	g_return_if_fail( CACT_IS_ICON_CHOOSER( editor ));

	if( !editor->private->dispose_has_run ){

		g_debug( "%s: dialog=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

		pos = na_settings_get_uint( NA_IPREFS_ICON_CHOOSER_PANED, NULL, NULL );
		if( pos ){
			paned = base_window_get_widget( BASE_WINDOW( editor ), "IconPaned" );
			gtk_paned_set_position( GTK_PANED( paned ), pos );
		}

		/* setup the initial icon
		 */
		editor->private->current_icon = g_strdup( editor->private->initial_icon );
		on_current_icon_changed( editor );

		/* fillup the icon stores
		 */
		fillup_themed_icons( editor );
		fillup_icons_by_path( editor );

		/*  intercept Escape key: we do not quit on Esc.
		 */
		base_window_signal_connect(
				BASE_WINDOW( editor ),
				G_OBJECT( base_window_get_gtk_toplevel( BASE_WINDOW( editor ))),
				"key-press-event",
				G_CALLBACK( on_key_pressed_event ));

		/* OK/Cancel buttons
		 */
		base_window_signal_connect_by_name(
				BASE_WINDOW( editor ),
				"CancelButton",
				"clicked",
				G_CALLBACK( on_cancel_clicked ));

		base_window_signal_connect_by_name(
				BASE_WINDOW( editor ),
				"OKButton",
				"clicked",
				G_CALLBACK( on_ok_clicked ));
	}
}

static void
fillup_themed_icons( CactIconChooser *editor )
{
	GtkTreeView *context_view;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkIconView *icon_view;

	icon_view = GTK_ICON_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconView" ));
	base_window_signal_connect(
			BASE_WINDOW( editor ),
			G_OBJECT( icon_view ),
			"selection-changed",
			G_CALLBACK( on_themed_icon_changed ));

	/* catch double-click */
	base_window_signal_connect(
			BASE_WINDOW( editor ),
			G_OBJECT( icon_view ),
			"button-press-event",
			G_CALLBACK( on_icon_view_button_press_event ));

	context_view = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ThemedTreeView" ));
	selection = gtk_tree_view_get_selection( context_view );
	base_window_signal_connect(
			BASE_WINDOW( editor ),
			G_OBJECT( selection ),
			"changed",
			G_CALLBACK( on_themed_context_changed ));

	path = gtk_tree_path_new_first();
	gtk_tree_selection_select_path( selection, path );
	gtk_tree_path_free( path );
	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"ThemedApplyButton",
			"clicked",
			G_CALLBACK( on_themed_apply_button_clicked ));
}

static void
fillup_icons_by_path( CactIconChooser *editor )
{
	GtkFileChooser *file_chooser;
	gchar *uri;

	file_chooser = GTK_FILE_CHOOSER( base_window_get_widget( BASE_WINDOW( editor ), "FileChooser" ));
	editor->private->path_preview = gtk_image_new();
	gtk_file_chooser_set_preview_widget( file_chooser, editor->private->path_preview );

	gtk_file_chooser_unselect_all( file_chooser );

	uri = na_settings_get_string( NA_IPREFS_ICON_CHOOSER_URI, NULL, NULL );
	if( uri ){
		gtk_file_chooser_set_current_folder_uri( file_chooser, uri );
		g_free( uri );
	} else if( editor->private->current_icon ){
		gtk_file_chooser_set_filename( file_chooser, editor->private->current_icon );
	}

	base_window_signal_connect(
			BASE_WINDOW( editor ),
			G_OBJECT( file_chooser ),
			"selection-changed",
			G_CALLBACK( on_path_selection_changed ));

	base_window_signal_connect(
			BASE_WINDOW( editor ),
			G_OBJECT( file_chooser ),
			"update-preview",
			G_CALLBACK( on_path_update_preview ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"PathApplyButton",
			"clicked",
			G_CALLBACK( on_path_apply_button_clicked ));
}

static void
on_base_show_widgets( CactIconChooser *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_icon_chooser_on_base_show_widgets";
	GtkWidget *about_button;

	g_return_if_fail( CACT_IS_ICON_CHOOSER( editor ));

	if( !editor->private->dispose_has_run ){

		g_debug( "%s: dialog=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

		/* hide about button not used here
		 */
		about_button = base_window_get_widget( BASE_WINDOW( editor ), "AboutButton" );
		gtk_widget_hide( about_button );
	}
}

static void
on_cancel_clicked( GtkButton *button, CactIconChooser *editor )
{
	GtkWindow *toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_CLOSE );
}

static void
on_ok_clicked( GtkButton *button, CactIconChooser *editor )
{
	GtkWindow *toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_OK );
}

static void
on_dialog_cancel( BaseDialog *dialog )
{
	static const gchar *thisfn = "cact_icon_chooser_on_dialog_cancel";
	CactIconChooser *editor;

	g_return_if_fail( CACT_IS_ICON_CHOOSER( dialog ));

	editor = CACT_ICON_CHOOSER( dialog );

	if( !editor->private->dispose_has_run ){
		g_debug( "%s: dialog=%p", thisfn, ( void * ) dialog );

		g_free( editor->private->current_icon );
		editor->private->current_icon = g_strdup( editor->private->initial_icon );
	}
}

/*
 * display at the top of the dialog the icon addressed in @icon
 * this is this icon which will be returned if the user validates
 * this dialog
 */
static void
on_current_icon_changed( const CactIconChooser *editor )
{
	GtkImage *image;
	gchar *icon_label;
	GtkLabel *label;

	image = GTK_IMAGE( base_window_get_widget( BASE_WINDOW( editor ), "IconImage" ));
	base_gtk_utils_render( editor->private->current_icon, image, CURRENT_ICON_SIZE );

	if( editor->private->current_icon ){
		if( g_path_is_absolute( editor->private->current_icon )){
			icon_label = g_filename_to_utf8( editor->private->current_icon, -1, NULL, NULL, NULL );
		} else {
			icon_label = g_strdup( editor->private->current_icon );
		}
		label = GTK_LABEL( base_window_get_widget( BASE_WINDOW( editor ), "IconLabel" ));
		gtk_label_set_label( label, icon_label );
		g_free( icon_label );
	}
}

static gboolean
on_destroy( GtkWidget *widget, GdkEvent *event, void *foo )
{
	static const gchar *thisfn = "cact_icon_chooser_on_destroy";
	GtkTreeView *context_view;
	GtkListStore *context_store;
	GtkTreeIter context_iter;
	GtkListStore *icon_store;
	gchar *context_label;

	g_debug( "%s: widget=%p", thisfn, ( void * ) widget );

	/* clear the various models
	 */
	context_view = GTK_TREE_VIEW( na_gtk_utils_find_widget_by_name( GTK_CONTAINER( widget ), "ThemedTreeView" ));
	context_store = GTK_LIST_STORE( gtk_tree_view_get_model( context_view ));

	if( gtk_tree_model_get_iter_first( GTK_TREE_MODEL( context_store ), &context_iter )){
		while( TRUE ){

			gtk_tree_model_get( GTK_TREE_MODEL( context_store ), &context_iter,
					THEME_CONTEXT_LABEL_COLUMN, &context_label,
					THEME_CONTEXT_STORE_COLUMN, &icon_store,
					-1 );
			if( icon_store ){
				g_debug( "%s: context=%s, clearing store=%p", thisfn, context_label, ( void * ) icon_store );
				gtk_list_store_clear( icon_store );
				g_object_unref( icon_store );
			}

			g_free( context_label );

			if( !gtk_tree_model_iter_next( GTK_TREE_MODEL( context_store ), &context_iter )){
				break;
			}
		}
	}

	gtk_list_store_clear( context_store );

	/* let other handlers get this message */
	return( FALSE );
}

/*
 * mouse click on the themed icons icon view
 */
static gboolean
on_icon_view_button_press_event( GtkWidget *widget, GdkEventButton *event, CactIconChooser *editor )
{
	gboolean stop = FALSE;

	/* double-click of left button
	 * > triggers a 'Apply' action
	 */
	if( event->type == GDK_2BUTTON_PRESS && event->button == 1 ){
		on_themed_apply_triggered( editor );
		stop = TRUE;
	}

	return( stop );
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, CactIconChooser *editor )
{
	gboolean stop = FALSE;

	g_return_val_if_fail( CACT_IS_ICON_CHOOSER( editor ), FALSE );

	if( !editor->private->dispose_has_run ){

		/* inhibit Escape key */
		if( event->keyval == CACT_KEY_Escape ){
			stop = TRUE;
		}
	}

	return( stop );
}

static void
on_themed_context_changed( GtkTreeSelection *selection, CactIconChooser *editor )
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkListStore *store;
	gchar *context, *last_path;
	GtkTreePath *path;
	GtkWidget *preview_image, *preview_label;

	if( gtk_tree_selection_get_selected( selection, &model, &iter )){
		gtk_tree_model_get( model, &iter,
				THEME_CONTEXT_LABEL_COLUMN, &context,
				THEME_CONTEXT_STORE_COLUMN, &store,
				THEME_CONTEXT_LAST_SELECTED_COLUMN, &last_path,
				-1 );

		if( !store ){
			store = theme_context_load_icons( editor, context );
			gtk_list_store_set( GTK_LIST_STORE( model ), &iter, THEME_CONTEXT_STORE_COLUMN, store, -1 );
		}

		GtkIconView *iconview = GTK_ICON_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconView" ));
		gtk_icon_view_set_model( iconview, GTK_TREE_MODEL( store ));

		if( last_path ){
			path = gtk_tree_path_new_from_string( last_path );
			gtk_icon_view_select_path( iconview, path );
			gtk_tree_path_free( path );

		} else {
			preview_image = base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconImage" );
			gtk_image_set_from_pixbuf( GTK_IMAGE( preview_image ), NULL );
			preview_label = base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconName" );
			gtk_label_set_text( GTK_LABEL( preview_label ), "" );
		}

		g_free( last_path );
		g_free( context );
		g_object_unref( store );
	}
}

static void
on_themed_icon_changed( GtkIconView *icon_view, CactIconChooser *editor )
{
	GList *selected;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *label;
	GtkWidget *preview_image, *preview_label;
	GtkTreeView *context_view;
	GtkListStore *context_store;
	GtkTreeSelection *context_selection;
	GtkTreeIter context_iter;
	gchar *icon_path;

	selected = gtk_icon_view_get_selected_items( icon_view );
	if( selected ){
		model = gtk_icon_view_get_model( icon_view );

		if( gtk_tree_model_get_iter( model, &iter, ( GtkTreePath * ) selected->data )){
			gtk_tree_model_get( model, &iter,
					THEME_ICON_LABEL_COLUMN, &label,
					-1 );

			preview_image = base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconImage" );
			base_gtk_utils_render( label, GTK_IMAGE( preview_image ), PREVIEW_ICON_SIZE );
			preview_label = base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconName" );
			gtk_label_set_text( GTK_LABEL( preview_label ), label );

			/* record in context tree view the path to the last selected icon
			 */
			context_view = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ThemedTreeView" ));
			context_selection = gtk_tree_view_get_selection( context_view );
			if( gtk_tree_selection_get_selected( context_selection, ( GtkTreeModel ** ) &context_store, &context_iter )){
				icon_path = gtk_tree_model_get_string_from_iter( model, &iter );
				gtk_list_store_set( context_store, &context_iter, THEME_CONTEXT_LAST_SELECTED_COLUMN, icon_path, -1 );
				g_free( icon_path );
			}

			g_free( label );
		}

		g_list_foreach( selected, ( GFunc ) gtk_tree_path_free, NULL );
		g_list_free( selected );
	}
}

static void
on_themed_apply_button_clicked( GtkButton *button, CactIconChooser *editor )
{
	on_themed_apply_triggered( editor );
}

static void
on_themed_apply_triggered( CactIconChooser *editor )
{
	GtkWidget *icon_label;
	const gchar *icon_name;

	icon_label = base_window_get_widget( BASE_WINDOW( editor ), "ThemedIconName" );
	icon_name = gtk_label_get_text( GTK_LABEL( icon_label ));

	g_free( editor->private->current_icon );
	editor->private->current_icon = g_strdup( icon_name );
	on_current_icon_changed( editor );
}

static void
on_path_selection_changed( GtkFileChooser *file_chooser, CactIconChooser *editor )
{
	gchar *uri;

	uri = gtk_file_chooser_get_current_folder_uri( file_chooser );
	if( uri ){
		na_settings_set_string( NA_IPREFS_ICON_CHOOSER_URI, uri );
		g_free( uri );
	}
}

static void
on_path_update_preview( GtkFileChooser *file_chooser, CactIconChooser *editor )
{
	static const gchar *thisfn = "cact_icon_chooser_on_path_update_preview";
	char *filename;
	GdkPixbuf *pixbuf;
	gboolean have_preview;
	gint width, height;

	if( !gtk_icon_size_lookup( PREVIEW_ICON_SIZE, &width, &height )){
		width = PREVIEW_ICON_WIDTH;
		height = PREVIEW_ICON_WIDTH;
	}

	have_preview = FALSE;
	filename = gtk_file_chooser_get_preview_filename( file_chooser );
	g_debug( "%s: file_chooser=%p, editor=%p, filename=%s",
			thisfn, ( void * ) file_chooser, ( void * ) editor, filename );

	if( filename ){
		pixbuf = gdk_pixbuf_new_from_file_at_size( filename, width, height, NULL );
		have_preview = ( pixbuf != NULL );
		g_free( filename );
	}

	if( have_preview ){
		gtk_image_set_from_pixbuf( GTK_IMAGE( editor->private->path_preview ), pixbuf );
		g_object_unref( pixbuf );
	}

	gtk_file_chooser_set_preview_widget_active( file_chooser, TRUE );
}

static void
on_path_apply_button_clicked( GtkButton *button, CactIconChooser *editor )
{
	GtkFileChooser *file_chooser = GTK_FILE_CHOOSER( base_window_get_widget( BASE_WINDOW( editor ), "FileChooser" ));

	/* this is a filename in the character set specified by the G_FILENAME_ENCODING
	 * environment variable
	 */
	g_free( editor->private->current_icon );
	editor->private->current_icon = gtk_file_chooser_get_filename( file_chooser );
	on_current_icon_changed( editor );
}

static GtkListStore *
theme_context_load_icons( CactIconChooser *editor, const gchar *context )
{
	static const gchar *thisfn = "cact_icon_chooser_theme_context_load_icons";
	GtkTreeIter iter;
	GList *ic;
	GError *error;
	gint width, height;

	g_debug( "%s: editor=%p, context=%s", thisfn, ( void * ) editor, context );

	GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
	GtkListStore *store = gtk_list_store_new( THEME_ICON_N_COLUMN, G_TYPE_STRING, GDK_TYPE_PIXBUF );

	GList *icon_list = g_list_sort( gtk_icon_theme_list_icons( icon_theme, context ), ( GCompareFunc ) g_utf8_collate );

	if( !gtk_icon_size_lookup( VIEW_ICON_SIZE, &width, &height )){
		width = VIEW_ICON_DEFAULT_WIDTH;
	}
	g_debug( "%s: width=%d", thisfn, width );

	for( ic = icon_list ; ic ; ic = ic->next ){
		const gchar *icon_name = ( const gchar * ) ic->data;
		error = NULL;
		GdkPixbuf *pixbuf = gtk_icon_theme_load_icon(
				icon_theme, icon_name, width, GTK_ICON_LOOKUP_GENERIC_FALLBACK, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );
		} else {
			gtk_list_store_append( store, &iter );
			gtk_list_store_set( store, &iter,
					THEME_ICON_LABEL_COLUMN, icon_name,
					THEME_ICON_PIXBUF_COLUMN, pixbuf,
					-1 );
			g_object_unref( pixbuf );
		}
	}
	g_debug( "%s: %d loaded icons in store=%p", thisfn, g_list_length( icon_list ), ( void * ) store );
	g_list_foreach( icon_list, ( GFunc ) g_free, NULL );
	g_list_free( icon_list );

	return( store );
}
