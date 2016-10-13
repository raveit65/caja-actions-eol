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
#include <stdlib.h>

#include <api/na-object-api.h>
#include <api/na-timeout.h>

#include <core/na-iprefs.h>
#include <core/na-pivot.h>

#include "base-isession.h"
#include "cact-iaction-tab.h"
#include "cact-icommand-tab.h"
#include "cact-ibasenames-tab.h"
#include "cact-imimetypes-tab.h"
#include "cact-ifolders-tab.h"
#include "cact-ischemes-tab.h"
#include "cact-icapabilities-tab.h"
#include "cact-ienvironment-tab.h"
#include "cact-iexecution-tab.h"
#include "cact-iproperties-tab.h"
#include "cact-main-tab.h"
#include "cact-main-statusbar.h"
#include "cact-main-window.h"
#include "cact-marshal.h"
#include "cact-menubar.h"
#include "cact-tree-view.h"
#include "cact-confirm-logout.h"
#include "cact-sort-buttons.h"

/* private class data
 */
struct _CactMainWindowClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CactMainWindowPrivate {
	gboolean         dispose_has_run;

	NAUpdater       *updater;

	/**
	 * Current action or menu.
	 *
	 * This is the action or menu which is displayed in tabs Action/Menu
	 * and Properties ; it may be different of the exact row being currently
	 * selected, e.g. when a sub-profile is edited.
	 *
	 * Can be null, and this implies that @current_profile is also null,
	 * e.g. when the list is empty or in the case of a multiple selection.
	 *
	 * 'editable' property is set on selection change;
	 * This is the actual current writability status of the item at this time.
	 */
	NAObjectItem    *current_item;
	gboolean         editable;
	guint            reason;

	/**
	 * Current profile.
	 *
	 * This is the profile which is displayed in tab Command;
	 * it may be different of the exact row being currently selected,
	 * e.g. when an action with only one profile is selected.
	 *
	 * Can be null if @current_item is a menu, or an action with more
	 * than one profile is selected, or the list is empty, or in the
	 * case of a multiple selection.
	 *
	 * In other words, it is not null if:
	 * a) a profile is selected,
	 * b) an action is selected and it has exactly one profile.
	 */
	NAObjectProfile *current_profile;

	/**
	 * Current context.
	 *
	 * This is the #NAIContext data which corresponds to @current_profile
	 * or @current_item, depending of which one is actually selected.
	 */
	NAIContext      *current_context;

	/**
	 * Some convenience objects and data.
	 */
	CactTreeView    *items_view;
	gboolean         is_tree_modified;
	CactClipboard   *clipboard;
	CactMenubar     *menubar;

	gulong           pivot_handler_id;
	NATimeout        pivot_timeout;
};

/* properties set against the main window
 * these are set on selection changes
 */
enum {
	MAIN_PROP_0 = 0,

	MAIN_PROP_ITEM_ID,
	MAIN_PROP_PROFILE_ID,
	MAIN_PROP_CONTEXT_ID,
	MAIN_PROP_EDITABLE_ID,
	MAIN_PROP_REASON_ID,

	MAIN_PROP_N_PROPERTIES
};

/* signals
 */
enum {
	MAIN_ITEM_UPDATED,
	TAB_ITEM_UPDATED,
	SELECTION_CHANGED,
	CONTEXT_MENU,
	LAST_SIGNAL
};

static const gchar     *st_xmlui_filename         = PKGUIDIR "/caja-actions-config-tool.ui";
static const gchar     *st_toplevel_name          = "MainWindow";
static const gchar     *st_wsp_name               = NA_IPREFS_MAIN_WINDOW_WSP;

static gint             st_burst_timeout          = 2500;		/* burst timeout in msec */
static BaseWindowClass *st_parent_class           = NULL;
static gint             st_signals[ LAST_SIGNAL ] = { 0 };

static GType      register_type( void );
static void       class_init( CactMainWindowClass *klass );
static void       iaction_tab_iface_init( CactIActionTabInterface *iface, void *user_data );
static void       icommand_tab_iface_init( CactICommandTabInterface *iface, void *user_data );
static void       ibasenames_tab_iface_init( CactIBasenamesTabInterface *iface, void *user_data );
static void       imimetypes_tab_iface_init( CactIMimetypesTabInterface *iface, void *user_data );
static void       ifolders_tab_iface_init( CactIFoldersTabInterface *iface, void *user_data );
static void       ischemes_tab_iface_init( CactISchemesTabInterface *iface, void *user_data );
static void       icapabilities_tab_iface_init( CactICapabilitiesTabInterface *iface, void *user_data );
static void       ienvironment_tab_iface_init( CactIEnvironmentTabInterface *iface, void *user_data );
static void       iexecution_tab_iface_init( CactIExecutionTabInterface *iface, void *user_data );
static void       iproperties_tab_iface_init( CactIPropertiesTabInterface *iface, void *user_data );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void       instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void       instance_constructed( GObject *window );
static void       instance_dispose( GObject *window );
static void       instance_finalize( GObject *window );

