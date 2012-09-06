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

#include <string.h>
#include <uuid/uuid.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

/* private class data
 */
struct NAObjectItemClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAObjectItemPrivate {
	gboolean   dispose_has_run;

	void      *provider_data;

	/* dynamically set when reading the item from the I/O storage
	 * subsystem; may be reset from FALSE to TRUE if a write operation
	 * has returned an error.
	 * defaults to FALSE for snew, not yet written to a provider, item
	 */
	gboolean   readonly;
};

static NAObjectIdClass *st_parent_class = NULL;

static GType  register_type( void );
static void   class_init( NAObjectItemClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *object );
static void   instance_finalize( GObject *object );

static void   object_copy( NAObject*target, const NAObject *source, gboolean recursive );

static gchar *object_id_new_id( const NAObjectId *item, const NAObjectId *new_parent );

static void   copy_children( NAObjectItem *target, const NAObjectItem *source );
static void   rebuild_children_slist( NAObjectItem *item );

GType
na_object_item_get_type( void )
{
	static GType item_type = 0;

	if( item_type == 0 ){
		item_type = register_type();
	}

	return( item_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_object_item_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAObjectItemClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAObjectItem ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( NA_OBJECT_ID_TYPE, "NAObjectItem", &info, 0 );

	return( type );
}

static void
class_init( NAObjectItemClass *klass )
{
	static const gchar *thisfn = "na_object_item_class_init";
	GObjectClass *object_class;
	NAObjectClass *naobject_class;
	NAObjectIdClass *naobjectid_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	naobject_class = NA_OBJECT_CLASS( klass );
	naobject_class->dump = NULL;
	naobject_class->copy = object_copy;
	naobject_class->are_equal = NULL;
	naobject_class->is_valid = NULL;

	naobjectid_class = NA_OBJECT_ID_CLASS( klass );
	naobjectid_class->new_id = object_id_new_id;

	klass->private = g_new0( NAObjectItemClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_object_item_instance_init";
	NAObjectItem *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( NA_IS_OBJECT_ITEM( instance ));

	self = NA_OBJECT_ITEM( instance );

	self->private = g_new0( NAObjectItemPrivate, 1 );
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_object_item_instance_dispose";
	NAObjectItem *self;
	GList *children;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_if_fail( NA_IS_OBJECT_ITEM( object ));

	self = NA_OBJECT_ITEM( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		children = na_object_get_items( self );
		g_debug( "%s: children=%p (count=%d)", thisfn, ( void * ) children, g_list_length( children ));
		na_object_set_items( self, NULL );
		na_object_unref_items( children );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	NAObjectItem *self;

	g_return_if_fail( NA_IS_OBJECT_ITEM( object ));

	self = NA_OBJECT_ITEM( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
object_copy( NAObject *target, const NAObject *source, gboolean recursive )
{
	g_return_if_fail( NA_IS_OBJECT_ITEM( target ));
	g_return_if_fail( NA_IS_OBJECT_ITEM( source ));

	if( !NA_OBJECT_ITEM( target )->private->dispose_has_run &&
		!NA_OBJECT_ITEM( source )->private->dispose_has_run ){

		if( recursive ){
			copy_children( NA_OBJECT_ITEM( target ), NA_OBJECT_ITEM( source ));
		}
	}
}

/*
 * new_parent is not relevant when allocating a new UUID for an action
 * or a menu ; it may safely be left as NULL though there is no gain to
 * check this
 */
static gchar *
object_id_new_id( const NAObjectId *item, const NAObjectId *new_parent )
{
	GList *childs, *it;
	uuid_t uuid;
	gchar uuid_str[64];
	gchar *new_uuid = NULL;

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), NULL );

	if( !NA_OBJECT_ITEM( item )->private->dispose_has_run ){

		/* recurse into NAObjectItems childs
		 * i.e., if a menu, recurse into embedded actions
		 */
		childs = na_object_get_items( item );
		for( it = childs ; it ; it = it->next ){
			na_object_set_new_id( it->data, new_parent );
		}

		uuid_generate( uuid );
		uuid_unparse_lower( uuid, uuid_str );
		new_uuid = g_strdup( uuid_str );
	}

	return( new_uuid );
}

/**
 * na_object_item_are_equal:
 * @a: the first (original) #NAObjectItem instance.
 * @b: the second #NAObjectItem instance.
 *
 * Returns: %TRUE if @a is equal to @b.
 *
 * This function participates to the #na_iduplicable_check_status() stack,
 * and is triggered after all comparable elementary data (in #NAIFactoryObject
 * sense) have already been successfully compared.
 *
 * We have to deal here with the subitems: comparing children by their ids
 * between @a and @b.
 *
 * Note that, when called from na_object_check_status, the status of children
 * have already been checked, and so we should be able to rely on them.
 */
gboolean
na_object_item_are_equal( const NAObjectItem *a, const NAObjectItem *b )
{
	static const gchar *thisfn = "na_object_item_are_equal";
	gboolean equal;
	GList *a_children, *b_children, *it;
	gchar *first_id, *second_id;
	NAObjectId *first_obj, *second_obj;
	gint first_pos, second_pos;
	GList *second_list;

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( a ), FALSE );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( b ), FALSE );

	equal = FALSE;

	if( !NA_OBJECT_ITEM( a )->private->dispose_has_run &&
		!NA_OBJECT_ITEM( b )->private->dispose_has_run ){

		equal = TRUE;

		if( equal ){
			a_children = na_object_get_items( a );
			b_children = na_object_get_items( b );
			equal = ( g_list_length( a_children ) == g_list_length( b_children ));
			if( !equal ){
				g_debug( "%s: %p (%s) not equal as g_list_length not equal",
						thisfn, ( void * ) b, G_OBJECT_TYPE_NAME( b ));
				g_debug( "a=%p children_count=%u", ( void * ) a, g_list_length( a_children ));
				for( it = a_children ; it ; it = it->next ){
					g_debug( "a_child=%p", ( void * ) it->data );
				}
				g_debug( "b=%p children_count=%u", ( void * ) b, g_list_length( b_children ));
				for( it = b_children ; it ; it = it->next ){
					g_debug( "b_child=%p", ( void * ) it->data );
				}
			}
		}

		if( equal ){
			for( it = a_children ; it && equal ; it = it->next ){
				first_id = na_object_get_id( it->data );
				second_obj = na_object_get_item( b, first_id );
				first_pos = -1;
				second_pos = -1;

				if( second_obj ){
					first_pos = g_list_position( a_children, it );
					second_list = g_list_find( b_children, second_obj );
					second_pos = g_list_position( b_children, second_list );

					if( first_pos != second_pos ){
						equal = FALSE;
						g_debug( "%s: %p (%s) not equal as child %s is at pos %u",
								thisfn, ( void * ) b, G_OBJECT_TYPE_NAME( b ), first_id, second_pos );
					}

				} else {
					equal = FALSE;
					g_debug( "%s: %p (%s) not equal as child %s removed",
							thisfn, ( void * ) b, G_OBJECT_TYPE_NAME( b ), first_id );
				}

				g_free( first_id );
			}
		}

		if( equal ){
			for( it = b_children ; it && equal ; it = it->next ){
				second_id = na_object_get_id( it->data );
				first_obj = na_object_get_item( a, second_id );

				if( !first_obj ){
					equal = FALSE;
					g_debug( "%s: %p (%s) not equal as child %s added",
							thisfn, ( void * ) b, G_OBJECT_TYPE_NAME( b ), second_id );

				} else {
					equal = !na_object_is_modified( it->data );

					if( !equal ){
						g_debug( "%s: %p (%s) not equal as child %s modified",
								thisfn, ( void * ) b, G_OBJECT_TYPE_NAME( b ), second_id );
					}
				}

				g_free( second_id );
			}
		}

		/*g_debug( "na_object_item_object_are_equal: a=%p (%s), b=%p (%s), are_equal=%s",
				( void * ) a, G_OBJECT_TYPE_NAME( a ),
				( void * ) b, G_OBJECT_TYPE_NAME( b ),
				equal ? "True":"False" );*/
	}

	return( equal );
}

/**
 * na_object_item_get_item:
 * @item: the #NAObjectItem from which we want retrieve a subitem.
 * @id: the id of the searched subitem.
 *
 * Returns: a pointer to the #NAObjectId-derived child with the required id.
 *
 * The returned #NAObjectId is owned by the @item object ; the
 * caller should not try to g_free() nor g_object_unref() it.
 */
NAObjectId *
na_object_item_get_item( const NAObjectItem *item, const gchar *id )
{
	GList *childs, *it;
	NAObjectId *found = NULL;
	NAObjectId *isub;
	gchar *isubid;

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), NULL );

	if( !item->private->dispose_has_run ){

		childs = na_object_get_items( item );
		for( it = childs ; it && !found ; it = it->next ){
			isub = NA_OBJECT_ID( it->data );
			isubid = na_object_get_id( isub );
			if( !strcmp( id, isubid )){
				found = isub;
			}
			g_free( isubid );
		}
	}

	return( found );
}

