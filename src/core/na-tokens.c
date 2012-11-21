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

#include <gio/gunixinputstream.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include "na-mate-vfs-uri.h"
#include "na-selected-info.h"
#include "na-settings.h"
#include "na-tokens.h"

/* private class data
 */
struct _NATokensClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _NATokensPrivate {
	gboolean dispose_has_run;
	guint    count;
	GSList  *uris;
	GSList  *filenames;
	GSList  *basedirs;
	GSList  *basenames;
	GSList  *basenames_woext;
	GSList  *exts;
	GSList  *mimetypes;
	gchar   *hostname;
	gchar   *username;
	guint    port;
	gchar   *scheme;
};

/*  the structure passed to the callback which waits for the end of the child
 */
typedef struct {
	gchar   *command;
	gboolean is_output_displayed;
	gint     child_stdout;
	gint     child_stderr;
}
	ChildStr;

static GObjectClass *st_parent_class = NULL;

static GType     register_type( void );
static void      class_init( NATokensClass *klass );
static void      instance_init( GTypeInstance *instance, gpointer klass );
static void      instance_dispose( GObject *object );
static void      instance_finalize( GObject *object );

static void      child_watch_fn( GPid pid, gint status, ChildStr *child_str );
static void      display_output( const gchar *command, int fd_stdout, int fd_stderr );
static gchar    *display_output_get_content( int fd );
static void      execute_action_command( gchar *command, const NAObjectProfile *profile, const NATokens *tokens );
static gchar    *get_command_execution_display_output( const gchar *command );
static gchar    *get_command_execution_embedded( const gchar *command );
static gchar    *get_command_execution_normal( const gchar *command );
static gchar    *get_command_execution_terminal( const gchar *command );
static gboolean  is_singular_exec( const NATokens *tokens, const gchar *exec );
static gchar    *parse_singular( const NATokens *tokens, const gchar *input, guint i, gboolean utf8, gboolean quoted );
static GString  *quote_string( GString *input, const gchar *name, gboolean quoted );
static GString  *quote_string_list( GString *input, GSList *names, gboolean quoted );

