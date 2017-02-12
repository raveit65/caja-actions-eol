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

#ifndef __CORE_NA_IOPTION_H__
#define __CORE_NA_IOPTION_H__

/*
 * SECTION: ioptions
 * @title: NAIOption
 * @short_description: The Option Interface v 1
 * @include: core/na-ioption.h
 *
 * The #NAIOption interface is to be implemented by #GObject -derived object which
 * are part of a #NAIOptionsList interface.
 *
 * <refsect2>
 *  <title>Versions historic</title>
 *  <table>
 *    <title>Historic of the versions of the #NAIOption interface</title>
 *    <tgroup rowsep="1" colsep="1" align="center" cols="3">
 *      <colspec colname="na-version" />
 *      <colspec colname="api-version" />
 *      <colspec colname="current" />
 *      <thead>
 *        <row>
 *          <entry>&prodname; version</entry>
 *          <entry>#NAIOption interface version</entry>
 *          <entry></entry>
 *        </row>
 *      </thead>
 *      <tbody>
 *        <row>
 *          <entry>since 3.2</entry>
 *          <entry>1</entry>
 *          <entry>current version</entry>
 *        </row>
 *      </tbody>
 *    </tgroup>
 *  </table>
 * </refsect2>
 */

#include "gdk-pixbuf/gdk-pixbuf.h"

G_BEGIN_DECLS

#define NA_TYPE_IOPTION                      ( na_ioption_get_type())
#define NA_IOPTION( instance )               ( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_TYPE_IOPTION, NAIOption ))
#define NA_IS_IOPTION( instance )            ( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_TYPE_IOPTION ))
#define NA_IOPTION_GET_INTERFACE( instance ) ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_TYPE_IOPTION, NAIOptionInterface ))

typedef struct _NAIOption                      NAIOption;
typedef struct _NAIOptionInterfacePrivate      NAIOptionInterfacePrivate;
typedef struct _NAIOptionImportFromUriParms    NAIOptionImportFromUriParms;
typedef struct _NAIOptionManageImportModeParms NAIOptionManageImportModeParms;

/*
 * NAIOptionInterface:
 * @get_version:     returns the version of this interface that the
 *                   instance implements.
 * @get_id:          returns the string identifier of the option.
 * @get_label:       returns the label of the option.
 * @get_description: returns the description of the option.
 * @get_pixbuf:      returns the image associated to the option.
 *
 * This defines the interface that a #NAIOption implementation should provide.
 */
typedef struct {
	/*< private >*/
	GTypeInterface             parent;
	NAIOptionInterfacePrivate *private;

	/*< public >*/
	/*
	 * get_version:
	 * @instance: the #NAIOption instance of the implementation.
	 *
	 * This method is supposed to let know to any caller which version of this
	 * interface the implementation provides. This may be useful when this
	 * interface will itself be upgraded.
	 *
	 * If this method is not provided by the implementation, one should suppose
	 * that the implemented version is at last the version 1.
	 *
	 * Returns: the version of this interface provided by the implementation.
	 *
	 * Since: 3.2
	 */
	guint       ( *get_version )    ( const NAIOption *instance );

	/*
	 * get_id:
	 * @instance: the #NAIOption instance of the implementation.
	 *
	 * Returns: the string identifier of the option, as a newly allocated string
	 * which should be g_free() by the caller.
	 *
	 * Since: 3.2
	 */
	gchar *     ( *get_id )         ( const NAIOption *instance );

	/*
	 * get_label:
	 * @instance: the #NAIOption instance of the implementation.
	 *
	 * Returns: the label of the option, as a newly allocated string
	 * which should be g_free() by the caller.
	 *
	 * Since: 3.2
	 */
	gchar *     ( *get_label )      ( const NAIOption *instance );

	/*
	 * get_description:
	 * @instance: the #NAIOption instance of the implementation.
	 *
	 * Returns: the description of the option, as a newly allocated string
	 * which should be g_free() by the caller.
	 *
	 * Since: 3.2
	 */
	gchar *     ( *get_description )( const NAIOption *instance );

	/*
	 * get_pixbuf:
	 * @instance: the #NAIOption instance of the implementation.
	 *
	 * Returns: the image assocated to the option, as a newly allocated string
	 * which should be g_object_unref() by the caller.
	 *
	 * Since: 3.2
	 */
	GdkPixbuf * ( *get_pixbuf )     ( const NAIOption *instance );
}
	NAIOptionInterface;

GType      na_ioption_get_type       ( void );

gchar     *na_ioption_get_id         ( const NAIOption *option );
gchar     *na_ioption_get_label      ( const NAIOption *option );
gchar     *na_ioption_get_description( const NAIOption *option );
GdkPixbuf *na_ioption_get_pixbuf     ( const NAIOption *option );

G_END_DECLS

#endif /* __CORE_NA_IOPTION_H__ */
