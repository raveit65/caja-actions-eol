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

#ifndef __CACT_MAIN_WINDOW_H__
#define __CACT_MAIN_WINDOW_H__

/**
 * SECTION: main-window
 * @title: CactMainWindow
 * @short_description: The Main Window class definition
 * @include: cact-main-window.h
 *
 * This class is derived from CactWindow and manages the main window.
 *
 * It embeds:
 * - the menubar,
 * - the toolbar,
 * - a button bar with sort buttons,
 * - the hierarchical list of items,
 * - a notebook which displays the content of the current item,
 * - a status bar.
 *
 * CactApplication    CactMainWindow    CactTreeView    CactTreeModel   CactMenubar
 *  |
 *  +-> cact_main_window_new()
 *  |   |
 *  |   +-> CactMainWindow::instance_contructed()
 *  |   |   |
 *  |   |   +-> connect to base-init-gtk       [window]
 *  |   |   |              base-init-window    [window]
 *  |   |   |              base-show-widgets   [window]
 *  |   |   |              pivot-items-changed [updater]
 *  |   |   |              tab-item-updated    [window]
 *  |   |   |
 *  |   |   +-> cact_menubar_new()
 *  |   |   |   |
 *  |   |   |   +-> CactMenubar::cact_menubar_new()
 *  |   |   |   |   |
 *  |   |   |   |   +-> cact_sort_buttons_new()
 *  |   |   |   |   +-> connect to base-init-window [window]
 *  |   |   |   |   |
 *  |   |   |   |  <-
 *  |   |   |  <-
 *  |   |   |
 *  |   |   +-> cact_clipboard_new()
 *  |   |   |
 *  |   |   +-> initialize each notebook tab
 *  |   |   |
 *  |   |   |   CactMainWindow has connected to "base-init-window" signal _before_
 *  |   |   |   other widgets have been created or initialized. We so are sure that
 *  |   |   |   the CactMainWindow handlers will be called first.
 *  |   |  <-
 *  |   |
 *  |   +-> base_window_init()
 *  |   |   |
 *  |   |   +-> setup builder
 *  |   |   +-> load gtk toplevel
 *  |   |   +-> emit signal base-initialize-gtk
 *  |   |   |
 *  |   |   |   [H]-> CactMainWindow::on_base_initialize_gtk()
 *  |   |   |         |
 *  |   |   |         +-> cact_tree_view_new()
 *  |   |   |         |   |
 *  |   |   |         |   +-> CactTreeView::instance_contructed()
 *  |   |   |         |   |   |
 *  |   |   |         |   |   +-> connect to base-init-window  [window]
 *  |   |   |         |   |   |              base-show-widgets [window]
 *  |   |   |         |   |   |
 *  |   |   |         |   |   +-> initialize_gtk()
 *  |   |   |         |   |   |   |
 *  |   |   |         |   |   |   +-> cact_tree_model_new()
 *  |   |   |         |   |   |   |   |
 *  |   |   |         |   |   |   |   +-> CactTreeModel::cact_tree_model_new()
 *  |   |   |         |   |   |   |   |
 *  |   |   |         |   |   |   |  <-
 *  |   |   |         |   |   |  <-
 *  |   |   |         |   |  <-
 *  |   |   |         |  <-
 *  |   |   |         |
 *  |   |   |         +-> cact_main_statusbar_initialize_load_toplevel()
 *  |   |   |
 *  |   |   |   [H]-> CactIxxxxxxTab::on_base_initialize_gtk()
 *  |   |   |
 *  |   |   +-> emit signal base-initialize-window
 *  |   |   |
 *  |   |   |   [H]-> CactMainWindow::on_base_initialize_base_window()
 *  |   |   |         |
 *  |   |   |         +-> connect to delete-event
 *  |   |   |         |              base-quit-requested
 *  |   |   |         |
 *  |   |   |         +-> connect to tree-selection-changed
 *  |   |   |         |              tree-modified-status-changed
 *  |   |   |         |
 *  |   |   |         |   Because CactMainWindow was the first class to connect to
 *  |   |   |         |   "base-initialize-window" signal, then this handler has been
 *  |   |   |         |   the first to be called. And so the "tree-selection-changed"
 *  |   |   |         |   signal handler will also the first to be called, and we can
 *  |   |   |         |   safely rely on that.
 *  |   |   |        <-
 *  |   |   |
 *  |   |   |   [H]-> CactIxxxxxxTab::on_base_initialize_window()
 *  |   |   |
 *  |   |   |   [H]-> CactTreeView::on_base_initialize_view()
 *  |   |   |         |
 *  |   |   |         +-> monitors the selection in the tree
 *  |   |   |         |   in order to be able to send the "tree-selection-changed" signal
 *  |   |   |         |
 *  |   |   |         +- cact_tree_ieditable_initialize()
 *  |   |   |         |
 *  |   |   |        <-
 *  |   |   |
 *  |   |   +-> emit signal base-show-widgets
 *  |   |   |
 *  |   |   |   [H]-> CactMainWindow::on_base_show_widgets()
 *  |   |   |         |
 *  |   |   |         +-> load items from pivot
 *  |   |   |
 *  |   |   |   [H]-> CactIxxxxxxTab::on_base_show_widgets()
 *  |   |   |
 *  |   |   +-> gtk_widget_show_all()
 *  |   |   |
 *  |   |  <-
 *  |  <-
 *  |
 *  +-> gtk_main()
 *  |
 * [X] End of initialization process
 *
 * Some signals and their usages
 * =============================
 * MAIN_SIGNAL_SELECTION_CHANGED
 *   The signal is sent by the tree view on the BaseWindow each time the
 *   selection has changed. By construction (cf. initialization process),
 *   the main window handler is the first to be triggered by this signal:
 *   it sets the 'current' main window properties to reflect this new
 *   selection.
 *   Args:
 *   - the list of selected items, may be NULL.
 *   Consumers:
 *   - all tabs should take advantage of this signal to enable/disable
 *     their page, setup the content of their widgets, and so on.
 *   - the menubar updates its indicator depending of the current selection
 *
 * TAB_UPDATABLE_SIGNAL_ITEM_UPDATED
 *   The signal is sent on the BaseWindow each time a widget is updated; the widget
 *   callback must setup the edited object with the new value, and then should call
 *   this signal with a flag indicating if the tree display should be refreshed now.
 *   Args:
 *   - an OR-ed list of modified flags, or 0 if not relevant
 *   Consumers are:
 *   - the main window checks the modification/validity status of the object
 *   - if the 'refresh tree display' flag is set, then the tree model refreshes
 *     the current row with current label and icon, then flags current row as
 *     modified
 *
 * MAIN_SIGNAL_ITEM_UPDATED
 *   The signal is sent on the BaseWindow after a data has been modified elsewhere
 *   that in a tab: either the label has been edited inline in the tree view,
 *   or a new i/o provider has been identified. The relevant NAObject has
 *   been updated accordingly.
 *   Args:
 *   - an OR-ed list of modified flags, or 0 if not relevant
 *   Consumers are:
 *   - IActionTab and ICommandTab should update their label widgets
 *   - IPropertiesTab updates its provider label
 *
 * MAIN_SIGNAL_CONTEXT_MENU
 *   Opens the specified context menu.
 *
 * TREE_SIGNAL_FOCUS_IN
 * TREE_SIGNAL_FOCUS_OUT
 * TREE_SIGNAL_COUNT_CHANGED
 * TREE_SIGNAL_LEVEL_ZERO_CHANGED
 * TREE_SIGNAL_MODIFIED_STATUS_CHANGED
 *
 * TAB_UPDATABLE_SIGNAL_PROVIDER_CHANGED
 *
 * Object
 */

