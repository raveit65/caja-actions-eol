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
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-io-provider.h>

#include "base-window.h"
#include "cact-application.h"
#include "cact-main-statusbar.h"
#include "base-gtk-utils.h"
#include "cact-main-tab.h"
#include "cact-iaction-tab.h"
#include "cact-icon-chooser.h"

/* private interface data
 */
struct _CactIActionTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* Context identifier, set against the menubar
 */
#define IACTION_TAB_CONTEXT				"cact-iaction-tab-context"

/* data set against the instance
 */
typedef struct {
	gboolean on_selection_change;
}
	IActionData;

#define IACTION_TAB_PROP_DATA			"cact-iaction-tab-data"

static guint st_initializations = 0;	/* interface initialisation count */

static GType        register_type( void );
static void         interface_base_init( CactIActionTabInterface *klass );
static void         interface_base_finalize( CactIActionTabInterface *klass );

static void         on_base_initialize_gtk( CactIActionTab *instance, GtkWindow *toplevel, gpointer user_data );
static void         on_base_initialize_window( CactIActionTab *instance, gpointer user_data );

static void         on_tree_view_content_changed( CactIActionTab *instance, NAObject *object, gpointer user_data );
static void         on_main_selection_changed( CactIActionTab *instance, GList *selected_items, gpointer user_data );

static void         on_target_selection_toggled( GtkToggleButton *button, CactIActionTab *instance );
static void         on_target_location_toggled( GtkToggleButton *button, CactIActionTab *instance );
static void         check_for_label( CactIActionTab *instance, GtkEntry *entry, const gchar *label );
static void         on_label_changed( GtkEntry *entry, CactIActionTab *instance );
static void         set_label_label( CactIActionTab *instance, const gchar *color );
static void         on_target_toolbar_toggled( GtkToggleButton *button, CactIActionTab *instance );
static void         on_toolbar_same_label_toggled( GtkToggleButton *button, CactIActionTab *instance );
static void         toolbar_same_label_set_sensitive( CactIActionTab *instance, NAObjectItem *item );
static void         setup_toolbar_label( CactIActionTab *instance, NAObjectItem *item, const gchar *label );
static void         on_toolbar_label_changed( GtkEntry *entry, CactIActionTab *instance );
static void         toolbar_label_set_sensitive( CactIActionTab *instance, NAObjectItem *item );
static void         on_tooltip_changed( GtkEntry *entry, CactIActionTab *instance );
static void         on_icon_browse( GtkButton *button, CactIActionTab *instance );
static void         on_icon_changed( GtkEntry *entry, CactIActionTab *instance );

static IActionData *get_iaction_data( CactIActionTab *instance );
static void         on_instance_finalized( gpointer user_data, CactIActionTab *instance );

GType
cact_iaction_tab_get_type( void )
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
	static const gchar *thisfn = "cact_iaction_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactIActionTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "CactIActionTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_TYPE_WINDOW );

	return( type );
}

