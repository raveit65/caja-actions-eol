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

#include <api/na-object-api.h>

#include <core/na-gtk-utils.h>

#include "base-keysyms.h"
#include "cact-application.h"
#include "cact-main-window.h"
#include "cact-marshal.h"
#include "cact-tree-view.h"
#include "cact-tree-model.h"
#include "cact-tree-ieditable.h"

/* private class data
 */
struct _CactTreeViewClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CactTreeViewPrivate {
	gboolean       dispose_has_run;

	/* properties set at instanciation time
	 */
	BaseWindow    *window;
	GtkContainer  *parent;
	gchar         *widget_name;
	guint          mode;
	gboolean       notify_allowed;

	/* runtime data
	 */
	GtkTreeView   *tree_view;
};

/* instance properties
 */
enum {
	TREE_PROP_0,

	TREE_PROP_WINDOW_ID,
	TREE_PROP_PARENT_ID,
	TREE_PROP_WIDGET_NAME_ID,
	TREE_PROP_MODE_ID,
	TREE_PROP_NOTIFY_ALLOWED_ID,

	TREE_PROP_N_PROPERTIES
};

/* signals
 */
enum {
	COUNT_CHANGED,
	FOCUS_IN,
	FOCUS_OUT,
	LEVEL_ZERO_CHANGED,
	MODIFIED_STATUS,
	SELECTION_CHANGED,
	LAST_SIGNAL
};

/* when toggle collapse/expand rows, we want all rows have the same
 * behavior, e.g. all rows collapse, or all rows expand
 * this behavior is fixed by the first rows which will actually be
 * toggled
 */
enum {
	TOGGLE_UNDEFINED,
	TOGGLE_COLLAPSE,
	TOGGLE_EXPAND
};

/* iter on selection prototype
 */
typedef gboolean ( *FnIterOnSelection )( CactTreeView *, GtkTreeModel *, GtkTreeIter *, NAObject *, gpointer );

static gint          st_signals[ LAST_SIGNAL ] = { 0 };
static GObjectClass *st_parent_class           = NULL;

static GType      register_type( void );
static void       class_init( CactTreeViewClass *klass );
static void       tree_ieditable_iface_init( CactTreeIEditableInterface *iface, void *user_data );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void       instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void       instance_constructed( GObject *object );
static void       instance_dispose( GObject *application );
static void       instance_finalize( GObject *application );

static void       initialize_gtk( CactTreeView *view );
static void       on_base_initialize_view( BaseWindow *window, gpointer user_data );
static void       on_base_all_widgets_showed( BaseWindow *window, gpointer user_data );
static gboolean   on_button_press_event( GtkWidget *widget, GdkEventButton *event, BaseWindow *window );
static gboolean   on_focus_in( GtkWidget *widget, GdkEventFocus *event, BaseWindow *window );
static gboolean   on_focus_out( GtkWidget *widget, GdkEventFocus *event, BaseWindow *window );
static gboolean   on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseWindow *window );
static gboolean   on_popup_menu( GtkWidget *widget, BaseWindow *window );
static void       on_selection_changed( GtkTreeSelection *selection, BaseWindow *window );
static void       on_tree_view_realized( GtkWidget *treeview, BaseWindow *window );
static void       clear_selection( CactTreeView *view );
static void       display_label( GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, CactTreeView *view );
static void       extend_selection_to_children( CactTreeView *view, GtkTreeModel *model, GtkTreeIter *parent );
static GList     *get_selected_items( CactTreeView *view );
static GtkWidget *get_tree_view( CactTreeView *items_view );
static void       iter_on_selection( CactTreeView *view, FnIterOnSelection fn_iter, gpointer user_data );
static void       navigate_to_child( CactTreeView *view );
static void       navigate_to_parent( CactTreeView *view );
static void       do_open_popup( BaseWindow *window, GdkEventButton *event );
static void       select_row_at_path_by_string( CactTreeView *view, const gchar *path );
static void       toggle_collapse( CactTreeView *view );
static gboolean   toggle_collapse_iter( CactTreeView *view, GtkTreeModel *model, GtkTreeIter *iter, NAObject *object, gpointer user_data );
static void       toggle_collapse_row( GtkTreeView *treeview, GtkTreePath *path, guint *toggle );

GType
cact_tree_view_get_type( void )
{
	static GType type = 0;

	static const GInterfaceInfo tree_ieditable_iface_info = {
		( GInterfaceInitFunc ) tree_ieditable_iface_init,
		NULL,
		NULL
	};

	if( !type ){
		type = register_type();
		g_type_add_interface_static( type, CACT_TREE_IEDITABLE_TYPE, &tree_ieditable_iface_info );
	}

	return( type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_tree_view_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactTreeViewClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactTreeView ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "CactTreeView", &info, 0 );

	return( type );
}

