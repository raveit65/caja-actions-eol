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

#ifndef __CAXML_KEYS_H__
#define __CAXML_KEYS_H__

#include <api/na-data-def.h>

G_BEGIN_DECLS

/* XML element names (MateConf schema)
 * used in MATECONF_SCHEMA_V1 and MATECONF_SCHEMA_V2
 *
 * Up to v 1.10, export used to contain a full schema description,
 * while import only checked for applyto keys (along with locale
 * and default)
 *
 * Starting with 1.11, we have introduced a lighter export schema
 * (without owner and short and long descriptions)
 */
#define CAXML_KEY_SCHEMA_ROOT					"mateconfschemafile"
#define CAXML_KEY_SCHEMA_LIST					"schemalist"
#define CAXML_KEY_SCHEMA_NODE					"schema"

#define CAXML_KEY_SCHEMA_NODE_KEY				"key"
#define CAXML_KEY_SCHEMA_NODE_APPLYTO			"applyto"
#define CAXML_KEY_SCHEMA_NODE_OWNER				"owner"			/* v1 only */
#define CAXML_KEY_SCHEMA_NODE_TYPE				"type"
#define CAXML_KEY_SCHEMA_NODE_LISTTYPE			"list_type"
#define CAXML_KEY_SCHEMA_NODE_LOCALE			"locale"
#define CAXML_KEY_SCHEMA_NODE_DEFAULT			"default"

#define CAXML_KEY_SCHEMA_NODE_LOCALE_DEFAULT	"default"
#define CAXML_KEY_SCHEMA_NODE_LOCALE_SHORT		"short"			/* v1 only */
#define CAXML_KEY_SCHEMA_NODE_LOCALE_LONG		"long"			/* v1 only */

/* this structure is statically allocated (cf. caxml-keys.c)
 * and let us check the validity of each element node
 */
typedef struct {
	gchar   *key;						/* static data */
	gboolean v1;
	gboolean v2;

	gboolean reader_found;				/* dynamic data */
}
	CAXMLKeyStr;

/* XML element names (MateConf dump)
 * used in FORMAT_MATECONF_ENTRY
 */
#define CAXML_KEY_DUMP_ROOT							"mateconfentryfile"
#define CAXML_KEY_DUMP_LIST							"entrylist"
#define CAXML_KEY_DUMP_NODE							"entry"

#define CAXML_KEY_DUMP_LIST_PARM_BASE				"base"

#define CAXML_KEY_DUMP_NODE_KEY						"key"
#define CAXML_KEY_DUMP_NODE_VALUE					"value"

#define CAXML_KEY_DUMP_NODE_VALUE_TYPE_STRING		"string"
#define CAXML_KEY_DUMP_NODE_VALUE_LIST				"list"
#define CAXML_KEY_DUMP_NODE_VALUE_LIST_PARM_TYPE	"type"

G_END_DECLS

#endif /* __CAXML_KEYS_H__ */
