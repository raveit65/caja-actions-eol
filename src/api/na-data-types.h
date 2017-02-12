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

#ifndef __CAJA_ACTIONS_API_NA_DATA_TYPES_H__
#define __CAJA_ACTIONS_API_NA_DATA_TYPES_H__

/**
 * SECTION: data-type
 * @title: NADataType
 * @short_description: The Data Factory Type Definitions
 * @include: caja-actions/na-data-types.h
 */

#include <glib.h>

G_BEGIN_DECLS

/**
 * NADataType:
 * @NA_DATA_TYPE_BOOLEAN:       a boolean
 *                              can be initialized with "true" or "false" (case insensitive)
 * @NA_DATA_TYPE_POINTER:       a ( void * ) pointer
 * @NA_DATA_TYPE_STRING:        an ASCII string
 * @NA_DATA_TYPE_STRING_LIST:   a list of ASCII strings
 * @NA_DATA_TYPE_LOCALE_STRING: a localized UTF-8 string
 * @NA_DATA_TYPE_UINT:          an unsigned integer
 * @NA_DATA_TYPE_UINT_LIST:     a list of unsigned integers
 *
 * Each elementary data which would take advantage of #NABoxed facilities
 * should be typed at instanciation time.
 *
 * #NAIFactoryProvider implementations should provide a primitive for reading
 * (resp. writing) a value for each of these elementary data types.
 *
 * <note>
 *   <para>
 * Please note that this enumeration may be compiled in by the extensions.
 * They must so remain fixed, unless you are prepared to see strange effects
 * (e.g. an extension has been compiled with %NA_DATA_TYPE_STRING = 2, while you
 * have inserted another element, making it to 3 !) - or you know what
 * you are doing...
 *   </para>
 *   <para>
 *     So, only add new items at the end of the enum. You have been warned!
 *   </para>
 * </note>
 *
 * Since: 2.30
 */
typedef enum {
	NA_DATA_TYPE_BOOLEAN = 1,
	NA_DATA_TYPE_POINTER,
	NA_DATA_TYPE_STRING,
	NA_DATA_TYPE_STRING_LIST,
	NA_DATA_TYPE_LOCALE_STRING,
	NA_DATA_TYPE_UINT,
	NA_DATA_TYPE_UINT_LIST,
	/*< private >*/
	/* count of defined types */
	NA_DATA_TYPE_N
}
	NADataType;

const gchar *na_data_types_get_mateconf_dump_key( guint type );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_DATA_TYPES_H__ */