static void
class_init( CactTreeViewClass *klass )
{
	static const gchar *thisfn = "cact_tree_view_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->get_property = instance_get_property;
	object_class->set_property = instance_set_property;
	object_class->constructed = instance_constructed;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	g_object_class_install_property( object_class, TREE_PROP_WINDOW_ID,
			g_param_spec_pointer(
					TREE_PROP_WINDOW,
					/* i18n: this is a class name and should not be translated */
					_( "BaseWindow" ),
					/* i18n: CactTreeView property long description */
					_( "The BaseWindow parent" ),
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, TREE_PROP_PARENT_ID,
			g_param_spec_pointer(
					TREE_PROP_PARENT,
					/* i18n: CactTreeView property short description */
					_( "Container" ),
					/* i18n: CactTreeView property long description */
					_( "A GtkContainer which contains the GtkTreeView widget" ),
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, TREE_PROP_WIDGET_NAME_ID,
			g_param_spec_string(
					TREE_PROP_WIDGET_NAME,
					/* i18n: CactTreeView property short description */
					_( "Widget name" ),
					/* i18n: CactTreeView property long description */
					_( "The name of GtkTreeView widget" ),
					"",
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, TREE_PROP_MODE_ID,
			g_param_spec_uint(
					TREE_PROP_MODE,
					/* i18n: CactTreeView property short description */
					_( "Management mode" ),
					/* i18n: CactTreeView property long description */
					_( "Management mode of the tree view, selection or edition" ),
					0,
					TREE_MODE_N_MODES,
					TREE_MODE_SELECTION,
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, TREE_PROP_NOTIFY_ALLOWED_ID,
			g_param_spec_boolean(
					TREE_PROP_NOTIFY_ALLOWED,
					/* i18n: CactTreeView property short description */
					_( "Allow notify" ),
					/* i18n: CactTreeView property loong description */
					_( "Whether notifications are allowed" ),
					FALSE,
					G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	/**
	 * CactTreeView::tree-signal-count-changed:
	 *
	 * This signal is emitted on BaseWindow parent when the count of items
	 * has changed in the underlying tree store.
	 *
	 *   Counting rows is needed to maintain action sensitivities in the
	 *   menubar : at least 'Tools\Export' menu item depends if we have
	 *   exportables, i.e. at least one menu or action.
	 *   Also, the total count of menu/actions/profiles is displayed in the
	 *   statusbar.
	 *
	 *   Rows are first counted when the treeview is primarily filled, or
	 *   refilled on demand. Counters are then incremented (resp. decremented)
	 *   when inserting or pasting (resp. deleting) rows in the list.
	 *
	 * Signal args:
	 * - reset if %TRUE, add if %FALSE
	 * - menus count/increment (may be negative)
	 * - actions count/increment (may be negative)
	 * - profiles count/increment (may be negative)
	 *
	 * Handler prototype:
	 * void ( *handler )( BaseWindow *window, gboolean reset,
	 *                    guint menus_count, guint actions_count, guint profiles_count, gpointer user_data );
	 */
	st_signals[ COUNT_CHANGED ] = g_signal_new(
			TREE_SIGNAL_COUNT_CHANGED,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,						/* no default handler */
			NULL,
			NULL,
			cact_cclosure_marshal_VOID__BOOLEAN_INT_INT_INT,
			G_TYPE_NONE,
			4,
			G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT );

	/**
	 * CactTreeView::tree-signal-focus-in:
	 *
	 * This signal is emitted on the window when the view gains the focus.
	 * In particular, edition menu is disabled outside of the treeview.
	 *
	 * Signal args: none
	 *
	 * Handler prototype:
	 * void ( *handler )( BaseWindow *window, gpointer user_data )
	 */
	st_signals[ FOCUS_IN ] = g_signal_new(
			TREE_SIGNAL_FOCUS_IN,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0 );

	/**
	 * CactTreeView::tree-signal-focus-out:
	 *
	 * This signal is emitted on the window when the view loses the focus.
	 * In particular, edition menu is disabled outside of the treeview.
	 *
	 * Signal args: none
	 *
	 * Handler prototype:
	 * void ( *handler )( BaseWindow *window, gpointer user_data )
	 */
	st_signals[ FOCUS_OUT ] = g_signal_new(
			TREE_SIGNAL_FOCUS_OUT,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0 );

	/**
	 * CactTreeView::tree-signal-level-zero-changed:
	 *
	 * This signal is emitted on the BaseWindow each time the level zero
	 * has changed because an item has been removed or inserted, or when
	 * the level zero has been saved.
	 *
	 * Signal args:
	 * - whether the level zero has changed (%TRUE), or comes to be saved
	 *   (%FALSE).
	 *
	 * Handler prototype:
	 * void ( *handler )( BaseWindow *window, gboolean is_modified, gpointer user_data );
	 */
	st_signals[ LEVEL_ZERO_CHANGED ] = g_signal_new(
			TREE_SIGNAL_LEVEL_ZERO_CHANGED,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,					/* no default handler */
			NULL,
			NULL,
			g_cclosure_marshal_VOID__BOOLEAN,
			G_TYPE_NONE,
			1,
			G_TYPE_BOOLEAN );

	/**
	 * CactTreeView::tree-signal-modified-status-changed:
	 *
	 * This signal is emitted on the BaseWindow when the view detects that
	 * the count of modified NAObjectItems has changed, when an item is
	 * deleted, or when the level zero is changed.
	 *
	 * The signal is actually emitted by the CactTreeIEditable interface
	 * which takes care of all edition features.
	 *
	 * Signal args:
	 * - the new modification status of the tree
	 *
	 * Handler prototype:
	 * void ( *handler )( BaseWindow *window, gboolean is_modified, gpointer user_data )
	 */
	st_signals[ MODIFIED_STATUS ] = g_signal_new(
			TREE_SIGNAL_MODIFIED_STATUS_CHANGED,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			g_cclosure_marshal_VOID__BOOLEAN,
			G_TYPE_NONE,
			1,
			G_TYPE_BOOLEAN );

	klass->private = g_new0( CactTreeViewClassPrivate, 1 );
}

static void
tree_ieditable_iface_init( CactTreeIEditableInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_tree_ieditable_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_tree_view_instance_init";
	CactTreeView *self;

	g_return_if_fail( CACT_IS_TREE_VIEW( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_TREE_VIEW( instance );

	self->private = g_new0( CactTreeViewPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	CactTreeView *self;

	g_return_if_fail( CACT_IS_TREE_VIEW( object ));
	self = CACT_TREE_VIEW( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case TREE_PROP_WINDOW_ID:
				g_value_set_pointer( value, self->private->window );
				break;

			case TREE_PROP_PARENT_ID:
				g_value_set_pointer( value, self->private->parent );
				break;

			case TREE_PROP_WIDGET_NAME_ID:
				g_value_set_string( value, self->private->widget_name );
				break;

			case TREE_PROP_MODE_ID:
				g_value_set_uint( value, self->private->mode );
				break;

			case TREE_PROP_NOTIFY_ALLOWED_ID:
				g_value_set_boolean( value, self->private->notify_allowed );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec )
{
	CactTreeView *self;

	g_return_if_fail( CACT_IS_TREE_VIEW( object ));
	self = CACT_TREE_VIEW( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case TREE_PROP_WINDOW_ID:
				self->private->window = g_value_get_pointer( value );
				break;

			case TREE_PROP_PARENT_ID:
				self->private->parent = g_value_get_pointer( value );
				break;

			case TREE_PROP_WIDGET_NAME_ID:
				g_free( self->private->widget_name );
				self->private->widget_name = g_value_dup_string( value );
				break;

			case TREE_PROP_MODE_ID:
				self->private->mode = g_value_get_uint( value );
				break;

			case TREE_PROP_NOTIFY_ALLOWED_ID:
				self->private->notify_allowed = g_value_get_boolean( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_constructed( GObject *object )
{
	static const gchar *thisfn = "cact_tree_view_instance_constructed";
	CactTreeViewPrivate *priv;

	g_return_if_fail( CACT_IS_TREE_VIEW( object ));

	priv = CACT_TREE_VIEW( object )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( object );
		}

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		base_window_signal_connect(
				priv->window,
				G_OBJECT( priv->window ),
				BASE_SIGNAL_INITIALIZE_WINDOW,
				G_CALLBACK( on_base_initialize_view ));

		base_window_signal_connect(
				priv->window,
				G_OBJECT( priv->window ),
				BASE_SIGNAL_SHOW_WIDGETS,
				G_CALLBACK( on_base_all_widgets_showed ));

		g_object_set_data( G_OBJECT( priv->window ), WINDOW_DATA_TREE_VIEW, object );

		initialize_gtk( CACT_TREE_VIEW( object ));
	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cact_tree_view_instance_dispose";
	CactTreeView *self;

	g_return_if_fail( CACT_IS_TREE_VIEW( object ));

	self = CACT_TREE_VIEW( object );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		if( self->private->mode == TREE_MODE_EDITION ){
			cact_tree_ieditable_terminate( CACT_TREE_IEDITABLE( self ));
		}

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *instance )
{
	static const gchar *thisfn = "cact_tree_view_instance_finalize";
	CactTreeView *self;

	g_return_if_fail( CACT_IS_TREE_VIEW( instance ));

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	self = CACT_TREE_VIEW( instance );

	g_free( self->private->widget_name );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( instance );
	}
}

/**
 * cact_tree_view_new:
 * @window: the BaseWindow which embeds the tree view.
 * @parent: the GtkContainer parent of the tree view.
 * @treeview_name: the GtkTreeView widget name.
 * @mode: management mode.
 *
 * Returns: a newly allocated CactTreeView object, which will be owned
 * by the caller. It is useless to unref it as it will automatically
 * auto-destroys itself at @window finalization.
 *
 * This function is called from CactMainWindow::on_base_initialize_gtk()
 * signal handler, thus only once during the application life.
 */
CactTreeView *
cact_tree_view_new( BaseWindow *window, GtkContainer *parent, const gchar *treeview_name, CactTreeMode mode )
{
	CactTreeView *view;

	view = g_object_new( CACT_TYPE_TREE_VIEW,
			TREE_PROP_WINDOW,      window,
			TREE_PROP_PARENT,      parent,
			TREE_PROP_WIDGET_NAME, treeview_name,
			TREE_PROP_MODE,        mode,
			NULL );

	return( view );
}

/*
 * called from instance_constructed()
 *   from cact_tree_view_new()
 *     from CactMainWindow::on_base_initialize_gtk() signal handler
 */
static void
initialize_gtk( CactTreeView *items_view )
{
	static const gchar *thisfn = "cact_tree_view_initialize_gtk";
	GtkTreeView *treeview;
	CactTreeModel *model;
	GtkWidget *label;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;
	GList *renderers;

	g_debug( "%s: items_view=%p", thisfn, ( void * ) items_view );

	treeview = GTK_TREE_VIEW( get_tree_view( items_view ));
	model = cact_tree_model_new( items_view->private->window, treeview, items_view->private->mode );
	gtk_tree_view_set_model( treeview, GTK_TREE_MODEL( model ));
	g_object_unref( model );
	g_debug( "%s: cact_tree_model_ref_count=%d", thisfn, G_OBJECT( model )->ref_count );

	/* associates the ItemsView to the label */
	label = na_gtk_utils_find_widget_by_name( items_view->private->parent, "ActionsListLabel" );
	gtk_label_set_mnemonic_widget( GTK_LABEL( label ), GTK_WIDGET( treeview ));

	/* create visible columns on the tree view
	 */
	column = gtk_tree_view_column_new_with_attributes(
			"icon",
			gtk_cell_renderer_pixbuf_new(),
			"pixbuf", TREE_COLUMN_ICON,
			NULL );
	gtk_tree_view_append_column( treeview, column );

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
			"label",
			renderer,
			"text", TREE_COLUMN_LABEL,
			NULL );
	gtk_tree_view_column_set_sort_column_id( column, TREE_COLUMN_LABEL );
	gtk_tree_view_append_column( treeview, column );

	/* allow multiple selection
	 */
	selection = gtk_tree_view_get_selection( treeview );
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );

	/* misc properties
	 */
	gtk_tree_view_set_enable_tree_lines( treeview, TRUE );

	if( items_view->private->mode == TREE_MODE_EDITION ){
		column = gtk_tree_view_get_column( treeview, TREE_COLUMN_LABEL );
		renderers = gtk_cell_layout_get_cells( GTK_CELL_LAYOUT( column ));
		renderer = GTK_CELL_RENDERER( renderers->data );
		gtk_tree_view_column_set_cell_data_func(
				column, renderer, ( GtkTreeCellDataFunc ) display_label, items_view, NULL );
	}
}

static void
on_base_initialize_view( BaseWindow *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_tree_view_on_base_initialize_view";
	CactTreeView *items_view;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;

	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));

	if( !items_view->private->dispose_has_run ){

		g_debug( "%s: window=%p (%s), user_data=%p",
				thisfn,
				( void * ) window, G_OBJECT_TYPE_NAME( window ),
				( void * ) user_data );

		treeview = GTK_TREE_VIEW( get_tree_view( items_view ));
		items_view->private->tree_view = treeview;

		/* monitors the selection */
		selection = gtk_tree_view_get_selection( treeview );
		base_window_signal_connect(
				window,
				G_OBJECT( selection ),
				"changed",
				G_CALLBACK( on_selection_changed ));

		/* delay all other signal connections until the widget be realized
		 */
		base_window_signal_connect(
				window,
				G_OBJECT( treeview ),
				"realize",
				G_CALLBACK( on_tree_view_realized ));

		if( items_view->private->mode == TREE_MODE_EDITION ){
			cact_tree_ieditable_initialize( CACT_TREE_IEDITABLE( items_view ), treeview, window );
		}
	}
}

static void
on_base_all_widgets_showed( BaseWindow *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_tree_view_on_base_all_widgets_showed";
	CactTreeView *items_view;

	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));

	if( !items_view->private->dispose_has_run ){
		g_debug( "%s: window=%p (%s), user_data=%p",
				thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ), ( void * ) user_data );
	}
}

static gboolean
on_button_press_event( GtkWidget *widget, GdkEventButton *event, BaseWindow *window )
{
	gboolean stop = FALSE;

	/* single click on right button */
	if( event->type == GDK_BUTTON_PRESS && event->button == 3 ){
		do_open_popup( window, event );
		stop = TRUE;
	}

	return( stop );
}

/*
 * focus is monitored to avoid an accelerator being pressed while on a tab
 * triggers an unwaited operation on the list
 * e.g. when editing an entry field on the tab, pressing Del should _not_
 * delete current row in the list!
 */
static gboolean
on_focus_in( GtkWidget *widget, GdkEventFocus *event, BaseWindow *window )
{
	gboolean stop = FALSE;

	g_signal_emit_by_name( window, TREE_SIGNAL_FOCUS_IN );

	return( stop );
}

static gboolean
on_focus_out( GtkWidget *widget, GdkEventFocus *event, BaseWindow *window )
{
	gboolean stop = FALSE;

	g_signal_emit_by_name( window, TREE_SIGNAL_FOCUS_OUT );

	return( stop );
}

static gboolean
on_key_pressed_event( GtkWidget *widget, GdkEventKey *event, BaseWindow *window )
{
	gboolean stop = FALSE;
	CactTreeView *items_view;

	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));

	if( !items_view->private->dispose_has_run ){

		if( event->keyval == CACT_KEY_Return || event->keyval == CACT_KEY_KP_Enter ){
			toggle_collapse( items_view );
			stop = TRUE;
		}
		if( event->keyval == CACT_KEY_Right ){
			navigate_to_child( items_view );
			stop = TRUE;
		}
		if( event->keyval == CACT_KEY_Left ){
			navigate_to_parent( items_view );
			stop = TRUE;
		}
	}

	return( stop );
}

/*
 * triggered by the "popup-menu" signal, itself triggered by the keybindings
 */
static gboolean
on_popup_menu( GtkWidget *widget, BaseWindow *window )
{
	do_open_popup( window, NULL );
	return( TRUE );
}
/*
 * handles the "changed" signal emitted on the GtkTreeSelection
 */
static void
on_selection_changed( GtkTreeSelection *selection, BaseWindow *window )
{
	static const gchar *thisfn = "cact_tree_view_on_selection_changed";
	CactTreeView *items_view;
	GList *selected_items;

	items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));

	if( !items_view->private->dispose_has_run ){
		if( items_view->private->notify_allowed ){
			g_debug( "%s: selection=%p, window=%p", thisfn, ( void * ) selection, ( void * ) window );

			selected_items = get_selected_items( items_view );
			g_signal_emit_by_name( window, MAIN_SIGNAL_SELECTION_CHANGED, selected_items );
		}
	}
}

