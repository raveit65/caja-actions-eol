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

#include <api/na-ifactory-object-data.h>
#include <api/na-data-def.h>
#include <api/na-data-types.h>

/*
 * As of 3.2 non copyables data are:
 * - the parent pointer
 */

NADataDef data_def_id [] = {

	/* this data is marked non readable / non writable as it has to be read
	 * written specifically when serializing / deserializing items
	 */
	{ NAFO_DATA_ID,
				FALSE,
				FALSE,
				TRUE,
				"NAObjectId identifier",
				"Internal identifier of the NAObjectId object. " \
				"Historically a UUID used as a MateConf directory (thus ASCII, case insensitive), " \
				"it is also the basename of the .desktop file (thus UTF-8, case sensitive).",
				NA_DATA_TYPE_STRING,
				"",
				FALSE,
				TRUE,
				TRUE,
				TRUE,			/* mandatory */
				FALSE,
				NULL,
				NULL,
				0,
				NULL,
				0,
				0,
				NULL,
				NULL },

	/* dynamic data, so not readable / not writable
	 */
	{ NAFO_DATA_PARENT,
				FALSE,
				FALSE,
				TRUE,
				"NAObjectId Parent",
				"The NAObjectItem which is the parent of this object.",
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

	{ NULL },
};
