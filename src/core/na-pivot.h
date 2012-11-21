/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu pivots.
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

#ifndef __CORE_NA_PIVOT_H__
#define __CORE_NA_PIVOT_H__

/* @title: NAPivot
 * @short_description: The #NAPivot Class Definition
 * @include: core/na-pivot.h
 *
 * A consuming program should allocate one new NAPivot object in its
 * startup phase. The class takes care of declaring the I/O interfaces,
 * while registering the known providers.
 * 		NAPivot *pivot = na_pivot_new();
 *
 * With this newly allocated #NAPivot object, the consuming program
 * is then able to ask for loading the items.
 * 		na_pivot_set_loadable( pivot, PIVOT_LOADABLE_SET );
 * 		na_pivot_load_items( pivot );
 *
 * Notification system.
 *
 * The NAPivot object acts as a sort of "summarizing relay" for notification
 * messages sent by I/O storage providers:
 *
 * - When an I/O storage subsystem detects a change on an item it manages,
 *   action or menu, it is first supposed to do its best effort in order
 *   to summarize its notifications messages;
 *
 * - At the end of this first stage of summarization, the I/O provider
 *   should call the na_iio_provider_item_changed() function, which
 *   itself will emit the "io-provider-item-changed" signal.
 *   This is done so that an external I/O provider does not have to know
 *   anything with the signal name, but has only to take care of calling
 *   a function of the NAIIOProvider API.
 *
 * - The emitted signal is catched by na_pivot_on_item_changed_handler(),
 *   which was connected when the I/O provider plugin was associated with
 *   the NAIOProvider object.
 *
 * - The NAPivot object receives these notifications originating from all
 *   loaded I/O providers, itself summarizes them, and only then notify its
 *   consumers with only one message for a whole set of modifications.
 *
 * It is eventually up to the consumer to connect to this signal, and
 * choose itself whether to reload items or not.
 */

#include <api/na-iio-provider.h>
#include <api/na-object-api.h>

#include "na-settings.h"

G_BEGIN_DECLS

#define NA_TYPE_PIVOT                ( na_pivot_get_type())
#define NA_PIVOT( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_PIVOT, NAPivot ))
#define NA_PIVOT_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_PIVOT, NAPivotClass ))
#define NA_IS_PIVOT( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_PIVOT ))
#define NA_IS_PIVOT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_PIVOT ))
#define NA_PIVOT_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_PIVOT, NAPivotClass ))

typedef struct _NAPivotPrivate       NAPivotPrivate;

typedef struct {
	/*< private >*/
	GObject         parent;
	NAPivotPrivate *private;
}
	NAPivot;

typedef struct _NAPivotClassPrivate  NAPivotClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass         parent;
	NAPivotClassPrivate *private;
}
	NAPivotClass;

GType    na_pivot_get_type( void );

/* properties
 */
#define PIVOT_PROP_LOADABLE						"pivot-prop-loadable"
#define PIVOT_PROP_TREE							"pivot-prop-tree"

/* signals
 *
 * NAPivot acts as a 'summarizing' proxy for signals emitted by the
 * NAIIOProvider providers when they detect a modification in their
 * underlying items storage subsystems.
 *
 * As several to many signals may be emitted when such a modification occurs,
 * NAPivot summarizes all these signals in an only one 'items-changed' event.
 */
#define PIVOT_SIGNAL_ITEMS_CHANGED				"pivot-items-changed"

/* Loadable population
 * CACT management user interface defaults to PIVOT_LOAD_ALL
 * N-A plugin set the loadable population to !PIVOT_LOAD_DISABLED & !PIVOT_LOAD_INVALID
 */
typedef enum {
	PIVOT_LOAD_NONE     = 0,
	PIVOT_LOAD_DISABLED = 1 << 0,
	PIVOT_LOAD_INVALID  = 1 << 1,
	PIVOT_LOAD_ALL      = 0xff
}
	NAPivotLoadableSet;

NAPivot      *na_pivot_new ( void );
void          na_pivot_dump( const NAPivot *pivot );

/* Management of the plugins which claim to implement a Caja-Actions interface.
 * As of 2.30, these may be NAIIOProvider, NAIImporter or NAIExporter
 */
GList        *na_pivot_get_providers ( const NAPivot *pivot, GType type );
void          na_pivot_free_providers( GList *providers );

/* Items, menus and actions, management
 */
NAObjectItem *na_pivot_get_item     ( const NAPivot *pivot, const gchar *id );
GList        *na_pivot_get_items    ( const NAPivot *pivot );
void          na_pivot_load_items   ( NAPivot *pivot );
void          na_pivot_set_new_items( NAPivot *pivot, GList *tree );

void          na_pivot_on_item_changed_handler( NAIIOProvider *provider, NAPivot *pivot  );

/* NAPivot properties and configuration
 */
void          na_pivot_set_loadable     ( NAPivot *pivot, guint loadable );

G_END_DECLS

#endif /* __CORE_NA_PIVOT_H__ */
