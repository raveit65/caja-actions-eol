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

#include <api/na-object-api.h>

#include <core/na-exporter.h>
#include <core/na-export-format.h>
#include <core/na-gtk-utils.h>
#include <core/na-ioptions-list.h>

#include "cact-application.h"
#include "cact-export-ask.h"
#include "base-gtk-utils.h"

/* private class data
 */
struct _CactExportAskClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _CactExportAskPrivate {
	gboolean      dispose_has_run;
	BaseWindow   *parent;
	gboolean      preferences_locked;
	NAObjectItem *item;
	gchar        *format;
	gboolean      format_mandatory;
	gboolean      keep_last_choice;
	gboolean      keep_last_choice_mandatory;
};

static const gchar     *st_xmlui_filename = PKGUIDIR "/cact-assistant-export.ui";
static const gchar     *st_toplevel_name  = "ExportAskDialog";
static const gchar     *st_wsp_name       = NA_IPREFS_EXPORT_ASK_USER_WSP;

static BaseDialogClass *st_parent_class   = NULL;

static GType    register_type( void );
static void     class_init( CactExportAskClass *klass );
static void     ioptions_list_iface_init( NAIOptionsListInterface *iface, void *user_data );
static GList   *ioptions_list_get_formats( const NAIOptionsList *instance, GtkWidget *container );
static void     ioptions_list_free_formats( const NAIOptionsList *instance, GtkWidget *container, GList *formats );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_constructed( GObject *dialog );
static void     instance_dispose( GObject *dialog );
static void     instance_finalize( GObject *dialog );

static void     on_base_initialize_gtk( CactExportAsk *editor, GtkDialog *toplevel, gpointer user_data );
static void     on_base_initialize_window( CactExportAsk *editor, gpointer user_data );
static void     keep_choice_on_toggled( GtkToggleButton *button, CactExportAsk *editor );
static void     on_cancel_clicked( GtkButton *button, CactExportAsk *editor );
static void     on_ok_clicked( GtkButton *button, CactExportAsk *editor );
static gchar   *get_export_format( CactExportAsk *editor );

GType
cact_export_ask_get_type( void )
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
	static const gchar *thisfn = "cact_export_ask_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactExportAskClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactExportAsk ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo ioptions_list_iface_info = {
		( GInterfaceInitFunc ) ioptions_list_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_TYPE_DIALOG, "CactExportAsk", &info, 0 );

	g_type_add_interface_static( type, NA_TYPE_IOPTIONS_LIST, &ioptions_list_iface_info );

	return( type );
}

static void
class_init( CactExportAskClass *klass )
{
	static const gchar *thisfn = "cact_export_ask_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->constructed = instance_constructed;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactExportAskClassPrivate, 1 );
}

static void
ioptions_list_iface_init( NAIOptionsListInterface *iface, void *user_data )
{
	static const gchar *thisfn = "cact_assistant_export_ioptions_list_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->get_options = ioptions_list_get_formats;
	iface->free_options = ioptions_list_free_formats;
}

static GList *
ioptions_list_get_formats( const NAIOptionsList *instance, GtkWidget *container )
{
	CactExportAsk *window;
	CactApplication *application;
	NAUpdater *updater;
	GList *formats;

	g_return_val_if_fail( CACT_IS_EXPORT_ASK( instance ), NULL );
	window = CACT_EXPORT_ASK( instance );

	application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( window )));
	updater = cact_application_get_updater( application );
	formats = na_exporter_get_formats( NA_PIVOT( updater ));

	return( formats );
}

static void
ioptions_list_free_formats( const NAIOptionsList *instance, GtkWidget *container, GList *formats )
{
	na_exporter_free_formats( formats );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_export_ask_instance_init";
	CactExportAsk *self;

	g_return_if_fail( CACT_IS_EXPORT_ASK( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = CACT_EXPORT_ASK( instance );

	self->private = g_new0( CactExportAskPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_constructed( GObject *dialog )
{
	static const gchar *thisfn = "cact_export_ask_instance_constructed";
	CactExportAskPrivate *priv;

	g_return_if_fail( CACT_IS_EXPORT_ASK( dialog ));

	priv = CACT_EXPORT_ASK( dialog )->private;

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
	}
}

static void
instance_dispose( GObject *dialog )
{
	static const gchar *thisfn = "cact_export_ask_instance_dispose";
	CactExportAsk *self;

	g_return_if_fail( CACT_IS_EXPORT_ASK( dialog ));

	self = CACT_EXPORT_ASK( dialog );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( dialog );
		}
	}
}

static void
instance_finalize( GObject *dialog )
{
	static const gchar *thisfn = "cact_export_ask_instance_finalize";
	CactExportAsk *self;

	g_return_if_fail( CACT_IS_EXPORT_ASK( dialog ));

	g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

	self = CACT_EXPORT_ASK( dialog );

	g_free( self->private->format );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( dialog );
	}
}

