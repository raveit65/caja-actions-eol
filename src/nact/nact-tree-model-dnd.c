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

#include <mateconf/mateconf-client.h>
#include <glib/gi18n.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-iprefs.h>
#include <core/na-mate-vfs-uri.h>
#include <core/na-importer.h>

#include "nact-application.h"
#include "nact-clipboard.h"
#include "nact-iactions-list.h"
#include "nact-iprefs.h"
#include "nact-main-menubar-edit.h"
#include "nact-main-statusbar.h"
#include "nact-main-window.h"
#include "nact-tree-model.h"
#include "nact-tree-model-dnd.h"
#include "nact-tree-model-priv.h"

/*
 * call once egg_tree_multi_drag_add_drag_support( treeview ) at init time (before gtk_main)
 *
 * when we start with drag
 * 	 call once egg_tree_multi_dnd_on_button_press_event( treeview, event, drag_source )
 *   call many egg_tree_multi_dnd_on_motion_event( treeview, event, drag_source )
 *     until mouse quits the selected area
 *
 * as soon as mouse has quitted the selected area
 *   call once egg_tree_multi_dnd_stop_drag_check( treeview )
 *   call once nact_tree_model_imulti_drag_source_row_draggable: drag_source=0x92a0d70, path_list=0x9373c90
 *   call once nact_clipboard_on_drag_begin( treeview, context, main_window )
 *
 * when we drop (e.g. in Caja)
 *   call once egg_tree_multi_dnd_on_drag_data_get( treeview, context, selection_data, info=0, time )
 *   call once nact_tree_model_imulti_drag_source_drag_data_get( drag_source, context, selection_data, path_list, atom=XdndDirectSave0 )
 *   call once nact_tree_model_idrag_dest_drag_data_received
 *   call once nact_clipboard_on_drag_end( treeview, context, main_window )
 *
 * when we drop in Caja Actions
 *   call once egg_tree_multi_dnd_on_drag_data_get( treeview, context, selection_data, info=0, time )
 *   call once nact_tree_model_imulti_drag_source_drag_data_get( drag_source, context, selection_data, path_list, atom=XdndCajaActions )
 *   call once nact_clipboard_get_data_for_intern_use
 *   call once nact_tree_model_idrag_dest_drag_data_received
 *   call once nact_clipboard_on_drag_end( treeview, context, main_window )
 */

#define MAX_XDS_ATOM_VAL_LEN			4096
#define TEXT_ATOM						gdk_atom_intern( "text/plain", FALSE )
#define XDS_ATOM						gdk_atom_intern( "XdndDirectSave0", FALSE )
#define XDS_FILENAME					"xds.txt"

#define NACT_ATOM						gdk_atom_intern( "XdndCajaActions", FALSE )

/* as a dnd source, we provide
 * - a special XdndCajaAction format for internal move/copy
 * - a XdndDirectSave, suitable for exporting to a file manager
 *   (note that Caja recognized the "XdndDirectSave0" format as XDS
 *   protocol)
 * - a text (xml) format, to go to clipboard or a text editor
 */
static GtkTargetEntry dnd_source_formats[] = {
	{ "XdndCajaActions", GTK_TARGET_SAME_WIDGET, NACT_XCHANGE_FORMAT_NACT },
	{ "XdndDirectSave0",     GTK_TARGET_OTHER_APP,   NACT_XCHANGE_FORMAT_XDS },
	{ "application/xml",     GTK_TARGET_OTHER_APP,   NACT_XCHANGE_FORMAT_APPLICATION_XML },
	{ "text/plain",          GTK_TARGET_OTHER_APP,   NACT_XCHANGE_FORMAT_TEXT_PLAIN },
};

/* as a dnd dest, we accept
 * - of course, the same special XdndCajaAction format for internal move/copy
 * - a list of uris, to be imported
 * - a XML buffer, to be imported
 * - a plain text, which we are goint to try to import as a XML description
 */
GtkTargetEntry tree_model_dnd_dest_formats[] = {
	{ "XdndCajaActions", 0, NACT_XCHANGE_FORMAT_NACT },
	{ "text/uri-list",       0, NACT_XCHANGE_FORMAT_URI_LIST },
	{ "application/xml",     0, NACT_XCHANGE_FORMAT_APPLICATION_XML },
	{ "text/plain",          0, NACT_XCHANGE_FORMAT_TEXT_PLAIN },
};

guint tree_model_dnd_dest_formats_count = G_N_ELEMENTS( tree_model_dnd_dest_formats );

static const gchar *st_refuse_drop_profile = N_( "Unable to drop a profile here" );
static const gchar *st_refuse_drop_item = N_( "Unable to drop an action or a menu here" );
static const gchar *st_parent_not_writable = N_( "Unable to drop here as parent is not writable" );
static const gchar *st_level_zero_not_writable = N_( "Unable to drop here as level zero is not writable" );

