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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <api/na-mateconf-utils.h>
#include <api/na-object-api.h>

#include "na-io-provider.h"
#include "na-updater.h"

/* private class data
 */
struct NAUpdaterClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAUpdaterPrivate {
	gboolean           dispose_has_run;
};

static NAPivotClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( NAUpdaterClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *object );
static void     instance_finalize( GObject *object );

GType
na_updater_get_type( void )
{
	static GType object_type = 0;

	if( !object_type ){
		object_type = register_type();
	}

	return( object_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_updater_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAUpdaterClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAUpdater ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( NA_PIVOT_TYPE, "NAUpdater", &info, 0 );

	return( type );
}

static void
class_init( NAUpdaterClass *klass )
{
	static const gchar *thisfn = "na_updater_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAUpdaterClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_updater_instance_init";
	NAUpdater *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( NA_IS_UPDATER( instance ));
	self = NA_UPDATER( instance );

	self->private = g_new0( NAUpdaterPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_updater_instance_dispose";
	NAUpdater *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NA_IS_UPDATER( object ));
	self = NA_UPDATER( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_updater_instance_finalize";
	NAUpdater *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_UPDATER( object ));
	self = NA_UPDATER( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * na_updater_new:
 *
 * Returns: a newly allocated #NAUpdater object.
 */
NAUpdater *
na_updater_new( void )
{
	static const gchar *thisfn = "na_updater_new";
	NAUpdater *updater;

	g_debug( "%s", thisfn );

	updater = g_object_new( NA_UPDATER_TYPE, NULL );

	return( updater );
}

/**
 * na_updater_append_item:
 * @updater: this #NAUpdater object.
 * @item: a #NAObjectItem-derived object to be appended to the tree.
 *
 * Append a new item at the end of the global tree.
 */
void
na_updater_append_item( NAUpdater *updater, NAObjectItem *item )
{
	GList *tree;

	g_return_if_fail( NA_IS_UPDATER( updater ));
	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));

	if( !updater->private->dispose_has_run ){

		g_object_get( G_OBJECT( updater ), NAPIVOT_PROP_TREE, &tree, NULL );
		tree = g_list_append( tree, item );
		g_object_set( G_OBJECT( updater ), NAPIVOT_PROP_TREE, tree, NULL );
	}
}

/**
 * na_updater_insert_item:
 * @updater: this #NAUpdater object.
 * @item: a #NAObjectItem-derived object to be inserted in the tree.
 * @parent_id: the id of the parent, or %NULL.
 * @pos: the position in the children of the parent, starting at zero, or -1.
 *
 * Insert a new item in the global tree.
 */
void
na_updater_insert_item( NAUpdater *updater, NAObjectItem *item, const gchar *parent_id, gint pos )
{
	GList *tree;
	NAObjectItem *parent;

	g_return_if_fail( NA_IS_UPDATER( updater ));
	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));

	if( !updater->private->dispose_has_run ){

		parent = NULL;
		g_object_get( G_OBJECT( updater ), NAPIVOT_PROP_TREE, &tree, NULL );

		if( parent_id ){
			parent = na_pivot_get_item( NA_PIVOT( updater ), parent_id );
		}

		if( parent ){
			na_object_insert_at( parent, item, pos );

		} else {
			tree = g_list_append( tree, item );
			g_object_set( G_OBJECT( updater ), NAPIVOT_PROP_TREE, tree, NULL );
		}
	}
}

/**
 * na_updater_remove_item:
 * @updater: this #NAPivot instance.
 * @item: the #NAObjectItem to be removed from the list.
 *
 * Removes a #NAObjectItem from the hierarchical tree.
 *
 * Note that #NAUpdater also g_object_unref() the removed #NAObjectItem.
 *
 * Last, note that the @item may have been already deleted, when its
 * parents has itself been removed from @updater.
 */
void
na_updater_remove_item( NAUpdater *updater, NAObject *item )
{
	GList *tree;

	g_debug( "na_updater_remove_item: updater=%p, item=%p (%s)",
			( void * ) updater,
			( void * ) item, G_IS_OBJECT( item ) ? G_OBJECT_TYPE_NAME( item ) : "(null)" );

	g_return_if_fail( NA_IS_PIVOT( updater ));

	if( !updater->private->dispose_has_run ){

		if( !na_object_get_parent( item )){
			g_object_get( G_OBJECT( updater ), NAPIVOT_PROP_TREE, &tree, NULL );
			tree = g_list_remove( tree, ( gconstpointer ) item );
			g_object_set( G_OBJECT( updater ), NAPIVOT_PROP_TREE, tree, NULL );
		}

		g_object_unref( item );
	}
}

/**
 * na_updater_is_item_writable:
 * @updater: this #NAUpdater object.
 * @item: the #NAObjectItem to be written.
 * @reason: the reason for why @item may not be writable.
 *
 * Returns: %TRUE: if @item is actually writable, given the current
 * status of its provider, %FALSE else.
 *
 * For an item be actually writable:
 * - the item must not be itself in a read-only store, which has been
 *   checked when first reading it
 * - the provider must be willing (resp. able) to write
 * - the provider must not has been locked by the admin
 * - the writability of the provider must not have been removed by the user
 * - the whole configuration must not have been locked by the admin.
 */
