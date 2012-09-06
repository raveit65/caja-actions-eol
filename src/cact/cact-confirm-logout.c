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

#include "cact-confirm-logout.h"
#include "cact-main-menubar-file.h"

/* private class data
 */
struct CactConfirmLogoutClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct CactConfirmLogoutPrivate {
	gboolean dispose_has_run;
	gboolean willing_to_quit;
};

enum {
	BTN_QUIT_WITHOUT_SAVING = 1,
	BTN_CANCEL,
	BTN_SAVE_AND_QUIT
};

static BaseDialogClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( CactConfirmLogoutClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *dialog );
static void     instance_finalize( GObject *dialog );

static CactConfirmLogout *confirm_logout_new( BaseWindow *parent );

static gchar   *base_get_dialog_name( const BaseWindow *window );
static void     on_base_initial_load_dialog( CactConfirmLogout *editor, gpointer user_data );
static void     on_base_runtime_init_dialog( CactConfirmLogout *editor, gpointer user_data );
static void     on_base_all_widgets_showed( CactConfirmLogout *editor, gpointer user_data );
static void     on_quit_without_saving_clicked( GtkButton *button, CactConfirmLogout *editor );
static void     on_cancel_clicked( GtkButton *button, CactConfirmLogout *editor );
static void     on_save_and_quit_clicked( GtkButton *button, CactConfirmLogout *editor );

static gboolean base_dialog_response( GtkDialog *dialog, gint code, BaseWindow *window );
static void     close_dialog( CactConfirmLogout *editor, gboolean willing_to );

GType
cact_confirm_logout_get_type( void )
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
	static const gchar *thisfn = "cact_confirm_logout_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( CactConfirmLogoutClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( CactConfirmLogout ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_DIALOG_TYPE, "CactConfirmLogout", &info, 0 );

	return( type );
}

static void
class_init( CactConfirmLogoutClass *klass )
{
	static const gchar *thisfn = "cact_confirm_logout_class_init";
	GObjectClass *object_class;
	BaseWindowClass *base_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( CactConfirmLogoutClassPrivate, 1 );

	base_class = BASE_WINDOW_CLASS( klass );
	base_class->dialog_response = base_dialog_response;
	base_class->get_toplevel_name = base_get_dialog_name;
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "cact_confirm_logout_instance_init";
	CactConfirmLogout *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( CACT_IS_CONFIRM_LOGOUT( instance ));
	self = CACT_CONFIRM_LOGOUT( instance );

	self->private = g_new0( CactConfirmLogoutPrivate, 1 );

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
	static const gchar *thisfn = "cact_confirm_logout_instance_dispose";
	CactConfirmLogout *self;

	g_debug( "%s: dialog=%p (%s)", thisfn, ( void * ) dialog, G_OBJECT_TYPE_NAME( dialog ));
	g_return_if_fail( CACT_IS_CONFIRM_LOGOUT( dialog ));
	self = CACT_CONFIRM_LOGOUT( dialog );

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
	static const gchar *thisfn = "cact_confirm_logout_instance_finalize";
	CactConfirmLogout *self;

	g_debug( "%s: dialog=%p", thisfn, ( void * ) dialog );
	g_return_if_fail( CACT_IS_CONFIRM_LOGOUT( dialog ));
	self = CACT_CONFIRM_LOGOUT( dialog );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( dialog );
	}
}

/*
 * Returns a newly allocated CactConfirmLogout object.
 *
 * @parent: the BaseWindow parent of this dialog (usually, the main
 * toplevel window of the application).
 */
static CactConfirmLogout *
confirm_logout_new( BaseWindow *parent )
{
	return( g_object_new( CACT_CONFIRM_LOGOUT_TYPE, BASE_WINDOW_PROP_PARENT, parent, NULL ));
}

/**
 * cact_confirm_logout_run:
 * @parent: the CactMainWindow parent of this dialog
 * (usually the CactMainWindow).
 *
 * Initializes and runs the dialog.
 */