static gboolean      drop_inside( NactTreeModel *model, GtkTreePath *dest, GtkSelectionData  *selection_data );
static gboolean      is_drop_possible( NactTreeModel *model, GtkTreePath *dest, NAObjectItem **parent );
static gboolean      is_drop_possible_before_iter( NactTreeModel *model, GtkTreeIter *iter, NactMainWindow *window, NAObjectItem **parent );
static gboolean      is_drop_possible_into_dest( NactTreeModel *model, GtkTreePath *dest, NactMainWindow *window, NAObjectItem **parent );
static void          drop_inside_move_dest( NactTreeModel *model, GList *rows, GtkTreePath **dest );
static gboolean      drop_uri_list( NactTreeModel *model, GtkTreePath *dest, GtkSelectionData  *selection_data );
static NAObjectItem *is_dropped_already_exists( const NAObjectItem *importing, const NactMainWindow *window );
static char         *get_xds_atom_value( GdkDragContext *context );
static gboolean      is_parent_accept_new_childs( NactApplication *application, NactMainWindow *window, NAObjectItem *parent );
static guint         target_atom_to_id( GdkAtom atom );

/**
 * nact_tree_model_dnd_idrag_dest_drag_data_received:
 * @drag_dest:
 * @dest:
 * @selection_data:
 *
 * Called when a drop from the outside occurs in the treeview;
 * this may be an import action, or a move/copy inside of the tree.
 *
 * Returns: %TRUE if the specified rows were successfully inserted at
 * the given dest, %FALSE else.
 *
 * When importing:
 * - selection=XdndSelection
 * - target=text/uri-list
 * - type=text/uri-list
 *
 * When moving/copy from the treeview to the treeview:
 * - selection=XdndSelection
 * - target=XdndCajaActions
 * - type=XdndCajaActions
 */
gboolean
nact_tree_model_dnd_idrag_dest_drag_data_received( GtkTreeDragDest *drag_dest, GtkTreePath *dest, GtkSelectionData  *selection_data )
{
	static const gchar *thisfn = "nact_tree_model_dnd_idrag_dest_drag_data_received";
	gboolean result = FALSE;
	gchar *atom_name;
	guint info;
	gchar *path_str;

	g_debug( "%s: drag_dest=%p, dest=%p, selection_data=%p", thisfn, ( void * ) drag_dest, ( void * ) dest, ( void * ) selection_data );
	g_return_val_if_fail( NACT_IS_TREE_MODEL( drag_dest ), FALSE );

	atom_name = gdk_atom_name( selection_data->selection );
	g_debug( "%s: selection=%s", thisfn, atom_name );
	g_free( atom_name );

	atom_name = gdk_atom_name( selection_data->target );
	g_debug( "%s: target=%s", thisfn, atom_name );
	g_free( atom_name );

	atom_name = gdk_atom_name( selection_data->type );
	g_debug( "%s: type=%s", thisfn, atom_name );
	g_free( atom_name );

	g_debug( "%s: format=%d, length=%d", thisfn, selection_data->format, selection_data->length );

	info = target_atom_to_id( selection_data->type );
	g_debug( "%s: info=%u", thisfn, info );

	path_str = gtk_tree_path_to_string( dest );
	g_debug( "%s: dest_path=%s", thisfn, path_str );
	g_free( path_str );

	switch( info ){
		case NACT_XCHANGE_FORMAT_NACT:
			result = drop_inside( NACT_TREE_MODEL( drag_dest ), dest, selection_data );
			break;

		/* drop some actions from outside
		 * most probably from the file manager as a list of uris
		 */
		case NACT_XCHANGE_FORMAT_URI_LIST:
			result = drop_uri_list( NACT_TREE_MODEL( drag_dest ), dest, selection_data );
			break;

		default:
			break;
	}

	return( result );
}

/**
 * nact_tree_model_dnd_idrag_dest_row_drop_possible:
 * @drag_dest:
 * @dest_path:
 * @selection_data:
 *
 * Seems to only be called when the drop in _on_ a row (a square is
 * displayed), but not when dropped between two rows (a line is displayed),
 * nor during the motion.
 */
gboolean
nact_tree_model_dnd_idrag_dest_row_drop_possible( GtkTreeDragDest *drag_dest, GtkTreePath *dest_path, GtkSelectionData *selection_data )
{
	static const gchar *thisfn = "nact_tree_model_dnd_idrag_dest_row_drop_possible";

	g_debug( "%s: drag_dest=%p, dest_path=%p, selection_data=%p", thisfn, ( void * ) drag_dest, ( void * ) dest_path, ( void * ) selection_data );

	return( TRUE );
}

