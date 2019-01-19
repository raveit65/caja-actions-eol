/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
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

#ifndef __CAJA_ACTIONS_API_NA_IIO_PROVIDER_H__
#define __CAJA_ACTIONS_API_NA_IIO_PROVIDER_H__

/**
 * SECTION: iio-provider
 * @title: NAIIOProvider
 * @short_description: The I/O Provider Interface
 * @include: caja-actions/na-iio-provider.h
 *
 * The #NAIIOProvider interface is defined in order to let both &prodname;
 * internal and user-provided external plugins provide read and write accesses
 * to their own private storage subsystem.
 *
 * &prodname; core does not provide by itself input/output code. Instead,
 * we entirely relies on input/output facilities provided by implementations
 * of this interface.
 *
 * &prodname; is bundled with several I/O providers.
 * Since version 3, the <literal>na-desktop</literal> I/O provider, which
 * implements the
 * <ulink role="online-location" url="http://www.caja-actions.org/?q=node/377/">DES-EMA</ulink>
 * specification, is the preferred way of storing (and sharing) items.
 *
 * The #NAIIOProvider interface provides three types of services:
 * <itemizedlist>
 *  <listitem>
 *   <formalpara>
 *    <title>
 *     loading items
 *    </title>
 *    <para>
 *     Loading items is used both by the &caja; plugin, by the
 *     &cact; program, and by the command-line utilities.
 *    </para>
 *   </formalpara>
 *  </listitem>
 *  <listitem>
 *   <formalpara>
 *    <title>
 *     creating, updating or deleting items
 *    </title>
 *    <para>
 *     Updating items is a feature only used by the &cact; program and
 *     by some command-line utilities.
 *    </para>
 *   </formalpara>
 *  </listitem>
 *  <listitem>
 *   <formalpara>
 *    <title>
 *     informing &prodname; of extern modifications
 *    </title>
 *    <para>
 *     The I/O provider should inform &prodname; when an item happens to
 *     have been modified in the underlying storage subsystem.
 *    </para>
 *   </formalpara>
 *   <para>
 *    This feature is only used by the &caja; plugin and by the
 *    &cact; program.
 *   </para>
 *   <para>
 *    The #NAIIOProvider interface does not define specific monitoring
 *    methods (but you can also take a glance at #NATimeout object).
 *    Instead, it is waited that the I/O provider module takes care
 *    itself of managing its own monitoring services at
 *    load/unload time, calling the na_iio_provider_item_changed()
 *    function when appropriate.
 *   </para>
 *  </listitem>
 * </itemizedlist>
 *
 * These services may be fully implemented by the I/O provider itself.
 * Or, the I/O provider may also prefer to take advantage of the data
 * factory management (see #NAIFactoryObject and #NAIFactoryProvider
 * interfaces) services.
 *
 * <refsect2>
 *  <title>I/O provider identifier</title>
 *  <para>
 *   For its own internal needs, &prodname; requires that each I/O provider
 *   have its own identifier, as an ASCII string.
 *  </para>
 *  <para>
 *   In order to avoid any collision, this I/O provider identifier is
 *   allocated by the &prodname; maintainers team. If you wish develop
 *   yourself a new I/O provider, and so need a new provider identifier,
 *   please contact the maintainers (see caja-actions.doap at the
 *   root of the source tree).
 *  </para>
 *  <para>
 *   Below is a list of currently allocated I/O provider identifiers.
 *   This list has been last updated on 2010, Feb. 14th.
 *  </para>
 *  <table>
 *   <title>Currently allocated I/O provider identifiers</title>
 *    <tgroup rowsep="1" colsep="1" cols="4">
 *      <colspec colname="id" />
 *      <colspec colname="label" />
 *      <colspec colname="holder" />
 *      <colspec colname="allocated" align="center" />
 *      <thead>
 *        <row>
 *          <entry>Identifier</entry>
 *          <entry>Name</entry>
 *          <entry>Holder</entry>
 *          <entry>Allocated on</entry>
 *        </row>
 *      </thead>
 *      <tbody>
 *        <row>
 *          <entry><literal>all</literal></entry>
 *          <entry>Reserved for &prodname; internal needs</entry>
 *          <entry>&prodname;</entry>
 *          <entry>2010-01-28</entry>
 *        </row>
 *        <row>
 *          <entry><literal>na-desktop</literal></entry>
 *          <entry>NA Desktop I/O Provider</entry>
 *          <entry>&prodname;</entry>
 *          <entry>2009-12-16</entry>
 *        </row>
 *        <row>
 *          <entry><literal>na-mateconf</literal></entry>
 *          <entry>NA MateConf I/O Provider</entry>
 *          <entry>&prodname;</entry>
 *          <entry>2009-12-16</entry>
 *        </row>
 *        <row>
 *          <entry><literal>na-xml</literal></entry>
 *          <entry>NA XML module</entry>
 *          <entry>&prodname;</entry>
 *          <entry>2010-02-14</entry>
 *        </row>
 *      </tbody>
 *    </tgroup>
 *  </table>
 * </refsect2>
 *
 * <refsect2>
 *  <title>Versions historic</title>
 *  <table>
 *    <title>Historic of the versions of the #NAIIOProvider interface</title>
 *    <tgroup rowsep="1" colsep="1" align="center" cols="3">
 *      <colspec colname="na-version" />
 *      <colspec colname="api-version" />
 *      <colspec colname="current" />
 *      <thead>
 *        <row>
 *          <entry>&prodname; version</entry>
 *          <entry>#NAIIOProvider interface version</entry>
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

#include "na-object-item.h"

G_BEGIN_DECLS

#define NA_TYPE_IIO_PROVIDER                      ( na_iio_provider_get_type())
#define NA_IIO_PROVIDER( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_TYPE_IIO_PROVIDER, NAIIOProvider ))
#define NA_IS_IIO_PROVIDER( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_TYPE_IIO_PROVIDER ))
#define NA_IIO_PROVIDER_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_TYPE_IIO_PROVIDER, NAIIOProviderInterface ))

typedef struct _NAIIOProvider                     NAIIOProvider;
typedef struct _NAIIOProviderInterfacePrivate     NAIIOProviderInterfacePrivate;

/**
 * NAIIOProviderInterface:
 * @get_version:         [should] returns the version of this interface that the
 *                                plugin implements.
 * @get_id:              [must]   returns the internal id of the plugin.
 * @get_name:            [should] returns the public name of the plugin.
 * @read_items:          [should] reads items.
 * @is_willing_to_write: [should] asks the plugin whether it is willing to write.
 * @is_able_to_write:    [should] asks the plugin whether it is able to write.
 * @write_item:          [should] writes an item.
 * @delete_item:         [should] deletes an item.
 * @duplicate_data:      [may]    let the I/O provider duplicates its specific data.
 *
 * This defines the methods that a #NAIIOProvider may, should, or must
 * implement.
 */