static void
on_tree_view_realized( GtkWidget *treeview, BaseWindow *window )
{
	g_debug( "cact_tree_view_on_tree_view_realized" );

	/* expand/collapse with the keyboard */
	base_window_signal_connect(
			window,
			G_OBJECT( treeview ),
			"key-press-event",
			G_CALLBACK( on_key_pressed_event ));

	/* monitors whether the focus is on the view */
	base_window_signal_connect(
			window,
			G_OBJECT( treeview ),
			"focus-in-event",
			G_CALLBACK( on_focus_in ));

	base_window_signal_connect(
			window,
			G_OBJECT( treeview ),
			"focus-out-event",
			G_CALLBACK( on_focus_out ));

	/* monitors mouse events */
	base_window_signal_connect(
			window,
			G_OBJECT( treeview ),
			"button-press-event",
			G_CALLBACK( on_button_press_event ));

	base_window_signal_connect(
			window,
			G_OBJECT( treeview ),
			"popup-menu",
			G_CALLBACK( on_popup_menu ));

	/* force the treeview to have the focus at start
	 * and select the first row if it exists
	 */
	gtk_widget_grab_focus( treeview );
}

/**
 * cact_tree_view_fill:
 * @view: this #CactTreeView instance.
 *
 * Fill the tree view with the provided list of items.
 *
 * Notification of selection changes is temporary suspended during the
 * fillup of the list, so that client is not cluttered with tons of
 * selection-changed messages.
 */
