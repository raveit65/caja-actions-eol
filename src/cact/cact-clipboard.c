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

#include <gtk/gtk.h>
#include <string.h>

#include <api/na-object-api.h>

#include <core/na-exporter.h>
#include <core/na-export-format.h>

#include "cact-application.h"
#include "cact-iprefs.h"
#include "cact-export-ask.h"
#include "cact-tree-model.h"
#include "cact-clipboard.h"

/* private class data
 */
struct CactClipboardClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
typedef struct {
	guint    target;
	gchar   *folder;
	GList   *rows;
	gboolean copy;
}
	CactClipboardDndData;

typedef struct {
	GList *items;
	gint   mode;
	guint  nb_actions;
	guint  nb_profiles;
	guint  nb_menus;
}
	PrimaryData;

struct CactClipboardPrivate {
	gboolean      dispose_has_run;
	BaseWindow   *window;
	GtkClipboard *dnd;
	GtkClipboard *primary;
	PrimaryData  *primary_data;
	gboolean      primary_got;
};

#define CACT_CLIPBOARD_ATOM				gdk_atom_intern( "_CACT_CLIPBOARD", FALSE )
#define CACT_CLIPBOARD_CACT_ATOM		gdk_atom_intern( "ClipboardCajaActions", FALSE )

enum {
	CACT_CLIPBOARD_FORMAT_CACT = 0,
	CACT_CLIPBOARD_FORMAT_APPLICATION_XML,
	CACT_CLIPBOARD_FORMAT_TEXT_PLAIN
};

/* clipboard formats
 * - a special ClipboardCajaAction format for internal move/copy
 *   and also used by drag and drop operations
 * - a XdndDirectSave, suitable for exporting to a file manager
 *   (note that Caja recognized the "XdndDirectSave0" format as XDS
 *   protocol)
 * - a text (xml) format, to go to clipboard or a text editor
 */
static GtkTargetEntry clipboard_formats[] = {
	{ "ClipboardCajaActions", 0, CACT_CLIPBOARD_FORMAT_CACT },
	{ "application/xml",          0, CACT_CLIPBOARD_FORMAT_APPLICATION_XML },
	{ "text/plain",               0, CACT_CLIPBOARD_FORMAT_TEXT_PLAIN },
};

static GObjectClass *st_parent_class = NULL;

static GType  register_type( void );
static void   class_init( CactClipboardClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *application );
static void   instance_finalize( GObject *application );

static void   get_from_dnd_clipboard_callback( GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, guchar *data );
static void   clear_dnd_clipboard_callback( GtkClipboard *clipboard, CactClipboardDndData *data );
static gchar *export_rows( CactClipboard *clipboard, GList *rows, const gchar *dest_folder );
static gchar *export_objects( CactClipboard *clipboard, GList *objects, const gchar *dest_folder );
static gchar *export_row_object( CactClipboard *clipboard, NAObject *object, const gchar *dest_folder, GList **exported );

static void   get_from_primary_clipboard_callback( GtkClipboard *gtk_clipboard, GtkSelectionData *selection_data, guint info, CactClipboard *clipboard );
static void   clear_primary_clipboard( CactClipboard *clipboard );
static void   clear_primary_clipboard_callback( GtkClipboard *gtk_clipboard, CactClipboard *clipboard );
static void   dump_primary_clipboard( CactClipboard *clipboard );

static gchar *clipboard_mode_to_string( gint mode );

GType
cact_clipboard_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "cact_clipboard_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactClipboardClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactClipboard ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "CactClipboard", &info, 0 );

	return( type );
}

static void
class_init( CactClipboardClass *klass )
{
	static const gchar *thisfn = "cact_clipboard_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactClipboardClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_clipboard_instance_init";
	CactClipboard *self;
	GdkDisplay *display;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_assert( CACT_IS_CLIPBOARD( instance ));
	self = CACT_CLIPBOARD( instance );

	self->private = g_new0( CactClipboardPrivate, 1 );

	self->private->dispose_has_run = FALSE;

	display = gdk_display_get_default();
	self->private->dnd = gtk_clipboard_get_for_display( display, CACT_CLIPBOARD_ATOM );
	self->private->primary = gtk_clipboard_get_for_display( display, GDK_SELECTION_CLIPBOARD );
	self->private->primary_data = NULL;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "cact_clipboard_instance_dispose";
	CactClipboard *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_assert( CACT_IS_CLIPBOARD( object ));
	self = CACT_CLIPBOARD( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		gtk_clipboard_clear( self->private->dnd );
		gtk_clipboard_clear( self->private->primary );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *window )
{
	static const gchar *thisfn = "cact_clipboard_instance_finalize";
	CactClipboard *self;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );
	g_assert( CACT_IS_CLIPBOARD( window ));
	self = CACT_CLIPBOARD( window );

	if( self->private->primary_data ){
		clear_primary_clipboard( self );
		g_free( self->private->primary_data );
	}

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( window );
	}
}

