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

#include <api/na-core-utils.h>
#include <api/na-mateconf-monitor.h>
#include <api/na-mateconf-utils.h>

#include "na-io-provider.h"
#include "na-iprefs.h"
#include "na-module.h"
#include "na-pivot.h"

/* private class data
 */
struct NAPivotClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAPivotPrivate {
	gboolean  dispose_has_run;

	guint     loadable_set;

	/* dynamically loaded modules (extension plugins)
	 */
	GList    *modules;

	/* list of instances to be notified of configuration updates
	 * these are called 'consumers' of NAPivot
	 */
	GList    *consumers;

	/* configuration tree of actions and menus
	 */
	GList    *tree;

	/* whether to automatically reload the whole configuration tree
	 * when a modification is detected in one of the underlying I/O
	 * storage subsystems
	 * defaults to FALSE
	 */
	gboolean  automatic_reload;
	GTimeVal  last_event;
	guint     event_source_id;

	/* list of monitoring objects on runtime preferences
	 */
	GList    *monitors;
};

/* NAPivot properties
 */
enum {
	NAPIVOT_PROP_TREE_ID = 1,
};


static GObjectClass *st_parent_class = NULL;
static gint          st_timeout_msec = 100;
static gint          st_timeout_usec = 100000;

static GType         register_type( void );
static void          class_init( NAPivotClass *klass );
static void          instance_init( GTypeInstance *instance, gpointer klass );
static void          instance_constructed( GObject *object );
static void          instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void          instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void          instance_dispose( GObject *object );
static void          instance_finalize( GObject *object );

static void          iprefs_iface_init( NAIPrefsInterface *iface );

static NAObjectItem *get_item_from_tree( const NAPivot *pivot, GList *tree, const gchar *id );

/* NAIPivotConsumer management */
static void          free_consumers( GList *list );

/* NAIIOProvider management */
static gboolean      on_item_changed_timeout( NAPivot *pivot );
static gulong        time_val_diff( const GTimeVal *recent, const GTimeVal *old );

/* NAMateConf runtime preferences management */
static void          monitor_runtime_preferences( NAPivot *pivot );
static void          on_mandatory_prefs_changed( MateConfClient *client, guint cnxn_id, MateConfEntry *entry, NAPivot *pivot );
static void          on_preferences_change( MateConfClient *client, guint cnxn_id, MateConfEntry *entry, NAPivot *pivot );
static void          display_order_changed( NAPivot *pivot );
static void          create_root_menu_changed( NAPivot *pivot );
static void          display_about_changed( NAPivot *pivot );

GType
na_pivot_get_type( void )
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
	static const gchar *thisfn = "na_pivot_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAPivotClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAPivot ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo iprefs_iface_info = {
		( GInterfaceInitFunc ) iprefs_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NAPivot", &info, 0 );

	g_type_add_interface_static( type, NA_IPREFS_TYPE, &iprefs_iface_info );

	return( type );
}