typedef struct {
	/*< private >*/
	GTypeInterface                 parent;
	NAIIOProviderInterfacePrivate *private;

	/*< public >*/
	/**
	 * get_version:
	 * @instance: the NAIIOProvider provider.
	 *
	 * Caja-Actions calls this method each time it needs to know
	 * which version of this interface the plugin implements.
	 *
	 * If this method is not implemented by the plugin,
	 * Caja-Actions considers that the plugin only implements
	 * the version 1 of the NAIIOProvider interface.
	 *
	 * Return value: if implemented, this method must return the version
	 * number of this interface the I/O provider is supporting.
	 *
	 * Defaults to 1.
	 *
	 * Since: 2.30
	 */
	guint    ( *get_version )        ( const NAIIOProvider *instance );

	/**
	 * get_id:
	 * @instance: the NAIIOProvider provider.
	 *
	 * The I/O provider must implement this method.
	 *
	 * Return value: the implementation must return the internal identifier
	 * of the I/O provider, as a newly allocated string which will be g_free()
	 * by the caller.
	 *
	 * Since: 2.30
	 */
	gchar *  ( *get_id )             ( const NAIIOProvider *instance );

	/**
	 * get_name:
	 * @instance: the NAIIOProvider provider.
	 *
	 * Return value: if implemented, this method must return the display
	 * name of the I/O provider, as a newly allocated string which will be
	 * g_free() by the caller.
	 *
	 * This may be the name of the module itself, but this also may be a
	 * special name the modules gives to this interface.
	 *
	 * Defaults to an empty string.
	 *
	 * Since: 2.30
	 */
	gchar *  ( *get_name )           ( const NAIIOProvider *instance );

	/**
	 * read_items:
	 * @instance: the NAIIOProvider provider.
	 * @messages: a pointer to a GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Reads the whole items list from the specified I/O provider.
	 *
	 * The I/O provider should implement this method, but if it doesn't,
	 * then this greatly lowerize the interest of this I/O provider (!).
	 *
	 * Return value: if implemented, this method must return a unordered
	 * flat GList of NAObjectItem-derived objects (menus or actions);
	 * the actions embed their own profiles.
	 *
	 * Defaults to NULL list.
	 *
	 * Since: 2.30
	 */
	GList *  ( *read_items )         ( const NAIIOProvider *instance, GSList **messages );

	/**
	 * is_willing_to_write:
	 * @instance: the NAIIOProvider provider.
	 *
	 * The 'willing_to_write' property is intrinsic to the I/O provider.
	 * It is not supposed to make any assumption on the environment it is
	 * currently running on.
	 * This property just says that the developer/maintainer has released
	 * the needed code in order to update/create/delete NAObjectItem-derived
	 * objects.
	 *
	 * Note that even if this property is TRUE, there is yet many
	 * reasons for not being able to update/delete existing items or
	 * create new ones (see e.g. is_able_to_write() method below).
	 *
	 * Return value: if implemented, this method must return a boolean
	 * value, whose purpose is to let know to Caja-Actions whether
	 * this I/O provider is, or not, willing to write.
	 *
	 * Defaults to FALSE.
	 *
	 * Since: 2.30
	 */
	gboolean ( *is_willing_to_write )( const NAIIOProvider *instance );

	/**
	 * is_able_to_write:
	 * @instance: the NAIIOProvider provider.
	 *
	 * The 'able_to_write' property is a runtime one.
	 * When returning TRUE, the I/O provider insures that it has
	 * successfully checked that it was able to write some things
	 * down to its storage subsystem(s).
	 *
	 * The 'able_to_write' property is independent of the
	 * 'willing_to_write' above, though it is only checked if the
	 * I/O provider is actually willing to write.
	 *
	 * This condition is only relevant when trying to define new items,
	 * to see if a willing_to provider is actually able to do write
	 * operations. It it not relevant for updating/deleting already
	 * existing items as they have already checked their own runtime
	 * writability status when read from the storage subsystems.
	 *
	 * Note that even if this property is TRUE, there is yet many
	 * reasons for not being able to update/delete existing items or
	 * create new ones (see e.g. 'locked' user preference key).
	 *
	 * Return value: if implemented, this method must return a boolean
	 * value, whose purpose is to let know to Caja-Actions whether
	 * this I/O provider is eventually able to write.
	 *
	 * Defaults to FALSE.
	 *
	 * Since: 2.30
	 */
	gboolean ( *is_able_to_write )   ( const NAIIOProvider *instance );

	/**
	 * write_item:
	 * @instance: the NAIIOProvider provider.
	 * @item: a NAObjectItem-derived item, menu or action.
	 * @messages: a pointer to a GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Writes a new @item down to the privat underlying storage subsystem
	 * which happens to be managed by the I/O provider.
	 *
	 * There is no update_item function; it is the responsibility
	 * of the provider to delete the previous version of an item before
	 * actually writing the new one.
	 *
	 * The I/O provider should implement this method, or return
	 * FALSE in is_willing_to_write() method above.
	 *
	 * Return value: NA_IIO_PROVIDER_CODE_OK if the write operation
	 * was successful, or another code depending of the detected error.
	 *
	 * Since: 2.30
	 */
	guint    ( *write_item )         ( const NAIIOProvider *instance, const NAObjectItem *item, GSList **messages );

	/**
	 * delete_item:
	 * @instance: the NAIIOProvider provider.
	 * @item: a NAObjectItem-derived item, menu or action.
	 * @messages: a pointer to a GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Deletes an existing @item from the I/O subsystem.
	 *
	 * The I/O provider should implement this method, or return
	 * FALSE in is_willing_to_write() method above.
	 *
	 * Return value: NA_IIO_PROVIDER_CODE_OK if the delete operation was
	 * successful, or another code depending of the detected error.
	 *
	 * Since: 2.30
	 */
	guint    ( *delete_item )        ( const NAIIOProvider *instance, const NAObjectItem *item, GSList **messages );

	/**
	 * duplicate_data:
	 * @instance: the NAIIOProvider provider.
	 * @dest: a NAObjectItem-derived item, menu or action.
	 * @source: a NAObjectItem-derived item, menu or action.
	 * @messages: a pointer to a GSList list of strings; the provider
	 *  may append messages to this list, but shouldn't reinitialize it.
	 *
	 * Caja-Actions typically calls this method while duplicating
	 * a NAObjectItem-derived object, in order to let the I/O provider
	 * duplicates itself specific data (if any) it may have set on
	 * @source object.
	 *
	 * Note that this does not duplicate in any way any
	 * NAObjectItem-derived object. We are just dealing here with
	 * the provider-specific data which may have been attached to
	 * the NAObjectItem-derived object.
	 *
	 * Return value: NA_IIO_PROVIDER_CODE_OK if the duplicate operation
	 * was successful, or another code depending of the detected error.
	 *
	 * Since: 2.30
	 */
	guint    ( *duplicate_data )     ( const NAIIOProvider *instance, NAObjectItem *dest, const NAObjectItem *source, GSList **messages );
}
	NAIIOProviderInterface;