/**
 * na_object_item_get_position:
 * @object: this #NAObjectItem object.
 * @child: a #NAObjectId-derived child.
 *
 * Returns: the position of @child in the subitems list of @object,
 * starting from zero, or -1 if not found.
 */
gint
na_object_item_get_position( const NAObjectItem *object, const NAObjectId *child )
{
	gint pos = -1;
	GList *childs;

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( object ), pos );
	g_return_val_if_fail( NA_IS_OBJECT_ID( child ), pos );

	if( !child ){
		return( pos );
	}

	if( !object->private->dispose_has_run ){

		childs = na_object_get_items( object );
		pos = g_list_index( childs, ( gconstpointer ) child );
	}

	return( pos );
}

/**
 * na_object_item_append_item:
 * @item: the #NAObjectItem to which add the subitem.
 * @child: a #NAObjectId to be added to list of subitems.
 *
 * Appends a new @child to the list of subitems of @item,
 * and setup the parent pointer of the child to its new parent.
 *
 * Doesn't modify the reference count on @object.
 */
void
na_object_item_append_item( NAObjectItem *item, const NAObjectId *child )
{
	GList *children;

	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));
	g_return_if_fail( NA_IS_OBJECT_ID( child ));

	if( !item->private->dispose_has_run ){

		children = na_object_get_items( item );

		if( !g_list_find( children, ( gpointer ) child )){

			children = g_list_append( children, ( gpointer ) child );
			na_object_set_parent( child, item );
			na_object_set_items( item, children );
		}
	}
}