static void
class_init( NAPivotClass *klass )
{
	static const gchar *thisfn = "na_pivot_class_init";
	GObjectClass *object_class;
	GParamSpec *spec;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->constructed = instance_constructed;
	object_class->set_property = instance_set_property;
	object_class->get_property = instance_get_property;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	spec = g_param_spec_pointer(
			NAPIVOT_PROP_TREE,
			"Items tree",
			"Hierarchical tree of items",
			G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE );
	g_object_class_install_property( object_class, NAPIVOT_PROP_TREE_ID, spec );

	klass->private = g_new0( NAPivotClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_pivot_instance_init";
	NAPivot *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( NA_IS_PIVOT( instance ));
	self = NA_PIVOT( instance );

	self->private = g_new0( NAPivotPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->loadable_set = PIVOT_LOAD_NONE;
	self->private->modules = NULL;
	self->private->consumers = NULL;
	self->private->tree = NULL;
	self->private->automatic_reload = FALSE;
	self->private->event_source_id = 0;
	self->private->monitors = NULL;
}

static void
instance_constructed( GObject *object )
{
	static const gchar *thisfn = "na_pivot_instance_constructed";
	NAPivot *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_PIVOT( object ));
	self = NA_PIVOT( object );

	if( !self->private->dispose_has_run ){

		self->private->modules = na_module_load_modules();

		monitor_runtime_preferences( self );

		/* force class initialization and io-factory registration
		 */
		g_object_unref( na_object_action_new_with_profile());
		g_object_unref( na_object_menu_new());

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->constructed ){
			G_OBJECT_CLASS( st_parent_class )->constructed( object );
		}
	}
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	NAPivot *self;

	g_return_if_fail( NA_IS_PIVOT( object ));
	self = NA_PIVOT( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case NAPIVOT_PROP_TREE_ID:
				g_value_set_pointer( value, self->private->tree );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec )
{
	NAPivot *self;

	g_return_if_fail( NA_IS_PIVOT( object ));
	self = NA_PIVOT( object );

	if( !self->private->dispose_has_run ){

		switch( property_id ){
			case NAPIVOT_PROP_TREE_ID:
				self->private->tree = g_value_get_pointer( value );
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID( object, property_id, spec );
				break;
		}
	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_pivot_instance_dispose";
	NAPivot *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NA_IS_PIVOT( object ));
	self = NA_PIVOT( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* release modules */
		na_module_release_modules( self->private->modules );
		self->private->modules = NULL;

		/* release list of NAIPivotConsumers */
		free_consumers( self->private->consumers );
		self->private->consumers = NULL;

		/* release item tree */
		g_debug( "%s: tree=%p, count=%u", thisfn, ( void * ) self->private->tree, g_list_length( self->private->tree ));
		na_object_unref_items( self->private->tree );
		self->private->tree = NULL;

		/* release the MateConf monitoring */
		na_mateconf_monitor_release_monitors( self->private->monitors );

		/* release the I/O Provider objects */
		na_io_provider_terminate();

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_pivot_instance_finalize";
	NAPivot *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NA_IS_PIVOT( object ));
	self = NA_PIVOT( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
iprefs_iface_init( NAIPrefsInterface *iface )
{
	static const gchar *thisfn = "na_pivot_iprefs_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );
}

/**
 * na_pivot_new:
 *
 * Returns: a newly allocated #NAPivot object.
 */
NAPivot *
na_pivot_new( void )
{
	static const gchar *thisfn = "na_pivot_new";
	NAPivot *pivot;

	g_debug( "%s", thisfn );

	pivot = g_object_new( NA_PIVOT_TYPE, NULL );

	return( pivot );
}

/**
 * na_pivot_dump:
 * @pivot: the #NAPivot object do be dumped.
 *
 * Dumps the content of a #NAPivot object.
 */
void
na_pivot_dump( const NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_dump";
	GList *it;
	int i;

	if( !pivot->private->dispose_has_run ){

		g_debug( "%s:     loadable_set=%d", thisfn, pivot->private->loadable_set );
		g_debug( "%s:          modules=%p (%d elts)", thisfn, ( void * ) pivot->private->modules, g_list_length( pivot->private->modules ));
		g_debug( "%s:        consumers=%p (%d elts)", thisfn, ( void * ) pivot->private->consumers, g_list_length( pivot->private->consumers ));
		g_debug( "%s:             tree=%p (%d elts)", thisfn, ( void * ) pivot->private->tree, g_list_length( pivot->private->tree ));
		g_debug( "%s: automatic_reload=%s", thisfn, pivot->private->automatic_reload ? "True":"False" );
		g_debug( "%s:         monitors=%p (%d elts)", thisfn, ( void * ) pivot->private->monitors, g_list_length( pivot->private->monitors ));

		for( it = pivot->private->tree, i = 0 ; it ; it = it->next ){
			g_debug( "%s:     [%d]: %p", thisfn, i++, it->data );
		}
	}
}

/**
 * na_pivot_get_providers:
 * @pivot: this #NAPivot instance.
 * @type: the type of searched interface.
 * For now, we only have NA_IIO_PROVIDER_TYPE interfaces.
 *
 * Returns: a newly allocated list of providers of the required interface.
 *
 * This function is called by interfaces API in order to find the
 * list of providers registered for their own given interface.
 *
 * The returned list should be release by calling na_pivot_free_providers().
 */
GList *
na_pivot_get_providers( const NAPivot *pivot, GType type )
{
	static const gchar *thisfn = "na_pivot_get_providers";
	GList *list = NULL;

	g_debug( "%s: pivot=%p, type=%lu (%s)", thisfn, ( void * ) pivot, ( unsigned long ) type, g_type_name( type ));
	g_return_val_if_fail( NA_IS_PIVOT( pivot ), NULL );

	if( !pivot->private->dispose_has_run ){

		list = na_module_get_extensions_for_type( pivot->private->modules, type );
		g_debug( "%s: list=%p, count=%d", thisfn, ( void * ) list, list ? g_list_length( list ) : 0 );
	}

	return( list );
}

/**
 * na_pivot_free_providers:
 * @providers: a list of providers.
 *
 * Frees a list of providers as returned from na_pivot_get_providers().
 */
void
na_pivot_free_providers( GList *providers )
{
	static const gchar *thisfn = "na_pivot_free_providers";

	g_debug( "%s: providers=%p", thisfn, ( void * ) providers );

	na_module_free_extensions_list( providers );
}

/**
 * na_pivot_get_item:
 * @pivot: this #NAPivot instance.
 * @id: the required item identifier.
 *
 * Returns the specified item, action or menu.
 *
 * Returns: the required #NAObjectItem-derived object, or %NULL if not
 * found.
 *
 * The returned pointer is owned by #NAPivot, and should not be
 * g_free() nor g_object_unref() by the caller.
 */
NAObjectItem *
na_pivot_get_item( const NAPivot *pivot, const gchar *id )
{
	NAObjectItem *object = NULL;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), NULL );

	if( !pivot->private->dispose_has_run ){

		if( !id || !strlen( id )){
			return( NULL );
		}

		object = get_item_from_tree( pivot, pivot->private->tree, id );
	}

	return( object );
}

static NAObjectItem *
get_item_from_tree( const NAPivot *pivot, GList *tree, const gchar *id )
{
	GList *subitems, *ia;
	NAObjectItem *found = NULL;

	for( ia = tree ; ia && !found ; ia = ia->next ){

		gchar *i_id = na_object_get_id( NA_OBJECT( ia->data ));

		if( !g_ascii_strcasecmp( id, i_id )){
			found = NA_OBJECT_ITEM( ia->data );
		}

		if( !found && NA_IS_OBJECT_ITEM( ia->data )){
			subitems = na_object_get_items( ia->data );
			found = get_item_from_tree( pivot, subitems, id );
		}
	}

	return( found );
}

/**
 * na_pivot_get_items:
 * @pivot: this #NAPivot instance.
 *
 * Returns: the current configuration tree.
 *
 * The returned list is owned by this #NAPivot object, and should not
 * be g_free(), nor g_object_unref() by the caller.
 */
GList *
na_pivot_get_items( const NAPivot *pivot )
{
	GList *tree;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), NULL );

	tree = NULL;

	if( !pivot->private->dispose_has_run ){

		tree = pivot->private->tree;
	}

	return( tree );
}