void
cact_tree_view_fill( CactTreeView *view, GList *items )
{
	static const gchar *thisfn = "cact_tree_view_fill";
	CactTreeModel *model;
	gint nb_menus, nb_actions, nb_profiles;

	g_return_if_fail( CACT_IS_TREE_VIEW( view ));

	if( !view->private->dispose_has_run ){
		g_debug( "%s: view=%p, items=%p (count=%u)",
				thisfn, ( void * ) view, ( void * ) items, g_list_length( items ));

		clear_selection( view );
		view->private->notify_allowed = FALSE;
		model = CACT_TREE_MODEL( gtk_tree_view_get_model( view->private->tree_view ));
		cact_tree_model_fill( model, items );
		g_debug( "%s: cact_tree_model_ref_count=%d", thisfn, G_OBJECT( model )->ref_count );

		view->private->notify_allowed = TRUE;
		na_object_count_items( items, &nb_menus, &nb_actions, &nb_profiles );
		g_signal_emit_by_name( view->private->window, TREE_SIGNAL_COUNT_CHANGED, TRUE, nb_menus, nb_actions, nb_profiles );
		g_signal_emit_by_name( view->private->window, TREE_SIGNAL_MODIFIED_STATUS_CHANGED, FALSE );

		select_row_at_path_by_string( view, "0" );
	}
}