/**
 * na_object_item_insert_at:
 * @item: the #NAObjectItem in which add the subitem.
 * @object: a #NAObjectId-derived to be inserted in the list of subitems.
 * @pos: the position at which the @object child should be inserted.
 *
 * Inserts a new @object in the list of subitems of @item.
 *
 * Doesn't modify the reference count on @object.
 */
void
na_object_item_insert_at( NAObjectItem *item, const NAObjectId *object, gint pos )
{
	GList *childs, *it;
	gint i;

	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));
	g_return_if_fail( NA_IS_OBJECT_ID( object ));

	if( !item->private->dispose_has_run ){

		childs = na_object_get_items( item );
		if( pos == -1 || pos >= g_list_length( childs )){
			na_object_append_item( item, object );

		} else {
			i = 0;
			for( it = childs ; it && i <= pos ; it = it->next ){
				if( i == pos ){
					childs = g_list_insert_before( childs, it, ( gpointer ) object );
				}
				i += 1;
			}
			na_object_set_items( item, childs );
		}
	}
}

/**
 * na_object_item_insert_item:
 * @item: the #NAObjectItem to which add the subitem.
 * @object: a #NAObject to be inserted in the list of subitems.
 * @before: the #NAObject before which the @object should be inserted.
 *
 * Inserts a new @object in the list of subitems of @item.
 *
 * Doesn't modify the reference count on @object.
 */
void
na_object_item_insert_item( NAObjectItem *item, const NAObject *object, const NAObject *before )
{
	GList *childs;
	GList *before_list;

	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));
	g_return_if_fail( NA_IS_OBJECT( object ));
	g_return_if_fail( !before || NA_IS_OBJECT( before ));

	if( !item->private->dispose_has_run ){

		childs = na_object_get_items( item );
		if( !g_list_find( childs, ( gpointer ) object )){

			before_list = NULL;

			if( before ){
				before_list = g_list_find( childs, ( gconstpointer ) before );
			}

			if( before_list ){
				childs = g_list_insert_before( childs, before_list, ( gpointer ) object );
			} else {
				childs = g_list_prepend( childs, ( gpointer ) object );
			}

			na_object_set_items( item, childs );
		}
	}
}

/**
 * na_object_item_remove_item:
 * @item: the #NAObjectItem from which the subitem must be removed.
 * @object: a #NAObjectId-derived to be removed from the list of subitems.
 *
 * Removes an @object from the list of subitems of @item.
 *
 * Doesn't modify the reference count on @object.
 */
void
na_object_item_remove_item( NAObjectItem *item, const NAObjectId *object )
{
	GList *children;

	g_return_if_fail( NA_IS_OBJECT_ITEM( item ));
	g_return_if_fail( NA_IS_OBJECT_ID( object ));

	if( !item->private->dispose_has_run ){

		children = na_object_get_items( item );

		if( children ){
			g_debug( "na_object_item_remove_item: removing %p (%s) from %p (%s)",
					( void * ) object, G_OBJECT_TYPE_NAME( object ),
					( void * ) item, G_OBJECT_TYPE_NAME( item ));

			children = g_list_remove( children, ( gconstpointer ) object );
			g_debug( "na_object_item_remove_item: after: children=%p, count=%u", ( void * ) children, g_list_length( children ));
			na_object_set_items( item, children );
		}
	}
}