/**
 * nact_tree_model_dnd_imulti_drag_source_drag_data_get:
 * @context: contains
 *  - the suggested action, as choosen by the drop site,
 *    between those we have provided in imulti_drag_source_get_drag_actions()
 *  - the target folder (XDS protocol)
 * @selection_data:
 * @rows: list of row references which are to be dropped
 * @info: the suggested format, as choosen by the drop site, between those
 *  we have provided in imulti_drag_source_get_target_list()
 *
 * This function is called when we release the selected items onto the
 * destination
 *
 * NACT_XCHANGE_FORMAT_NACT:
 * internal format for drag and drop inside the treeview:
 * - copy in the clipboard the list of row references
 * - selection data is empty
 *
 * NACT_XCHANGE_FORMAT_XDS:
 * exchange format to drop to outside:
 * - copy in the clipboard the list of row references
 * - set the destination folder
 * - selection data is 'success' or 'failure'
 *
 * NACT_XCHANGE_FORMAT_APPLICATION_XML:
 * NACT_XCHANGE_FORMAT_TEXT_PLAIN:
 * exchange format to export to outside:
 * - do not use dnd clipboard
 * - selection data receives the export in text format
 *
 * Returns: %TRUE if required data was actually provided by the source,
 * %FALSE else.
 */
gboolean
nact_tree_model_dnd_imulti_drag_source_drag_data_get( EggTreeMultiDragSource *drag_source,
				   GdkDragContext         *context,
				   GtkSelectionData       *selection_data,
				   GList                  *rows,
				   guint                   info )
{
	static const gchar *thisfn = "nact_tree_model_dnd_imulti_drag_source_drag_data_get";
	gchar *atom_name;
	NactTreeModel *model;
	gchar *data;
	gboolean ret = FALSE;
	gchar *dest_folder, *folder;
	gboolean is_writable;
	gboolean copy_data;
	NAMateVFSURI *vfs;

	atom_name = gdk_atom_name( selection_data->target );
	g_debug( "%s: drag_source=%p, context=%p, action=%d, selection_data=%p, rows=%p, atom=%s",
			thisfn, ( void * ) drag_source, ( void * ) context, ( int ) context->suggested_action, ( void * ) selection_data, ( void * ) rows,
			atom_name );
	g_free( atom_name );

	model = NACT_TREE_MODEL( drag_source );
	g_return_val_if_fail( model->private->window, FALSE );

	if( !model->private->dispose_has_run ){

		if( !rows || !g_list_length( rows )){
			return( FALSE );
		}

		switch( info ){
			case NACT_XCHANGE_FORMAT_NACT:
				copy_data = ( context->action == GDK_ACTION_COPY );
				gtk_selection_data_set( selection_data, selection_data->target, 8, ( guchar * ) "", 0 );
				nact_clipboard_dnd_set( model->private->clipboard, info, rows, NULL, copy_data );
				ret = TRUE;
				break;

			case NACT_XCHANGE_FORMAT_XDS:
				/* get the dest default filename as an uri
				 * e.g. file:///home/pierre/data/eclipse/caja-actions/trash/xds.txt
				 */
				folder = get_xds_atom_value( context );
				/* get the dest folder as a path
				 */
				vfs = g_new0( NAMateVFSURI, 1 );
				na_mate_vfs_uri_parse( vfs, folder );
				dest_folder = g_path_get_dirname( vfs->path );
				na_mate_vfs_uri_free( vfs );
				g_free( folder );
				/* check that target folder is writable
				 */
				is_writable = na_core_utils_dir_is_writable_path( dest_folder );
				gtk_selection_data_set( selection_data, selection_data->target, 8, ( guchar * )( is_writable ? "S" : "F" ), 1 );
				if( is_writable ){
					nact_clipboard_dnd_set( model->private->clipboard, info, rows, dest_folder, TRUE );
				}
				g_free( dest_folder );
				ret = is_writable;
				break;

			case NACT_XCHANGE_FORMAT_APPLICATION_XML:
			case NACT_XCHANGE_FORMAT_TEXT_PLAIN:
				data = nact_clipboard_dnd_get_text( model->private->clipboard, rows );
				gtk_selection_data_set( selection_data, selection_data->target, 8, ( guchar * ) data, strlen( data ));
				g_free( data );
				ret = TRUE;
				break;

			default:
				break;
		}
	}

	return( ret );
}

/**
 * nact_tree_model_dnd_imulti_drag_source_drag_data_delete:
 * @drag_source:
 * @rows:
 */
gboolean
nact_tree_model_dnd_imulti_drag_source_drag_data_delete( EggTreeMultiDragSource *drag_source, GList *rows )
{
	static const gchar *thisfn = "nact_tree_model_dnd_imulti_drag_source_drag_data_delete";

	g_debug( "%s: drag_source=%p, path_list=%p", thisfn, ( void * ) drag_source, ( void * ) rows );

	return( TRUE );
}

/**
 * nact_tree_model_dnd_imulti_drag_source_get_drag_actions:
 * @drag_source:
 */
GdkDragAction
nact_tree_model_dnd_imulti_drag_source_get_drag_actions( EggTreeMultiDragSource *drag_source )
{
	return( GDK_ACTION_COPY | GDK_ACTION_MOVE );
}