static void
interface_base_init( CactIActionTabInterface *klass )
{
	static const gchar *thisfn = "cact_iaction_tab_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactIActionTabInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( CactIActionTabInterface *klass )
{
	static const gchar *thisfn = "cact_iaction_tab_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/**
 * cact_iaction_tab_init:
 * @instance: this #CactIActionTab instance.
 *
 * Initialize the interface
 * Connect to #BaseWindow signals
 */
void
cact_iaction_tab_init( CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_init";
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	g_debug( "%s: instance=%p (%s)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_GTK,
			G_CALLBACK( on_base_initialize_gtk ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_base_initialize_window ));

	cact_main_tab_init( CACT_MAIN_WINDOW( instance ), TAB_ACTION );

	data = get_iaction_data( instance );
	data->on_selection_change = FALSE;

	g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
}

/*
 * GTK_ICON_SIZE_MENU         : 16x16
 * GTK_ICON_SIZE_SMALL_TOOLBAR: 18x18
 * GTK_ICON_SIZE_LARGE_TOOLBAR: 24x24
 * GTK_ICON_SIZE_BUTTON       : 20x20
 * GTK_ICON_SIZE_DND          : 32x32
 * GTK_ICON_SIZE_DIALOG       : 48x48
 *
 * icon is rendered for GTK_ICON_SIZE_MENU (na_object_item_get_pixbuf)
 *
 * Starting with 3.0.3, the ComboBox is dynamically created into its container.
 * Starting with 3.1.0, the ComboBox is replaced with a GtkEntry (thanks to new
 * Icon Chooser).
 */
static void
on_base_initialize_gtk( CactIActionTab *instance, GtkWindow *toplevel, void *user_data )
{
	static const gchar *thisfn = "cact_iaction_tab_on_base_initialize_gtk";
	GtkFrame *frame;
	GtkButton *button;
	gint size;

/* gtk_widget_size_request() is deprecated since Gtk+ 3.0
 * see http://library.gnome.org/devel/gtk/unstable/GtkWidget.html#gtk-widget-render-icon
 * and http://git.gnome.org/browse/gtk+/commit/?id=07eeae15825403037b7df139acf9bfa104d5559d
 */
#if GTK_CHECK_VERSION( 2, 91, 7 )
	GtkRequisition minimal_size, natural_size;
#else
	GtkRequisition requisition;
#endif

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	g_debug( "%s: instance=%p (%s), toplevel=%p, user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) toplevel,
			( void * ) user_data );

	button = GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconBrowseButton" ));
	frame = GTK_FRAME( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconFrame" ));

#if GTK_CHECK_VERSION( 2, 91, 7 )
	gtk_widget_get_preferred_size( GTK_WIDGET( button ), &minimal_size, &natural_size );
	size = MAX( minimal_size.height, natural_size.height );
#else
	gtk_widget_size_request( GTK_WIDGET( button ), &requisition );
	size = requisition.height;
#endif

	gtk_widget_set_size_request( GTK_WIDGET( frame ), size, size );
	gtk_frame_set_shadow_type( frame, GTK_SHADOW_IN );
}

static void
on_base_initialize_window( CactIActionTab *instance, void *user_data )
{
	static const gchar *thisfn = "cact_iaction_tab_on_base_initialize_window";
	GtkWidget *label_widget, *tooltip_widget, *icon_entry;
	GtkWidget *button;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_ITEM_UPDATED,
			G_CALLBACK( on_tree_view_content_changed ));

	button = base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetSelectionButton" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( button ),
			"toggled",
			G_CALLBACK( on_target_selection_toggled ));

	button = base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetLocationButton" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( button ),
			"toggled",
			G_CALLBACK( on_target_location_toggled ));

	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelEntry" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( label_widget ),
			"changed",
			G_CALLBACK( on_label_changed ));

	button = base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetToolbarButton" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( button ),
			"toggled",
			G_CALLBACK( on_target_toolbar_toggled ));

	button = base_window_get_widget( BASE_WINDOW( instance ), "ToolbarSameLabelButton" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( button ),
			"toggled",
			G_CALLBACK( on_toolbar_same_label_toggled ));

	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelEntry" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( label_widget ),
			"changed",
			G_CALLBACK( on_toolbar_label_changed ));

	tooltip_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionTooltipEntry" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( tooltip_widget ),
			"changed",
			G_CALLBACK( on_tooltip_changed ));

	icon_entry = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconEntry" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( icon_entry ),
			"changed",
			G_CALLBACK( on_icon_changed ));

	button = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconBrowseButton" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( button ),
			"clicked",
			G_CALLBACK( on_icon_browse ));
}

/**
 * cact_iaction_tab_has_label:
 * @window: this #CactIActionTab instance.
 *
 * An action or a menu can only be written if it has at least a label.
 *
 * Returns %TRUE if the label of the action or of the menu is not empty.
 */
gboolean
cact_iaction_tab_has_label( CactIActionTab *instance )
{
	GtkWidget *label_widget;
	const gchar *label;
	gboolean has_label = FALSE;

	g_return_val_if_fail( CACT_IS_IACTION_TAB( instance ), FALSE );

	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelEntry" );
	label = gtk_entry_get_text( GTK_ENTRY( label_widget ));
	has_label = ( g_utf8_strlen( label, -1 ) > 0 );

	return( has_label );
}

static void
on_tree_view_content_changed( CactIActionTab *instance, NAObject *object, gpointer user_data )
{
	GtkWidget *label_widget;
	gchar *label;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	if( object && NA_IS_OBJECT_ITEM( object )){
		label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelEntry" );
		label = na_object_get_label( object );
		gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
		g_free( label );
	}
}

