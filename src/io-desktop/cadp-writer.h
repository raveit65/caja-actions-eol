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

#ifndef __CADP_WRITER_H__
#define __CADP_WRITER_H__

#include <api/na-iio-provider.h>
#include <api/na-iexporter.h>
#include <api/na-ifactory-provider.h>

G_BEGIN_DECLS

gboolean cadp_iio_provider_is_willing_to_write ( const NAIIOProvider *provider );
gboolean cadp_iio_provider_is_able_to_write    ( const NAIIOProvider *provider );

guint    cadp_iio_provider_write_item          ( const NAIIOProvider *provider, const NAObjectItem *item, GSList **messages );
guint    cadp_iio_provider_delete_item         ( const NAIIOProvider *provider, const NAObjectItem *item, GSList **messages );
guint    cadp_iio_provider_duplicate_data      ( const NAIIOProvider *provider, NAObjectItem *dest, const NAObjectItem *source, GSList **messages );

guint    cadp_writer_iexporter_export_to_buffer( const NAIExporter *instance, NAIExporterBufferParmsv2 *parms );
guint    cadp_writer_iexporter_export_to_file  ( const NAIExporter *instance, NAIExporterFileParmsv2 *parms );

guint    cadp_writer_ifactory_provider_write_start(
				const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object,
				GSList **messages  );

guint    cadp_writer_ifactory_provider_write_data(
				const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object,
				const NADataBoxed *boxed, GSList **messages );

guint    cadp_writer_ifactory_provider_write_done(
				const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object,
				GSList **messages  );

G_END_DECLS

#endif /* __CADP_WRITER_H__ */
