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

#include <api/na-core-utils.h>

#include "na-gtk-utils.h"
#include "na-ioptions-list.h"

/* private interface data
 */
struct _NAIOptionsListInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* column ordering in the tree view mode
 */
enum {
	IMAGE_COLUMN = 0,
	LABEL_COLUMN,
	TOOLTIP_COLUMN,
	OBJECT_COLUMN,
	N_COLUMN
};

/* data associated to the container of the instance
 *
 * Note that data may be set against the instance itself (has it been
 * initialized ?), at the container level or at the option level. It
 * is not really worth to have a dedicated structure (because we should
 * actually have three!).
 */
#define IOPTIONS_LIST_DATA_EDITABLE			"ioptions-list-data-editable"
#define IOPTIONS_LIST_DATA_FIRST_BUTTON		"ioptions-list-data-first-button"
#define IOPTIONS_LIST_DATA_INITIALIZED		"ioptions-list-data-initialized"
#define IOPTIONS_LIST_DATA_OPTION			"ioptions-list-data-option"
#define IOPTIONS_LIST_DATA_OPTION_ID		"ioptions-list-data-option-id"
#define IOPTIONS_LIST_DATA_SENSITIVE		"ioptions-list-data-sensitive"

static guint st_initializations = 0;	/* interface initialization count */

static GType        register_type( void );
static void         interface_base_init( NAIOptionsListInterface *iface );
static void         interface_base_finalize( NAIOptionsListInterface *iface );

static guint        ioptions_list_get_version( const NAIOptionsList *instance );
static void         ioptions_list_free_options( const NAIOptionsList *instance, GtkWidget *container_parent, GList *options );
static void         ioptions_list_free_ask_option( const NAIOptionsList *instance, GtkWidget *container_parent, NAIOption *option );
static GList       *options_list_get_options( const NAIOptionsList *instance, GtkWidget *container_parent );
static void         options_list_free_options( const NAIOptionsList *instance, GtkWidget *container_parent, GList *options );
static NAIOption   *options_list_get_ask_option( const NAIOptionsList *instance, GtkWidget *container_parent );
static void         options_list_free_ask_option( const NAIOptionsList *instance, GtkWidget *container_parent, NAIOption *ask_option );
static gboolean     get_options_list_container_initialized( GtkWidget *container_parent );
static void         set_options_list_container_initialized( GtkWidget *container_parent, gboolean initialized );
static gboolean     get_options_list_editable( GtkWidget *container_parent );
static void         set_options_list_editable( GtkWidget *container_parent, gboolean editable );
static GtkWidget   *get_options_list_first_button( GtkWidget *container );
static void         set_options_list_first_button( GtkWidget *container, GtkWidget *button );
static gboolean     get_options_list_instance_initialized( const NAIOptionsList *instance );
static void         set_options_list_instance_initialized( const NAIOptionsList *instance, gboolean initialized );
static NAIOption   *get_options_list_option( GtkWidget *container );
static void         set_options_list_option( GtkWidget *container, NAIOption *option );
static const gchar *get_options_list_option_id( GtkWidget *container );
static void         set_options_list_option_id( GtkWidget *container, const gchar *id );
static gboolean     get_options_list_sensitive( GtkWidget *container_parent );
static void         set_options_list_sensitive( GtkWidget *container_parent, gboolean sensitive );
static void         check_for_initializations( const NAIOptionsList *instance, GtkWidget *container_parent );
static void         on_parent_container_finalized( gpointer user_data, GObject *container );
static void         on_instance_finalized( gpointer user_data, GObject *instance );
static void         radio_button_create_group( const NAIOptionsList *instance, GtkWidget *container_parent, gboolean with_ask );
static void         radio_button_draw_vbox( GtkWidget *container_parent, const NAIOption *option );
static void         radio_button_weak_notify( NAIOption *option, GObject *vbox );
static void         tree_view_create_model( const NAIOptionsList *instance, GtkWidget *container_parent );
static void         tree_view_populate( const NAIOptionsList *instance, GtkWidget *container_parent, gboolean with_ask );
static void         tree_view_add_item( GtkTreeView *listview, GtkTreeModel *model, const NAIOption *option );
static void         tree_view_weak_notify( GtkTreeModel *model, GObject *tree_view );
static void         radio_button_select_iter( GtkWidget *container_option, GtkWidget *container_parent );
static gboolean     tree_view_select_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GtkWidget *container_parent );
static void         radio_button_get_selected_iter( GtkWidget *container_option, GtkWidget *container_parent );
static void         tree_view_get_selected( const NAIOptionsList *instance, GtkWidget *container_parent );

