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
#include <libintl.h>

#include <api/na-iimporter.h>

#include <core/na-desktop-environment.h>
#include <core/na-exporter.h>
#include <core/na-export-format.h>
#include <core/na-gtk-utils.h>
#include <core/na-import-mode.h>
#include <core/na-importer.h>
#include <core/na-ioptions-list.h>
#include <core/na-iprefs.h>
#include <core/na-tokens.h>

#include "cact-application.h"
#include "base-gtk-utils.h"
#include "cact-schemes-list.h"
#include "cact-providers-list.h"
#include "cact-preferences-editor.h"

/* private class data
 */
struct _CactPreferencesEditorClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CactPreferencesEditorPrivate {
	gboolean dispose_has_run;
	gboolean preferences_locked;

	/* first tab: runtime preferences */
	guint    order_mode;
	gboolean order_mode_mandatory;
	gboolean root_menu;
	gboolean root_menu_mandatory;
	gboolean about_item;
	gboolean about_item_mandatory;

	/* second tab: runtime execution */
	gchar   *terminal_pattern;
	gboolean terminal_pattern_mandatory;
	gchar   *desktop;
	gboolean desktop_mandatory;

	/* third tab: ui preferences */
	gboolean relabel_menu;
	gboolean relabel_menu_mandatory;
	gboolean relabel_action;
	gboolean relabel_action_mandatory;
	gboolean relabel_profile;
	gboolean relabel_profile_mandatory;
	gboolean esc_quit;
	gboolean esc_quit_mandatory;
	gboolean esc_confirm;
	gboolean esc_confirm_mandatory;
	gboolean auto_save;
	gboolean auto_save_mandatory;
	guint    auto_save_period;
	gboolean auto_save_period_mandatory;

	/* fourth tab: import mode */
	guint    import_mode;
	gboolean import_mode_mandatory;

	/* fifth tab: export format */
	gboolean export_format_mandatory;

	/* sixth tab: default list of available schemes */
	/* seventh tab: i/o providers */
};

/* column ordering in the desktop environment combobox
 */
enum {
	DESKTOP_ID_COLUMN = 0,
	DESKTOP_LABEL_COLUMN = 0,
	DESKTOP_N_COLUMN
};

/* i18n: the user is not willing to identify his current desktop environment,
 *       and prefers rely on the runtime detection */
static const NADesktopEnv st_no_desktop     = { "None", N_( "Rely on runtime detection" ) };

static const gchar       *st_xmlui_filename = PKGUIDIR "/cact-preferences.ui";
static const gchar       *st_toplevel_name  = "PreferencesDialog";
static const gchar       *st_wsp_name       = NA_IPREFS_PREFERENCES_WSP;

static GObjectClass      *st_parent_class   = NULL;
static guint              st_last_tab       = 0;

static GType      register_type( void );
static void       class_init( CactPreferencesEditorClass *klass );
static void       ioptions_list_iface_init( NAIOptionsListInterface *iface, void *user_data );
static GList     *ioptions_list_get_options( const NAIOptionsList *instance, GtkWidget *container );
static void       ioptions_list_free_options( const NAIOptionsList *instance, GtkWidget *container, GList *options );
static NAIOption *ioptions_list_get_ask_option( const NAIOptionsList *instance, GtkWidget *container );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_constructed( GObject *dialog );
static void       instance_dispose( GObject *dialog );
static void       instance_finalize( GObject *dialog );

static void       on_base_initialize_gtk( CactPreferencesEditor *editor, GtkDialog *toplevel, gpointer user_data );
static void       on_base_initialize_window( CactPreferencesEditor *editor, gpointer user_data );
static void       on_base_show_widgets( CactPreferencesEditor *editor, gpointer user_data );
static void       order_mode_setup( CactPreferencesEditor *editor );
static void       order_mode_on_alpha_asc_toggled( GtkToggleButton *togglebutton, CactPreferencesEditor *editor );
static void       order_mode_on_alpha_desc_toggled( GtkToggleButton *togglebutton, CactPreferencesEditor *editor );
static void       order_mode_on_manual_toggled( GtkToggleButton *togglebutton, CactPreferencesEditor *editor );
static void       order_mode_on_toggled( CactPreferencesEditor *editor, GtkToggleButton *togglebutton, GCallback cb, guint order_mode );
static void       root_menu_setup( CactPreferencesEditor *editor );
static void       root_menu_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       about_item_setup( CactPreferencesEditor *editor );
static void       about_item_set_sensitive( CactPreferencesEditor *editor );
static void       about_item_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       terminal_pattern_setup( CactPreferencesEditor *editor );
static void       terminal_pattern_on_changed( GtkEntry *entry, CactPreferencesEditor *editor );
static void       desktop_create_model( CactPreferencesEditor *editor );
static void       desktop_setup( CactPreferencesEditor *editor );
static void       desktop_on_changed( GtkComboBox *combo, CactPreferencesEditor *editor );
static void       relabel_menu_setup( CactPreferencesEditor *editor );
static void       relabel_menu_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       relabel_action_setup( CactPreferencesEditor *editor );
static void       relabel_action_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       relabel_profile_setup( CactPreferencesEditor *editor );
static void       relabel_profile_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       esc_quit_setup( CactPreferencesEditor *editor );
static void       esc_quit_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       esc_confirm_setup( CactPreferencesEditor *editor );
static void       esc_confirm_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       auto_save_setup( CactPreferencesEditor *editor );
static void       auto_save_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor );
static void       auto_save_period_on_change_value( GtkSpinButton *spinbutton, CactPreferencesEditor *editor );
static void       on_cancel_clicked( GtkButton *button, CactPreferencesEditor *editor );
static void       on_ok_clicked( GtkButton *button, CactPreferencesEditor *editor );
static void       on_dialog_ok( BaseDialog *dialog );

