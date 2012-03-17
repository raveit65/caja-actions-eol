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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_API_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_API_H__

/**
 * SECTION: na_object
 * @short_description: #NAObject public API.
 * @include: caja-actions/na-object-api.h
 *
 * We define here a common API which makes easier to write (and read)
 * the code ; all object functions are named na_object ; all arguments
 * are casted directly in the macro.
 */

#include "na-ifactory-object.h"
#include "na-ifactory-object-data.h"
#include "na-iduplicable.h"
#include "na-icontext.h"
#include "na-object-action.h"
#include "na-object-profile.h"
#include "na-object-menu.h"

G_BEGIN_DECLS

/* NAIDuplicable
 */
#define na_object_duplicate( obj )						na_iduplicable_duplicate( NA_IDUPLICABLE( obj ))
#define na_object_check_status( obj )					na_object_object_check_status( NA_OBJECT( obj ))
#define na_object_check_status_up( obj )				na_object_object_check_status_up( NA_OBJECT( obj ))

#define na_object_get_origin( obj )						na_iduplicable_get_origin( NA_IDUPLICABLE( obj ))
#define na_object_is_valid( obj )						na_iduplicable_is_valid( NA_IDUPLICABLE( obj ))
#define na_object_is_modified( obj )					na_iduplicable_is_modified( NA_IDUPLICABLE( obj ))

#define na_object_set_origin( obj, origin )				na_iduplicable_set_origin( NA_IDUPLICABLE( obj ), ( NAIDuplicable * )( origin ))
#define na_object_reset_origin( obj, origin )			na_object_object_reset_origin( NA_OBJECT( obj ), ( NAObject * )( origin ))

/* NAObject
 */
#define na_object_copy( tgt, src, rec )					na_object_object_copy( NA_OBJECT( tgt ), NA_OBJECT( src ), ( rec ))
#define na_object_dump( obj )							na_object_object_dump( NA_OBJECT( obj ))
#define na_object_dump_norec( obj )						na_object_object_dump_norec( NA_OBJECT( obj ))
#define na_object_dump_tree( tree )						na_object_object_dump_tree( tree )
#define na_object_get_hierarchy( obj )					na_object_object_get_hierarchy( NA_OBJECT( obj ))
#define na_object_ref( obj )							na_object_object_ref( NA_OBJECT( obj ))
#define na_object_unref( obj )							na_object_object_unref( NA_OBJECT( obj ))

#define na_object_debug_invalid( obj, reason )			na_object_object_debug_invalid( NA_OBJECT( obj ), ( const gchar * )( reason ))

/* NAObjectId
 */
#define na_object_get_id( obj )							(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ID ))
#define na_object_get_label( obj )						(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), ( NA_IS_OBJECT_PROFILE( obj ) ? NAFO_DATA_DESCNAME : NAFO_DATA_LABEL )))
#define na_object_get_parent( obj )						(( NAObjectItem * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARENT ))

#define na_object_set_id( obj, id )						na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ID, ( const void * )( id ))
#define na_object_set_label( obj, label )				na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), ( NA_IS_OBJECT_PROFILE( obj ) ? NAFO_DATA_DESCNAME : NAFO_DATA_LABEL ), ( const void * )( label ))
#define na_object_set_parent( obj, parent )				na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARENT, ( const void * )( parent ))

#define na_object_sort_alpha_asc( a, b )				na_object_id_sort_alpha_asc( NA_OBJECT_ID( a ), NA_OBJECT_ID( b ))
#define na_object_sort_alpha_desc( a, b )				na_object_id_sort_alpha_desc( NA_OBJECT_ID( a ), NA_OBJECT_ID( b ))

#define na_object_prepare_for_paste( obj, relabel, renumber, parent ) \
														na_object_id_prepare_for_paste( NA_OBJECT_ID( obj ), ( relabel ), ( renumber ), ( NAObjectId * )( parent ))
#define na_object_set_copy_of_label( obj )				na_object_id_set_copy_of_label( NA_OBJECT_ID( obj ))
#define na_object_set_new_id( obj, parent )				na_object_id_set_new_id( NA_OBJECT_ID( obj ), ( NAObjectId * )( parent ))

/* NAObjectItem
 */
#define na_object_get_tooltip( obj )					(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLTIP ))
#define na_object_get_icon( obj )						(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ICON ))
#define na_object_get_items( obj )						(( GList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS ))
#define na_object_get_items_slist( obj )				(( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS_SLIST ))
#define na_object_is_enabled( obj )						(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ENABLED )))
#define na_object_is_readonly( obj )					(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_READONLY )))
#define na_object_get_provider( obj )					na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER )
#define na_object_get_provider_data( obj )				na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER_DATA )

#define na_object_set_tooltip( obj, tooltip )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLTIP, ( const void * )( tooltip ))
#define na_object_set_icon( obj, icon )					na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ICON, ( const void * )( icon ))
#define na_object_set_items( obj, list )				na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS, ( const void * )( list ))
#define na_object_set_items_slist( obj, slist )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS_SLIST, ( const void * )( slist ))
#define na_object_set_enabled( obj, enabled )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ENABLED, ( const void * ) GUINT_TO_POINTER( enabled ))
#define na_object_set_readonly( obj, readonly )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_READONLY, ( const void * ) GUINT_TO_POINTER( readonly ))
#define na_object_set_provider( obj, provider )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER, ( const void * )( provider ))
#define na_object_set_provider_data( obj, data )		na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER_DATA, ( const void * )( data ))

