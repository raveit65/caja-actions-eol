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

#ifndef __CORE_NA_EXPORTER_H__
#define __CORE_NA_EXPORTER_H__

/**
 * SECTION: na_iexporter
 * @short_description: #NAIExporter internal functions.
 * @include: core/na-exporter.h
 */

#include <api/na-object-api.h>

#include <core/na-pivot.h>

G_BEGIN_DECLS

GList *na_exporter_get_formats ( const NAPivot *pivot );
void   na_exporter_free_formats( GList *formats );

gchar *na_exporter_to_buffer( const NAPivot *pivot, const NAObjectItem *item, GQuark format, GSList **messages );
gchar *na_exporter_to_file  ( const NAPivot *pivot, const NAObjectItem *item, const gchar *uri, GQuark format, GSList **messages );

G_END_DECLS

#endif /* __CORE_NA_EXPORTER_H__ */