GType
cact_preferences_editor_get_type( void )
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
	static const gchar *thisfn = "cact_preferences_editor_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactPreferencesEditorClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactPreferencesEditor ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo ioptions_list_iface_info = {
		( GInterfaceInitFunc ) ioptions_list_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_TYPE_DIALOG, "CactPreferencesEditor", &info, 0 );

	g_type_add_interface_static( type, NA_TYPE_IOPTIONS_LIST, &ioptions_list_iface_info );

	return( type );
}

static void
class_init( CactPreferencesEditorClass *klass )
{
	static const gchar *thisfn = "cact_preferences_editor_class_init";
	GObjectClass *object_class;
	BaseDialogClass *dialog_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->constructed = instance_constructed;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactPreferencesEditorClassPrivate, 1 );

	dialog_class = BASE_DIALOG_CLASS( klass );
	dialog_class->ok = on_dialog_ok;
}

static void
ioptions_list_iface_init( NAIOptionsListInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_assistant_export_ioptions_list_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->get_options = ioptions_list_get_options;
	iface->free_options = ioptions_list_free_options;
	iface->get_ask_option = ioptions_list_get_ask_option;
}

/*
 * ioptions_list_get_options, ioptions_list_free_options:
 * manages import mode options or export format options depending of the
 * current container
 */
static GList *
ioptions_list_get_options( const NAIOptionsList *instance, GtkWidget *container )
{
	static const gchar *thisfn = "cact_preferences_editor_ioptions_list_get_options";
	GList *options;
	CactApplication *application;
	NAUpdater *updater;

	g_return_val_if_fail( CACT_IS_PREFERENCES_EDITOR( instance ), NULL );

	options = NULL;

	if( container == base_window_get_widget( BASE_WINDOW( instance ), "PreferencesExportFormatVBox" )){
		application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( instance )));
		updater = cact_application_get_updater( application );
		options = na_exporter_get_formats( NA_PIVOT( updater ));

	} else if( container == base_window_get_widget( BASE_WINDOW( instance ), "PreferencesImportModeVBox" )){
		options = na_importer_get_modes();

	} else {
		g_warning( "%s: container=%p (%s): unmanaged container",
				thisfn,
				( void * ) container, G_OBJECT_TYPE_NAME( container ));
	}

	return( options );
}

static void
ioptions_list_free_options( const NAIOptionsList *instance, GtkWidget *container, GList *options )
{
	static const gchar *thisfn = "cact_preferences_editor_ioptions_list_free_options";

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( instance ));

	if( container == base_window_get_widget( BASE_WINDOW( instance ), "PreferencesExportFormatVBox" )){
		na_exporter_free_formats( options );

	} else if( container == base_window_get_widget( BASE_WINDOW( instance ), "PreferencesImportModeVBox" )){
		na_importer_free_modes( options );

	} else {
		g_warning( "%s: container=%p (%s): unmanaged container",
				thisfn,
				( void * ) container, G_OBJECT_TYPE_NAME( container ));
	}
}

static NAIOption *
ioptions_list_get_ask_option( const NAIOptionsList *instance, GtkWidget *container )
{
	static const gchar *thisfn = "cact_preferences_editor_ioptions_list_get_ask_option";
	NAIOption *option;

	g_return_val_if_fail( CACT_IS_PREFERENCES_EDITOR( instance ), NULL );

	option = NULL;

	if( container == base_window_get_widget( BASE_WINDOW( instance ), "PreferencesExportFormatVBox" )){
		option = na_exporter_get_ask_option();

	} else if( container == base_window_get_widget( BASE_WINDOW( instance ), "PreferencesImportModeVBox" )){
		option = na_importer_get_ask_mode();

	} else {
		g_warning( "%s: container=%p (%s): unmanaged container",
				thisfn,
				( void * ) container, G_OBJECT_TYPE_NAME( container ));
	}

	return( option );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_preferences_editor_instance_init";
	CactPreferencesEditor *self;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( instance ));

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );

	self = CACT_PREFERENCES_EDITOR( instance );

	self->private = g_new0( CactPreferencesEditorPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_constructed( GObject *dialog )
{
	static const gchar *thisfn = "cact_preferences_editor_instance_constructed";
	CactPreferencesEditorPrivate *priv;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( dialog ));

	priv = CACT_PREFERENCES_EDITOR( dialog )->private;

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
	static const gchar *thisfn = "cact_preferences_editor_instance_dispose";
	CactPreferencesEditor *self;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( dialog ));

	self = CACT_PREFERENCES_EDITOR( dialog );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

		self->private->dispose_has_run = TRUE;

		cact_schemes_list_dispose( BASE_WINDOW( self ));
		cact_providers_list_dispose( BASE_WINDOW( self ));

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( dialog );
		}
	}
}