#include "cact-application.h"
#include "cact-clipboard.h"
#include "cact-tree-view.h"

G_BEGIN_DECLS

#define CACT_TYPE_MAIN_WINDOW                ( cact_main_window_get_type())
#define CACT_MAIN_WINDOW( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_MAIN_WINDOW, CactMainWindow ))
#define CACT_MAIN_WINDOW_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_MAIN_WINDOW, CactMainWindowClass ))
#define CACT_IS_MAIN_WINDOW( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_MAIN_WINDOW ))
#define CACT_IS_MAIN_WINDOW_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_MAIN_WINDOW ))
#define CACT_MAIN_WINDOW_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_MAIN_WINDOW, CactMainWindowClass ))

typedef struct _CactMainWindowPrivate        CactMainWindowPrivate;

typedef struct {
	/*< private >*/
	BaseWindow             parent;
	CactMainWindowPrivate *private;
}
	CactMainWindow;

typedef struct _CactMainWindowClassPrivate   CactMainWindowClassPrivate;

typedef struct {
	/*< private >*/
	BaseWindowClass             parent;
	CactMainWindowClassPrivate *private;
}
	CactMainWindowClass;

/**
 * Signals emitted by the main window
 */
#define MAIN_SIGNAL_ITEM_UPDATED			"main-item-updated"
#define MAIN_SIGNAL_SELECTION_CHANGED		"main-selection-changed"
#define MAIN_SIGNAL_CONTEXT_MENU			"main-signal-open-popup"

/**
 * The data which, when modified, should be redisplayed asap.
 * This is used by MAIN_SIGNAL_ITEM_UPDATED and TAB_UPDATABLE_SIGNAL_ITEM_UPDATED
 * signals.
 */
enum {
	MAIN_DATA_LABEL    = 1<<0,
	MAIN_DATA_ICON     = 1<<1,
	MAIN_DATA_PROVIDER = 1<<2,
};

/**
 * Properties set against the main window
 */
#define MAIN_PROP_ITEM						"main-current-item"
#define MAIN_PROP_PROFILE					"main-current-profile"
#define MAIN_PROP_CONTEXT					"main-current-context"
#define MAIN_PROP_EDITABLE					"main-editable"
#define MAIN_PROP_REASON					"main-reason"

GType           cact_main_window_get_type( void );

CactMainWindow *cact_main_window_new           ( const CactApplication *application );

CactClipboard  *cact_main_window_get_clipboard ( const CactMainWindow *window );
CactTreeView   *cact_main_window_get_items_view( const CactMainWindow *window );

void            cact_main_window_reload      ( CactMainWindow *window );
void            cact_main_window_block_reload( CactMainWindow *window );
gboolean        cact_main_window_quit        ( CactMainWindow *window );

G_END_DECLS

#endif /* __CACT_MAIN_WINDOW_H__ */