static void       on_base_initialize_gtk( CactMainWindow *window, GtkWindow *toplevel, gpointer user_data );
static void       on_base_initialize_window( CactMainWindow *window, gpointer user_data );
static void       on_base_show_widgets( CactMainWindow *window, gpointer user_data );

static void       on_block_items_changed_timeout( CactMainWindow *window );
static void       on_tree_view_modified_status_changed( CactMainWindow *window, gboolean is_modified, gpointer user_data );
static void       on_tree_view_selection_changed( CactMainWindow *window, GList *selected_items, gpointer user_data );
static void       on_selection_changed_cleanup_handler( BaseWindow *window, GList *selected_items );
static void       on_tab_updatable_item_updated( CactMainWindow *window, NAIContext *context, guint data, gpointer user_data );
static void       raz_selection_properties( CactMainWindow *window );
static void       setup_current_selection( CactMainWindow *window, NAObjectId *selected_row );
static void       setup_dialog_title( CactMainWindow *window );
static void       setup_writability_status( CactMainWindow *window );

/* items have changed */
static void       on_pivot_items_changed( NAUpdater *updater, CactMainWindow *window );
static gboolean   confirm_for_giveup_from_pivot( const CactMainWindow *window );
static gboolean   confirm_for_giveup_from_menu( const CactMainWindow *window );
static void       load_or_reload_items( CactMainWindow *window );

/* application termination */
static gboolean   on_base_quit_requested( CactApplication *application, CactMainWindow *window );
static gboolean   on_delete_event( GtkWidget *toplevel, GdkEvent *event, CactMainWindow *window );
static gboolean   warn_modified( CactMainWindow *window );