/**
 * cact_clipboard_new:
 *
 * Returns: a new #CactClipboard object.
 */
CactClipboard *
cact_clipboard_new( BaseWindow *window )
{
	CactClipboard *clipboard;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), NULL );

	clipboard = g_object_new( CACT_CLIPBOARD_TYPE, NULL );

	clipboard->private->window = window;

	return( clipboard );
}

/**
 * cact_clipboard_dnd_set:
 * @clipboard: this #CactClipboard instance.
 * @rows: the list of row references of dragged items.
 * @folder: the target folder if any (XDS protocol to outside).
 * @copy_data: %TRUE if data is to be copied, %FALSE else
 *  (only relevant when drag and drop occurs inside of the tree view).
 *
 * Set the selected items into our dnd clipboard.
 */
void
cact_clipboard_dnd_set( CactClipboard *clipboard, guint target, GList *rows, const gchar *folder, gboolean copy_data )
{
	static const gchar *thisfn = "cact_clipboard_dnd_set";
	CactClipboardDndData *data;
	GtkTreeModel *model;
	GList *it;

	g_return_if_fail( CACT_IS_CLIPBOARD( clipboard ));
	g_return_if_fail( rows && g_list_length( rows ));

	if( !clipboard->private->dispose_has_run ){

		data = g_new0( CactClipboardDndData, 1 );

		data->target = target;
		data->folder = g_strdup( folder );
		data->rows = NULL;
		data->copy = copy_data;

		model = gtk_tree_row_reference_get_model(( GtkTreeRowReference * ) rows->data );

		for( it = rows ; it ; it = it->next ){
			data->rows = g_list_append(
					data->rows,
					gtk_tree_row_reference_copy(( GtkTreeRowReference * ) it->data ));
		}

		gtk_clipboard_set_with_data( clipboard->private->dnd,
				clipboard_formats, G_N_ELEMENTS( clipboard_formats ),
				( GtkClipboardGetFunc ) get_from_dnd_clipboard_callback,
				( GtkClipboardClearFunc ) clear_dnd_clipboard_callback,
				data );

		g_debug( "%s: clipboard=%p, data=%p", thisfn, ( void * ) clipboard, ( void * ) data );
	}
}

/**
 * cact_clipboard_dnd_get_data:
 * @clipboard: this #CactClipboard instance.
 * @copy_data: will be set to the original value of the drag and drop.
 *
 * Returns the list of rows references privously stored.
 *
 * The returned list should be gtk_tree_row_reference_free() by the
 * caller.
 */
GList *
cact_clipboard_dnd_get_data( CactClipboard *clipboard, gboolean *copy_data )
{
	static const gchar *thisfn = "cact_clipboard_dnd_get_data";
	GList *rows = NULL;
	GtkSelectionData *selection;
	CactClipboardDndData *data;
	GtkTreeModel *model;
	GList *it;

	g_debug( "%s: clipboard=%p", thisfn, ( void * ) clipboard );
	g_return_val_if_fail( CACT_IS_CLIPBOARD( clipboard ), NULL );

	if( copy_data ){
		*copy_data = FALSE;
	}

	if( !clipboard->private->dispose_has_run ){

		selection = gtk_clipboard_wait_for_contents( clipboard->private->dnd, CACT_CLIPBOARD_CACT_ATOM );
		if( selection ){

			data = ( CactClipboardDndData * ) selection->data;
			if( data->target == CACT_XCHANGE_FORMAT_CACT ){

				model = gtk_tree_row_reference_get_model(( GtkTreeRowReference * ) data->rows->data );

				for( it = data->rows ; it ; it = it->next ){
					rows = g_list_append( rows,
							gtk_tree_row_reference_copy(( GtkTreeRowReference * ) it->data ));
				}
				*copy_data = data->copy;
			}
		}
		gtk_selection_data_free( selection );
	}

	return( rows );
}

