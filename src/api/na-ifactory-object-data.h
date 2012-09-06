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

#ifndef __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_DATA_H__
#define __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_DATA_H__

/**
 * SECTION: na_ifactory_object
 * @short_description: Declaration of all serializable elementary datas.
 * @include: caja-actions/na-ifactory-object-data.h
 */

#include <glib.h>

G_BEGIN_DECLS

#define NA_FACTORY_OBJECT_ID_GROUP			"na-factory-group-id"
#define NAFO_DATA_ID						"na-factory-data-id"
#define NAFO_DATA_PARENT					"na-factory-data-parent"
#define NAFO_DATA_CONDITIONS				"na-factory-data-conditions"

#define NA_FACTORY_OBJECT_ITEM_GROUP		"na-factory-group-item"
#define NAFO_DATA_TYPE						"na-factory-data-type"
#define NAFO_DATA_LABEL						"na-factory-data-label"
#define NAFO_DATA_TOOLTIP					"na-factory-data-tooltip"
#define NAFO_DATA_ICON						"na-factory-data-icon"
#define NAFO_DATA_DESCRIPTION				"na-factory-data-description"
#define NAFO_DATA_SUBITEMS					"na-factory-data-items"
#define NAFO_DATA_SUBITEMS_SLIST			"na-factory-data-items-slist"
#define NAFO_DATA_ENABLED					"na-factory-data-enabled"
#define NAFO_DATA_READONLY					"na-factory-data-readonly"
#define NAFO_DATA_PROVIDER					"na-factory-data-provider"
#define NAFO_DATA_PROVIDER_DATA				"na-factory-data-provider-data"

#define NA_FACTORY_OBJECT_ACTION_GROUP		"na-factory-group-action"
#define NAFO_DATA_VERSION					"na-factory-data-version"
#define NAFO_DATA_TARGET_SELECTION			"na-factory-data-target-selection"
#define NAFO_DATA_TARGET_LOCATION			"na-factory-data-target-location"
#define NAFO_DATA_TARGET_TOOLBAR			"na-factory-data-target-toolbar"
#define NAFO_DATA_TOOLBAR_LABEL				"na-factory-data-toolbar-label"
#define NAFO_DATA_TOOLBAR_SAME_LABEL		"na-factory-data-toolbar-same-label"
#define NAFO_DATA_LAST_ALLOCATED			"na-factory-data-last-allocated"

#define NA_FACTORY_ACTION_V1_GROUP			"na-factory-group-action-v1"

#define NA_FACTORY_OBJECT_MENU_GROUP		"na-factory-group-menu"

#define NA_FACTORY_OBJECT_PROFILE_GROUP		"na-factory-group-profile"
#define NAFO_DATA_DESCNAME					"na-factory-data-descname"
#define NAFO_DATA_PATH						"na-factory-data-path"
#define NAFO_DATA_PARAMETERS				"na-factory-data-parameters"

#define NA_FACTORY_OBJECT_CONDITIONS_GROUP	"na-factory-group-conditions"
#define NAFO_DATA_BASENAMES					"na-factory-data-basenames"
#define NAFO_DATA_MATCHCASE					"na-factory-data-matchcase"
#define NAFO_DATA_MIMETYPES					"na-factory-data-mimetypes"
#define NAFO_DATA_ISFILE					"na-factory-data-isfile"
#define NAFO_DATA_ISDIR						"na-factory-data-isdir"
#define NAFO_DATA_MULTIPLE					"na-factory-data-multiple"
#define NAFO_DATA_SCHEMES					"na-factory-data-schemes"
#define NAFO_DATA_FOLDERS					"na-factory-data-folders"

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_DATA_H__ */