static void
instance_finalize( GObject *dialog )
{
	static const gchar *thisfn = "cact_preferences_editor_instance_finalize";
	CactPreferencesEditor *self;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( dialog ));

	g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

	self = CACT_PREFERENCES_EDITOR( dialog );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( dialog );
	}
}

/**
 * cact_preferences_editor_run:
 * @parent: the BaseWindow parent of this dialog
 * (usually the CactMainWindow).
 *
 * Initializes and runs the dialog.
 */
void
cact_preferences_editor_run( BaseWindow *parent )
{
	static const gchar *thisfn = "cact_preferences_editor_run";
	CactPreferencesEditor *editor;
	gboolean are_locked, mandatory;
	GtkNotebook *notebook;

	g_return_if_fail( BASE_IS_WINDOW( parent ));

	g_debug( "%s: parent=%p (%s)", thisfn, ( void * ) parent, G_OBJECT_TYPE_NAME( parent ));

	editor = g_object_new( CACT_TYPE_PREFERENCES_EDITOR,
					BASE_PROP_PARENT,          parent,
					BASE_PROP_XMLUI_FILENAME,  st_xmlui_filename,
					/*
					 * having our own builder let us, e.g., set a weak reference on
					 * pixbufs allocated by plugins - but this way we are losing
					 * mutualization of gtk initializations...
					 */
					/*BASE_PROP_HAS_OWN_BUILDER, TRUE,*/
					BASE_PROP_TOPLEVEL_NAME,   st_toplevel_name,
					BASE_PROP_WSP_NAME,        st_wsp_name,
					NULL );

	are_locked = na_settings_get_boolean( NA_IPREFS_ADMIN_PREFERENCES_LOCKED, NULL, &mandatory );
	editor->private->preferences_locked = are_locked && mandatory;
	g_debug( "%s: are_locked=%s, mandatory=%s",
			thisfn, are_locked ? "True":"False", mandatory ? "True":"False" );

	base_window_run( BASE_WINDOW( editor ));

	notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( editor ), "PreferencesNotebook" ));
	st_last_tab = gtk_notebook_get_current_page( notebook );

	g_object_unref( editor );
}

static void
on_base_initialize_gtk( CactPreferencesEditor *editor, GtkDialog *toplevel, gpointer user_data )
{
	static const gchar *thisfn = "cact_preferences_editor_on_base_initialize_gtk";
	GtkWidget *container;
	GtkTreeView *listview;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( editor ));

	if( !editor->private->dispose_has_run ){

		g_debug( "%s: dialog=%p, toplevel=%p, user_data=%p",
				thisfn, ( void * ) editor, ( void * ) toplevel, ( void * ) user_data );

		desktop_create_model( editor );

		container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesImportModeVBox" );
		na_ioptions_list_gtk_init( NA_IOPTIONS_LIST( editor ), container, TRUE );

		container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesExportFormatVBox" );
		na_ioptions_list_gtk_init( NA_IOPTIONS_LIST( editor ), container, TRUE );

		listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "SchemesTreeView" ));
		cact_schemes_list_create_model( listview, SCHEMES_LIST_FOR_PREFERENCES );

		listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ProvidersTreeView" ));
		cact_providers_list_create_model( listview );
	}
}

