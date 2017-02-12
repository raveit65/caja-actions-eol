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

#ifndef __CORE_NA_IO_PROVIDER_H__
#define __CORE_NA_IO_PROVIDER_H__

/* @title: NAIOProvider
 * @short_description: The NAIOProvider Class Definition
 * @include: core/na-io-provider.h
 *
 * #NAIOProvider is the Caja-Actions class which is used to manage
 * external I/O Providers which implement #NAIIOProvider interface. Each
 * #NAIOProvider objects may (or not) encapsulates one #NAIIOProvider
 * provider.
 *
 * Internal Caja-Actions code should never directly call a
 * #NAIIOProvider interface method, but rather should call the
 * corresponding NAIOProvider class method.
 *
 * Two preferences are used for each i/o provider:
 * 'readable': means that the i/o provider should be read when building
 *  the items hierarchy
 * 'writable': means that the i/o provider is candidate when writing a
 *  new item; this also means that existing items are deletable.
 *
 * To be actually writable, a i/o provider must:
 * - be set as 'writable' from a configuration point of view
 *   this may or not be edited depending of this is a mandatory or user
 *   preference
 * - be willing to write: this is an intrisinc i/o provider attribute
 * - be able to write: this is a runtime i/o provider property
 *
 * and the whole configuration must not have been locked by an admin.
 */

#include "na-pivot.h"

G_BEGIN_DECLS

#define NA_IO_PROVIDER_TYPE                ( na_io_provider_get_type())
#define NA_IO_PROVIDER( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_IO_PROVIDER_TYPE, NAIOProvider ))
#define NA_IO_PROVIDER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_IO_PROVIDER_TYPE, NAIOProviderClass ))
#define NA_IS_IO_PROVIDER( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_IO_PROVIDER_TYPE ))
#define NA_IS_IO_PROVIDER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_IO_PROVIDER_TYPE ))
#define NA_IO_PROVIDER_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_IO_PROVIDER_TYPE, NAIOProviderClass ))

typedef struct _NAIOProviderPrivate        NAIOProviderPrivate;

typedef struct {
	/*< private >*/
	GObject              parent;
	NAIOProviderPrivate *private;
}
	NAIOProvider;

typedef struct _NAIOProviderClassPrivate   NAIOProviderClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass              parent;
	NAIOProviderClassPrivate *private;
}
	NAIOProviderClass;

/* signal sent from a NAIIOProvider
 * via the na_iio_provider_item_changed() function
 */
#define IO_PROVIDER_SIGNAL_ITEM_CHANGED		"io-provider-item-changed"

GType         na_io_provider_get_type ( void );

NAIOProvider *na_io_provider_find_writable_io_provider( const NAPivot *pivot );
NAIOProvider *na_io_provider_find_io_provider_by_id   ( const NAPivot *pivot, const gchar *id );
const GList  *na_io_provider_get_io_providers_list    ( const NAPivot *pivot );
void          na_io_provider_unref_io_providers_list  ( void );

gchar        *na_io_provider_get_id             ( const NAIOProvider *provider );
gchar        *na_io_provider_get_name           ( const NAIOProvider *provider );
gboolean      na_io_provider_is_available       ( const NAIOProvider *provider );
gboolean      na_io_provider_is_conf_readable   ( const NAIOProvider *provider, const NAPivot *pivot, gboolean *mandatory );
gboolean      na_io_provider_is_conf_writable   ( const NAIOProvider *provider, const NAPivot *pivot, gboolean *mandatory );
gboolean      na_io_provider_is_finally_writable( const NAIOProvider *provider, guint *reason );

GList        *na_io_provider_load_items( const NAPivot *pivot, guint loadable_set, GSList **messages );

guint         na_io_provider_write_item    ( const NAIOProvider *provider, const NAObjectItem *item, GSList **messages );
guint         na_io_provider_delete_item   ( const NAIOProvider *provider, const NAObjectItem *item, GSList **messages );
guint         na_io_provider_duplicate_data( const NAIOProvider *provider, NAObjectItem *dest, const NAObjectItem *source, GSList **messages );

gchar        *na_io_provider_get_readonly_tooltip ( guint reason );
gchar        *na_io_provider_get_return_code_label( guint code );

G_END_DECLS

#endif /* __CORE_NA_IO_PROVIDER_H__ */