#define na_object_get_item( obj, id )					na_object_item_get_item( NA_OBJECT_ITEM( obj ),( const gchar * )( id ))
#define na_object_get_position( obj, child )			na_object_item_get_position( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ))
#define na_object_append_item( obj, child )				na_object_item_append_item( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ))
#define na_object_insert_at( obj, child, pos )			na_object_item_insert_at( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ), ( pos ))
#define na_object_insert_item( obj, child, sibling )	na_object_item_insert_item( NA_OBJECT_ITEM( obj ), NA_OBJECT( child ), ( NAObject * )( sibling ))
#define na_object_remove_item( obj, child )				na_object_item_remove_item( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ))

#define na_object_get_items_count( obj )				na_object_item_get_items_count( NA_OBJECT_ITEM( obj ))
#define na_object_count_items( list, cm, ca, cp, brec )	na_object_item_count_items( list, ( cm ), ( ca ), ( cp ), ( brec ))
#define na_object_unref_items( tree )					na_object_item_unref_items( tree )
#define na_object_unref_selected_items( tree )			na_object_item_unref_items_rec( tree )

/* NAObjectAction
 */
#define na_object_get_version( obj )					(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_VERSION ))
#define na_object_is_target_selection( obj )			(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_SELECTION )))
#define na_object_is_target_location( obj )				(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_LOCATION )))
#define na_object_is_target_toolbar( obj )				(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_TOOLBAR )))
#define na_object_get_toolbar_label( obj )				(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_LABEL ))
#define na_object_is_toolbar_same_label( obj )			(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_SAME_LABEL )))
#define na_object_get_last_allocated( obj )				(( guint ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_LAST_ALLOCATED )))

#define na_object_set_version( obj, version )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_VERSION, ( const void * )( version ))
#define na_object_set_target_selection( obj, target )	na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_SELECTION, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_target_location( obj, target )	na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_LOCATION, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_target_toolbar( obj, target )		na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_TOOLBAR, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_toolbar_label( obj, label )		na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_LABEL, ( const void * )( label ))
#define na_object_set_toolbar_same_label( obj, same )	na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_SAME_LABEL, ( const void * ) GUINT_TO_POINTER( same ))
#define na_object_set_last_allocated( obj, last )		na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_LAST_ALLOCATED, ( const void * ) GUINT_TO_POINTER( last ))

#define na_object_set_last_version( obj )				na_object_action_set_last_version( NA_OBJECT_ACTION( obj ))
#define na_object_reset_last_allocated( obj )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_LAST_ALLOCATED, ( const void * ) GUINT_TO_POINTER( 0 ))
#define na_object_attach_profile( obj, profile )		na_object_action_attach_profile( NA_OBJECT_ACTION( obj ), NA_OBJECT_PROFILE( profile ))

/* NAObjectProfile
 */
#define na_object_get_path( obj )						(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PATH ))
#define na_object_get_parameters( obj )					(( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARAMETERS ))
#define na_object_get_basenames( obj )					(( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_BASENAMES ))
#define na_object_is_matchcase( obj )					(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MATCHCASE )))
#define na_object_get_mimetypes( obj )					(( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MIMETYPES ))
#define na_object_is_file( obj )						(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ISFILE )))
#define na_object_is_dir( obj )							(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ISDIR )))
#define na_object_is_multiple( obj )					(( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MULTIPLE )))
#define na_object_get_schemes( obj )					(( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SCHEMES ))
#define na_object_get_folders( obj )					(( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_FOLDERS ))

#define na_object_set_path( obj, path )					na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PATH, ( const void * )( path ))
#define na_object_set_parameters( obj, parms )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARAMETERS, ( const void * )( parms ))
#define na_object_set_basenames( obj, bnames )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_BASENAMES, ( const void * )( bnames ))
#define na_object_set_matchcase( obj, match )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MATCHCASE, ( const void * ) GUINT_TO_POINTER( match ))
#define na_object_set_mimetypes( obj, types )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MIMETYPES, ( const void * )( types ))
#define na_object_set_isfile( obj, isfile )				na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ISFILE, ( const void * ) GUINT_TO_POINTER( isfile ))
#define na_object_set_isdir( obj, isdir )				na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ISDIR, ( const void * ) GUINT_TO_POINTER( isdir ))
#define na_object_set_multiple( obj, multiple )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MULTIPLE, ( const void * ) GUINT_TO_POINTER( multiple ))
#define na_object_set_schemes( obj, schemes )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SCHEMES, ( const void * )( schemes ))
#define na_object_set_folders( obj, folders )			na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_FOLDERS, ( const void * )( folders ))

/* NAIContext
 */
#define na_object_set_scheme( obj, scheme, add )		na_icontext_set_scheme( NA_ICONTEXT( obj ), ( const gchar * )( scheme ), ( add ))
#define na_object_replace_folder( obj, old, new )		na_icontext_replace_folder( NA_ICONTEXT( obj ), ( const gchar * )( old ), ( const gchar * )( new ))

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_API_H__ */