GtkTargetList *
nact_tree_model_dnd_imulti_drag_source_get_format_list( EggTreeMultiDragSource *drag_source )
{
	GtkTargetList *target_list;

	target_list = gtk_target_list_new( dnd_source_formats, G_N_ELEMENTS( dnd_source_formats ));

	return( target_list );
}

/**
 * nact_tree_model_dnd_imulti_drag_source_row_draggable:
 * @drag_source:
 * @rows:
 *
 * All selectable rows are draggable.
 * Nonetheless, it's a good place to store the dragged row references.
 * We only make use of them in on_drag_motion handler.
 */
gboolean
nact_tree_model_dnd_imulti_drag_source_row_draggable( EggTreeMultiDragSource *drag_source, GList *rows )
{
	static const gchar *thisfn = "nact_tree_model_dnd_imulti_drag_source_row_draggable";
	NactTreeModel *model;
	GtkTreeModel *store;
	GtkTreePath *path;
	GtkTreeIter iter;
	NAObject *object;
	GList *it;

	g_debug( "%s: drag_source=%p, rows=%p (%d items)",
			thisfn, ( void * ) drag_source, ( void * ) rows, g_list_length( rows ));

	g_return_val_if_fail( NACT_IS_TREE_MODEL( drag_source ), FALSE );
	model = NACT_TREE_MODEL( drag_source );

	if( !model->private->dispose_has_run ){

		model->private->drag_has_profiles = FALSE;
		store = gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model ));

		for( it = rows ; it && !model->private->drag_has_profiles ; it = it->next ){

			path = gtk_tree_row_reference_get_path(( GtkTreeRowReference * ) it->data );
			gtk_tree_model_get_iter( store, &iter, path );
			gtk_tree_model_get( store, &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );

			if( NA_IS_OBJECT_PROFILE( object )){
				model->private->drag_has_profiles = TRUE;
			}

			g_object_unref( object );
			gtk_tree_path_free( path );
		}
	}

	return( TRUE );
}

/**
 * nact_tree_model_dnd_on_drag_begin:
 * @widget: the GtkTreeView from which item is to be dragged.
 * @context:
 * @window: the parent #NactMainWindow instance.
 *
 * This function is called once, at the beginning of the drag operation,
 * when we are dragging from the IActionsList treeview.
 */
void
nact_tree_model_dnd_on_drag_begin( GtkWidget *widget, GdkDragContext *context, BaseWindow *window )
{
	static const gchar *thisfn = "nact_tree_model_dnd_on_drag_begin";
	NactTreeModel *model;

	g_debug( "%s: widget=%p, context=%p, window=%p",
			thisfn, ( void * ) widget, ( void * ) context, ( void * ) window );

	model = NACT_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( widget )));
	g_return_if_fail( NACT_IS_TREE_MODEL( model ));

	if( !model->private->dispose_has_run ){

		model->private->drag_highlight = FALSE;
		model->private->drag_drop = FALSE;

		nact_clipboard_dnd_clear( model->private->clipboard );

		gdk_property_change(
				context->source_window,
				XDS_ATOM, TEXT_ATOM, 8, GDK_PROP_MODE_REPLACE, ( guchar * ) XDS_FILENAME, strlen( XDS_FILENAME ));
	}
}

/**
 * nact_tree_model_dnd_on_drag_end:
 * @widget:
 * @context:
 * @window:
 */
void
nact_tree_model_dnd_on_drag_end( GtkWidget *widget, GdkDragContext *context, BaseWindow *window )
{
	static const gchar *thisfn = "nact_tree_model_dnd_on_drag_end";
	NactTreeModel *model;

	g_debug( "%s: widget=%p, context=%p, window=%p",
			thisfn, ( void * ) widget, ( void * ) context, ( void * ) window );

	model = NACT_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( widget )));
	g_return_if_fail( NACT_IS_TREE_MODEL( model ));

	if( !model->private->dispose_has_run ){

		nact_clipboard_dnd_drag_end( model->private->clipboard );
		nact_clipboard_dnd_clear( model->private->clipboard );
		gdk_property_delete( context->source_window, XDS_ATOM );
	}
}

/*
 * called when a drop occurs in the treeview for a move/copy inside of
 * the tree
 *
 * Returns: %TRUE if the specified rows were successfully inserted at
 * the given dest, %FALSE else.
 *
 * The dest path is computed based on the current appearance of the list
 * Drop should so occurs inside an inchanged list to keep a valid path
 * in the case of a move, this leads to :
 *  1) marks dragged items as 'to be deleted'
 *  2) insert new dropped items
 *  3) remove 'to be deleted' items
 * -> not an easy idea as we want modify the id of all the dragged
 *    hierarchy
 *
 * adjusting the path: quid if the target dest is not at the same level
 */