static void
on_base_initialize_window( CactPreferencesEditor *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_preferences_editor_on_base_initialize_window";
	GtkWidget *container;
	GtkTreeView *listview;
	GtkWidget *ok_button;
	gchar *export_format;
	gchar *import_mode;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( editor ));

	if( !editor->private->dispose_has_run ){

		g_debug( "%s: dialog=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

		/* first tab: runtime preferences
		 */
		order_mode_setup( editor );
		root_menu_setup( editor );
		about_item_setup( editor );

		/* second tab: runtime execution
		 */
		terminal_pattern_setup( editor );
		desktop_setup( editor );

		/* third tab: ui preferences
		 */
		relabel_menu_setup( editor );
		relabel_action_setup( editor );
		relabel_profile_setup( editor );
		esc_quit_setup( editor );
		esc_confirm_setup( editor );
		auto_save_setup( editor );

		/* fourth tab: import mode
		 */
		container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesImportModeVBox" );
		import_mode = na_settings_get_string( NA_IPREFS_IMPORT_PREFERRED_MODE, NULL, &editor->private->import_mode_mandatory );
		na_ioptions_list_set_editable(
				NA_IOPTIONS_LIST( editor ), container,
				!editor->private->import_mode_mandatory && !editor->private->preferences_locked );
		na_ioptions_list_set_default(
				NA_IOPTIONS_LIST( editor ), container,
				import_mode );
		g_free( import_mode );

		/* fifth tab: export format
		 */
		container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesExportFormatVBox" );
		export_format = na_settings_get_string( NA_IPREFS_EXPORT_PREFERRED_FORMAT, NULL, &editor->private->export_format_mandatory );
		na_ioptions_list_set_editable(
				NA_IOPTIONS_LIST( editor ), container,
				!editor->private->export_format_mandatory && !editor->private->preferences_locked );
		na_ioptions_list_set_default(
				NA_IOPTIONS_LIST( editor ), container,
				export_format );
		g_free( export_format );

		/* sixth tab: default schemes
		 */
		listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "SchemesTreeView" ));
		cact_schemes_list_init_view( listview, BASE_WINDOW( editor ), NULL, NULL );

		/* seventh tab: I/O providers priorities
		 */
		listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ProvidersTreeView" ));
		cact_providers_list_init_view( BASE_WINDOW( editor ), listview );

		/* dialog buttons
		 */
		base_window_signal_connect_by_name( BASE_WINDOW( editor ),
				"CancelButton", "clicked", G_CALLBACK( on_cancel_clicked ));

		ok_button = base_window_get_widget( BASE_WINDOW( editor ), "OKButton" );
		base_window_signal_connect( BASE_WINDOW( editor ),
				G_OBJECT( ok_button ), "clicked", G_CALLBACK( on_ok_clicked ));
		base_gtk_utils_set_editable( G_OBJECT( ok_button ), !editor->private->preferences_locked );
	}
}

static void
on_base_show_widgets( CactPreferencesEditor *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_preferences_editor_on_base_show_widgets";
	GtkNotebook *notebook;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( editor ));

	if( !editor->private->dispose_has_run ){

		g_debug( "%s: dialog=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

		notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( editor ), "PreferencesNotebook" ));
		gtk_notebook_set_current_page( notebook, st_last_tab );

		cact_schemes_list_show_all( BASE_WINDOW( editor ));
	}
}

/*
 * 'order mode' is editable if preferences in general are not locked,
 * and this one is not mandatory.
 *
 * This is not related in any way to the level zero order writability status
 *
 * The radio button group is insensitive if preferences are locked.
 * If preferences in general are not locked, but this one is mandatory,
 * then the radio group is sensitive, but not editable.
 */
