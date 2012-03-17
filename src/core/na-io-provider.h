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

#ifndef __CORE_NA_IO_PROVIDER_H__
#define __CORE_NA_IO_PROVIDER_H__

/**
 * SECTION: na_io_provider
 * @short_description: #NAIOProvider class definition.
 * @include: core/na-io-provider.h
 *
 * NAIOProvider is the Caja-Actions class which is used to manage
 * external I/O Providers which implement NAIIOProvider interface.
 */

#include "na-iprefs.h"
#include "na-pivot.h"

G_BEGIN_DECLS

#define NA_IO_PROVIDER_TYPE					( na_io_provider_get_type())
#define NA_IO_PROVIDER( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, NA_IO_PROVIDER_TYPE, NAIOProvider ))
#define NA_IO_PROVIDER_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NA_IO_PROVIDER_TYPE, NAIOProviderClass ))
#define NA_IS_IO_PROVIDER( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_IO_PROVIDER_TYPE ))
#define NA_IS_IO_PROVIDER_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_IO_PROVIDER_TYPE ))
#define NA_IO_PROVIDER_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_IO_PROVIDER_TYPE, NAIOProviderClass ))

typedef struct NAIOProviderPrivate      NAIOProviderPrivate;

typedef struct {
	GObject              parent;
	NAIOProviderPrivate *private;
}
	NAIOProvider;

typedef struct NAIOProviderClassPrivate NAIOProviderClassPrivate;

typedef struct {
	GObjectClass              parent;
	NAIOProviderClassPrivate *private;
}
	NAIOProviderClass;

/* MateConf preferences key
 */
#define IO_PROVIDER_KEY_ROOT			"io-providers"
#define IO_PROVIDER_KEY_READABLE		"read-at-startup"
#define IO_PROVIDER_KEY_WRITABLE		"writable"
#define IO_PROVIDER_KEY_ORDER			"io-providers-order"

GType          na_io_provider_get_type ( void );

void           na_io_provider_terminate( void );

GList         *na_io_provider_get_providers_list( const NAPivot *pivot );
void           na_io_provider_reorder_providers_list( const NAPivot *pivot );
void           na_io_provider_dump_providers_list( GList *providers );

NAIOProvider  *na_io_provider_find_provider_by_id( GList *providers, const gchar *id );
NAIOProvider  *na_io_provider_get_writable_provider( const NAPivot *pivot );

GList         *na_io_provider_read_items( const NAPivot *pivot, GSList **messages );

gchar         *na_io_provider_get_id                     ( const NAIOProvider *provider );
gchar         *na_io_provider_get_name                   ( const NAIOProvider *provider );
gboolean       na_io_provider_is_user_readable_at_startup( const NAIOProvider *provider, const NAIPrefs *iprefs );
gboolean       na_io_provider_is_user_writable           ( const NAIOProvider *provider, const NAIPrefs *iprefs );
gboolean       na_io_provider_is_locked_by_admin         ( const NAIOProvider *provider, const NAIPrefs *iprefs );
NAIIOProvider *na_io_provider_get_provider               ( const NAIOProvider *provider );
gboolean       na_io_provider_is_willing_to_write        ( const NAIOProvider *provider );
gboolean       na_io_provider_is_able_to_write           ( const NAIOProvider *provider );
gboolean       na_io_provider_has_write_api              ( const NAIOProvider *provider );

guint          na_io_provider_write_item ( const NAIOProvider *provider, const NAObjectItem *item, GSList **messages );
guint          na_io_provider_delete_item( const NAIOProvider *provider, const NAObjectItem *item, GSList **messages );

gchar         *na_io_provider_get_readonly_tooltip( guint reason );
gchar         *na_io_provider_get_return_code_label( guint code );

G_END_DECLS

#endif /* __CORE_NA_IO_PROVIDER_H__ */