/**
 * na_pivot_load_items:
 * @pivot: this #NAPivot instance.
 *
 * Loads the hierarchical list of items from I/O providers.
 */
void
na_pivot_load_items( NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_load_items";
	GSList *messages, *im;

	g_debug( "%s: pivot=%p", thisfn, ( void * ) pivot );
	g_return_if_fail( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		na_object_unref_items( pivot->private->tree );

		messages = NULL;

		pivot->private->tree = na_io_provider_read_items( pivot, &messages );

		for( im = messages ; im ; im = im->next ){
			g_warning( "%s: %s", thisfn, ( const gchar * ) im->data );
		}

		na_core_utils_slist_free( messages );
	}
}

/**
 * na_pivot_item_changed_handler:
 * @provider: the #NAIIOProvider which has emitted the signal.
 * @id: the id of the changed #NAObjectItem-derived object.
 * @pivot: this #NAPivot instance.
 *
 * This handler is trigerred by IIOProviders when an action is changed
 * in their underlying storage subsystems.
 * We don't care of updating our internal list with each and every
 * atomic modification; instead we wait for the end of notifications
 * serie, and then reload the whole list of actions
 */
void
na_pivot_item_changed_handler( NAIIOProvider *provider, const gchar *id, NAPivot *pivot  )
{
	static const gchar *thisfn = "na_pivot_item_changed_handler";

	g_debug( "%s: provider=%p, id=%s, pivot=%p", thisfn, ( void * ) provider, id, ( void * ) pivot );

	g_return_if_fail( NA_IS_IIO_PROVIDER( provider ));
	g_return_if_fail( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		/* set a timeout to notify clients at the end of the serie */
		g_get_current_time( &pivot->private->last_event );

		if( !pivot->private->event_source_id ){
			pivot->private->event_source_id =
				g_timeout_add( st_timeout_msec, ( GSourceFunc ) on_item_changed_timeout, pivot );
		}
	}
}