static void
order_mode_setup( CactPreferencesEditor *editor )
{
	gboolean editable;
	GtkWidget *alpha_asc_button, *alpha_desc_button, *manual_button;
	GtkWidget *active_button;
	GCallback active_handler;

	editor->private->order_mode = na_iprefs_get_order_mode( &editor->private->order_mode_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->order_mode_mandatory;

	alpha_asc_button = base_window_get_widget( BASE_WINDOW( editor ), "OrderAlphaAscButton" );
	base_window_signal_connect( BASE_WINDOW( editor ), G_OBJECT( alpha_asc_button ), "toggled", G_CALLBACK( order_mode_on_alpha_asc_toggled ));

	alpha_desc_button = base_window_get_widget( BASE_WINDOW( editor ), "OrderAlphaDescButton" );
	base_window_signal_connect( BASE_WINDOW( editor ), G_OBJECT( alpha_desc_button ), "toggled", G_CALLBACK( order_mode_on_alpha_desc_toggled ));

	manual_button = base_window_get_widget( BASE_WINDOW( editor ), "OrderManualButton" );
	base_window_signal_connect( BASE_WINDOW( editor ), G_OBJECT( manual_button ), "toggled", G_CALLBACK( order_mode_on_manual_toggled ));

	switch( editor->private->order_mode ){
		case IPREFS_ORDER_ALPHA_ASCENDING:
			active_button = alpha_asc_button;
			active_handler = G_CALLBACK( order_mode_on_alpha_asc_toggled );
			break;
		case IPREFS_ORDER_ALPHA_DESCENDING:
			active_button = alpha_desc_button;
			active_handler = G_CALLBACK( order_mode_on_alpha_desc_toggled );
			break;
		case IPREFS_ORDER_MANUAL:
		default:
			active_button = manual_button;
			active_handler = G_CALLBACK( order_mode_on_manual_toggled );
			break;
	}

	base_gtk_utils_radio_set_initial_state(
			GTK_RADIO_BUTTON( active_button ),
			active_handler, editor, editable, !editor->private->preferences_locked );
}

static void
order_mode_on_alpha_asc_toggled( GtkToggleButton *toggle_button, CactPreferencesEditor *editor )
{
	order_mode_on_toggled( editor, toggle_button, G_CALLBACK( order_mode_on_alpha_asc_toggled ), IPREFS_ORDER_ALPHA_ASCENDING );
}

static void
order_mode_on_alpha_desc_toggled( GtkToggleButton *toggle_button, CactPreferencesEditor *editor )
{
	order_mode_on_toggled( editor, toggle_button, G_CALLBACK( order_mode_on_alpha_desc_toggled ), IPREFS_ORDER_ALPHA_DESCENDING );
}

static void
order_mode_on_manual_toggled( GtkToggleButton *toggle_button, CactPreferencesEditor *editor )
{
	order_mode_on_toggled( editor, toggle_button, G_CALLBACK( order_mode_on_manual_toggled ), IPREFS_ORDER_MANUAL );
}

static void
order_mode_on_toggled( CactPreferencesEditor *editor, GtkToggleButton *toggle_button, GCallback cb, guint order_mode )
{
	gboolean editable;
	gboolean active;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( toggle_button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		active = gtk_toggle_button_get_active( toggle_button );
		if( active ){
			editor->private->order_mode = order_mode;
		}
	} else {
		base_gtk_utils_radio_reset_initial_state( GTK_RADIO_BUTTON( toggle_button ), cb );
	}
}

/*
 * create a root menu
 */
static void
root_menu_setup( CactPreferencesEditor *editor )
{
	gboolean editable;

	editor->private->root_menu = na_settings_get_boolean( NA_IPREFS_ITEMS_CREATE_ROOT_MENU, NULL, &editor->private->root_menu_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->root_menu_mandatory;

	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"CreateRootMenuButton", G_CALLBACK( root_menu_on_toggled ),
			editor->private->root_menu, editable, !editor->private->preferences_locked );
}

static void
root_menu_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->root_menu = gtk_toggle_button_get_active( button );
		about_item_set_sensitive( editor );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

/*
 * add an about item
 *
 * The About item is only added if the Caja-Actions root menu exists
 */
static void
about_item_setup( CactPreferencesEditor *editor )
{
	gboolean editable;

	editor->private->about_item = na_settings_get_boolean( NA_IPREFS_ITEMS_ADD_ABOUT_ITEM, NULL, &editor->private->about_item_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->about_item_mandatory;

	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"AddAboutButton", G_CALLBACK( about_item_on_toggled ),
			editor->private->about_item, editable, !editor->private->preferences_locked );

	about_item_set_sensitive( editor );
}

static void
about_item_set_sensitive( CactPreferencesEditor *editor )
{
	GtkWidget *add_about;

	add_about = base_window_get_widget( BASE_WINDOW( editor ), "AddAboutButton" );
	gtk_widget_set_sensitive( add_about, editor->private->root_menu );
}

static void
about_item_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->about_item = gtk_toggle_button_get_active( button );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

/*
 * terminal command
 * the command when execution mode is 'Terminal'
 * should have a 'COMMAND' keyword inside
 */
static void
terminal_pattern_setup( CactPreferencesEditor *editor )
{
	gboolean editable;
	GtkWidget *entry;

	editor->private->terminal_pattern = na_settings_get_string( NA_IPREFS_TERMINAL_PATTERN, NULL, &editor->private->terminal_pattern_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->terminal_pattern_mandatory;

	entry = base_window_get_widget( BASE_WINDOW( editor ), "TerminalPrefixEntry" );
	gtk_entry_set_text( GTK_ENTRY( entry ), editor->private->terminal_pattern );
	gtk_widget_set_sensitive( entry, !editor->private->preferences_locked );
	base_gtk_utils_set_editable( G_OBJECT( entry ), editable );

	terminal_pattern_on_changed( GTK_ENTRY( entry ), editor );

	base_window_signal_connect( BASE_WINDOW( editor ),
			G_OBJECT( entry ), "changed", G_CALLBACK( terminal_pattern_on_changed ));
}

static void
terminal_pattern_on_changed( GtkEntry *entry, CactPreferencesEditor *editor )
{
	gboolean editable;
	gchar *example_label;
	gchar *example_markup;
	GtkWidget *example_widget;

	editable = !editor->private->preferences_locked && !editor->private->terminal_pattern_mandatory;

	if( editable ){
		g_free( editor->private->terminal_pattern );
		editor->private->terminal_pattern = g_strdup( gtk_entry_get_text( entry ));

		example_widget = base_window_get_widget( BASE_WINDOW( editor ), "TerminalPrefixExample" );
		example_label = na_tokens_command_for_terminal( editor->private->terminal_pattern, "ls -l" );

		/* i18n: command-line example: Ex.: mate-terminal -c "ls -l" */
		example_markup = g_markup_printf_escaped(
				"<i><b><span size=\"small\">%s %s</span></b></i>", _( "Ex.:" ), example_label );

		gtk_label_set_label( GTK_LABEL( example_widget ), example_markup );

		g_free( example_label );
		g_free( example_markup );
	}
}

/*
 * desktop environment
 * for OnlyShowIn and NotshowIn contexts
 */
static void
desktop_create_model( CactPreferencesEditor *editor )
{
	GtkWidget *combo;
	GtkListStore *model;
	GtkCellRenderer *text_cell;
	const NADesktopEnv *desktops;
	guint i;
	GtkTreeIter row;

	combo = base_window_get_widget( BASE_WINDOW( editor ), "DesktopComboBox" );
	model = gtk_list_store_new( DESKTOP_N_COLUMN, G_TYPE_STRING, G_TYPE_STRING );
	gtk_combo_box_set_model( GTK_COMBO_BOX( combo ), GTK_TREE_MODEL( model ));
	g_object_unref( model );

	/* id */
	text_cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), text_cell, FALSE );
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( combo ), text_cell, "text", 0, NULL );
	gtk_cell_renderer_set_visible( GTK_CELL_RENDERER( text_cell ), FALSE );

	/* label */
	text_cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), text_cell, TRUE );
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( combo ), text_cell, "text", 0, NULL );
	gtk_cell_renderer_set_visible( GTK_CELL_RENDERER( text_cell ), TRUE );

	/* insert the "rely on runtime detection item"
	 * this item must be inserted first (see desktop_setup())
	 */
	gtk_list_store_append( model, &row );
	gtk_list_store_set( model, &row,
			DESKTOP_ID_COLUMN, st_no_desktop.id,
			DESKTOP_LABEL_COLUMN, gettext( st_no_desktop.label ),
			-1 );

	/* insert list of known desktops
	 */
	desktops = na_desktop_environment_get_known_list();
	for( i = 0 ; desktops[i].id ; ++i ){
		gtk_list_store_append( model, &row );
		gtk_list_store_set( model, &row,
				DESKTOP_ID_COLUMN, desktops[i].id,
				DESKTOP_LABEL_COLUMN, gettext( desktops[i].label ),
				-1 );
	}
}

