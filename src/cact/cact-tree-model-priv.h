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

/**
 * SECTION: cact_tree_model
 * @short_description: #CactTreeModel private data definition.
 * @include: cact/cact-tree-model-priv.h
 */

#ifndef __CACT_TREE_MODEL_PRIV_H__
#define __CACT_TREE_MODEL_PRIV_H__

G_BEGIN_DECLS

/* private instance data
 */
struct CactTreeModelPrivate {
	gboolean       dispose_has_run;
	BaseWindow    *window;
	GtkTreeView   *treeview;
	gboolean       have_dnd;
	gboolean       are_profiles_displayed;
	CactClipboard *clipboard;
	gboolean       drag_has_profiles;
	gboolean       drag_highlight;		/* defined for on_drag_motion handler */
	gboolean       drag_drop;			/* defined for on_drag_motion handler */
};

#define TREE_MODEL_STATUSBAR_CONTEXT	"cact-tree-model-statusbar-context"

G_END_DECLS

#endif /* __CACT_TREE_MODEL_PRIV_H__ */
