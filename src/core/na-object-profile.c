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
#include <string.h>

#include <libcaja-extension/caja-file-info.h>

#include <api/na-core-utils.h>
#include <api/na-iio-provider.h>
#include <api/na-ifactory-object.h>
#include <api/na-object-api.h>

#include "na-factory-provider.h"
#include "na-factory-object.h"
#include "na-selected-info.h"
#include "na-mate-vfs-uri.h"

/* private class data
 */
struct NAObjectProfileClassPrivate {
	void *empty;							/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAObjectProfilePrivate {
	gboolean dispose_has_run;
};

#define PROFILE_NAME_PREFIX					"profile-"

extern NADataGroup profile_data_groups [];	/* defined in na-item-profile-factory.c */

static NAObjectIdClass *st_parent_class = NULL;

static GType        register_type( void );
static void         class_init( NAObjectProfileClass *klass );
static void         instance_init( GTypeInstance *instance, gpointer klass );
static void         instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec );
static void         instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec );
static void         instance_dispose( GObject *object );
static void         instance_finalize( GObject *object );

static void         object_copy( NAObject *target, const NAObject *source, gboolean recursive );
static gboolean     object_is_valid( const NAObject *object );

static void         ifactory_object_iface_init( NAIFactoryObjectInterface *iface );
static guint        ifactory_object_get_version( const NAIFactoryObject *instance );
static NADataGroup *ifactory_object_get_groups( const NAIFactoryObject *instance );
static gboolean     ifactory_object_is_valid( const NAIFactoryObject *object );
static void         ifactory_object_read_done( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages );
static guint        ifactory_object_write_done( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages );

static void         icontext_conditions_iface_init( NAIContextInterface *iface );

static gboolean     profile_is_valid( const NAObjectProfile *profile );
static gboolean     is_valid_path_parameters( const NAObjectProfile *profile );

static gchar       *object_id_new_id( const NAObjectId *item, const NAObjectId *new_parent );

GType
na_object_profile_get_type( void )
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
	static const gchar *thisfn = "na_object_profile_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAObjectProfileClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAObjectProfile ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo icontext_conditions_iface_info = {
		( GInterfaceInitFunc ) icontext_conditions_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ifactory_object_iface_info = {
		( GInterfaceInitFunc ) ifactory_object_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( NA_OBJECT_ID_TYPE, "NAObjectProfile", &info, 0 );

	g_type_add_interface_static( type, NA_ICONTEXT_TYPE, &icontext_conditions_iface_info );

	g_type_add_interface_static( type, NA_IFACTORY_OBJECT_TYPE, &ifactory_object_iface_info );

	return( type );
}