/**
 * na_ioptions_list_get_type:
 *
 * Returns: the #GType type of this interface.
 */
GType
na_ioptions_list_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

/*
 * na_ioptions_list_register_type:
 *
 * Registers this interface.
 */
static GType
register_type( void )
{
	static const gchar *thisfn = "na_ioptions_list_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIOptionsListInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIOptionsList", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIOptionsListInterface *iface )
{
	static const gchar *thisfn = "na_ioptions_list_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: iface=%p (%s)", thisfn, ( void * ) iface, G_OBJECT_CLASS_NAME( iface ));

		iface->private = g_new0( NAIOptionsListInterfacePrivate, 1 );

		iface->get_version = ioptions_list_get_version;
		iface->get_options = NULL;
		iface->free_options = ioptions_list_free_options;
		iface->get_ask_option = NULL;
		iface->free_ask_option = ioptions_list_free_ask_option;
	}

	st_initializations += 1;
}

static void
interface_base_finalize( NAIOptionsListInterface *iface )
{
	static const gchar *thisfn = "na_ioptions_list_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

		g_free( iface->private );
	}
}

/*
 * defaults implemented by the interface
 */
static guint
ioptions_list_get_version( const NAIOptionsList *instance )
{
	return( 1 );
}

static void
ioptions_list_free_options( const NAIOptionsList *instance, GtkWidget *container_parent, GList *options )
{
	static const gchar *thisfn = "na_ioptions_list_free_options";

	g_debug( "%s: instance=%p, container_parent=%p, options=%p",
			thisfn, ( void * ) instance, ( void * ) container_parent, ( void * ) options );

	g_list_foreach( options, ( GFunc ) g_object_unref, NULL );
	g_list_free( options );
}

static void
ioptions_list_free_ask_option( const NAIOptionsList *instance, GtkWidget *container_parent, NAIOption *ask_option )
{
	static const gchar *thisfn = "na_ioptions_list_free_ask_option";

	g_debug( "%s: instance=%p, container_parent=%p, ask_option=%p",
			thisfn, ( void * ) instance, ( void * ) container_parent, ( void * ) ask_option );

	g_object_unref( ask_option );
}

/*
 * call these functions will trigger either the implementation method
 * or the default provided by the interface
 */
static GList *
options_list_get_options( const NAIOptionsList *instance, GtkWidget *container_parent )
{
	GList *options;

	options = NULL;

	if( NA_IOPTIONS_LIST_GET_INTERFACE( instance )->get_options ){
		options = NA_IOPTIONS_LIST_GET_INTERFACE( instance )->get_options( instance, container_parent );
	}

	return( options );
}

static void
options_list_free_options( const NAIOptionsList *instance, GtkWidget *container_parent, GList *options )
{
	if( NA_IOPTIONS_LIST_GET_INTERFACE( instance )->free_options ){
		NA_IOPTIONS_LIST_GET_INTERFACE( instance )->free_options( instance, container_parent, options );
	}
}

static NAIOption *
options_list_get_ask_option( const NAIOptionsList *instance, GtkWidget *container_parent )
{
	NAIOption *option;

	option = NULL;

	if( NA_IOPTIONS_LIST_GET_INTERFACE( instance )->get_ask_option ){
		option = NA_IOPTIONS_LIST_GET_INTERFACE( instance )->get_ask_option( instance, container_parent );
	}

	return( option );
}

static void
options_list_free_ask_option( const NAIOptionsList *instance, GtkWidget *container_parent, NAIOption *ask_option )
{
	if( NA_IOPTIONS_LIST_GET_INTERFACE( instance )->free_ask_option ){
		NA_IOPTIONS_LIST_GET_INTERFACE( instance )->free_ask_option( instance, container_parent, ask_option );
	}
}

/*
 * get/set pseudo-properties
 * pseudo properties are set:
 * - against the instance: whether it has been initialized
 * - against the parent container: editable, sensitive, default and current options
 * - against each option container when drawing inside of a VBox: the corresponding option
 */
/* whether the container has been initialized
 *
 * initializing the container so that its pseudo-properties are valid
 */
