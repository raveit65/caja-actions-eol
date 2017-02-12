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

#include <api/na-object-api.h>

#include <core/na-iprefs.h>

#include "cact-application.h"
#include "cact-clipboard.h"
#include "base-gtk-utils.h"
#include "cact-main-tab.h"
#include "cact-tree-model.h"
#include "cact-tree-model-priv.h"

/* private class data
 */
struct _CactTreeModelClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* instance properties
 */
#define TREE_PROP_TREEVIEW				"tree-prop-treeview"
#define TREE_PROP_MODE					"tree-prop-mode"

enum {
	TREE_PROP_0,

	TREE_PROP_WINDOW_ID,
	TREE_PROP_TREEVIEW_ID,
	TREE_PROP_MODE_ID,

	TREE_PROP_N_PROPERTIES
};

/* iter on tree store
 */
typedef gboolean ( *FnIterOnStore )( const CactTreeModel *, GtkTreeStore *, GtkTreePath *, NAObject *, gpointer );

/* getting the list of items
 * - mode is the indicator of the wished content
 * - list is the returned list
 */
typedef struct {
	guint  mode;
	GList *items;
}
	ntmGetItems;

/* when iterating while searching for an object by id
 * setting the iter if found
 */
typedef struct {
	gchar       *id;
	NAObject    *object;
	GtkTreeIter *iter;
}
	ntmFindId;

/* when iterating while searching for an object by its address
 * setting the iter if found
 */
typedef struct {
	const NAObject *object;
	GtkTreeIter    *iter;
	GtkTreePath    *path;
}
	ntmFindObject;

/* dump the content of the tree
 */
typedef struct {
	gchar *fname;
	gchar *prefix;
}
	ntmDumpStruct;

/* drop formats, as defined in cact-tree-model-dnd.c
 */
extern GtkTargetEntry tree_model_dnd_dest_formats[];
extern guint          tree_model_dnd_dest_formats_count;

#define TREE_MODEL_ORDER_MODE			"cact-tree-model-order-mode"

static GtkTreeModelFilterClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( CactTreeModelClass *klass );
static void     imulti_drag_source_init( EggTreeMultiDragSourceIface *iface, void *user_data );
static void     idrag_dest_init( GtkTreeDragDestIface *iface, void *user_data );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void     instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void     instance_constructed( GObject *model );
static void     instance_dispose( GObject *model );
static void     instance_finalize( GObject *model );

static void     on_settings_order_mode_changed( const gchar *group, const gchar *key, gconstpointer new_value, gboolean mandatory, CactTreeModel *model );
static void     on_tab_updatable_item_updated( BaseWindow *window, NAIContext *context, guint data, CactTreeModel *model );

static void     append_item( GtkTreeStore *model, GtkTreeView *treeview, GtkTreeIter *parent, GtkTreeIter *iter, const NAObject *object );
static void     display_item( GtkTreeStore *model, GtkTreeView *treeview, GtkTreeIter *iter, const NAObject *object );
static void     display_order_change( CactTreeModel *model, gint order_mode );
#if 0
static void     dump( CactTreeModel *model );
static gboolean dump_store( CactTreeModel *model, GtkTreePath *path, NAObject *object, ntmDumpStruct *ntm );
#endif
static void     fill_tree_store( GtkTreeStore *model, GtkTreeView *treeview, NAObject *object, GtkTreeIter *parent );
static gboolean filter_visible( GtkTreeModel *store, GtkTreeIter *iter, CactTreeModel *model );
static gboolean find_item_iter( CactTreeModel *model, GtkTreeStore *store, GtkTreePath *path, NAObject *object, ntmFindId *nfo );
static gboolean find_object_iter( CactTreeModel *model, GtkTreeStore *store, GtkTreePath *path, NAObject *object, ntmFindObject *nfo );
static gboolean get_items_iter( const CactTreeModel *model, GtkTreeStore *store, GtkTreePath *path, NAObject *object, ntmGetItems *ngi );
static void     iter_on_store( const CactTreeModel *model, GtkTreeModel *store, GtkTreeIter *parent, FnIterOnStore fn, gpointer user_data );
static gboolean iter_on_store_item( const CactTreeModel *model, GtkTreeModel *store, GtkTreeIter *iter, FnIterOnStore fn, gpointer user_data );
static void     remove_if_exists( CactTreeModel *model, GtkTreeModel *store, const NAObject *object );
static gboolean delete_items_rec( GtkTreeStore *store, GtkTreeIter *iter );
static gint     sort_actions_list( GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data );

GType
cact_tree_model_get_type( void )
{
	static GType model_type = 0;

	if( !model_type ){
		model_type = register_type();
	}

	return( model_type );
}

