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

#ifndef __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_H__
#define __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_H__

/**
 * SECTION: ifactory-object
 * @title: NAIFactoryObject
 * @short_description: The #NAObjectItem Interface v 1
 * @include: caja-actions/na-ifactory_object.h
 *
 * This interface is implemented by #NAObjectItem derived objects so that they
 * can take advantage of our data factory management system.
 *
 * A #NAObjectItem derived object which would implement this #NAIFactoryObject
 * interface must meet following conditions:
 * <itemizedlist>
 *   <listitem>
 *     <para>
 *       accept an empty constructor
 *     </para>
 *   </listitem>
 * </itemizedlist>
 *
 * <refsect2>
 *  <title>Versions historic</title>
 *  <table>
 *    <title>Historic of the versions of the #NAIFactoryObject interface</title>
 *    <tgroup rowsep="1" colsep="1" align="center" cols="3">
 *      <colspec colname="na-version" />
 *      <colspec colname="api-version" />
 *      <colspec colname="current" />
 *      <thead>
 *        <row>
 *          <entry>&prodname; version</entry>
 *          <entry>#NAIFactoryObject interface version</entry>
 *          <entry></entry>
 *        </row>
 *      </thead>
 *      <tbody>
 *        <row>
 *          <entry>since 2.30</entry>
 *          <entry>1</entry>
 *          <entry>current version</entry>
 *        </row>
 *      </tbody>
 *    </tgroup>
 *  </table>
 * </refsect2>
 */

#include "na-data-def.h"
#include "na-data-boxed.h"
#include "na-ifactory-provider-provider.h"

G_BEGIN_DECLS

#define NA_TYPE_IFACTORY_OBJECT                      ( na_ifactory_object_get_type())
#define NA_IFACTORY_OBJECT( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_TYPE_IFACTORY_OBJECT, NAIFactoryObject ))
#define NA_IS_IFACTORY_OBJECT( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_TYPE_IFACTORY_OBJECT ))
#define NA_IFACTORY_OBJECT_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_TYPE_IFACTORY_OBJECT, NAIFactoryObjectInterface ))

typedef struct _NAIFactoryObject                     NAIFactoryObject;
typedef struct _NAIFactoryObjectInterfacePrivate     NAIFactoryObjectInterfacePrivate;

/**
 * NAIFactoryObjectInterface:
 * @get_version: returns the version of this interface the NAObjectItem implements.
 * @get_groups:  returns a pointer to the NADataGroup which defines this object.
 * @copy:        post copy callback.
 * @are_equal:   tests if two NAObjectItem are equals.
 * @is_valid:    tests if one NAObjectItem is valid.
 * @read_start:  triggered before serializing a NAObjectItem.
 * @read_done:   triggered after a NAObjectItem has been serialized.
 * @write_start: triggered before unserializing a NAObjectItem.
 * @write_done:  triggered after a NAObjectItem has been unserialized.
 *
 * In order to take full advantage of our data managament system,
 * NAObjectItem-derived objects all implement this #NAIFactoryObject
 * interface.
 */
typedef struct {
	/*< private >*/
	GTypeInterface                    parent;
	NAIFactoryObjectInterfacePrivate *private;

	/*< public >*/
	/**
	 * get_version:
	 * @instance: this #NAIFactoryObject instance.
	 *
	 * Defaults to 1.
	 *
	 * Returns: the version of this interface supported by @instance implementation.
	 *
	 * Since: 2.30
	 */
	guint         ( *get_version )( const NAIFactoryObject *instance );

	/**
	 * get_groups:
	 * @instance: this #NAIFactoryObject instance.
	 *
	 * Returns: a pointer to the NADataGroup which defines this object.
	 *
	 * Since: 2.30
	 */
	NADataGroup * ( *get_groups ) ( const NAIFactoryObject *instance );

	/**
	 * copy:
	 * @instance: the target #NAIFactoryObject instance.
	 * @source: the source #NAIFactoryObject instance.
	 *
	 * This function is triggered after having copied @source to
	 * @instance target. This later may take advantage of this call
	 * to do some particular copy tasks.
	 *
	 * Since: 2.30
	 */
	void          ( *copy )       ( NAIFactoryObject *instance, const NAIFactoryObject *source );

	/**
	 * are_equal:
	 * @a: the first #NAIFactoryObject instance.
	 * @b: the second #NAIFactoryObject instance.
	 *
	 * This function is triggered after all elementary data comparisons
	 * have been sucessfully made.
	 *
	 * Returns: %TRUE if @a is equal to @b.
	 *
	 * Since: 2.30
	 */
	gboolean      ( *are_equal )  ( const NAIFactoryObject *a, const NAIFactoryObject *b );

	/**
	 * is_valid:
	 * @object: the #NAIFactoryObject instance whose validity is to be checked.
	 *
	 * This function is triggered after all elementary data comparisons
	 * have been sucessfully made.
	 *
	 * Returns: %TRUE if @object is valid.
	 *
	 * Since: 2.30
	 */
	gboolean      ( *is_valid )   ( const NAIFactoryObject *object );

	/**
	 * read_start:
	 * @instance: this #NAIFactoryObject instance.
	 * @reader: the instance which has provided read services.
	 * @reader_data: the data associated to @reader.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called just before the object is unserialized.
	 *
	 * Since: 2.30
	 */
	void          ( *read_start ) ( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );

	/**
	 * read_done:
	 * @instance: this #NAIFactoryObject instance.
	 * @reader: the instance which has provided read services.
	 * @reader_data: the data associated to @reader.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called when the object has been unserialized.
	 *
	 * Since: 2.30
	 */
	void          ( *read_done )  ( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );

	/**
	 * write_start:
	 * @instance: this #NAIFactoryObject instance.
	 * @writer: the instance which has provided writing services.
	 * @writer_data: the data associated to @writer.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called just before the object is serialized.
	 *
	 * Returns: a NAIIOProvider operation return code.
	 *
	 * Since: 2.30
	 */
	guint         ( *write_start )( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );

	/**
	 * write_done:
	 * @instance: this #NAIFactoryObject instance.
	 * @writer: the instance which has provided writing services.
	 * @writer_data: the data associated to @writer.
	 * @messages: a pointer to a #GSList list of strings; the instance
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Called when the object has been serialized.
	 *
	 * Returns: a NAIIOProvider operation return code.
	 *
	 * Since: 2.30
	 */
	guint         ( *write_done ) ( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );
}
	NAIFactoryObjectInterface;

GType        na_ifactory_object_get_type( void );

NADataBoxed *na_ifactory_object_get_data_boxed ( const NAIFactoryObject *object, const gchar *name );
NADataGroup *na_ifactory_object_get_data_groups( const NAIFactoryObject *object );
void        *na_ifactory_object_get_as_void    ( const NAIFactoryObject *object, const gchar *name );
void         na_ifactory_object_set_from_void  ( NAIFactoryObject *object, const gchar *name, const void *data );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IFACTORY_OBJECT_H__ */
