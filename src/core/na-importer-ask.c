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
#include <gtk/gtk.h>

#include <api/na-object-api.h>

#include "na-gtk-utils.h"
#include "na-iprefs.h"
#include "na-importer-ask.h"

/* private class data
 */
struct NAImporterAskClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAImporterAskPrivate {
	gboolean          dispose_has_run;
	GtkBuilder       *builder;
	GtkWindow        *toplevel;
	NAIImporterUriParms *parms;
	NAObjectItem     *existing;
	guint             mode;
	MateConfClient      *mateconf;
	gint              dialog_code;
};

static GtkDialogClass *st_parent_class = NULL;

static GType      register_type( void );
static void       class_init( NAImporterAskClass *klass );
static void       instance_init( GTypeInstance *instance, gpointer klass );
static void       instance_dispose( GObject *dialog );
static void       instance_finalize( GObject *dialog );

static NAImporterAsk *import_ask_new();

static void       init_dialog( NAImporterAsk *editor );
static void       on_cancel_clicked( GtkButton *button, NAImporterAsk *editor );
static void       on_ok_clicked( GtkButton *button, NAImporterAsk *editor );
static void       get_selected_mode( NAImporterAsk *editor );
static gboolean   on_dialog_response( NAImporterAsk *editor, gint code );

GType
na_importer_ask_get_type( void )
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
	static const gchar *thisfn = "na_importer_ask_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAImporterAskClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAImporterAsk ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( GTK_TYPE_DIALOG, "NAImporterAsk", &info, 0 );

	return( type );
}

static void
class_init( NAImporterAskClass *klass )
{
	static const gchar *thisfn = "na_importer_ask_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAImporterAskClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_importer_ask_instance_init";
	NAImporterAsk *self;
	GError *error;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( NA_IS_IMPORTER_ASK( instance ));
	self = NA_IMPORTER_ASK( instance );

	self->private = g_new0( NAImporterAskPrivate, 1 );

	self->private->builder = gtk_builder_new();

	error = NULL;
	gtk_builder_add_from_file( self->private->builder, PKGDATADIR "/na-importer-ask.ui", &error );
	if( error ){
		g_warning( "%s: %s", thisfn, error->message );
		g_error_free( error );

	} else {
		self->private->toplevel = GTK_WINDOW( gtk_builder_get_object( self->private->builder, "ImporterAskDialog" ));
	}

	self->private->mateconf = mateconf_client_get_default();

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *dialog )
{
	static const gchar *thisfn = "na_importer_ask_instance_dispose";
	NAImporterAsk *self;

	g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));

	g_return_if_fail( NA_IS_IMPORTER_ASK( dialog ));
	self = NA_IMPORTER_ASK( dialog );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		g_object_unref( self->private->mateconf );
		g_object_unref( self->private->builder );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( dialog );
		}
	}
}

static void
instance_finalize( GObject *dialog )
{
	static const gchar *thisfn = "na_importer_ask_instance_finalize";
	NAImporterAsk *self;

	g_debug( "%s: dialog=%p", thisfn, ( void * ) dialog );
	g_return_if_fail( NA_IS_IMPORTER_ASK( dialog ));
	self = NA_IMPORTER_ASK( dialog );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( dialog );
	}
}

/*
 * Returns a newly allocated NAImporterAsk object.
 */
static NAImporterAsk *
import_ask_new()
{
	return( g_object_new( NA_IMPORTER_ASK_TYPE, NULL ));
}

/**
 * na_importer_ask_user:
 * @importer: this #NAIImporter instance.
 * @parms: a #NAIImporterUriParms structure.
 * @existing: the #NAObjectItem-derived already existing object.
 *
 * Ask the user for what to do when an imported item has the same ID
 * that an already existing one.
 *
 * Returns: the definitive import mode.
 *
 * When the user selects 'Keep same choice without asking me', this choice
 * becomes his preference import mode.
 */
guint
na_importer_ask_user( const NAIImporterUriParms *parms, const NAObjectItem *existing )
{
	static const gchar *thisfn = "na_importer_ask_user";
	NAImporterAsk *dialog;
	guint mode;
	gint code;

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( existing ), IMPORTER_MODE_NO_IMPORT );

	g_debug( "%s: parms=%p, existing=%p", thisfn, ( void * ) parms, ( void * ) existing );

	mode = IMPORTER_MODE_NO_IMPORT;
	dialog = import_ask_new();

	if( dialog->private->toplevel ){

		dialog->private->parms = ( NAIImporterUriParms * ) parms;
		dialog->private->existing = ( NAObjectItem * ) existing;
		dialog->private->mode = na_iprefs_get_import_mode( dialog->private->mateconf, IPREFS_IMPORT_ASK_LAST_MODE );

		init_dialog( dialog );
		/* toplevel is modal, not dialog
		g_debug( "dialog is modal: %s", gtk_window_get_modal( GTK_WINDOW( dialog )) ? "True":"False" );
		g_debug( "toplevel is modal: %s", gtk_window_get_modal( dialog->private->toplevel ) ? "True":"False" );*/
		do {
			code = gtk_dialog_run( GTK_DIALOG( dialog->private->toplevel ));
		} while ( !on_dialog_response( dialog, code ));

		mode = dialog->private->mode;

		gtk_widget_hide_all( GTK_WIDGET( dialog->private->toplevel ));
		gtk_widget_destroy( GTK_WIDGET( dialog->private->toplevel ));

	} else {
		g_object_unref( dialog );
	}

	return( mode );
}