static void
on_main_selection_changed( CactIActionTab *instance, GList *selected_items, gpointer user_data )
{
	static const gchar *thisfn = "cact_iaction_tab_on_main_selection_changed";
	guint count_selected;
	gboolean enable_tab;
	NAObjectItem *item;
	gboolean editable;
	gboolean target_selection, target_location, target_toolbar;
	gboolean enable_label;
	gboolean same_label;
	GtkWidget *label_widget, *tooltip_widget, *icon_widget;
	gchar *label, *tooltip, *icon;
	GtkButton *icon_button;
	GtkToggleButton *toggle;
	IActionData *data;

	g_return_if_fail( BASE_IS_WINDOW( instance ));
	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	count_selected = g_list_length( selected_items );
	g_debug( "%s: instance=%p (%s), selected_items=%p (count=%u)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) selected_items, count_selected );

	enable_tab = ( count_selected == 1 );
	cact_main_tab_enable_page( CACT_MAIN_WINDOW( instance ), TAB_ACTION, enable_tab );

	data = get_iaction_data( instance );
	data->on_selection_change = TRUE;

	g_object_get(
		G_OBJECT( instance ),
		MAIN_PROP_ITEM, &item,
		MAIN_PROP_EDITABLE, &editable,
		NULL );

	target_selection =
			enable_tab &&
			item != NULL &&
			NA_IS_OBJECT_ACTION( item ) &&
			na_object_is_target_selection( item );

	target_location =
			enable_tab &&
			item != NULL &&
			NA_IS_OBJECT_ACTION( item ) &&
			na_object_is_target_location( item );

	target_toolbar =
			enable_tab &&
			item != NULL &&
			NA_IS_OBJECT_ACTION( item ) &&
			na_object_is_target_toolbar( item );

	toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetSelectionButton" ));
	gtk_toggle_button_set_active( toggle, target_selection || ( item && NA_IS_OBJECT_MENU( item )));
	gtk_widget_set_sensitive( GTK_WIDGET( toggle ), item && NA_IS_OBJECT_ACTION( item ));
	base_gtk_utils_set_editable( G_OBJECT( toggle ), editable );

	toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetLocationButton" ));
	gtk_toggle_button_set_active( toggle, target_location || ( item && NA_IS_OBJECT_MENU( item )));
	gtk_widget_set_sensitive( GTK_WIDGET( toggle ), item && NA_IS_OBJECT_ACTION( item ));
	base_gtk_utils_set_editable( G_OBJECT( toggle ), editable );

	enable_label = target_selection || target_location || ( item && NA_IS_OBJECT_MENU( item ));
	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelEntry" );
	label = item ? na_object_get_label( item ) : g_strdup( "" );
	label = label ? label : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
	if( item ){
		check_for_label( instance, GTK_ENTRY( label_widget ), label );
	}
	g_free( label );
	gtk_widget_set_sensitive( label_widget, enable_label );
	base_gtk_utils_set_editable( G_OBJECT( label_widget ), editable );

	toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetToolbarButton" ));
	gtk_toggle_button_set_active( toggle, target_toolbar );
	gtk_widget_set_sensitive( GTK_WIDGET( toggle ), item && NA_IS_OBJECT_ACTION( item ));
	base_gtk_utils_set_editable( G_OBJECT( toggle ), editable );

	toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ToolbarSameLabelButton" ));
	same_label = item && NA_IS_OBJECT_ACTION( item ) ? na_object_is_toolbar_same_label( item ) : FALSE;
	gtk_toggle_button_set_active( toggle, same_label );
	gtk_widget_set_sensitive( GTK_WIDGET( toggle ), target_toolbar );
	base_gtk_utils_set_editable( G_OBJECT( toggle ), editable );

	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelEntry" );
	label = item && NA_IS_OBJECT_ACTION( item ) ? na_object_get_toolbar_label( item ) : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
	g_free( label );
	gtk_widget_set_sensitive( label_widget, target_toolbar && !same_label );
	base_gtk_utils_set_editable( G_OBJECT( label_widget ), editable );

	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelLabel" );
	gtk_widget_set_sensitive( label_widget, target_toolbar && !same_label );

	tooltip_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionTooltipEntry" );
	tooltip = item ? na_object_get_tooltip( item ) : g_strdup( "" );
	tooltip = tooltip ? tooltip : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( tooltip_widget ), tooltip );
	g_free( tooltip );
	base_gtk_utils_set_editable( G_OBJECT( tooltip_widget ), editable );

	icon_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconEntry" );
	icon = item ? na_object_get_icon( item ) : g_strdup( "" );
	icon = icon ? icon : g_strdup( "" );
	gtk_entry_set_text( GTK_ENTRY( icon_widget ), icon );
	g_free( icon );
	base_gtk_utils_set_editable( G_OBJECT( icon_widget ), editable );

	icon_button = GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconBrowseButton" ));
	base_gtk_utils_set_editable( G_OBJECT( icon_button ), editable );

	data->on_selection_change = FALSE;
}

