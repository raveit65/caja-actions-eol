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

#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <api/na-boxed.h>
#include <api/na-data-types.h>
#include <api/na-core-utils.h>
#include <api/na-timeout.h>

#include "na-settings.h"
#include "na-marshal.h"

#define NA_SETTINGS_TYPE                ( settings_get_type())
#define NA_SETTINGS( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_SETTINGS_TYPE, NASettings ))
#define NA_SETTINGS_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_SETTINGS_TYPE, NASettingsClass ))
#define NA_IS_SETTINGS( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_SETTINGS_TYPE ))
#define NA_IS_SETTINGS_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_SETTINGS_TYPE ))
#define NA_SETTINGS_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_SETTINGS_TYPE, NASettingsClass ))

typedef struct _NASettingsPrivate       NASettingsPrivate;

typedef struct {
	/*< private >*/
	GObject            parent;
	NASettingsPrivate *private;
}
	NASettings;

typedef struct _NASettingsClassPrivate  NASettingsClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass            parent;
	NASettingsClassPrivate *private;
}
	NASettingsClass;

/* private class data
 */
struct _NASettingsClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* The characteristics of a configuration file.
 * We manage two configuration files:
 * - the global configuration file handles mandatory preferences;
 * - the user configuration file handles.. well, user preferences.
 */
typedef struct {
	gchar        *fname;
	gboolean      mandatory;
	GKeyFile     *key_file;
	GFileMonitor *monitor;
	gulong        handler;
}
	KeyFile;

/* Each consumer may register a callback function which will be triggered
 * when a key is modified.
 *
 * The monitored key usually is the real key read in the file;
 * as a special case, composite keys are defined:
 * - NA_IPREFS_IO_PROVIDERS_READ_STATUS monitors the 'readable' key for all i/o providers
 *
 * Note that we actually monitor the _user_view_ of the configuration:
 * e.g. if a key has a mandatory value in global conf, then the same
 * key in user conf will just be ignored.
 */
typedef struct {
	gchar    *monitored_key;
	GCallback callback;
	gpointer  user_data;
}
	Consumer;

/* private instance data
 */
struct _NASettingsPrivate {
	gboolean  dispose_has_run;
	KeyFile  *mandatory;
	KeyFile  *user;
	GList    *content;
	GList    *consumers;
	NATimeout timeout;
};

#define GROUP_CACT						"cact"
#define GROUP_RUNTIME					"runtime"

typedef struct {
	const gchar *key;
	const gchar *group;
	guint        type;
	const gchar *default_value;
}
	KeyDef;

