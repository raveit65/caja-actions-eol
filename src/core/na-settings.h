/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu modules.
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

#ifndef __CORE_NA_SETTINGS_H__
#define __CORE_NA_SETTINGS_H__

/* @title: NASettings
 * @short_description: The Settings Class Definition
 * @include: core/na-settings.h
 *
 * The #NASettings class manages users preferences.
 *
 * Actual configuration may come from two sources:
 * - a global configuration, which apply to all users, as read-only
 *   parameters;
 * - a per-user configuration.
 *
 * The configuration is implemented as keyed files:
 * - global configuration is sysconfdir/xdg/caja-actions/caja-actions.conf
 * - per-user configuration is HOME/.config/caja-actions/caja-actions.conf
 *
 * Each setting may so have its own read-only attribute, whether it
 * has been read from the global configuration or from the
 * per-user one.
 *
 * NASettings class monitors the whole configuration.
 * A client may be informed of a modification of the value of a key either by
 * pre-registering a callback on this key (see na_settings_register_key_callback()
 * function), or by connecting to and filtering the notification signal.
 *
 * #NASettings class defines a singleton object, which allocates itself
 * when needed
 */

#include <glib-object.h>

G_BEGIN_DECLS

/* This is a composite key;
 * by registering a callback on this key, a client may be informed of any
 * modification regarding the readability status of the i/o providers.
 */
#define NA_IPREFS_IO_PROVIDERS_READ_STATUS			"io-providers-read-status-composite-key"

/* other keys, mainly user preferences
 */
#define NA_IPREFS_ADMIN_PREFERENCES_LOCKED			"preferences-locked"
#define NA_IPREFS_ADMIN_IO_PROVIDERS_LOCKED			"io-providers-locked"
#define NA_IPREFS_ASSISTANT_ESC_CONFIRM				"assistant-esc-confirm"
#define NA_IPREFS_ASSISTANT_ESC_QUIT				"assistant-esc-quit"
#define NA_IPREFS_CAPABILITY_ADD_CAPABILITY_WSP		"capability-add-capability-wsp"
#define NA_IPREFS_COMMAND_CHOOSER_WSP				"command-command-chooser-wsp"
#define NA_IPREFS_COMMAND_CHOOSER_URI				"command-command-chooser-lfu"
#define NA_IPREFS_COMMAND_LEGEND_WSP				"command-legend-wsp"
#define NA_IPREFS_DESKTOP_ENVIRONMENT				"desktop-environment"
#define NA_IPREFS_CONFIRM_LOGOUT_WSP				"confirm-logout-wsp"
#define NA_IPREFS_WORKING_DIR_WSP					"command-working-dir-chooser-wsp"
#define NA_IPREFS_WORKING_DIR_URI					"command-working-dir-chooser-lfu"
#define NA_IPREFS_SHOW_IF_RUNNING_WSP				"environment-show-if-running-wsp"
#define NA_IPREFS_SHOW_IF_RUNNING_URI				"environment-show-if-running-lfu"
#define NA_IPREFS_TRY_EXEC_WSP						"environment-try-exec-wsp"
#define NA_IPREFS_TRY_EXEC_URI						"environment-try-exec-lfu"
#define NA_IPREFS_EXPORT_ASK_USER_WSP				"export-ask-user-wsp"
#define NA_IPREFS_EXPORT_ASK_USER_LAST_FORMAT		"export-ask-user-last-format"
#define NA_IPREFS_EXPORT_ASK_USER_KEEP_LAST_CHOICE	"export-ask-user-keep-last-choice"
#define NA_IPREFS_EXPORT_ASSISTANT_WSP				"export-assistant-wsp"
#define NA_IPREFS_EXPORT_ASSISTANT_URI				"export-assistant-lfu"
#define NA_IPREFS_EXPORT_ASSISTANT_PANED			"export-assistant-paned-width"
#define NA_IPREFS_EXPORT_PREFERRED_FORMAT			"export-preferred-format"
#define NA_IPREFS_FOLDER_CHOOSER_WSP				"folder-chooser-wsp"
#define NA_IPREFS_FOLDER_CHOOSER_URI				"folder-chooser-lfu"
#define NA_IPREFS_IMPORT_ASK_USER_WSP				"import-ask-user-wsp"
#define NA_IPREFS_IMPORT_ASK_USER_LAST_MODE			"import-ask-user-last-mode"
#define NA_IPREFS_IMPORT_ASSISTANT_WSP				"import-assistant-wsp"
#define NA_IPREFS_IMPORT_ASSISTANT_URI				"import-assistant-lfu"
#define NA_IPREFS_IMPORT_ASK_USER_KEEP_LAST_CHOICE	"import-ask-user-keep-last-choice"
#define NA_IPREFS_IMPORT_PREFERRED_MODE				"import-preferred-mode"
#define NA_IPREFS_ICON_CHOOSER_URI					"item-icon-chooser-lfu"
#define NA_IPREFS_ICON_CHOOSER_PANED				"item-icon-chooser-paned-width"
#define NA_IPREFS_ICON_CHOOSER_WSP					"item-icon-chooser-wsp"
#define NA_IPREFS_IO_PROVIDERS_WRITE_ORDER			"io-providers-write-order"
#define NA_IPREFS_ITEMS_ADD_ABOUT_ITEM				"items-add-about-item"
#define NA_IPREFS_ITEMS_CREATE_ROOT_MENU			"items-create-root-menu"
#define NA_IPREFS_ITEMS_LEVEL_ZERO_ORDER			"items-level-zero-order"
#define NA_IPREFS_ITEMS_LIST_ORDER_MODE				"items-list-order-mode"
#define NA_IPREFS_MAIN_PANED						"main-paned-width"
#define NA_IPREFS_MAIN_SAVE_AUTO					"main-save-auto"
#define NA_IPREFS_MAIN_SAVE_PERIOD					"main-save-period"
#define NA_IPREFS_MAIN_TABS_POS						"main-tabs-pos"
#define NA_IPREFS_MAIN_TOOLBAR_EDIT_DISPLAY			"main-toolbar-edit-display"
#define NA_IPREFS_MAIN_TOOLBAR_FILE_DISPLAY			"main-toolbar-file-display"
#define NA_IPREFS_MAIN_TOOLBAR_HELP_DISPLAY			"main-toolbar-help-display"
#define NA_IPREFS_MAIN_TOOLBAR_TOOLS_DISPLAY		"main-toolbar-tools-display"
#define NA_IPREFS_MAIN_WINDOW_WSP					"main-window-wsp"
#define NA_IPREFS_PREFERENCES_WSP					"preferences-wsp"
#define NA_IPREFS_PLUGIN_MENU_LOG					"plugin-menu-log-enabled"
#define NA_IPREFS_RELABEL_DUPLICATE_ACTION			"relabel-when-duplicate-action"
#define NA_IPREFS_RELABEL_DUPLICATE_MENU			"relabel-when-duplicate-menu"
#define NA_IPREFS_RELABEL_DUPLICATE_PROFILE			"relabel-when-duplicate-profile"
#define NA_IPREFS_SCHEME_ADD_SCHEME_WSP				"scheme-add-scheme-wsp"
#define NA_IPREFS_SCHEME_DEFAULT_LIST				"scheme-default-list"
#define NA_IPREFS_TERMINAL_PATTERN					"terminal-pattern"

