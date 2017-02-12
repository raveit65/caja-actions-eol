/*
 * Caja ObjectActions
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>

#include <api/na-core-utils.h>
#include <api/na-iio-provider.h>
#include <api/na-object-api.h>

#include "na-factory-provider.h"
#include "na-factory-object.h"

/* private class data
 */
struct _NAObjectActionClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _NAObjectActionPrivate {
	gboolean dispose_has_run;
};

/* i18n: default label for a new action */
#define NEW_CAJA_ACTION				N_( "New Caja action" )

extern NADataGroup action_data_groups [];		/* defined in na-object-action-factory.c */
extern NADataDef   data_def_action_v1 [];		/* defined in na-object-action-factory.c */

static NAObjectItemClass *st_parent_class = NULL;

static GType        register_type( void );
static void         class_init( NAObjectActionClass *klass );
static void         instance_init( GTypeInstance *instance, gpointer klass );
static void         instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void         instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void         instance_dispose( GObject *object );
static void         instance_finalize( GObject *object );

static void         object_dump( const NAObject *object );
static gboolean     object_are_equal( const NAObject *a, const NAObject *b );
static gboolean     object_is_valid( const NAObject *object );

static void         ifactory_object_iface_init( NAIFactoryObjectInterface *iface, void *user_data );
static guint        ifactory_object_get_version( const NAIFactoryObject *instance );
static NADataGroup *ifactory_object_get_groups( const NAIFactoryObject *instance );
static void         ifactory_object_read_done( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
static guint        ifactory_object_write_start( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );
static guint        ifactory_object_write_done( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );

static void         icontext_iface_init( NAIContextInterface *iface, void *user_data );
static gboolean     icontext_is_candidate( NAIContext *object, guint target, GList *selection );

static NAObjectProfile *read_done_convert_v1_to_v2( NAIFactoryObject *instance );
static void             read_done_deals_with_toolbar_label( NAIFactoryObject *instance );

static guint        write_done_write_profiles( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );

static gboolean     is_valid_label( const NAObjectAction *action );
static gboolean     is_valid_toolbar_label( const NAObjectAction *action );

GType
na_object_action_get_type( void )
{
	static GType action_type = 0;

	if( action_type == 0 ){

		action_type = register_type();
	}

	return( action_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_object_action_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAObjectActionClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAObjectAction ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo icontext_iface_info = {
		( GInterfaceInitFunc ) icontext_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ifactory_object_iface_info = {
		( GInterfaceInitFunc ) ifactory_object_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( NA_TYPE_OBJECT_ITEM, "NAObjectAction", &info, 0 );

	g_type_add_interface_static( type, NA_TYPE_ICONTEXT, &icontext_iface_info );

	g_type_add_interface_static( type, NA_TYPE_IFACTORY_OBJECT, &ifactory_object_iface_info );

	return( type );
}

static void
class_init( NAObjectActionClass *klass )
{
	static const gchar *thisfn = "na_object_action_class_init";
	GObjectClass *object_class;
	NAObjectClass *naobject_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->set_property = instance_set_property;
	object_class->get_property = instance_get_property;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	naobject_class = NA_OBJECT_CLASS( klass );
	naobject_class->dump = object_dump;
	naobject_class->are_equal = object_are_equal;
	naobject_class->is_valid = object_is_valid;

	klass->private = g_new0( NAObjectActionClassPrivate, 1 );

	na_factory_object_define_properties( object_class, action_data_groups );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_object_action_instance_init";
	NAObjectAction *self;

	g_return_if_fail( NA_IS_OBJECT_ACTION( instance ));

	self = NA_OBJECT_ACTION( instance );

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self->private = g_new0( NAObjectActionPrivate, 1 );
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	g_return_if_fail( NA_IS_OBJECT_ACTION( object ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NA_OBJECT_ACTION( object )->private->dispose_has_run ){

		na_factory_object_get_as_value( NA_IFACTORY_OBJECT( object ), g_quark_to_string( property_id ), value );
	}
}

static void
instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec )
{
	g_return_if_fail( NA_IS_OBJECT_ACTION( object ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NA_OBJECT_ACTION( object )->private->dispose_has_run ){

		na_factory_object_set_from_value( NA_IFACTORY_OBJECT( object ), g_quark_to_string( property_id ), value );
	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_object_action_instance_dispose";
	NAObjectAction *self;

	g_return_if_fail( NA_IS_OBJECT_ACTION( object ));

	self = NA_OBJECT_ACTION( object );

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
	static const gchar *thisfn = "na_object_action_instance_finalize";
	NAObjectAction *self;

	g_return_if_fail( NA_IS_OBJECT_ACTION( object ));

	self = NA_OBJECT_ACTION( object );

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
object_dump( const NAObject *object )
{
	static const char *thisfn = "na_object_action_object_dump";
	NAObjectAction *self;

	g_return_if_fail( NA_IS_OBJECT_ACTION( object ));

	self = NA_OBJECT_ACTION( object );

	if( !self->private->dispose_has_run ){
		g_debug( "%s: object=%p (%s, ref_count=%d)", thisfn,
				( void * ) object, G_OBJECT_TYPE_NAME( object ), G_OBJECT( object )->ref_count );

		/* chain up to the parent class */
		if( NA_OBJECT_CLASS( st_parent_class )->dump ){
			NA_OBJECT_CLASS( st_parent_class )->dump( object );
		}

		g_debug( "+- end of dump" );
	}
}

/*
 * @a is the original object
 * @b is the current one
 *
 * Even if they have both the same children list, the current action is
 * considered modified as soon as one of its profile is itself modified.
 */
static gboolean
object_are_equal( const NAObject *a, const NAObject *b )
{
	static const gchar *thisfn = "na_object_action_object_are_equal";
	GList *it;
	gboolean are_equal;

	g_debug( "%s: a=%p, b=%p", thisfn, ( void * ) a, ( void * ) b );

	for( it = na_object_get_items( b ) ; it ; it = it->next ){
		if( na_object_is_modified( it->data )){
			return( FALSE );
		}
	}

	are_equal = TRUE;

	/* chain call to parent class */
	if( NA_OBJECT_CLASS( st_parent_class )->are_equal ){
		are_equal &= NA_OBJECT_CLASS( st_parent_class )->are_equal( a, b );
	}

	return( are_equal );
}

static gboolean
object_is_valid( const NAObject *object )
{
	static const gchar *thisfn = "na_object_action_object_is_valid";
	gboolean is_valid;
	NAObjectAction *action;

	g_return_val_if_fail( NA_IS_OBJECT_ACTION( object ), FALSE );

	is_valid = FALSE;
	action = NA_OBJECT_ACTION( object );

	if( !action->private->dispose_has_run ){
		g_debug( "%s: action=%p (%s)", thisfn, ( void * ) action, G_OBJECT_TYPE_NAME( action ));

		is_valid = TRUE;

		if( na_object_is_target_toolbar( action )){
			is_valid &= is_valid_toolbar_label( action );
		}

		if( na_object_is_target_selection( action ) || na_object_is_target_location( action )){
			is_valid &= is_valid_label( action );
		}

		if( !is_valid ){
			na_object_debug_invalid( action, "no valid profile" );
		}
	}

	/* chain up to the parent class */
	if( NA_OBJECT_CLASS( st_parent_class )->is_valid ){
		is_valid &= NA_OBJECT_CLASS( st_parent_class )->is_valid( object );
	}

	return( is_valid );
}

static void
ifactory_object_iface_init( NAIFactoryObjectInterface *iface, void *user_data )
{
	static const gchar *thisfn = "na_object_action_ifactory_object_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->get_version = ifactory_object_get_version;
	iface->get_groups = ifactory_object_get_groups;
	iface->read_done = ifactory_object_read_done;
	iface->write_start = ifactory_object_write_start;
	iface->write_done = ifactory_object_write_done;
}

static guint
ifactory_object_get_version( const NAIFactoryObject *instance )
{
	return( 1 );
}

static NADataGroup *
ifactory_object_get_groups( const NAIFactoryObject *instance )
{
	return( action_data_groups );
}

/*
 * at this time, we don't yet have read the profiles as this will be
 * triggered by ifactory_provider_read_done - we so just are able to deal with
 * action-specific properties (not to check for profiles consistency)
 */
static void
ifactory_object_read_done( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages )
{
	guint iversion;
	NAObjectProfile *profile;

	g_debug( "na_object_action_ifactory_object_read_done: instance=%p", ( void * ) instance );

	na_object_item_deals_with_version( NA_OBJECT_ITEM( instance ));

	/* should attach a new profile if we detect a pre-v2 action
	 * the v1_to_v2 conversion must be followed by a v2_to_v3 one
	 */
	iversion = na_object_get_iversion( instance );
	if( iversion < 2 ){
		profile = read_done_convert_v1_to_v2( instance );
		na_object_profile_convert_v2_to_last( profile );
	}

	/* deals with obsoleted data, i.e. data which may have been written in the past
	 * but are no long written by now - not relevant for a menu
	 */
	read_done_deals_with_toolbar_label( instance );

	/* prepare the context after the reading
	 */
	na_icontext_read_done( NA_ICONTEXT( instance ));

	/* last, set action defaults
	 */
	na_factory_object_set_defaults( instance );
}

static guint
ifactory_object_write_start( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	na_object_item_rebuild_children_slist( NA_OBJECT_ITEM( instance ));

	return( NA_IIO_PROVIDER_CODE_OK );
}

static guint
ifactory_object_write_done( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	guint code;

	g_return_val_if_fail( NA_IS_OBJECT_ACTION( instance ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );

	code = write_done_write_profiles( instance, writer, writer_data, messages );

	return( code );
}

static void
icontext_iface_init( NAIContextInterface *iface, void *user_data )
{
	static const gchar *thisfn = "na_object_action_icontext_iface_init";

	g_debug( "%s: iface=%p, user_data=%p", thisfn, ( void * ) iface, ( void * ) user_data );

	iface->is_candidate = icontext_is_candidate;
}

static gboolean
icontext_is_candidate( NAIContext *object, guint target, GList *selection )
{
	return( TRUE );
}

/*
 * if we have a pre-v2 action
 *  any data found in data_def_action_v1 (defined in na-object-action-factory.c)
 *  is obsoleted and should be moved to a new profile
 *
 * actions read from .desktop already have iversion=3 (cf. desktop_read_start)
 * so v1 actions may only come from xml or mateconf
 *
 * returns the newly defined profile
 */
static NAObjectProfile *
read_done_convert_v1_to_v2( NAIFactoryObject *instance )
{
	static const gchar *thisfn = "na_object_action_read_done_read_done_convert_v1_to_last";
	GList *to_move;
	NADataDef *def;
	NADataBoxed *boxed;
	GList *ibox;
	NAObjectProfile *profile;

	/* search for old data in the body: this is only possible before profiles
	 * because starting with contexts, iversion was written
	 */
	to_move = NULL;
	def = data_def_action_v1;

	while( def->name ){
		boxed = na_ifactory_object_get_data_boxed( instance, def->name );
		if( boxed ){
			g_debug( "%s: boxed=%p (%s) marked to be moved from action body to profile",
							 thisfn, ( void * ) boxed, def->name );
			to_move = g_list_prepend( to_move, boxed );
		}
		def++;
	}

	/* now create a new profile
	 */
	profile = na_object_profile_new();
	na_object_set_id( profile, "profile-pre-v2" );
	na_object_set_label( profile, _( "Profile automatically created from pre-v2 action" ));
	na_object_attach_profile( instance, profile );

	for( ibox = to_move ; ibox ; ibox = ibox->next ){
		na_factory_object_move_boxed(
				NA_IFACTORY_OBJECT( profile ), instance, NA_DATA_BOXED( ibox->data ));
	}

	return( profile );
}

/*
 * if toolbar-same-label is true, then ensure that this is actually true
 */
static void
read_done_deals_with_toolbar_label( NAIFactoryObject *instance )
{
	gchar *toolbar_label;
	gchar *action_label;
	gboolean same_label;

	toolbar_label = na_object_get_toolbar_label( instance );
	action_label = na_object_get_label( instance );

	if( !toolbar_label || !g_utf8_strlen( toolbar_label, -1 )){
		na_object_set_toolbar_label( instance, action_label );
		na_object_set_toolbar_same_label( instance, TRUE );

	} else {
		same_label = ( na_core_utils_str_collate( action_label, toolbar_label ) == 0 );
		na_object_set_toolbar_same_label( instance, same_label );
	}

	g_free( action_label );
	g_free( toolbar_label );
}

/*
 * write the profiles of the action
 * note that subitems string list has been rebuilt on write_start
 */
static guint
write_done_write_profiles( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	static const gchar *thisfn = "na_object_action_write_done_write_profiles";
	guint code;
	GSList *children_slist, *ic;
	NAObjectProfile *profile;

	code = NA_IIO_PROVIDER_CODE_OK;
	children_slist = na_object_get_items_slist( instance );

	for( ic = children_slist ; ic && code == NA_IIO_PROVIDER_CODE_OK ; ic = ic->next ){
		profile = NA_OBJECT_PROFILE( na_object_get_item( instance, ic->data ));

		if( profile ){
			code = na_ifactory_provider_write_item( writer, writer_data, NA_IFACTORY_OBJECT( profile ), messages );

		} else {
			g_warning( "%s: profile not found: %s", thisfn, ( const gchar * ) ic->data );
		}
	}

	return( code );
}

static gboolean
is_valid_label( const NAObjectAction *action )
{
	gboolean is_valid;
	gchar *label;

	label = na_object_get_label( action );
	is_valid = ( label && g_utf8_strlen( label, -1 ) > 0 );
	g_free( label );

	if( !is_valid ){
		na_object_debug_invalid( action, "label" );
	}

	return( is_valid );
}

static gboolean
is_valid_toolbar_label( const NAObjectAction *action )
{
	gboolean is_valid;
	gchar *label;

	label = na_object_get_toolbar_label( action );
	is_valid = ( label && g_utf8_strlen( label, -1 ) > 0 );
	g_free( label );

	if( !is_valid ){
		na_object_debug_invalid( action, "toolbar-label" );
	}

	return( is_valid );
}

/**
 * na_object_action_new:
 *
 * Allocates a new #NAObjectAction object.
 *
 * The new #NAObjectAction object is initialized with suitable default values,
 * but without any profile.
 *
 * Returns: the newly allocated #NAObjectAction object.
 *
 * Since: 2.30
 */
NAObjectAction *
na_object_action_new( void )
{
	NAObjectAction *action;

	action = g_object_new( NA_TYPE_OBJECT_ACTION, NULL );

	return( action );
}

/**
 * na_object_action_new_with_profile:
 *
 * Allocates a new #NAObjectAction object along with a default profile.
 *
 * Returns: the newly allocated #NAObjectAction action.
 *
 * Since: 2.30
 */
NAObjectAction *
na_object_action_new_with_profile( void )
{
	NAObjectAction *action;
	NAObjectProfile *profile;

	action = na_object_action_new();
	profile = na_object_profile_new();
	na_object_attach_profile( action, profile );

	return( action );
}

/**
 * na_object_action_new_with_defaults:
 *
 * Allocates a new #NAObjectAction object along with a default profile.
 * These two objects have suitable default values.
 *
 * Returns: the newly allocated #NAObjectAction action.
 *
 * Since: 2.30
 */
NAObjectAction *
na_object_action_new_with_defaults( void )
{
	NAObjectAction *action;
	NAObjectProfile *profile;

	action = na_object_action_new();
	na_object_set_new_id( action, NULL );
	na_object_set_label( action, gettext( NEW_CAJA_ACTION ));
	na_object_set_toolbar_label( action, gettext( NEW_CAJA_ACTION ));
	na_factory_object_set_defaults( NA_IFACTORY_OBJECT( action ));

	profile = na_object_profile_new_with_defaults();
	na_object_attach_profile( action, profile );

	return( action );
}

/**
 * na_object_action_get_new_profile_name:
 * @action: the #NAObjectAction object which will receive a new profile.
 *
 * Returns a name suitable as a new profile name.
 *
 * The search is made by iterating over the standard profile name
 * prefix : basically, we increment a counter until finding a name
 * which is not yet allocated. The provided name is so only suitable
 * for the specified @action.
 *
 * When inserting a list of profiles in the action, we iter first for
 * new names, before actually do the insertion. We so keep the last
 * allocated name to avoid to allocate the same one twice.
 *
 * Returns: a newly allocated profile name, which should be g_free() by
 * the caller.
 *
 * Since: 2.30
 */
gchar *
na_object_action_get_new_profile_name( const NAObjectAction *action )
{
	int i;
	gboolean ok = FALSE;
	gchar *candidate = NULL;
	guint last_allocated;

	g_return_val_if_fail( NA_IS_OBJECT_ACTION( action ), NULL );

	if( !action->private->dispose_has_run ){

		last_allocated = na_object_get_last_allocated( action );

		for( i = last_allocated + 1 ; !ok ; ++i ){
			g_free( candidate );
			candidate = g_strdup_printf( "profile-%d", i );

			if( !na_object_get_item( action, candidate )){
				ok = TRUE;
				na_object_set_last_allocated( action, i );
			}
		}

		if( !ok ){
			g_free( candidate );
			candidate = NULL;
		}
	}

	/*g_debug( "returning candidate=%s", candidate );*/
	return( candidate );
}

/**
 * na_object_action_attach_profile:
 * @action: the #NAObjectAction action to which the profile will be attached.
 * @profile: the #NAObjectProfile profile to be attached to @action.
 *
 * Adds a profile at the end of the list of profiles.
 *
 * Since: 2.30
 */
void
na_object_action_attach_profile( NAObjectAction *action, NAObjectProfile *profile )
{
	g_return_if_fail( NA_IS_OBJECT_ACTION( action ));
	g_return_if_fail( NA_IS_OBJECT_PROFILE( profile ));

	if( !action->private->dispose_has_run ){

		na_object_append_item( action, profile );
		na_object_set_parent( profile, action );
	}
}

/**
 * na_object_action_set_last_version:
 * @action: the #NAObjectAction action to update.
 *
 * Set the version number of the @action to the last one.
 *
 * Since: 2.30
 */
void
na_object_action_set_last_version( NAObjectAction *action )
{
	g_return_if_fail( NA_IS_OBJECT_ACTION( action ));

	if( !action->private->dispose_has_run ){

		na_object_set_version( action, "2.0" );
	}
}