static gboolean
get_options_list_container_initialized( GtkWidget *container_parent )
{
	gboolean initialized;

	initialized = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_INITIALIZED ));

	return( initialized );
}

static void
set_options_list_container_initialized( GtkWidget *container_parent, gboolean initialized )
{
	g_object_set_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_INITIALIZED, GUINT_TO_POINTER( initialized ));
}

/* whether the selectable user's preference is editable
 * most of the time, a user's preference is not editable if it is set as mandatory,
 * or if the whole user's preference are not writable
 */
static gboolean
get_options_list_editable( GtkWidget *container_parent )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_EDITABLE ));

	return( editable );
}

static void
set_options_list_editable( GtkWidget *container_parent, gboolean editable )
{
	g_object_set_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_EDITABLE, GUINT_TO_POINTER( editable ));
}

/* stores the first button of the radio button group
 */
static GtkWidget *
get_options_list_first_button( GtkWidget *container_parent )
{
	GtkWidget *button;

	button = ( GtkWidget * ) g_object_get_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_FIRST_BUTTON );

	return( button );
}

static void
set_options_list_first_button( GtkWidget *container_parent, GtkWidget *button )
{
	g_object_set_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_FIRST_BUTTON, button );
}

/* whether the instance has been initialized
 *
 * initializing the instance let us register a 'weak notify' signal on the instance
 * we will so be able to free any allocated resources when the instance will be
 * finalized
 */
static gboolean
get_options_list_instance_initialized( const NAIOptionsList *instance )
{
	gboolean initialized;

	initialized = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( instance ), IOPTIONS_LIST_DATA_INITIALIZED ));

	return( initialized );
}

static void
set_options_list_instance_initialized( const NAIOptionsList *instance, gboolean initialized )
{
	g_object_set_data( G_OBJECT( instance ), IOPTIONS_LIST_DATA_INITIALIZED, GUINT_TO_POINTER( initialized ));
}

/* the parent container: the current option
 * the option container: the option attached to this container (VBox only)
 */
static NAIOption *
get_options_list_option( GtkWidget *container )
{
	return(( NAIOption * ) g_object_get_data( G_OBJECT( container ), IOPTIONS_LIST_DATA_OPTION ));
}

static void
set_options_list_option( GtkWidget *container, NAIOption *option )
{
	g_object_set_data( G_OBJECT( container ), IOPTIONS_LIST_DATA_OPTION, option );
}

/* the string identifier of the option (stored as a GQuark)
 * it is set in two places:
 * - at the option container level, in order to identify the option (gtk initialization)
 * - at the parent container level, when setting the default option (display initialization)
 */
static const gchar *
get_options_list_option_id( GtkWidget *container )
{
	GQuark quark;
	const gchar *id;

	quark = GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( container ), IOPTIONS_LIST_DATA_OPTION_ID ));
	id = g_quark_to_string( quark );

	return( id );
}

static void
set_options_list_option_id( GtkWidget *container, const gchar *id )
{
	g_object_set_data( G_OBJECT( container ), IOPTIONS_LIST_DATA_OPTION_ID, GUINT_TO_POINTER( g_quark_from_string( id )));
}

/* whether the selectable user's preference is sensitive
 *
 * an option should be made insensitive when it is not relevant in
 * the considered case
 */
static gboolean
get_options_list_sensitive( GtkWidget *container_parent )
{
	gboolean sensitive;

	sensitive = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_SENSITIVE ));

	return( sensitive );
}

static void
set_options_list_sensitive( GtkWidget *container_parent, gboolean sensitive )
{
	g_object_set_data( G_OBJECT( container_parent ), IOPTIONS_LIST_DATA_SENSITIVE, GUINT_TO_POINTER( sensitive ));
}

/*
 * check_for_initializations:
 * @instance: the interface implementation.
 * @container: the #Gtkwidget which embeds our list of values.
 *
 * Setup both instance and container initialization levels.
 *
 * Having these levels explicitely initialized not only let us setup weak
 * references on the objects, but also assert that pseudo-properties have
 * valid values.
 */
