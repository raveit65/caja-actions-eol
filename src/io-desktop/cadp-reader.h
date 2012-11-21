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

#ifndef __CADP_READER_H__
#define __CADP_READER_H__

#include <api/na-iio-provider.h>
#include <api/na-iimporter.h>
#include <api/na-ifactory-provider.h>

G_BEGIN_DECLS

GList       *cadp_iio_provider_read_items            ( const NAIIOProvider *provider, GSList **messages );

guint        cadp_reader_iimporter_import_from_uri   ( const NAIImporter *instance, void *parms_ptr );

void         cadp_reader_ifactory_provider_read_start( const NAIFactoryProvider *reader, void *reader_data, const NAIFactoryObject *serializable, GSList **messages );
NADataBoxed *cadp_reader_ifactory_provider_read_data ( const NAIFactoryProvider *reader, void *reader_data, const NAIFactoryObject *serializable, const NADataDef *iddef, GSList **messages );
void         cadp_reader_ifactory_provider_read_done ( const NAIFactoryProvider *reader, void *reader_data, const NAIFactoryObject *serializable, GSList **messages );

G_END_DECLS

#endif /* __CADP_READER_H__ */