static void
on_target_selection_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_target_selection_toggled";
	NAObjectItem *item;
	gboolean is_target;
	gboolean editable;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	data = get_iaction_data( instance );

	if( !data->on_selection_change ){
		g_debug( "%s: button=%p, instance=%p (%s)",
				thisfn,
				( void * ) button,
				( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_ITEM, &item,
			MAIN_PROP_EDITABLE, &editable,
			NULL );

		g_debug( "%s: item=%p (%s), editable=%s",
				thisfn, ( void * ) item, item ? G_OBJECT_TYPE_NAME( item ) : "null",
				editable ? "True":"False" );

		if( item && NA_IS_OBJECT_ACTION( item )){
			is_target = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_target_selection( item, is_target );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, 0 );

			} else {
				g_signal_handlers_block_by_func(( gpointer ) button, on_target_selection_toggled, instance );
				gtk_toggle_button_set_active( button, !is_target );
				g_signal_handlers_unblock_by_func(( gpointer ) button, on_target_selection_toggled, instance );
			}
		}
	}
}

static void
on_target_location_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_target_location_toggled";
	NAObjectItem *item;
	gboolean is_target;
	gboolean editable;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	data = get_iaction_data( instance );

	if( !data->on_selection_change ){
		g_debug( "%s: button=%p, instance=%p (%s)",
				thisfn,
				( void * ) button,
				( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_ITEM, &item,
			MAIN_PROP_EDITABLE, &editable,
			NULL );

		g_debug( "%s: item=%p (%s), editable=%s",
				thisfn, ( void * ) item, item ? G_OBJECT_TYPE_NAME( item ) : "null",
				editable ? "True":"False" );

		if( item && NA_IS_OBJECT_ACTION( item )){
			is_target = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_target_location( item, is_target );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, 0 );

			} else {
				g_signal_handlers_block_by_func(( gpointer ) button, on_target_location_toggled, instance );
				gtk_toggle_button_set_active( button, !is_target );
				g_signal_handlers_unblock_by_func(( gpointer ) button, on_target_location_toggled, instance );
			}
		}
	}
}

static void
check_for_label( CactIActionTab *instance, GtkEntry *entry, const gchar *label )
{
	NAObjectItem *item;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));
	g_return_if_fail( GTK_IS_ENTRY( entry ));

	cact_main_statusbar_hide_status(
			CACT_MAIN_WINDOW( instance ),
			IACTION_TAB_CONTEXT );

	set_label_label( instance, "black" );

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_ITEM, &item,
			NULL );

	if( item && g_utf8_strlen( label, -1 ) == 0 ){

		/* i18n: status bar message when the action label is empty */
		cact_main_statusbar_display_status(
				CACT_MAIN_WINDOW( instance ),
				IACTION_TAB_CONTEXT,
				_( "Caution: a label is mandatory for the action or the menu." ));

		set_label_label( instance, "red" );
	}
}

static void
on_label_changed( GtkEntry *entry, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_label_changed";
	NAObjectItem *item;
	const gchar *label;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	data = get_iaction_data( instance );

	if( !data->on_selection_change ){
		g_debug( "%s: entry=%p, instance=%p (%s)",
				thisfn,
				( void * ) entry,
				( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_ITEM, &item,
				NULL );

		if( item ){
			label = gtk_entry_get_text( entry );
			na_object_set_label( item, label );
			check_for_label( instance, entry, label );

			if( NA_IS_OBJECT_ACTION( item )){
				setup_toolbar_label( instance, item, label );
			}

			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, MAIN_DATA_LABEL );
		}
	}
}