/**
 * cact_export_ask_user:
 * @parent: the CactAssistant parent of this dialog.
 * @item: the NAObjectItem to be exported.
 * @first: whether this is the first call of a serie.
 *  On a first call, the user is really asked for his choice.
 *  The next times, the 'keep-last-choice' flag will be considered.
 *
 * Initializes and runs the dialog.
 *
 * This is a small dialog which is to be ran during export operations,
 * when the set export format is 'Ask me'. Each exported file runs this
 * dialog, unless the user selects the 'keep same choice' box.
 *
 * Returns: the export format chosen by the user as a newly allocated
 * string which should be g_free() by the caller.
 * The function defaults to returning NA_IPREFS_DEFAULT_EXPORT_FORMAT.
 *
 * When the user selects 'Keep same choice without asking me', this choice
 * becomes his new preferred export format.
 */
gchar *
cact_export_ask_user( BaseWindow *parent, NAObjectItem *item, gboolean first )
{
	static const gchar *thisfn = "cact_export_ask_user";
	CactExportAsk *editor;
	gboolean are_locked, mandatory;
	gboolean keep, keep_mandatory;
	int code;
	gchar *format;

	g_return_val_if_fail( BASE_IS_WINDOW( parent ), NULL );

	g_debug( "%s: parent=%p, item=%p (%s), first=%s",
			thisfn,
			( void * ) parent,
			( void * ) item, G_OBJECT_TYPE_NAME( item ),
			first ? "True":"False" );

	format = na_settings_get_string( NA_IPREFS_EXPORT_ASK_USER_LAST_FORMAT, NULL, &mandatory );
	keep = na_settings_get_boolean( NA_IPREFS_EXPORT_ASK_USER_KEEP_LAST_CHOICE, NULL, &keep_mandatory );

	if( first || !keep ){
		editor = g_object_new( CACT_TYPE_EXPORT_ASK,
				BASE_PROP_PARENT,         parent,
				BASE_PROP_XMLUI_FILENAME, st_xmlui_filename,
				BASE_PROP_TOPLEVEL_NAME,  st_toplevel_name,
				BASE_PROP_WSP_NAME,       st_wsp_name,
				NULL );

		editor->private->format = g_strdup( format );
		editor->private->format_mandatory = mandatory;
		editor->private->keep_last_choice = keep;
		editor->private->keep_last_choice_mandatory = keep_mandatory;
		editor->private->parent = parent;
		editor->private->item = item;

		are_locked = na_settings_get_boolean( NA_IPREFS_ADMIN_PREFERENCES_LOCKED, NULL, &mandatory );
		editor->private->preferences_locked = are_locked && mandatory;
		code = base_window_run( BASE_WINDOW( editor ));

		switch( code ){
			case GTK_RESPONSE_OK:
				g_free( format );
				format = get_export_format( editor );
				break;

			case GTK_RESPONSE_CANCEL:
			/* base_dialog::do_run only returns OK or CANCEL */
			default:
				g_free( format );
				format = g_strdup( EXPORTER_FORMAT_NOEXPORT );
				break;
		}

		g_object_unref( editor );
	}

	return( format );
}

static void
on_base_initialize_gtk( CactExportAsk *editor, GtkDialog *toplevel, gpointer user_data )
{
	static const gchar *thisfn = "cact_export_ask_on_base_initialize_gtk";
	GtkWidget *container;

	g_return_if_fail( CACT_IS_EXPORT_ASK( editor ));

	if( !editor->private->dispose_has_run ){

		g_debug( "%s: dialog=%p, toplevel=%p, user_data=%p",
				thisfn, ( void * ) editor, ( void * ) toplevel, ( void * ) user_data );

		container = base_window_get_widget( BASE_WINDOW( editor ), "ExportFormatAskVBox" );
		na_ioptions_list_gtk_init( NA_IOPTIONS_LIST( editor ), container, FALSE );

#if !GTK_CHECK_VERSION( 2,22,0 )
		gtk_dialog_set_has_separator( toplevel, FALSE );
#endif
	}
}