/**
 * na_object_item_get_items_count:
 * @item: the #NAObjectItem from which we want a count of subitems.
 *
 * Returns: the count of subitems of @item.
 */
guint
na_object_item_get_items_count( const NAObjectItem *item )
{
	guint count = 0;
	GList *childs;

	/*g_debug( "na_object_item_get_items_count: item=%p (%s)", ( void * ) item, G_OBJECT_TYPE_NAME( item ));*/
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), 0 );

	if( !item->private->dispose_has_run ){

		childs = na_object_get_items( item );
		count = childs ? g_list_length( childs ) : 0;
	}

	return( count );
}

/**
 * na_object_item_count_items:
 * @items: a list if #NAObject-derived to be counted.
 * @menus: will be set to the count of menus.
 * @actions: will be set to the count of actions.
 * @profiles: will be set to the count of profiles.
 * @recurse: whether to recursively count all items, or only those in
 *  level zero of the list.
 *
 * Count the numbers of items if the provided list.
 *
 * As this function is recursive, the counters should be initialized by
 * the caller before calling it.
 */
void
na_object_item_count_items( GList *items, gint *menus, gint *actions, gint *profiles, gboolean recurse )
{
	GList *it;

	/*g_debug( "na_object_item_count_items: items=%p (count=%d), menus=%d, actions=%d, profiles=%d",
			( void * ) items, items ? g_list_length( items ) : 0, *menus, *actions, *profiles );*/

	for( it = items ; it ; it = it->next ){

		if( recurse ){
			if( NA_IS_OBJECT_ITEM( it->data )){
				GList *subitems = na_object_get_items( it->data );
				na_object_count_items( subitems, menus, actions, profiles, recurse );
			}
		}

		if( NA_IS_OBJECT_MENU( it->data )){
			*menus += 1;

		} else if( NA_IS_OBJECT_ACTION( it->data )){
			*actions += 1;

		} else if( NA_IS_OBJECT_PROFILE( it->data )){
			*profiles += 1;
		}
	}
}

/**
 * na_object_item_unref_items:
 * @list: a list of #NAObject-derived items.
 *
 * Unref only the first level the #NAObject of the list, freeing the list at last.
 *
 * This is rather only used by #NAPivot.
 */
void
na_object_item_unref_items( GList *items )
{
	g_list_foreach( items, ( GFunc ) g_object_unref, NULL );
	g_list_free( items );
}

/**
 * na_object_item_unref_items_rec:
 * @list: a list of #NAObject-derived items.
 *
 * Recursively unref the #NAObject of the list, freeing the list at last.
 *
 * This is heavily used by NACT.
 */
void
na_object_item_unref_items_rec( GList *items )
{
	GList *it;

	for( it = items ; it ; it = it->next ){
		na_object_unref( it->data );
	}

	g_list_free( items );
}

/**
 * na_object_item_factory_write_start:
 * @item: this #NAObjectItem-derived object.
 *
 * Rebuild the string list of children.
 */
void
na_object_item_factory_write_start( NAObjectItem *item )
{
	rebuild_children_slist( item );
}

static void
copy_children( NAObjectItem *target, const NAObjectItem *source )
{
	static const gchar *thisfn = "na_object_item_copy_children";
	GList *tgt_children, *src_children, *ic;
	NAObject *dup;

	tgt_children = na_object_get_items( target );
	if( tgt_children ){
		g_warning( "%s: target_children=%p (count=%d)",
				thisfn, ( void * ) tgt_children, g_list_length( tgt_children ));
		g_return_if_fail( tgt_children == NULL );
	}

	src_children = na_object_get_items( source );
	for( ic = src_children ; ic ; ic = ic->next ){

		dup = ( NAObject * ) na_object_duplicate( ic->data );
		na_object_set_parent( dup, target );
		tgt_children = g_list_prepend( tgt_children, dup );
	}

	tgt_children = g_list_reverse( tgt_children );
	na_object_set_items( target, tgt_children );
}

static void
rebuild_children_slist( NAObjectItem *item )
{
	GSList *slist;
	GList *subitems, *it;
	gchar *id;

	slist = NULL;

	if( !item->private->dispose_has_run ){

		subitems = na_object_get_items( item );

		for( it = subitems ; it ; it = it->next ){
			id = na_object_get_id( it->data );
			slist = g_slist_prepend( slist, id );
		}
		slist = g_slist_reverse( slist );

		na_object_set_items_slist( item, slist );

		na_core_utils_slist_free( slist );
	}
}
