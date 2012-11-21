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

#include <string.h>

#include <api/na-object-api.h>

#include "base-gtk-utils.h"
#include "cact-main-tab.h"
#include "cact-iexecution-tab.h"

/* private interface data
 */
struct _CactIExecutionTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* data set against the instance
 */
typedef struct {
	gboolean on_selection_change;
}
	IExecutionData;

#define IEXECUTION_TAB_PROP_DATA		"cact-iexecution-tab-data"

static guint st_initializations = 0;	/* interface initialization count */

static GType           register_type( void );
static void            interface_base_init( CactIExecutionTabInterface *klass );
static void            interface_base_finalize( CactIExecutionTabInterface *klass );

static void            on_base_initialize_window( CactIExecutionTab *instance, gpointer user_data );

static void            on_main_selection_changed( CactIExecutionTab *instance, GList *selected_items, gpointer user_data );

static void            on_normal_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance );
static void            on_terminal_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance );
static void            on_embedded_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance );
static void            on_display_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance );
static void            execution_mode_toggle( CactIExecutionTab *instance, GtkToggleButton *togglebutton, GCallback cb, const gchar *mode );
static void            on_startup_notify_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance );
static void            on_startup_class_changed( GtkEntry *entry, CactIExecutionTab *instance );
static void            on_execute_as_changed( GtkEntry *entry, CactIExecutionTab *instance );

static IExecutionData *get_iexecution_data( CactIExecutionTab *instance );
static void            on_instance_finalized( gpointer user_data, CactIExecutionTab *instance );

GType
cact_iexecution_tab_get_type( void )
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
	static const gchar *thisfn = "cact_iexecution_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( CactIExecutionTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "CactIExecutionTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_TYPE_WINDOW );

	return( type );
}