/*
 * Get text/plain from selected actions.
 *
 * This is called when we drop or paste a selection onto an application
 * willing to deal with Xdnd protocol, for text/plain or application/xml
 * mime types.
 *
 * Selected items may include menus, actions and profiles.
 * For now, we only exports actions (and profiles) as XML files.
 *
 * FIXME: na_xml_writer_get_xml_buffer() returns a valid XML document,
 * which includes the <?xml ...?> header. Concatenating several valid
 * XML documents doesn't provide a valid global XML doc, because of
 * the presence of several <?xml ?> headers inside.
 */
gchar *
cact_clipboard_dnd_get_text( CactClipboard *clipboard, GList *rows )
{
	static const gchar *thisfn = "cact_clipboard_dnd_get_text";
	gchar *buffer;

	g_return_val_if_fail( CACT_IS_CLIPBOARD( clipboard ), NULL );

	g_debug( "%s: clipboard=%p, rows=%p (count=%u)",
			thisfn, ( void * ) clipboard, ( void * ) rows, g_list_length( rows ));

	buffer = NULL;

	if( !clipboard->private->dispose_has_run ){

		buffer = export_rows( clipboard, rows, NULL );
		g_debug( "%s: returning buffer=%p (length=%lu)", thisfn, ( void * ) buffer, g_utf8_strlen( buffer, -1 ));
	}

	return( buffer );
}

/**
 * cact_clipboard_dnd_drag_end:
 * @clipboard: this #CactClipboard instance.
 *
 * On drag-end, exports the objects if needed.
 */
void
cact_clipboard_dnd_drag_end( CactClipboard *clipboard )
{
	static const gchar *thisfn = "cact_clipboard_dnd_drag_end";
	GtkSelectionData *selection;
	CactClipboardDndData *data;

	g_debug( "%s: clipboard=%p", thisfn, ( void * ) clipboard );
	g_return_if_fail( CACT_IS_CLIPBOARD( clipboard ));

	if( !clipboard->private->dispose_has_run ){

		selection = gtk_clipboard_wait_for_contents( clipboard->private->dnd, CACT_CLIPBOARD_CACT_ATOM );
		g_debug( "%s: selection=%p", thisfn, ( void * ) selection );

		if( selection ){
			data = ( CactClipboardDndData * ) selection->data;
			g_debug( "%s: data=%p (CactClipboardDndData)", thisfn, ( void * ) data );

			if( data->target == CACT_XCHANGE_FORMAT_XDS ){
				export_rows( clipboard, data->rows, data->folder );
			}

			gtk_selection_data_free( selection );
		}
	}
}

/**
 * cact_clipboard_dnd_clear:
 * @clipboard: this #CactClipboard instance.
 *
 * Clears the drag-and-drop clipboard.
 *
 * At least called on drag-begin.
 */
void
cact_clipboard_dnd_clear( CactClipboard *clipboard )
{
	g_debug( "cact_clipboard_dnd_clear: clipboard=%p", ( void * ) clipboard );

	gtk_clipboard_clear( clipboard->private->dnd );
}

static void
get_from_dnd_clipboard_callback( GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, guchar *data )
{
	static const gchar *thisfn = "cact_clipboard_get_from_dnd_clipboard_callback";

	g_debug( "%s: clipboard=%p, selection_data=%p, target=%s, info=%d, data=%p",
			thisfn, ( void * ) clipboard,
			( void * ) selection_data, gdk_atom_name( selection_data->target ), info, ( void * ) data );

	gtk_selection_data_set( selection_data, selection_data->target, 8, data, sizeof( CactClipboardDndData ));
}

static void
clear_dnd_clipboard_callback( GtkClipboard *clipboard, CactClipboardDndData *data )
{
	static const gchar *thisfn = "cact_clipboard_clear_dnd_clipboard_callback";

	g_debug( "%s: clipboard=%p, data=%p", thisfn, ( void * ) clipboard, ( void * ) data );

	g_free( data->folder );
	g_list_foreach( data->rows, ( GFunc ) gtk_tree_row_reference_free, NULL );
	g_list_free( data->rows );
	g_free( data );
}