static void
check_for_initializations( const NAIOptionsList *instance, GtkWidget *container_parent )
{
	static const gchar *thisfn = "na_ioptions_list_check_for_initializations";

	if( !get_options_list_instance_initialized( instance )){

		g_debug( "%s: instance=%p", thisfn, ( void * ) instance );

		g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );

		set_options_list_instance_initialized( instance, TRUE );
	}

	if( !get_options_list_container_initialized( container_parent )){

		g_debug( "%s: container_parent=%p", thisfn, ( void * ) container_parent );

		set_options_list_editable( container_parent, TRUE );
		set_options_list_sensitive( container_parent, TRUE );

		g_object_weak_ref( G_OBJECT( container_parent ), ( GWeakNotify ) on_parent_container_finalized, NULL );

		set_options_list_container_initialized( container_parent, TRUE );
	}
}

static void
on_parent_container_finalized( gpointer user_data, GObject *container )
{
	static const gchar *thisfn = "na_ioptions_list_on_parent_container_finalized";

	g_debug( "%s: user_data=%p, container=%p", thisfn, ( void * ) user_data, ( void * ) container );
}

static void
on_instance_finalized( gpointer user_data, GObject *instance )
{
	static const gchar *thisfn = "na_ioptions_list_on_instance_finalized";

	g_debug( "%s: user_data=%p, instance=%p", thisfn, ( void * ) user_data, ( void * ) instance );
}

/*
 * na_ioptions_list_gtk_init:
 * @instance: the object which implements this #NAIOptionsList interface.
 * @container_parent: the #GtkWidget parent container in which we are going to setup the
 *  list of values. @container_parent may be a #GtkVBox or a #GtkTreeView.
 * @with_ask: whether we should also display an 'Ask me' option.
 *
 * Initialize the gtk objects which will be used to display the selection.
 */
void
na_ioptions_list_gtk_init( const NAIOptionsList *instance, GtkWidget *container_parent, gboolean with_ask )
{
	static const gchar *thisfn = "na_ioptions_list_gtk_init";

	g_return_if_fail( NA_IS_IOPTIONS_LIST( instance ));

	check_for_initializations( instance, container_parent );

	g_debug( "%s: instance=%p (%s), container_parent=%p (%s), with_ask=%s",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ),
			with_ask ? "True":"False" );

	if( GTK_IS_BOX( container_parent )){
		radio_button_create_group( instance, container_parent, with_ask );

	} else if( GTK_IS_TREE_VIEW( container_parent )){
		tree_view_create_model( instance, container_parent );
		tree_view_populate( instance, container_parent, with_ask );

	} else {
		g_warning( "%s: unknown container_parent type: %s", thisfn, G_OBJECT_TYPE_NAME( container_parent ));
	}
}

static void
radio_button_create_group( const NAIOptionsList *instance, GtkWidget *container_parent, gboolean with_ask )
{
	static const gchar *thisfn = "na_ioptions_list_radio_button_create_group";
	GList *options, *iopt;
	NAIOption *option;

	g_debug( "%s: instance=%p, container_parent=%p (%s), with_ask=%s",
			thisfn,
			( void * ) instance,
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ),
			with_ask ? "True":"False" );

	options = options_list_get_options( instance, container_parent );

	/* draw the formats as a group of radio buttons
	 */
	for( iopt = options ; iopt ; iopt = iopt->next ){
		radio_button_draw_vbox( container_parent, NA_IOPTION( iopt->data ));
	}

	options_list_free_options( instance, container_parent, options );

	/* eventually add the 'Ask me' mode
	 */
	if( with_ask ){
		option = options_list_get_ask_option( instance, container_parent );
		radio_button_draw_vbox( container_parent, option );
		options_list_free_ask_option( instance, container_parent, option );
	}
}

/*
 * @container_parent_parent used to be a glade-defined GtkVBox in which we dynamically
 * add a radio button and its label for each mode:
 *  +- vbox
 *  |   +- radio button + label
 *  |   +- radio button + label
 */
static void
radio_button_draw_vbox( GtkWidget *container_parent, const NAIOption *option )
{
#if 0
	static const gchar *thisfn = "na_ioptions_list_radio_button_draw_vbox";
#endif
	gchar *description;
	GtkWidget *button;
	GtkWidget *first;
	gchar *label;

	/* first button of the group does not have the property set
	 */
	label = na_ioption_get_label( option );
	first = get_options_list_first_button( container_parent );
	if( first ){
		button = gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( first ), label );
	} else {
		button = gtk_radio_button_new_with_label( NULL, label );
		set_options_list_first_button( container_parent, button );
	}
	g_free( label );
	gtk_button_set_use_underline( GTK_BUTTON( button ), TRUE );

	description = na_ioption_get_description( option );
	g_object_set( G_OBJECT( button ), "tooltip-text", description, NULL );
	g_free( description );

	gtk_box_pack_start( GTK_BOX( container_parent ), button, FALSE, TRUE, 0 );

	set_options_list_option( button, g_object_ref(( gpointer ) option ));
	g_object_weak_ref( G_OBJECT( button ), ( GWeakNotify ) radio_button_weak_notify, ( gpointer ) option );
}