static void
set_label_label( CactIActionTab *instance, const gchar *color_str )
{
	GtkWidget *label;

	label = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelLabel" );

/* gtk_widget_modify_fg() is deprecated as of Gtk+ 3.0
 */
#if GTK_CHECK_VERSION( 2, 91, 7 )
	GdkRGBA color;
	gdk_rgba_parse( &color, color_str );
	gtk_widget_override_color( label, GTK_STATE_FLAG_ACTIVE, &color );
#else
	GdkColor color;
	gdk_color_parse( color_str, &color );
	gtk_widget_modify_fg( label, GTK_STATE_NORMAL, &color );
#endif
}

static void
on_target_toolbar_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_target_toolbar_toggled";
	NAObjectAction *item;
	gboolean is_target;
	gboolean editable;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	data = get_iaction_data( instance );

	if( !data->on_selection_change ){
		g_debug( "%s: button=%p, instance=%p (%s)",
				thisfn,
				( void * ) button,
				( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_ITEM, &item,
				MAIN_PROP_EDITABLE, &editable,
				NULL );

		g_debug( "%s: item=%p (%s), editable=%s",
				thisfn, ( void * ) item, item ? G_OBJECT_TYPE_NAME( item ) : "null",
				editable ? "True":"False" );

		if( item && NA_IS_OBJECT_ACTION( item )){
			is_target = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_target_toolbar( item, is_target );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, 0 );
				toolbar_same_label_set_sensitive( instance, NA_OBJECT_ITEM( item ));
				toolbar_label_set_sensitive( instance, NA_OBJECT_ITEM( item ));

			} else {
				g_signal_handlers_block_by_func(( gpointer ) button, on_target_toolbar_toggled, instance );
				gtk_toggle_button_set_active( button, !is_target );
				g_signal_handlers_unblock_by_func(( gpointer ) button, on_target_toolbar_toggled, instance );
			}
		}
	}
}

static void
on_toolbar_same_label_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_toolbar_same_label_toggled";
	NAObjectItem *item;
	gboolean same_label;
	gboolean editable;
	gchar *label;
	GtkWidget *label_widget;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	data = get_iaction_data( instance );

	if( !data->on_selection_change ){
		g_debug( "%s: button=%p, instance=%p (%s)",
				thisfn,
				( void * ) button,
				( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_ITEM, &item,
				MAIN_PROP_EDITABLE, &editable,
				NULL );

		g_debug( "%s: item=%p (%s), editable=%s",
				thisfn, ( void * ) item, item ? G_OBJECT_TYPE_NAME( item ) : "null",
				editable ? "True":"False" );

		if( item && NA_IS_OBJECT_ACTION( item )){
			same_label = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_toolbar_same_label( NA_OBJECT_ACTION( item ), same_label );

				if( same_label ){
					label = na_object_get_label( item );
					label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelEntry" );
					gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
					g_free( label );
				}

				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, 0 );
				toolbar_same_label_set_sensitive( instance, NA_OBJECT_ITEM( item ));
				toolbar_label_set_sensitive( instance, NA_OBJECT_ITEM( item ));

			} else {
				g_signal_handlers_block_by_func(( gpointer ) button, on_toolbar_same_label_toggled, instance );
				gtk_toggle_button_set_active( button, !same_label );
				g_signal_handlers_unblock_by_func(( gpointer ) button, on_toolbar_same_label_toggled, instance );
			}
		}
	}
}

static void
toolbar_same_label_set_sensitive( CactIActionTab *instance, NAObjectItem *item )
{
	GtkToggleButton *toggle;
	gboolean target_toolbar;
	gboolean readonly;

	readonly = item ? na_object_is_readonly( item ) : FALSE;
	toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ToolbarSameLabelButton" ));
	target_toolbar = item && NA_IS_OBJECT_ACTION( item ) ? na_object_is_target_toolbar( NA_OBJECT_ACTION( item )) : FALSE;
	gtk_widget_set_sensitive( GTK_WIDGET( toggle ), target_toolbar && !readonly );
}

/*
 * setup the label of the toolbar according to the toolbar_same_label flag
 */