static gboolean
drop_inside( NactTreeModel *model, GtkTreePath *dest, GtkSelectionData  *selection_data )
{
	static const gchar *thisfn = "nact_tree_model_dnd_inside_drag_and_drop";
	NactApplication *application;
	NAUpdater *updater;
	NactMainWindow *main_window;
	NAObjectItem *parent;
	gboolean copy_data;
	GList *rows;
	GtkTreePath *new_dest;
	GtkTreePath *path;
	NAObject *current;
	NAObject *inserted;
	GList *object_list, *it;
	GtkTreeIter iter;
	GList *deletable;
	gboolean relabel;

	application = NACT_APPLICATION( base_window_get_application( model->private->window ));
	updater = nact_application_get_updater( application );
	main_window = NACT_MAIN_WINDOW( base_application_get_main_window( BASE_APPLICATION( application )));

	/*
	 * NACT format (may embed profiles, or not)
	 * 	with profiles: only valid dest is inside an action
	 *  without profile: only valid dest is outside (besides of) an action
	 */
	parent = NULL;
	rows = nact_clipboard_dnd_get_data( model->private->clipboard, &copy_data );

	if( !is_drop_possible( model, dest, &parent )){
		return( FALSE );
	}

	new_dest = gtk_tree_path_copy( dest );
	if( !copy_data ){
		drop_inside_move_dest( model, rows, &new_dest );
	}

	g_debug( "%s: rows has %d items, copy_data=%s", thisfn, g_list_length( rows ), copy_data ? "True":"False" );
	object_list = NULL;
	for( it = rows ; it ; it = it->next ){
		path = gtk_tree_row_reference_get_path(( GtkTreeRowReference * ) it->data );
		if( path ){
			if( gtk_tree_model_get_iter( GTK_TREE_MODEL( model ), &iter, path )){
				gtk_tree_model_get( GTK_TREE_MODEL( model ), &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &current, -1 );
				g_object_unref( current );

				if( copy_data ){
					inserted = ( NAObject * ) na_object_duplicate( current );
					na_object_set_origin( inserted, NULL );
					na_object_check_status( inserted );

				} else {
					inserted = na_object_ref( current );
					deletable = g_list_prepend( NULL, inserted );
					nact_iactions_list_bis_delete( NACT_IACTIONS_LIST( main_window ), deletable, FALSE );
					g_list_free( deletable );
				}

				relabel = nact_main_menubar_edit_is_pasted_object_relabeled( inserted, NA_PIVOT( updater ));
				na_object_prepare_for_paste( inserted, relabel, copy_data, parent );
				object_list = g_list_prepend( object_list, inserted );
				g_debug( "%s: dropped=%s", thisfn, na_object_get_label( inserted ));
			}
			gtk_tree_path_free( path );
		}
	}
	object_list = g_list_reverse( object_list );

	nact_iactions_list_bis_insert_at_path( NACT_IACTIONS_LIST( main_window ), object_list, new_dest );

	if( gtk_tree_path_get_depth( new_dest ) == 1 ){
		g_signal_emit_by_name( main_window, MAIN_WINDOW_SIGNAL_LEVEL_ZERO_ORDER_CHANGED, GINT_TO_POINTER( TRUE ));
	}

	g_list_foreach( object_list, ( GFunc ) na_object_object_unref, NULL );
	g_list_free( object_list );
	gtk_tree_path_free( new_dest );

	g_list_foreach( rows, ( GFunc ) gtk_tree_row_reference_free, NULL );
	g_list_free( rows );

	return( TRUE );
}

/*
 * is a drop possible at given dest ?
 *
 * the only case where we would be led to have to modify the dest if
 * we'd want be able to drop a profile into another profile, accepting
 * it, actually dropping the profile just before the target
 *
 * -> it appears both clearer for the user interface and easyer from a
 *    code point of view to just refuse to drop a profile into a profile
 *
 * so this function is just to check if a drop is possible at the given
 * dest
 */
static gboolean
is_drop_possible( NactTreeModel *model, GtkTreePath *dest, NAObjectItem **parent )
{
	gboolean drop_ok;
	NactApplication *application;
	NactMainWindow *main_window;
	GtkTreeIter iter;
	NAObjectItem *parent_dest;

	drop_ok = FALSE;
	parent_dest = NULL;
	application = NACT_APPLICATION( base_window_get_application( model->private->window ));
	main_window = NACT_MAIN_WINDOW( base_application_get_main_window( BASE_APPLICATION( application )));

	/* if we can have an iter on given dest, then the dest already exists
	 * so dropped items should be of the same type that already existing
	 */
	if( gtk_tree_model_get_iter( GTK_TREE_MODEL( model ), &iter, dest )){
		drop_ok = is_drop_possible_before_iter( model, &iter, main_window, &parent_dest );

	/* inserting at the end of the list
	 * parent_dest is NULL
	 */
	} else if( gtk_tree_path_get_depth( dest ) == 1 ){

		if( model->private->drag_has_profiles ){
			nact_main_statusbar_display_with_timeout(
						main_window, TREE_MODEL_STATUSBAR_CONTEXT, st_refuse_drop_profile );

		} else {
			drop_ok = TRUE;
		}

	/* we cannot have an iter on the dest: this means that we try to
	 * insert items into not-opened dest (an empty menu or an action with
	 * zero or one profile) : check what is the parent
	 */
	} else {
		drop_ok = is_drop_possible_into_dest( model, dest, main_window, &parent_dest );
	}

	if( drop_ok ){
		drop_ok = is_parent_accept_new_childs( application, main_window, parent_dest );
	}

	if( drop_ok && parent ){
		*parent = parent_dest;
	}

	return( drop_ok );
}