/*
 * release the resources attached to each individual 'option' container
 * when destroying the window
 */
static void
radio_button_weak_notify( NAIOption *option, GObject *vbox )
{
	static const gchar *thisfn = "na_ioptions_list_radio_button_weak_notify";

	g_debug( "%s: option=%p, vbox=%p", thisfn, ( void * ) option, ( void * ) vbox );

	g_object_unref( option );
}

static void
tree_view_create_model( const NAIOptionsList *instance, GtkWidget *container_parent )
{
	static const gchar *thisfn = "na_ioptions_list_tree_view_create_model";
	GtkListStore *model;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	g_return_if_fail( GTK_IS_TREE_VIEW( container_parent ));
	g_debug( "%s: instance=%p, container_parent=%p (%s)",
			thisfn,
			( void * ) instance,
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ));

	model = gtk_list_store_new( N_COLUMN, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_OBJECT );
	gtk_tree_view_set_model( GTK_TREE_VIEW( container_parent ), GTK_TREE_MODEL( model ));
	g_object_unref( model );

	/* create visible columns on the tree view
	 */
	column = gtk_tree_view_column_new_with_attributes(
			"image",
			gtk_cell_renderer_pixbuf_new(),
			"pixbuf", IMAGE_COLUMN,
			NULL );
	gtk_tree_view_append_column( GTK_TREE_VIEW( container_parent ), column );

	column = gtk_tree_view_column_new_with_attributes(
			"label",
			gtk_cell_renderer_text_new(),
			"text", LABEL_COLUMN,
			NULL );
	gtk_tree_view_append_column( GTK_TREE_VIEW( container_parent ), column );

	g_object_set( G_OBJECT( container_parent ), "tooltip-column", TOOLTIP_COLUMN, NULL );

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( container_parent ));
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );

	g_object_weak_ref( G_OBJECT( container_parent ), ( GWeakNotify ) tree_view_weak_notify, ( gpointer ) model );
}

static void
tree_view_populate( const NAIOptionsList *instance, GtkWidget *container_parent, gboolean with_ask )
{
	static const gchar *thisfn = "na_ioptions_list_tree_view_populate";
	GtkTreeModel *model;
	NAIOption *option;
	GList *options, *iopt;

	g_return_if_fail( GTK_IS_TREE_VIEW( container_parent ));
	g_debug( "%s: instance=%p, container_parent=%p (%s), with_ask=%s",
			thisfn,
			( void * ) instance,
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ),
			with_ask ? "True":"False" );

	model = gtk_tree_view_get_model( GTK_TREE_VIEW( container_parent ));
	options = options_list_get_options( instance, container_parent );

	for( iopt = options ; iopt ; iopt = iopt->next ){
		option = NA_IOPTION( iopt->data );
		tree_view_add_item( GTK_TREE_VIEW( container_parent ), model, option );
	}

	options_list_free_options( instance, container_parent, options );

	/* eventually add the 'Ask me' mode
	 */
	if( with_ask ){
		option = options_list_get_ask_option( instance, container_parent );
		tree_view_add_item( GTK_TREE_VIEW( container_parent ), model, option );
		options_list_free_ask_option( instance, container_parent, option );
	}
}

static void
tree_view_add_item( GtkTreeView *listview, GtkTreeModel *model, const NAIOption *option )
{
	GtkTreeIter iter;
	gchar *label, *label2, *description;
	GdkPixbuf *pixbuf;

	label = na_ioption_get_label( option );
	label2 = na_core_utils_str_remove_char( label, "_" );
	description = na_ioption_get_description( option );
	pixbuf = na_ioption_get_pixbuf( option );
	gtk_list_store_append( GTK_LIST_STORE( model ), &iter );
	gtk_list_store_set(
			GTK_LIST_STORE( model ),
			&iter,
			IMAGE_COLUMN, pixbuf,
			LABEL_COLUMN, label2,
			TOOLTIP_COLUMN, description,
			OBJECT_COLUMN, option,
			-1 );
	if( pixbuf ){
		g_object_unref( pixbuf );
	}
	g_free( label );
	g_free( label2 );
	g_free( description );
}