static GType
register_type (void)
{
	static const gchar *thisfn = "cact_tree_model_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactTreeModelClass ),
		NULL,							/* base_init */
		NULL,							/* base_finalize */
		( GClassInitFunc ) class_init,
		NULL,							/* class_finalize */
		NULL,							/* class_data */
		sizeof( CactTreeModel ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo imulti_drag_source_info = {
		( GInterfaceInitFunc ) imulti_drag_source_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo idrag_dest_info = {
		( GInterfaceInitFunc ) idrag_dest_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( GTK_TYPE_TREE_MODEL_FILTER, "CactTreeModel", &info, 0 );

	g_type_add_interface_static( type, EGG_TYPE_TREE_MULTI_DRAG_SOURCE, &imulti_drag_source_info );

	g_type_add_interface_static( type, GTK_TYPE_TREE_DRAG_DEST, &idrag_dest_info );

	return( type );
}

static void
class_init( CactTreeModelClass *klass )
{
	static const gchar *thisfn = "cact_tree_model_class_init";
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
					_( "Parent BaseWindow" ),
					_( "A pointer (not a reference) to the BaseWindow parent of the embedding treeview" ),
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, TREE_PROP_TREEVIEW_ID,
			g_param_spec_pointer(
					TREE_PROP_TREEVIEW,
					_( "Embedding GtkTreeView" ),
					_( "The GtkTreeView which relies on this CactTreeModel" ),
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, TREE_PROP_MODE_ID,
			g_param_spec_uint(
					TREE_PROP_MODE,
					_( "Edition mode" ),
					_( "Edition vs. Selection mode" ),
					0, TREE_MODE_N_MODES, 0,
					G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	klass->private = g_new0( CactTreeModelClassPrivate, 1 );
}

static void
imulti_drag_source_init( EggTreeMultiDragSourceIface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_tree_model_imulti_drag_source_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->row_draggable = cact_tree_model_dnd_imulti_drag_source_row_draggable;
	iface->drag_data_get = cact_tree_model_dnd_imulti_drag_source_drag_data_get;
	iface->drag_data_delete = cact_tree_model_dnd_imulti_drag_source_drag_data_delete;
	iface->get_target_list = cact_tree_model_dnd_imulti_drag_source_get_format_list;
	iface->free_target_list = NULL;
	iface->get_drag_actions = cact_tree_model_dnd_imulti_drag_source_get_drag_actions;
}

static void
idrag_dest_init( GtkTreeDragDestIface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_tree_model_idrag_dest_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->drag_data_received = cact_tree_model_dnd_idrag_dest_drag_data_received;
	iface->row_drop_possible = cact_tree_model_dnd_idrag_dest_row_drop_possible;
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_tree_model_instance_init";
	CactTreeModel *self;

	g_return_if_fail( CACT_IS_TREE_MODEL( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_TREE_MODEL( instance );

	self->private = g_new0( CactTreeModelPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	CactTreeModel *self;

	g_return_if_fail( CACT_IS_TREE_MODEL( object ));
	self = CACT_TREE_MODEL( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case TREE_PROP_WINDOW_ID:
				g_value_set_pointer( value, self->private->window );
				break;

			case TREE_PROP_TREEVIEW_ID:
				g_value_set_pointer( value, self->private->treeview );
				break;

			case TREE_PROP_MODE_ID:
				g_value_set_uint( value, self->private->mode );
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
	CactTreeModel *self;

	g_return_if_fail( CACT_IS_TREE_MODEL( object ));
	self = CACT_TREE_MODEL( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case TREE_PROP_WINDOW_ID:
				self->private->window = g_value_get_pointer( value );
				break;

			case TREE_PROP_TREEVIEW_ID:
				self->private->treeview = g_value_get_pointer( value );
				break;

			case TREE_PROP_MODE_ID:
				self->private->mode = g_value_get_uint( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

/*
 * Initializes the tree model.
 *
 * We use drag and drop:
 * - inside of treeview, for duplicating items, or moving items between menus
 * - from treeview to the outside world (e.g. Caja) to export actions
 * - from outside world (e.g. Caja) to import actions
 */
static void
instance_constructed( GObject *model )
{
	static const gchar *thisfn = "cact_tree_model_instance_constructed";
	CactTreeModelPrivate *priv;

	g_return_if_fail( CACT_IS_TREE_MODEL( model ));

	priv = CACT_TREE_MODEL( model )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( model );
		}

		g_debug( "%s: model=%p (%s)", thisfn, ( void * ) model, G_OBJECT_TYPE_NAME( model ));

		priv->clipboard = cact_clipboard_new( priv->window );

		if( priv->mode == TREE_MODE_EDITION ){

			egg_tree_multi_drag_add_drag_support(
					EGG_TREE_MULTI_DRAG_SOURCE( model ),
					priv->treeview );

			gtk_tree_view_enable_model_drag_dest(
					priv->treeview,
					tree_model_dnd_dest_formats,
					tree_model_dnd_dest_formats_count,
					cact_tree_model_dnd_imulti_drag_source_get_drag_actions( EGG_TREE_MULTI_DRAG_SOURCE( model )));

			base_window_signal_connect(
					priv->window,
					G_OBJECT( priv->treeview ),
					"drag-begin",
					G_CALLBACK( cact_tree_model_dnd_on_drag_begin ));

			/* connect: implies that we have to do all hard stuff
			 * connect_after: no more triggers drag-drop signal
			 */
			/*base_window_signal_connect_after( window,
					G_OBJECT( model->private->treeview ), "drag-motion", G_CALLBACK( on_drag_motion ));*/

			/*base_window_signal_connect( window,
					G_OBJECT( model->private->treeview ), "drag-drop", G_CALLBACK( on_drag_drop ));*/

			base_window_signal_connect(
					priv->window,
					G_OBJECT( priv->treeview ),
					"drag-end",
					G_CALLBACK( cact_tree_model_dnd_on_drag_end ));

			na_settings_register_key_callback(
					NA_IPREFS_ITEMS_LIST_ORDER_MODE,
					G_CALLBACK( on_settings_order_mode_changed ),
					model );

			base_window_signal_connect_with_data(
					priv->window,
					G_OBJECT( priv->window ),
					TAB_UPDATABLE_SIGNAL_ITEM_UPDATED,
					G_CALLBACK( on_tab_updatable_item_updated ),
					model );
		}

	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cact_tree_model_instance_dispose";
	CactTreeModel *self;
	GtkTreeStore *ts_model;

	g_return_if_fail( CACT_IS_TREE_MODEL( object ));

	self = CACT_TREE_MODEL( object );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		ts_model = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( self )));
		gtk_tree_store_clear( ts_model );
		g_debug( "%s: tree store cleared", thisfn );

		g_object_unref( self->private->clipboard );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "cact_tree_model_instance_finalize";
	CactTreeModel *self;

	g_return_if_fail( CACT_IS_TREE_MODEL( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = CACT_TREE_MODEL( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * cact_tree_model_new:
 * @window: a #BaseWindow window which embeds our CactTreeView
 * @treeview: the #GtkTreeView widget.
 * @mode: management mode.
 *
 * Returns: a newly created CactTreeModel object.
 *
 * The returned reference is owned by the #GtkTreeView, which will automatically
 * take care of g_object_unref() its tree model when destroying its widget.
 *
 * Called from CactTreeView::initialize_gtk() method
 *   [..]
 *     which happens to be eventually called from CactMainWindow::on_initialize_gtk()
 *     signal handler.
 */
CactTreeModel *
cact_tree_model_new( BaseWindow *window, GtkTreeView *treeview, CactTreeMode mode )
{
	static const gchar *thisfn = "cact_tree_model_new";
	GtkTreeStore *ts_model;
	CactTreeModel *model;
	gint order_mode;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );
	g_return_val_if_fail( GTK_IS_TREE_VIEW( treeview ), NULL );

	g_debug( "%s: window=%p, treeview=%p, mode=%u", thisfn, ( void * ) window, ( void * ) treeview, mode );

	ts_model = gtk_tree_store_new(
			TREE_N_COLUMN, GDK_TYPE_PIXBUF, G_TYPE_STRING, NA_TYPE_OBJECT );

	/* create the filter model
	 */
	model = g_object_new( CACT_TYPE_TREE_MODEL,
			"child-model",      ts_model,
			TREE_PROP_WINDOW,   window,
			TREE_PROP_TREEVIEW, treeview,
			TREE_PROP_MODE,     mode,
			NULL );
	g_object_unref( ts_model );

	gtk_tree_model_filter_set_visible_func(
			GTK_TREE_MODEL_FILTER( model ), ( GtkTreeModelFilterVisibleFunc ) filter_visible, model, NULL );

	/* initialize the sortable interface
	 */
	order_mode = na_iprefs_get_order_mode( NULL );
	display_order_change( model, order_mode );

	return( model );
}

/*
 * NASettings callback for a change on NA_IPREFS_ITEMS_LIST_ORDER_MODE key
 */
static void
on_settings_order_mode_changed( const gchar *group, const gchar *key, gconstpointer new_value, gboolean mandatory, CactTreeModel *model )
{
	static const gchar *thisfn = "cact_tree_model_on_settings_order_mode_changed";
	const gchar *order_mode_str;
	guint order_mode;

	g_return_if_fail( CACT_IS_TREE_MODEL( model ));

	if( !model->private->dispose_has_run ){

		order_mode_str = ( const gchar * ) new_value;
		order_mode = na_iprefs_get_order_mode_by_label( order_mode_str );

		g_debug( "%s: group=%s, key=%s, order_mode=%u (%s), mandatory=%s, model=%p (%s)",
				thisfn, group, key, order_mode, order_mode_str,
				mandatory ? "True":"False", ( void * ) model, G_OBJECT_TYPE_NAME( model ));

		display_order_change( model, order_mode );
	}
}

/*
 * if force_display is true, then refresh the display of the view
 */
static void
on_tab_updatable_item_updated( BaseWindow *window, NAIContext *context, guint data, CactTreeModel *model )
{
	static const gchar *thisfn = "cact_tree_model_on_tab_updatable_item_updated";
	GtkTreePath *path;
	GtkTreeStore *store;
	GtkTreeIter iter;

	if( !model->private->dispose_has_run ){

		g_debug( "%s: window=%p, context=%p (%s), data=%u, model=%p",
				thisfn,
				( void * ) window,
				( void * ) context, G_OBJECT_TYPE_NAME( context ),
				data,
				( void * ) model );

		if( data & ( MAIN_DATA_LABEL | MAIN_DATA_ICON )){
			path = cact_tree_model_object_to_path( model, ( NAObject * ) context );
			if( path ){
				store = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model )));
				if( gtk_tree_model_get_iter( GTK_TREE_MODEL( store ), &iter, path )){
					display_item( store, model->private->treeview, &iter, ( NAObject * ) context );
					gtk_tree_model_row_changed( GTK_TREE_MODEL( store ), path, &iter );
				}
				gtk_tree_path_free( path );
			}
		}
	}
}

/**
 * cact_tree_model_delete:
 * @model: this #CactTreeModel instance.
 * @object: the #NAObject to be deleted.
 *
 * Recursively deletes the specified object.
 *
 * Returns: a path which may be suitable for the next selection.
 */
GtkTreePath *
cact_tree_model_delete( CactTreeModel *model, NAObject *object )
{
	GtkTreePath *path;
	static const gchar *thisfn = "cact_tree_model_delete";
	GtkTreeIter iter;
	GtkTreeStore *store;
	NAObjectItem *parent;

	g_return_val_if_fail( CACT_IS_TREE_MODEL( model ), NULL );

	path = NULL;

	if( !model->private->dispose_has_run ){
		g_debug( "%s: model=%p, object=%p (%s)",
				thisfn, ( void * ) model, ( void * ) object, object ? G_OBJECT_TYPE_NAME( object ) : "null" );

		path = cact_tree_model_object_to_path( model, object );

		if( path != NULL ){

			/* detaching to-be-deleted object from its current parent
			 */
			parent = na_object_get_parent( object );
			g_debug( "%s: object=%p, parent=%p", thisfn, ( void * ) object, ( void * ) parent );
			if( parent ){
				na_object_remove_item( parent, object );
			}

			/* then recursively remove the object and its children from the store
			 */
			store = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model )));
			if( gtk_tree_model_get_iter( GTK_TREE_MODEL( store ), &iter, path )){
				delete_items_rec( store, &iter );
			}
		}
	}

	return( path );
}

/**
 * cact_tree_model_fill:
 * @model: this #CactTreeModel instance.
 * @ŧreeview: the #GtkTreeView widget.
 * @items: this list of items, usually from #NAPivot, which will be used
 *  to fill up the tree store.
 *
 * Fill up the tree store with specified items.
 *
 * We enter with the GSList owned by NAPivot which contains the ordered
 * list of level-zero items. We want have a duplicate of this list in
 * tree store, so that we are able to freely edit it.
 */
void
cact_tree_model_fill( CactTreeModel *model, GList *items )
{
	static const gchar *thisfn = "cact_tree_model_fill";
	GtkTreeStore *ts_model;
	GList *it;
	NAObject *duplicate;

	g_return_if_fail( CACT_IS_TREE_MODEL( model ));

	g_debug( "%s: model=%p, items=%p (count=%d)",
			thisfn, ( void * ) model, ( void * ) items, g_list_length( items ));

	if( !model->private->dispose_has_run ){

		ts_model = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model )));
		gtk_tree_store_clear( ts_model );

		for( it = items ; it ; it = it->next ){
			duplicate = ( NAObject * ) na_object_duplicate( it->data, DUPLICATE_REC );
			na_object_check_status( duplicate );
			fill_tree_store( ts_model, model->private->treeview, duplicate, NULL );
			na_object_unref( duplicate );
		}
	}
}

/**
 * cact_tree_model_insert_before:
 * @model: this #CactTreeModel instance.
 * @object: a #NAObject-derived object to be inserted.
 * @path: the #GtkTreePath which specifies the insertion path.
 * @parent: set to the parent or the object itself.
 *
 * Insert a new row at the given position.
 *
 * Gtk API uses to returns iter ; but at least when inserting a new
 * profile in an action, we may have store_iter_path="0:1" (good), but
 * iter_path="0:0" (bad) - so we'd rather return a string path.
 *
 * Returns: the actual insertion path, which may be different from the
 * asked insertion path if tree is sorted.
 */
GtkTreePath *
cact_tree_model_insert_before( CactTreeModel *model, const NAObject *object, GtkTreePath *path )
{
	static const gchar *thisfn = "cact_tree_model_insert_before";
	GtkTreeModel *store;
	gchar *path_str;
	GtkTreeIter iter;
	GtkTreeIter parent_iter;
	GtkTreePath *parent_path;
	GtkTreePath *inserted_path;
	NAObject *parent_obj;
	gboolean has_parent;
	GtkTreeIter sibling_iter;
	NAObject *sibling_obj;
	gboolean has_sibling;

	path_str = gtk_tree_path_to_string( path );
	g_debug( "%s: model=%p, object=%p (%s, ref_count=%d), path=%p (%s)",
			thisfn,
			( void * ) model,
			( void * ) object, G_OBJECT_TYPE_NAME( object ), G_OBJECT( object )->ref_count,
			( void * ) path, path_str );
	g_free( path_str );
	g_return_val_if_fail( CACT_IS_TREE_MODEL( model ), NULL );
	g_return_val_if_fail( NA_IS_OBJECT( object ), NULL );

	inserted_path = NULL;

	if( !model->private->dispose_has_run ){

		store = gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model ));
		has_parent = FALSE;
		parent_obj = NULL;
		sibling_obj = NULL;

		remove_if_exists( model, store, object );

		/* may be FALSE when store is empty */
		has_sibling = gtk_tree_model_get_iter( store, &sibling_iter, path );
		if( has_sibling ){
			gtk_tree_model_get( store, &sibling_iter, TREE_COLUMN_NAOBJECT, &sibling_obj, -1 );
			g_object_unref( sibling_obj );
		}
		g_debug( "%s: has_sibling=%s, sibling_obj=%p", thisfn, has_sibling ? "True":"False", ( void * ) sibling_obj );

		if( gtk_tree_path_get_depth( path ) > 1 ){

			has_parent = TRUE;
			parent_path = gtk_tree_path_copy( path );
			gtk_tree_path_up( parent_path );
			gtk_tree_model_get_iter( store, &parent_iter, parent_path );
			gtk_tree_path_free( parent_path );

			gtk_tree_model_get( store, &parent_iter, TREE_COLUMN_NAOBJECT, &parent_obj, -1 );
			g_object_unref( parent_obj );

			if( has_sibling ){
				na_object_insert_item( parent_obj, object, sibling_obj );
			} else {
				na_object_append_item( parent_obj, object );
			}

			na_object_set_parent( object, parent_obj );
		}
		g_debug( "%s: has_parent=%s, parent_obj=%p", thisfn, has_parent ? "True":"False", ( void * ) parent_obj );

		gtk_tree_store_insert_before(
				GTK_TREE_STORE( store ), &iter,
				has_parent ? &parent_iter : NULL,
				has_sibling ? &sibling_iter : NULL );
		gtk_tree_store_set( GTK_TREE_STORE( store ), &iter, TREE_COLUMN_NAOBJECT, object, -1 );
		display_item( GTK_TREE_STORE( store ), model->private->treeview, &iter, object );

		inserted_path = gtk_tree_model_get_path( store, &iter );
		path_str = gtk_tree_path_to_string( inserted_path );
		g_debug( "%s: object %p (%s) inserted at path %s",
				thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ), path_str );
		g_free( path_str );
	}

	return( inserted_path );
}

