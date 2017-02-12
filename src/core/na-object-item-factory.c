/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
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

/*
 * As of 3.2 non copyables data are:
 * - the type
 * - non localized icon name
 * - subitems pointers list and string list
 * - read-only status
 */

NADataDef data_def_item [] = {

	/* this data is marked as non readable as it has to be read specifically
	 * in order to be able to create the corresponding NAObjectItem-derived
	 * object
	 * it is not writable as different I/O providers may have different values
	 * for it, and thus it must be written specifically
	 * (cf. nagp_writer_write_start(), cadp_writer_write_start())
	 * it is not instantiated because we never need to have an actual value
	 * (actually being determined at runtime by object class)
	 * it is just left here to be able to define the corresponding MateConf schema
	 * and to export it as XML
	 */
	{ NAFO_DATA_TYPE,
				FALSE,
				FALSE,
				FALSE,
				N_( "Type of the item" ),
				N_( "Defines if the item is an action or a menu. Possible values are :\n" \
					"- 'Action',\n" \
					"- 'Menu'.\n" \
					"The value is case sensitive and must not be localized." ),
				NA_DATA_TYPE_STRING,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"type",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	/* this data is common between actions and menus
	 * so default value is directly set in na_object_action_new_with_defaults()
	 * and na_object_menu_new_with_defaults()
	 */
	{ NAFO_DATA_LABEL,
				TRUE,
				TRUE,
				TRUE,
				N_( "Label of the context menu item (mandatory)" ),
				N_( "The label of the menu item that will appear in the file manager context " \
					"menu when the selection matches the appearance condition settings.\n" \
					"It is also used as a default for the toolbar label of an action." ),
				NA_DATA_TYPE_LOCALE_STRING,
				N_( "Empty label" ),
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				TRUE,
				"label",
				"Name",
				'l',
				"label",
				0,
				G_OPTION_ARG_STRING,
				NULL,
				N_( "<STRING>" ) },

	{ NAFO_DATA_TOOLTIP,
				TRUE,
				TRUE,
				TRUE,
				N_( "Tooltip of the context menu item" ),
				N_( "The tooltip of the menu item that will appear in the file manager " \
					"statusbar when the user points to the file manager context menu item " \
					"with his/her mouse." ),
				NA_DATA_TYPE_LOCALE_STRING,
				"",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				TRUE,
				"tooltip",
				"Tooltip",
				't',
				"tooltip",
				0,
				G_OPTION_ARG_STRING,
				NULL,
				N_( "<STRING>" ) },

	{ NAFO_DATA_ICON,
				TRUE,
				TRUE,
				TRUE,
				N_( "Icon of the context menu item" ),
				N_( "The icon of the menu item that will appear next to the label " \
					"in the file manager context menu when the selection matches the appearance " \
					"conditions settings.\n" \
					"May be the localized name of a themed icon, or a full path to any appropriate image." ),
				NA_DATA_TYPE_LOCALE_STRING,
				"",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				TRUE,
				"icon",
				"Icon",
				'i',
				"icon",
				0,
				G_OPTION_ARG_STRING,
				NULL,
				N_( "<PATH|NAME>" ) },

	/* icon used to be not localized up to and included 2.29.4
	 */
	{ NAFO_DATA_ICON_NOLOC,
				TRUE,
				FALSE,
				FALSE,
				"Unlocalized icon name or path",
				NULL,
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"icon",
				NULL,
				0,
				"icon",
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_DESCRIPTION,
				TRUE,
				TRUE,
				TRUE,
				N_( "Description relative to the item" ),
				N_( "Some text which explains the goal of the menu or the action.\n" \
					"May be used, e.g. when displaying available items on a web site." ),
				NA_DATA_TYPE_LOCALE_STRING,
				"",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				TRUE,
				"description",
				"Description",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_SHORTCUT,
				TRUE,
				TRUE,
				TRUE,
				N_( "Suggested shortcut" ),
				N_( "A shortcut suggested for the action or the menu.\n" \
					"Please note that this might be only a suggestion as the shortcut may " \
					"be already reserved for another use. Implementation should not override " \
					"an already existing shortcut to define this one.\n" \
					"The format may look like \"<Control>a\" or \"<Shift><Alt>F1\".\n" \
					"Defaults to empty." ),
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"shortcut",
				"SuggestedShortcut",
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	/* dynamic data, so not readable / not writable
	 */
	{ NAFO_DATA_SUBITEMS,
				FALSE,			/* not serializable */
				FALSE,
				TRUE,
				"Subitems",
				"List of subitems objects",
				NA_DATA_TYPE_POINTER,
				NULL,
				FALSE,
				FALSE,			/* not copyable */
				FALSE,			/* not comparable */
				FALSE,			/* not mandatory */
				FALSE,			/* not localized */
				NULL,
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	/* list of subitems as a string list
	 * dynamically rebuilt on write_start()
	 */
	{ NAFO_DATA_SUBITEMS_SLIST,
				TRUE,
				TRUE,
				TRUE,
				N_( "List of subitem ids" ),
				N_( "Ordered list of the IDs of the subitems. This may be actions or menus " \
					"if the item is a menu, or profiles if the item is an action.\n" \
					"If this list doesn't exist or is empty for an action or a menu, " \
					"subitems are attached in the order of the read operations." ),
				NA_DATA_TYPE_STRING_LIST,
				NULL,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				"items",
				NULL,		/* Profiles or ItemsList */
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NAFO_DATA_ENABLED,
				TRUE,
				TRUE,
				TRUE,
				N_( "Whether the action or the menu is enabled" ),
				N_( "If the or the menu action is disabled, it will never appear in the " \
					"file manager context menu.\n" \
					"Defaults to TRUE." ),
				NA_DATA_TYPE_BOOLEAN,
				"true",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"enabled",
				"Enabled",
				'e',
				"enabled",
				0,
				G_OPTION_ARG_NONE,
				N_( "Whether the action or the menu is enabled [enabled]" ),
				NULL },

	/* dynamic data, so non readable / non writable
	 * must be set by the NAIIOProvider when reading the item
	 */
	{ NAFO_DATA_READONLY,
				FALSE,
				FALSE,
				TRUE,
				"Read-only",
				"Is the item only readable ? " \
				"This is an intrinsic property, dynamically set when the item is unserialized. " \
				"This property being FALSE doesn't mean that the item will actually be updatable, " \
				"as this also depend of parameters set by user and administrator. " \
				"Also, a property initially set to FALSE when first unserializing may be set to" \
				"TRUE if an eccor occurs on a later write operation.",
				NA_DATA_TYPE_BOOLEAN,
				"false",
				FALSE,
				FALSE,
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

	/* dynamic data, so non readable / non writable
	 */
	{ NAFO_DATA_PROVIDER,
				FALSE,
				FALSE,
				TRUE,
				"I/O provider",
				"A pointer to the NAIOProvider object.",
				NA_DATA_TYPE_POINTER,
				NULL,
				FALSE,
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

	/* dynamic data, so non readable / non writable (but has property)
	 * is left at the NAIIOProvider disposition
	 * not copyable as directly duplicated by NAIIOProvider interface
	 * (see NAObjectItem::object_copy)
	 */
	{ NAFO_DATA_PROVIDER_DATA,
				FALSE,
				FALSE,
				TRUE,
				"I/O provider data",
				"A pointer to some NAIOProvider specific data.",
				NA_DATA_TYPE_POINTER,
				NULL,
				FALSE,
				FALSE,
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

	/* this integer version number is introduced with .desktop files and obsoletes
	 * the previous string version number which was only set on actions
	 * we so have: "1.0" < "1.1" < "2.0" < 3
	 * Also note that iversion=3 is only written in MateConf, while .desktop files
	 * do not have this key.
	 * Only when version will need to be incremented again, we will write a version
	 * key in .desktop files (probably 1 or 2), and increment this iversion to 4.
	 * As this version number will not be the same in .desktop and in MateConf, it
	 * is marked as not automatically writable: it has to be written specifically
	 * by each i/o provider.
	 */
	{ NAFO_DATA_IVERSION,
				TRUE,
				FALSE,
				TRUE,
				N_( "Version of the format" ),
				N_( "The version of the configuration format that will be used to manage backward compatibility." ),
				NA_DATA_TYPE_UINT,
				"3",
				FALSE,
				TRUE,
				TRUE,
				FALSE,
				FALSE,
				"iversion",
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	{ NULL },
};