/*
 * release the data structure attached to each individual 'option' container
 * when destroying the window
 */
static void
tree_view_weak_notify( GtkTreeModel *model, GObject *tree_view )
{
	static const gchar *thisfn = "na_iptions_list_tree_view_weak_notify";

	g_debug( "%s: model=%p, tree_view=%p", thisfn, ( void * ) model, ( void * ) tree_view );
}

/*
 * na_ioptions_list_set_default:
 * @instance: the object which implements this #NAIOptionsList interface.
 * @container_parent: the #GtkWidget which embeds our list of values.
 * @default_id: the string identifier of the #NAIOption to be set as the default.
 *
 * Set the default, either of the radio button group or of the tree view.
 */
void
na_ioptions_list_set_default(
		const NAIOptionsList *instance, GtkWidget *container_parent, const gchar *default_id )
{
	static const gchar *thisfn = "na_ioptions_list_set_default";
	GtkTreeModel *model;

	g_return_if_fail( NA_IS_IOPTIONS_LIST( instance ));

	check_for_initializations( instance, container_parent );

	g_debug( "%s: instance=%p (%s), container_parent=%p (%s), default_id=%s",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ),
			default_id );

	set_options_list_option_id( container_parent, default_id );

	if( GTK_IS_BOX( container_parent )){
		gtk_container_foreach(
				GTK_CONTAINER( container_parent ),
				( GtkCallback ) radio_button_select_iter,
				container_parent );

	} else if( GTK_IS_TREE_VIEW( container_parent )){
		model = gtk_tree_view_get_model( GTK_TREE_VIEW( container_parent ));
		gtk_tree_model_foreach(
				model,
				( GtkTreeModelForeachFunc ) tree_view_select_iter,
				container_parent );

	} else {
		g_warning( "%s: unknown container_parent type: %s", thisfn, G_OBJECT_TYPE_NAME( container_parent ));
	}
}

/*
 * iterating through each radio button of the group:
 * - connecting to 'toggled' signal
 * - activating the button which holds our default value
 */
static void
radio_button_select_iter( GtkWidget *button, GtkWidget *container_parent )
{
	const gchar *default_id;
	NAIOption *option;
	gboolean editable, sensitive;
	gchar *option_id;

	default_id = get_options_list_option_id( container_parent );
	option = get_options_list_option( button );
	option_id = na_ioption_get_id( option );

	if( !strcmp( default_id, option_id )){
		editable = get_options_list_editable( container_parent );
		sensitive = get_options_list_sensitive( container_parent );
		na_gtk_utils_radio_set_initial_state( GTK_RADIO_BUTTON( button ), NULL, NULL, editable, sensitive );
		g_debug( "na_ioptions_list_radio_button_select_iter: container_parent=%p, set active button=%p",
				( void * ) container_parent, ( void * ) button );
	}

	g_free( option_id );
}

/*
 * walks through the rows of the liststore until the function returns %TRUE
 */
static gboolean
tree_view_select_iter( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GtkWidget *container_parent )
{
	gboolean stop;
	GtkTreeView *tree_view;
	NAIOption *option;
	const gchar *default_id;
	gchar *option_id;
	GtkTreeSelection *selection;

	stop = FALSE;
	tree_view = ( GtkTreeView * ) container_parent;
	g_return_val_if_fail( GTK_IS_TREE_VIEW( tree_view ), TRUE );
	default_id = get_options_list_option_id( container_parent );

	gtk_tree_model_get( model, iter, OBJECT_COLUMN, &option, -1 );
	g_object_unref( option );
	option_id = na_ioption_get_id( option );

	if( !strcmp( default_id, option_id )){
		selection = gtk_tree_view_get_selection( tree_view );
		gtk_tree_selection_select_iter( selection, iter );
		stop = TRUE;
	}

	g_free( option_id );

	return( stop );
}