/**
 * cact_tree_model_insert_into:
 * @model: this #CactTreeModel instance.
 * @object: the #NAObject to be inserted.
 * @path: the wished #GtkTreePath path.
 *
 * Insert the @object at ou near the wished @path, and attaches the object
 * to its new parent.
 *
 * Returns the actual insertion path, wchich should be gtk_tree_path_free()
 * by the caller.
 */
GtkTreePath *
cact_tree_model_insert_into( CactTreeModel *model, const NAObject *object, GtkTreePath *path )
{
	static const gchar *thisfn = "cact_tree_model_insert_into";
	GtkTreeModel *store;
	GtkTreeIter iter;
	GtkTreeIter parent_iter;
	GtkTreePath *new_path;
	gchar *path_str;
	NAObject *parent;

	path_str = gtk_tree_path_to_string( path );
	g_debug( "%s: model=%p, object=%p (%s, ref_count=%d), path=%p (%s), parent=%p",
			thisfn,
			( void * ) model,
			( void * ) object, G_OBJECT_TYPE_NAME( object ), G_OBJECT( object )->ref_count,
			( void * ) path, path_str, ( void * ) parent );
	g_free( path_str );
	g_return_val_if_fail( CACT_IS_TREE_MODEL( model ), NULL );
	g_return_val_if_fail( NA_IS_OBJECT( object ), NULL );

	new_path = NULL;

	if( !model->private->dispose_has_run ){

		store = gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model ));

		if( !gtk_tree_model_get_iter( store, &parent_iter, path )){
			path_str = gtk_tree_path_to_string( path );
			g_warning( "%s: unable to get iter at path %s", thisfn, path_str );
			g_free( path_str );
			return( NULL );
		}

		gtk_tree_model_get( store, &parent_iter, TREE_COLUMN_NAOBJECT, &parent, -1 );
		g_object_unref( parent );
		na_object_insert_item( parent, object, NULL );
		na_object_set_parent( object, parent );

		gtk_tree_store_insert_after( GTK_TREE_STORE( store ), &iter, &parent_iter, NULL );
		gtk_tree_store_set( GTK_TREE_STORE( store ), &iter, TREE_COLUMN_NAOBJECT, object, -1 );
		display_item( GTK_TREE_STORE( store ), model->private->treeview, &iter, object );

		new_path = gtk_tree_model_get_path( store, &iter );
		path_str = gtk_tree_path_to_string( new_path );
		g_debug( "%s: object %p (%s) inserted at path %s",
				thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ), path_str );
		g_free( path_str );
	}

	return( new_path );
}

