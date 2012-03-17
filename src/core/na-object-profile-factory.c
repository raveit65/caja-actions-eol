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

extern NADataDef data_def_id [];			/* defined in na-object-id-factory.c */
extern NADataDef data_def_conditions [];	/* defined in na-iconditions-factory.c */

static NADataDef data_def_profile [] = {

	{ NAFO_DATA_DESCNAME,
				TRUE,
				TRUE,
				TRUE,
				N_( "Name of the profile" ),
				N_( "May be used as a description for the function of the profile.\n" \
					"If not set, it defaults to an auto-generated name." ),
				NAFD_TYPE_LOCALE_STRING,
				"",
				TRUE,
				TRUE,
				FALSE,
				TRUE,
				"desc-name",
				"Name",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_PATH,
				TRUE,
				TRUE,
				TRUE,
				N_( "Path of the command" ),
				N_( "The path of the command to be executed when the user select the menu item " \
					"in the file manager context menu or in the toolbar." ),
				NAFD_TYPE_STRING,
				"",
				TRUE,
				TRUE,
				TRUE,
				FALSE,
				"path",
				"Exec",
				'x',
				"command",
				0,
				G_OPTION_ARG_FILENAME,
				NULL,
				N_( "<PATH>" ) },

	{ NAFO_DATA_PARAMETERS,
				TRUE,
				TRUE,
				TRUE,
				N_( "Parameters of the command" ),
										/* too long string for iso c: 665 (max=509) */
				N_( "The parameters of the command to be executed when the user selects the menu " \
					"item in the file manager context menu or in the toolbar.\n" \
					"The parameters may contain some special tokens which are replaced by the " \
					"informations provided by the file manager before starting the command:\n" \
					"%d: base folder of the selected file(s)\n" \
					"%f: the name of the selected file or the first one if several are selected\n" \
					"%h: hostname of the URI\n" \
					"%m: space-separated list of the basenames of the selected file(s)/folder(s)\n" \
					"%M: space-separated list of the selected file(s)/folder(s), with their full paths\n" \
					"%p: port number of the first URI\n" \
					"%R: space-separated list of selected URIs\n" \
					"%s: scheme of the URI\n" \
					"%u: URI\n" \
					"%U: username of the URI\n" \
					"%%: a percent sign." ),
				NAFD_TYPE_STRING,
				"",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"parameters",
				NULL,
				'p',
				"parameters",
				0,
				G_OPTION_ARG_STRING,
				NULL,
				N_( "<STRING>" ) },

	{ NULL },
};

NADataGroup profile_data_groups [] = {
	{ NA_FACTORY_OBJECT_ID_GROUP,         data_def_id },
	{ NA_FACTORY_OBJECT_PROFILE_GROUP,    data_def_profile },
	{ NA_FACTORY_OBJECT_CONDITIONS_GROUP, data_def_conditions },
	{ NULL }
};
