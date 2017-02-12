/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu modules.
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

#ifndef __CORE_NA_TOKENS_H__
#define __CORE_NA_TOKENS_H__

/* @title: NATokens
 * @short_description: The #NATokens Class Definition
 * @include: core/na-tokens.h
 *
 * The #NATokens class manages the tokens which are to be replaced with
 * elements of the current selection at runtime.
 *
 * Note that until v2.30, tokens were parsed against selection list only
 * when an item was selected in the Caja context menu (i.e. at
 * execution time).
 * Starting with unstable v2.99 (stable v3.0), this same parsing may occur
 * for each displayed label (as new specs accept tokens in labels) - we so
 * factorize this parsing one time for each new selection in the Caja
 * plugin, attaching the result to each item in the context menu.
 *
 * Adding a parameter requires updating of:
 * - doc/cact/C/figures/cact-legend.png screenshot
 * - doc/cact/C/cact-execution.xml "Multiple execution" paragraph
 * - src/core/na-tokens.c::is_singular_exec() function
 * - src/core/na-tokens.c::parse_singular() function
 * - src/cact/caja-actions-config-tool.ui:LegendDialog labels
 * - src/core/na-object-profile-factory.c:NAFO_DATA_PARAMETERS comment
 *
 * Valid parameters are :
 *
 * %b: (first) basename
 * %B: space-separated list of basenames
 * %c: count of selected items
 * %d: (first) base directory
 * %D: space-separated list of base directory of each selected item
 * %f: (first) file name
 * %F: space-separated list of selected file names
 * %h: hostname of the (first) URI
 * %m: (first) mimetype
 * %M: space-separated list of mimetypes
 * %n: username of the (first) URI
 * %o: no-op operator which forces a singular form of execution
 * %O: no-op operator which forces a plural form of execution
 * %p: port number of the (first) URI
 * %s: scheme of the (first) URI
 * %u: (first) URI
 * %U: space-separated list of selected URIs
 * %w: (first) basename without the extension
 * %W: space-separated list of basenames without their extension
 * %x: (first) extension
 * %X: space-separated list of extensions
 * %%: the « % » character
 */

#include <api/na-object-profile.h>

G_BEGIN_DECLS

#define NA_TYPE_TOKENS                ( na_tokens_get_type())
#define NA_TOKENS( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_TOKENS, NATokens ))
#define NA_TOKENS_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_TOKENS, NATokensClass ))
#define NA_IS_TOKENS( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_TOKENS ))
#define NA_IS_TOKENS_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_TOKENS ))
#define NA_TOKENS_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_TOKENS, NATokensClass ))

typedef struct _NATokensPrivate       NATokensPrivate;

typedef struct {
	/*< private >*/
	GObject          parent;
	NATokensPrivate *private;
}
	NATokens;

typedef struct _NATokensClassPrivate  NATokensClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass          parent;
	NATokensClassPrivate *private;
}
	NATokensClass;

GType     na_tokens_get_type            ( void );

NATokens *na_tokens_new_for_example     ( void );
NATokens *na_tokens_new_from_selection  ( GList *selection );

gchar    *na_tokens_parse_for_display   ( const NATokens *tokens, const gchar *string, gboolean utf8 );
void      na_tokens_execute_action      ( const NATokens *tokens, const NAObjectProfile *profile );

gchar    *na_tokens_command_for_terminal( const gchar *pattern, const gchar *command );

G_END_DECLS

#endif /* __CORE_NA_TOKENS_H__ */
