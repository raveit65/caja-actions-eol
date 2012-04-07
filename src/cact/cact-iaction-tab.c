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

#include <glib/gi18n.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include <core/na-io-provider.h>

#include "base-iprefs.h"
#include "base-window.h"
#include "cact-application.h"
#include "cact-iprefs.h"
#include "cact-main-statusbar.h"
#include "cact-gtk-utils.h"
#include "cact-iactions-list.h"
#include "cact-main-tab.h"
#include "cact-iaction-tab.h"

/* private interface data
 */
struct CactIActionTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* columns in the icon combobox
 */
enum {
	ICON_STOCK_COLUMN = 0,
	ICON_LABEL_COLUMN,
	ICON_N_COLUMN
};

#define IPREFS_ICONS_DIALOG					"icons-chooser"
#define IPREFS_ICONS_PATH					"icons-path"

/* IActionTab properties, set against the GObject instance
 */
#define IACTION_TAB_PROP_STATUS_CONTEXT		"cact-iaction-tab-status-context"

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;
static gboolean st_on_selection_change = FALSE;

static GType         register_type( void );
static void          interface_base_init( CactIActionTabInterface *klass );
static void          interface_base_finalize( CactIActionTabInterface *klass );

static void          on_iactions_list_column_edited( CactIActionTab *instance, NAObject *object, gchar *text, gint column );
static void          on_tab_updatable_selection_changed( CactIActionTab *instance, gint count_selected );
static void          on_tab_updatable_provider_changed( CactIActionTab *instance, NAObjectItem *item );

static void          on_target_selection_toggled( GtkToggleButton *button, CactIActionTab *instance );
static void          on_target_location_toggled( GtkToggleButton *button, CactIActionTab *instance );

static void          check_for_label( CactIActionTab *instance, GtkEntry *entry, const gchar *label );
static void          on_label_changed( GtkEntry *entry, CactIActionTab *instance );
static void          set_label_label( CactIActionTab *instance, const gchar *color );

static void          on_target_toolbar_toggled( GtkToggleButton *button, CactIActionTab *instance );

static void          on_toolbar_same_label_toggled( GtkToggleButton *button, CactIActionTab *instance );
static void          toolbar_same_label_set_sensitive( CactIActionTab *instance, NAObjectItem *item );

static void          setup_toolbar_label( CactIActionTab *instance, NAObjectItem *item, const gchar *label );
static void          on_toolbar_label_changed( GtkEntry *entry, CactIActionTab *instance );
static void          toolbar_label_set_sensitive( CactIActionTab *instance, NAObjectItem *item );

static void          on_tooltip_changed( GtkEntry *entry, CactIActionTab *instance );

static GtkTreeModel *create_stock_icon_model( void );
static void          icon_combo_list_fill( GtkComboBoxEntry* combo );
static void          on_icon_browse( GtkButton *button, CactIActionTab *instance );
static void          on_icon_changed( GtkEntry *entry, CactIActionTab *instance );
static gint          sort_stock_ids( gconstpointer a, gconstpointer b );
static gchar        *strip_underscore( const gchar *text );
static void          release_icon_combobox( CactIActionTab *instance );

static GtkButton    *get_enabled_button( CactIActionTab *instance );
static void          on_enabled_toggled( GtkToggleButton *button, CactIActionTab *instance );

static void          on_readonly_toggled( GtkToggleButton *button, CactIActionTab *instance );

static void          display_provider_name( CactIActionTab *instance, NAObjectItem *item );

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

	g_type_interface_add_prerequisite( type, BASE_WINDOW_TYPE );

	return( type );
}

