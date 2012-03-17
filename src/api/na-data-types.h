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

#ifndef __CAJA_ACTIONS_API_NA_FACTORY_DATA_TYPES_H__
#define __CAJA_ACTIONS_API_NA_FACTORY_DATA_TYPES_H__

/**
 * SECTION: na_data
 * @short_description: NADataBoxed type definitions.
 * @include: caja-actions/na-data-types.h
 */

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * Elementary factory data types
 * Each elementary factory data must be typed as one of these
 * IFactoryProvider implementations should provide a primitive for reading
 * (resp. writing) a value for each of these elementary data types.
 *
 * IMPORTANT NOTE
 * Please note that this enumeration may  be compiled in by extensions.
 * They must so remain fixed, unless you want see strange effects (e.g.
 * an extension has been compiled with NAFD_TYPE_STRING = 2, while you
 * have inserted another element, making it to 3 !) - or you know what
 * you are doing...
 */

enum {
	NAFD_TYPE_STRING = 1,				/* an ASCII string */

	NAFD_TYPE_LOCALE_STRING,			/* a localized UTF-8 string */

	NAFD_TYPE_BOOLEAN,					/* a boolean
										 * can be initialized with "true" or "false" (case insensitive) */

	NAFD_TYPE_STRING_LIST,				/* a list of ASCII strings */

	NAFD_TYPE_POINTER,					/* a ( void * ) pointer
										 * should be initialized to NULL */

	NAFD_TYPE_UINT,						/* an unsigned integer */
};

const gchar *na_data_types_get_mateconf_dump_key( guint type );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_FACTORY_DATA_TYPES_H__ */