gboolean
na_updater_is_item_writable( const NAUpdater *updater, const NAObjectItem *item, gint *reason )
{
	gboolean writable;
	NAIOProvider *provider;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), FALSE );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), FALSE );

	writable = FALSE;
	if( reason ){
		*reason = NA_IIO_PROVIDER_STATUS_UNDETERMINED;
	}

	if( !updater->private->dispose_has_run ){

		writable = TRUE;
		if( reason ){
			*reason = NA_IIO_PROVIDER_STATUS_WRITABLE;
		}

		if( writable ){
			if( na_object_is_readonly( item )){
				writable = FALSE;
				if( reason ){
					*reason = NA_IIO_PROVIDER_STATUS_ITEM_READONLY;
				}
			}
		}

		if( writable ){
			provider = na_object_get_provider( item );
			if( provider ){
				if( !na_io_provider_is_willing_to_write( provider )){
					writable = FALSE;
					if( reason ){
						*reason = NA_IIO_PROVIDER_STATUS_PROVIDER_NOT_WILLING_TO;
					}
				} else if( na_io_provider_is_locked_by_admin( provider, NA_IPREFS( updater ))){
					writable = FALSE;
					if( reason ){
						*reason = NA_IIO_PROVIDER_STATUS_PROVIDER_LOCKED_BY_ADMIN;
					}
				} else if( !na_io_provider_is_user_writable( provider, NA_IPREFS( updater ))){
					writable = FALSE;
					if( reason ){
						*reason = NA_IIO_PROVIDER_STATUS_PROVIDER_LOCKED_BY_USER;
					}
				} else if( na_pivot_is_configuration_locked_by_admin( NA_PIVOT( updater ))){
					writable = FALSE;
					if( reason ){
						*reason = NA_IIO_PROVIDER_STATUS_CONFIGURATION_LOCKED_BY_ADMIN;
					}
				} else if( !na_io_provider_has_write_api( provider )){
					writable = FALSE;
					if( reason ){
						*reason = NA_IIO_PROVIDER_STATUS_NO_API;
					}
				}

			/* the get_writable_provider() api already takes above checks
			 */
			} else {
				provider = na_io_provider_get_writable_provider( NA_PIVOT( updater ));
				if( !provider ){
					writable = FALSE;
					if( reason ){
						*reason = NA_IIO_PROVIDER_STATUS_NO_PROVIDER_FOUND;
					}
				}
			}
		}
	}

	return( writable );
}

/**
 * na_updater_write_item:
 * @updater: this #NAUpdater instance.
 * @item: a #NAObjectItem to be written down to the storage subsystem.
 * @messages: the I/O provider can allocate and store here its error
 * messages.
 *
 * Writes an item (an action or a menu).
 *
 * Returns: the #NAIIOProvider return code.
 */
guint
na_updater_write_item( const NAUpdater *updater, NAObjectItem *item, GSList **messages )
{
	guint ret;
	gint reason;

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );
	g_return_val_if_fail( messages, ret );

	if( !updater->private->dispose_has_run ){

		NAIOProvider *provider = na_object_get_provider( item );
		if( !provider ){
			provider = na_io_provider_get_writable_provider( NA_PIVOT( updater ));

			if( !provider ){
				ret = NA_IIO_PROVIDER_STATUS_NO_PROVIDER_FOUND;

			} else {
				na_object_set_provider( item, provider );
			}
		}

		if( provider ){
			if( !na_updater_is_item_writable( updater, item, &reason )){
				ret = ( guint ) reason;

			} else {
				ret = na_io_provider_write_item( provider, item, messages );
			}
		}
	}

	return( ret );
}

/**
 * na_updater_delete_item:
 * @updater: this #NAUpdater instance.
 * @item: the #NAObjectItem to be deleted from the storage subsystem.
 * @messages: the I/O provider can allocate and store here its error
 * messages.
 *
 * Deletes an item, action or menu, from the I/O storage subsystem.
 *
 * Returns: the #NAIIOProvider return code.
 *
 * Note that a new item, not already written to an I/O subsystem,
 * doesn't have any attached provider. We so do nothing and return OK...
 */
guint
na_updater_delete_item( const NAUpdater *updater, const NAObjectItem *item, GSList **messages )
{
	guint ret;
	gint reason;

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );
	g_return_val_if_fail( messages, ret );

	if( !updater->private->dispose_has_run ){

		NAIOProvider *provider = na_object_get_provider( item );
		if( provider ){

			if( !na_updater_is_item_writable( updater, item, &reason )){
				ret = ( guint ) reason;

			} else {
				ret = na_io_provider_delete_item( provider, item, messages );
			}

		} else {
			ret = NA_IIO_PROVIDER_CODE_OK;
		}
	}

	return( ret );
}