static void
desktop_setup( CactPreferencesEditor *editor )
{
	GtkWidget *combo;
	const NADesktopEnv *desktops;
	guint i;
	gint found;
	GtkWidget *widget;
	const gchar *desktop_id;
	const gchar *desktop_label;

	found = -1;
	editor->private->desktop = na_settings_get_string( NA_IPREFS_DESKTOP_ENVIRONMENT, NULL, &editor->private->desktop_mandatory );

	if( editor->private->desktop && strlen( editor->private->desktop )){
		desktops = na_desktop_environment_get_known_list();
		for( i = 0 ; desktops[i].id && found == -1 ; ++i ){
			if( !strcmp( desktops[i].id, editor->private->desktop )){
				found = 1+i;
			}
		}
	}

	/* if the user has not selected a "preferred" desktop,
	 * we suppose he relies on runtime detection
	 */
	if( found == -1 ){
		found = 0;
	}

	combo = base_window_get_widget( BASE_WINDOW( editor ), "DesktopComboBox" );
	gtk_combo_box_set_active( GTK_COMBO_BOX( combo ), found );

	base_window_signal_connect(
			BASE_WINDOW( editor ),
			G_OBJECT( combo ), "changed",
			G_CALLBACK( desktop_on_changed ));

	/* set the currently detected desktop
	 */
	widget = base_window_get_widget( BASE_WINDOW( editor ), "DesktopLabel" );
	desktop_id = na_desktop_environment_detect_running_desktop();
	desktop_label = na_desktop_environment_get_label( desktop_id );
	gtk_label_set_text( GTK_LABEL( widget ), desktop_label );
}

static void
desktop_on_changed( GtkComboBox *combo, CactPreferencesEditor *editor )
{
	gboolean editable;
	gint active;
	const NADesktopEnv *desktops;

	editable = !editor->private->preferences_locked && !editor->private->desktop_mandatory;

	if( editable ){
		g_free( editor->private->desktop );
		editor->private->desktop = NULL;

		active = gtk_combo_box_get_active( combo );
		if( active > 0 ){
			desktops = na_desktop_environment_get_known_list();
			editor->private->desktop = g_strdup( desktops[active-1].id );
		}
	}
}

/*
 * relabel copied/paster menu ?
 */
static void
relabel_menu_setup( CactPreferencesEditor *editor )
{
	gboolean editable;

	editor->private->relabel_menu = na_settings_get_boolean( NA_IPREFS_RELABEL_DUPLICATE_MENU, NULL, &editor->private->relabel_menu_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->relabel_menu_mandatory;

	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"RelabelMenuButton", G_CALLBACK( relabel_menu_on_toggled ),
			editor->private->relabel_menu, editable, !editor->private->preferences_locked );
}

static void
relabel_menu_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->relabel_menu = gtk_toggle_button_get_active( button );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