static void
init_dialog( NAImporterAsk *editor )
{
	static const gchar *thisfn = "na_importer_ask_init_dialog";
	gchar *imported_label, *existing_label;
	gchar *label;
	GtkWidget *widget;
	GtkWidget *button;

	g_return_if_fail( NA_IS_IMPORTER_ASK( editor ));

	g_debug( "%s: editor=%p", thisfn, ( void * ) editor );

	imported_label = na_object_get_label( editor->private->parms->imported );
	existing_label = na_object_get_label( editor->private->existing );

	if( NA_IS_OBJECT_ACTION( editor->private->parms->imported )){
		/* i18n: The action <action_label> imported from <file> has the same id than <existing_label> */
		label = g_strdup_printf(
				_( "The action \"%s\" imported from \"%s\" has the same identifiant than the already existing \"%s\"." ),
				imported_label, editor->private->parms->uri, existing_label );

	} else {
		/* i18n: The menu <menu_label> imported from <file> has the same id than <existing_label> */
		label = g_strdup_printf(
				_( "The menu \"%s\" imported from \"%s\" has the same identifiant than the already existing \"%s\"." ),
				imported_label, editor->private->parms->uri, existing_label );
	}

	widget = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "ImporterAskLabel" );
	gtk_label_set_text( GTK_LABEL( widget ), label );
	g_free( label );

	switch( editor->private->mode ){
		case IMPORTER_MODE_RENUMBER:
			button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "AskRenumberButton" );
			break;

		case IMPORTER_MODE_OVERRIDE:
			button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "AskOverrideButton" );
			break;

		case IMPORTER_MODE_NO_IMPORT:
		default:
			button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "AskNoImportButton" );
			break;
	}
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );

	button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "AskKeepChoiceButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), FALSE );

	button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "OKButton" );
	g_signal_connect(
			G_OBJECT( button ),
			"clicked",
			G_CALLBACK( on_ok_clicked ),
			editor );

	button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "CancelButton" );
	g_signal_connect(
			G_OBJECT( button ),
			"clicked",
			G_CALLBACK( on_cancel_clicked ),
			editor );

	if( editor->private->parms->window ){
		gtk_window_set_transient_for( editor->private->toplevel, editor->private->parms->window );
	}

	gtk_widget_show_all( GTK_WIDGET( editor->private->toplevel ));
}

static void
on_cancel_clicked( GtkButton *button, NAImporterAsk *editor )
{
	g_debug( "na_importer_ask_on_cancel_clicked" );
	editor->private->dialog_code = GTK_RESPONSE_CANCEL;
	gtk_dialog_response( GTK_DIALOG( editor ), GTK_RESPONSE_CANCEL );
}

static void
on_ok_clicked( GtkButton *button, NAImporterAsk *editor )
{
	g_debug( "na_importer_ask_on_ok_clicked" );
	editor->private->dialog_code = GTK_RESPONSE_OK;
	gtk_dialog_response( GTK_DIALOG( editor ), GTK_RESPONSE_OK );
}

static void
get_selected_mode( NAImporterAsk *editor )
{
	guint import_mode;
	GtkWidget *button;
	gboolean keep;

	import_mode = IMPORTER_MODE_NO_IMPORT;

	button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "AskRenumberButton" );
	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
		import_mode = IMPORTER_MODE_RENUMBER;

	} else {
		button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "AskOverrideButton" );
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
			import_mode = IMPORTER_MODE_OVERRIDE;
		}
	}

	editor->private->mode = import_mode;
	na_iprefs_set_import_mode( editor->private->mateconf, IPREFS_IMPORT_ASK_LAST_MODE, editor->private->mode );

	button = na_gtk_utils_search_for_child_widget( GTK_CONTAINER( editor->private->toplevel ), "AskKeepChoiceButton" );
	keep = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	if( keep ){
		na_iprefs_set_import_mode( editor->private->mateconf, IPREFS_IMPORT_ITEMS_IMPORT_MODE, import_mode );
	}
}

/*
 * very curiously, the code sent by on_cancel_clicked() and
 * on_ok_clicked() arrives here at zero ! we so rely on a private one !
 */
static gboolean
on_dialog_response( NAImporterAsk *editor, gint code )
{
	static const gchar *thisfn = "na_importer_ask_on_dialog_response";

	g_return_val_if_fail( NA_IS_IMPORTER_ASK( editor ), FALSE );

	g_debug( "%s: editor=%p, code=%d", thisfn, ( void * ) editor, code );

	switch( editor->private->dialog_code ){
		case GTK_RESPONSE_CANCEL:
			editor->private->mode = IMPORTER_MODE_NO_IMPORT;
			return( TRUE );
			break;

		case GTK_RESPONSE_OK:
			get_selected_mode( editor );
			return( TRUE );
			break;

		case GTK_RESPONSE_NONE:
		case GTK_RESPONSE_DELETE_EVENT:
		case GTK_RESPONSE_CLOSE:
			break;
	}

	return( FALSE );
}
