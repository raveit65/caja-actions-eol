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

#include <glib/gi18n.h>

#include <api/na-core-utils.h>
#include <api/na-object-api.h>

/* private class data
 */
struct NAObjectIdClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAObjectIdPrivate {
	gboolean   dispose_has_run;
};

static NAObjectClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( NAObjectIdClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *object );
static void     instance_finalize( GObject *object );

static gboolean object_is_valid( const NAObject *object );

static gchar   *v_new_id( const NAObjectId *object, const NAObjectId *new_parent );

GType
na_object_id_get_type( void )
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
	static const gchar *thisfn = "na_object_id_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAObjectIdClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAObjectId ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( NA_OBJECT_TYPE, "NAObjectId", &info, 0 );

	return( type );
}

static void
class_init( NAObjectIdClass *klass )
{
	static const gchar *thisfn = "na_object_id_class_init";
	GObjectClass *object_class;
	NAObjectClass *naobject_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	naobject_class = NA_OBJECT_CLASS( klass );
	naobject_class->dump = NULL;
	naobject_class->copy = NULL;
	naobject_class->are_equal = NULL;
	naobject_class->is_valid = object_is_valid;

	klass->private = g_new0( NAObjectIdClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_object_id_instance_init";
	NAObjectId *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( NA_IS_OBJECT_ID( instance ));

	self = NA_OBJECT_ID( instance );

	self->private = g_new0( NAObjectIdPrivate, 1 );
}

/*
 * note that when the tree store is cleared, Gtk begins with the deepest
 * levels, so that children are disposed before their parent
 * as we try to dispose all children when disposing a parent, we have to
 * remove a disposing child from its parent
 */
static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_object_id_instance_dispose";
	NAObjectId *self;
	NAObjectItem *parent;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_if_fail( NA_IS_OBJECT_ID( object ));

	self = NA_OBJECT_ID( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		parent = na_object_get_parent( object );
		g_debug( "%s: parent=%p (%s)",
				thisfn, ( void * ) parent, parent ? G_OBJECT_TYPE_NAME( parent ) : "n/a" );
		if( parent ){
			na_object_remove_item( parent, object );
			na_object_set_parent( object, NULL );
		}

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
	NAObjectId *self;

	g_return_if_fail( NA_IS_OBJECT_ID( object ));

	self = NA_OBJECT_ID( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/*
 * a NAObjectId is valid if it has a non-null id
 */
static gboolean
object_is_valid( const NAObject *object )
{
	gboolean is_valid;
	gchar *id;

	is_valid = TRUE;

	if( is_valid ){
		id = na_object_get_id( object );
		is_valid = ( id != NULL && strlen( id ) > 0 );
		g_free( id );
	}

	return( is_valid );
}

/**
 * na_object_id_sort_alpha_asc:
 * @a: first #NAObjectId.
 * @b: second #NAObjectId.
 *
 * Sort the objects in alphabetical ascending order of their label.
 *
 * Returns:
 * -1 if @a must be sorted before @b,
 *  0 if @a and @b are equal from the local point of view,
 *  1 if @a must be sorted after @b.
 */
gint
na_object_id_sort_alpha_asc( const NAObjectId *a, const NAObjectId *b )
{
	gchar *label_a, *label_b;
	gint compare;

	label_a = na_object_get_label( a );
	label_b = na_object_get_label( b );

	compare = na_core_utils_str_collate( label_a, label_b );

	g_free( label_b );
	g_free( label_a );

	return( compare );
}

/**
 * na_object_id_sort_alpha_desc:
 * @a: first #NAObjectId.
 * @b: second #NAObjectId.
 *
 * Sort the objects in alphabetical descending order of their label.
 *
 * Returns:
 * -1 if @a must be sorted before @b,
 *  0 if @a and @b are equal from the local point of view,
 *  1 if @a must be sorted after @b.
 */
gint
na_object_id_sort_alpha_desc( const NAObjectId *a, const NAObjectId *b )
{
	return( -1 * na_object_id_sort_alpha_asc( a, b ));
}

/**
 * na_object_id_prepare_for_paste:
 * @object: the #NAObjectId object to be pasted.
 * @relabel: whether this object should be relabeled when pasted.
 * @relabel: whether this item should be renumbered ?
 * @parent: the parent of @object, or %NULL.
 *
 * Prepares @object to be pasted.
 *
 * If a #NAObjectProfile, then @object is attached to the specified
 * #NAObjectAction @action. The identifier is always renumbered to be
 * suitable with the already existing profiles.
 *
 * If a #NAObjectAction or a #NAObjectMenu, a new UUID is allocated if
 * and only if @relabel is %TRUE.
 *
 * Actual relabeling takes place if @relabel is %TRUE, depending of the
 * user preferences.
 */
void
na_object_id_prepare_for_paste( NAObjectId *object, gboolean relabel, gboolean renumber, NAObjectId *parent )
{
	static const gchar *thisfn = "na_object_id_prepare_for_paste";
	GList *subitems, *it;

	g_debug( "%s: object=%p, relabel=%s, renumber=%s, parent=%p",
			thisfn, ( void * ) object, relabel ? "True":"False", renumber ? "True":"False", ( void * ) parent );

	g_return_if_fail( NA_IS_OBJECT_ID( object ));
	g_return_if_fail( !parent || NA_IS_OBJECT_ITEM( parent ));

	if( !object->private->dispose_has_run ){

		if( NA_IS_OBJECT_PROFILE( object )){
			na_object_set_parent( object, parent );
			na_object_set_new_id( object, parent );
			if( renumber && relabel ){
				na_object_set_copy_of_label( object );
			}

		} else {
			if( renumber ){
				na_object_set_new_id( object, NULL );
				if( relabel ){
					na_object_set_copy_of_label( object );
				}
				na_object_set_provider( object, NULL );
				na_object_set_readonly( object, FALSE );
			}
			if( NA_IS_OBJECT_MENU( object )){
				subitems = na_object_get_items( object );
				for( it = subitems ; it ; it = it->next ){
					na_object_prepare_for_paste( it->data, relabel, renumber, NULL );
				}
			}
		}
	}
}

/**
 * na_object_id_set_copy_of_label:
 * @object: the #NAObjectId object whose label is to be changed.
 *
 * Sets the 'Copy of' label.
 */
void
na_object_id_set_copy_of_label( NAObjectId *object )
{
	gchar *label, *new_label;

	g_return_if_fail( NA_IS_OBJECT_ID( object ));

	if( !object->private->dispose_has_run ){

		label = na_object_get_label( object );

		/* i18n: copied items have a label as 'Copy of original label' */
		new_label = g_strdup_printf( _( "Copy of %s" ), label );

		na_object_set_label( object, new_label );

		g_free( new_label );
		g_free( label );
	}
}

/**
 * na_object_id_set_new_id:
 * @object: the #NAObjectId object whose internal identifiant is to be
 * set.
 * @new_parent: if @object is a #NAObjectProfile, then @new_parent
 * should be set to the #NAObjectActio new parent. Else, it would not
 * be possible to allocate a new profile id compatible with already
 * existing ones.
 *
 * Request a new id to the derived class, and set it.
 */
void
na_object_id_set_new_id( NAObjectId *object, const NAObjectId *new_parent )
{
	gchar *id;

	g_return_if_fail( NA_IS_OBJECT_ID( object ));
	g_return_if_fail( !new_parent || NA_IS_OBJECT_ID( new_parent ));

	if( !object->private->dispose_has_run ){

		id = v_new_id( object, new_parent );

		if( id ){
			na_object_set_id( object, id );
			g_free( id );
		}
	}
}

static gchar *
v_new_id( const NAObjectId *object, const NAObjectId *new_parent )
{
	gchar *new_id;
	GList *hierarchy, *ih;
	gboolean found;

	found = FALSE;
	new_id = NULL;
	hierarchy = g_list_reverse( na_object_get_hierarchy( NA_OBJECT( object )));
	/*g_debug( "na_object_id_most_derived_id: object=%p (%s)",
					( void * ) object, G_OBJECT_TYPE_NAME( object ));*/

	for( ih = hierarchy ; ih && !found ; ih = ih->next ){
		if( NA_OBJECT_ID_CLASS( ih->data )->new_id ){
			new_id = NA_OBJECT_ID_CLASS( ih->data )->new_id( object, new_parent );
			found = TRUE;
		}
		if( G_OBJECT_CLASS_TYPE( ih->data ) == NA_OBJECT_ID_TYPE ){
			break;
		}
	}

	na_object_free_hierarchy( hierarchy );

	return( new_id );
}