static gchar *
export_rows( CactClipboard *clipboard, GList *rows, const gchar *dest_folder )
{
	GString *data;
	GtkTreeModel *model;
	GList *exported, *irow;
	GtkTreePath *path;
	GtkTreeIter iter;
	NAObject *object;
	gchar *buffer;

	buffer = NULL;
	exported = NULL;
	data = g_string_new( "" );
	model = gtk_tree_row_reference_get_model(( GtkTreeRowReference * ) rows->data );

	for( irow = rows ; irow ; irow = irow->next ){
		path = gtk_tree_row_reference_get_path(( GtkTreeRowReference * ) irow->data );
		if( path ){
			gtk_tree_model_get_iter( model, &iter, path );
			gtk_tree_path_free( path );
			gtk_tree_model_get( model, &iter, IACTIONS_LIST_NAOBJECT_COLUMN, &object, -1 );
			buffer = export_row_object( clipboard, object, dest_folder, &exported );
			if( buffer && strlen( buffer )){
				data = g_string_append( data, buffer );
				g_free( buffer );
			}
			g_object_unref( object );
		}
	}

	g_list_free( exported );
	return( g_string_free( data, FALSE ));
}

static gchar *
export_objects( CactClipboard *clipboard, GList *objects, const gchar *dest_folder )
{
	gchar *buffer;
	GString *data;
	GList *exported;
	GList *iobj;
	NAObject *object;

	buffer = NULL;
	exported = NULL;
	data = g_string_new( "" );

	for( iobj = objects ; iobj ; iobj = iobj->next ){
		object = NA_OBJECT( iobj->data );
		buffer = export_row_object( clipboard, object, dest_folder, &exported );
		if( buffer && strlen( buffer )){
			data = g_string_append( data, buffer );
			g_free( buffer );
		}
		g_object_unref( object );
	}

	g_list_free( exported );
	return( g_string_free( data, FALSE ));
}

/*
 * export to a buffer if dest_folder is null
 * else export to a new file in the target directory
 */
static gchar *
export_row_object( CactClipboard *clipboard, NAObject *object, const gchar *dest_folder, GList **exported )
{
	GList *subitems, *isub;
	CactApplication *application;
	NAUpdater *updater;
	NAObjectAction *action;
	gint index;
	GString *data;
	gchar *buffer;
	GQuark format;
	gchar *fname;
	GSList *msgs;

	data = g_string_new( "" );

	if( NA_IS_OBJECT_MENU( object )){
		subitems = na_object_get_items( object );

		for( isub = subitems ; isub ; isub = isub->next ){
			buffer = export_row_object( clipboard, isub->data, dest_folder, exported );
			if( buffer && strlen( buffer )){
				data = g_string_append( data, buffer );
				g_free( buffer );
			}
		}
	}

	msgs = NULL;
	action = ( NAObjectAction * ) object;
	if( NA_IS_OBJECT_PROFILE( object )){
		action = NA_OBJECT_ACTION( na_object_get_parent( object ));
	}

	application = CACT_APPLICATION( base_window_get_application( clipboard->private->window ));
	updater = cact_application_get_updater( application );

	index = g_list_index( *exported, ( gconstpointer ) action );
	if( index == -1 ){

		*exported = g_list_prepend( *exported, ( gpointer ) action );
		format = cact_iprefs_get_export_format( clipboard->private->window, IPREFS_EXPORT_FORMAT );

		if( format == IPREFS_EXPORT_FORMAT_ASK ){
			format = cact_export_ask_user( clipboard->private->window, NA_OBJECT_ITEM( action ));
		}

		if( format != IPREFS_EXPORT_NO_EXPORT ){

			if( dest_folder ){
				fname = na_exporter_to_file( NA_PIVOT( updater ), NA_OBJECT_ITEM( action), dest_folder, format, &msgs );
				g_free( fname );

			} else {
				buffer = na_exporter_to_buffer( NA_PIVOT( updater ), NA_OBJECT_ITEM( action ), format, &msgs );
				if( buffer && strlen( buffer )){
					data = g_string_append( data, buffer );
					g_free( buffer );
				}
			}
		}
	}

	return( g_string_free( data, FALSE ));
}

/**
 * cact_clipboard_primary_set:
 * @clipboard: this #CactClipboard object.
 * @items: a list of #NAObject items
 * @mode: where do these items come from ?
 *  Or what is the operation which has led the items to the clipboard?
 *
 * Installs a copy of provided items in the clipboard.
 *
 * Rationale: when cutting an item to the clipboard, the next paste
 * will keep its same original id, and it is safe because this is
 * actually what we want when we cut/paste.
 *
 * Contrarily, when we copy/paste, we are waiting for a new element
 * which has the same characteristics that the previous one ; we so
 * have to renumber actions/menus items when copying into the clipboard.
 *
 * Note that we use NAIDuplicable interface without actually taking care
 * of what is origin or so, as origin will be reinitialized when getting
 * data out of the clipboard.
 */