static void
interface_base_init( CactIActionTabInterface *klass )
{
	static const gchar *thisfn = "cact_iaction_tab_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactIActionTabInterfacePrivate, 1 );

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( CactIActionTabInterface *klass )
{
	static const gchar *thisfn = "cact_iaction_tab_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
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
 */
void
cact_iaction_tab_initial_load_toplevel( CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_initial_load_toplevel";
	GtkWidget *icon_widget;
	GtkTreeModel *model;
	GtkButton *button;
	GtkRequisition requisition;
	GtkFrame *frame;
	gint size;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	if( st_initialized && !st_finalized ){

		button = GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconBrowseButton" ));
		gtk_widget_size_request( GTK_WIDGET( button ), &requisition );
		g_debug( "%s: button requisition width=%d, height=%d", thisfn, requisition.width, requisition.height );
		frame = GTK_FRAME( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconFrame" ));
		size = requisition.height - 4;
		gtk_widget_set_size_request( GTK_WIDGET( frame ), size, size );
		gtk_frame_set_shadow_type( frame, GTK_SHADOW_IN );

		icon_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconComboBoxEntry" );
		model = create_stock_icon_model();
		gtk_combo_box_set_model( GTK_COMBO_BOX( icon_widget ), model );
		g_object_unref( model );
		icon_combo_list_fill( GTK_COMBO_BOX_ENTRY( icon_widget ));
	}
}

void
cact_iaction_tab_runtime_init_toplevel( CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_runtime_init_toplevel";
	GtkWidget *label_widget, *tooltip_widget, *icon_widget;
	GtkButton *enabled_button;
	GtkWidget *button;

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	if( st_initialized && !st_finalized ){

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				MAIN_WINDOW_SIGNAL_SELECTION_CHANGED,
				G_CALLBACK( on_tab_updatable_selection_changed ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				TAB_UPDATABLE_SIGNAL_PROVIDER_CHANGED,
				G_CALLBACK( on_tab_updatable_provider_changed ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				IACTIONS_LIST_SIGNAL_COLUMN_EDITED,
				G_CALLBACK( on_iactions_list_column_edited ));

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

		icon_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconComboBoxEntry" );
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( GTK_BIN( icon_widget )->child ),
				"changed",
				G_CALLBACK( on_icon_changed ));

		button = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconBrowseButton" );
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( button ),
				"clicked",
				G_CALLBACK( on_icon_browse ));

		enabled_button = get_enabled_button( instance );
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( enabled_button ),
				"toggled",
				G_CALLBACK( on_enabled_toggled ));

		button = base_window_get_widget( BASE_WINDOW( instance ), "ActionReadonlyButton" );
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( button ),
				"toggled",
				G_CALLBACK( on_readonly_toggled ));
	}
}

void
cact_iaction_tab_all_widgets_showed( CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_all_widgets_showed";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	if( st_initialized && !st_finalized ){
	}
}

void
cact_iaction_tab_dispose( CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_dispose";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	if( st_initialized && !st_finalized ){

		release_icon_combobox( instance );
	}
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

	if( st_initialized && !st_finalized ){

		label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelEntry" );
		label = gtk_entry_get_text( GTK_ENTRY( label_widget ));
		has_label = ( g_utf8_strlen( label, -1 ) > 0 );
	}

	return( has_label );
}

static void
on_iactions_list_column_edited( CactIActionTab *instance, NAObject *object, gchar *text, gint column )
{
	GtkWidget *label_widget;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	if( st_initialized && !st_finalized ){

		if( NA_IS_OBJECT_ACTION( object )){
			label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelEntry" );
			gtk_entry_set_text( GTK_ENTRY( label_widget ), text );
		}
	}
}