static void
interface_base_init( CactIExecutionTabInterface *klass )
{
	static const gchar *thisfn = "cact_iexecution_tab_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( CactIExecutionTabInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( CactIExecutionTabInterface *klass )
{
	static const gchar *thisfn = "cact_iexecution_tab_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/**
 * cact_iexecution_tab_init:
 * @instance: this #CactIExecutionTab instance.
 *
 * Initialize the interface
 * Connect to #BaseWindow signals
 */
void
cact_iexecution_tab_init( CactIExecutionTab *instance )
{
	static const gchar *thisfn = "cact_iexecution_tab_init";
	IExecutionData *data;

	g_return_if_fail( CACT_IS_IEXECUTION_TAB( instance ));

	g_debug( "%s: instance=%p (%s)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_base_initialize_window ));

	cact_main_tab_init( CACT_MAIN_WINDOW( instance ), TAB_EXECUTION );

	data = get_iexecution_data( instance );
	data->on_selection_change = FALSE;

	g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
}

/*
 * on_base_initialize_window:
 * @window: this #CactIExecutionTab instance.
 *
 * Initializes the tab widget at each time the widget will be displayed.
 * Connect signals and setup runtime values.
 */
static void
on_base_initialize_window( CactIExecutionTab *instance, void *user_data )
{
	static const gchar *thisfn = "cact_iexecution_tab_on_base_initialize_window";

	g_return_if_fail( CACT_IS_IEXECUTION_TAB( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ExecutionModeNormal",
			"toggled",
			G_CALLBACK( on_normal_mode_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ExecutionModeTerminal",
			"toggled",
			G_CALLBACK( on_terminal_mode_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ExecutionModeEmbedded",
			"toggled",
			G_CALLBACK( on_embedded_mode_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ExecutionModeDisplayOutput",
			"toggled",
			G_CALLBACK( on_display_mode_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"StartupNotifyButton",
			"toggled",
			G_CALLBACK( on_startup_notify_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"StartupWMClassEntry",
			"changed",
			G_CALLBACK( on_startup_class_changed ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( instance ),
			"ExecuteAsEntry",
			"changed",
			G_CALLBACK( on_execute_as_changed ));
}

static void
on_main_selection_changed( CactIExecutionTab *instance, GList *selected_items, gpointer user_data )
{
	static const gchar *thisfn = "cact_iexecution_tab_on_main_selection_changed";
	NAObjectProfile *profile;
	gboolean editable;
	gboolean enable_tab;
	gchar *mode;
	GtkWidget *normal_toggle, *terminal_toggle, *embedded_toggle, *display_toggle;
	gboolean notify;
	GtkWidget *notify_check, *frame;
	gchar *class, *user;
	GtkWidget *entry;
	IExecutionData *data;

	g_return_if_fail( CACT_IS_IEXECUTION_TAB( instance ));

	g_debug( "%s: instance=%p (%s), selected_items=%p (count=%d)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) selected_items, g_list_length( selected_items ));

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_PROFILE, &profile,
			MAIN_PROP_EDITABLE, &editable,
			NULL );

	enable_tab = ( profile != NULL );
	cact_main_tab_enable_page( CACT_MAIN_WINDOW( instance ), TAB_EXECUTION, enable_tab );

	data = get_iexecution_data( instance );
	data->on_selection_change = TRUE;

	normal_toggle = base_window_get_widget( BASE_WINDOW( instance ), "ExecutionModeNormal" );
	terminal_toggle = base_window_get_widget( BASE_WINDOW( instance ), "ExecutionModeTerminal" );
	embedded_toggle = base_window_get_widget( BASE_WINDOW( instance ), "ExecutionModeEmbedded" );
	display_toggle = base_window_get_widget( BASE_WINDOW( instance ), "ExecutionModeDisplayOutput" );

	mode = profile ? na_object_get_execution_mode( profile ) : g_strdup( "Normal" );
	gtk_toggle_button_set_inconsistent( GTK_TOGGLE_BUTTON( normal_toggle ), profile == NULL );

	if( !strcmp( mode, "Normal" )){
		base_gtk_utils_radio_set_initial_state(
				GTK_RADIO_BUTTON( normal_toggle ),
				G_CALLBACK( on_normal_mode_toggled ), instance, editable, ( profile != NULL ));

	} else if( !strcmp( mode, "Terminal" )){
		base_gtk_utils_radio_set_initial_state(
				GTK_RADIO_BUTTON( terminal_toggle ),
				G_CALLBACK( on_terminal_mode_toggled ), instance, editable, ( profile != NULL ));

	} else if( !strcmp( mode, "Embedded" )){
		base_gtk_utils_radio_set_initial_state(
				GTK_RADIO_BUTTON( embedded_toggle ),
				G_CALLBACK( on_embedded_mode_toggled ), instance, editable, ( profile != NULL ));

	} else if( !strcmp( mode, "DisplayOutput" )){
		base_gtk_utils_radio_set_initial_state(
				GTK_RADIO_BUTTON( display_toggle ),
				G_CALLBACK( on_display_mode_toggled ), instance, editable, ( profile != NULL ));

	} else {
		g_warning( "%s: unable to setup execution mode '%s'", thisfn, mode );
	}

	g_free( mode );

	frame = base_window_get_widget( BASE_WINDOW( instance ), "StartupModeFrame" );
	gtk_widget_set_sensitive( frame, FALSE );

	notify = profile ? na_object_get_startup_notify( profile ) : FALSE;
	notify_check = base_window_get_widget( BASE_WINDOW( instance ), "StartupNotifyButton" );
	base_gtk_utils_set_editable( G_OBJECT( notify_check ), editable );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( notify_check ), notify );

	class = profile ? na_object_get_startup_class( profile ) : g_strdup( "" );
	entry = base_window_get_widget( BASE_WINDOW( instance ), "StartupWMClassEntry" );
	gtk_entry_set_text( GTK_ENTRY( entry ), class );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );
	g_free( class );

	frame = base_window_get_widget( BASE_WINDOW( instance ), "UserFrame" );
	gtk_widget_set_sensitive( frame, FALSE );

	user = profile ? na_object_get_execute_as( profile ) : g_strdup( "" );
	entry = base_window_get_widget( BASE_WINDOW( instance ), "ExecuteAsEntry" );
	gtk_entry_set_text( GTK_ENTRY( entry ), user );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );
	g_free( user );

	data->on_selection_change = FALSE;
}

static void
on_normal_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance )
{
	execution_mode_toggle( instance, togglebutton, G_CALLBACK( on_normal_mode_toggled ), "Normal" );
}

static void
on_terminal_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance )
{
	execution_mode_toggle( instance, togglebutton, G_CALLBACK( on_terminal_mode_toggled ), "Terminal" );
}

static void
on_embedded_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance )
{
	execution_mode_toggle( instance, togglebutton, G_CALLBACK( on_embedded_mode_toggled ), "Embedded" );
}

static void
on_display_mode_toggled( GtkToggleButton *togglebutton, CactIExecutionTab *instance )
{
	execution_mode_toggle( instance, togglebutton, G_CALLBACK( on_display_mode_toggled ), "DisplayOutput" );
}

static void
execution_mode_toggle( CactIExecutionTab *instance, GtkToggleButton *toggle_button, GCallback cb, const gchar *mode )
{
	NAObjectProfile *profile;
	gboolean editable;
	gboolean active;
	gboolean is_normal;
	GtkWidget *widget;

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_PROFILE, &profile,
			MAIN_PROP_EDITABLE, &editable,
			NULL );

	if( profile ){
		active = gtk_toggle_button_get_active( toggle_button );

		if( editable ){
			if( active ){
				na_object_set_execution_mode( profile, mode );

				is_normal = ( strcmp( mode, "Normal" ) == 0 );
				widget = base_window_get_widget( BASE_WINDOW( instance ), "StartupNotifyButton" );
				gtk_widget_set_sensitive( widget, is_normal );
				widget = base_window_get_widget( BASE_WINDOW( instance ), "StartupWMClassEntry" );
				gtk_widget_set_sensitive( widget, is_normal );

				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, 0 );
			}

		} else {
			base_gtk_utils_radio_reset_initial_state( GTK_RADIO_BUTTON( toggle_button ), cb );
		}
	}
}

static void
on_startup_notify_toggled( GtkToggleButton *toggle_button, CactIExecutionTab *instance )
{
	NAObjectProfile *profile;
	gboolean editable;
	gboolean active;

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_PROFILE, &profile,
			MAIN_PROP_EDITABLE, &editable,
			NULL );

	if( profile ){
		active = gtk_toggle_button_get_active( toggle_button );

		if( editable ){
			na_object_set_startup_notify( profile, active );
			g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, 0 );

		} else {
			g_signal_handlers_block_by_func(( gpointer ) toggle_button, on_startup_notify_toggled, instance );
			gtk_toggle_button_set_active( toggle_button, !active );
			g_signal_handlers_unblock_by_func(( gpointer ) toggle_button, on_startup_notify_toggled, instance );
		}
	}
}