/*
 * na_ioptions_list_set_editable:
 * @instance: the object which implements this #NAIOptionsList interface.
 * @container_parent: the #GtkWidget which embeds our list of values.
 * @editable: whether the selectable user's preference is editable;
 *  Most of time, a user's preference is not editable if it is set as mandatory,
 *  or if the whole user's preferences are not writable.
 *
 * Set the @editable pseudo-property on the @instance #NAIOptionsList instance.
 */
void
na_ioptions_list_set_editable( const NAIOptionsList *instance, GtkWidget *container_parent, gboolean editable )
{
	static const gchar *thisfn = "na_ioptions_list_set_editable";

	g_return_if_fail( NA_IS_IOPTIONS_LIST( instance ));

	check_for_initializations( instance, container_parent );

	g_debug( "%s: instance=%p (%s), container_parent=%p (%s), editable=%s",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ),
			editable ? "True":"False" );

	set_options_list_editable( container_parent, editable );
}

/*
 * na_ioptions_list_set_sensitive:
 * @instance: the object which implements this #NAIOptionsList interface.
 * @container_parent: the #GtkWidget which embeds our list of values.
 * @sensitive: whether the radio button group is to be sensitive;
 *  a widget should not be sensitive if the selectable option is not relevant
 *  in the considered case.
 *
 * Set the @sensitive pseudo-property on the @instance #NAIOptionsList instance.
 */
void
na_ioptions_list_set_sensitive( const NAIOptionsList *instance, GtkWidget *container_parent, gboolean sensitive )
{
	static const gchar *thisfn = "na_ioptions_list_set_sensitive";

	g_return_if_fail( NA_IS_IOPTIONS_LIST( instance ));

	check_for_initializations( instance, container_parent );

	g_debug( "%s: instance=%p (%s), container_parent=%p (%s), sensitive=%s",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ),
			sensitive ? "True":"False" );

	set_options_list_sensitive( container_parent, sensitive );
}

/*
 * na_ioptions_list_get_selected:
 * @instance: the object which implements this #NAIOptionsList interface.
 * @container_parent: the #GtkWidget which embeds our list of values.
 *
 * Returns: the currently selected #NAIOption.
 */
NAIOption *
na_ioptions_list_get_selected( const NAIOptionsList *instance, GtkWidget *container_parent )
{
	static const gchar *thisfn = "na_ioptions_list_get_selected";
	NAIOption *option;

	g_return_val_if_fail( NA_IS_IOPTIONS_LIST( instance ), NULL );

	check_for_initializations( instance, container_parent );

	g_debug( "%s: instance=%p (%s), container_parent=%p (%s)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) container_parent, G_OBJECT_TYPE_NAME( container_parent ));

	option = NULL;

	if( GTK_IS_BOX( container_parent )){
		gtk_container_foreach( GTK_CONTAINER( container_parent ),
				( GtkCallback ) radio_button_get_selected_iter, container_parent );
		option = get_options_list_option( container_parent );

	} else if( GTK_IS_TREE_VIEW( container_parent )){
		tree_view_get_selected( instance, container_parent );
		option = get_options_list_option( container_parent );

	} else {
		g_warning( "%s: unknown container_parent type: %s", thisfn, G_OBJECT_TYPE_NAME( container_parent ));
	}

	return( option );
}

static void
radio_button_get_selected_iter( GtkWidget *button, GtkWidget *container_parent )
{
	NAIOption *option;

	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
		option = get_options_list_option( button );
		set_options_list_option( container_parent, option );
		g_debug( "na_ioptions_list_radio_button_get_selected_iter: container_parent=%p, active button=%p",
				( void * ) container_parent, ( void * ) button );
	}
}

static void
tree_view_get_selected( const NAIOptionsList *instance, GtkWidget *container_parent )
{
	GtkTreeSelection *selection;
	GList *rows;
	GtkTreeModel *model;
	GtkTreeIter iter;
	NAIOption *option;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( container_parent ));
	rows = gtk_tree_selection_get_selected_rows( selection, &model );
	g_return_if_fail( g_list_length( rows ) == 1 );

	gtk_tree_model_get_iter( model, &iter, ( GtkTreePath * ) rows->data );
	gtk_tree_model_get( model, &iter, OBJECT_COLUMN, &option, -1 );
	g_object_unref( option );

	g_list_foreach( rows, ( GFunc ) gtk_tree_path_free, NULL );
	g_list_free( rows );

	set_options_list_option( container_parent, option );
}