static void
on_tab_updatable_selection_changed( CactIActionTab *instance, gint count_selected )
{
	static const gchar *thisfn = "cact_iaction_tab_on_tab_updatable_selection_changed";
	NAObjectItem *item;
	gboolean enable_tab;
	gboolean target_selection, target_location, target_toolbar;
	gboolean enable_label;
	gboolean same_label;
	GtkWidget *label_widget, *tooltip_widget, *icon_widget, *title_widget;
	gchar *label, *tooltip, *icon;
	GtkButton *icon_button;
	GtkButton *enabled_button;
	GtkToggleButton *readonly_button;
	gboolean enabled_item;
	GtkToggleButton *toggle;
	gboolean editable;
	GtkNotebook *notebook;
	GtkWidget *page;

	g_debug( "%s: instance=%p, count_selected=%d", thisfn, ( void * ) instance, count_selected );
	g_return_if_fail( BASE_IS_WINDOW( instance ));
	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));

	if( st_initialized && !st_finalized ){

		st_on_selection_change = TRUE;

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &item,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		g_return_if_fail( !item || NA_IS_OBJECT_ITEM( item ));

		enable_tab = ( count_selected == 1 );
		cact_main_tab_enable_page( CACT_MAIN_WINDOW( instance ), TAB_ACTION, enable_tab );

		target_selection = ( item && (
				( NA_IS_OBJECT_ACTION( item ) && na_object_is_target_selection( item )) ||
				( NA_IS_OBJECT_MENU( item ))));

		target_location = ( item && (
				( NA_IS_OBJECT_ACTION( item ) && na_object_is_target_location( item )) ||
				( NA_IS_OBJECT_MENU( item ))));

		target_toolbar = ( item && (
				( NA_IS_OBJECT_ACTION( item ) && na_object_is_target_toolbar( NA_OBJECT_ACTION( item )))));

		toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetSelectionButton" ));
		gtk_toggle_button_set_active( toggle, target_selection );
		gtk_widget_set_sensitive( GTK_WIDGET( toggle ), item && NA_IS_OBJECT_ACTION( item ));
		cact_gtk_utils_set_editable( GTK_OBJECT( toggle ), editable );

		toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetLocationButton" ));
		gtk_toggle_button_set_active( toggle, target_location );
		gtk_widget_set_sensitive( GTK_WIDGET( toggle ), item && NA_IS_OBJECT_ACTION( item ));
		cact_gtk_utils_set_editable( GTK_OBJECT( toggle ), editable );

		enable_label = ( item && ( NA_IS_OBJECT_MENU( item ) || target_selection || target_location ));
		label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelEntry" );
		label = item ? na_object_get_label( item ) : g_strdup( "" );
		label = label ? label : g_strdup( "" );
		gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
		if( item ){
			check_for_label( instance, GTK_ENTRY( label_widget ), label );
		}
		g_free( label );
		gtk_widget_set_sensitive( label_widget, enable_label );
		cact_gtk_utils_set_editable( GTK_OBJECT( label_widget ), editable );

		toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionTargetToolbarButton" ));
		gtk_toggle_button_set_active( toggle, target_toolbar );
		gtk_widget_set_sensitive( GTK_WIDGET( toggle ), item && NA_IS_OBJECT_ACTION( item ));
		cact_gtk_utils_set_editable( GTK_OBJECT( toggle ), editable );

		toggle = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ToolbarSameLabelButton" ));
		same_label = item && NA_IS_OBJECT_ACTION( item ) ? na_object_is_toolbar_same_label( NA_OBJECT_ACTION( item )) : FALSE;
		gtk_toggle_button_set_active( toggle, same_label );
		toolbar_same_label_set_sensitive( instance, item );
		cact_gtk_utils_set_editable( GTK_OBJECT( toggle ), editable );

		label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelEntry" );
		label = item && NA_IS_OBJECT_ACTION( item ) ? na_object_get_toolbar_label( item ) : g_strdup( "" );
		label = label ? label : g_strdup( "" );
		gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
		g_free( label );
		toolbar_label_set_sensitive( instance, item );
		cact_gtk_utils_set_editable( GTK_OBJECT( label_widget ), editable );

		tooltip_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionTooltipEntry" );
		tooltip = item ? na_object_get_tooltip( item ) : g_strdup( "" );
		tooltip = tooltip ? tooltip : g_strdup( "" );
		gtk_entry_set_text( GTK_ENTRY( tooltip_widget ), tooltip );
		g_free( tooltip );
		gtk_widget_set_sensitive( tooltip_widget, item != NULL );
		cact_gtk_utils_set_editable( GTK_OBJECT( tooltip_widget ), editable );

		icon_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconComboBoxEntry" );
		icon = item ? na_object_get_icon( item ) : g_strdup( "" );
		icon = icon ? icon : g_strdup( "" );
		gtk_entry_set_text( GTK_ENTRY( GTK_BIN( icon_widget )->child ), icon );
		g_free( icon );
		gtk_widget_set_sensitive( icon_widget, item != NULL );
		cact_gtk_utils_set_editable( GTK_OBJECT( icon_widget ), editable );

		icon_button = GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconBrowseButton" ));
		gtk_widget_set_sensitive( GTK_WIDGET( icon_button ), item != NULL );
		cact_gtk_utils_set_editable( GTK_OBJECT( icon_button ), editable );

		if( enable_tab ){
			notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( instance ), "MainNotebook" ));
			page = gtk_notebook_get_nth_page( notebook, TAB_ACTION );
			title_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionPropertiesTitle" );
			if( item && NA_IS_OBJECT_MENU( item )){
				gtk_notebook_set_tab_label_text( notebook, page, _( "Menu" ));
				gtk_label_set_markup( GTK_LABEL( title_widget ), _( "<b>Menu properties</b>" ));
			} else {
				gtk_notebook_set_tab_label_text( notebook, page, _( "Action" ));
				gtk_label_set_markup( GTK_LABEL( title_widget ), _( "<b>Action properties</b>" ));
			}
		}

		enabled_button = get_enabled_button( instance );
		enabled_item = item ? na_object_is_enabled( NA_OBJECT_ITEM( item )) : FALSE;
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( enabled_button ), enabled_item );
		gtk_widget_set_sensitive( GTK_WIDGET( enabled_button ), item != NULL );
		cact_gtk_utils_set_editable( GTK_OBJECT( enabled_button ), editable );

		/* read-only toggle only indicates the intrinsic writability status of this item
		 * _not_ the writability status of the provider
		 */
		readonly_button = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionReadonlyButton" ));
		gtk_toggle_button_set_active( readonly_button, item ? na_object_is_readonly( item ) : FALSE );
		gtk_widget_set_sensitive( GTK_WIDGET( readonly_button ), item != NULL );
		cact_gtk_utils_set_editable( GTK_OBJECT( readonly_button ), FALSE );

		label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionItemID" );
		label = item ? na_object_get_id( item ) : g_strdup( "" );
		gtk_label_set_text( GTK_LABEL( label_widget ), label );
		g_free( label );
		gtk_widget_set_sensitive( label_widget, item != NULL );

		display_provider_name( instance, item );

		st_on_selection_change = FALSE;
	}
}