static const KeyDef st_def_keys[] = {
	{ NA_IPREFS_ADMIN_PREFERENCES_LOCKED,         GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_ADMIN_IO_PROVIDERS_LOCKED,        GROUP_RUNTIME, NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_ASSISTANT_ESC_CONFIRM,            GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "true" },
	{ NA_IPREFS_ASSISTANT_ESC_QUIT,               GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "true" },
	{ NA_IPREFS_CAPABILITY_ADD_CAPABILITY_WSP,    GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_COMMAND_CHOOSER_WSP,              GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_COMMAND_CHOOSER_URI,              GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///bin" },
	{ NA_IPREFS_COMMAND_LEGEND_WSP,               GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_CONFIRM_LOGOUT_WSP,               GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_DESKTOP_ENVIRONMENT,              GROUP_RUNTIME, NA_DATA_TYPE_STRING,      "" },
	{ NA_IPREFS_WORKING_DIR_WSP,                  GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_WORKING_DIR_URI,                  GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///" },
	{ NA_IPREFS_SHOW_IF_RUNNING_WSP,              GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_SHOW_IF_RUNNING_URI,              GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///bin" },
	{ NA_IPREFS_TRY_EXEC_WSP,                     GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_TRY_EXEC_URI,                     GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///bin" },
	{ NA_IPREFS_EXPORT_ASK_USER_WSP,              GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_EXPORT_ASK_USER_LAST_FORMAT,      GROUP_CACT,    NA_DATA_TYPE_STRING,      "Desktop1" },
	{ NA_IPREFS_EXPORT_ASK_USER_KEEP_LAST_CHOICE, GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_EXPORT_ASSISTANT_WSP,             GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_EXPORT_ASSISTANT_URI,             GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///tmp" },
	{ NA_IPREFS_EXPORT_ASSISTANT_PANED,           GROUP_CACT,    NA_DATA_TYPE_UINT,        "200" },
	{ NA_IPREFS_EXPORT_PREFERRED_FORMAT,          GROUP_CACT,    NA_DATA_TYPE_STRING,      "Ask" },
	{ NA_IPREFS_FOLDER_CHOOSER_WSP,               GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_FOLDER_CHOOSER_URI,               GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///" },
	{ NA_IPREFS_IMPORT_ASK_USER_WSP,              GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_IMPORT_ASK_USER_LAST_MODE,        GROUP_CACT,    NA_DATA_TYPE_STRING,      "NoImport" },
	{ NA_IPREFS_IMPORT_ASSISTANT_WSP,             GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_IMPORT_ASSISTANT_URI,             GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///tmp" },
	{ NA_IPREFS_IMPORT_ASK_USER_KEEP_LAST_CHOICE, GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_IMPORT_PREFERRED_MODE,            GROUP_CACT,    NA_DATA_TYPE_STRING,      "Ask" },
	{ NA_IPREFS_IO_PROVIDERS_WRITE_ORDER,         GROUP_CACT,    NA_DATA_TYPE_STRING_LIST, "" },
	{ NA_IPREFS_ICON_CHOOSER_URI,                 GROUP_CACT,    NA_DATA_TYPE_STRING,      "file:///" },
	{ NA_IPREFS_ICON_CHOOSER_PANED,               GROUP_CACT,    NA_DATA_TYPE_UINT,        "200" },
	{ NA_IPREFS_ICON_CHOOSER_WSP,                 GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_ITEMS_ADD_ABOUT_ITEM,             GROUP_RUNTIME, NA_DATA_TYPE_BOOLEAN,     "true" },
	{ NA_IPREFS_ITEMS_CREATE_ROOT_MENU,           GROUP_RUNTIME, NA_DATA_TYPE_BOOLEAN,     "true" },
	{ NA_IPREFS_ITEMS_LEVEL_ZERO_ORDER,           GROUP_RUNTIME, NA_DATA_TYPE_STRING_LIST, "" },
	{ NA_IPREFS_ITEMS_LIST_ORDER_MODE,            GROUP_RUNTIME, NA_DATA_TYPE_STRING,      "AscendingOrder" },
	{ NA_IPREFS_MAIN_PANED,                       GROUP_CACT,    NA_DATA_TYPE_UINT,        "200" },
	{ NA_IPREFS_MAIN_SAVE_AUTO,                   GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_MAIN_SAVE_PERIOD,                 GROUP_CACT,    NA_DATA_TYPE_UINT,        "5" },
	{ NA_IPREFS_MAIN_TABS_POS,                    GROUP_CACT,    NA_DATA_TYPE_STRING,      "Top" },
	{ NA_IPREFS_MAIN_TOOLBAR_EDIT_DISPLAY,        GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "true" },
	{ NA_IPREFS_MAIN_TOOLBAR_FILE_DISPLAY,        GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "true" },
	{ NA_IPREFS_MAIN_TOOLBAR_HELP_DISPLAY,        GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "true" },
	{ NA_IPREFS_MAIN_TOOLBAR_TOOLS_DISPLAY,       GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_MAIN_WINDOW_WSP,                  GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_PREFERENCES_WSP,                  GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_PLUGIN_MENU_LOG,                  GROUP_RUNTIME, NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_RELABEL_DUPLICATE_ACTION,         GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_RELABEL_DUPLICATE_MENU,           GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_RELABEL_DUPLICATE_PROFILE,        GROUP_CACT,    NA_DATA_TYPE_BOOLEAN,     "false" },
	{ NA_IPREFS_SCHEME_ADD_SCHEME_WSP,            GROUP_CACT,    NA_DATA_TYPE_UINT_LIST,   "" },
	{ NA_IPREFS_SCHEME_DEFAULT_LIST,              GROUP_CACT,    NA_DATA_TYPE_STRING_LIST, "" },
	{ NA_IPREFS_TERMINAL_PATTERN,                 GROUP_RUNTIME, NA_DATA_TYPE_STRING,      "" },
	{ NA_IPREFS_IO_PROVIDER_READABLE,             NA_IPREFS_IO_PROVIDER_GROUP, NA_DATA_TYPE_BOOLEAN, "true" },
	{ NA_IPREFS_IO_PROVIDER_WRITABLE,             NA_IPREFS_IO_PROVIDER_GROUP, NA_DATA_TYPE_BOOLEAN, "true" },
	{ 0 }
};

/* The configuration content is handled as a GList of KeyValue structs.
 * This list is loaded at initialization time, and then compared each
 * time our file monitors signal us that a change has occured.
 */
typedef struct {
	const KeyDef *def;
	const gchar  *group;
	gboolean      mandatory;
	NABoxed      *boxed;
}
	KeyValue;

/* signals
 */
enum {
	KEY_CHANGED,
	LAST_SIGNAL
};

static GObjectClass *st_parent_class           = NULL;
static gint          st_burst_timeout          = 100;		/* burst timeout in msec */
static gint          st_signals[ LAST_SIGNAL ] = { 0 };
static NASettings   *st_settings               = NULL;

static GType     settings_get_type( void );
static GType     register_type( void );
static void      class_init( NASettingsClass *klass );
static void      instance_init( GTypeInstance *instance, gpointer klass );
static void      instance_dispose( GObject *object );
static void      instance_finalize( GObject *object );

static void      settings_new( void );

static GList    *content_diff( GList *old, GList *new );
static GList    *content_load_keys( GList *content, KeyFile *keyfile );
static KeyDef   *get_key_def( const gchar *key );
static KeyFile  *key_file_new( const gchar *dir );
static void      on_keyfile_changed( GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type );
static void      on_keyfile_changed_timeout( void );
static void      on_key_changed_final_handler( NASettings *settings, gchar *group, gchar *key, NABoxed *new_value, gboolean mandatory );
static KeyValue *peek_key_value_from_content( GList *content, const gchar *group, const gchar *key );
static KeyValue *read_key_value( const gchar *group, const gchar *key, gboolean *found, gboolean *mandatory );
static KeyValue *read_key_value_from_key_file( KeyFile *keyfile, const gchar *group, const gchar *key, const KeyDef *key_def );
static void      release_consumer( Consumer *consumer );
static void      release_key_file( KeyFile *key_file );
static void      release_key_value( KeyValue *value );
static gboolean  set_key_value( const gchar *group, const gchar *key, const gchar *string );
static gboolean  write_user_key_file( void );

static GType
settings_get_type( void )
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
	static const gchar *thisfn = "na_settings_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NASettingsClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NASettings ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NASettings", &info, 0 );

	return( type );
}

static void
class_init( NASettingsClass *klass )
{
	static const gchar *thisfn = "na_settings_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NASettingsClassPrivate, 1 );

	/*
	 * NASettings::settings-key-changed:
	 *
	 * This signal is sent by NASettings when the value of a key is modified.
	 *
	 * Arguments are the group, the key, the new value as a NABoxed,
	 * and whether it is mandatory.
	 *
	 * Handler is of type:
	 * void ( *handler )( NASettings *settings,
	 * 						const gchar *group,
	 * 						const gchar *key,
	 * 						NABoxed *value,
	 * 						gboolean mandatory,
	 * 						gpointer user_data );
	 *
	 * The default class handler frees these datas.
	 */
	st_signals[ KEY_CHANGED ] = g_signal_new_class_handler(
				SETTINGS_SIGNAL_KEY_CHANGED,
				NA_SETTINGS_TYPE,
				G_SIGNAL_RUN_CLEANUP | G_SIGNAL_ACTION,
				G_CALLBACK( on_key_changed_final_handler ),
				NULL,								/* accumulator */
				NULL,								/* accumulator data */
				na_cclosure_marshal_VOID__STRING_STRING_POINTER_BOOLEAN,
				G_TYPE_NONE,
				4,
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_BOOLEAN );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_settings_instance_init";
	NASettings *self;

	g_return_if_fail( NA_IS_SETTINGS( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = NA_SETTINGS( instance );

	self->private = g_new0( NASettingsPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->mandatory = NULL;
	self->private->user = NULL;
	self->private->content = NULL;
	self->private->consumers = NULL;

	self->private->timeout.timeout = st_burst_timeout;
	self->private->timeout.handler = ( NATimeoutFunc ) on_keyfile_changed_timeout;
	self->private->timeout.user_data = NULL;
	self->private->timeout.source_id = 0;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_settings_instance_dispose";
	NASettings *self;

	g_return_if_fail( NA_IS_SETTINGS( object ));

	self = NA_SETTINGS( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

		self->private->dispose_has_run = TRUE;

		release_key_file( self->private->mandatory );
		release_key_file( self->private->user );

		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_settings_instance_finalize";
	NASettings *self;

	g_return_if_fail( NA_IS_SETTINGS( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = NA_SETTINGS( object );

	g_list_foreach( self->private->content, ( GFunc ) release_key_value, NULL );
	g_list_free( self->private->content );

	g_list_foreach( self->private->consumers, ( GFunc ) release_consumer, NULL );
	g_list_free( self->private->consumers );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * na_settings_new:
 *
 * Allocates a new #NASettings object which should be na_settings_free()
 * by the caller.
 */
static void
settings_new( void )
{
	static const gchar *thisfn = "na_settings_new";
	gchar *dir;
	GList *content;

	if( !st_settings ){
		st_settings = g_object_new( NA_SETTINGS_TYPE, NULL );

		g_debug( "%s: reading mandatory configuration", thisfn );
		dir = g_build_filename( SYSCONFDIR, "xdg", PACKAGE, NULL );
		st_settings->private->mandatory = key_file_new( dir );
		g_free( dir );
		st_settings->private->mandatory->mandatory = TRUE;
		content = content_load_keys( NULL, st_settings->private->mandatory );

		g_debug( "%s: reading user configuration", thisfn );
		dir = g_build_filename( g_get_home_dir(), ".config", PACKAGE, NULL );
		g_mkdir_with_parents( dir, 0750 );
		st_settings->private->user = key_file_new( dir );
		g_free( dir );
		st_settings->private->mandatory->mandatory = FALSE;
		content = content_load_keys( content, st_settings->private->user );

		st_settings->private->content = g_list_copy( content );
		g_list_free( content );
	}
}

/**
 * na_settings_free:
 */
void
na_settings_free( void )
{
	if( st_settings ){
		g_object_unref( st_settings );
		st_settings = NULL;
	}
}

/**
 * na_settings_register_key_callback:
 * @key: the key to be monitored.
 * @callback: the function to be called when the value of the key changes.
 * @user_data: data to be passed to the @callback function.
 *
 * Registers a new consumer of the monitoring of the @key.
 *
 * Since: 3.1
 */
void
na_settings_register_key_callback( const gchar *key, GCallback callback, gpointer user_data )
{
	static const gchar *thisfn = "na_settings_register_key_callback";

	g_debug( "%s: key=%s, callback=%p, user_data=%p",
			thisfn, key, ( void * ) callback, ( void * ) user_data );

	Consumer *consumer = g_new0( Consumer, 1 );
	consumer->monitored_key = g_strdup( key );
	consumer->callback = callback;
	consumer->user_data = user_data;

	settings_new();
	st_settings->private->consumers = g_list_prepend( st_settings->private->consumers, consumer );
}

/**
 * na_settings_get_boolean:
 * @key: the key whose value is to be returned.
 * @found: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the searched @key has been found (%TRUE), or if the returned
 *  value comes from default (%FALSE).
 * @mandatory: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the returned value has been read from mandatory preferences
 *  (%TRUE), or from the user preferences (%FALSE). When the @key has not
 *  been found, @mandatory is set to %FALSE.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: the value of the key, of its default value if not found.
 *
 * Since: 3.1
 */
gboolean
na_settings_get_boolean( const gchar *key, gboolean *found, gboolean *mandatory )
{
	return( na_settings_get_boolean_ex( NULL, key, found, mandatory ));
}

/**
 * na_settings_get_boolean_ex:
 * @group: the group where the @key is to be searched for. May be %NULL.
 * @key: the key whose value is to be returned.
 * @found: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the searched @key has been found (%TRUE), or if the returned
 *  value comes from default (%FALSE).
 * @mandatory: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the returned value has been read from mandatory preferences
 *  (%TRUE), or from the user preferences (%FALSE). When the @key has not
 *  been found, @mandatory is set to %FALSE.
 *
 * Returns: the value of the key, of its default value if not found.
 *
 * Since: 3.1
 */
gboolean
na_settings_get_boolean_ex( const gchar *group, const gchar *key, gboolean *found, gboolean *mandatory )
{
	gboolean value;
	KeyValue *key_value;
	KeyDef *key_def;

	value = FALSE;
	key_value = read_key_value( group, key, found, mandatory );

	if( key_value ){
		value = na_boxed_get_boolean( key_value->boxed );
		release_key_value( key_value );

	} else {
		key_def = get_key_def( key );
		if( key_def ){
			value = ( key_def->default_value ? ( strcasecmp( key_def->default_value, "true" ) == 0 || atoi( key_def->default_value ) != 0 ) : FALSE );
		}
	}

	return( value );
}

/**
 * na_settings_get_string:
 * @key: the key whose value is to be returned.
 * @found: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the searched @key has been found (%TRUE), or if the returned
 *  value comes from default (%FALSE).
 * @mandatory: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the returned value has been read from mandatory preferences
 *  (%TRUE), or from the user preferences (%FALSE). When the @key has not
 *  been found, @mandatory is set to %FALSE.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: the value of the key as a newly allocated string, which should
 * be g_free() by the caller.
 *
 * Since: 3.1
 */
gchar *
na_settings_get_string( const gchar *key, gboolean *found, gboolean *mandatory )
{
	gchar *value;
	KeyValue *key_value;
	KeyDef *key_def;

	value = NULL;
	key_value = read_key_value( NULL, key, found, mandatory );

	if( key_value ){
		value = na_boxed_get_string( key_value->boxed );
		release_key_value( key_value );

	} else {
		key_def = get_key_def( key );
		if( key_def && key_def->default_value ){
			value = g_strdup( key_def->default_value );
		}
	}

	return( value );
}

/**
 * na_settings_get_string_list:
 * @key: the key whose value is to be returned.
 * @found: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the searched @key has been found (%TRUE), or if the returned
 *  value comes from default (%FALSE).
 * @mandatory: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the returned value has been read from mandatory preferences
 *  (%TRUE), or from the user preferences (%FALSE). When the @key has not
 *  been found, @mandatory is set to %FALSE.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: the value of the key as a newly allocated list of strings.
 * The returned list should be na_core_utils_slist_free() by the caller.
 *
 * Since: 3.1
 */
GSList *
na_settings_get_string_list( const gchar *key, gboolean *found, gboolean *mandatory )
{
	GSList *value;
	KeyValue *key_value;
	KeyDef *key_def;

	value = NULL;
	key_value = read_key_value( NULL, key, found, mandatory );

	if( key_value ){
		value = na_boxed_get_string_list( key_value->boxed );
		release_key_value( key_value );

	} else {
		key_def = get_key_def( key );
		if( key_def && key_def->default_value && strlen( key_def->default_value )){
			value = g_slist_append( NULL, g_strdup( key_def->default_value ));
		}
	}

	return( value );
}

/**
 * na_settings_get_uint:
 * @key: the key whose value is to be returned.
 * @found: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the searched @key has been found (%TRUE), or if the returned
 *  value comes from default (%FALSE).
 * @mandatory: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the returned value has been read from mandatory preferences
 *  (%TRUE), or from the user preferences (%FALSE). When the @key has not
 *  been found, @mandatory is set to %FALSE.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: the value of the key.
 *
 * Since: 3.1
 */
guint
na_settings_get_uint( const gchar *key, gboolean *found, gboolean *mandatory )
{
	guint value;
	KeyDef *key_def;
	KeyValue *key_value;

	value = 0;
	key_value = read_key_value( NULL, key, found, mandatory );

	if( key_value ){
		value = na_boxed_get_uint( key_value->boxed );
		release_key_value( key_value );

	} else {
		key_def = get_key_def( key );
		if( key_def && key_def->default_value ){
			value = atoi( key_def->default_value );
		}
	}

	return( value );
}

/**
 * na_settings_get_uint_list:
 * @key: the key whose value is to be returned.
 * @found: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the searched @key has been found (%TRUE), or if the returned
 *  value comes from default (%FALSE).
 * @mandatory: if not %NULL, a pointer to a gboolean in which we will store
 *  whether the returned value has been read from mandatory preferences
 *  (%TRUE), or from the user preferences (%FALSE). When the @key has not
 *  been found, @mandatory is set to %FALSE.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: the value of the key as a newly allocated list of uints.
 * The returned list should be g_list_free() by the caller.
 *
 * Since: 3.1
 */
GList *
na_settings_get_uint_list( const gchar *key, gboolean *found, gboolean *mandatory )
{
	GList *value;
	KeyDef *key_def;
	KeyValue *key_value;

	value = NULL;
	key_value = read_key_value( NULL, key, found, mandatory );

	if( key_value ){
		value = na_boxed_get_uint_list( key_value->boxed );
		release_key_value( key_value );

	} else {
		key_def = get_key_def( key );
		if( key_def && key_def->default_value ){
			value = g_list_append( NULL, GUINT_TO_POINTER( atoi( key_def->default_value )));
		}
	}

	return( value );
}

/**
 * na_settings_set_boolean:
 * @key: the key whose value is to be returned.
 * @value: the boolean to be written.
 *
 * This function writes @value as a user preference.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.1
 */
gboolean
na_settings_set_boolean( const gchar *key, gboolean value )
{
	gchar *string;
	gboolean ok;

	string = g_strdup_printf( "%s", value ? "true" : "false" );
	ok = set_key_value( NULL, key, string );
	g_free( string );

	return( ok );
}

/**
 * na_settings_set_boolean_ex:
 * @group: the group in the keyed file;
 * @key: the key whose value is to be returned.
 * @value: the boolean to be written.
 *
 * This function writes @value as a user preference.
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.1
 */
gboolean
na_settings_set_boolean_ex( const gchar *group, const gchar *key, gboolean value )
{
	gchar *string;
	gboolean ok;

	string = g_strdup_printf( "%s", value ? "true" : "false" );
	ok = set_key_value( group, key, string );
	g_free( string );

	return( ok );
}

/**
 * na_settings_set_string:
 * @key: the key whose value is to be returned.
 * @value: the string to be written.
 *
 * This function writes @value as a user preference.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.1
 */
gboolean
na_settings_set_string( const gchar *key, const gchar *value )
{
	return( set_key_value( NULL, key, value ));
}

/**
 * na_settings_set_string_ex:
 * @group: the group in the keyed file;
 * @key: the key whose value is to be returned.
 * @value: the string to be written.
 *
 * This function writes @value as a user preference.
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.2
 */
gboolean
na_settings_set_string_ex( const gchar *group, const gchar *key, const gchar *value )
{
	return( set_key_value( group, key, value ));
}

/**
 * na_settings_set_string_list:
 * @key: the key whose value is to be returned.
 * @value: the list of strings to be written.
 *
 * This function writes @value as a user preference.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.1
 */
gboolean
na_settings_set_string_list( const gchar *key, const GSList *value )
{
	GString *string;
	const GSList *it;
	gboolean ok;

	string = g_string_new( "" );
	for( it = value ; it ; it = it->next ){
		g_string_append_printf( string, "%s;", ( const gchar * ) it->data );
	}
	ok = set_key_value( NULL, key, string->str );
	g_string_free( string, TRUE );

	return( ok );
}

/**
 * na_settings_set_int_ex:
 * @group: the group in the keyed file;
 * @key: the key whose value is to be returned.
 * @value: the unsigned integer to be written.
 *
 * This function writes @value as a user preference.
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.2
 */
gboolean
na_settings_set_int_ex( const gchar *group, const gchar *key, int value )
{
	gchar *string;
	gboolean ok;

	string = g_strdup_printf( "%d", value );
	ok = set_key_value( group, key, string );
	g_free( string );

	return( ok );
}

/**
 * na_settings_set_uint:
 * @key: the key whose value is to be returned.
 * @value: the unsigned integer to be written.
 *
 * This function writes @value as a user preference.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.1
 */
gboolean
na_settings_set_uint( const gchar *key, guint value )
{
	gchar *string;
	gboolean ok;

	string = g_strdup_printf( "%u", value );
	ok = set_key_value( NULL, key, string );
	g_free( string );

	return( ok );
}

/**
 * na_settings_set_uint_list:
 * @key: the key whose value is to be returned.
 * @value: the list of unsigned integers to be written.
 *
 * This function writes @value as a user preference.
 *
 * This function should only be called for unambiguous keys; the resultat
 * is otherwise undefined (and rather unpredictable).
 *
 * Returns: %TRUE is the writing has been successful, %FALSE else.
 *
 * Since: 3.1
 */
gboolean
na_settings_set_uint_list( const gchar *key, const GList *value )
{
	GString *string;
	const GList *it;
	gboolean ok;

	string = g_string_new( "" );
	for( it = value ; it ; it = it->next ){
		g_string_append_printf( string, "%u;", GPOINTER_TO_UINT( it->data ));
	}
	ok = set_key_value( NULL, key, string->str );
	g_string_free( string, TRUE );

	return( ok );
}

/**
 * na_settings_get_groups:
 *
 * Returns: the list of groups in the configuration; this list should be
 * na_core_utils_slist_free() by the caller.
 *
 * This function participates to a rather bad hack to obtain the list of
 * known i/o providers from preferences. We do not care of returning unique
 * or sorted group names.
 *
 * Since: 3.1
 */
GSList *
na_settings_get_groups( void )
{
	GSList *groups;
	gchar **array;

	groups = NULL;
	settings_new();

	array = g_key_file_get_groups( st_settings->private->mandatory->key_file, NULL );
	if( array ){
		groups = na_core_utils_slist_from_array(( const gchar ** ) array );
		g_strfreev( array );
	}

	array = g_key_file_get_groups( st_settings->private->user->key_file, NULL );
	if( array ){
		groups = g_slist_concat( groups, na_core_utils_slist_from_array(( const gchar ** ) array ));
		g_strfreev( array );
	}

	return( groups );
}

/*
 * returns a list of modified KeyValue
 * - order in the lists is not signifiant
 * - the mandatory flag is not signifiant
 * - a key is modified:
 *   > if it appears in new
 *   > if it disappears: the value is so reset to its default
 *   > if the value has been modified
 *
 * we return here a new list, with newly allocated KeyValue structs
 * which hold the new value of each modified key
 */
static GList *
content_diff( GList *old, GList *new )
{
	GList *diffs, *io, *in;
	KeyValue *kold, *knew, *kdiff;
	gboolean found;

	diffs = NULL;

	for( io = old ; io ; io = io->next ){
		kold = ( KeyValue * ) io->data;
		found = FALSE;
		for( in = new ; in && !found ; in = in->next ){
			knew = ( KeyValue * ) in->data;
			if( !strcmp( kold->group, knew->group ) && ( gpointer ) kold->def == ( gpointer ) knew->def ){
				found = TRUE;
				if( !na_boxed_are_equal( kold->boxed, knew->boxed )){
					/* a key has been modified */
					kdiff = g_new0( KeyValue, 1 );
					kdiff->group = g_strdup( knew->group );
					kdiff->def = knew->def;
					kdiff->mandatory = knew->mandatory;
					kdiff->boxed = na_boxed_copy( knew->boxed );
					diffs = g_list_prepend( diffs, kdiff );
				}
			}
		}
		if( !found ){
			/* a key has disappeared */
			kdiff = g_new0( KeyValue, 1 );
			kdiff->group = g_strdup( kold->group );
			kdiff->def = kold->def;
			kdiff->mandatory = FALSE;
			kdiff->boxed = na_boxed_new_from_string( kold->def->type, kold->def->default_value );
			diffs = g_list_prepend( diffs, kdiff );
		}
	}

	for( in = new ; in ; in = in->next ){
		knew = ( KeyValue * ) in->data;
		found = FALSE;
		for( io = old ; io && !found ; io = io->next ){
			kold = ( KeyValue * ) io->data;
			if( !strcmp( kold->group, knew->group ) && ( gpointer ) kold->def == ( gpointer ) knew->def ){
				found = TRUE;
			}
		}
		if( !found ){
			/* a key is new */
			kdiff = g_new0( KeyValue, 1 );
			kdiff->group = g_strdup( knew->group );
			kdiff->def = knew->def;
			kdiff->mandatory = knew->mandatory;
			kdiff->boxed = na_boxed_copy( knew->boxed );
			diffs = g_list_prepend( diffs, kdiff );
		}
	}

	return( diffs );
}

/* add the content of a configuration files to those already loaded
 *
 * when the two configuration files have been read, then the content of
 * _the_ configuration has been loaded, while preserving the mandatory
 * keys
 */
static GList *
content_load_keys( GList *content, KeyFile *keyfile )
{
	static const gchar *thisfn = "na_settings_content_load_keys";
	GError *error;
	gchar **groups, **ig;
	gchar **keys, **ik;
	KeyValue *key_value;
	KeyDef *key_def;

	error = NULL;
	if( !g_key_file_load_from_file( keyfile->key_file, keyfile->fname, G_KEY_FILE_KEEP_COMMENTS, &error )){
		if( error->code != G_FILE_ERROR_NOENT ){
			g_warning( "%s: %s (%d) %s", thisfn, keyfile->fname, error->code, error->message );
		} else {
			g_debug( "%s: %s: file doesn't exist", thisfn, keyfile->fname );
		}
		g_error_free( error );
		error = NULL;

	} else {
		groups = g_key_file_get_groups( keyfile->key_file, NULL );
		ig = groups;
		while( *ig ){
			keys = g_key_file_get_keys( keyfile->key_file, *ig, NULL, NULL );
			ik = keys;
			while( *ik ){
				key_def = get_key_def( *ik );
				if( key_def ){
					key_value = peek_key_value_from_content( content, *ig, *ik );
					if( !key_value ){
						key_value = read_key_value_from_key_file( keyfile, *ig, *ik, key_def );
						if( key_value ){
							key_value->mandatory = keyfile->mandatory;
							content = g_list_prepend( content, key_value );
						}
					}
				}
				ik++;
			}
			g_strfreev( keys );
			ig++;
		}
		g_strfreev( groups );
	}

	return( content );
}

static KeyDef *
get_key_def( const gchar *key )
{
	static const gchar *thisfn = "na_settings_get_key_def";
	KeyDef *found = NULL;
	KeyDef *idef;

	idef = ( KeyDef * ) st_def_keys;
	while( idef->key && !found ){
		if( !strcmp( idef->key, key )){
			found = idef;
		}
		idef++;
	}
	if( !found ){
		g_warning( "%s: no KeyDef found for key=%s", thisfn, key );
	}

	return( found );
}

/*
 * called from na_settings_new
 * allocate and load the key files for global and user preferences
 */
static KeyFile *
key_file_new( const gchar *dir )
{
	static const gchar *thisfn = "na_settings_key_file_new";
	KeyFile *keyfile;
	GError *error;
	GFile *file;

	keyfile = g_new0( KeyFile, 1 );

	keyfile->key_file = g_key_file_new();
	keyfile->fname = g_strdup_printf( "%s/%s.conf", dir, PACKAGE );
	na_core_utils_file_list_perms( keyfile->fname, thisfn );

	error = NULL;
	file = g_file_new_for_path( keyfile->fname );
	keyfile->monitor = g_file_monitor_file( file, 0, NULL, &error );
	if( error ){
		g_warning( "%s: %s: %s", thisfn, keyfile->fname, error->message );
		g_error_free( error );
		error = NULL;
	} else {
		keyfile->handler = g_signal_connect( keyfile->monitor, "changed", ( GCallback ) on_keyfile_changed, NULL );
	}
	g_object_unref( file );

	return( keyfile );
}

/*
 * one of the two monitored configuration files have changed on the disk
 * we do not try to identify which keys have actually change
 * instead we trigger each registered consumer for the 'global' event
 *
 * consumers which register for the 'global_conf' event are recorded
 * with a NULL key
 */
static void
on_keyfile_changed( GFileMonitor *monitor,
		GFile *file, GFile *other_file, GFileMonitorEvent event_type )
{
	settings_new();
	na_timeout_event( &st_settings->private->timeout );
}

static void
on_keyfile_changed_timeout( void )
{
	static const gchar *thisfn = "na_settings_on_keyfile_changed_timeout";
	GList *new_content;
	GList *modifs;
	GList *ic, *im;
	const KeyValue *changed;
	const Consumer *consumer;
	gchar *group_prefix, *key;
#ifdef NA_MAINTAINER_MODE
	gchar *value;
#endif

	/* last individual notification is older that the st_burst_timeout
	 * we may so suppose that the burst is terminated
	 */
	new_content = content_load_keys( NULL, st_settings->private->mandatory );
	new_content = content_load_keys( new_content, st_settings->private->user );
	modifs = content_diff( st_settings->private->content, new_content );

#ifdef NA_MAINTAINER_MODE
	g_debug( "%s: %d found update(s)", thisfn, g_list_length( modifs ));
	for( im = modifs ; im ; im = im->next ){
		changed = ( const KeyValue * ) im->data;
		value = na_boxed_get_string( changed->boxed );
		g_debug( "%s: group=%s, key=%s, value=%s", thisfn, changed->group, changed->def->key, value );
		g_free( value );
	}
#endif

	/* for each modification found,
	 * - check if a consumer has registered for this key, and triggers callback if apply
	 * - send a notification message
	 */
	for( im = modifs ; im ; im = im->next ){
		changed = ( const KeyValue * ) im->data;

		for( ic = st_settings->private->consumers ; ic ; ic = ic->next ){
			consumer = ( const Consumer * ) ic->data;
			group_prefix = NULL;

			if( !strcmp( consumer->monitored_key, NA_IPREFS_IO_PROVIDERS_READ_STATUS )){
				group_prefix = g_strdup_printf( "%s ", NA_IPREFS_IO_PROVIDER_GROUP );
				key = NA_IPREFS_IO_PROVIDER_READABLE;
			} else {
				key = consumer->monitored_key;
			}

			if(( !group_prefix || g_str_has_prefix( changed->group, group_prefix )) && !strcmp( changed->def->key, key )){
				( *( NASettingsKeyCallback ) consumer->callback )(
						changed->group,
						changed->def->key,
						na_boxed_get_pointer( changed->boxed ),
						changed->mandatory,
						consumer->user_data );
			}

			g_free( group_prefix );
		}

		g_debug( "%s: sending signal for group=%s, key=%s", thisfn, changed->group, changed->def->key );
		g_signal_emit_by_name( st_settings,
				SETTINGS_SIGNAL_KEY_CHANGED,
				changed->group, changed->def->key, changed->boxed, changed->mandatory );
	}

	g_debug( "%s: releasing content", thisfn );
	g_list_foreach( st_settings->private->content, ( GFunc ) release_key_value, NULL );
	g_list_free( st_settings->private->content );
	st_settings->private->content = new_content;

	g_debug( "%s: releasing modifs", thisfn );
	g_list_foreach( modifs, ( GFunc ) release_key_value, NULL );
	g_list_free( modifs );
}

static void
on_key_changed_final_handler( NASettings *settings, gchar *group, gchar *key, NABoxed *new_value, gboolean mandatory )
{
	g_debug( "na_settings_on_key_changed_final_handler: group=%s, key=%s", group, key );
	na_boxed_dump( new_value );
}

static KeyValue *
peek_key_value_from_content( GList *content, const gchar *group, const gchar *key )
{
	KeyValue *value, *found;
	GList *ic;

	found = NULL;
	for( ic = content ; ic && !found ; ic = ic->next ){
		value = ( KeyValue * ) ic->data;
		if( !strcmp( value->group, group ) && !strcmp( value->def->key, key )){
			found = value;
		}
	}

	return( found );
}

/* group may be NULL
 */
static KeyValue *
read_key_value( const gchar *group, const gchar *key, gboolean *found, gboolean *mandatory )
{
	static const gchar *thisfn = "na_settings_read_key_value";
	KeyDef *key_def;
	gboolean has_entry;
	KeyValue *key_value;

	key_value = NULL;
	if( found ){
		*found = FALSE;
	}
	if( mandatory ){
		*mandatory = FALSE;
	}

	settings_new();
	key_def = get_key_def( key );

	if( key_def ){
		has_entry = FALSE;
		key_value = read_key_value_from_key_file( st_settings->private->mandatory, group ? group : key_def->group, key, key_def );
		if( key_value ){
			has_entry = TRUE;
			if( found ){
				*found = TRUE;
			}
			if( mandatory ){
				*mandatory = TRUE;
				g_debug( "%s: %s: key is mandatory", thisfn, key );
			}
		}
		if( !has_entry ){
			key_value = read_key_value_from_key_file( st_settings->private->user, group ? group : key_def->group, key, key_def );
			if( key_value ){
				has_entry = TRUE;
				if( found ){
					*found = TRUE;
				}
			}
		}
	}

	return( key_value );
}

static KeyValue *
read_key_value_from_key_file( KeyFile *keyfile, const gchar *group, const gchar *key, const KeyDef *key_def )
{
	static const gchar *thisfn = "na_settings_read_key_value_from_key_file";
	KeyValue *value;
	gchar *str;
	GError *error;

	value = NULL;
	error = NULL;
	str = NULL;

	switch( key_def->type ){

		case NA_DATA_TYPE_STRING:
		case NA_DATA_TYPE_STRING_LIST:
		case NA_DATA_TYPE_UINT:
		case NA_DATA_TYPE_UINT_LIST:
		case NA_DATA_TYPE_BOOLEAN:
			str = g_key_file_get_string( keyfile->key_file, group, key, &error );
			if( error ){
				if( error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND && error->code != G_KEY_FILE_ERROR_GROUP_NOT_FOUND ){
					g_warning( "%s: %s", thisfn, error->message );
				}
				g_error_free( error );

			/* key exists, but may be empty */
			} else {
				value = g_new0( KeyValue, 1 );
				value->group = g_strdup( group );
				value->def = key_def;
				switch( key_def->type ){
					case NA_DATA_TYPE_STRING:
					case NA_DATA_TYPE_UINT:
					case NA_DATA_TYPE_BOOLEAN:
					case NA_DATA_TYPE_STRING_LIST:
					case NA_DATA_TYPE_UINT_LIST:
						value->boxed = na_boxed_new_from_string( key_def->type, str );
						break;
				}
			}
			break;

		default:
			g_warning( "%s: group=%s, key=%s - unmanaged boxed type: %d", thisfn, group, key, key_def->type );
			return( NULL );
	}

	if( value ){
		g_debug( "%s: group=%s, key=%s, value=%s, mandatory=%s",
				thisfn, group, key, str, keyfile->mandatory ? "True":"False" );
	}

	g_free( str );

	return( value );
}

/*
 * called from instance_finalize
 * release the list of registered consumers
 */
static void
release_consumer( Consumer *consumer )
{
	g_free( consumer->monitored_key );
	g_free( consumer );
}

/*
 * called from instance_dispose
 * release the opened and monitored GKeyFiles
 */
static void
release_key_file( KeyFile *key_file )
{
	g_key_file_free( key_file->key_file );
	if( key_file->monitor ){
		if( key_file->handler ){
			g_signal_handler_disconnect( key_file->monitor, key_file->handler );
		}
		g_file_monitor_cancel( key_file->monitor );
		g_object_unref( key_file->monitor );
	}
	g_free( key_file->fname );
	g_free( key_file );
}

/*
 * called from instance_finalize
 * release a KeyValue struct
 */
static void
release_key_value( KeyValue *value )
{
	g_free(( gpointer ) value->group );
	g_object_unref( value->boxed );
	g_free( value );
}

static gboolean
set_key_value( const gchar *group, const gchar *key, const gchar *string )
{
	static const gchar *thisfn = "na_settings_set_key_value";
	KeyDef *key_def;
	const gchar *wgroup;
	gboolean ok;
	GError *error;

	ok = FALSE;
	settings_new();

	wgroup = group;
	if( !wgroup ){
		key_def = get_key_def( key );
		if( key_def ){
			wgroup = key_def->group;
		}
	}
	if( wgroup ){
		ok = TRUE;

		if( string ){
			g_key_file_set_string( st_settings->private->user->key_file, wgroup, key, string );

		} else {
			error = NULL;
			ok = g_key_file_remove_key( st_settings->private->user->key_file, wgroup, key, &error );
			if( error ){
				g_warning( "%s: g_key_file_remove_key: %s", thisfn, error->message );
				g_error_free( error );
			}
		}

		ok &= write_user_key_file();
	}

	return( ok );
}

static gboolean
write_user_key_file( void )
{
	static const gchar *thisfn = "na_settings_write_user_key_file";
	gchar *data;
	GFile *file;
	GFileOutputStream *stream;
	GError *error;
	gsize length;

	error = NULL;
	settings_new();
	data = g_key_file_to_data( st_settings->private->user->key_file, &length, NULL );
	file = g_file_new_for_path( st_settings->private->user->fname );

	stream = g_file_replace( file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &error );
	if( error ){
		g_warning( "%s: g_file_replace: %s", thisfn, error->message );
		g_error_free( error );
		if( stream ){
			g_object_unref( stream );
		}
		g_object_unref( file );
		g_free( data );
		return( FALSE );
	}

	g_output_stream_write( G_OUTPUT_STREAM( stream ), data, length, NULL, &error );
	if( error ){
		g_warning( "%s: g_output_stream_write: %s", thisfn, error->message );
		g_error_free( error );
		g_object_unref( stream );
		g_object_unref( file );
		g_free( data );
		return( FALSE );
	}

	g_output_stream_close( G_OUTPUT_STREAM( stream ), NULL, &error );
	if( error ){
		g_warning( "%s: g_output_stream_close: %s", thisfn, error->message );
		g_error_free( error );
		g_object_unref( stream );
		g_object_unref( file );
		g_free( data );
		return( FALSE );
	}

	g_object_unref( stream );
	g_object_unref( file );
	g_free( data );

	return( TRUE );
}