/**
 * cact_tree_view_are_notify_allowed:
 * @view: this #CactTreeView instance.
 *
 * Returns: %TRUE if notifications are allowed, %FALSE else.
 */
gboolean
cact_tree_view_are_notify_allowed( const CactTreeView *view )
{
	gboolean are_allowed;

	g_return_val_if_fail( CACT_IS_TREE_VIEW( view ), FALSE );

	are_allowed = FALSE;

	if( !view->private->dispose_has_run ){

		are_allowed = view->private->notify_allowed;
	}

	return( are_allowed );
}

/**
 * cact_tree_view_set_notify_allowed:
 * @view: this #CactTreeView instance.
 * @allow: whether the notifications are to be allowed.
 */
void
cact_tree_view_set_notify_allowed( CactTreeView *view, gboolean allow )
{
	g_return_if_fail( CACT_IS_TREE_VIEW( view ));

	if( !view->private->dispose_has_run ){

		view->private->notify_allowed = allow;
	}
}

/**
 * cact_tree_view_collapse_all:
 * @view: this #CactTreeView instance.
 *
 * Collapse all the tree hierarchy.
 */
void
cact_tree_view_collapse_all( const CactTreeView *view )
{
	g_return_if_fail( CACT_IS_TREE_VIEW( view ));

	if( !view->private->dispose_has_run ){

		gtk_tree_view_collapse_all( view->private->tree_view );
	}
}