static void
on_tab_updatable_provider_changed( CactIActionTab *instance, NAObjectItem *item )
{
	static const gchar *thisfn = "cact_iaction_tab_on_tab_updatable_provider_changed";

	g_debug( "%s: instance=%p, item=%p", thisfn, ( void * ) instance, ( void * ) item );

	if( st_initialized && !st_finalized ){

		display_provider_name( instance, item );
	}
}

static void
on_target_selection_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_target_selection_toggled";
	NAObjectAction *action;
	gboolean is_target;
	gboolean editable;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &action,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		g_debug( "%s: item=%p (%s), editable=%s",
				thisfn, ( void * ) action, action ? G_OBJECT_TYPE_NAME( action ) : "(null)",
				editable ? "True":"False" );

		if( action && NA_IS_OBJECT_ACTION( action )){

			is_target = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_target_selection( action, is_target );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ENABLE_TAB, action );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, action, FALSE );

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
	NAObjectAction *action;
	gboolean is_target;
	gboolean editable;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &action,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		g_debug( "%s: item=%p (%s), editable=%s",
				thisfn, ( void * ) action, action ? G_OBJECT_TYPE_NAME( action ) : "(null)",
				editable ? "True":"False" );

		if( action && NA_IS_OBJECT_ACTION( action )){

			is_target = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_target_location( action, is_target );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ENABLE_TAB, action );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, action, FALSE );

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
	NAObjectItem *edited;

	g_return_if_fail( CACT_IS_IACTION_TAB( instance ));
	g_return_if_fail( GTK_IS_ENTRY( entry ));

	if( st_initialized && !st_finalized ){

		cact_main_statusbar_hide_status(
				CACT_MAIN_WINDOW( instance ),
				IACTION_TAB_PROP_STATUS_CONTEXT );

		set_label_label( instance, "black" );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &edited,
				NULL );

		if( edited && g_utf8_strlen( label, -1 ) == 0 ){

			/* i18n: status bar message when the action label is empty */
			cact_main_statusbar_display_status(
					CACT_MAIN_WINDOW( instance ),
					IACTION_TAB_PROP_STATUS_CONTEXT,
					_( "Caution: a label is mandatory for the action or the menu." ));

			set_label_label( instance, "red" );
		}
	}
}