/**
 * cact_tree_model_get_item_by_id:
 * @model: this #CactTreeModel object.
 * @id: the searched #NAObjectItem.
 *
 * Returns: a pointer on the searched #NAObjectItem if it exists, or %NULL.
 *
 * The returned pointer is owned by the underlying tree store, and should
 * not be released by the caller.
 */
NAObjectItem *
cact_tree_model_get_item_by_id( const CactTreeModel *model, const gchar *id )
{
	static const gchar *thisfn = "cact_tree_model_get_item_by_id";
	GtkTreeStore *store;
	ntmFindId nfi;

	g_return_val_if_fail( CACT_IS_TREE_MODEL( model ), NULL );

	nfi.object = NULL;

	if( !model->private->dispose_has_run ){
		g_debug( "%s: model=%p, id=%s", thisfn, ( void * ) model, id );

		nfi.id = ( gchar * ) id;
		store = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model )));
		iter_on_store( model, GTK_TREE_MODEL( store ), NULL, ( FnIterOnStore ) find_item_iter, &nfi );
	}

	return(( NAObjectItem * ) nfi.object );
}

/**
 * cact_tree_model_get_items:
 * @model: this #CactTreeModel object.
 * @mode: the content indicator for the returned list
 *
 * Returns: the content of the current store as a newly allocated list
 * which should be na_object_free_items() by the caller.
 */