static void
setup_toolbar_label( CactIActionTab *instance, NAObjectItem *item, const gchar *label )
{
	GtkWidget *label_widget;

	if( item && NA_IS_OBJECT_ACTION( item )){
		if( na_object_is_toolbar_same_label( item )){
			label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelEntry" );
			gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
		}
	}
}

static void
on_toolbar_label_changed( GtkEntry *entry, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_toolbar_label_changed";
	NAObjectItem *item;
	const gchar *label;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	data = get_iaction_data( instance );

	if( !data->on_selection_change ){
		g_debug( "%s: entry=%p, instance=%p (%s)",
				thisfn,
				( void * ) entry,
				( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_ITEM, &item,
				NULL );

		if( item && NA_IS_OBJECT_ACTION( item )){
			label = gtk_entry_get_text( entry );
			na_object_set_toolbar_label( NA_OBJECT_ACTION( item ), label );

			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, 0 );
		}
	}
}

static void
toolbar_label_set_sensitive( CactIActionTab *instance, NAObjectItem *item )
{
	gboolean is_action;
	gboolean same_label;
	GtkWidget *label_widget;

	is_action = item && NA_IS_OBJECT_ACTION( item );
	same_label = is_action ? na_object_is_toolbar_same_label( NA_OBJECT_ACTION( item )) : FALSE;
	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelEntry" );
	gtk_widget_set_sensitive( label_widget, is_action && !same_label );
}

static void
on_tooltip_changed( GtkEntry *entry, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_tooltip_changed";
	NAObjectItem *item;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	data = get_iaction_data( instance );

	if( !data->on_selection_change ){
		g_debug( "%s: entry=%p, instance=%p (%s)",
				thisfn,
				( void * ) entry,
				( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

		g_object_get(
				G_OBJECT( instance ),
				MAIN_PROP_ITEM, &item,
				NULL );

		if( item ){
			na_object_set_tooltip( item, gtk_entry_get_text( entry ));
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, 0 );
		}
	}
}

static void
on_icon_browse( GtkButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_icon_browse";
	NAObjectItem *item;
	GtkWidget *icon_entry;
	gchar *icon_name;
	gchar *new_icon_name;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	g_debug( "%s: button=%p, instance=%p (%s)",
			thisfn,
			( void * ) button,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_ITEM, &item,
			NULL );

	if( item ){
		icon_name = na_object_get_icon( item );
		new_icon_name = cact_icon_chooser_choose_icon( BASE_WINDOW( instance ), icon_name );

		if( g_utf8_collate( icon_name, new_icon_name ) != 0 ){
			icon_entry = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconEntry" );
			gtk_entry_set_text( GTK_ENTRY( icon_entry ), new_icon_name );
		}

		g_free( icon_name );
		g_free( new_icon_name );
	}
}

static void
on_icon_changed( GtkEntry *icon_entry, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_icon_changed";
	GtkImage *image;
	NAObjectItem *item;
	gchar *icon_name;
	IActionData *data;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	g_debug( "%s: icon_entry=%p, instance=%p (%s)",
			thisfn,
			( void * ) icon_entry,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	icon_name = NULL;
	data = get_iaction_data( instance );

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_ITEM, &item,
			NULL );

	if( item ){
		if( !data->on_selection_change ){
			icon_name = g_strdup( gtk_entry_get_text( icon_entry ));
			na_object_set_icon( item, icon_name );
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, item, MAIN_DATA_ICON );

		} else {
			icon_name = na_object_get_icon( item );
		}
	}

	/* icon_name may be null if there is no current item
	 * in such a case, we blank the image
	 */
	image = GTK_IMAGE( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconImage" ));
	base_gtk_utils_render( icon_name, image, GTK_ICON_SIZE_SMALL_TOOLBAR );
	g_free( icon_name );
}

static IActionData *
get_iaction_data( CactIActionTab *instance )
{
	IActionData *data;

	data = ( IActionData * ) g_object_get_data( G_OBJECT( instance ), IACTION_TAB_PROP_DATA );

	if( !data ){
		data = g_new0( IActionData, 1 );
		g_object_set_data( G_OBJECT( instance ), IACTION_TAB_PROP_DATA, data );
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_instance_finalized";
	IActionData *data;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );

	data = get_iaction_data( instance );

	g_free( data );
}