/* -- adding a new status here should imply also adding a new tooltip
 * -- in na_io_provider_get_readonly_tooltip().
 */
/**
 * NAIIOProviderWritabilityStatus:
 * @NA_IIO_PROVIDER_STATUS_WRITABLE:          item and i/o provider are writable.
 * @NA_IIO_PROVIDER_STATUS_UNAVAILABLE:       unavailable i/o provider.
 * @NA_IIO_PROVIDER_STATUS_INCOMPLETE_API:    i/o provider has an incomplete write api.
 * @NA_IIO_PROVIDER_STATUS_NOT_WILLING_TO:    i/o provider is not willing to write.
 * @NA_IIO_PROVIDER_STATUS_NOT_ABLE_TO:       i/o provider is not able to write.
 * @NA_IIO_PROVIDER_STATUS_LOCKED_BY_ADMIN:   i/o provider has been locked by the administrator.
 * @NA_IIO_PROVIDER_STATUS_LOCKED_BY_USER:    i/o provider has been locked by the user.
 * @NA_IIO_PROVIDER_STATUS_ITEM_READONLY:     item is read-only.
 * @NA_IIO_PROVIDER_STATUS_NO_PROVIDER_FOUND: no writable i/o provider found.
 * @NA_IIO_PROVIDER_STATUS_LEVEL_ZERO:        level zero is not writable.
 * @NA_IIO_PROVIDER_STATUS_UNDETERMINED:      unknown reason (and probably a bug).
 *
 * The reasons for which an item may not be writable.
 *
 * Not all reasons are to be managed at the I/O provider level.
 * Some are to be used only internally from &prodname; programs.
 */
