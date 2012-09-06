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

NADataDef data_def_conditions [] = {

	{ NAFO_DATA_BASENAMES,
				TRUE,
				TRUE,
				TRUE,
				N_( "List of patterns to be matched against the selected file(s)/folder(s)" ),
				N_( "A list of strings with joker '*' or '?' to be matched against the name(s) " \
					"of the selected file(s)/folder(s). Each selected items must match at least " \
					"one of the filename patterns for the action or the menu be candidate to " \
					"display.\n" \
					"This obviously only applies when there is a selection.\n" \
					"Defaults to '*'." ),
				NAFD_TYPE_STRING_LIST,
				"[*]",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"basenames",
				"Basenames",
				'b',
				"basename",
				0,
				G_OPTION_ARG_STRING_ARRAY,
				N_( "A pattern to be matched against basenames of selected file(s)/folder(s). " \
					"May include wildcards (* or ?). " \
					"You must set one option for each pattern you need" ),
				N_( "<EXPR>" ) },

	{ NAFO_DATA_MATCHCASE,
				TRUE,
				TRUE,
				TRUE,
				N_( "Whether the specified basenames are case sensitive (default)" ),
				N_( "Must be set to 'true' if the filename patterns are case sensitive, to 'false' " \
					"otherwise. E.g., if you need to match a filename in a case-sensitive manner, " \
					"set this key to 'true'. If you also want, for example '*.jpg' to match 'photo.JPG', " \
					"then set 'false'.\n" \
					"This obviously only applies when there is a selection.\n" \
					"Defaults to 'true'." ),
				NAFD_TYPE_BOOLEAN,
				"true",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"matchcase",
				"Matchcase",
				'a',
				"match-case",
				0,
				G_OPTION_ARG_NONE,
				NULL,
				NULL },

	{ NAFO_DATA_MIMETYPES,
				TRUE,
				TRUE,
				TRUE,
				N_( "List of patterns to be matched against the mimetypes of the selected file(s)/folder(s)" ),
				N_( "A list of strings with joker '*' to be matched against the mimetypes of the " \
					"selected file(s)/folder(s). Each selected items must match at least one of " \
					"the mimetype patterns for the action to appear.\n" \
					"This obviously only applies when there is a selection.\n" \
					"Defaults to '*'." ),
				NAFD_TYPE_STRING_LIST,
				"[*]",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"mimetypes",
				"MimeTypes",
				'm',
				"mimetype",
				0,
				G_OPTION_ARG_STRING_ARRAY,
				N_( "A pattern to be matched against mimetypes of selected file(s)/folder(s). " \
					"May include wildcards (* or ?). " \
					"You must set one option for each pattern you need" ),
				N_( "<EXPR>" ) },

	{ NAFO_DATA_ISFILE,
				TRUE,
				TRUE,
				TRUE,
				N_( "Whether the profile applies to files" ),
				N_( "Set to 'true' if the selection can have files, to 'false' otherwise.\n" \
					"This setting is tied in with the 'isdir' setting. The valid combinations are: \n" \
					"isfile=TRUE and isdir=FALSE: the selection may hold only files\n" \
					"isfile=FALSE and isdir=TRUE: the selection may hold only folders\n" \
					"isfile=TRUE and isdir=TRUE: the selection may hold both files and folders\n" \
					"isfile=FALSE and isdir=FALSE: this is an invalid combination " \
					"(your configuration will never appear).\n" \
					"This obviously only applies when there is a selection.\n" \
					"Defaults to 'true'." ),
				NAFD_TYPE_BOOLEAN,
				"true",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"isfile",
				NULL,
				'f',
				"accept-files",
				0,
				G_OPTION_ARG_NONE,
				NULL,
				NULL },

	{ NAFO_DATA_ISDIR,
				TRUE,
				TRUE,
				TRUE,
				N_( "Whether the profile applies to folders" ),
				N_( "Set to 'true' if the selection can have folders, to 'false' otherwise.\n" \
					"This setting is tied in with the 'isfile' setting. The valid combinations are: \n" \
					"isfile=TRUE and isdir=FALSE: the selection may hold only files\n" \
					"isfile=FALSE and isdir=TRUE: the selection may hold only folders\n" \
					"isfile=TRUE and isdir=TRUE: the selection may hold both files and folders\n" \
					"isfile=FALSE and isdir=FALSE: this is an invalid combination " \
					"(your configuration will never appear).\n" \
					"This obviously only applies when there is a selection.\n" \
					"Defaults to 'false'." ),
				NAFD_TYPE_BOOLEAN,
				"false",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"isdir",
				NULL,
				'd',
				"accept-dirs",
				0,
				G_OPTION_ARG_NONE,
				NULL,
				NULL },

	{ NAFO_DATA_MULTIPLE,
				TRUE,
				TRUE,
				TRUE,
				N_( "Whether the selection may be multiple" ),
				N_( "If you need more than one files or folders to be selected, set this " \
					"key to 'true'. If you want just one file or folder, set it to 'false'.\n" \
					"This obviously only applies when there is a selection.\n" \
					"Defaults to 'false'." ),
				NAFD_TYPE_BOOLEAN,
				"false",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"accept-multiple-files",
				NULL,
				'u',
				"accept-multiple",
				0,
				G_OPTION_ARG_NONE,
				NULL,
				NULL },

	{ NAFO_DATA_SCHEMES,
				TRUE,
				TRUE,
				TRUE,
				N_( "List of schemes to be matched against those of selected file(s)/folder(s)" ),
										/* too long string for iso c: 510 (max=509) */
				N_( "Defines the list of valid schemes to be matched against the selected " \
					"items. The scheme is the protocol used to access the files. The " \
					"keyword to use is the one used in the URI by the file manager.\n" \
					"Examples of valid URI include:\n" \
					"- file:///tmp/foo.txt\n" \
					"- sftp:///root@test.example.net/tmp/foo.txt\n" \
					"The most common schemes are:\n" \
					"'file': local files\n" \
					"'sftp': files accessed via SSH\n" \
					"'ftp': files accessed via FTP\n" \
					"'smb': files accessed via Samba (Windows share)\n" \
					"'dav': files accessed via WebDAV.\n" \
					"All schemes used by your favorite file manager may be used here.\n" \
					"This obviously only applies when there is a selection, " \
					"or when targeting the special 'x-caja-desktop' scheme.\n" \
					"Defaults to 'file'." ),
				NAFD_TYPE_STRING_LIST,
				"[file]",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"schemes",
				"Schemes",
				's',
				"scheme",
				0,
				G_OPTION_ARG_STRING,
				N_( "A valid GIO scheme where the selected file(s)/folder(s) should be located. " \
					"You must set one option for each scheme you need" ),
				N_( "<STRING>" ) },

	{ NAFO_DATA_FOLDERS,
				TRUE,
				TRUE,
				TRUE,
				N_( "List of folders" ),
				N_( "Defines the list of valid paths to be matched against the current folder.\n " \
					"All folders 'under' the specified path are considered valid.\n" \
					"Defaults to '/'." ),
				NAFD_TYPE_STRING_LIST,
				"[/]",
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"folders",
				"Folders",
				'r',
				"folder",
				0,
				G_OPTION_ARG_STRING,
				N_( "The path of a (parent) directory for which the item will be displayed. " \
					"You must set one option for each folder you need" ),
				N_( "<PATH>" ) },

	{ NULL },
};
