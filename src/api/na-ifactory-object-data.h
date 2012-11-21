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

#ifndef __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_DATA_H__
#define __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_DATA_H__

/**
 * SECTION: data-name
 * @title: Constants
 * @short_description: The Data Factory Constant Definitions
 * @include: caja-actions/na-ifactory-object-data.h
 *
 * Each elementary data get its own name here.
 *
 * Through #NADataDef and #NADataGroup definitions, each #NAObjectItem
 * derived object which implement the #NAIFactoryObject interface will
 * dynamically define a property for each attached elementary data.
 */

#include <glib.h>

G_BEGIN_DECLS

/**
 * NA_FACTORY_OBJECT_ID_GROUP:
 *
 * #NAObjectId common data.
 */
#define NA_FACTORY_OBJECT_ID_GROUP          "na-factory-group-id"
#define NAFO_DATA_ID                        "na-factory-data-id"
#define NAFO_DATA_LABEL                     "na-factory-data-label"
#define NAFO_DATA_PARENT                    "na-factory-data-parent"
#define NAFO_DATA_CONDITIONS                "na-factory-data-conditions"

/**
 * NA_FACTORY_OBJECT_ITEM_GROUP:
 *
 * #NAObjectItem common data.
 */
#define NA_FACTORY_OBJECT_ITEM_GROUP        "na-factory-group-item"
#define NAFO_DATA_IVERSION                  "na-factory-data-iversion"
#define NAFO_DATA_TYPE                      "na-factory-data-type"
#define NAFO_DATA_TOOLTIP                   "na-factory-data-tooltip"
#define NAFO_DATA_ICON                      "na-factory-data-icon"
#define NAFO_DATA_ICON_NOLOC                "na-factory-data-unlocalized-icon"
#define NAFO_DATA_DESCRIPTION               "na-factory-data-description"
#define NAFO_DATA_SHORTCUT                  "na-factory-data-shortcut"
#define NAFO_DATA_SUBITEMS                  "na-factory-data-items"
#define NAFO_DATA_SUBITEMS_SLIST            "na-factory-data-items-slist"
#define NAFO_DATA_ENABLED                   "na-factory-data-enabled"
#define NAFO_DATA_READONLY                  "na-factory-data-readonly"
#define NAFO_DATA_PROVIDER                  "na-factory-data-provider"
#define NAFO_DATA_PROVIDER_DATA             "na-factory-data-provider-data"

/**
 * NA_FACTORY_OBJECT_ACTION_GROUP:
 *
 * #NAObjectAction specific datas.
 */
#define NA_FACTORY_OBJECT_ACTION_GROUP      "na-factory-group-action"
#define NAFO_DATA_VERSION                   "na-factory-data-version"
#define NAFO_DATA_TARGET_SELECTION          "na-factory-data-target-selection"
#define NAFO_DATA_TARGET_LOCATION           "na-factory-data-target-location"
#define NAFO_DATA_TARGET_TOOLBAR            "na-factory-data-target-toolbar"
#define NAFO_DATA_TOOLBAR_LABEL             "na-factory-data-toolbar-label"
#define NAFO_DATA_TOOLBAR_SAME_LABEL        "na-factory-data-toolbar-same-label"
#define NAFO_DATA_LAST_ALLOCATED            "na-factory-data-last-allocated"

/**
 * NA_FACTORY_ACTION_V1_GROUP:
 *
 * A group of datas which are specific to v 1 actions. It happens to be
 * empty as all these datas have been alter embedded in #NAObjectItem
 * data group.
 */
#define NA_FACTORY_ACTION_V1_GROUP          "na-factory-group-action-v1"

/**
 * NA_FACTORY_OBJECT_MENU_GROUP:
 *
 * #NAObjectMenu specific datas. It happens to be empty as the definition
 * of a menu is very close of those of an action.
 */
#define NA_FACTORY_OBJECT_MENU_GROUP        "na-factory-group-menu"

/**
 * NA_FACTORY_OBJECT_PROFILE_GROUP:
 *
 * #NAObjectProfile specific datas.
 */
#define NA_FACTORY_OBJECT_PROFILE_GROUP     "na-factory-group-profile"
#define NAFO_DATA_DESCNAME                  "na-factory-data-descname"
#define NAFO_DATA_DESCNAME_NOLOC            "na-factory-data-unlocalized-descname"
#define NAFO_DATA_PATH                      "na-factory-data-path"
#define NAFO_DATA_PARAMETERS                "na-factory-data-parameters"
#define NAFO_DATA_WORKING_DIR               "na-factory-data-working-dir"
#define NAFO_DATA_EXECUTION_MODE            "na-factory-data-execution-mode"
#define NAFO_DATA_STARTUP_NOTIFY            "na-factory-data-startup-notify"
#define NAFO_DATA_STARTUP_WMCLASS           "na-factory-data-startup-wm-class"
#define NAFO_DATA_EXECUTE_AS                "na-factory-data-execute-as"

/**
 * NA_FACTORY_OBJECT_CONDITIONS_GROUP:
 *
 * The datas which determine the display conditions of a menu or an action.
 *
 * @see_also: #NAIContext interface.
 */
#define NA_FACTORY_OBJECT_CONDITIONS_GROUP  "na-factory-group-conditions"
#define NAFO_DATA_BASENAMES                 "na-factory-data-basenames"
#define NAFO_DATA_MATCHCASE                 "na-factory-data-matchcase"
#define NAFO_DATA_MIMETYPES                 "na-factory-data-mimetypes"
#define NAFO_DATA_MIMETYPES_IS_ALL             "na-factory-data-all-mimetypes"
#define NAFO_DATA_ISFILE                    "na-factory-data-isfile"
#define NAFO_DATA_ISDIR                     "na-factory-data-isdir"
#define NAFO_DATA_MULTIPLE                  "na-factory-data-multiple"
#define NAFO_DATA_SCHEMES                   "na-factory-data-schemes"
#define NAFO_DATA_FOLDERS                   "na-factory-data-folders"
#define NAFO_DATA_SELECTION_COUNT           "na-factory-data-selection-count"
#define NAFO_DATA_ONLY_SHOW                 "na-factory-data-only-show-in"
#define NAFO_DATA_NOT_SHOW                  "na-factory-data-not-show-in"
#define NAFO_DATA_TRY_EXEC                  "na-factory-data-try-exec"
#define NAFO_DATA_SHOW_IF_REGISTERED        "na-factory-data-show-if-registered"
#define NAFO_DATA_SHOW_IF_TRUE              "na-factory-data-show-if-true"
#define NAFO_DATA_SHOW_IF_RUNNING           "na-factory-data-show-if-running"
#define NAFO_DATA_CAPABILITITES             "na-factory-data-capabilitites"

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_DATA_H__ */
