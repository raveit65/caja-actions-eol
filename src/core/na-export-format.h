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

#ifndef __CORE_NA_EXPORT_FORMAT_H__
#define __CORE_NA_EXPORT_FORMAT_H__

/**
 * SECTION: na_export_format
 * @short_description: #NAExportFormat class definition.
 * @include: core/na-export-format.h
 */

#include <api/na-iexporter.h>

G_BEGIN_DECLS

#define NA_EXPORT_FORMAT_TYPE					( na_export_format_get_type())
#define NA_EXPORT_FORMAT( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_EXPORT_FORMAT_TYPE, NAExportFormat ))
#define NA_EXPORT_FORMAT_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, NA_EXPORT_FORMAT_TYPE, NAExportFormatClass ))
#define NA_IS_EXPORT_FORMAT( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_EXPORT_FORMAT_TYPE ))
#define NA_IS_EXPORT_FORMAT_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_EXPORT_FORMAT_TYPE ))
#define NA_EXPORT_FORMAT_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_EXPORT_FORMAT_TYPE, NAExportFormatClass ))

typedef struct NAExportFormatPrivate      NAExportFormatPrivate;

typedef struct {
	GObject                parent;
	NAExportFormatPrivate *private;
}
	NAExportFormat;

typedef struct NAExportFormatClassPrivate NAExportFormatClassPrivate;

typedef struct {
	GObjectClass                parent;
	NAExportFormatClassPrivate *private;
}
	NAExportFormatClass;

#define IPREFS_EXPORT_FORMAT_ASK		g_quark_from_static_string( "Ask" )

GType           na_export_format_get_type( void );

NAExportFormat *na_export_format_new( const NAIExporterFormat *format, const NAIExporter *exporter );

GQuark          na_export_format_get_quark      ( const NAExportFormat *format );
gchar          *na_export_format_get_id         ( const NAExportFormat *format );
gchar          *na_export_format_get_label      ( const NAExportFormat *format );
gchar          *na_export_format_get_description( const NAExportFormat *format );
NAIExporter    *na_export_format_get_exporter   ( const NAExportFormat *format );

G_END_DECLS

#endif /* __CORE_NA_EXPORT_FORMAT_H__ */