static void
on_label_changed( GtkEntry *entry, CactIActionTab *instance )
{
	NAObjectItem *edited;
	const gchar *label;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &edited,
			NULL );

	if( edited ){
		label = gtk_entry_get_text( entry );
		na_object_set_label( edited, label );
		check_for_label( instance, entry, label );

		if( NA_IS_OBJECT_ACTION( edited )){
			setup_toolbar_label( instance, edited, label );
		}

		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, TRUE );
	}
}

static void
set_label_label( CactIActionTab *instance, const gchar *color_str )
{
	GtkWidget *label;
	GdkColor color;

	label = base_window_get_widget( BASE_WINDOW( instance ), "ActionMenuLabelLabel" );
	gdk_color_parse( color_str, &color );
	gtk_widget_modify_fg( label, GTK_STATE_NORMAL, &color );
}

static void
on_target_toolbar_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_target_toolbar_toggled";
	NAObjectAction *action;
	gboolean is_target;
	gboolean editable;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &action,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		if( action && NA_IS_OBJECT_ACTION( action )){

			is_target = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_target_toolbar( action, is_target );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ENABLE_TAB, action );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, action, FALSE );
				toolbar_same_label_set_sensitive( instance, NA_OBJECT_ITEM( action ));
				toolbar_label_set_sensitive( instance, NA_OBJECT_ITEM( action ));

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
	NAObjectItem *edited;
	gboolean same_label;
	gboolean editable;
	gchar *label;
	GtkWidget *label_widget;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &edited,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		if( edited && NA_IS_OBJECT_ACTION( edited )){

			same_label = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_toolbar_same_label( NA_OBJECT_ACTION( edited ), same_label );
				if( same_label ){
					label = na_object_get_label( edited );
					label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionToolbarLabelEntry" );
					gtk_entry_set_text( GTK_ENTRY( label_widget ), label );
					g_free( label );
				}
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
				toolbar_same_label_set_sensitive( instance, NA_OBJECT_ITEM( edited ));
				toolbar_label_set_sensitive( instance, NA_OBJECT_ITEM( edited ));

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
	NAObjectItem *edited;
	const gchar *label;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &edited,
			NULL );

	if( edited && NA_IS_OBJECT_ACTION( edited )){

		label = gtk_entry_get_text( entry );
		na_object_set_toolbar_label( NA_OBJECT_ACTION( edited ), label );

		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
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
	NAObjectItem *edited;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &edited,
			NULL );

	if( edited ){
		na_object_set_tooltip( edited, gtk_entry_get_text( entry ));
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
	}
}