void
cact_clipboard_primary_set( CactClipboard *clipboard, GList *items, gint mode )
{
	static const gchar *thisfn = "cact_clipboard_primary_set";
	PrimaryData *user_data;
	GList *it;

	g_debug( "%s: clipboard=%p, items=%p (count=%d), mode=%d",
			thisfn, ( void * ) clipboard, ( void * ) items, g_list_length( items ), mode );
	g_return_if_fail( CACT_IS_CLIPBOARD( clipboard ));

	if( !clipboard->private->dispose_has_run ){

		user_data = clipboard->private->primary_data;

		if( user_data == NULL ){
			user_data = g_new0( PrimaryData, 1 );
			clipboard->private->primary_data = user_data;
			g_debug( "%s: allocating PrimaryData=%p", thisfn, ( void * ) user_data );

		} else {
			clear_primary_clipboard( clipboard );
		}

		na_object_count_items( items,
				( gint * ) &user_data->nb_menus,
				( gint * ) &user_data->nb_actions,
				( gint * ) &user_data->nb_profiles,
				FALSE );

		for( it = items ; it ; it = it->next ){
			user_data->items =
					g_list_prepend( user_data->items, na_object_duplicate( it->data ));
		}
		user_data->items = g_list_reverse( user_data->items );

		user_data->mode = mode;

		gtk_clipboard_set_with_data( clipboard->private->primary,
				clipboard_formats, G_N_ELEMENTS( clipboard_formats ),
				( GtkClipboardGetFunc ) get_from_primary_clipboard_callback,
				( GtkClipboardClearFunc ) clear_primary_clipboard_callback,
				clipboard );

		clipboard->private->primary_got = FALSE;
	}
}

/**
 * cact_clipboard_primary_get:
 * @clipboard: this #CactClipboard object.
 *
 * Returns: a copy of the list of items previously referenced in the
 * internal clipboard.
 *
 * We allocate a new id for items in order to be ready to paste another
 * time.
 */
GList *
cact_clipboard_primary_get( CactClipboard *clipboard, gboolean *relabel )
{
	static const gchar *thisfn = "cact_clipboard_primary_get";
	GList *items = NULL;
	GtkSelectionData *selection;
	PrimaryData *user_data;
	GList *it;
	NAObject *obj;

	g_debug( "%s: clipboard=%p", thisfn, ( void * ) clipboard );
	g_return_val_if_fail( CACT_IS_CLIPBOARD( clipboard ), NULL );
	g_return_val_if_fail( relabel, NULL );

	if( !clipboard->private->dispose_has_run ){

		selection = gtk_clipboard_wait_for_contents( clipboard->private->primary, CACT_CLIPBOARD_CACT_ATOM );

		if( selection ){
			user_data = ( PrimaryData * ) selection->data;
			g_debug( "%s: retrieving PrimaryData=%p", thisfn, ( void * ) user_data );

			if( user_data ){
				for( it = user_data->items ; it ; it = it->next ){
					obj = NA_OBJECT( na_object_duplicate( it->data ));
					na_object_set_origin( obj, NULL );
					items = g_list_prepend( items, obj );
				}
				items = g_list_reverse( items );

				*relabel = (( user_data->mode == CLIPBOARD_MODE_CUT && clipboard->private->primary_got ) ||
								user_data->mode == CLIPBOARD_MODE_COPY );

				clipboard->private->primary_got = TRUE;
			}

			gtk_selection_data_free( selection );
		}
	}

	return( items );
}

/**
 * cact_clipboard_primary_counts:
 * @clipboard: this #CactClipboard object.
 *
 * Returns some counters on content of primary clipboard.
 */
void
cact_clipboard_primary_counts( CactClipboard *clipboard, guint *actions, guint *profiles, guint *menus )
{
	PrimaryData *user_data;

	g_return_if_fail( CACT_IS_CLIPBOARD( clipboard ));
	g_return_if_fail( actions && profiles && menus );

	if( !clipboard->private->dispose_has_run ){

		*actions = 0;
		*profiles = 0;
		*menus = 0;

		user_data = clipboard->private->primary_data;
		if( user_data ){

			*actions = user_data->nb_actions;
			*profiles = user_data->nb_profiles;
			*menus = user_data->nb_menus;
		}
	}
}

