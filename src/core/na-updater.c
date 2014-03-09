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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

#include "na-io-provider.h"
#include "na-settings.h"
#include "na-updater.h"

/* private class data
 */
struct _NAUpdaterClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _NAUpdaterPrivate {
	gboolean dispose_has_run;
	gboolean are_preferences_locked;
	gboolean is_level_zero_writable;
};

static NAPivotClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( NAUpdaterClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *object );
static void     instance_finalize( GObject *object );

static gboolean are_preferences_locked( const NAUpdater *updater );
static gboolean is_level_zero_writable( const NAUpdater *updater );
static void     set_writability_status( NAObjectItem *item, const NAUpdater *updater );

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

	type = g_type_register_static( NA_TYPE_PIVOT, "NAUpdater", &info, 0 );

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

	g_return_if_fail( NA_IS_UPDATER( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = NA_UPDATER( instance );

	self->private = g_new0( NAUpdaterPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_updater_instance_dispose";
	NAUpdater *self;

	g_return_if_fail( NA_IS_UPDATER( object ));

	self = NA_UPDATER( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

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

	g_return_if_fail( NA_IS_UPDATER( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = NA_UPDATER( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/*
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

	updater = g_object_new( NA_TYPE_UPDATER, NULL );

	updater->private->are_preferences_locked = are_preferences_locked( updater );
	updater->private->is_level_zero_writable = is_level_zero_writable( updater );

	g_debug( "%s: is_level_zero_writable=%s",
			thisfn,
			updater->private->is_level_zero_writable ? "True":"False" );

	return( updater );
}

static gboolean
are_preferences_locked( const NAUpdater *updater )
{
	gboolean are_locked;
	gboolean mandatory;

	are_locked = na_settings_get_boolean( NA_IPREFS_ADMIN_PREFERENCES_LOCKED, NULL, &mandatory );

	return( are_locked && mandatory );
}

static gboolean
is_level_zero_writable( const NAUpdater *updater )
{
	GSList *level_zero;
	gboolean mandatory;

	level_zero = na_settings_get_string_list( NA_IPREFS_ITEMS_LEVEL_ZERO_ORDER, NULL, &mandatory );

	na_core_utils_slist_free( level_zero );

	g_debug( "na_updater_is_level_zero_writable: NA_IPREFS_ITEMS_LEVEL_ZERO_ORDER: mandatory=%s",
			mandatory ? "True":"False" );

	return( !mandatory );
}

/*
 * na_updater_check_item_writability_status:
 * @updater: this #NAUpdater object.
 * @item: the #NAObjectItem to be written.
 *
 * Compute and set the writability status of the @item.
 *
 * For an item be actually writable:
 * - the item must not be itself in a read-only store, which has been
 *   checked when first reading it
 * - the provider must be willing (resp. able) to write
 * - the provider must not has been locked by the admin, nor by the user
 *
 * If the item does not have a parent, then the level zero must be writable.
 */
void
na_updater_check_item_writability_status( const NAUpdater *updater, const NAObjectItem *item )
{
	gboolean writable;
	NAIOProvider *provider;
	NAObjectItem *parent;
	guint reason;

	g_return_if_fail( NA_IS_UPDATER( updater ));
	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));

	writable = FALSE;
	reason = NA_IIO_PROVIDER_STATUS_UNDETERMINED;

	if( !updater->private->dispose_has_run ){

		writable = TRUE;
		reason = NA_IIO_PROVIDER_STATUS_WRITABLE;

		/* Writability status of the item has been determined at load time
		 * (cf. e.g. io-desktop/cadp-reader.c:read_done_item_is_writable()).
		 * Though I'm plenty conscious that this status is subject to many
		 * changes during the life of the item (e.g. by modifying permissions
		 * on the underlying store), it is just more efficient to not reevaluate
		 * this status each time we need it, and enough for our needs..
		 */
		if( writable ){
			if( na_object_is_readonly( item )){
				writable = FALSE;
				reason = NA_IIO_PROVIDER_STATUS_ITEM_READONLY;
			}
		}

		if( writable ){
			provider = na_object_get_provider( item );
			if( provider ){
				writable = na_io_provider_is_finally_writable( provider, &reason );

			/* the get_writable_provider() api already takes care of above checks
			 */
			} else {
				provider = na_io_provider_find_writable_io_provider( NA_PIVOT( updater ));
				if( !provider ){
					writable = FALSE;
					reason = NA_IIO_PROVIDER_STATUS_NO_PROVIDER_FOUND;
				}
			}
		}

		/* if needed, the level zero must be writable
		 */
		if( writable ){
			parent = ( NAObjectItem * ) na_object_get_parent( item );
			if( !parent ){
				if( updater->private->is_level_zero_writable ){
					reason = NA_IIO_PROVIDER_STATUS_LEVEL_ZERO;
				}
			}
		}
	}

	na_object_set_writability_status( item, writable, reason );
}

/*
 * na_updater_are_preferences_locked:
 * @updater: the #NAUpdater application object.
 *
 * Returns: %TRUE if preferences have been globally locked down by an
 * admin, %FALSE else.
 */
gboolean
na_updater_are_preferences_locked( const NAUpdater *updater )
{
	gboolean are_locked;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), TRUE );

	are_locked = TRUE;

	if( !updater->private->dispose_has_run ){

		are_locked = updater->private->are_preferences_locked;
	}

	return( are_locked );
}

/*
 * na_updater_is_level_zero_writable:
 * @updater: the #NAUpdater application object.
 *
 * As of 3.1.0, level-zero is written as a user preference.
 *
 * This function considers that the level_zero is writable if it is not
 * a mandatory preference.
 * Whether preferences themselves are or not globally locked is not
 * considered here (as imho, level zero is not really and semantically
 * part of user preferences).
 *
 * This function only considers the case of the level zero itself.
 * It does not take into account whether the i/o provider (if any)
 * is writable, or if the item iself is not read only.
 *
 * Returns: %TRUE if we are able to update the level-zero list of items,
 * %FALSE else.
 */
gboolean
na_updater_is_level_zero_writable( const NAUpdater *updater )
{
	gboolean is_writable;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), FALSE );

	is_writable = FALSE;

	if( !updater->private->dispose_has_run ){

		is_writable = updater->private->is_level_zero_writable;
	}

	return( is_writable );
}

/*
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

		g_object_get( G_OBJECT( updater ), PIVOT_PROP_TREE, &tree, NULL );
		tree = g_list_append( tree, item );
		g_object_set( G_OBJECT( updater ), PIVOT_PROP_TREE, tree, NULL );
	}
}

/*
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
		g_object_get( G_OBJECT( updater ), PIVOT_PROP_TREE, &tree, NULL );

		if( parent_id ){
			parent = na_pivot_get_item( NA_PIVOT( updater ), parent_id );
		}

		if( parent ){
			na_object_insert_at( parent, item, pos );

		} else {
			tree = g_list_append( tree, item );
			g_object_set( G_OBJECT( updater ), PIVOT_PROP_TREE, tree, NULL );
		}
	}
}

/*
 * na_updater_remove_item:
 * @updater: this #NAPivot instance.
 * @item: the #NAObjectItem to be removed from the list.
 *
 * Removes a #NAObjectItem from the hierarchical tree. Does not delete it.
 */
void
na_updater_remove_item( NAUpdater *updater, NAObject *item )
{
	GList *tree;
	NAObjectItem *parent;

	g_return_if_fail( NA_IS_PIVOT( updater ));

	if( !updater->private->dispose_has_run ){

		g_debug( "na_updater_remove_item: updater=%p, item=%p (%s)",
				( void * ) updater,
				( void * ) item, G_IS_OBJECT( item ) ? G_OBJECT_TYPE_NAME( item ) : "(null)" );

		parent = na_object_get_parent( item );
		if( parent ){
			tree = na_object_get_items( parent );
			tree = g_list_remove( tree, ( gconstpointer ) item );
			na_object_set_items( parent, tree );

		} else {
			g_object_get( G_OBJECT( updater ), PIVOT_PROP_TREE, &tree, NULL );
			tree = g_list_remove( tree, ( gconstpointer ) item );
			g_object_set( G_OBJECT( updater ), PIVOT_PROP_TREE, tree, NULL );
		}
	}
}

/**
 * na_updater_should_pasted_be_relabeled:
 * @updater: this #NAUpdater instance.
 * @object: the considered #NAObject-derived object.
 *
 * Whether the specified object should be relabeled when pasted ?
 *
 * Returns: %TRUE if the object should be relabeled, %FALSE else.
 */
gboolean
na_updater_should_pasted_be_relabeled( const NAUpdater *updater, const NAObject *item )
{
	static const gchar *thisfn = "na_updater_should_pasted_be_relabeled";
	gboolean relabel;

	if( NA_IS_OBJECT_MENU( item )){
		relabel = na_settings_get_boolean( NA_IPREFS_RELABEL_DUPLICATE_MENU, NULL, NULL );

	} else if( NA_IS_OBJECT_ACTION( item )){
		relabel = na_settings_get_boolean( NA_IPREFS_RELABEL_DUPLICATE_ACTION, NULL, NULL );

	} else if( NA_IS_OBJECT_PROFILE( item )){
		relabel = na_settings_get_boolean( NA_IPREFS_RELABEL_DUPLICATE_PROFILE, NULL, NULL );

	} else {
		g_warning( "%s: unknown item type at %p", thisfn, ( void * ) item );
		g_return_val_if_reached( FALSE );
	}

	return( relabel );
}

/*
 * na_updater_load_items:
 * @updater: this #NAUpdater instance.
 *
 * Loads the items, updating simultaneously their writability status.
 *
 * Returns: a pointer (not a ref) on the loaded tree.
 *
 * Since: 3.1
 */
GList *
na_updater_load_items( NAUpdater *updater )
{
	static const gchar *thisfn = "na_updater_load_items";
	GList *tree;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), NULL );

	tree = NULL;

	if( !updater->private->dispose_has_run ){
		g_debug( "%s: updater=%p (%s)", thisfn, ( void * ) updater, G_OBJECT_TYPE_NAME( updater ));

		na_pivot_load_items( NA_PIVOT( updater ));
		tree = na_pivot_get_items( NA_PIVOT( updater ));
		g_list_foreach( tree, ( GFunc ) set_writability_status, ( gpointer ) updater );
	}

	return( tree );
}

