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

#ifndef __CAXML_WRITER_H__
#define __CAXML_WRITER_H__

/**
 * SECTION: caxml_writer
 * @short_description: #CAXMLWriter class definition.
 * @include: io-xml/caxml-writer.h
 *
 * This class exports Caja-Actions actions and menus as XML files.
 */

#include <api/na-data-boxed.h>
#include <api/na-iexporter.h>
#include <api/na-ifactory-provider.h>

G_BEGIN_DECLS

#define CAXML_WRITER_TYPE                ( caxml_writer_get_type())
#define CAXML_WRITER( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CAXML_WRITER_TYPE, CAXMLWriter ))
#define CAXML_WRITER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CAXML_WRITER_TYPE, CAXMLWriterClass ))
#define CAXML_IS_WRITER( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CAXML_WRITER_TYPE ))
#define CAXML_IS_WRITER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CAXML_WRITER_TYPE ))
#define CAXML_WRITER_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CAXML_WRITER_TYPE, CAXMLWriterClass ))

typedef struct _CAXMLWriterPrivate       CAXMLWriterPrivate;

typedef struct {
	/*< private >*/
	GObject             parent;
	CAXMLWriterPrivate *private;
}
	CAXMLWriter;

typedef struct _CAXMLWriterClassPrivate  CAXMLWriterClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass             parent;
	CAXMLWriterClassPrivate *private;
}
	CAXMLWriterClass;

GType  caxml_writer_get_type( void );

guint  caxml_writer_export_to_buffer( const NAIExporter *instance, NAIExporterBufferParmsv2 *parms );
guint  caxml_writer_export_to_file  ( const NAIExporter *instance, NAIExporterFileParmsv2 *parms );

guint  caxml_writer_write_start( const NAIFactoryProvider *writer, void *writer_data, const NAIFactoryObject *object, GSList **messages  );
guint  caxml_writer_write_data ( const NAIFactoryProvider *writer, void *writer_data, const NAIFactoryObject *object, const NADataBoxed *boxed, GSList **messages );
guint  caxml_writer_write_done ( const NAIFactoryProvider *writer, void *writer_data, const NAIFactoryObject *object, GSList **messages  );

G_END_DECLS

#endif /* __CAXML_WRITER_H__ */