GType
na_tokens_get_type( void )
{
	static GType object_type = 0;

	if( !object_type ){
		object_type = register_type();
	}

	return( object_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_tokens_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NATokensClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NATokens ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NATokens", &info, 0 );

	return( type );
}

static void
class_init( NATokensClass *klass )
{
	static const gchar *thisfn = "na_tokens_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NATokensClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_tokens_instance_init";
	NATokens *self;

	g_return_if_fail( NA_IS_TOKENS( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = NA_TOKENS( instance );

	self->private = g_new0( NATokensPrivate, 1 );

	self->private->uris = NULL;
	self->private->filenames = NULL;
	self->private->basedirs = NULL;
	self->private->basenames = NULL;
	self->private->basenames_woext = NULL;
	self->private->exts = NULL;
	self->private->mimetypes = NULL;
	self->private->hostname = NULL;
	self->private->username = NULL;
	self->private->port = 0;
	self->private->scheme = NULL;

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_tokens_instance_dispose";
	NATokens *self;

	g_return_if_fail( NA_IS_TOKENS( object ));

	self = NA_TOKENS( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_tokens_instance_finalize";
	NATokens *self;

	g_return_if_fail( NA_IS_TOKENS( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = NA_TOKENS( object );

	g_free( self->private->scheme );
	g_free( self->private->username );
	g_free( self->private->hostname );
	na_core_utils_slist_free( self->private->mimetypes );
	na_core_utils_slist_free( self->private->exts );
	na_core_utils_slist_free( self->private->basenames_woext );
	na_core_utils_slist_free( self->private->basenames );
	na_core_utils_slist_free( self->private->basedirs );
	na_core_utils_slist_free( self->private->filenames );
	na_core_utils_slist_free( self->private->uris );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/*
 * na_tokens_new_for_example:
 *
 * Returns: a new #NATokens object initialized with fake values for two
 * regular files, in order to be used as an example of an expanded command
 * line.
 */
NATokens *
na_tokens_new_for_example( void )
{
	NATokens *tokens;
	const gchar *ex_uri1 = _( "file:///path/to/file1.mid" );
	const gchar *ex_uri2 = _( "file:///path/to/file2.jpeg" );
	const gchar *ex_mimetype1 = _( "audio/x-midi" );
	const gchar *ex_mimetype2 = _( "image/jpeg" );
	const guint  ex_port = 8080;
	const gchar *ex_host = _( "test.example.net" );
	const gchar *ex_user = _( "user" );
	NAMateVFSURI *vfs;
	gchar *dirname, *bname, *bname_woext, *ext;
	GSList *is;
	gboolean first;

	tokens = g_object_new( NA_TYPE_TOKENS, NULL );
	first = TRUE;
	tokens->private->count = 2;

	tokens->private->uris = g_slist_append( tokens->private->uris, g_strdup( ex_uri1 ));
	tokens->private->uris = g_slist_append( tokens->private->uris, g_strdup( ex_uri2 ));

	for( is = tokens->private->uris ; is ; is = is->next ){
		vfs = g_new0( NAMateVFSURI, 1 );
		na_mate_vfs_uri_parse( vfs, is->data );

		tokens->private->filenames = g_slist_append( tokens->private->filenames, g_strdup( vfs->path ));
		dirname = g_path_get_dirname( vfs->path );
		tokens->private->basedirs = g_slist_append( tokens->private->basedirs, dirname );
		bname = g_path_get_basename( vfs->path );
		tokens->private->basenames = g_slist_append( tokens->private->basenames, bname );
		na_core_utils_dir_split_ext( bname, &bname_woext, &ext );
		tokens->private->basenames_woext = g_slist_append( tokens->private->basenames_woext, bname_woext );
		tokens->private->exts = g_slist_append( tokens->private->exts, ext );

		if( first ){
			tokens->private->scheme = g_strdup( vfs->scheme );
			first = FALSE;
		}

		na_mate_vfs_uri_free( vfs );
	}

	tokens->private->mimetypes = g_slist_append( tokens->private->mimetypes, g_strdup( ex_mimetype1 ));
	tokens->private->mimetypes = g_slist_append( tokens->private->mimetypes, g_strdup( ex_mimetype2 ));

	tokens->private->hostname = g_strdup( ex_host );
	tokens->private->username = g_strdup( ex_user );
	tokens->private->port = ex_port;

	return( tokens );
}

/*
 * na_tokens_new_from_selection:
 * @selection: a #GList list of #NASelectedInfo objects.
 *
 * Returns: a new #NATokens object which holds all possible tokens.
 */
NATokens *
na_tokens_new_from_selection( GList *selection )
{
	static const gchar *thisfn = "na_tokens_new_from_selection";
	NATokens *tokens;
	GList *it;
	gchar *uri, *filename, *basedir, *basename, *bname_woext, *ext, *mimetype;
	gboolean first;

	g_debug( "%s: selection=%p (count=%d)", thisfn, ( void * ) selection, g_list_length( selection ));

	first = TRUE;
	tokens = g_object_new( NA_TYPE_TOKENS, NULL );

	tokens->private->count = g_list_length( selection );

	for( it = selection ; it ; it = it->next ){
		mimetype = na_selected_info_get_mime_type( NA_SELECTED_INFO( it->data ));

		uri = na_selected_info_get_uri( NA_SELECTED_INFO( it->data ));
		filename = na_selected_info_get_path( NA_SELECTED_INFO( it->data ));
		basedir = na_selected_info_get_dirname( NA_SELECTED_INFO( it->data ));
		basename = na_selected_info_get_basename( NA_SELECTED_INFO( it->data ));
		na_core_utils_dir_split_ext( basename, &bname_woext, &ext );

		if( first ){
			tokens->private->hostname = na_selected_info_get_uri_host( NA_SELECTED_INFO( it->data ));
			tokens->private->username = na_selected_info_get_uri_user( NA_SELECTED_INFO( it->data ));
			tokens->private->port = na_selected_info_get_uri_port( NA_SELECTED_INFO( it->data ));
			tokens->private->scheme = na_selected_info_get_uri_scheme( NA_SELECTED_INFO( it->data ));
			first = FALSE;
		}

		tokens->private->uris = g_slist_append( tokens->private->uris, uri );
		tokens->private->filenames = g_slist_append( tokens->private->filenames, filename );
		tokens->private->basedirs = g_slist_append( tokens->private->basedirs, basedir );
		tokens->private->basenames = g_slist_append( tokens->private->basenames, basename );
		tokens->private->basenames_woext = g_slist_append( tokens->private->basenames_woext, bname_woext );
		tokens->private->exts = g_slist_append( tokens->private->exts, ext );
		tokens->private->mimetypes = g_slist_append( tokens->private->mimetypes, mimetype );
	}

	return( tokens );
}

/*
 * na_tokens_parse_for_display:
 * @tokens: a #NATokens object.
 * @string: the input string, may or may not contain tokens.
 * @utf8: whether the @input string is UTF-8 encoded, or a standard ASCII string.
 *
 * Expands the parameters in the given string.
 *
 * This expanded string is meant to be displayed only (not executed) as
 * filenames are not shell-quoted.
 *
 * Returns: a copy of @input string with tokens expanded, as a newly
 * allocated string which should be g_free() by the caller.
 */
gchar *
na_tokens_parse_for_display( const NATokens *tokens, const gchar *string, gboolean utf8 )
{
	return( parse_singular( tokens, string, 0, utf8, FALSE ));
}

/*
 * na_tokens_execute_action:
 * @tokens: a #NATokens object.
 * @profile: the #NAObjectProfile to be executed.
 *
 * Execute the given action, regarding the context described by @tokens.
 */
void
na_tokens_execute_action( const NATokens *tokens, const NAObjectProfile *profile )
{
	gchar *path, *parameters, *exec;
	gboolean singular;
	guint i;
	gchar *command;

	path = na_object_get_path( profile );
	parameters = na_object_get_parameters( profile );
	exec = g_strdup_printf( "%s %s", path, parameters );
	g_free( parameters );
	g_free( path );

	singular = is_singular_exec( tokens, exec );

	if( singular ){
		for( i = 0 ; i < tokens->private->count ; ++i ){
			command = parse_singular( tokens, exec, i, FALSE, TRUE );
			execute_action_command( command, profile, tokens );
			g_free( command );
		}

	} else {
		command = parse_singular( tokens, exec, 0, FALSE, TRUE );
		execute_action_command( command, profile, tokens );
		g_free( command );
	}

	g_free( exec );
}

static void
child_watch_fn( GPid pid, gint status, ChildStr *child_str )
{
	static const gchar *thisfn = "na_tokens_child_watch_fn";

	g_debug( "%s: pid=%u, status=%d", thisfn, ( guint ) pid, status );
	g_spawn_close_pid( pid );
	if( child_str->is_output_displayed ){
		display_output( child_str->command, child_str->child_stdout, child_str->child_stderr );
	}
	g_free( child_str->command );
	g_free( child_str );
}

static void
display_output( const gchar *command, int fd_stdout, int fd_stderr )
{
	GtkWidget *dialog;
	gchar *std_output, *std_error;

	dialog = gtk_message_dialog_new_with_markup(
			NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "<b>%s</b>", _( "Output of the run command" ));
	g_object_set( G_OBJECT( dialog ) , "title", PACKAGE_NAME, NULL );

	std_output = display_output_get_content( fd_stdout );
	std_error = display_output_get_content( fd_stderr );

	gtk_message_dialog_format_secondary_markup( GTK_MESSAGE_DIALOG( dialog ),
			"<b>%s</b>\n%s\n\n<b>%s</b>\n%s\n\n<b>%s</b>\n%s\n\n",
					_( "Run command:" ), command,
					_( "Standard output:" ), std_output,
					_( "Standard error:" ), std_error );

	gtk_dialog_run( GTK_DIALOG( dialog ));
	gtk_widget_destroy( dialog );

	g_free( std_output );
	g_free( std_error );
}

static gchar *
display_output_get_content( int fd )
{
	static const gchar *thisfn = "na_tokens_display_output_get_content";
	GInputStream *stream;
	GString *string;
	gchar buf[1024];
	GError *error;
	gchar *msg;

	string = g_string_new( "" );
	memset( buf, '\0', sizeof( buf ));

	if( fd > 0 ){
		stream = g_unix_input_stream_new( fd, TRUE );
		error = NULL;

		while( g_input_stream_read( stream, buf, sizeof( buf )-1, NULL, &error )){
			string = g_string_append( string, buf );
			memset( buf, '\0', sizeof( buf ));
		}
		if( error ){
			g_warning( "%s: g_input_stream_read: %s", thisfn, error->message );
			g_error_free( error );
		}
		g_input_stream_close( stream, NULL, NULL );
	}

	msg = g_locale_to_utf8( string->str, -1, NULL, NULL, NULL );
	g_string_free( string, TRUE );

	return( msg );
}

/*
 * Execution environment:
 * - Normal: just execute the specified command
 * - Terminal: use the user preference to have a terminal which stays openeded
 * - Embedded: id. Terminal
 * - DisplayOutput: execute in a shell
 */
static void
execute_action_command( gchar *command, const NAObjectProfile *profile, const NATokens *tokens )
{
	static const gchar *thisfn = "caja_actions_execute_action_command";
	GError *error;
	gchar *execution_mode, *run_command;
	gchar **argv;
	gint argc;
	gchar *wdir, *wdir_nq;
	GPid child_pid;
	ChildStr *child_str;

	g_debug( "%s: profile=%p", thisfn, ( void * ) profile );

	error = NULL;
	run_command = NULL;
	child_str = g_new0( ChildStr, 1 );
	child_pid = ( GPid ) 0;
	execution_mode = na_object_get_execution_mode( profile );

	if( !strcmp( execution_mode, "Normal" )){
		run_command = get_command_execution_normal( command );

	} else if( !strcmp( execution_mode, "Terminal" )){
		run_command = get_command_execution_terminal( command );

	} else if( !strcmp( execution_mode, "Embedded" )){
		run_command = get_command_execution_embedded( command );

	} else if( !strcmp( execution_mode, "DisplayOutput" )){
		child_str->is_output_displayed = TRUE;
		run_command = get_command_execution_display_output( command );

	} else {
		g_warning( "%s: unknown execution mode: %s", thisfn, execution_mode );
	}

	if( run_command ){
		child_str->command = g_strdup( run_command );

		if( !g_shell_parse_argv( run_command, &argc, &argv, &error )){
			g_warning( "%s: g_shell_parse_argv: %s", thisfn, error->message );
			g_error_free( error );

		} else {
			wdir = na_object_get_working_dir( profile );
			wdir_nq = parse_singular( tokens, wdir, 0, FALSE, FALSE );
			g_debug( "%s: run_command=%s, wdir=%s", thisfn, run_command, wdir_nq );

			/* it appears that at least mplayer does not support g_spawn_async_with_pipes
			 * (at least when not run in '-quiet' mode) while, e.g., totem and vlc rightly
			 * support this function
			 * So only use g_spawn_async_with_pipes when we really need to get back
			 * the content of output and error streams
			 * See https://bugzilla.gnome.org/show_bug.cgi?id=644289.
			 */
			if( child_str->is_output_displayed ){
				g_spawn_async_with_pipes(
						wdir_nq,
						argv,
						NULL,
						G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
						NULL,
						NULL,
						&child_pid,
						NULL,
						&child_str->child_stdout,
						&child_str->child_stderr,
						&error );

			} else {
				g_spawn_async(
						wdir_nq,
						argv,
						NULL,
						G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
						NULL,
						NULL,
						&child_pid,
						&error );
			}

			if( error ){
				g_warning( "%s: g_spawn_async: %s", thisfn, error->message );
				g_error_free( error );
				child_pid = ( GPid ) 0;

			} else {
				g_child_watch_add( child_pid, ( GChildWatchFunc ) child_watch_fn, child_str );
			}

			g_free( wdir );
			g_free( wdir_nq );
			g_strfreev( argv );
		}

		g_free( run_command );
	}

	g_free( execution_mode );

	if( child_pid == ( GPid ) 0 ){
		g_free( child_str->command );
		g_free( child_str );
	}
}

static gchar *
get_command_execution_display_output( const gchar *command )
{
	static const gchar *bin_sh = "/bin/sh -c COMMAND";
	return( na_tokens_command_for_terminal( bin_sh, command ));
}

static gchar *
get_command_execution_embedded( const gchar *command )
{
	return( get_command_execution_terminal( command ));
}

static gchar *
get_command_execution_normal( const gchar *command )
{
	return( g_strdup( command ));
}

static gchar *
get_command_execution_terminal( const gchar *command )
{
	gchar *run_command;
	gchar *pattern;

	pattern = na_settings_get_string( NA_IPREFS_TERMINAL_PATTERN, NULL, NULL );
	run_command = na_tokens_command_for_terminal( pattern, command );
	g_free( pattern );

	return( run_command );
}

/**
 * na_tokens_command_for_terminal:
 * @pattern: the command pattern; should include a 'COMMAND' keyword
 * @command: the command to be actually run in the terminal
 *
 * Returns: the command to be run, as a newly allocated string which should
 * be g_free() by the caller.
 */
gchar *
na_tokens_command_for_terminal( const gchar *pattern, const gchar *command )
{
	gchar *run_command;
	gchar *quoted;

	if( pattern && strlen( pattern )){
		quoted = g_shell_quote( command );
		run_command = na_core_utils_str_subst( pattern, "COMMAND", quoted );
		g_free( quoted );

	} else {
		run_command = g_strdup( command );
	}

	return( run_command );
}

/*
 * na_tokens_is_singular_exec:
 * @tokens: the current #NATokens object.
 * @exec: the to be executed command-line before having been parsed
 *
 * Returns: %TRUE if the first relevant parameter found in @exec
 * command-line is of singular form, %FALSE else.
 */
static gboolean
is_singular_exec( const NATokens *tokens, const gchar *exec )
{
	gboolean singular;
	gboolean found;
	gchar *iter;

	singular = FALSE;
	found = FALSE;
	iter = ( gchar * ) exec;

	while(( iter = g_strstr_len( iter, -1, "%" )) != NULL && !found ){

		switch( iter[1] ){
			case 'b':
			case 'd':
			case 'f':
			case 'm':
			case 'o':
			case 'u':
			case 'w':
			case 'x':
				found = TRUE;
				singular = TRUE;
				break;

			case 'B':
			case 'D':
			case 'F':
			case 'M':
			case 'O':
			case 'U':
			case 'W':
			case 'X':
				found = TRUE;
				singular = FALSE;
				break;

			/* all other parameters are irrelevant according to DES-EMA
			 * c: selection count
			 * h: hostname
			 * n: username
			 * p: port
			 * s: scheme
			 * %: %
			 */
		}

		iter += 2;			/* skip the % sign and the character after */
	}

	return( singular );
}

/*
 * parse_singular:
 * @tokens: a #NATokens object.
 * @input: the input string, may or may not contain tokens.
 * @i: the number of the iteration in a multiple selection, starting with zero.
 * @utf8: whether the @input string is UTF-8 encoded, or a standard ASCII
 *  string.
 * @quoted: whether the filenames have to be quoted (should be %TRUE when
 *  about to execute a command).
 *
 * A command is said of 'singular form' when its first parameter is not
 * of plural form. In the case of a multiple selection, singular form
 * commands are executed one time for each element of the selection
 *
 * Returns: a #GSList which contains two fields: the command and its parameters.
 * The returned #GSList should be na_core_utils_slist_free() by the caller.
 */
static gchar *
parse_singular( const NATokens *tokens, const gchar *input, guint i, gboolean utf8, gboolean quoted )
{
	GString *output;
	gchar *iter, *prev_iter;
	const gchar *nth;

	output = g_string_new( "" );

	/* return NULL if input is NULL
	 */
	if( !input ){
		return( g_string_free( output, TRUE ));
	}

	/* return an empty string if input is empty
	 */
	if( utf8 ){
		if( !g_utf8_strlen( input, -1 )){
			return( g_string_free( output, FALSE ));
		}
	} else {
		if( !strlen( input )){
			return( g_string_free( output, FALSE ));
		}
	}

	iter = ( gchar * ) input;
	prev_iter = iter;

	while(( iter = g_strstr_len( iter, -1, "%" ))){
		output = g_string_append_len( output, prev_iter, strlen( prev_iter ) - strlen( iter ));

		switch( iter[1] ){
			case 'b':
				if( tokens->private->basenames ){
					nth = ( const gchar * ) g_slist_nth_data( tokens->private->basenames, i );
					if( nth ){
						output = quote_string( output, nth, quoted );
					}
				}
				break;

			case 'B':
				if( tokens->private->basenames ){
					output = quote_string_list( output, tokens->private->basenames, quoted );
				}
				break;

			case 'c':
				g_string_append_printf( output, "%d", tokens->private->count );
				break;

			case 'd':
				if( tokens->private->basedirs ){
					nth = ( const gchar * ) g_slist_nth_data( tokens->private->basedirs, i );
					if( nth ){
						output = quote_string( output, nth, quoted );
					}
				}
				break;

			case 'D':
				if( tokens->private->basedirs ){
					output = quote_string_list( output, tokens->private->basedirs, quoted );
				}
				break;

			case 'f':
				if( tokens->private->filenames ){
					nth = ( const gchar * ) g_slist_nth_data( tokens->private->filenames, i );
					if( nth ){
						output = quote_string( output, nth, quoted );
					}
				}
				break;

			case 'F':
				if( tokens->private->filenames ){
					output = quote_string_list( output, tokens->private->filenames, quoted );
				}
				break;

			case 'h':
				if( tokens->private->hostname ){
					output = quote_string( output, tokens->private->hostname, quoted );
				}
				break;

			/* mimetypes are never quoted
			 */
			case 'm':
				if( tokens->private->mimetypes ){
					nth = ( const gchar * ) g_slist_nth_data( tokens->private->mimetypes, i );
					if( nth ){
						output = quote_string( output, nth, FALSE );
					}
				}
				break;

			case 'M':
				if( tokens->private->mimetypes ){
					output = quote_string_list( output, tokens->private->mimetypes, FALSE );
				}
				break;

			/* no-op operators */
			case 'o':
			case 'O':
				break;

			case 'n':
				if( tokens->private->username ){
					output = quote_string( output, tokens->private->username, quoted );
				}
				break;

			/* port number is never quoted
			 */
			case 'p':
				if( tokens->private->port > 0 ){
					g_string_append_printf( output, "%d", tokens->private->port );
				}
				break;

			case 's':
				if( tokens->private->scheme ){
					output = quote_string( output, tokens->private->scheme, quoted );
				}
				break;

			case 'u':
				if( tokens->private->uris ){
					nth = ( const gchar * ) g_slist_nth_data( tokens->private->uris, i );
					if( nth ){
						output = quote_string( output, nth, quoted );
					}
				}
				break;

			case 'U':
				if( tokens->private->uris ){
					output = quote_string_list( output, tokens->private->uris, quoted );
				}
				break;

			case 'w':
				if( tokens->private->basenames_woext ){
					nth = ( const gchar * ) g_slist_nth_data( tokens->private->basenames_woext, i );
					if( nth ){
						output = quote_string( output, nth, quoted );
					}
				}
				break;

			case 'W':
				if( tokens->private->basenames_woext ){
					output = quote_string_list( output, tokens->private->basenames_woext, quoted );
				}
				break;

			case 'x':
				if( tokens->private->exts ){
					nth = ( const gchar * ) g_slist_nth_data( tokens->private->exts, i );
					if( nth ){
						output = quote_string( output, nth, quoted );
					}
				}
				break;

			case 'X':
				if( tokens->private->exts ){
					output = quote_string_list( output, tokens->private->exts, quoted );
				}
				break;

			/* a percent sign
			 */
			case '%':
				output = g_string_append_c( output, '%' );
				break;
		}

		iter += 2;			/* skip the % sign and the character after */
		prev_iter = iter;	/* store the new start of the string */
	}

	output = g_string_append_len( output, prev_iter, strlen( prev_iter ));

	return( g_string_free( output, FALSE ));
}

static GString *
quote_string( GString *input, const gchar *name, gboolean quoted )
{
	gchar *tmp;

	if( quoted ){
		tmp = g_shell_quote( name );
		input = g_string_append( input, tmp );
		g_free( tmp );

	} else {
		input = g_string_append( input, name );
	}

	return( input );
}

static GString *
quote_string_list( GString *input, GSList *names, gboolean quoted )
{
	GSList *it;
	gchar *tmp;

	if( quoted ){
		GSList *quoted_names = NULL;
		for( it = names ; it ; it = it->next ){
			quoted_names = g_slist_append( quoted_names, g_shell_quote(( const gchar * ) it->data ));
		}
		tmp = na_core_utils_slist_join_at_end( quoted_names, " " );
		na_core_utils_slist_free( quoted_names );

	} else {
		tmp = na_core_utils_slist_join_at_end( g_slist_reverse( names ), " " );
	}

	input = g_string_append( input, tmp );
	g_free( tmp );

	return( input );
}