static gboolean
is_drop_possible_before_iter( NactTreeModel *model, GtkTreeIter *iter, NactMainWindow *window, NAObjectItem **parent )
{
	static const gchar *thisfn = "nact_tree_model_dnd_is_drop_possible_before_iter";
	gboolean drop_ok;
	NAObject *object;

	drop_ok = FALSE;
	*parent = NULL;

	gtk_tree_model_get( GTK_TREE_MODEL( model ), iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );
	g_object_unref( object );
	g_debug( "%s: current object at dest is %s", thisfn, G_OBJECT_TYPE_NAME( object ));

	if( model->private->drag_has_profiles ){

		if( NA_IS_OBJECT_PROFILE( object )){
			drop_ok = TRUE;
			*parent = na_object_get_parent( object );

		} else {
			/* unable to drop a profile here */
			nact_main_statusbar_display_with_timeout(
					window, TREE_MODEL_STATUSBAR_CONTEXT, st_refuse_drop_profile );
		}

	} else if( NA_IS_OBJECT_ITEM( object )){
			drop_ok = TRUE;
			*parent = na_object_get_parent( object );

	} else {
		/* unable to drop an action or a menu here */
		nact_main_statusbar_display_with_timeout(
				window, TREE_MODEL_STATUSBAR_CONTEXT, st_refuse_drop_item );
	}

	return( drop_ok );
}

static gboolean
is_drop_possible_into_dest( NactTreeModel *model, GtkTreePath *dest, NactMainWindow *window, NAObjectItem **parent )
{
	static const gchar *thisfn = "nact_tree_model_dnd_is_drop_possible_into_dest";
	gboolean drop_ok;
	GtkTreePath *path;
	GtkTreeIter iter;
	NAObject *object;

	drop_ok = FALSE;
	*parent = NULL;

	path = gtk_tree_path_copy( dest );

	if( gtk_tree_path_up( path )){
		if( gtk_tree_model_get_iter( GTK_TREE_MODEL( model ), &iter, path )){
			gtk_tree_model_get( GTK_TREE_MODEL( model ), &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );
			g_object_unref( object );
			g_debug( "%s: current object at parent dest is %s", thisfn, G_OBJECT_TYPE_NAME( object ));

			if( model->private->drag_has_profiles ){

				if( NA_IS_OBJECT_ACTION( object )){
					drop_ok = TRUE;
					*parent = NA_OBJECT_ITEM( object );

				} else {
					nact_main_statusbar_display_with_timeout(
							window, TREE_MODEL_STATUSBAR_CONTEXT, st_refuse_drop_profile );
				}

			} else if( NA_IS_OBJECT_MENU( object )){
					drop_ok = TRUE;
					*parent = na_object_get_parent( object );

			} else {
				nact_main_statusbar_display_with_timeout(
						window, TREE_MODEL_STATUSBAR_CONTEXT, st_refuse_drop_item );
			}
		}
	}

	gtk_tree_path_free( path );

	return( drop_ok );
}

static void
drop_inside_move_dest( NactTreeModel *model, GList *rows, GtkTreePath **dest )
{
	GList *it;
	GtkTreePath *path;
	gint before;
	gint i;
	gint *indices;

	g_return_if_fail( dest );

	before = 0;
	for( it = rows ; it ; it = it->next ){
		path = gtk_tree_row_reference_get_path(( GtkTreeRowReference * ) it->data );
		if( path ){
			if( gtk_tree_path_get_depth( path ) == 1 &&
				gtk_tree_path_compare( path, *dest ) == -1 ){

				before += 1;
			}
			gtk_tree_path_free( path );
		}
	}

	g_debug( "drop_inside_move_dest: before=%d", before );
	g_debug( "drop_inside_move_dest: dest=%s", gtk_tree_path_to_string( *dest ));

	if( before ){
		indices = gtk_tree_path_get_indices( *dest );
		indices[0] -= before;
		path = gtk_tree_path_new_from_indices( indices[0], -1 );
		for( i = 1 ; i < gtk_tree_path_get_depth( *dest ); ++i ){
			gtk_tree_path_append_index( path, indices[i] );
		}
		gtk_tree_path_free( *dest );
		*dest = gtk_tree_path_copy( path );
		gtk_tree_path_free( path );
	}

	g_debug( "drop_inside_move_dest: dest=%s", gtk_tree_path_to_string( *dest ));
}

