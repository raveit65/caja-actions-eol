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

#include <api/na-object-api.h>

#include <core/na-exporter.h>

#include "cact-application.h"
#include "cact-iprefs.h"
#include "cact-export-format.h"
#include "cact-export-ask.h"

/* private class data
 */
struct CactExportAskClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct CactExportAskPrivate {
	gboolean      dispose_has_run;
	BaseWindow   *parent;
	NAObjectItem *item;
	GQuark        format;
};

static BaseDialogClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( CactExportAskClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *dialog );
static void     instance_finalize( GObject *dialog );

static CactExportAsk *export_ask_new( BaseWindow *parent );

static gchar   *base_get_iprefs_window_id( const BaseWindow *window );
static gchar   *base_get_dialog_name( const BaseWindow *window );
static gchar   *base_get_ui_filename( const BaseWindow *dialog );
static void     on_base_initial_load_dialog( CactExportAsk *editor, gpointer user_data );
static void     on_base_runtime_init_dialog( CactExportAsk *editor, gpointer user_data );
static void     on_base_all_widgets_showed( CactExportAsk *editor, gpointer user_data );
static void     on_cancel_clicked( GtkButton *button, CactExportAsk *editor );
static void     on_ok_clicked( GtkButton *button, CactExportAsk *editor );
static GQuark   get_export_format( CactExportAsk *editor );
static gboolean base_dialog_response( GtkDialog *dialog, gint code, BaseWindow *window );

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

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_DIALOG_TYPE, "CactExportAsk", &info, 0 );

	return( type );
}

static void
class_init( CactExportAskClass *klass )
{
	static const gchar *thisfn = "cact_export_ask_class_init";
	GObjectClass *object_class;
	BaseWindowClass *base_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactExportAskClassPrivate, 1 );

	base_class = BASE_WINDOW_CLASS( klass );
	base_class->dialog_response = base_dialog_response;
	base_class->get_toplevel_name = base_get_dialog_name;
	base_class->get_iprefs_window_id = base_get_iprefs_window_id;
	base_class->get_ui_filename = base_get_ui_filename;
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_export_ask_instance_init";
	CactExportAsk *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( CACT_IS_EXPORT_ASK( instance ));
	self = CACT_EXPORT_ASK( instance );

	self->private = g_new0( CactExportAskPrivate, 1 );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_INITIAL_LOAD,
			G_CALLBACK( on_base_initial_load_dialog ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_RUNTIME_INIT,
			G_CALLBACK( on_base_runtime_init_dialog ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_ALL_WIDGETS_SHOWED,
			G_CALLBACK( on_base_all_widgets_showed));

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *dialog )
{
	static const gchar *thisfn = "cact_export_ask_instance_dispose";
	CactExportAsk *self;

	g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));
	g_return_if_fail( CACT_IS_EXPORT_ASK( dialog ));
	self = CACT_EXPORT_ASK( dialog );

	if( !self->private->dispose_has_run ){

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

	g_debug( "%s: dialog=%p", thisfn, ( void * ) dialog );
	g_return_if_fail( CACT_IS_EXPORT_ASK( dialog ));
	self = CACT_EXPORT_ASK( dialog );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( dialog );
	}
}

/*
 * Returns a newly allocated CactExportAsk object.
 */
static CactExportAsk *
export_ask_new( BaseWindow *parent )
{
	return( g_object_new( CACT_EXPORT_ASK_TYPE, BASE_WINDOW_PROP_PARENT, parent, NULL ));
}

/**
 * cact_export_ask_run:
 * @parent: the CactAssistant parent of this dialog.
 *
 * Initializes and runs the dialog.
 *
 * This is a small dialog which is to be ran during export operations,
 * when the set export format is 'Ask me'. Each exported file runs this
 * dialog, unless the user selects the 'keep same choice' box.
 *
 * Returns: the mode choosen by the user as a #GQuark which identifies
 * the export mode.
 * The function defaults to returning IPREFS_EXPORT_NO_EXPORT.
 *
 * When the user selects 'Keep same choice without asking me', this choice
 * becomes his new preferred export format.
 */
GQuark
cact_export_ask_user( BaseWindow *parent, NAObjectItem *item )
{
	static const gchar *thisfn = "cact_export_ask_run";
	CactExportAsk *editor;
	GQuark format;

	g_debug( "%s: parent=%p", thisfn, ( void * ) parent );

	format = ( GQuark ) IPREFS_EXPORT_NO_EXPORT;

	g_return_val_if_fail( BASE_IS_WINDOW( parent ), format );

	editor = export_ask_new( parent );

	editor->private->parent = parent;
	editor->private->item = item;
	editor->private->format = cact_iprefs_get_export_format( BASE_WINDOW( parent ), IPREFS_EXPORT_ASK_LAST_FORMAT );

	if( base_window_run( BASE_WINDOW( editor ))){

		if( editor->private->format ){

			format = editor->private->format;
			cact_iprefs_set_export_format( BASE_WINDOW( parent ), IPREFS_EXPORT_ASK_LAST_FORMAT, format );
		}
	}

	g_object_unref( editor );

	return( format );
}