gboolean
cact_confirm_logout_run( CactMainWindow *parent )
{
	static const gchar *thisfn = "cact_confirm_logout_run";
	CactConfirmLogout *editor;
	gboolean willing_to;

	g_debug( "%s: parent=%p", thisfn, ( void * ) parent );
	g_return_val_if_fail( BASE_IS_WINDOW( parent ), TRUE );

	editor = confirm_logout_new( BASE_WINDOW( parent ));

	base_window_run( BASE_WINDOW( editor ));
	willing_to = editor->private->willing_to_quit;
	g_object_unref( editor );

	return( willing_to );
}

static gchar *
base_get_dialog_name( const BaseWindow *window )
{
	return( g_strdup( "ConfirmLogoutDialog" ));
}

static void
on_base_initial_load_dialog( CactConfirmLogout *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_confirm_logout_on_initial_load_dialog";

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
	g_return_if_fail( CACT_IS_CONFIRM_LOGOUT( editor ));
}

static void
on_base_runtime_init_dialog( CactConfirmLogout *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_confirm_logout_on_runtime_init_dialog";

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"QuitNoSaveButton",
			"clicked",
			G_CALLBACK( on_quit_without_saving_clicked ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"CancelQuitButton",
			"clicked",
			G_CALLBACK( on_cancel_clicked ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"SaveQuitButton",
			"clicked",
			G_CALLBACK( on_save_and_quit_clicked ));
}

static void
on_base_all_widgets_showed( CactConfirmLogout *editor, gpointer user_data )
{
	static const gchar *thisfn = "cact_confirm_logout_on_all_widgets_showed";

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
}

static void
on_quit_without_saving_clicked( GtkButton *button, CactConfirmLogout *editor )
{
	static const gchar *thisfn = "cact_confirm_logout_on_quit_without_saving_clicked";

	g_debug( "%s: button=%p, editor=%p", thisfn, ( void * ) button, ( void * ) editor );

	close_dialog( editor, TRUE );
}

static void
on_cancel_clicked( GtkButton *button, CactConfirmLogout *editor )
{
	static const gchar *thisfn = "cact_confirm_logout_on_cancel_clicked";

	g_debug( "%s: button=%p, editor=%p", thisfn, ( void * ) button, ( void * ) editor );

	close_dialog( editor, FALSE );
}

static void
on_save_and_quit_clicked( GtkButton *button, CactConfirmLogout *editor )
{
	static const gchar *thisfn = "cact_confirm_logout_on_cancel_clicked";
	CactMainWindow *main_window;

	g_debug( "%s: button=%p, editor=%p", thisfn, ( void * ) button, ( void * ) editor );

	main_window = CACT_MAIN_WINDOW( base_window_get_parent( BASE_WINDOW( editor )));
	cact_main_menubar_file_save_items( main_window );

	close_dialog( editor, TRUE );
}

static gboolean
base_dialog_response( GtkDialog *dialog, gint code, BaseWindow *window )
{
	static const gchar *thisfn = "cact_confirm_logout_on_dialog_response";
	CactConfirmLogout *editor;

	g_debug( "%s: dialog=%p, code=%d, window=%p", thisfn, ( void * ) dialog, code, ( void * ) window );
	g_assert( CACT_IS_CONFIRM_LOGOUT( window ));
	editor = CACT_CONFIRM_LOGOUT( window );

	switch( code ){
		case GTK_RESPONSE_CLOSE:

			return( TRUE );
			break;
	}

	return( FALSE );
}

static void
close_dialog( CactConfirmLogout *editor, gboolean willing_to )
{
	static const gchar *thisfn = "cact_confirm_logout_close_dialog";
	GtkWindow *toplevel;

	g_debug( "%s: editor=%p, willing_to=%s", thisfn, ( void * ) editor, willing_to ? "True":"False" );

	editor->private->willing_to_quit = willing_to;

	toplevel = base_window_get_toplevel( BASE_WINDOW( editor ));
	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_CLOSE );
}