static GtkTreeModel *
create_stock_icon_model( void )
{
	GtkStockItem stock_item;
	gchar* label;
	GtkListStore *model;
	GtkTreeIter row;
	GSList *stock_list, *iter;
	GtkIconTheme *icon_theme;
	GtkIconInfo *icon_info;

	model = gtk_list_store_new( ICON_N_COLUMN, G_TYPE_STRING, G_TYPE_STRING );

	gtk_list_store_append( model, &row );
	/* i18n notes: when no icon is selected in the drop-down list */
	gtk_list_store_set( model, &row, ICON_STOCK_COLUMN, "", ICON_LABEL_COLUMN, _( "None" ), -1 );

	stock_list = gtk_stock_list_ids();
	icon_theme = gtk_icon_theme_get_default();
	stock_list = g_slist_sort( stock_list, ( GCompareFunc ) sort_stock_ids );

	for( iter = stock_list ; iter ; iter = iter->next ){
		icon_info = gtk_icon_theme_lookup_icon( icon_theme, ( gchar * ) iter->data, GTK_ICON_SIZE_MENU, GTK_ICON_LOOKUP_GENERIC_FALLBACK );
		if( icon_info ){
			if( gtk_stock_lookup(( gchar * ) iter->data, &stock_item )){
				gtk_list_store_append( model, &row );
				label = strip_underscore( stock_item.label );
				gtk_list_store_set( model, &row, ICON_STOCK_COLUMN, ( gchar * ) iter->data, ICON_LABEL_COLUMN, label, -1 );
				g_free( label );
			}
			gtk_icon_info_free( icon_info );
		}
	}

	g_slist_foreach( stock_list, ( GFunc ) g_free, NULL );
	g_slist_free( stock_list );

	return( GTK_TREE_MODEL( model ));
}

static void
icon_combo_list_fill( GtkComboBoxEntry* combo )
{
	GtkCellRenderer *cell_renderer_pix;
	GtkCellRenderer *cell_renderer_text;

	if( gtk_combo_box_entry_get_text_column( combo ) == -1 ){
		gtk_combo_box_entry_set_text_column( combo, ICON_STOCK_COLUMN );
	}
	gtk_cell_layout_clear( GTK_CELL_LAYOUT( combo ));

	cell_renderer_pix = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), cell_renderer_pix, FALSE );
	gtk_cell_layout_add_attribute( GTK_CELL_LAYOUT( combo ), cell_renderer_pix, "stock-id", ICON_STOCK_COLUMN );

	cell_renderer_text = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), cell_renderer_text, TRUE );
	gtk_cell_layout_add_attribute( GTK_CELL_LAYOUT( combo ), cell_renderer_text, "text", ICON_LABEL_COLUMN );

	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ), 0 );
}

static void
on_icon_browse( GtkButton *button, CactIActionTab *instance )
{
	GtkWidget *dialog;
	GtkWindow *toplevel;
	gchar *filename;
	GtkWidget *icon_widget;
	CactApplication *application;
	NAUpdater *updater;
	gchar *path;

	toplevel = base_window_get_toplevel( BASE_WINDOW( instance ));

	dialog = gtk_file_chooser_dialog_new(
			_( "Choosing an icon" ),
			toplevel,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL
			);

	base_iprefs_position_named_window( BASE_WINDOW( instance ), GTK_WINDOW( dialog ), IPREFS_ICONS_DIALOG );

	application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( instance )));
	updater = cact_application_get_updater( application );

	path = na_iprefs_read_string( NA_IPREFS( updater ), IPREFS_ICONS_PATH, "" );
	if( path && g_utf8_strlen( path, -1 )){
		gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( dialog ), path );
	}
	g_free( path );

	if( gtk_dialog_run( GTK_DIALOG( dialog )) == GTK_RESPONSE_ACCEPT ){
		filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ));
		icon_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconComboBoxEntry" );
		gtk_entry_set_text( GTK_ENTRY( GTK_BIN( icon_widget )->child ), filename );
	    g_free (filename);

		path = gtk_file_chooser_get_current_folder( GTK_FILE_CHOOSER( dialog ));
	    cact_iprefs_write_string( BASE_WINDOW( instance ), IPREFS_ICONS_PATH, path );
		g_free( path );
	  }

	base_iprefs_save_named_window_position( BASE_WINDOW( instance ), GTK_WINDOW( dialog ), IPREFS_ICONS_DIALOG );

	gtk_widget_destroy( dialog );
}