GType
cact_main_window_get_type( void )
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
	static const gchar *thisfn = "cact_main_window_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactMainWindowClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactMainWindow ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo iaction_tab_iface_info = {
		( GInterfaceInitFunc ) iaction_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo icommand_tab_iface_info = {
		( GInterfaceInitFunc ) icommand_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ibasenames_tab_iface_info = {
		( GInterfaceInitFunc ) ibasenames_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo imimetypes_tab_iface_info = {
		( GInterfaceInitFunc ) imimetypes_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ifolders_tab_iface_info = {
		( GInterfaceInitFunc ) ifolders_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ischemes_tab_iface_info = {
		( GInterfaceInitFunc ) ischemes_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo icapabilities_tab_iface_info = {
		( GInterfaceInitFunc ) icapabilities_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ienvironment_tab_iface_info = {
		( GInterfaceInitFunc ) ienvironment_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo iexecution_tab_iface_info = {
		( GInterfaceInitFunc ) iexecution_tab_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo iproperties_tab_iface_info = {
		( GInterfaceInitFunc ) iproperties_tab_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_TYPE_WINDOW, "CactMainWindow", &info, 0 );

	g_type_add_interface_static( type, CACT_TYPE_IACTION_TAB, &iaction_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_ICOMMAND_TAB, &icommand_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_IBASENAMES_TAB, &ibasenames_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_IMIMETYPES_TAB, &imimetypes_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_IFOLDERS_TAB, &ifolders_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_ISCHEMES_TAB, &ischemes_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_ICAPABILITIES_TAB, &icapabilities_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_IENVIRONMENT_TAB, &ienvironment_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_IEXECUTION_TAB, &iexecution_tab_iface_info );

	g_type_add_interface_static( type, CACT_TYPE_IPROPERTIES_TAB, &iproperties_tab_iface_info );

	return( type );
}

static void
class_init( CactMainWindowClass *klass )
{
	static const gchar *thisfn = "cact_main_window_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->set_property = instance_set_property;
	object_class->get_property = instance_get_property;
	object_class->constructed = instance_constructed;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	g_object_class_install_property( object_class, MAIN_PROP_ITEM_ID,
			g_param_spec_pointer(
					MAIN_PROP_ITEM,
					_( "Current NAObjectItem" ),
					_( "A pointer to the currently edited NAObjectItem, an action or a menu" ),
					G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, MAIN_PROP_PROFILE_ID,
			g_param_spec_pointer(
					MAIN_PROP_PROFILE,
					_( "Current NAObjectProfile" ),
					_( "A pointer to the currently edited NAObjectProfile" ),
					G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, MAIN_PROP_CONTEXT_ID,
			g_param_spec_pointer(
					MAIN_PROP_CONTEXT,
					_( "Current NAIContext" ),
					_( "A pointer to the currently edited NAIContext" ),
					G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, MAIN_PROP_EDITABLE_ID,
			g_param_spec_boolean(
					MAIN_PROP_EDITABLE,
					_( "Editable item ?" ),
					_( "Whether the item will be able to be updated against its I/O provider" ),
					FALSE,
					G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	g_object_class_install_property( object_class, MAIN_PROP_REASON_ID,
			g_param_spec_int(
					MAIN_PROP_REASON,
					_( "No edition reason" ),
					_( "Why is this item not editable" ),
					0, 255, 0,
					G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE ));

	klass->private = g_new0( CactMainWindowClassPrivate, 1 );

	/**
	 * CactMainWindow::main-item-updated:
	 *
	 * This signal is emitted on the BaseWindow when the item has been modified
	 * elsewhere that in a tab. The tabs should so update accordingly their
	 * widgets.
	 *
	 * Args:
	 * - an OR-ed list of the modified data, or 0 if not relevant.
	 */
	st_signals[ MAIN_ITEM_UPDATED ] = g_signal_new(
			MAIN_SIGNAL_ITEM_UPDATED,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,					/* no default handler */
			NULL,
			NULL,
			cact_cclosure_marshal_VOID__POINTER_UINT,
			G_TYPE_NONE,
			2,
			G_TYPE_POINTER,
			G_TYPE_UINT );

	/**
	 * cact-tab-updatable-item-updated:
	 *
	 * This signal is emitted by the notebook tabs, when any property
	 * of an item has been modified.
	 *
	 * Args:
	 * - an OR-ed list of the modified data, or 0 if not relevant.
	 *
	 * This main window is rather the only consumer of this message,
	 * does its tricks (title, etc.), and then reforward an item-updated
	 * message to IActionsList.
	 */
	st_signals[ TAB_ITEM_UPDATED ] = g_signal_new(
			TAB_UPDATABLE_SIGNAL_ITEM_UPDATED,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,					/* no default handler */
			NULL,
			NULL,
			cact_cclosure_marshal_VOID__POINTER_UINT,
			G_TYPE_NONE,
			2,
			G_TYPE_POINTER,
			G_TYPE_UINT );

	/**
	 * CactMainWindow::main-selection-changed:
	 *
	 * This signal is emitted on the window parent each time the selection
	 * has changed in the treeview, after having set the current item/profile/
	 * context properties.
	 *
	 * This way, we are sure that notebook edition tabs which required to
	 * have a current item/profile/context will have it, whenever they have
	 * connected to the 'selection-changed' signal.
	 *
	 * Signal args:
	 * - a #GList of currently selected #NAObjectItems.
	 *
	 * Handler prototype:
	 * void ( *handler )( BaseWindow *window, GList *selected, gpointer user_data );
	 */
	st_signals[ SELECTION_CHANGED ] = g_signal_new_class_handler(
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_CLEANUP,
			G_CALLBACK( on_selection_changed_cleanup_handler ),
			NULL,
			NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER );

	/**
	 * CactMainWindow::main-signal-open-popup
	 *
	 * This signal is emitted on the BaseWindow parent when the user right
	 * clicks somewhere (on an active zone).
	 *
	 * Signal args:
	 * - the GdkEvent
	 * - the popup name to be opened.
	 *
	 * Handler prototype:
	 * void ( *handler )( BaseWindow *window, GdkEvent *event, const gchar *popup_name, gpointer user_data );
	 */
	st_signals[ CONTEXT_MENU ] = g_signal_new(
			MAIN_SIGNAL_CONTEXT_MENU,
			G_TYPE_OBJECT,
			G_SIGNAL_RUN_LAST,
			0,
			NULL,
			NULL,
			cact_cclosure_marshal_VOID__POINTER_STRING,
			G_TYPE_NONE,
			2,
			G_TYPE_POINTER,
			G_TYPE_STRING);
}

static void
iaction_tab_iface_init( CactIActionTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_iaction_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
icommand_tab_iface_init( CactICommandTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_icommand_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
ibasenames_tab_iface_init( CactIBasenamesTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_ibasenames_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
imimetypes_tab_iface_init( CactIMimetypesTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_imimetypes_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
ifolders_tab_iface_init( CactIFoldersTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_ifolders_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
ischemes_tab_iface_init( CactISchemesTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_ischemes_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
icapabilities_tab_iface_init( CactICapabilitiesTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_icapabilities_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
ienvironment_tab_iface_init( CactIEnvironmentTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_ienvironment_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
iexecution_tab_iface_init( CactIExecutionTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_iexecution_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
iproperties_tab_iface_init( CactIPropertiesTabInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_main_window_iproperties_tab_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_main_window_instance_init";
	CactMainWindow *self;
	CactMainWindowPrivate *priv;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_MAIN_WINDOW( instance );
	self->private = g_new0( CactMainWindowPrivate, 1 );
	priv = self->private;
	priv->dispose_has_run = FALSE;

	/* initialize timeout parameters when blocking 'pivot-items-changed' handler
	 */
	priv->pivot_timeout.timeout = st_burst_timeout;
	priv->pivot_timeout.handler = ( NATimeoutFunc ) on_block_items_changed_timeout;
	priv->pivot_timeout.user_data = self;
	priv->pivot_timeout.source_id = 0;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	CactMainWindow *self;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( object ));
	self = CACT_MAIN_WINDOW( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case MAIN_PROP_ITEM_ID:
				g_value_set_pointer( value, self->private->current_item );
				break;

			case MAIN_PROP_PROFILE_ID:
				g_value_set_pointer( value, self->private->current_profile );
				break;

			case MAIN_PROP_CONTEXT_ID:
				g_value_set_pointer( value, self->private->current_context );
				break;

			case MAIN_PROP_EDITABLE_ID:
				g_value_set_boolean( value, self->private->editable );
				break;

			case MAIN_PROP_REASON_ID:
				g_value_set_int( value, self->private->reason );
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
	CactMainWindow *self;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( object ));
	self = CACT_MAIN_WINDOW( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case MAIN_PROP_ITEM_ID:
				self->private->current_item = g_value_get_pointer( value );
				break;

			case MAIN_PROP_PROFILE_ID:
				self->private->current_profile = g_value_get_pointer( value );
				break;

			case MAIN_PROP_CONTEXT_ID:
				self->private->current_context = g_value_get_pointer( value );
				break;

			case MAIN_PROP_EDITABLE_ID:
				self->private->editable = g_value_get_boolean( value );
				break;

			case MAIN_PROP_REASON_ID:
				self->private->reason = g_value_get_int( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_constructed( GObject *window )
{
	static const gchar *thisfn = "cact_main_window_instance_constructed";
	CactMainWindowPrivate *priv;
	CactApplication *application;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	priv = CACT_MAIN_WINDOW( window )->private;

	if( !priv->dispose_has_run ){

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( window );
		}

		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		/* connect to BaseWindow signals
		 * so that convenience objects instanciated later will have this same signal
		 * triggered after the one of CactMainWindow
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				BASE_SIGNAL_INITIALIZE_GTK,
				G_CALLBACK( on_base_initialize_gtk ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				BASE_SIGNAL_INITIALIZE_WINDOW,
				G_CALLBACK( on_base_initialize_window ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				BASE_SIGNAL_SHOW_WIDGETS,
				G_CALLBACK( on_base_show_widgets ));

		/* monitor the items stored on the disk for modifications
		 * outside of this application
		 */
		application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
		priv->updater = cact_application_get_updater( application );

		priv->pivot_handler_id = base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( priv->updater ),
				PIVOT_SIGNAL_ITEMS_CHANGED,
				G_CALLBACK( on_pivot_items_changed ));

		/* monitor the updates which originates from each property tab
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				TAB_UPDATABLE_SIGNAL_ITEM_UPDATED,
				G_CALLBACK( on_tab_updatable_item_updated ));

		/* create the menubar and other convenience objects
		 */
		priv->menubar = cact_menubar_new( BASE_WINDOW( window ));
		priv->clipboard = cact_clipboard_new( BASE_WINDOW( window ));

		/* initialize the notebook interfaces
		 */
		cact_iaction_tab_init( CACT_IACTION_TAB( window ));
		cact_icommand_tab_init( CACT_ICOMMAND_TAB( window ));
		cact_ibasenames_tab_init( CACT_IBASENAMES_TAB( window ));
		cact_imimetypes_tab_init( CACT_IMIMETYPES_TAB( window ));
		cact_ifolders_tab_init( CACT_IFOLDERS_TAB( window ));
		cact_ischemes_tab_init( CACT_ISCHEMES_TAB( window ));
		cact_icapabilities_tab_init( CACT_ICAPABILITIES_TAB( window ));
		cact_ienvironment_tab_init( CACT_IENVIRONMENT_TAB( window ));
		cact_iexecution_tab_init( CACT_IEXECUTION_TAB( window ));
		cact_iproperties_tab_init( CACT_IPROPERTIES_TAB( window ));
	}
}

static void
instance_dispose( GObject *window )
{
	static const gchar *thisfn = "cact_main_window_instance_dispose";
	CactMainWindow *self;
	GtkWidget *pane;
	gint pos;
	GtkNotebook *notebook;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	self = CACT_MAIN_WINDOW( window );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		self->private->dispose_has_run = TRUE;

		gtk_main_quit();

		g_object_unref( self->private->clipboard );
		g_object_unref( self->private->menubar );

		pane = base_window_get_widget( BASE_WINDOW( window ), "MainPaned" );
		pos = gtk_paned_get_position( GTK_PANED( pane ));
		na_settings_set_uint( NA_IPREFS_MAIN_PANED, pos );

		notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
		pos = gtk_notebook_get_tab_pos( notebook );
		na_iprefs_set_tabs_pos( pos );

		/* unref items view at last as gtk_tree_model_store_clear() will
		 * finalize all objects, thus invaliditing all our references
		 */
		g_object_unref( self->private->items_view );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( window );
		}
	}
}

static void
instance_finalize( GObject *window )
{
	static const gchar *thisfn = "cact_main_window_instance_finalize";
	CactMainWindow *self;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

	self = CACT_MAIN_WINDOW( window );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( window );
	}
}

/**
 * Returns a newly allocated CactMainWindow object.
 */
CactMainWindow *
cact_main_window_new( const CactApplication *application )
{
	CactMainWindow *window;

	g_return_val_if_fail( CACT_IS_APPLICATION( application ), NULL );

	window = g_object_new( CACT_TYPE_MAIN_WINDOW,
			BASE_PROP_APPLICATION,        application,
			BASE_PROP_XMLUI_FILENAME,     st_xmlui_filename,
			BASE_PROP_TOPLEVEL_NAME,      st_toplevel_name,
			BASE_PROP_WSP_NAME,           st_wsp_name,
			BASE_PROP_DESTROY_ON_DISPOSE, TRUE,
			NULL );

	if( !base_window_init( BASE_WINDOW( window ))){
		g_object_unref( window );
		window = NULL;
	}

	return( window );
}

/*
 * note that for this CactMainWindow, on_base_initialize_gtk_toplevel() and
 * on_base_initialize_base_window() are roughly equivalent, as there is only
 * one occurrence on this window in the application: closing this window
 * is the same than quitting the application
 */
static void
on_base_initialize_gtk( CactMainWindow *window, GtkWindow *toplevel, gpointer user_data )
{
	static const gchar *thisfn = "cact_main_window_on_base_initialize_gtk";
	GtkWidget *tree_parent;
	GtkNotebook *notebook;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p, toplevel=%p, user_data=%p",
				thisfn,
				( void * ) window,
				( void * ) toplevel,
				( void * ) user_data );

		/* create the tree view which will create itself its own tree model
		 */
		tree_parent = base_window_get_widget( BASE_WINDOW( window ), "MainVBox" );
		g_debug( "%s: tree_parent=%p (%s)", thisfn, ( void * ) tree_parent, G_OBJECT_TYPE_NAME( tree_parent ));
		window->private->items_view = cact_tree_view_new(
				BASE_WINDOW( window ),
				GTK_CONTAINER( tree_parent ),
				"ActionsList",
				TREE_MODE_EDITION );

		cact_main_statusbar_initialize_gtk_toplevel( window );

		/* enable popup menu on  the notebook
		 */
		notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
		gtk_notebook_popup_enable( notebook );
	}
}

static void
on_base_initialize_window( CactMainWindow *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_main_window_on_base_initialize_window";
	guint pos;
	GtkWidget *pane;
	CactApplication *application;
	GtkNotebook *notebook;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p, user_data=%p",
				thisfn,
				( void * ) window,
				( void * ) user_data );

		pos = na_settings_get_uint( NA_IPREFS_MAIN_PANED, NULL, NULL );
		if( pos ){
			pane = base_window_get_widget( BASE_WINDOW( window ), "MainPaned" );
			gtk_paned_set_position( GTK_PANED( pane ), pos );
		}

		/* terminate the application by clicking the top right [X] button
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( base_window_get_gtk_toplevel( BASE_WINDOW( window ))),
				"delete-event",
				G_CALLBACK( on_delete_event ));

		/* is willing to quit ?
		 * connect to the signal of the BaseISession interface
		 */
		application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( application ),
				BASE_SIGNAL_QUIT_REQUESTED,
				G_CALLBACK( on_base_quit_requested ));

		/* connect to treeview signals
		 */
		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				MAIN_SIGNAL_SELECTION_CHANGED,
				G_CALLBACK( on_tree_view_selection_changed ));

		base_window_signal_connect(
				BASE_WINDOW( window ),
				G_OBJECT( window ),
				TREE_SIGNAL_MODIFIED_STATUS_CHANGED,
				G_CALLBACK( on_tree_view_modified_status_changed ));

		/* restore the notebook tabs position
		 */
		pos = na_iprefs_get_tabs_pos( NULL );
		notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( window ), "MainNotebook" ));
		gtk_notebook_set_tab_pos( notebook, pos );
	}
}

static void
on_base_show_widgets( CactMainWindow *window, gpointer user_data )
{
	static const gchar *thisfn = "cact_main_window_on_base_show_widgets";

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		g_debug( "%s: window=%p, user_data=%p", thisfn, ( void * ) window, ( void * ) user_data );

		load_or_reload_items( window );
	}
}

/**
 * cact_main_window_get_clipboard:
 * @window: this #CactMainWindow instance.
 *
 * Returns: the #CactClipboard convenience object.
 */
CactClipboard *
cact_main_window_get_clipboard( const CactMainWindow *window )
{
	CactClipboard *clipboard;

	g_return_val_if_fail( CACT_IS_MAIN_WINDOW( window ), NULL );

	clipboard = NULL;

	if( !window->private->dispose_has_run ){

		clipboard = window->private->clipboard;
	}

	return( clipboard );
}

/**
 * cact_main_window_get_items_view:
 * @window: this #CactMainWindow instance.
 *
 * Returns: The #CactTreeView convenience object.
 */
CactTreeView *
cact_main_window_get_items_view( const CactMainWindow *window )
{
	CactTreeView *view;

	g_return_val_if_fail( CACT_IS_MAIN_WINDOW( window ), NULL );

	view = NULL;

	if( !window->private->dispose_has_run ){

		view = window->private->items_view;
	}

	return( view );
}

/**
 * cact_main_window_reload:
 * @window: this #CactMainWindow instance.
 *
 * Refresh the list of items.
 * If there is some non-yet saved modifications, a confirmation is
 * required before giving up with them.
 */
void
cact_main_window_reload( CactMainWindow *window )
{
	gboolean reload_ok;

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		reload_ok = confirm_for_giveup_from_menu( window );

		if( reload_ok ){
			load_or_reload_items( window );
		}
	}
}

/**
 * cact_main_window_block_reload:
 * @window: this #CactMainWindow instance.
 *
 * Temporarily blocks the handling of pivot-items-changed signal.
 */
void
cact_main_window_block_reload( CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_block_reload";

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	if( !window->private->dispose_has_run ){

		g_debug( "%s: blocking %s signal", thisfn, PIVOT_SIGNAL_ITEMS_CHANGED );
		g_signal_handler_block( window->private->updater, window->private->pivot_handler_id );
		na_timeout_event( &window->private->pivot_timeout );
	}
}

static void
on_block_items_changed_timeout( CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_on_block_items_changed_timeout";

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	g_debug( "%s: unblocking %s signal", thisfn, PIVOT_SIGNAL_ITEMS_CHANGED );
	g_signal_handler_unblock( window->private->updater, window->private->pivot_handler_id );
}

/*
 * the modification status of the items view has changed
 */
static void
on_tree_view_modified_status_changed( CactMainWindow *window, gboolean is_modified, gpointer user_data )
{
	static const gchar *thisfn = "cact_main_window_on_tree_view_modified_status_changed";

	g_debug( "%s: window=%p, is_modified=%s, user_data=%p",
			thisfn, ( void * ) window, is_modified ? "True":"False", ( void * ) user_data );

	if( !window->private->dispose_has_run ){

		window->private->is_tree_modified = is_modified;
		setup_dialog_title( window );
	}
}

/*
 * tree view selection has changed
 */
static void
on_tree_view_selection_changed( CactMainWindow *window, GList *selected_items, gpointer user_data )
{
	static const gchar *thisfn = "cact_main_window_on_tree_view_selection_changed";
	guint count;

	count = g_list_length( selected_items );

	if( !window->private->dispose_has_run ){
		g_debug( "%s: window=%p, selected_items=%p (count=%d), user_data=%p",
				thisfn, ( void * ) window,
				( void * ) selected_items, count, ( void * ) user_data );

		raz_selection_properties( window );

		if( count == 1 ){
			g_return_if_fail( NA_IS_OBJECT_ID( selected_items->data ));
			setup_current_selection( window, NA_OBJECT_ID( selected_items->data ));
			setup_writability_status( window );
		}

		setup_dialog_title( window );
	}
}

/*
 * cleanup handler for our MAIN_SIGNAL_SELECTION_CHANGED signal
 */
static void
on_selection_changed_cleanup_handler( BaseWindow *window, GList *selected_items )
{
	static const gchar *thisfn = "cact_main_window_on_selection_changed_cleanup_handler";

	g_debug( "%s: window=%p, selected_items=%p (count=%u)",
			thisfn, ( void * ) window,
			( void * ) selected_items, g_list_length( selected_items ));

	na_object_free_items( selected_items );
}

static void
on_tab_updatable_item_updated( CactMainWindow *window, NAIContext *context, guint data, gpointer user_data )
{
	static const gchar *thisfn = "cact_main_window_on_tab_updatable_item_updated";

	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	if( !window->private->dispose_has_run ){
		g_debug( "%s: window=%p, context=%p (%s), data=%u, user_data=%p",
				thisfn, ( void * ) window, ( void * ) context, G_OBJECT_TYPE_NAME( context ),
				data, ( void * ) user_data );

		if( context ){
			na_object_check_status( context );
		}
	}
}

static void
raz_selection_properties( CactMainWindow *window )
{
	window->private->current_item = NULL;
	window->private->current_profile = NULL;
	window->private->current_context = NULL;
	window->private->editable = FALSE;
	window->private->reason = 0;

	cact_main_statusbar_set_locked( window, FALSE, 0 );
}

/*
 * enter after raz_properties
 * only called when only one selected row
 */
static void
setup_current_selection( CactMainWindow *window, NAObjectId *selected_row )
{
	guint nb_profiles;
	GList *profiles;

	if( NA_IS_OBJECT_PROFILE( selected_row )){
		window->private->current_profile = NA_OBJECT_PROFILE( selected_row );
		window->private->current_context = NA_ICONTEXT( selected_row );
		window->private->current_item = NA_OBJECT_ITEM( na_object_get_parent( selected_row ));

	} else {
		g_return_if_fail( NA_IS_OBJECT_ITEM( selected_row ));
		window->private->current_item = NA_OBJECT_ITEM( selected_row );
		window->private->current_context = NA_ICONTEXT( selected_row );

		if( NA_IS_OBJECT_ACTION( selected_row )){
			nb_profiles = na_object_get_items_count( selected_row );

			if( nb_profiles == 1 ){
				profiles = na_object_get_items( selected_row );
				window->private->current_profile = NA_OBJECT_PROFILE( profiles->data );
				window->private->current_context = NA_ICONTEXT( profiles->data );
			}
		}
	}
}

/*
 * the title bar of the main window brings up three information:
 * - the name of the application
 * - the name of the currently selected item if there is only one
 * - an asterisk if anything has been modified
 */
static void
setup_dialog_title( CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_setup_dialog_title";
	GtkWindow *toplevel;
	CactApplication *application;
	gchar *title;
	gchar *label;
	gchar *tmp;
	gboolean is_modified;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	title = base_application_get_application_name( BASE_APPLICATION( application ));

	if( window->private->current_item ){
		label = na_object_get_label( window->private->current_item );
		is_modified = na_object_is_modified( window->private->current_item );
		tmp = g_strdup_printf( "%s%s - %s", is_modified ? "*" : "", label, title );
		g_free( label );
		g_free( title );
		title = tmp;
	}

	toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( window ));
	gtk_window_set_title( toplevel, title );
	g_free( title );
}

static void
setup_writability_status( CactMainWindow *window )
{
	g_return_if_fail( NA_IS_OBJECT_ITEM( window->private->current_item ));

	window->private->editable = na_object_is_finally_writable( window->private->current_item, &window->private->reason );
	cact_main_statusbar_set_locked( window, !window->private->editable, window->private->reason );
}

/*
 * The handler of the signal sent by NAPivot when items have been modified
 * in the underlying storage subsystems
 */
static void
on_pivot_items_changed( NAUpdater *updater, CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_on_pivot_items_changed";
	gboolean reload_ok;

	g_return_if_fail( NA_IS_UPDATER( updater ));
	g_return_if_fail( CACT_IS_MAIN_WINDOW( window ));

	if( !window->private->dispose_has_run ){
		g_debug( "%s: updater=%p (%s), window=%p (%s)", thisfn,
				( void * ) updater, G_OBJECT_TYPE_NAME( updater ),
				( void * ) window, G_OBJECT_TYPE_NAME( window ));

		reload_ok = confirm_for_giveup_from_pivot( window );

		if( reload_ok ){
			load_or_reload_items( window );
		}
	}
}

/*
 * informs the user that the actions in underlying storage subsystem
 * have changed, and propose for reloading
 *
 */
static gboolean
confirm_for_giveup_from_pivot( const CactMainWindow *window )
{
	gboolean reload_ok;
	gchar *first, *second;

	first = g_strdup(
				_( "One or more actions have been modified in the filesystem.\n"
					"You could keep to work with your current list of actions, "
					"or you may want to reload a fresh one." ));

	if( window->private->is_tree_modified){

		gchar *tmp = g_strdup_printf( "%s\n\n%s", first,
				_( "Note that reloading a fresh list of actions requires "
					"that you give up with your current modifications." ));
		g_free( first );
		first = tmp;
	}

	second = g_strdup( _( "Do you want to reload a fresh list of actions ?" ));

	reload_ok = base_window_display_yesno_dlg( BASE_WINDOW( window ), first, second );

	g_free( second );
	g_free( first );

	return( reload_ok );
}

/*
 * requires a confirmation from the user when is has asked for reloading
 * the actions via the Edit menu
 */
static gboolean
confirm_for_giveup_from_menu( const CactMainWindow *window )
{
	gboolean reload_ok = TRUE;
	gchar *first, *second;

	if( window->private->is_tree_modified ){

		first = g_strdup(
					_( "Reloading a fresh list of actions requires "
						"that you give up with your current modifications." ));

		second = g_strdup( _( "Do you really want to do this ?" ));

		reload_ok = base_window_display_yesno_dlg( BASE_WINDOW( window ), first, second );

		g_free( second );
		g_free( first );
	}

	return( reload_ok );
}

static void
load_or_reload_items( CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_load_or_reload_items";
	GList *tree;

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	raz_selection_properties( window );

	tree = na_updater_load_items( window->private->updater );
	cact_tree_view_fill( window->private->items_view, tree );

	g_debug( "%s: end of tree view filling", thisfn );
}

/**
 * cact_main_window_quit:
 * @window: the #CactMainWindow main window.
 *
 * Quit the window, thus terminating the application.
 *
 * Returns: %TRUE if the application will terminate, and the @window object
 * is no more valid; %FALSE if the application will continue to run.
 */
gboolean
cact_main_window_quit( CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_quit";
	gboolean terminated;

	g_return_val_if_fail( CACT_IS_MAIN_WINDOW( window ), FALSE );

	terminated = FALSE;

	if( !window->private->dispose_has_run ){
		g_debug( "%s: window=%p (%s)", thisfn, ( void * ) window, G_OBJECT_TYPE_NAME( window ));

		if( !window->private->is_tree_modified  || warn_modified( window )){
			g_object_unref( window );
			terminated = TRUE;
		}
	}

	return( terminated );
}

/*
 * signal handler
 * should return %FALSE if it is not willing to quit
 * this will also stop the emission of the signal (i.e. the first FALSE wins)
 */
static gboolean
on_base_quit_requested( CactApplication *application, CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_on_base_quit_requested";
	gboolean willing_to;

	g_return_val_if_fail( CACT_IS_MAIN_WINDOW( window ), TRUE );

	willing_to = TRUE;

	if( !window->private->dispose_has_run ){

		g_debug( "%s: application=%p, window=%p", thisfn, ( void * ) application, ( void * ) window );

		if( window->private->is_tree_modified ){
			willing_to = cact_confirm_logout_run( window );
		}
	}

	return( willing_to );
}

/*
 * triggered when the user clicks on the top right [X] button
 * returns %TRUE to stop the signal to be propagated (which would cause the
 * window to be destroyed); instead we gracefully quit the application
 */
static gboolean
on_delete_event( GtkWidget *toplevel, GdkEvent *event, CactMainWindow *window )
{
	static const gchar *thisfn = "cact_main_window_on_delete_event";

	g_debug( "%s: toplevel=%p, event=%p, window=%p",
			thisfn, ( void * ) toplevel, ( void * ) event, ( void * ) window );

	cact_main_window_quit( window );

	return( TRUE );
}

/*
 * warn_modified:
 * @window: this #CactWindow instance.
 *
 * Emits a warning if at least one item has been modified.
 *
 * Returns: %TRUE if the user confirms he wants to quit.
 */
static gboolean
warn_modified( CactMainWindow *window )
{
	gboolean confirm = FALSE;
	gchar *first;
	gchar *second;

	first = g_strdup_printf( _( "Some items have been modified." ));
	second = g_strdup( _( "Are you sure you want to quit without saving them ?" ));

	confirm = base_window_display_yesno_dlg( BASE_WINDOW( window ), first, second );

	g_free( second );
	g_free( first );

	return( confirm );
}
