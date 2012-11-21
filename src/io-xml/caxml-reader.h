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

#ifndef __CAXML_READER_H__
#define __CAXML_READER_H__

/**
 * SECTION: caxml_reader
 * @short_description: #CAXMLReader class definition.
 * @include: caxml-reader.h
 *
 * This is the base class for importing items from XML files.
 *
 * If the imported file is not an XML one, with a known document root,
 * then we returned IMPORTER_CODE_NOT_WILLING_TO.
 * In all other cases, errors or inconsistancies are signaled, but
 * we do our best to actually import the file and produce a valuable
 * #NAObjectItem-derived object.
 */

#include <api/na-data-boxed.h>
#include <api/na-iimporter.h>
#include <api/na-ifactory-provider.h>

G_BEGIN_DECLS

#define CAXML_READER_TYPE                ( caxml_reader_get_type())
#define CAXML_READER( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CAXML_READER_TYPE, CAXMLReader ))
#define CAXML_READER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CAXML_READER_TYPE, CAXMLReaderClass ))
#define CAXML_IS_READER( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CAXML_READER_TYPE ))
#define CAXML_IS_READER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CAXML_READER_TYPE ))
#define CAXML_READER_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CAXML_READER_TYPE, CAXMLReaderClass ))

typedef struct _CAXMLReaderPrivate       CAXMLReaderPrivate;

typedef struct {
	/*< private >*/
	GObject             parent;
	CAXMLReaderPrivate *private;
}
	CAXMLReader;

typedef struct _CAXMLReaderClassPrivate  CAXMLReaderClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass             parent;
	CAXMLReaderClassPrivate *private;
}
	CAXMLReaderClass;

GType        caxml_reader_get_type( void );

guint        caxml_reader_import_from_uri( const NAIImporter *instance, void *parms_ptr );

void         caxml_reader_read_start( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, GSList **messages  );
NADataBoxed *caxml_reader_read_data ( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, const NADataDef *def, GSList **messages );
void         caxml_reader_read_done ( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, GSList **messages  );

G_END_DECLS

#endif /* __CAXML_READER_H__ */