static void
on_icon_changed( GtkEntry *icon_entry, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_icon_changed";
	GtkImage *image;
	NAObjectItem *edited;
	const gchar *icon_name;

	g_debug( "%s: entry=%p, instance=%p", thisfn, ( void * ) icon_entry, ( void * ) instance );

	icon_name = NULL;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_EDITED_ACTION, &edited,
			NULL );

	if( edited ){
		icon_name = gtk_entry_get_text( icon_entry );
		na_object_set_icon( edited, icon_name );
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, TRUE );
	}

	image = GTK_IMAGE( base_window_get_widget( BASE_WINDOW( instance ), "ActionIconImage" ));
	cact_gtk_utils_render( icon_name, image, GTK_ICON_SIZE_MENU );
}

static gint
sort_stock_ids( gconstpointer a, gconstpointer b )
{
	GtkStockItem stock_item_a;
	GtkStockItem stock_item_b;
	gchar *label_a, *label_b;
	gboolean is_a, is_b;
	int retv = 0;

	is_a = gtk_stock_lookup(( gchar * ) a, &stock_item_a );
	is_b = gtk_stock_lookup(( gchar * ) b, &stock_item_b );

	if( is_a && !is_b ){
		retv = 1;

	} else if( !is_a && is_b ){
		retv = -1;

	} else if( !is_a && !is_b ){
		retv = 0;

	} else {
		label_a = strip_underscore( stock_item_a.label );
		label_b = strip_underscore( stock_item_b.label );
		retv = na_core_utils_str_collate( label_a, label_b );
		g_free( label_a );
		g_free( label_b );
	}

	return( retv );
}

static gchar *
strip_underscore( const gchar *text )
{
	/* Code from gtk-demo */
	gchar *p, *q, *result;

	result = g_strdup( text );
	p = q = result;
	while( *p ){
		if( *p != '_' ){
			*q = *p;
			q++;
		}
		p++;
	}
	*q = '\0';

	return( result );
}

static void
release_icon_combobox( CactIActionTab *instance )
{
	GtkWidget *combo;
	GtkTreeModel *model;

	combo = base_window_get_widget( BASE_WINDOW( instance ), "ActionIconComboBoxEntry" );
	model = gtk_combo_box_get_model( GTK_COMBO_BOX( combo ));
	gtk_list_store_clear( GTK_LIST_STORE( model ));
}

static GtkButton *
get_enabled_button( CactIActionTab *instance )
{
	return( GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionEnabledButton" )));
}

static void
on_enabled_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_enabled_toggled";
	NAObjectItem *edited;
	gboolean enabled;
	gboolean editable;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_EDITED_ACTION, &edited,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		if( edited && NA_IS_OBJECT_ITEM( edited )){

			enabled = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_enabled( edited, enabled );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );

			} else {
				g_signal_handlers_block_by_func(( gpointer ) button, on_enabled_toggled, instance );
				gtk_toggle_button_set_active( button, !enabled );
				g_signal_handlers_unblock_by_func(( gpointer ) button, on_enabled_toggled, instance );
			}
		}
	}
}

static void
on_readonly_toggled( GtkToggleButton *button, CactIActionTab *instance )
{
	static const gchar *thisfn = "cact_iaction_tab_on_readonly_toggled";
	gboolean active;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		active = gtk_toggle_button_get_active( button );

		g_signal_handlers_block_by_func(( gpointer ) button, on_readonly_toggled, instance );
		gtk_toggle_button_set_active( button, !active );
		g_signal_handlers_unblock_by_func(( gpointer ) button, on_readonly_toggled, instance );
	}
}

static void
display_provider_name( CactIActionTab *instance, NAObjectItem *item )
{
	GtkWidget *label_widget;
	gchar *label;
	NAIOProvider *provider;

	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionItemProvider" );
	label = NULL;
	if( item ){
		provider = na_object_get_provider( item );
		if( provider ){
			label = na_io_provider_get_name( provider );
		}
	}
	if( !label ){
		label = g_strdup( "" );
	}
	gtk_label_set_text( GTK_LABEL( label_widget ), label );
	g_free( label );
	gtk_widget_set_sensitive( label_widget, item != NULL );
}