/*
 * add an about item
 */
static void
relabel_action_setup( CactPreferencesEditor *editor )
{
	gboolean editable;

	editor->private->relabel_action = na_settings_get_boolean( NA_IPREFS_RELABEL_DUPLICATE_ACTION, NULL, &editor->private->relabel_action_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->relabel_action_mandatory;

	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"RelabelActionButton", G_CALLBACK( relabel_action_on_toggled ),
			editor->private->relabel_action, editable, !editor->private->preferences_locked );
}

static void
relabel_action_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->relabel_action = gtk_toggle_button_get_active( button );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

/*
 * add an about item
 */
static void
relabel_profile_setup( CactPreferencesEditor *editor )
{
	gboolean editable;

	editor->private->relabel_profile = na_settings_get_boolean( NA_IPREFS_RELABEL_DUPLICATE_PROFILE, NULL, &editor->private->relabel_profile_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->relabel_profile_mandatory;

	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"RelabelProfileButton", G_CALLBACK( relabel_profile_on_toggled ),
			editor->private->relabel_profile, editable, !editor->private->preferences_locked );
}

static void
relabel_profile_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->relabel_profile = gtk_toggle_button_get_active( button );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

/*
 * whether Esc key quits the assistants
 */
static void
esc_quit_setup( CactPreferencesEditor *editor )
{
	gboolean editable;

	editor->private->esc_quit = na_settings_get_boolean( NA_IPREFS_ASSISTANT_ESC_QUIT, NULL, &editor->private->esc_quit_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->esc_quit_mandatory;

	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"EscCloseButton", G_CALLBACK( esc_quit_on_toggled ),
			editor->private->esc_quit, editable, !editor->private->preferences_locked );
}

static void
esc_quit_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;
	GtkWidget *confirm_button;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->esc_quit = gtk_toggle_button_get_active( button );
		confirm_button = base_window_get_widget( BASE_WINDOW( editor ), "EscConfirmButton" );
		gtk_widget_set_sensitive( confirm_button, editor->private->esc_quit );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

/*
 * whether we should ask for a user confirmation when quitting an assistant
 * on 'Esc' key
 */
static void
esc_confirm_setup( CactPreferencesEditor *editor )
{
	gboolean editable;

	editor->private->esc_confirm = na_settings_get_boolean( NA_IPREFS_ASSISTANT_ESC_CONFIRM, NULL, &editor->private->esc_confirm_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->esc_confirm_mandatory;

	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"EscConfirmButton", G_CALLBACK( esc_confirm_on_toggled ),
			editor->private->esc_confirm, editable, !editor->private->preferences_locked );
}

static void
esc_confirm_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->esc_confirm = gtk_toggle_button_get_active( button );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

/*
 * add an about item
 */
static void
auto_save_setup( CactPreferencesEditor *editor )
{
	gboolean editable;
	GtkWidget *spin_button;
	GtkAdjustment *adjustment;

	g_debug( "cact_preferences_editor_auto_save_setup" );
	editor->private->auto_save = na_settings_get_boolean( NA_IPREFS_MAIN_SAVE_AUTO, NULL, &editor->private->auto_save_mandatory );
	editable = !editor->private->preferences_locked && !editor->private->auto_save_mandatory;

	editor->private->auto_save_period = na_settings_get_uint( NA_IPREFS_MAIN_SAVE_PERIOD, NULL, &editor->private->auto_save_period_mandatory );
	spin_button = base_window_get_widget( BASE_WINDOW( editor ), "AutoSavePeriodicitySpinButton" );
	adjustment = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON( spin_button ));
	gtk_adjustment_configure( adjustment, editor->private->auto_save_period, 1, 999, 1, 10, 0 );

	editable = !editor->private->preferences_locked && !editor->private->auto_save_period_mandatory;
	gtk_editable_set_editable( GTK_EDITABLE( spin_button ), editable );
	base_window_signal_connect( BASE_WINDOW( editor ),
			G_OBJECT( spin_button ), "value-changed", G_CALLBACK( auto_save_period_on_change_value ));

	editable = !editor->private->preferences_locked && !editor->private->auto_save_mandatory;
	base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
			"AutoSaveCheckButton", G_CALLBACK( auto_save_on_toggled ),
			editor->private->auto_save, editable, !editor->private->preferences_locked );
}

static void
auto_save_on_toggled( GtkToggleButton *button, CactPreferencesEditor *editor )
{
	gboolean editable;
	GtkWidget *widget;
	gboolean sensitive;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->auto_save = gtk_toggle_button_get_active( button );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}

	sensitive = editor->private->auto_save && !editor->private->preferences_locked;

	widget = base_window_get_widget( BASE_WINDOW( editor ), "AutoSavePeriodicitySpinButton" );
	gtk_widget_set_sensitive( widget, sensitive );

	widget = base_window_get_widget( BASE_WINDOW( editor ), "AutoSaveLabel1" );
	gtk_widget_set_sensitive( widget, sensitive  );

	widget = base_window_get_widget( BASE_WINDOW( editor ), "AutoSaveLabel2" );
	gtk_widget_set_sensitive( widget, sensitive  );
}