/*
 * called when a drop from the outside occurs in the treeview
 *
 * Returns: %TRUE if the specified rows were successfully inserted at
 * the given dest, %FALSE else.
 *
 * URI format only involves actions or menus
 *  so ony valid dest in outside (besides of) an action
 */
static gboolean
drop_uri_list( NactTreeModel *model, GtkTreePath *dest, GtkSelectionData  *selection_data )
{
	static const gchar *thisfn = "nact_tree_model_dnd_drop_uri_list";
	gboolean drop_done;
	NactApplication *application;
	NAUpdater *updater;
	NactMainWindow *main_window;
	NAIImporterListParms parms;
	MateConfClient *mateconf;
	GList *it;

	gchar *dest_str = gtk_tree_path_to_string( dest );
	g_debug( "%s: model=%p, dest=%p (%s), selection_data=%p",
			thisfn, ( void * ) model, ( void * ) dest, dest_str, ( void * ) selection_data );
	g_free( dest_str );

	drop_done = FALSE;
	model->private->drag_has_profiles = FALSE;

	if( !is_drop_possible( model, dest, NULL )){
		return( FALSE );
	}

	application = NACT_APPLICATION( base_window_get_application( model->private->window ));
	updater = nact_application_get_updater( application );
	main_window = NACT_MAIN_WINDOW( base_application_get_main_window( BASE_APPLICATION( application )));

	parms.version = 1;
	g_debug( "%s", ( const gchar * ) selection_data->data );
	parms.uris = g_slist_reverse( na_core_utils_slist_from_split(( const gchar * ) selection_data->data, "\r\n" ));

	mateconf = mateconf_client_get_default();
	parms.mode = na_iprefs_get_import_mode( mateconf, IPREFS_IMPORT_ITEMS_IMPORT_MODE );
	g_object_unref( mateconf );

	parms.window = base_window_get_toplevel( BASE_WINDOW( main_window ));
	parms.imported = NULL;
	parms.check_fn = ( NAIImporterCheckFn ) is_dropped_already_exists;
	parms.check_fn_data = main_window;
	parms.messages = NULL;

	na_importer_import_from_list( NA_PIVOT( updater ), &parms );

	/* display first message in status bar
	 */
	if( parms.messages ){
		nact_main_statusbar_display_with_timeout(
				main_window,
				TREE_MODEL_STATUSBAR_CONTEXT,
				parms.messages->data );
	}

	/* if there is more than one message, display them in a dialog box
	 */
	if( parms.messages && g_slist_length( parms.messages ) >= 2 ){
		GtkMessageDialog *dialog = GTK_MESSAGE_DIALOG( gtk_message_dialog_new(
				parms.window,
				GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
				"%s", _( "Some messages have occurred during drop operation." )));
		GString *str = g_string_new( "" );
		GSList *im;
		for( im = parms.messages ; im ; im = im->next ){
			g_string_append_printf( str, "%s\n", ( const gchar * ) im->data );
		}
		gtk_message_dialog_format_secondary_markup( dialog, "%s", str->str );
		g_string_free( str, TRUE );
	}

	/* check status of newly imported items, and insert them in the list view
	 */
	for( it = parms.imported ; it ; it = it->next ){
		na_object_check_status( it->data );
		na_object_dump( it->data );
		drop_done = TRUE;
	}

	nact_iactions_list_bis_insert_at_path( NACT_IACTIONS_LIST( main_window ), parms.imported, dest );
	nact_tree_model_dump( model );

	na_object_unref_items( parms.imported );
	na_core_utils_slist_free( parms.uris );
	na_core_utils_slist_free( parms.messages );

	return( drop_done );
}

static NAObjectItem *
is_dropped_already_exists( const NAObjectItem *importing, const NactMainWindow *window )
{
	gchar *id = na_object_get_id( importing );
	NAObjectItem *exists = nact_main_window_get_item( window, id );
	g_free( id );

	return( exists );
}

/*
 * this function works well, but only called from on_drag_motion handler...
 */
/*static gboolean
is_row_drop_possible( NactTreeModel *model, GtkTreePath *path, GtkTreeViewDropPosition pos )
{
	gboolean ok = FALSE;
	GtkTreeModel *store;
	GtkTreeIter iter;
	NAObject *object;

	store = gtk_tree_model_filter_get_model( GTK_TREE_MODEL_FILTER( model ));
	gtk_tree_model_get_iter( store, &iter, path );
	gtk_tree_model_get( store, &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );

	if( model->private->drag_has_profiles ){
		if( NA_IS_OBJECT_PROFILE( object )){
			ok = TRUE;
		} else if( NA_IS_OBJECT_ACTION( object )){
			ok = ( pos == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE || pos == GTK_TREE_VIEW_DROP_INTO_OR_AFTER );
		}
	} else {
		if( NA_IS_OBJECT_ITEM( object )){
			ok = TRUE;
		}
	}

	g_object_unref( object );
	return( ok );
}*/

