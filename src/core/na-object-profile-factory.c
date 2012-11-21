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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include <api/na-ifactory-object-data.h>
#include <api/na-data-def.h>
#include <api/na-data-types.h>

extern NADataDef data_def_id [];			/* defined in na-object-id-factory.c */
extern NADataDef data_def_conditions [];	/* defined in na-icontext-factory.c */

static NADataDef data_def_profile [] = {

	{ NAFO_DATA_DESCNAME,
				TRUE,
				TRUE,
				TRUE,
				N_( "Name of the profile" ),
				N_( "May be used as a description for the function of the profile.\n" \
					"If not set, it defaults to an auto-generated name." ),
				NA_DATA_TYPE_LOCALE_STRING,
				"",
				FALSE,
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

	/* label of the profile was unlocalized up to and included 1.11.0
	 */
	{ NAFO_DATA_DESCNAME_NOLOC,
				TRUE,
				FALSE,
				FALSE,
				"Unlocalized name of the profile",
				NULL,
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"desc-name",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	/* Path and Parameters are two separate data both in MateConf, in MateConf-derived
	 * export files and in CACT. Only in desktop files, they are merged as only
	 * one 'Exec' data which is splitted at read time.
	 */
	{ NAFO_DATA_PATH,
				TRUE,
				TRUE,
				TRUE,
				N_( "Path of the command" ),
				N_( "The path of the command to be executed when the user select the menu item " \
					"in the file manager context menu or in the toolbar." ),
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
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

	/* Desktop files not only introduced new properties conditions to item and profiles,
	 * but also slightly changed the meaning of some parameters. This is synchronized
	 * with the change of version data (from "2.0" string to '1' integer)
	 */
	{ NAFO_DATA_PARAMETERS,
				TRUE,
				TRUE,
				TRUE,
				N_( "Parameters of the command" ),
										/* too long string for iso c: (max=509) */
				N_( "The parameters of the command to be executed when the user selects the menu " \
					"item in the file manager context menu or in the toolbar.\n" \
					"The parameters may contain some special tokens which are replaced by the " \
					"informations provided by the file manager before starting the command:\n" \
					"- up to version 2.0:\n" \
					"  %d: base folder of the selected file(s)\n" \
					"  %f: the name of the selected file or the first one if several are selected\n" \
					"  %h: hostname of the URI\n" \
					"  %m: space-separated list of the basenames of the selected file(s)/folder(s)\n" \
					"  %M: space-separated list of the selected file(s)/folder(s), with their full paths\n" \
					"  %p: port number of the first URI\n" \
					"  %R: space-separated list of selected URIs\n" \
					"  %s: scheme of the URI\n" \
					"  %u: URI\n" \
					"  %U: username of the URI\n" \
					"  %%: a percent sign.\n" \
					"- starting from version 3:\n" \
					"  %b: (first) basename\n" \
					"  %B: space-separated list of the basenames of the selected file(s)/folder(s)\n" \
					"  %c: count the selected file(s)/folder(s)\n" \
					"  %d: (first) base directory\n" \
					"  %D: space-separated list of base directories of the selected file(s)/folder(s)\n" \
					"  %f: (first) filename\n" \
					"  %F: space-separated list of the filenames of the selected file(s)/folder(s)\n" \
					"  %h: hostname of the (first) URI\n" \
					"  %m: (first) mimetype\n" \
					"  %M: space-separated list of the mimetypes of the selected file(s)/folder(s)\n" \
					"  %n: username of the (first) URI\n" \
					"  %o: no-op operator which forces the singular form of execution (since 3.0.2)\n" \
					"  %O: no-op operator which forces the plural form of execution (since 3.0.2)\n" \
					"  %p: port number of the (first) URI\n" \
					"  %s: scheme of the (first) URI\n" \
					"  %u: (first) URI\n" \
					"  %U: space-separated list of the URIs of the selected file(s)/folder(s)\n" \
					"  %w: (first) basename without the extension\n" \
					"  %W: space-separated list of basenames without the extension\n" \
					"  %x: (first) extension\n" \
					"  %X: space-separated list of the extensions of the selected file(s)/folder(s)\n" \
					"  %%: a percent sign." ),
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
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

	{ NAFO_DATA_WORKING_DIR,
				TRUE,
				TRUE,
				TRUE,
				N_( "Working directory" ),
				N_( "The working directory the command will be started in.\n" \
					"Defaults to \"%d\"." ),
				NA_DATA_TYPE_STRING,
				"%d",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"working-dir",
				"Path",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_EXECUTION_MODE,
				TRUE,
				TRUE,
				TRUE,
				N_( "Execution mode" ),
				/* i18n: 'Normal', 'Terminal', 'Embedded' and 'DisplayOutput' are non-translatable keywords */
				N_( "Execution mode of the program.\n" \
					"This may be chosen between following values:\n" \
					"- Normal: starts as a standard graphical user interface\n" \
					"- Terminal: starts the preferred terminal of the graphical environment, " \
						"and runs the command in it\n" \
					"- Embedded: makes use of a special feature of the file manager which allows " \
						"a terminal to be ran inside of it; an acceptable fallback is Terminal\n" \
					"- DisplayOutput: the ran terminal may be closed at end of the command, but " \
						"standard streams (stdout, stderr) should be collected and displayed; " \
						"an acceptable fallback is Terminal.\n" \
					"Defaults to \"Normal\"." ),
				NA_DATA_TYPE_STRING,
				"Normal",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"execution-mode",
				"ExecutionMode",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_STARTUP_NOTIFY,
				TRUE,
				TRUE,
				TRUE,
				N_( "Startup notify" ),
				N_( "Only relevant when ExecutionMode=Normal.\n" \
					"Defaults to FALSE." ),
				NA_DATA_TYPE_BOOLEAN,
				"false",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"startup-notify",
				"StartupNotify",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_STARTUP_WMCLASS,
				TRUE,
				TRUE,
				TRUE,
				N_( "Startup WM Class" ),
				N_( "Only relevant when ExecutionMode=Normal.\n" \
					"Defaults to empty." ),
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"startup-wmclass",
				"StartupWMClass",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_EXECUTE_AS,
				TRUE,
				TRUE,
				TRUE,
				N_( "Execute as user" ),
				N_( "The user the command must be ran as. " \
					"The user may be identified by its numeric UID or by its login.\n" \
					"The profile is ignored if defined with a non-existing UID or login.\n" \
					"Defaults to empty: the command will be executed as the current user." ),
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"execute-as",
				"ExecuteAs",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NULL },
};

NADataGroup profile_data_groups [] = {
	{ NA_FACTORY_OBJECT_ID_GROUP,         data_def_id },
	{ NA_FACTORY_OBJECT_PROFILE_GROUP,    data_def_profile },
	{ NA_FACTORY_OBJECT_CONDITIONS_GROUP, data_def_conditions },
	{ NULL }
};
