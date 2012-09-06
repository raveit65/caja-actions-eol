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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include <api/na-ifactory-object-data.h>
#include <api/na-data-def.h>
#include <api/na-data-types.h>

extern NADataDef data_def_id[];			/* defined in na-object-id-factory.c */
extern NADataDef data_def_item [];		/* defined in na-object-item-factory.c */

static NADataDef data_def_action [] = {

	{ NAFO_DATA_VERSION,
				TRUE,
				TRUE,
				TRUE,
				N_( "Version of the format" ),
				N_( "The version of the configuration format that will be used to manage " \
					"backward compatibility." ),
				NAFD_TYPE_STRING,
				"2.0",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"version",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_TARGET_SELECTION,
				TRUE,
				TRUE,
				TRUE,
				N_( "Targets the selection context menu (default)" ),
				N_( "Whether the action targets the selection file manager context menus.\n" \
					"This used to be the historical behavior.\n" \
					"Note that menus are always potential candidate to the display in " \
					"selection context menus provided that they contain at least one action.\n" \
					"Defaults to TRUE." ),
				NAFD_TYPE_BOOLEAN,
				"true",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"target-selection",
				"TargetContext",
				'n',
				"selection",
				0,
				G_OPTION_ARG_NONE,
				NULL,
				NULL },

	{ NAFO_DATA_TARGET_LOCATION,
				TRUE,
				TRUE,
				TRUE,
				N_( "Targets the location context menu" ),
				N_( "Whether the action targets the file manager context menus " \
					"when there is no selection, thus applying to current location.\n" \
					"Note that menus are always potential candidate to the display in " \
					"selection context menus provided that they contain at least one action.\n" \
					"Defaults to FALSE" ),
				NAFD_TYPE_BOOLEAN,
				"false",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"target-location",
				"TargetLocation",
				'N',
				"location",
				0,
				G_OPTION_ARG_NONE,
				NULL,
				NULL },

	{ NAFO_DATA_TARGET_TOOLBAR,
				TRUE,
				TRUE,
				TRUE,
				N_( "Targets the toolbar" ),
				N_( "Whether the action is candidate to be displayed in file manager toolbar.\n" \
					"This only applies to current location.\n" \
					"Note that menus are never displayed in the toolbar.\n" \
					"Defaults to FALSE." ),
				NAFD_TYPE_BOOLEAN,
				"false",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"target-toolbar",
				"TargetToolbar",
				'o',
				"toolbar",
				0,
				G_OPTION_ARG_NONE,
				NULL,
				NULL },

	{ NAFO_DATA_TOOLBAR_LABEL,
				TRUE,
				TRUE,
				TRUE,
				N_( "Label of the toolbar item" ),
				N_( "The label displayed besides of the icon in the file manager toolbar.\n" \
					"Note that actual display may depend of your own Desktop Environment preferences.\n" \
					"Defaults to label of the context menu when not set or empty."),
				NAFD_TYPE_LOCALE_STRING,
				"",
				TRUE,
				TRUE,
				FALSE,
				TRUE,
				"toolbar-label",
				"ToolbarLabel",
				'L',
				"toolbar-label",
				0,
				G_OPTION_ARG_STRING,
				NULL,
				N_( "<STRING>" ) },

	/* this data has been introduced in 2.29.1 and has been left up to 2.29.4
	 * it has been removed starting with 2.29.5
	 * it is now only used in the NACT user interface
	 * it is so left readable, but no more writable (obsolete)
	 */
	{ NAFO_DATA_TOOLBAR_SAME_LABEL,
				TRUE,
				FALSE,
				TRUE,
				"Does the toolbar label is the same than the main one ?",
				"Does the toolbar label is the same than the main one ?",
				NAFD_TYPE_BOOLEAN,
				"true",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"toolbar-same-label",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	/* dynamic data, so non readable / non writable
	 */
	{ NAFO_DATA_LAST_ALLOCATED,
				FALSE,
				FALSE,
				TRUE,
				"Last allocated profile",
				"Last allocated profile number in na_object_action_get_new_profile_name(), " \
				"reset to zero when saving the action.",
				NAFD_TYPE_UINT,
				"0",
				TRUE,
				FALSE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NULL },
};

/* all these data are pre-profiles data
 * these are obsoleted since 1.9 (which was a non-official version)
 * readable but non writable, no default
 */
NADataDef data_def_action_v1 [] = {

	{ NAFO_DATA_PATH,
				TRUE,
				FALSE,
				FALSE,
				"Command path",
				NULL,
				NAFD_TYPE_STRING,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"path",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_PARAMETERS,
				TRUE,
				FALSE,
				FALSE,
				"Command parameters",
				NULL,
				NAFD_TYPE_STRING,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"parameters",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_BASENAMES,
				TRUE,
				FALSE,
				FALSE,
				"Basenames",
				NULL,
				NAFD_TYPE_STRING_LIST,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"basenames",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_MATCHCASE,
				TRUE,
				FALSE,
				FALSE,
				"Case sensitive",
				NULL,
				NAFD_TYPE_BOOLEAN,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"matchcase",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_MIMETYPES,
				TRUE,
				FALSE,
				FALSE,
				"Mimetypes",
				NULL,
				NAFD_TYPE_STRING_LIST,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"mimetypes",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_ISFILE,
				TRUE,
				FALSE,
				FALSE,
				"Applies to files only",
				NULL,
				NAFD_TYPE_BOOLEAN,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"isfile",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_ISDIR,
				TRUE,
				FALSE,
				FALSE,
				"Applies to directories only",
				NULL,
				NAFD_TYPE_BOOLEAN,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"isdir",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_MULTIPLE,
				TRUE,
				FALSE,
				FALSE,
				"Multiple selection",
				NULL,
				NAFD_TYPE_BOOLEAN,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"accept-multiple-files",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_SCHEMES,
				TRUE,
				FALSE,
				FALSE,
				"Schemes",
				NULL,
				NAFD_TYPE_STRING_LIST,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"schemes",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NULL },
};

NADataGroup action_data_groups [] = {
	{ NA_FACTORY_OBJECT_ID_GROUP,         data_def_id },
	{ NA_FACTORY_OBJECT_ITEM_GROUP,       data_def_item },
	{ NA_FACTORY_OBJECT_ACTION_GROUP,     data_def_action },
	{ NA_FACTORY_ACTION_V1_GROUP,         data_def_action_v1 },
	{ NA_FACTORY_OBJECT_CONDITIONS_GROUP, NULL },
	{ NULL }
};