typedef enum {
	NA_IIO_PROVIDER_STATUS_WRITABLE = 0,
	NA_IIO_PROVIDER_STATUS_UNAVAILABLE,
	NA_IIO_PROVIDER_STATUS_INCOMPLETE_API,
	NA_IIO_PROVIDER_STATUS_NOT_WILLING_TO,
	NA_IIO_PROVIDER_STATUS_NOT_ABLE_TO,
	NA_IIO_PROVIDER_STATUS_LOCKED_BY_ADMIN,
	NA_IIO_PROVIDER_STATUS_LOCKED_BY_USER,
	NA_IIO_PROVIDER_STATUS_ITEM_READONLY,
	NA_IIO_PROVIDER_STATUS_NO_PROVIDER_FOUND,
	NA_IIO_PROVIDER_STATUS_LEVEL_ZERO,
	NA_IIO_PROVIDER_STATUS_UNDETERMINED,
	/*< private >*/
	NA_IIO_PROVIDER_STATUS_LAST,
}
	NAIIOProviderWritabilityStatus;

/* -- adding a new code here should imply also adding a new label
 * -- in #na_io_provider_get_return_code_label().
 */
/**
 * NAIIOProviderOperationStatus:
 * @NA_IIO_PROVIDER_CODE_OK:            the requested operation has been successful.
 * @NA_IIO_PROVIDER_CODE_PROGRAM_ERROR: a program error has been detected;
 *                                      you should open a bug in
 *                                      <ulink url="https://bugzilla.gnome.org/enter_bug.cgi?product=caja-actions">Bugzilla</ulink>.
 * @NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN:   the provider is not willing
 *                                             to do the requested action.
 * @NA_IIO_PROVIDER_CODE_WRITE_ERROR:          a write error has been detected.
 * @NA_IIO_PROVIDER_CODE_DELETE_SCHEMAS_ERROR: the schemas could not be deleted.
 * @NA_IIO_PROVIDER_CODE_DELETE_CONFIG_ERROR:  the configuration could not be deleted.
 *
 * The return code of operations.
 */
typedef enum {
	NA_IIO_PROVIDER_CODE_OK = 0,
	NA_IIO_PROVIDER_CODE_PROGRAM_ERROR = 1 + NA_IIO_PROVIDER_STATUS_LAST,
	NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN,
	NA_IIO_PROVIDER_CODE_WRITE_ERROR,
	NA_IIO_PROVIDER_CODE_DELETE_SCHEMAS_ERROR,
	NA_IIO_PROVIDER_CODE_DELETE_CONFIG_ERROR,
}
	NAIIOProviderOperationStatus;

GType na_iio_provider_get_type    ( void );

/* -- to be called by the I/O provider when an item has changed
 */
void  na_iio_provider_item_changed( const NAIIOProvider *instance );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IIO_PROVIDER_H__ */