static void
class_init( NAObjectProfileClass *klass )
{
	static const gchar *thisfn = "na_object_profile_class_init";
	GObjectClass *object_class;
	NAObjectClass *naobject_class;
	NAObjectIdClass *naobjectid_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->set_property = instance_set_property;
	object_class->get_property = instance_get_property;
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	naobject_class = NA_OBJECT_CLASS( klass );
	naobject_class->dump = NULL;
	naobject_class->copy = object_copy;
	naobject_class->are_equal = NULL;
	naobject_class->is_valid = object_is_valid;

	naobjectid_class = NA_OBJECT_ID_CLASS( klass );
	naobjectid_class->new_id = object_id_new_id;

	klass->private = g_new0( NAObjectProfileClassPrivate, 1 );

	na_factory_object_define_properties( object_class, profile_data_groups );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_object_profile_instance_init";
	NAObjectProfile *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	g_return_if_fail( NA_IS_OBJECT_PROFILE( instance ));

	self = NA_OBJECT_PROFILE( instance );

	self->private = g_new0( NAObjectProfilePrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_get_property( GObject *object, guint property_id, GValue *value, GParamSpec *spec )
{
	g_return_if_fail( NA_IS_OBJECT_PROFILE( object ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NA_OBJECT_PROFILE( object )->private->dispose_has_run ){

		na_factory_object_get_as_value( NA_IFACTORY_OBJECT( object ), g_quark_to_string( property_id ), value );
	}
}

static void
instance_set_property( GObject *object, guint property_id, const GValue *value, GParamSpec *spec )
{
	g_return_if_fail( NA_IS_OBJECT_PROFILE( object ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	if( !NA_OBJECT_PROFILE( object )->private->dispose_has_run ){

		na_factory_object_set_from_value( NA_IFACTORY_OBJECT( object ), g_quark_to_string( property_id ), value );
	}
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_object_profile_instance_dispose";
	NAObjectProfile *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_if_fail( NA_IS_OBJECT_PROFILE( object ));

	self = NA_OBJECT_PROFILE( object );

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
	static const gchar *thisfn = "na_object_profile_instance_finalize";
	NAObjectProfile *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_if_fail( NA_IS_OBJECT_PROFILE( object ));

	self = NA_OBJECT_PROFILE( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
object_copy( NAObject *target, const NAObject *source, gboolean recursive )
{
	g_return_if_fail( NA_IS_OBJECT_PROFILE( target ));
	g_return_if_fail( NA_IS_OBJECT_PROFILE( source ));

	if( !NA_OBJECT_PROFILE( target )->private->dispose_has_run &&
		!NA_OBJECT_PROFILE( source )->private->dispose_has_run ){

		na_factory_object_copy( NA_IFACTORY_OBJECT( target ), NA_IFACTORY_OBJECT( source ));
	}
}

static gboolean
object_is_valid( const NAObject *object )
{
	g_return_val_if_fail( NA_IS_OBJECT_PROFILE( object ), FALSE );

	return( profile_is_valid( NA_OBJECT_PROFILE( object )));
}

static void
ifactory_object_iface_init( NAIFactoryObjectInterface *iface )
{
	static const gchar *thisfn = "na_object_profile_ifactory_object_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = ifactory_object_get_version;
	iface->get_groups = ifactory_object_get_groups;
	iface->copy = NULL;
	iface->are_equal = NULL;
	iface->is_valid = ifactory_object_is_valid;
	iface->read_start = NULL;
	iface->read_done = ifactory_object_read_done;
	iface->write_start = NULL;
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
	return( profile_data_groups );
}

static gboolean
ifactory_object_is_valid( const NAIFactoryObject *object )
{
	static const gchar *thisfn = "na_object_profile_ifactory_object_is_valid: object";

	g_debug( "%s: object=%p (%s)",
			thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	g_return_val_if_fail( NA_IS_OBJECT_PROFILE( object ), FALSE );

	return( profile_is_valid( NA_OBJECT_PROFILE( object )));
}

static void
ifactory_object_read_done( NAIFactoryObject *instance, const NAIFactoryProvider *reader, void *reader_data, GSList **messages )
{
	na_factory_object_set_defaults( instance );
}

static guint
ifactory_object_write_done( NAIFactoryObject *instance, const NAIFactoryProvider *writer, void *writer_data, GSList **messages )
{
	return( NA_IIO_PROVIDER_CODE_OK );
}

static void
icontext_conditions_iface_init( NAIContextInterface *iface )
{
	static const gchar *thisfn = "na_object_profile_icontext_conditions_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );
}

static gboolean
profile_is_valid( const NAObjectProfile *profile )
{
	gboolean is_valid;

	is_valid = FALSE;

	if( !profile->private->dispose_has_run ){

		is_valid = \
				is_valid_path_parameters( profile ) &&
				na_icontext_is_valid( NA_ICONTEXT( profile ));
	}

	return( is_valid );
}

/*
 * historical behavior was to not check path nor parameters at all
 * 2.29.x serie, and up to 2.30.0, have tried to check an actual executable path
 * but most of already actions only used a command, relying on the PATH env variable
 * so, starting with 2.30.1, we only check for non empty path+parameters
 */
static gboolean
is_valid_path_parameters( const NAObjectProfile *profile )
{
	gboolean valid;
	gchar *path, *parameters;
	gchar *command;

	path = na_object_get_path( profile );
	parameters = na_object_get_parameters( profile );

	command = g_strdup_printf( "%s %s", path, parameters );
	g_strstrip( command );

	valid = g_utf8_strlen( command, -1 ) > 0;

	g_free( command );
	g_free( parameters );
	g_free( path );

	if( !valid ){
		na_object_debug_invalid( profile, "command" );
	}

	return( valid );
}

/*
 * new_parent is specifically set to be able to allocate a new id for
 * the current profile into the target parent
 */
static gchar *
object_id_new_id( const NAObjectId *item, const NAObjectId *new_parent )
{
	gchar *id = NULL;

	g_return_val_if_fail( NA_IS_OBJECT_PROFILE( item ), NULL );
	g_return_val_if_fail( new_parent && NA_IS_OBJECT_ACTION( new_parent ), NULL );

	if( !NA_OBJECT_PROFILE( item )->private->dispose_has_run ){

		id = na_object_action_get_new_profile_name( NA_OBJECT_ACTION( new_parent ));
	}

	return( id );
}

/**
 * na_object_profile_new:
 *
 * Allocates a new profile.
 *
 * Returns: the newly allocated #NAObjectProfile profile.
 */
NAObjectProfile *
na_object_profile_new( void )
{
	NAObjectProfile *profile;

	profile = g_object_new( NA_OBJECT_PROFILE_TYPE, NULL );

	return( profile );
}

/**
 * na_object_profile_new_with_defaults:
 *
 * Allocates a new profile, and set default values.
 *
 * Returns: the newly allocated #NAObjectProfile profile.
 */
NAObjectProfile *
na_object_profile_new_with_defaults( void )
{
	NAObjectProfile *profile = na_object_profile_new();
	na_object_set_id( profile, "profile-zero" );
	/* i18n: label for the default profile */
	na_object_set_label( profile, _( "Default profile" ));
	na_factory_object_set_defaults( NA_IFACTORY_OBJECT( profile ));

	return( profile );
}

/**
 * Expands the parameters path, in function of the found tokens.
 *
 * @profile: the selected profile.
 * @target: the current target.
 * @files: the list of currently selected #NASelectedInfo items.
 *
 * Valid parameters are :
 *
 * %d : base dir of the (first) selected file(s)/folder(s)
 * %f : the name of the (first) selected file/folder
 * %h : hostname of the (first) URI
 * %m : list of the basename of the selected files/directories separated by space.
 * %M : list of the selected files/directories with their complete path separated by space.
 * %p : port number from the (first) URI
 * %R : space-separated list of URIs
 * %s : scheme of the (first) URI
 * %u : (first) URI
 * %U : username of the (first) URI
 * %% : a percent sign
 *
 * Adding a parameter requires updating of :
 * - caja-actions/core/na-object-profile.c::na_object_profile_parse_parameters()
 * - caja-actions/nact/nact-icommand-tab.c:parse_parameters()
 * - caja-actions/nact/caja-actions-config-tool.ui:LegendDialog
 */
gchar *
na_object_profile_parse_parameters( const NAObjectProfile *profile, gint target, GList* files )
{
	gchar *parsed = NULL;
	GString *string;
	GList *ifi;
	gboolean first;
	gchar *iuri, *ipath, *ibname;
	GFile *iloc;
	gchar *uri = NULL;
	gchar *scheme = NULL;
	gchar *dirname = NULL;
	gchar *filename = NULL;
	gchar *hostname = NULL;
	gchar *username = NULL;
	gint port_number = 0;
	GString *basename_list, *pathname_list, *uris_list;
	gchar *tmp, *iter, *old_iter;
	NAMateVFSURI *vfs;

	g_return_val_if_fail( NA_IS_OBJECT_PROFILE( profile ), NULL );

	if( profile->private->dispose_has_run ){
		return( NULL );
	}

	string = g_string_new( "" );
	basename_list = g_string_new( "" );
	pathname_list = g_string_new( "" );
	uris_list = g_string_new( "" );
	first = TRUE;

	for( ifi = files ; ifi ; ifi = ifi->next ){

		iuri = na_selected_info_get_uri( NA_SELECTED_INFO( ifi->data ));
		iloc = na_selected_info_get_location( NA_SELECTED_INFO( ifi->data ));
		ipath = g_file_get_path( iloc );
		ibname = g_file_get_basename( iloc );

		if( first ){

			vfs = g_new0( NAMateVFSURI, 1 );
			na_mate_vfs_uri_parse( vfs, iuri );

			uri = g_strdup( iuri );
			dirname = ipath ? g_path_get_dirname( ipath ) : NULL;
			scheme = g_strdup( vfs->scheme );
			filename = g_strdup( ibname );
			hostname = g_strdup( vfs->host_name );
			username = g_strdup( vfs->user_name );
			port_number = vfs->host_port;

			first = FALSE;
			na_mate_vfs_uri_free( vfs );
		}

		if( ibname ){
			if( strlen( basename_list->str )){
				basename_list = g_string_append( basename_list, " " );
			}
			tmp = g_shell_quote( ibname );
			g_string_append_printf( basename_list, "%s", tmp );
			g_free( tmp );
		}

		if( ipath ){
			if( strlen( pathname_list->str )){
				pathname_list = g_string_append( pathname_list, " " );
			}
			tmp = g_shell_quote( ipath );
			g_string_append_printf( pathname_list, "%s", tmp );
			g_free( tmp );
		}

		if( strlen( uris_list->str )){
			uris_list = g_string_append( uris_list, " " );
		}
		tmp = g_shell_quote( iuri );
		g_string_append_printf( uris_list, "%s", tmp );
		g_free( tmp );

		g_free( ibname );
		g_free( ipath );
		g_object_unref( iloc );
		g_free( iuri );
	}

	iter = na_object_get_parameters( profile );
	old_iter = iter;

	while(( iter = g_strstr_len( iter, strlen( iter ), "%" ))){

		string = g_string_append_len( string, old_iter, strlen( old_iter ) - strlen( iter ));
		switch( iter[1] ){

			/* base dir of the (first) selected item
			 */
			case 'd':
				if( dirname ){
					tmp = g_shell_quote( dirname );
					string = g_string_append( string, tmp );
					g_free( tmp );
				}
				break;

			/* basename of the (first) selected item
			 */
			case 'f':
				if( filename ){
					tmp = g_shell_quote( filename );
					string = g_string_append( string, tmp );
					g_free( tmp );
				}
				break;

			/* hostname of the (first) URI
			 */
			case 'h':
				if( hostname ){
					string = g_string_append( string, hostname );
				}
				break;

			/* space-separated list of the basenames
			 */
			case 'm':
				if( basename_list->str ){
					string = g_string_append( string, basename_list->str );
				}
				break;

			/* space-separated list of full pathnames
			 */
			case 'M':
				if( pathname_list->str ){
					string = g_string_append( string, pathname_list->str );
				}
				break;

			/* port number of the (first) URI
			 */
			case 'p':
				if( port_number > 0 ){
					g_string_append_printf( string, "%d", port_number );
				}
				break;

			/* list of URIs
			 */
			case 'R':
				if( uris_list->str ){
					string = g_string_append( string, uris_list->str );
				}
				break;

			/* scheme of the (first) URI
			 */
			case 's':
				if( scheme ){
					string = g_string_append( string, scheme );
				}
				break;

			/* URI of the first item
			 */
			case 'u':
				if( uri ){
					string = g_string_append( string, uri );
				}
				break;

			/* username of the (first) URI
			 */
			case 'U':
				if( username ){
					string = g_string_append( string, username );
				}
				break;

			/* a percent sign
			 */
			case '%':
				string = g_string_append_c( string, '%' );
				break;
		}

		iter += 2;			/* skip the % sign and the character after */
		old_iter = iter;	/* store the new start of the string */
	}

	string = g_string_append_len( string, old_iter, strlen( old_iter ));

	g_free( uri );
	g_free( dirname );
	g_free( scheme );
	g_free( hostname );
	g_free( username );
	g_free( iter );
	g_string_free( uris_list, TRUE );
	g_string_free( basename_list, TRUE );
	g_string_free( pathname_list, TRUE );

	parsed = g_string_free( string, FALSE );
	return( parsed );
}
