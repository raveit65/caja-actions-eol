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

#ifndef __CORE_NA_EXPORTER_H__
#define __CORE_NA_EXPORTER_H__

/* @title: NAIExporter
 * @short_description: The #NAIExporter Internal Functions
 * @include: core/na-exporter.h
 */

#include <api/na-iexporter.h>
#include <api/na-object-api.h>

#include "na-ioption.h"
#include "na-pivot.h"

G_BEGIN_DECLS

#define EXPORTER_FORMAT_ASK				"Ask"
#define EXPORTER_FORMAT_NOEXPORT		"NoExport"

GList       *na_exporter_get_formats    ( const NAPivot *pivot );
void         na_exporter_free_formats   ( GList *formats );
NAIOption   *na_exporter_get_ask_option ( void );

gchar       *na_exporter_to_buffer      ( const NAPivot *pivot,
                                          const NAObjectItem *item,
                                          const gchar *format,
                                          GSList **messages );

gchar       *na_exporter_to_file        ( const NAPivot *pivot,
                                          const NAObjectItem *item,
                                          const gchar *folder_uri,
                                          const gchar *format,
                                          GSList **messages );

NAIExporter *na_exporter_find_for_format( const NAPivot *pivot,
		                                  const gchar *format );

G_END_DECLS

#endif /* __CORE_NA_EXPORTER_H__ */