#define NA_IPREFS_IO_PROVIDER_GROUP					"io-provider"
#define NA_IPREFS_IO_PROVIDER_READABLE				"readable"
#define NA_IPREFS_IO_PROVIDER_WRITABLE				"writable"

/* pre-registration of a callback
 */
typedef void ( *NASettingsKeyCallback )( const gchar *group, const gchar *key, gconstpointer new_value, gboolean mandatory, void *user_data );

void      na_settings_register_key_callback( const gchar *key, GCallback callback, gpointer user_data );

/* signal sent when the value of a key changes
 */
#define SETTINGS_SIGNAL_KEY_CHANGED					"settings-key-changed"

void      na_settings_free                 ( void );

gboolean  na_settings_get_boolean          ( const gchar *key, gboolean *found, gboolean *mandatory );
gboolean  na_settings_get_boolean_ex       ( const gchar *group, const gchar *key, gboolean *found, gboolean *mandatory );
gchar    *na_settings_get_string           ( const gchar *key, gboolean *found, gboolean *mandatory );
GSList   *na_settings_get_string_list      ( const gchar *key, gboolean *found, gboolean *mandatory );
guint     na_settings_get_uint             ( const gchar *key, gboolean *found, gboolean *mandatory );
GList    *na_settings_get_uint_list        ( const gchar *key, gboolean *found, gboolean *mandatory );

gboolean  na_settings_set_boolean          ( const gchar *key, gboolean value );
gboolean  na_settings_set_boolean_ex       ( const gchar *group, const gchar *key, gboolean value );
gboolean  na_settings_set_string           ( const gchar *key, const gchar *value );
gboolean  na_settings_set_string_ex        ( const gchar *group, const gchar *key, const gchar *value );
gboolean  na_settings_set_string_list      ( const gchar *key, const GSList *value );
gboolean  na_settings_set_int_ex           ( const gchar *group, const gchar *key, int value );
gboolean  na_settings_set_uint             ( const gchar *key, guint value );
gboolean  na_settings_set_uint_list        ( const gchar *key, const GList *value );

GSList   *na_settings_get_groups           ( void );

G_END_DECLS

#endif /* __CORE_NA_SETTINGS_H__ */
