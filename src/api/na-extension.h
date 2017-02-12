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

#ifndef __CAJA_ACTIONS_API_NA_EXTENSION_H__
#define __CAJA_ACTIONS_API_NA_EXTENSION_H__

/**
 * SECTION: extension
 * @title: Plugins
 * @short_description: The Caja-Actions Extension Interface Definition v 1
 * @include: caja-actions/na-extension.h
 *
 * &prodname; accepts extensions as dynamically loadable libraries
 * (aka plugins).
 *
 * As of today, &prodname; may be extended in the following areas:
 *  <itemizedlist>
 *    <listitem>
 *      <formalpara>
 *        <title>
 *          Storing menus and actions in a specific storage subsystem
 *        </title>
 *        <para>
 *          This extension is provided via the public
 *          <link linkend="NAIIOProvider">NAIIOProvider</link>
 *          interface; it takes care of reading and writing menus
 *          and actions to a specific storage subsystem.
 *        </para>
 *      </formalpara>
 *    </listitem>
 *    <listitem>
 *      <formalpara>
 *        <title>
 *          Exporting menus and actions
 *        </title>
 *        <para>
 *          This extension is provided via the public
 *          <link linkend="NAIExporter">NAIExporter</link>
 *          interface; it takes care of exporting menus and actions
 *          to the filesystem from the &prodname; Configuration Tool
 *          user interface.
 *        </para>
 *      </formalpara>
 *    </listitem>
 *    <listitem>
 *      <formalpara>
 *        <title>
 *          Importing menus and actions
 *        </title>
 *        <para>
 *          This extension is provided via the public
 *          <link linkend="NAIImporter">NAIImporter</link>
 *          interface; it takes care of importing menus and actions
 *          from the filesystem into the &prodname; Configuration Tool
 *          user interface.
 *        </para>
 *      </formalpara>
 *    </listitem>
 *  </itemizedlist>
 *
 * In order to be recognized as a valid &prodname; plugin, the library
 * must at least export the functions described in this extension API.
 *
 * <refsect2>
 *   <title>Developing a &prodname; plugin</title>
 *   <refsect3>
 *     <title>Building the dynamically loadable library</title>
 *       <para>
 * The suggested way of producing a dynamically loadable library is to
 * use
 * <application><ulink url="http://www.gnu.org/software/autoconf/">autoconf</ulink></application>,
 * <application><ulink url="http://www.gnu.org/software/automake/">automake</ulink></application>
 * and
 * <application><ulink url="http://www.gnu.org/software/libtool/">libtool</ulink></application>
 * GNU applications.
 *       </para>
 *       <para>
 * In this case, it should be enough to use the <option>-module</option>
 * option in your <filename>Makefile.am</filename>, as in:
 * <programlisting>
 *   libna_io_desktop_la_LDFLAGS = -module -no-undefined -avoid-version
 * </programlisting>
 *       </para>
 *    </refsect3>
 *    <refsect3>
 *      <title>Installing the library</title>
 *       <para>
 * At startup time, &prodname; searches for its candidate libraries in
 * <filename>PKGLIBDIR</filename> directory, which most often happens to
 * be <filename>/usr/lib/caja-actions/</filename> or
 * <filename>/usr/lib64/caja-actions/</filename>,
 * depending of your system.
 *       </para>
 *   </refsect3>
 * </refsect2>
 *
 * <refsect2>
 *  <title>Versions historic</title>
 *  <table>
 *    <title>Historic of the versions of this extension API</title>
 *    <tgroup rowsep="1" colsep="1" align="center" cols="3">
 *      <colspec colname="na-version" />
 *      <colspec colname="api-version" />
 *      <colspec colname="current" />
 *      <thead>
 *        <row>
 *          <entry>&prodname; version</entry>
 *          <entry>extension API version</entry>
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

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * na_extension_startup:
 * @module: the #GTypeModule of the plugin library being loaded.
 *
 * This function is called by the Caja-Actions plugin manager when
 * the plugin library is first loaded in memory. The library may so take
 * advantage of this call by initializing itself, registering its
 * internal #GType types, etc.
 *
 * A Caja-Actions extension must implement this function in order
 * to be considered as a valid candidate to dynamic load.
 *
 * <example>
 *   <programlisting>
 *     static GType st_module_type = 0;
 *
 *     gboolean
 *     na_extension_startup( GTypeModule *plugin )
 *     {
 *         static GTypeInfo info = {
 *             sizeof( CappDesktopProviderClass ),
 *             NULL,
 *             NULL,
 *             ( GClassInitFunc ) class_init,
 *             NULL,
 *             NULL,
 *             sizeof( CappDesktopProvider ),
 *             0,
 *             ( GInstanceInitFunc ) instance_init
 *         };
 *
 *         static const GInterfaceInfo iio_provider_iface_info = {
 *             ( GInterfaceInitFunc ) iio_provider_iface_init,
 *             NULL,
 *             NULL
 *         };
 *
 *         st_module_type = g_type_module_register_type( plugin, G_TYPE_OBJECT, "CappDesktopProvider", &amp;info, 0 );
 *
 *         g_type_module_add_interface( plugin, st_module_type, NA_TYPE_IIO_PROVIDER, &amp;iio_provider_iface_info );
 *
 *         return( TRUE );
 *     }
 *   </programlisting>
 * </example>
 *
 * Returns: %TRUE if the initialization is successful, %FALSE else.
 * In this later case, the library is unloaded and no more considered.
 *
 * Since: 2.30
 */
