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

#include <api/na-data-types.h>

typedef struct {
	guint  type;
	gchar *mateconf_dump_key;
}
	FactoryType;

static FactoryType st_factory_type[] = {
		{ NAFD_TYPE_STRING,        "string" },
		{ NAFD_TYPE_LOCALE_STRING, "string" },
		{ NAFD_TYPE_BOOLEAN,       "bool" },
		{ NAFD_TYPE_STRING_LIST,   "list" },
		{ NAFD_TYPE_POINTER,        NULL },
		{ NAFD_TYPE_UINT,          "int" },
		{ 0 }
};

/**
 * na_data_types_get_mateconf_dump_key:
 * @type: the FactoryData type.
 *
 * Returns: the MateConf key suitable for this type.
 *
 * The returned key is owned by FactoryData, and should not be released
 * by the caller.
 */
const gchar *
na_data_types_get_mateconf_dump_key( guint type )
{
	FactoryType *str;

	str = st_factory_type;
	while( str->type ){
		if( str->type == type ){
			return( str->mateconf_dump_key );
		}
		str++;
	}

	return( NULL );
}