GList *
cact_tree_model_get_items( const CactTreeModel *model, guint mode )
{
	static const gchar *thisfn = "cact_tree_model_get_items";
	GList *items;
	GtkTreeStore *store;
	ntmGetItems ngi;

	g_return_val_if_fail( CACT_IS_TREE_MODEL( model ), NULL );

	items = NULL;

	if( !model->private->dispose_has_run ){
		g_debug( "%s: model=%p, mode=0x%xh", thisfn, ( void * ) model, mode );

		ngi.mode = mode;
		ngi.items = NULL;
		store = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model )));
		iter_on_store( model, GTK_TREE_MODEL( store ), NULL, ( FnIterOnStore ) get_items_iter, &ngi );
		items = g_list_reverse( ngi.items );
	}

	return( items );
}

/**
 * cact_tree_model_object_at_path:
 * @model: this #CactTreeModel instance.
 * @path: the #GtkTreePath to be searched.
 *
 * Returns: the #NAObject at the given @path if any, or NULL.
 *
 * The reference count of the object is not modified. The returned reference
 * is owned by the tree store and should not be released by the caller.
 */
NAObject *
cact_tree_model_object_at_path( const CactTreeModel *model, GtkTreePath *path )
{
	NAObject *object;
	GtkTreeModel *store;
	GtkTreeIter iter;

	g_return_val_if_fail( CACT_IS_TREE_MODEL( model ), NULL );

	object = NULL;

	if( !model->private->dispose_has_run ){

		store = gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model ));
		if( gtk_tree_model_get_iter( store, &iter, path )){
			gtk_tree_model_get( store, &iter, TREE_COLUMN_NAOBJECT, &object, -1 );
			g_object_unref( object );
		}
	}

	return( object );
}