gboolean na_extension_startup    ( GTypeModule *module );

/**
 * na_extension_get_version:
 *
 * This function is called by the &prodname; program each time
 * it needs to know which version of this API the plugin
 * implements.
 *
 * If this function is not exported by the library,
 * the plugin manager considers that the library only implements the
 * version 1 of this extension API.
 *
 * Returns: the version of this API supported by the module.
 *
 * Since: 2.30
 */
guint    na_extension_get_version( void );

/**
 * na_extension_list_types:
 * @types: the address where to store the zero-terminated array of
 *  instantiable #GType types this library implements.
 *
 * Returned #GType types must already have been registered in the
 * #GType system (e.g. at #na_extension_startup() time), and the objects
 * they describe may implement one or more of the interfaces defined in
 * this Caja-Actions public API.
 *
 * The Caja-Actions plugin manager will instantiate one #GTypeInstance-
 * derived object for each returned #GType type, and associate these objects
 * to this library.
 *
 * A Caja-Actions extension must implement this function in order
 * to be considered as a valid candidate to dynamic load.
 *
 * <example>
 *   <programlisting>
 *     &lcomment; the count of GType types provided by this extension
 *      * each new GType type must
 *      * - be registered in na_extension_startup()
 *      * - be addressed in na_extension_list_types().
 *      &rcomment;
 *     #define CADP_TYPES_COUNT    1
 *
 *     guint
 *     na_extension_list_types( const GType **types )
 *     {
 *          static GType types_list [1+CADP_TYPES_COUNT];
 *
 *          &lcomment; CADP_TYPE_DESKTOP_PROVIDER has been previously
 *           * registered in na_extension_startup function
 *           &rcomment;
 *          types_list[0] = CADP_TYPE_DESKTOP_PROVIDER;
 *
 *          types_list[CADP_TYPES_COUNT] = 0;
 *          *types = types_list;
 *
 *          return( CADP_TYPES_COUNT );
 *     }
 *   </programlisting>
 * </example>
 *
 * Returns: the number of #GType types returned in the @types array, not
 * counting the terminating zero item.
 *
 * Since: 2.30
 */
guint    na_extension_list_types ( const GType **types );

/**
 * na_extension_shutdown:
 *
 * This function is called by Caja-Actions when it is about to
 * shutdown itself.
 *
 * The dynamically loaded library may take advantage of this call to
 * release any resource, handle, and so on, it may have previously
 * allocated.
 *
 * A Caja-Actions extension must implement this function in order
 * to be considered as a valid candidate to dynamic load.
 *
 * Since: 2.30
 */
void     na_extension_shutdown   ( void );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_EXTENSION_H__ */
