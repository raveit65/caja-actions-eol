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

#ifndef __CORE_NA_FACTORY_PROVIDER_H__
#define __CORE_NA_FACTORY_PROVIDER_H__

/**
 * SECTION: na_ifactory_provider
 * @short_description: #NAIFactoryProvider interface definition.
 * @include: core/na-factory-provider.h
 *
 * Declare the function only accessed from core library (not published as API).
 */

#include <api/na-data-boxed.h>
#include <api/na-ifactory-provider.h>

G_BEGIN_DECLS

NADataBoxed *na_factory_provider_read_data ( const NAIFactoryProvider *reader, void *reader_data,
									const NAIFactoryObject *object, const NADataDef *def,
									GSList **messages );

guint        na_factory_provider_write_data( const NAIFactoryProvider *writer, void *writer_data,
									const NAIFactoryObject *object, const NADataBoxed *boxed,
									GSList **messages );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_FACTORY_PROVIDER_H__ */
