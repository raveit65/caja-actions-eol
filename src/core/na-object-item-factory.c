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

NADataDef data_def_item [] = {

	/* this data is marked as non readable as it has to be readen specifically
	 * in order to be able to create the corresponding NAObjectItem-derived object
	 * it is not writable as different I/O providers have different values for it
	 * it is not instantiated
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
				NAFD_TYPE_STRING,
				NULL,
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
				NAFD_TYPE_LOCALE_STRING,
				"",
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
				NAFD_TYPE_LOCALE_STRING,
				"",
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
				NAFD_TYPE_LOCALE_STRING,
				"",
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

	{ NAFO_DATA_DESCRIPTION,
				TRUE,
				TRUE,
				TRUE,
				N_( "Description relative to the item" ),
				N_( "Some text which explains the goal of the menu or the action.\n" \
					"May be used, e.g. when displaying available items on a web site." ),
				NAFD_TYPE_LOCALE_STRING,
				"",
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

	/* dynamic data, so not readable / not writable
	 */
	{ NAFO_DATA_SUBITEMS,
				FALSE,			/* not serializable */
				FALSE,
				TRUE,
				"Subitems",
				"List of subitems objects",
				NAFD_TYPE_POINTER,
				NULL,
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
				NAFD_TYPE_STRING_LIST,
				NULL,
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
				N_( "Whether the action or the menu is enabled (default)" ),
				N_( "If the or the menu action is disabled, it will never appear in the " \
					"file manager context menu.\n" \
					"Defaults to TRUE." ),
				NAFD_TYPE_BOOLEAN,
				"true",
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
				NULL,
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
				NAFD_TYPE_BOOLEAN,
				"false",
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

	/* dynamic data, so non readable / non writable
	 */
	{ NAFO_DATA_PROVIDER,
				FALSE,
				FALSE,
				TRUE,
				"I/O provider",
				"A pointer to the NAIOProvider object.",
				NAFD_TYPE_POINTER,
				NULL,
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
	 */
	{ NAFO_DATA_PROVIDER_DATA,
				FALSE,
				FALSE,
				TRUE,
				"I/O provider data",
				"A pointer to some NAIOProvider specific data.",
				NAFD_TYPE_POINTER,
				NULL,
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