static void
auto_save_period_on_change_value( GtkSpinButton *spinbutton, CactPreferencesEditor *editor )
{
	g_debug( "cact_preferences_editor_auto_save_period_on_change_value" );
	editor->private->auto_save_period = gtk_spin_button_get_value_as_int( spinbutton );
}

static void
on_cancel_clicked( GtkButton *button, CactPreferencesEditor *editor )
{
	GtkWindow *toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_CLOSE );
}

static void
on_ok_clicked( GtkButton *button, CactPreferencesEditor *editor )
{
	GtkWindow *toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_OK );
}

static void
on_dialog_ok( BaseDialog *dialog )
{
	CactPreferencesEditor *editor;
	GtkWidget *container;
	NAIOption *option;
	gchar *import_mode;
	gchar *export_format;

	g_return_if_fail( CACT_IS_PREFERENCES_EDITOR( dialog ));

	editor = CACT_PREFERENCES_EDITOR( dialog );

	if( !editor->private->preferences_locked ){

		/* first tab: runtime preferences
		 */
		if( !editor->private->order_mode_mandatory ){
			na_iprefs_set_order_mode( editor->private->order_mode );
		}
		if( !editor->private->root_menu_mandatory ){
			na_settings_set_boolean( NA_IPREFS_ITEMS_CREATE_ROOT_MENU, editor->private->root_menu );
		}
		if( !editor->private->about_item_mandatory ){
			na_settings_set_boolean( NA_IPREFS_ITEMS_ADD_ABOUT_ITEM, editor->private->about_item );
		}

		/* second tab: runtime execution
		 */
		if( !editor->private->terminal_pattern_mandatory ){
			na_settings_set_string( NA_IPREFS_TERMINAL_PATTERN, editor->private->terminal_pattern );
		}
		if( !editor->private->desktop_mandatory ){
			na_settings_set_string( NA_IPREFS_DESKTOP_ENVIRONMENT, editor->private->desktop );
		}

		/* third tab: ui preferences
		 */
		if( !editor->private->relabel_menu_mandatory ){
			na_settings_set_boolean( NA_IPREFS_RELABEL_DUPLICATE_MENU, editor->private->relabel_menu );
		}
		if( !editor->private->relabel_action_mandatory ){
			na_settings_set_boolean( NA_IPREFS_RELABEL_DUPLICATE_ACTION, editor->private->relabel_action );
		}
		if( !editor->private->relabel_profile_mandatory ){
			na_settings_set_boolean( NA_IPREFS_RELABEL_DUPLICATE_PROFILE, editor->private->relabel_profile );
		}
		if( !editor->private->esc_quit_mandatory ){
			na_settings_set_boolean( NA_IPREFS_ASSISTANT_ESC_QUIT, editor->private->esc_quit );
		}
		if( !editor->private->esc_confirm_mandatory ){
			na_settings_set_boolean( NA_IPREFS_ASSISTANT_ESC_CONFIRM, editor->private->esc_confirm );
		}
		if( !editor->private->auto_save_mandatory ){
			na_settings_set_boolean( NA_IPREFS_MAIN_SAVE_AUTO, editor->private->auto_save );
		}
		if( !editor->private->auto_save_period_mandatory ){
			na_settings_set_uint( NA_IPREFS_MAIN_SAVE_PERIOD, editor->private->auto_save_period );
		}

		/* fourth tab: import mode
		 */
		if( !editor->private->import_mode_mandatory ){
			container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesImportModeVBox" );
			option = na_ioptions_list_get_selected( NA_IOPTIONS_LIST( editor ), container );
			g_return_if_fail( NA_IS_IMPORT_MODE( option ));
			import_mode = na_ioption_get_id( option );
			na_settings_set_string( NA_IPREFS_IMPORT_PREFERRED_MODE, import_mode );
			g_free( import_mode );
		}

		/* fifth tab: export format
		 */
		if( !editor->private->export_format_mandatory ){
			container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesExportFormatVBox" );
			option = na_ioptions_list_get_selected( NA_IOPTIONS_LIST( editor ), container );
			g_debug( "cact_preferences_editor_on_dialog_ok: option=%p", ( void * ) option );
			g_return_if_fail( NA_IS_EXPORT_FORMAT( option ));
			export_format = na_ioption_get_id( option );
			na_settings_set_string( NA_IPREFS_EXPORT_PREFERRED_FORMAT, export_format );
			g_free( export_format );
		}

		/* sixth tab: list of default schemes
		 */
		cact_schemes_list_save_defaults( BASE_WINDOW( editor ));

		/* seventh tab: priorities of I/O providers
		 */
		cact_providers_list_save( BASE_WINDOW( editor ));
	}
}