static void
on_base_initialize_window( CactExportAsk *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_export_ask_on_base_initialize_window";
	gchar *item_label, *label;
	GtkWidget *widget;
	CactExportAskPrivate *priv;

	g_return_if_fail( CACT_IS_EXPORT_ASK( editor ));

	priv = editor->private;

	if( !priv->dispose_has_run ){

		g_debug( "%s: dialog=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

		item_label = na_object_get_label( priv->item );

		if( NA_IS_OBJECT_ACTION( priv->item )){
			/* i18n: The action <label> is about to be exported */
			label = g_strdup_printf( _( "The action \"%s\" is about to be exported." ), item_label );
		} else {
			/* i18n: The menu <label> is about to be exported */
			label = g_strdup_printf( _( "The menu \"%s\" is about to be exported." ), item_label );
		}

		widget = base_window_get_widget( BASE_WINDOW( editor ), "ExportAskLabel" );
		gtk_label_set_text( GTK_LABEL( widget ), label );
		g_free( label );
		g_free( item_label );

		widget = base_window_get_widget( BASE_WINDOW( editor ), "ExportFormatAskVBox" );
		na_ioptions_list_set_editable(
				NA_IOPTIONS_LIST( editor ), widget,
				!priv->format_mandatory && !priv->preferences_locked );
		na_ioptions_list_set_default(
				NA_IOPTIONS_LIST( editor ), widget,
				priv->format );

		base_gtk_utils_toggle_set_initial_state( BASE_WINDOW( editor ),
				"AskKeepChoiceButton", G_CALLBACK( keep_choice_on_toggled ),
				priv->keep_last_choice,
				!priv->keep_last_choice_mandatory, !priv->preferences_locked );

		base_window_signal_connect_by_name(
				BASE_WINDOW( editor ),
				"CancelButton",
				"clicked",
				G_CALLBACK( on_cancel_clicked ));

		base_window_signal_connect_by_name(
				BASE_WINDOW( editor ),
				"OKButton",
				"clicked",
				G_CALLBACK( on_ok_clicked ));
	}
}

static void
keep_choice_on_toggled( GtkToggleButton *button, CactExportAsk *editor )
{
	gboolean editable;

	editable = ( gboolean ) GPOINTER_TO_UINT( g_object_get_data( G_OBJECT( button ), NA_TOGGLE_DATA_EDITABLE ));

	if( editable ){
		editor->private->keep_last_choice = gtk_toggle_button_get_active( button );

	} else {
		base_gtk_utils_toggle_reset_initial_state( button );
	}
}

static void
on_cancel_clicked( GtkButton *button, CactExportAsk *editor )
{
	GtkWindow *toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_CLOSE );
}

static void
on_ok_clicked( GtkButton *button, CactExportAsk *editor )
{
	GtkWindow *toplevel = base_window_get_gtk_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_OK );
}

/*
 * we have come here because preferred_export_format was 'Ask'
 * in all cases, the chosen format is kept in 'ask_user_last_chosen_format'
 * and so this will be the default format which will be proposed the next
 * time we come here
 * if the 'remember_my_choice' is cheecked, then we do not even ask the user
 * but returns as soon as we enter
 *
 * not overrinding the preferred export format (i.e. letting it to 'Ask')
 * let the user go back in ask dialog box the next time he will have some
 * files to export
 */
static gchar *
get_export_format( CactExportAsk *editor )
{
	GtkWidget *widget;
	NAIOption *format;
	gchar *format_id;

	widget = base_window_get_widget( BASE_WINDOW( editor ), "ExportFormatAskVBox" );
	format = na_ioptions_list_get_selected( NA_IOPTIONS_LIST( editor ), widget );
	g_return_val_if_fail( NA_IS_EXPORT_FORMAT( format ), 0 );

	if( !editor->private->keep_last_choice_mandatory ){
		na_settings_set_boolean( NA_IPREFS_EXPORT_ASK_USER_KEEP_LAST_CHOICE, editor->private->keep_last_choice );
	}

	format_id = na_ioption_get_id( format );
	na_settings_set_string( NA_IPREFS_EXPORT_ASK_USER_LAST_FORMAT, format_id );

	return( format_id );
}