static gchar *
base_get_iprefs_window_id( const BaseWindow *window )
{
	return( g_strdup( "export-ask-user" ));
}

static gchar *
base_get_dialog_name( const BaseWindow *window )
{
	return( g_strdup( "ExportAskDialog" ));
}

static gchar *
base_get_ui_filename( const BaseWindow *dialog )
{
	return( g_strdup( PKGDATADIR "/cact-assistant-export.ui" ));
}

static void
on_base_initial_load_dialog( CactExportAsk *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_export_ask_on_initial_load_dialog";
	CactApplication *application;
	NAUpdater *updater;
	GtkWidget *container;

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
	g_return_if_fail( CACT_IS_EXPORT_ASK( editor ));

	application = CACT_APPLICATION( base_window_get_application( BASE_WINDOW( editor )));
	updater = cact_application_get_updater( application );
	container = base_window_get_widget( BASE_WINDOW( editor ), "ExportFormatAskVBox" );
	cact_export_format_init_display( NA_PIVOT( updater ), container, EXPORT_FORMAT_DISPLAY_ASK );
}

static void
on_base_runtime_init_dialog( CactExportAsk *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_export_ask_on_runtime_init_dialog";
	GtkWidget *container;
	gchar *item_label, *label;
	GtkWidget *widget;
	GtkWidget *button;

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
	g_return_if_fail( CACT_IS_EXPORT_ASK( editor ));

	item_label = na_object_get_label( editor->private->item );

	if( NA_IS_OBJECT_ACTION( editor->private->item )){
		/* i18n: The action <label> is about to be exported */
		label = g_strdup_printf( _( "The action \"%s\" is about to be exported." ), item_label );
	} else {
		/* i18n: The menu <label> is about to be exported */
		label = g_strdup_printf( _( "The menu \"%s\" is about to be exported." ), item_label );
	}

	widget = base_window_get_widget( BASE_WINDOW( editor ), "ExportAskLabel1" );
	gtk_label_set_text( GTK_LABEL( widget ), label );
	g_free( label );
	g_free( item_label );

	button = base_window_get_widget( BASE_WINDOW( editor ), "AskKeepChoiceButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), FALSE );

	container = base_window_get_widget( BASE_WINDOW( editor ), "ExportFormatAskVBox" );
	cact_export_format_select( container, editor->private->format );

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

static void
on_base_all_widgets_showed( CactExportAsk *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_export_ask_on_all_widgets_showed";

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
	g_return_if_fail( CACT_IS_EXPORT_ASK( editor ));
}

static void
on_cancel_clicked( GtkButton *button, CactExportAsk *editor )
{
	GtkWindow *toplevel = base_window_get_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_CLOSE );
}

static void
on_ok_clicked( GtkButton *button, CactExportAsk *editor )
{
	GtkWindow *toplevel = base_window_get_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_OK );
}

static GQuark
get_export_format( CactExportAsk *editor )
{
	GtkWidget *container;
	NAExportFormat *format;
	GQuark format_quark;
	GtkWidget *button;
	gboolean keep;

	container = base_window_get_widget( BASE_WINDOW( editor ), "ExportFormatAskVBox" );
	format = cact_export_format_get_selected( container );
	format_quark = na_export_format_get_quark( format );

	button = base_window_get_widget( BASE_WINDOW( editor ), "AskKeepChoiceButton" );
	keep = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	if( keep ){
		cact_iprefs_set_export_format( BASE_WINDOW( editor ), IPREFS_EXPORT_FORMAT, format_quark );
	}

	return( format_quark );
}

static gboolean
base_dialog_response( GtkDialog *dialog, gint code, BaseWindow *window )
{
	static const gchar *thisfn = "cact_export_ask_on_dialog_response";
	CactExportAsk *editor;

	g_debug( "%s: dialog=%p, code=%d, window=%p", thisfn, ( void * ) dialog, code, ( void * ) window );
	g_assert( CACT_IS_EXPORT_ASK( window ));
	editor = CACT_EXPORT_ASK( window );

	switch( code ){
		case GTK_RESPONSE_NONE:
		case GTK_RESPONSE_DELETE_EVENT:
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_CANCEL:

			editor->private->format = 0;
			return( TRUE );
			break;

		case GTK_RESPONSE_OK:
			editor->private->format = get_export_format( editor );
			return( TRUE );
			break;
	}

	return( FALSE );
}