/**
 * cact_tree_model_object_to_path:
 * @model: this #CactTreeModel.
 * @object: the searched NAObject.
 *
 * Returns: a newly allocated GtkTreePath which is the current position
 * of @object in the tree store, or %NULL.
 *
 * The returned path should be gtk_tree_path_free() by the caller.
 */
GtkTreePath *
cact_tree_model_object_to_path( const CactTreeModel *model, const NAObject *object )
{
	static const gchar *thisfn = "cact_tree_model_object_to_path";
	ntmFindObject nfo;
	GtkTreeIter iter;
	GtkTreeStore *store;

	g_return_val_if_fail( CACT_IS_TREE_MODEL( model ), NULL );

	nfo.path = NULL;

	if( !model->private->dispose_has_run ){
		g_debug( "%s: model=%p, object=%p (%s)",
				thisfn, ( void * ) model, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		nfo.object = object;
		nfo.iter = &iter;

		store = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model )));
		iter_on_store( model, GTK_TREE_MODEL( store ), NULL, ( FnIterOnStore ) find_object_iter, &nfo );
	}

	return( nfo.path );
}

static void
append_item( GtkTreeStore *model, GtkTreeView *treeview, GtkTreeIter *parent, GtkTreeIter *iter, const NAObject *object )
{
	/*g_debug( "cact_tree_model_append_item: object=%p (ref_count=%d), parent=%p",
					( void * ) object, G_OBJECT( object )->ref_count, ( void * ) parent );*/

	gtk_tree_store_append( model, iter, parent );
	gtk_tree_store_set( model, iter, TREE_COLUMN_NAOBJECT, object, -1 );
	display_item( model, treeview, iter, object );
}

static void
display_item( GtkTreeStore *model, GtkTreeView *treeview, GtkTreeIter *iter, const NAObject *object )
{
	gchar *label = na_object_get_label( object );
	gtk_tree_store_set( model, iter, TREE_COLUMN_LABEL, label, -1 );
	g_free( label );

	if( NA_IS_OBJECT_ITEM( object )){
		gchar *icon_name = na_object_get_icon( object );
		GdkPixbuf *icon = base_gtk_utils_get_pixbuf( icon_name, GTK_WIDGET( treeview ), GTK_ICON_SIZE_MENU );
		gtk_tree_store_set( model, iter, TREE_COLUMN_ICON, icon, -1 );
		g_object_unref( icon );
	}
}

/*
 * cact_tree_model_display_order_change:
 * @model: this #CactTreeModel.
 * @order_mode: the new order mode.
 *
 * Setup the new order mode.
 */