/**
 * cact_tree_view_expand_all:
 * @view: this #CactTreeView instance.
 *
 * Collapse all the tree hierarchy.
 */
void
cact_tree_view_expand_all( const CactTreeView *view )
{
	g_return_if_fail( CACT_IS_TREE_VIEW( view ));

	if( !view->private->dispose_has_run ){

		gtk_tree_view_expand_all( view->private->tree_view );
	}
}

/**
 * cact_tree_view_get_item_by_id:
 * @view: this #CactTreeView instance.
 * @id: the searched #NAObjectItem.
 *
 * Returns: a pointer on the searched #NAObjectItem if it exists, or %NULL.
 *
 * The returned pointer is owned by the underlying tree store, and should
 * not be released by the caller.
 */
NAObjectItem *
cact_tree_view_get_item_by_id( const CactTreeView *view, const gchar *id )
{
	NAObjectItem *item;
	CactTreeModel *model;

	g_return_val_if_fail( CACT_IS_TREE_VIEW( view ), NULL );

	item = NULL;

	if( !view->private->dispose_has_run ){

		model = CACT_TREE_MODEL( gtk_tree_view_get_model( view->private->tree_view ));
		item = cact_tree_model_get_item_by_id( model, id );
	}

	return( item );
}

/**
 * cact_tree_view_get_items:
 * @view: this #CactTreeView instance.
 *
 * Returns: the content of the current tree as a newly allocated list
 * which should be na_object_free_items() by the caller.
 */
GList *
cact_tree_view_get_items( const CactTreeView *view )
{
	return( cact_tree_view_get_items_ex( view, TREE_LIST_ALL ));
}

/**
 * cact_tree_view_get_items_ex:
 * @view: this #CactTreeView instance.
 * @mode: the list content
 *
 * Returns: the content of the current tree as a newly allocated list
 * which should be na_object_free_items() by the caller.
 */
GList *
cact_tree_view_get_items_ex( const CactTreeView *view, guint mode )
{
	GList *items;
	CactTreeModel *model;
	GList *deleted;

	g_return_val_if_fail( CACT_IS_TREE_VIEW( view ), NULL );

	items = NULL;

	if( !view->private->dispose_has_run ){

		deleted = NULL;

		if( view->private->mode == TREE_MODE_EDITION ){
			if( mode & TREE_LIST_DELETED ){
				deleted = cact_tree_ieditable_get_deleted( CACT_TREE_IEDITABLE( view ));
			}
		}

		model = CACT_TREE_MODEL( gtk_tree_view_get_model( view->private->tree_view ));
		items = cact_tree_model_get_items( model, mode );

		items = g_list_concat( items, deleted );
	}

	return( items );
}

/**
 * cact_tree_view_select_row_at_path:
 * @view: this #CactTreeView object.
 * @path: the #GtkTreePath to be selected.
 *
 * Select the row at the required path, or the immediate previous, or
 * the next following, or eventually the immediate parent.
 *
 * If nothing can be selected (and notify is allowed), at least send a
 * message with an empty selection.
 */
void
cact_tree_view_select_row_at_path( CactTreeView *view, GtkTreePath *path )
{
	static const gchar *thisfn = "cact_tree_view_select_row_at_path";
	gchar *path_str;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gboolean something = FALSE;

	g_return_if_fail( CACT_IS_TREE_VIEW( view ));

	if( !view->private->dispose_has_run ){

		path_str = gtk_tree_path_to_string( path );
		g_debug( "%s: view=%p, path=%s", thisfn, ( void * ) view, path_str );
		g_free( path_str );

		if( path ){
			gtk_tree_view_expand_to_path( view->private->tree_view, path );
			model = gtk_tree_view_get_model( view->private->tree_view );

			if( gtk_tree_model_get_iter( model, &iter, path )){
				gtk_tree_view_set_cursor( view->private->tree_view, path, NULL, FALSE );
				something = TRUE;

			} else if( gtk_tree_path_prev( path ) && gtk_tree_model_get_iter( model, &iter, path )){
				gtk_tree_view_set_cursor( view->private->tree_view, path, NULL, FALSE );
				something = TRUE;

			} else {
				gtk_tree_path_next( path );
				if( gtk_tree_model_get_iter( model, &iter, path )){
					gtk_tree_view_set_cursor( view->private->tree_view, path, NULL, FALSE );
					something = TRUE;

				} else if( gtk_tree_path_get_depth( path ) > 1 &&
							gtk_tree_path_up( path ) &&
							gtk_tree_model_get_iter( model, &iter, path )){

								gtk_tree_view_set_cursor( view->private->tree_view, path, NULL, FALSE );
								something = TRUE;
				}
			}
		}

		if( !something ){
			if( view->private->notify_allowed ){
				g_signal_emit_by_name( view->private->window, MAIN_SIGNAL_SELECTION_CHANGED, NULL );
			}
		}
	}
}

