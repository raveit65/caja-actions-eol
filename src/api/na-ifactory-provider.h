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

#ifndef __CAJA_ACTIONS_API_NA_IFACTORY_PROVIDER_H__
#define __CAJA_ACTIONS_API_NA_IFACTORY_PROVIDER_H__

/**
 * SECTION: na_ifactory_provider
 * @short_description: #NAIFactoryProvider interface definition.
 * @include: caja-actions/na-ifactory_provider.h
 *
 * This is the interface used by data factory management system for
 * having serialization/unserialization services. This interface should
 * be implemented by I/O providers which would take advantage of this
 * system.
 *
 * Caja-Actions v 2.30 - API version:  1
 */

#include "na-data-boxed.h"
#include "na-ifactory-object.h"
#include "na-ifactory-provider-provider.h"

G_BEGIN_DECLS

#define NA_IFACTORY_PROVIDER_TYPE						( na_ifactory_provider_get_type())
#define NA_IFACTORY_PROVIDER( instance )				( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_IFACTORY_PROVIDER_TYPE, NAIFactoryProvider ))
#define NA_IS_IFACTORY_PROVIDER( instance )				( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_IFACTORY_PROVIDER_TYPE ))
#define NA_IFACTORY_PROVIDER_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_IFACTORY_PROVIDER_TYPE, NAIFactoryProviderInterface ))

typedef struct NAIFactoryProviderInterfacePrivate NAIFactoryProviderInterfacePrivate;

typedef struct {
	GTypeInterface                      parent;
	NAIFactoryProviderInterfacePrivate *private;

	/**
	 * get_version:
	 * @instance: this #NAIFactoryProvider instance.
	 *
	 * Returns: the version of this interface supported by @instance implementation.
	 *
	 * Defaults to 1.
	 */
	guint         ( *get_version )( const NAIFactoryProvider *instance );

	/**
	 * read_start:
	 * @reader: this #NAIFactoryProvider instance.
	 * @reader_data: the data associated to this instance.
	 * @object: the #NAIFactoryObject object which comes to be readen.
	 * @messages: a pointer to a #GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * API called by #NAIFactoryObject just before starting with reading data.
	 */
	void          ( *read_start ) ( const NAIFactoryProvider *reader, void *reader_data, const NAIFactoryObject *object, GSList **messages  );

	/**
	 * read_data:
	 * @reader: this #NAIFactoryProvider instance.
	 * @reader_data: the data associated to this instance.
	 * @object: the #NAIFactoryobject being unserialized.
	 * @def: a #NADataDef structure which identifies the data to be unserialized.
	 * @messages: a pointer to a #GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Returns: a newly allocated NADataBoxed which contains the readen value.
	 * Should return %NULL if data is not found.
	 *
	 * This method must be implemented in order any data be read.
	 */
	NADataBoxed * ( *read_data )  ( const NAIFactoryProvider *reader, void *reader_data, const NAIFactoryObject *object, const NADataDef *def, GSList **messages );

	/**
	 * read_done:
	 * @reader: this #NAIFactoryProvider instance.
	 * @reader_data: the data associated to this instance.
	 * @object: the #NAIFactoryObject object which comes to be readen.
	 * @messages: a pointer to a #GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * API called by #NAIFactoryObject when all data have been readen.
	 * Implementor may take advantage of this to do some cleanup.
	 */
	void          ( *read_done )  ( const NAIFactoryProvider *reader, void *reader_data, const NAIFactoryObject *object, GSList **messages  );

	/**
	 * write_start:
	 * @writer: this #NAIFactoryProvider instance.
	 * @writer_data: the data associated to this instance.
	 * @object: the #NAIFactoryObject object which comes to be written.
	 * @messages: a pointer to a #GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Returns: a NAIIOProvider operation return code.
	 *
	 * API called by #NAIFactoryObject just before starting with writing data.
	 */
	guint         ( *write_start )( const NAIFactoryProvider *writer, void *writer_data, const NAIFactoryObject *object, GSList **messages  );

	/**
	 * write_data:
	 * @writer: this #NAIFactoryProvider instance.
	 * @writer_data: the data associated to this instance.
	 * @object: the #NAIFactoryObject object being written.
	 * @def: the description of the data to be written.
	 * @value: the #NADataBoxed to be written down.
	 * @messages: a pointer to a #GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Write the data embedded in @value down to @instance.
	 *
	 * Returns: a NAIIOProvider operation return code.
	 *
	 * This method must be implemented in order any data be written.
	 */
	guint         ( *write_data ) ( const NAIFactoryProvider *writer, void *writer_data, const NAIFactoryObject *object, const NADataBoxed *boxed, GSList **messages );

	/**
	 * write_done:
	 * @writer: this #NAIFactoryProvider instance.
	 * @writer_data: the data associated to this instance.
	 * @object: the #NAIFactoryObject object which comes to be written.
	 * @messages: a pointer to a #GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Returns: a NAIIOProvider operation return code.
	 *
	 * API called by #NAIFactoryObject when all data have been written.
	 * Implementor may take advantage of this to do some cleanup.
	 */
	guint         ( *write_done ) ( const NAIFactoryProvider *writer, void *writer_data, const NAIFactoryObject *object, GSList **messages  );
}
	NAIFactoryProviderInterface;

GType na_ifactory_provider_get_type( void );

void  na_ifactory_provider_read_item ( const NAIFactoryProvider *reader, void *reader_data, NAIFactoryObject *object, GSList **messages );
guint na_ifactory_provider_write_item( const NAIFactoryProvider *writer, void *writer_data, NAIFactoryObject *object, GSList **messages );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IFACTORY_PROVIDER_H__ */