static void
display_order_change( CactTreeModel *model, gint order_mode )
{
	GtkTreeStore *store;

	g_return_if_fail( CACT_IS_TREE_MODEL( model ));

	if( !model->private->dispose_has_run ){

		store = GTK_TREE_STORE( gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model )));
		g_object_set_data( G_OBJECT( store ), TREE_MODEL_ORDER_MODE, GINT_TO_POINTER( order_mode ));

		switch( order_mode ){

			case IPREFS_ORDER_ALPHA_ASCENDING:

				gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( store ),
						TREE_COLUMN_LABEL, GTK_SORT_ASCENDING );

				gtk_tree_sortable_set_sort_func( GTK_TREE_SORTABLE( store ),
						TREE_COLUMN_LABEL, ( GtkTreeIterCompareFunc ) sort_actions_list, NULL, NULL );
				break;

			case IPREFS_ORDER_ALPHA_DESCENDING:

				gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( store ),
						TREE_COLUMN_LABEL, GTK_SORT_DESCENDING );

				gtk_tree_sortable_set_sort_func( GTK_TREE_SORTABLE( store ),
						TREE_COLUMN_LABEL, ( GtkTreeIterCompareFunc ) sort_actions_list, NULL, NULL );
				break;

			case IPREFS_ORDER_MANUAL:
			default:

				gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE( store ),
						GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, 0 );
				break;
		}
	}
}

#if 0
/*
 * dump:
 * @model: this #CactTreeModel instance.
 *
 * Briefly dumps the content of the tree.
 */
static void
dump( CactTreeModel *model )
{
	GList *items;

	items = cact_tree_model_get_items( model, TREE_LIST_ALL );
	na_object_dump_tree( items );
	na_object_free_items( items );
}

static gboolean
dump_store( CactTreeModel *model, GtkTreePath *path, NAObject *object, ntmDumpStruct *ntm )
{
	gint depth;
	gint i;
	GString *prefix;
	gchar *id, *label;
	NAObjectItem *origin;

	depth = gtk_tree_path_get_depth( path );
	prefix = g_string_new( ntm->prefix );
	for( i=1 ; i<depth ; ++i ){
		g_string_append_printf( prefix, "  " );
	}

	id = na_object_get_id( object );
	label = na_object_get_label( object );
	origin = ( NAObjectItem * ) na_object_get_origin( object );
	g_debug( "%s: %s%s at %p (ref_count=%d) \"[%s] %s\" origin=%p (%s)",
			ntm->fname, prefix->str,
			G_OBJECT_TYPE_NAME( object ), ( void * ) object, G_OBJECT( object )->ref_count, id, label,
			( void * ) origin, origin ? G_OBJECT_TYPE_NAME( object ) : "null" );
	g_free( label );
	g_free( id );

	g_string_free( prefix, TRUE );

	/* don't stop iteration */
	return( FALSE );
}
#endif

static void
fill_tree_store( GtkTreeStore *model, GtkTreeView *treeview, NAObject *object, GtkTreeIter *parent )
{
	static const gchar *thisfn = "cact_tree_model_fill_tree_store";
	GList *subitems, *it;
	GtkTreeIter iter;

	g_debug( "%s entering: object=%p (%s, ref_count=%d)", thisfn,
			( void * ) object, G_OBJECT_TYPE_NAME( object ), G_OBJECT( object )->ref_count );

	/* an action or a menu
	 */
	if( NA_IS_OBJECT_ITEM( object )){
		append_item( model, treeview, parent, &iter, object );
		subitems = na_object_get_items( object );
		for( it = subitems ; it ; it = it->next ){
			fill_tree_store( model, treeview, it->data, &iter );
		}

	} else {
		g_return_if_fail( NA_IS_OBJECT_PROFILE( object ));
		append_item( model, treeview, parent, &iter, object );
	}

	/*g_debug( "%s quitting: object=%p (%s, ref_count=%d)", thisfn,
			( void * ) object, G_OBJECT_TYPE_NAME( object ), G_OBJECT( object )->ref_count );*/
}

/*
 * Only display profiles when we are in edition mode.
 *
 * This function is called as soon as a new row is created in the tree store,
 * so is called the first time _before_ the NAObject be set on the row.
 */
static gboolean
filter_visible( GtkTreeModel *store, GtkTreeIter *iter, CactTreeModel *model )
{
	NAObject *object;
	NAObjectAction *action;
	gint count;

	gtk_tree_model_get( store, iter, TREE_COLUMN_NAOBJECT, &object, -1 );

	if( object ){
		g_object_unref( object );

		/* an action or a menu are always displayed, whatever the current
		 * management mode may be
		 */
		if( NA_IS_OBJECT_ITEM( object )){
			return( TRUE );
		}

		/* profiles are just never displayed in selection mode
		 * in edition mode, they are displayed only when the action has
		 * more than one profile
		 */
		g_return_val_if_fail( NA_IS_OBJECT_PROFILE( object ), FALSE );

		if( CACT_TREE_MODEL( model )->private->mode == TREE_MODE_SELECTION ){
			return( FALSE );
		}

		action = NA_OBJECT_ACTION( na_object_get_parent( object ));
		count = na_object_get_items_count( action );
		return( count > 1 );
	}

	return( FALSE );
}

/*
 * search for an object, given its id
 */