/*
 * this timer is set when we receive the first event of a serie
 * we continue to loop until last event is at least one half of a
 * second old
 *
 * there is no race condition here as we are not multithreaded
 * or .. is there ?
 */
static gboolean
on_item_changed_timeout( NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_on_item_changed_timeout";
	GTimeVal now;
	gulong diff;
	GList *ic;

	g_debug( "%s: pivot=%p", thisfn, pivot );
	g_return_val_if_fail( NA_IS_PIVOT( pivot ), FALSE );

	g_get_current_time( &now );
	diff = time_val_diff( &now, &pivot->private->last_event );
	if( diff < st_timeout_usec ){
		return( TRUE );
	}

	if( pivot->private->automatic_reload ){
		na_pivot_load_items( pivot );
	}

	for( ic = pivot->private->consumers ; ic ; ic = ic->next ){
		na_ipivot_consumer_notify_of_items_changed( NA_IPIVOT_CONSUMER( ic->data ));
	}

	pivot->private->event_source_id = 0;
	return( FALSE );
}

/*
 * returns the difference in microseconds.
 */
static gulong
time_val_diff( const GTimeVal *recent, const GTimeVal *old )
{
	gulong microsec = 1000000 * ( recent->tv_sec - old->tv_sec );
	microsec += recent->tv_usec  - old->tv_usec;
	return( microsec );
}

/**
 * na_pivot_write_level_zero:
 * @pivot: this #NAPivot instance.
 * @items: the #GList of items whose first level is to be written.
 *
 * Rewrite the level-zero items in MateConf preferences.
 *
 * Returns: %TRUE if successfully written (i.e. writable, not locked,
 * and so on), %FALSE else.
 */
gboolean
na_pivot_write_level_zero( const NAPivot *pivot, GList *items )
{
	static const gchar *thisfn = "na_pivot_write_level_zero";
	gboolean written;
	GList *it;
	gchar *id;
	GSList *content;

	g_debug( "%s: pivot=%p", thisfn, ( void * ) pivot);

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), FALSE );

	written = FALSE;

	if( !pivot->private->dispose_has_run &&
		na_pivot_is_level_zero_writable( pivot )){

			content = NULL;
			for( it = items ; it ; it = it->next ){

				id = na_object_get_id( it->data );
				content = g_slist_prepend( content, id );
			}
			content = g_slist_reverse( content );

			na_iprefs_write_string_list( NA_IPREFS( pivot ), IPREFS_LEVEL_ZERO_ITEMS, content );

			na_core_utils_slist_free( content );

			written = TRUE;
	}

	return( written );
}

/**
 * na_pivot_register_consumer:
 * @pivot: this #NAPivot instance.
 * @consumer: a #NAIPivotConsumer which wishes be notified of any
 * modification of an action or a menu in any of the underlying I/O
 * storage subsystems.
 *
 * Registers a new consumer to be notified of configuration modification.
 */
void
na_pivot_register_consumer( NAPivot *pivot, const NAIPivotConsumer *consumer )
{
	static const gchar *thisfn = "na_pivot_register_consumer";

	g_debug( "%s: pivot=%p, consumer=%p", thisfn, ( void * ) pivot, ( void * ) consumer );
	g_return_if_fail( NA_IS_PIVOT( pivot ));
	g_return_if_fail( NA_IS_IPIVOT_CONSUMER( consumer ));

	if( !pivot->private->dispose_has_run ){

		pivot->private->consumers = g_list_prepend( pivot->private->consumers, ( gpointer ) consumer );
	}
}