static void
clear_selection( CactTreeView *view )
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection( view->private->tree_view );
	gtk_tree_selection_unselect_all( selection );
}

/*
 * item modified: italic
 * item not saveable (invalid): red
 */
static void
display_label( GtkTreeViewColumn *column, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, CactTreeView *view )
{
	NAObject *object;
	gchar *label;

	g_return_if_fail( view->private->mode == TREE_MODE_EDITION );

	gtk_tree_model_get( model, iter, TREE_COLUMN_NAOBJECT, &object, -1 );

	if( object ){
		g_object_unref( object );
		g_return_if_fail( NA_IS_OBJECT( object ));

		label = na_object_get_label( object );
		g_object_set( cell, "style-set", FALSE, NULL );
		g_object_set( cell, "foreground-set", FALSE, NULL );

		if( na_object_is_modified( object )){
			g_object_set( cell, "style", PANGO_STYLE_ITALIC, "style-set", TRUE, NULL );
		}

		if( !na_object_is_valid( object )){
			g_object_set( cell, "foreground", "Red", "foreground-set", TRUE, NULL );
		}

		g_object_set( cell, "text", label, NULL );
		g_free( label );
	}
}

/*
 * when expanding a selected row which has children
 */
static void
extend_selection_to_children( CactTreeView *view, GtkTreeModel *model, GtkTreeIter *parent )
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gboolean ok;

	selection = gtk_tree_view_get_selection( view->private->tree_view );
	ok = gtk_tree_model_iter_children( model, &iter, parent );

	while( ok ){
		GtkTreePath *path = gtk_tree_model_get_path( model, &iter );
		gtk_tree_selection_select_path( selection, path );
		gtk_tree_path_free( path );
		ok = gtk_tree_model_iter_next( model, &iter );
	}
}

/*
 * get_selected_items:
 * @view: this #CactTreeView instance.
 *
 * We acquire here a new reference on objects corresponding to actually
 * selected rows, and their children.
 *
 * Returns: the currently selected rows as a #GList of #NAObjectItems
 * which should be na_object_free_items() by the caller.
 */
static GList *
get_selected_items( CactTreeView *view )
{
	static const gchar *thisfn = "cact_tree_view_get_selected_items";
	GList *items;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *it, *listrows;
	NAObjectId *object;

	items = NULL;
	selection = gtk_tree_view_get_selection( view->private->tree_view );
	listrows = gtk_tree_selection_get_selected_rows( selection, &model );

	for( it = listrows ; it ; it = it->next ){
		path = ( GtkTreePath * ) it->data;
		gtk_tree_model_get_iter( model, &iter, path );
		gtk_tree_model_get( model, &iter, TREE_COLUMN_NAOBJECT, &object, -1 );
		items = g_list_prepend( items, na_object_ref( object ));
		g_object_unref( object );
		g_debug( "%s: object=%p (%s) ref_count=%d",
				thisfn,
				( void * ) object, G_OBJECT_TYPE_NAME( object ), G_OBJECT( object )->ref_count );
	}

	g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( listrows );

	return( g_list_reverse( items ));
}

static GtkWidget *
get_tree_view( CactTreeView *items_view )
{
	return( na_gtk_utils_find_widget_by_name(
			items_view->private->parent,
			items_view->private->widget_name ));
}

static void
iter_on_selection( CactTreeView *view, FnIterOnSelection fn_iter, gpointer user_data )
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *listrows, *ipath;
	GtkTreePath *path;
	GtkTreeIter iter;
	NAObject *object;
	gboolean stop;

	stop = FALSE;
	selection = gtk_tree_view_get_selection( view->private->tree_view );
	listrows = gtk_tree_selection_get_selected_rows( selection, &model );
	listrows = g_list_reverse( listrows );

	for( ipath = listrows ; !stop && ipath ; ipath = ipath->next ){
		path = ( GtkTreePath * ) ipath->data;
		gtk_tree_model_get_iter( model, &iter, path );
		gtk_tree_model_get( model, &iter, TREE_COLUMN_NAOBJECT, &object, -1 );

		stop = fn_iter( view, model, &iter, object, user_data );

		g_object_unref( object );
	}

	g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( listrows );
}

/*
 * navigate_to_child:
 * @view: this #CactTreeView object.
 *
 * On right arrow, if collapsed, then expand
 * if already expanded, then goto first child
 *
 * Note:
 * a row which does not have any child is always considered as collapsed;
 * trying to expand it has no visible effect.
 */
