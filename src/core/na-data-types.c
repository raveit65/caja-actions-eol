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

#include <api/na-data-types.h>
#include <api/na-core-utils.h>

typedef struct {
	guint  type;
	gchar *mateconf_dump_key;
	gchar *mateconf_secondary_key;
}
	FactoryType;

static FactoryType st_factory_type[] = {
		{ NA_DATA_TYPE_BOOLEAN,       "bool",   NULL },
		{ NA_DATA_TYPE_POINTER,        NULL,    NULL },
		{ NA_DATA_TYPE_STRING,        "string", NULL },
		{ NA_DATA_TYPE_STRING_LIST,   "list",   "string" },
		{ NA_DATA_TYPE_LOCALE_STRING, "string", NULL },
		{ NA_DATA_TYPE_UINT,          "int",    NULL },
		{ NA_DATA_TYPE_UINT_LIST,     "list",   "int" },
		{ 0 }
};

/**
 * na_data_types_get_mateconf_dump_key:
 * @type: the FactoryData type.
 *
 * Returns: the MateConf key suitable for this type.
 *
 * The returned key is owned by the factory data management system, and
 * should not be released by the caller.
 *
 * Since: 2.30
 */
const gchar *
na_data_types_get_mateconf_dump_key( guint type )
{
	static const gchar *thisfn = "na_data_types_get_mateconf_dump_key";
	FactoryType *str;

	str = st_factory_type;
	while( str->type ){
		if( str->type == type ){
			return( str->mateconf_dump_key );
		}
		str++;
	}

	g_warning( "%s: unknown data type: %d", thisfn, type );
	return( NULL );
}