static void
free_consumers( GList *consumers )
{
	/*g_list_foreach( consumers, ( GFunc ) g_object_unref, NULL );*/
	g_list_free( consumers );
}

/**
 * na_pivot_set_automatic_reload:
 * @pivot: this #NAPivot instance.
 * @reload: whether this #NAPivot instance should automatically reload
 * its list of actions when I/O providers advertize it of a
 * modification.
 *
 * Sets the automatic reload flag.
 *
 * Note that even if the #NAPivot instance is not authorized to
 * automatically reload its list of actions when it is advertized of
 * a modification by one of the I/O providers, it always sends an
 * ad-hoc notification to its consumers.
 */
void
na_pivot_set_automatic_reload( NAPivot *pivot, gboolean reload )
{
	g_return_if_fail( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		pivot->private->automatic_reload = reload;
	}
}

/**
 * na_pivot_is_disable_loadable:
 * @pivot: this #NAPivot instance.
 *
 * Returns: %TRUE if disabled items should be loaded, %FALSE else.
 */
gboolean
na_pivot_is_disable_loadable( const NAPivot *pivot )
{
	gboolean is_loadable;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), FALSE );

	is_loadable = FALSE;

	if( !pivot->private->dispose_has_run ){

		is_loadable = ( pivot->private->loadable_set & PIVOT_LOAD_DISABLED );
	}

	return( is_loadable );
}

/**
 * na_pivot_is_invalid_loadable:
 * @pivot: this #NAPivot instance.
 *
 * Returns: %TRUE if invalid items should be loaded, %FALSE else.
 */
gboolean
na_pivot_is_invalid_loadable( const NAPivot *pivot )
{
	gboolean is_loadable;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), FALSE );

	is_loadable = FALSE;

	if( !pivot->private->dispose_has_run ){

		is_loadable = ( pivot->private->loadable_set & PIVOT_LOAD_INVALID );
	}

	return( is_loadable );
}

/**
 * na_pivot_set_loadable:
 * @pivot: this #NAPivot instance.
 * @loadable: the population of items to be loaded.
 *
 * Sets the loadable set.
 */
void
na_pivot_set_loadable( NAPivot *pivot, guint loadable )
{
	g_return_if_fail( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		pivot->private->loadable_set = loadable;
	}
}

/**
 * na_pivot_is_level_zero_writable:
 * @pivot: this #NAPivot instance.
 *
 * Returns: %TRUE if we are able to update the level-zero list of items,
 * %FALSE else.
 */
gboolean
na_pivot_is_level_zero_writable( const NAPivot *pivot )
{
	gboolean all_locked;
	gboolean mateconf_locked;
	MateConfClient *mateconf;
	gchar *path;

	all_locked = FALSE;
	mateconf_locked = FALSE;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), FALSE );

	if( !pivot->private->dispose_has_run ){

		all_locked = na_pivot_is_configuration_locked_by_admin( pivot );

		mateconf = na_iprefs_get_mateconf_client( NA_IPREFS( pivot ));

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_BASEDIR, "mandatory/io-mateconf/locked" );
		mateconf_locked = na_mateconf_utils_read_bool( mateconf, path, FALSE, FALSE );
		g_free( path );
	}

	return( !all_locked && !mateconf_locked );
}

/**
 * na_pivot_is_configuration_locked_by_admin:
 * @pivot: this #NAPivot.
 *
 * Returns: %TRUE if the whole configuration has been locked by an
 * administrator, %FALSE else.
 */
gboolean
na_pivot_is_configuration_locked_by_admin( const NAPivot *pivot )
{
	gboolean locked;
	MateConfClient *mateconf;
	gchar *path;

	locked = FALSE;
	g_return_val_if_fail( NA_IS_PIVOT( pivot ), FALSE );

	if( !pivot->private->dispose_has_run ){

		mateconf = na_iprefs_get_mateconf_client( NA_IPREFS( pivot ));

		path = mateconf_concat_dir_and_key( IPREFS_MATECONF_BASEDIR, "mandatory/all/locked" );
		locked = na_mateconf_utils_read_bool( mateconf, path, FALSE, FALSE );
		g_free( path );
	}

	return( locked );
}