/*
 * called when the user moves into the target widget
 * returns TRUE if a drop zone
 */
#if 0
static gboolean
on_drag_motion( GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, BaseWindow *window )
{
	NactTreeModel *model;
	gboolean ok = FALSE;
	GtkTreePath *path;
	GtkTreeViewDropPosition pos;

	model = NACT_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( widget )));
	g_return_val_if_fail( NACT_IS_TREE_MODEL( model ), FALSE );

	if( !model->private->dispose_has_run ){

		if( !model->private->drag_highlight ){
			model->private->drag_highlight = TRUE;
			gtk_drag_highlight( widget );
		}

		/*target = gtk_drag_dest_find_target( widget, context, NULL );
		if( target == GDK_NONE ){
			gdk_drag_status( context, 0, time );
		} else {
			gtk_drag_get_data( widget, context, target, time );
		}*/

		if( gtk_tree_view_get_dest_row_at_pos( GTK_TREE_VIEW( widget ), x, y, &path, &pos )){
			ok = is_row_drop_possible( model, path, pos );
			if( ok ){
				gdk_drag_status( context, 0, time );
			}
		}

		gtk_tree_path_free( path );
		g_debug( "nact_tree_model_on_drag_motion: ok=%s, pos=%d", ok ? "True":"False", pos );
	}

	return( ok );
}
#endif

/*
 * called when the user drops the data
 * returns TRUE if a drop zone
 */
/*static gboolean
on_drag_drop( GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, BaseWindow *window )
{
	NactTreeModel *model;

	model = NACT_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( widget )));
	g_return_val_if_fail( NACT_IS_TREE_MODEL( model ), FALSE );

	if( !model->private->dispose_has_run ){

		model->private->drag_drop = TRUE;
	}

	return( TRUE );
}*/

/* The following function taken from bugzilla
 * (http://bugzilla.gnome.org/attachment.cgi?id=49362&action=view)
 * Author: Christian Neumair
 * Copyright: 2005 Free Software Foundation, Inc
 * License: GPL
 *
 * On a 32-bits system:
 * get_xds_atom_value: actual_length=63, actual_length=15
 * get_xds_atom_value: ret=file:///home/pierre/data/eclipse/caja-actions/trash/xds.txt0x8299
 * get_xds_atom_value: dup=file:///home/pierre/data/eclipse/caja-actions/trash/xds.txt
 * get_xds_atom_value: ret=file:///home/pi
 *
 * idem on a 64bits system.
 */
static char *
get_xds_atom_value( GdkDragContext *context )
{
	gchar *ret;
	gint actual_length;

	g_return_val_if_fail( context != NULL, NULL );
	g_return_val_if_fail( context->source_window != NULL, NULL );

	gdk_property_get( context->source_window,		/* a GdkWindow */
						XDS_ATOM, 					/* the property to retrieve */
						TEXT_ATOM,					/* the desired property type */
						0, 							/* offset (in 4 bytes chunks) */
						MAX_XDS_ATOM_VAL_LEN,		/* max length (in 4 bytes chunks) */
						FALSE, 						/* whether to delete the property */
						NULL, 						/* actual property type */
						NULL, 						/* actual format */
						&actual_length,				/* actual length (in 4 bytes chunks) */
						( guchar ** ) &ret );		/* data pointer */

	ret[actual_length] = '\0';

	return( ret );
}

/*
 * when dropping something somewhere, we must ensure that we will be able
 * to register the new child
 */
static gboolean
is_parent_accept_new_childs( NactApplication *application, NactMainWindow *window, NAObjectItem *parent )
{
	gboolean accept_ok;
	NAUpdater *updater;

	accept_ok = FALSE;
	updater = nact_application_get_updater( application );

	/* inserting as a level zero item
	 * ensure that level zero is writable
	 */
	if( parent == NULL ){

		if( na_pivot_is_level_zero_writable( NA_PIVOT( updater ))){
			accept_ok = TRUE;

		} else {
			nact_main_statusbar_display_with_timeout(
						window, TREE_MODEL_STATUSBAR_CONTEXT, st_level_zero_not_writable );
		}

	/* see if the parent is writable
	 */
	} else if( na_updater_is_item_writable( updater, parent, NULL )){
		accept_ok = TRUE;

	} else {
			nact_main_statusbar_display_with_timeout(
						window, TREE_MODEL_STATUSBAR_CONTEXT, st_parent_not_writable );
	}

	return( accept_ok );
}

static guint
target_atom_to_id( GdkAtom atom )
{
	gint i;
	guint info = 0;
	gchar *atom_name;

	atom_name = gdk_atom_name( atom );
	for( i = 0 ; i < tree_model_dnd_dest_formats_count ; ++i ){
		if( !g_ascii_strcasecmp( tree_model_dnd_dest_formats[i].target, atom_name )){
			info = tree_model_dnd_dest_formats[i].info;
			break;
		}
	}
	g_free( atom_name );
	return( info );
}
