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

#ifndef __NAGP_MATECONF_PROVIDER_KEYS_H__
#define __NAGP_MATECONF_PROVIDER_KEYS_H__

#define NAGP_CONFIGURATIONS_PATH		"/apps/caja-actions/configurations"
#define NAGP_SCHEMAS_PATH				"/schemas/apps/caja-actions/configurations"

#define NAGP_ENTRY_TYPE					"type"
#define NAGP_VALUE_TYPE_MENU			"Menu"
#define NAGP_VALUE_TYPE_ACTION			"Action"

#define NAGP_ENTRY_LABEL				"label"
#define NAGP_ENTRY_TOOLTIP				"tooltip"
#define NAGP_ENTRY_ICON					"icon"
#define NAGP_ENTRY_ENABLED				"enabled"
#define NAGP_ENTRY_ITEMS_LIST			"items"

#define NAGP_ENTRY_VERSION				"version"
#define NAGP_ENTRY_TARGET_SELECTION		"target-selection"
#define NAGP_ENTRY_TARGET_CONTEXT		"target-context"
#define NAGP_ENTRY_TARGET_BACKGROUND	"target-background"
#define NAGP_ENTRY_TARGET_TOOLBAR		"target-toolbar"
#define NAGP_ENTRY_TOOLBAR_LABEL		"toolbar-label"
#define NAGP_ENTRY_TOOLBAR_SAME_LABEL	"toolbar-same-label"

#define NAGP_ENTRY_PROFILE_LABEL		"desc-name"
#define NAGP_ENTRY_PATH					"path"
#define NAGP_ENTRY_PARAMETERS			"parameters"
#define NAGP_ENTRY_BASENAMES			"basenames"
#define NAGP_ENTRY_MATCHCASE			"matchcase"
#define NAGP_ENTRY_MIMETYPES			"mimetypes"
#define NAGP_ENTRY_ISFILE				"isfile"
#define NAGP_ENTRY_ISDIR				"isdir"
#define NAGP_ENTRY_MULTIPLE				"accept-multiple-files"
#define NAGP_ENTRY_SCHEMES				"schemes"
#define NAGP_ENTRY_FOLDERS				"folders"

#endif /* __NAGP_MATECONF_PROVIDER_KEYS_H__ */
