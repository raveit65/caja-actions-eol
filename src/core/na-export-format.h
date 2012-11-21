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

#ifndef __CORE_NA_EXPORT_FORMAT_H__
#define __CORE_NA_EXPORT_FORMAT_H__

/* @title: NAExportFormat
 * @short_description: The #NAExportFormat Class Definition
 * @include: core/na-export-format.h
 */

#include <api/na-iexporter.h>

G_BEGIN_DECLS

#define NA_TYPE_EXPORT_FORMAT                ( na_export_format_get_type())
#define NA_EXPORT_FORMAT( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_EXPORT_FORMAT, NAExportFormat ))
#define NA_EXPORT_FORMAT_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_EXPORT_FORMAT, NAExportFormatClass ))
#define NA_IS_EXPORT_FORMAT( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_EXPORT_FORMAT ))
#define NA_IS_EXPORT_FORMAT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_EXPORT_FORMAT ))
#define NA_EXPORT_FORMAT_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_EXPORT_FORMAT, NAExportFormatClass ))

typedef struct _NAExportFormatPrivate        NAExportFormatPrivate;

typedef struct {
	GObject                parent;
	NAExportFormatPrivate *private;
}
	NAExportFormat;

typedef struct _NAExportFormatClassPrivate   NAExportFormatClassPrivate;

typedef struct {
	GObjectClass                parent;
	NAExportFormatClassPrivate *private;
}
	NAExportFormatClass;

GType           na_export_format_get_type    ( void );

NAExportFormat *na_export_format_new         ( const NAIExporterFormatv2 *exporter_format );

NAIExporter    *na_export_format_get_provider( const NAExportFormat *format );

G_END_DECLS

#endif /* __CORE_NA_EXPORT_FORMAT_H__ */