static void
on_startup_class_changed( GtkEntry *entry, CactIExecutionTab *instance )
{
	NAObjectProfile *profile;
	const gchar *text;

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_PROFILE, &profile,
			NULL );

	if( profile ){
		text = gtk_entry_get_text( entry );
		na_object_set_startup_class( profile, text );
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, 0 );
	}
}

static void
on_execute_as_changed( GtkEntry *entry, CactIExecutionTab *instance )
{
	NAObjectProfile *profile;
	const gchar *text;

	g_object_get(
			G_OBJECT( instance ),
			MAIN_PROP_PROFILE, &profile,
			NULL );

	if( profile ){
		text = gtk_entry_get_text( entry );
		na_object_set_execute_as( profile, text );
		g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, profile, 0 );
	}
}

static IExecutionData *
get_iexecution_data( CactIExecutionTab *instance )
{
	IExecutionData *data;

	data = ( IExecutionData * ) g_object_get_data( G_OBJECT( instance ), IEXECUTION_TAB_PROP_DATA );

	if( !data ){
		data = g_new0( IExecutionData, 1 );
		g_object_set_data( G_OBJECT( instance ), IEXECUTION_TAB_PROP_DATA, data );
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, CactIExecutionTab *instance )
{
	static const gchar *thisfn = "cact_iexecution_tab_on_instance_finalized";
	IExecutionData *data;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );

	data = get_iexecution_data( instance );

	g_free( data );
}