static void
get_from_primary_clipboard_callback( GtkClipboard *gtk_clipboard, GtkSelectionData *selection_data, guint info, CactClipboard *clipboard )
{
	static const gchar *thisfn = "cact_clipboard_get_from_primary_clipboard_callback";
	PrimaryData *user_data;
	gchar *buffer;

	g_debug( "%s: gtk_clipboard=%p, selection_data=%p, target=%s, info=%d, clipboard=%p",
			thisfn, ( void * ) gtk_clipboard,
			( void * ) selection_data, gdk_atom_name( selection_data->target ), info, ( void * ) clipboard );

	user_data = clipboard->private->primary_data;

	if( info == CACT_CLIPBOARD_FORMAT_TEXT_PLAIN ){
		buffer = export_objects( clipboard, user_data->items, NULL );
		gtk_selection_data_set( selection_data, selection_data->target, 8, ( const guchar * ) buffer, strlen( buffer ));
		g_free( buffer );

	} else {
		gtk_selection_data_set( selection_data, selection_data->target, 8, ( const guchar * ) user_data, sizeof( PrimaryData ));
	}
}

static void
clear_primary_clipboard( CactClipboard *clipboard )
{
	static const gchar *thisfn = "cact_clipboard_clear_primary_clipboard";
	PrimaryData *user_data;

	g_debug( "%s: clipboard=%p", thisfn, ( void * ) clipboard );

	user_data = clipboard->private->primary_data;
	g_return_if_fail( user_data != NULL );

	g_list_foreach( user_data->items, ( GFunc ) g_object_unref, NULL );
	g_list_free( user_data->items );
	user_data->items = NULL;
	user_data->nb_menus = 0;
	user_data->nb_actions = 0;
	user_data->nb_profiles = 0;

	clipboard->private->primary_got = FALSE;
}

static void
clear_primary_clipboard_callback( GtkClipboard *gtk_clipboard, CactClipboard *clipboard )
{
}

/**
 * cact_clipboard_dump:
 * @clipboard: this #CactClipboard instance.
 *
 * Dumps the content of the primary clipboard.
 */
void
cact_clipboard_dump( CactClipboard *clipboard )
{
	static const gchar *thisfn = "cact_clipboard_dump";

	g_return_if_fail( CACT_IS_CLIPBOARD( clipboard ));

	if( !clipboard->private->dispose_has_run ){

		g_debug( "%s:       window=%p (%s)", thisfn, ( void * ) clipboard->private->window, G_OBJECT_TYPE_NAME( clipboard->private->window ));
		g_debug( "%s:          dnd=%p", thisfn, ( void * ) clipboard->private->dnd );
		g_debug( "%s:      primary=%p", thisfn, ( void * ) clipboard->private->primary );
		g_debug( "%s: primary_data=%p", thisfn, ( void * ) clipboard->private->primary_data );

		if( clipboard->private->primary_data ){
			dump_primary_clipboard( clipboard );
		}
	}
}

static void
dump_primary_clipboard( CactClipboard *clipboard )
{
	static const gchar *thisfn = "cact_clipboard_dump_primary";
	PrimaryData *user_data;
	gchar *mode;
	GList *it;

	g_return_if_fail( CACT_IS_CLIPBOARD( clipboard ));

	if( !clipboard->private->dispose_has_run ){

		user_data = clipboard->private->primary_data;

		if( user_data ){
			g_debug( "%s:           user_data->nb_actions=%d", thisfn, user_data->nb_actions );
			g_debug( "%s:          user_data->nb_profiles=%d", thisfn, user_data->nb_profiles );
			g_debug( "%s:             user_data->nb_menus=%d", thisfn, user_data->nb_menus );
			g_debug( "%s:                user_data->items=%p (count=%d)",
					thisfn,
					( void * ) user_data->items,
					user_data->items ? g_list_length( user_data->items ) : 0 );
			mode = clipboard_mode_to_string( user_data->mode );
			g_debug( "%s:                 user_data->mode=%d (%s)", thisfn, user_data->mode, mode );
			g_free( mode );
			for( it = user_data->items ; it ; it = it->next ){
				na_object_object_dump( NA_OBJECT( it->data ));
			}
		}

		g_debug( "%s: clipboard->private->primary_got=%s", thisfn, clipboard->private->primary_got ? "True":"False" );
	}
}

static gchar *
clipboard_mode_to_string( gint mode )
{
	gchar *mode_str;

	switch( mode ){
		case CLIPBOARD_MODE_CUT:
			mode_str = g_strdup( "CutMode" );
			break;

		case CLIPBOARD_MODE_COPY:
			mode_str = g_strdup( "CopyMode" );
			break;

		default:
			mode_str = g_strdup( "unknown mode" );
			break;
	}

	return( mode_str );
}