static gboolean
find_item_iter( CactTreeModel *model, GtkTreeStore *store, GtkTreePath *path, NAObject *object, ntmFindId *nfi )
{
	gchar *id;
	gboolean found = FALSE;

	if( NA_IS_OBJECT_ITEM( object )){
		id = na_object_get_id( object );
		found = ( g_ascii_strcasecmp( id, nfi->id ) == 0 );
		g_free( id );

		if( found ){
			nfi->object = object;
		}
	}

	/* stop iteration if found */
	return( found );
}

static gboolean
find_object_iter( CactTreeModel *model, GtkTreeStore *store, GtkTreePath *path, NAObject *object, ntmFindObject *nfo )
{
	if( object == nfo->object ){
		if( gtk_tree_model_get_iter( GTK_TREE_MODEL( store ), nfo->iter, path )){
			nfo->path = gtk_tree_path_copy( path );
		}
	}

	/* stop iteration when found */
	return( nfo->path != NULL );
}

/*
 * Builds the tree by iterating on the store
 * we may want selected, modified or both, or a combination of these modes
 *
 * This function is called from iter_on_store_item();
 */
static gboolean
get_items_iter( const CactTreeModel *model, GtkTreeStore *store, GtkTreePath *path, NAObject *object, ntmGetItems *ngi )
{
	if( ngi->mode & TREE_LIST_ALL ){
		if( gtk_tree_path_get_depth( path ) == 1 ){
			ngi->items = g_list_prepend( ngi->items, na_object_ref( object ));
		}
	}

	/* don't stop iteration */
	return( FALSE );
}

static void
iter_on_store( const CactTreeModel *model, GtkTreeModel *store, GtkTreeIter *parent, FnIterOnStore fn, gpointer user_data )
{
	GtkTreeIter iter;
	gboolean stop;

	if( gtk_tree_model_iter_children( store, &iter, parent )){
		stop = iter_on_store_item( model, store, &iter, fn, user_data );

		while( !stop && gtk_tree_model_iter_next( store, &iter )){
			stop = iter_on_store_item( model, store, &iter, fn, user_data );
		}
	}
}

static gboolean
iter_on_store_item( const CactTreeModel *model, GtkTreeModel *store, GtkTreeIter *iter, FnIterOnStore fn, gpointer user_data )
{
	NAObject *object;
	GtkTreePath *path;
	gboolean stop;

	/* unreffing as soon as we got the pointer so that the ref count is
	 * unchanged in dump_store
	 */
	gtk_tree_model_get( store, iter, TREE_COLUMN_NAOBJECT, &object, -1 );
	g_object_unref( object );

	/*
	g_debug( "cact_tree_model_iter_on_store_item: object=%p (%s, ref_count=%d)",
			( void * ) object, G_OBJECT_TYPE_NAME( object ), G_OBJECT( object )->ref_count );
			*/

	path = gtk_tree_model_get_path( store, iter );
	stop = ( *fn )( model, GTK_TREE_STORE( store ), path, object, user_data );
	gtk_tree_path_free( path );

	if( !stop ){
		iter_on_store( model, store, iter, fn, user_data );
	}

	return( stop );
}

/*
 * if the object, identified by its id (historically a uuid), already exists,
 * then remove it first
 */
static void
remove_if_exists( CactTreeModel *model, GtkTreeModel *store, const NAObject *object )
{
	ntmFindId nfi;
	GtkTreeIter iter;

	if( NA_IS_OBJECT_ITEM( object )){

		nfi.id = na_object_get_id( object );
		nfi.object = NULL;
		nfi.iter = &iter;

		iter_on_store( model, store, NULL, ( FnIterOnStore ) find_item_iter, &nfi );

		if( nfi.object ){
			g_debug( "cact_tree_model_remove_if_exists: removing %s %p",
					G_OBJECT_TYPE_NAME( object ), ( void * ) object );
			gtk_tree_store_remove( GTK_TREE_STORE( store ), nfi.iter );
		}

		g_free( nfi.id );
	}
}

/*
 * recursively delete child items starting with iter
 * returns TRUE if iter is always valid after the remove
 */
static gboolean
delete_items_rec( GtkTreeStore *store, GtkTreeIter *iter )
{
	GtkTreeIter child;
	gboolean valid;

	while( gtk_tree_model_iter_children( GTK_TREE_MODEL( store ), &child, iter )){
		delete_items_rec( store, &child );
	}
	valid = gtk_tree_store_remove( store, iter );

	return( valid );
}

static gint
sort_actions_list( GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data )
{
	/*static const gchar *thisfn = "cact_tree_model_sort_actions_list";*/
	NAObjectId *obj_a, *obj_b;
	gint ret;

	/*g_debug( "%s: model=%p, a=%p, b=%p, window=%p", thisfn, ( void * ) model, ( void * ) a, ( void * ) b, ( void * ) window );*/

	gtk_tree_model_get( model, a, TREE_COLUMN_NAOBJECT, &obj_a, -1 );
	gtk_tree_model_get( model, b, TREE_COLUMN_NAOBJECT, &obj_b, -1 );

	g_object_unref( obj_b );
	g_object_unref( obj_a );

	if( NA_IS_OBJECT_PROFILE( obj_a )){
		ret = 0;
	} else {
		ret = na_object_sort_alpha_asc( obj_a, obj_b );
	}

	/*g_debug( "%s: ret=%d", thisfn, ret );*/
	return( ret );
}