static void
set_writability_status( NAObjectItem *item, const NAUpdater *updater )
{
	GList *children;

	na_updater_check_item_writability_status( updater, item );

	if( NA_IS_OBJECT_MENU( item )){
		children = na_object_get_items( item );
		g_list_foreach( children, ( GFunc ) set_writability_status, ( gpointer ) updater );
	}
}

/*
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

	ret = NA_IIO_PROVIDER_CODE_PROGRAM_ERROR;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), ret );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), ret );
	g_return_val_if_fail( messages, ret );

	if( !updater->private->dispose_has_run ){

		NAIOProvider *provider = na_object_get_provider( item );

		if( !provider ){
			provider = na_io_provider_find_writable_io_provider( NA_PIVOT( updater ));
			g_return_val_if_fail( provider, NA_IIO_PROVIDER_STATUS_NO_PROVIDER_FOUND );
		}

		if( provider ){
			ret = na_io_provider_write_item( provider, item, messages );
		}
	}

	return( ret );
}

/*
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
	NAIOProvider *provider;

	g_return_val_if_fail( NA_IS_UPDATER( updater ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
	g_return_val_if_fail( messages, NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );

	ret = NA_IIO_PROVIDER_CODE_OK;

	if( !updater->private->dispose_has_run ){

		provider = na_object_get_provider( item );

		/* provider may be NULL if the item has been deleted from the UI
		 * without having been ever saved
		 */
		if( provider ){
			g_return_val_if_fail( NA_IS_IO_PROVIDER( provider ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
			ret = na_io_provider_delete_item( provider, item, messages );
		}
	}

	return( ret );
}