static void
navigate_to_child( CactTreeView *view )
{
	GtkTreeSelection *selection;
	GList *listrows;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreePath *child_path;

	selection = gtk_tree_view_get_selection( view->private->tree_view );
	listrows = gtk_tree_selection_get_selected_rows( selection, &model );

	if( g_list_length( listrows ) == 1 ){
		path = ( GtkTreePath * ) listrows->data;

		if( !gtk_tree_view_row_expanded( view->private->tree_view, path )){
			gtk_tree_view_expand_row( view->private->tree_view, path, FALSE );

		} else {
			gtk_tree_model_get_iter( model, &iter, path );

			if( gtk_tree_model_iter_has_child( model, &iter )){
				child_path = gtk_tree_path_copy( path );
				gtk_tree_path_append_index( child_path, 0 );
				cact_tree_view_select_row_at_path( view, child_path );
				gtk_tree_path_free( child_path );
			}
		}
	}

	g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( listrows );
}

/*
 * navigate_to_parent:
 * @view: this #CactTreeView object.
 *
 * On left arrow, go to the parent.
 * if already on a parent, collapse it
 * if already collapsed, up to the next parent
 *
 * e.g. path="2:0", depth=2
 * this means that depth is always >= 1, with depth=1 being the root.
 */
static void
navigate_to_parent( CactTreeView *view )
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *listrows;
	GtkTreePath *path;
	GtkTreePath *parent_path;

	selection = gtk_tree_view_get_selection( view->private->tree_view );
	listrows = gtk_tree_selection_get_selected_rows( selection, &model );

	if( g_list_length( listrows ) == 1 ){
		path = ( GtkTreePath * ) listrows->data;

		if( gtk_tree_view_row_expanded( view->private->tree_view, path )){
			gtk_tree_view_collapse_row( view->private->tree_view, path );

		} else if( gtk_tree_path_get_depth( path ) > 1 ){
			parent_path = gtk_tree_path_copy( path );
			gtk_tree_path_up( parent_path );
			cact_tree_view_select_row_at_path( view, parent_path );
			gtk_tree_path_free( parent_path );
		}
	}

	g_list_foreach( listrows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( listrows );
}

static void
do_open_popup( BaseWindow *window, GdkEventButton *event )
{
	CactTreeView *items_view;
	GtkTreePath *path;

	if( event ){
		items_view = CACT_TREE_VIEW( g_object_get_data( G_OBJECT( window ), WINDOW_DATA_TREE_VIEW ));

		if( gtk_tree_view_get_path_at_pos( items_view->private->tree_view, event->x, event->y, &path, NULL, NULL, NULL )){
			cact_tree_view_select_row_at_path( items_view, path );
			gtk_tree_path_free( path );
		}
	}

	g_signal_emit_by_name( window, MAIN_SIGNAL_CONTEXT_MENU, event, "/ui/TreeContext" );
}

/*
 * cact_tree_view_select_row_at_path_by_string:
 * @view: this #CactTreeView object.
 * @path: the #GtkTreePath to be selected.
 *
 * cf. cact_tree_view_select_row_at_path().
 */
static void
select_row_at_path_by_string( CactTreeView *view, const gchar *path_str )
{
	GtkTreePath *path;

	path = gtk_tree_path_new_from_string( path_str );
	cact_tree_view_select_row_at_path( view, path );
	gtk_tree_path_free( path );
}

/*
 * Toggle or collapse the current subtree.
 */
static void
toggle_collapse( CactTreeView *view )
{
	guint toggle = TOGGLE_UNDEFINED;

	iter_on_selection( view, ( FnIterOnSelection ) toggle_collapse_iter, &toggle );
}

static gboolean
toggle_collapse_iter( CactTreeView *view, GtkTreeModel *model,
						GtkTreeIter *iter, NAObject *object, gpointer user_data )
{
	guint count;
	guint *toggle;

	toggle = ( guint * ) user_data;

	if( NA_IS_OBJECT_ITEM( object )){
		GtkTreePath *path = gtk_tree_model_get_path( model, iter );
		count = na_object_get_items_count( object );

		if(( count > 1 && NA_IS_OBJECT_ACTION( object )) ||
			( count > 0 && NA_IS_OBJECT_MENU( object ))){

			toggle_collapse_row( view->private->tree_view, path, toggle );
		}

		gtk_tree_path_free( path );

		/* do not extend selection to children */
		if( 0 ){
			if( *toggle == TOGGLE_EXPAND ){
				extend_selection_to_children( view, model, iter );
			}
		}
	}

	/* do not stop iteration */
	return( FALSE );
}

/*
 * toggle mode can be undefined, collapse or expand
 * it is set on the first row
 */
static void
toggle_collapse_row( GtkTreeView *treeview, GtkTreePath *path, guint *toggle )
{
	if( *toggle == TOGGLE_UNDEFINED ){
		*toggle = gtk_tree_view_row_expanded( treeview, path ) ? TOGGLE_COLLAPSE : TOGGLE_EXPAND;
	}

	if( *toggle == TOGGLE_COLLAPSE ){
		if( gtk_tree_view_row_expanded( treeview, path )){
			gtk_tree_view_collapse_row( treeview, path );
		}
	} else {
		if( !gtk_tree_view_row_expanded( treeview, path )){
			gtk_tree_view_expand_row( treeview, path, TRUE );
		}
	}
}