static void
monitor_runtime_preferences( NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_monitor_runtime_preferences";
	GList *list = NULL;
	gchar *path;

	g_debug( "%s: pivot=%p", thisfn, ( void * ) pivot );
	g_return_if_fail( NA_IS_PIVOT( pivot ));
	g_return_if_fail( !pivot->private->dispose_has_run );

	list = g_list_prepend( list,
			na_mateconf_monitor_new(
					IPREFS_MATECONF_PREFS_PATH,
					( MateConfClientNotifyFunc ) on_preferences_change,
					pivot ));

	path = mateconf_concat_dir_and_key( IPREFS_MATECONF_BASEDIR, "mandatory" );
	list = g_list_prepend( list,
			na_mateconf_monitor_new(
					path,
					( MateConfClientNotifyFunc ) on_mandatory_prefs_changed,
					pivot ));
	g_free( path );

	pivot->private->monitors = list;
}

static void
on_mandatory_prefs_changed( MateConfClient *client, guint cnxn_id, MateConfEntry *entry, NAPivot *pivot )
{
	const gchar *key;
	gchar *key_entry;
	GList *ic;

	g_return_if_fail( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		key = mateconf_entry_get_key( entry );
		key_entry = g_path_get_basename( key );

		if( !strcmp( key_entry, "locked" )){
			for( ic = pivot->private->consumers ; ic ; ic = ic->next ){
				na_ipivot_consumer_notify_of_mandatory_prefs_changed( NA_IPIVOT_CONSUMER( ic->data ));
			}
		}

		g_free( key_entry );
	}
}

static void
on_preferences_change( MateConfClient *client, guint cnxn_id, MateConfEntry *entry, NAPivot *pivot )
{
	/*static const gchar *thisfn = "na_pivot_on_preferences_change";*/
	const gchar *key;
	gchar *key_entry;

	g_return_if_fail( NA_IS_PIVOT( pivot ));

	key = mateconf_entry_get_key( entry );
	key_entry = g_path_get_basename( key );
	/*g_debug( "%s: key=%s", thisfn, key_entry );*/

	if( !g_ascii_strcasecmp( key_entry, IPREFS_CREATE_ROOT_MENU )){
		create_root_menu_changed( pivot );
	}

	if( !g_ascii_strcasecmp( key_entry, IPREFS_ADD_ABOUT_ITEM )){
		display_about_changed( pivot );
	}

	if( !g_ascii_strcasecmp( key_entry, IPREFS_DISPLAY_ALPHABETICAL_ORDER )){
		display_order_changed( pivot );
	}

	g_free( key_entry );
}

static void
display_order_changed( NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_display_order_changed";
	GList *ic;
	gint order_mode;

	g_debug( "%s: pivot=%p", thisfn, ( void * ) pivot );
	g_assert( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		order_mode = na_iprefs_get_order_mode( NA_IPREFS( pivot ));

		for( ic = pivot->private->consumers ; ic ; ic = ic->next ){
			na_ipivot_consumer_notify_of_display_order_changed( NA_IPIVOT_CONSUMER( ic->data ), order_mode );
		}
	}
}

static void
create_root_menu_changed( NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_create_root_menu_changed";
	GList *ic;
	gboolean should_create;

	g_debug( "%s: pivot=%p", thisfn, ( void * ) pivot );
	g_assert( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		should_create = na_iprefs_read_bool( NA_IPREFS( pivot ), IPREFS_CREATE_ROOT_MENU, FALSE );
		for( ic = pivot->private->consumers ; ic ; ic = ic->next ){
			na_ipivot_consumer_notify_of_create_root_menu_changed( NA_IPIVOT_CONSUMER( ic->data ), should_create );
		}
	}
}

static void
display_about_changed( NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_display_about_changed";
	GList *ic;
	gboolean display_about;

	g_debug( "%s: pivot=%p", thisfn, ( void * ) pivot );
	g_assert( NA_IS_PIVOT( pivot ));

	if( !pivot->private->dispose_has_run ){

		display_about = na_iprefs_read_bool( NA_IPREFS( pivot ), IPREFS_ADD_ABOUT_ITEM, TRUE );

		for( ic = pivot->private->consumers ; ic ; ic = ic->next ){
			na_ipivot_consumer_notify_of_display_about_changed( NA_IPIVOT_CONSUMER( ic->data ), display_about );
		}
	}
}
