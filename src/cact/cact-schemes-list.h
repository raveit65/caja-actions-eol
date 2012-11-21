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

#ifndef __CACT_SCHEMES_LIST_H__
#define __CACT_SCHEMES_LIST_H__

/**
 * SECTION: cact_schemes_list
 * @short_description: Schemes list view management.
 * @include: cact/cact-schemes-list.h
 *
 * This set of functions manages the schemes list view.
 *
 * Up to 2.30.x, two modes were possible:
 * - for action: the full list is displayed, and a check box is made active
 *   when the scheme is actually selected in the profile.
 *   Adding a scheme insert an editable new row (without the description).
 *   Inline edition of the scheme is possible.
 *   Removing a scheme is possible.
 * - in preferences, when editing the default list of schemes
 *   the 'active' checkbox is not displayed
 *   the two columns 'scheme' and 'description' are editable inline
 *   adding/removing a scheme is possible
 *
 * Starting with 2.31.x serie (future 3.0), the scheme conditions of a
 * #NAIContext are handled by cact-match-list.{c,h} set of function.
 * This set of functions is only used:
 *  a) to edit the preferences
 *     add/remove scheme
 *     edit keyword and description
 *     In this mode, the widget is embedded in the Preferences notebook.
 *     Modifications are saved when user clicks the global OK button.
 *  b) to select a scheme from the default list
 *     schemes already used by the current #NAIContext are marked as used
 *     edition of the current list is not available
 *     In this mode, widget is embedded in a dedicated #CactAddSchemeDialog
 *     dialog box
 *     OK returns the current selection (only available if current scheme
 *     is not already used)
 *     Cancel returns NULL.
 */

#include <gtk/gtk.h>

#include "base-window.h"

G_BEGIN_DECLS

typedef void ( *pf_new_selection_cb )( const gchar *, gboolean, void * );

enum {
	SCHEMES_LIST_FOR_PREFERENCES = 1,
	SCHEMES_LIST_FOR_ADD_FROM_DEFAULTS
};

void    cact_schemes_list_create_model      ( GtkTreeView *treeview, guint mode );
void    cact_schemes_list_init_view         ( GtkTreeView *treeview, BaseWindow *window, pf_new_selection_cb pf, void *user_data );
void    cact_schemes_list_setup_values      ( BaseWindow *window, GSList *schemes );
void    cact_schemes_list_show_all          ( BaseWindow *window );
gchar  *cact_schemes_list_get_current_scheme( BaseWindow *window );
void    cact_schemes_list_save_defaults     ( BaseWindow *window );
void    cact_schemes_list_dispose           ( BaseWindow *window );

G_END_DECLS

#endif /* __CACT_SCHEMES_LIST_H__ */
