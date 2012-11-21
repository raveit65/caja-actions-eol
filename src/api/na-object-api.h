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

#ifndef __CAJA_ACTIONS_API_NA_OBJECT_API_H__
#define __CAJA_ACTIONS_API_NA_OBJECT_API_H__

/**
 * SECTION: object-api
 * @title: API
 * @short_description: The Common Public #NAObject API
 * @include: caja-actions/na-object-api.h
 *
 * We define here a common API which makes easier to write (and read)
 * the code; all object functions are named na_object; all arguments
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
#define na_object_duplicate( obj, mode )                na_iduplicable_duplicate( NA_IDUPLICABLE( obj ), mode )
#define na_object_check_status( obj )                   na_object_object_check_status_rec( NA_OBJECT( obj ))

#define na_object_get_origin( obj )                     na_iduplicable_get_origin( NA_IDUPLICABLE( obj ))
#define na_object_is_valid( obj )                       na_iduplicable_is_valid( NA_IDUPLICABLE( obj ))
#define na_object_is_modified( obj )                    na_iduplicable_is_modified( NA_IDUPLICABLE( obj ))

#define na_object_set_origin( obj, origin )             na_iduplicable_set_origin( NA_IDUPLICABLE( obj ), ( NAIDuplicable * )( origin ))
#define na_object_reset_origin( obj, origin )           na_object_object_reset_origin( NA_OBJECT( obj ), ( NAObject * )( origin ))

/* NAObject
 */
#define na_object_dump( obj )                           na_object_object_dump( NA_OBJECT( obj ))
#define na_object_dump_norec( obj )                     na_object_object_dump_norec( NA_OBJECT( obj ))
#define na_object_dump_tree( tree )                     na_object_object_dump_tree( tree )
#define na_object_ref( obj )                            na_object_object_ref( NA_OBJECT( obj ))
#define na_object_unref( obj )                          na_object_object_unref( NA_OBJECT( obj ))

#define na_object_debug_invalid( obj, reason )          na_object_object_debug_invalid( NA_OBJECT( obj ), ( const gchar * )( reason ))

/* NAObjectId
 */
#define na_object_get_id( obj )                         (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ID ))
#define na_object_get_label( obj )                      (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), ( NA_IS_OBJECT_PROFILE( obj ) ? NAFO_DATA_DESCNAME : NAFO_DATA_LABEL )))
#define na_object_get_label_noloc( obj )                (( gchar * )( NA_IS_OBJECT_PROFILE( obj ) ? na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_DESCNAME_NOLOC ) : NULL ))
#define na_object_get_parent( obj )                     (( NAObjectItem * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARENT ))

#define na_object_set_id( obj, id )                     na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ID, ( const void * )( id ))
#define na_object_set_label( obj, label )               na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), ( NA_IS_OBJECT_PROFILE( obj ) ? NAFO_DATA_DESCNAME : NAFO_DATA_LABEL ), ( const void * )( label ))
#define na_object_set_parent( obj, parent )             na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARENT, ( const void * )( parent ))

#define na_object_sort_alpha_asc( a, b )                na_object_id_sort_alpha_asc( NA_OBJECT_ID( a ), NA_OBJECT_ID( b ))
#define na_object_sort_alpha_desc( a, b )               na_object_id_sort_alpha_desc( NA_OBJECT_ID( a ), NA_OBJECT_ID( b ))

#define na_object_prepare_for_paste( obj, relabel, renumber, parent ) \
                                                        na_object_id_prepare_for_paste( NA_OBJECT_ID( obj ), ( relabel ), ( renumber ), ( NAObjectId * )( parent ))
#define na_object_set_copy_of_label( obj )              na_object_id_set_copy_of_label( NA_OBJECT_ID( obj ))
#define na_object_set_new_id( obj, parent )             na_object_id_set_new_id( NA_OBJECT_ID( obj ), ( NAObjectId * )( parent ))

/* NAObjectItem
 */
#define na_object_get_tooltip( obj )                    (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLTIP ))
#define na_object_get_icon( obj )                       (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ICON ))
#define na_object_get_icon_noloc( obj )                 (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ICON_NOLOC ))
#define na_object_get_description( obj )                (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_DESCRIPTION ))
#define na_object_get_items( obj )                      (( GList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS ))
#define na_object_get_items_slist( obj )                (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS_SLIST ))
#define na_object_is_enabled( obj )                     (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ENABLED )))
#define na_object_is_readonly( obj )                    (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_READONLY )))
#define na_object_get_provider( obj )                   na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER )
#define na_object_get_provider_data( obj )              na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER_DATA )
#define na_object_get_iversion( obj )                   GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_IVERSION ))
#define na_object_get_shortcut( obj )                   (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHORTCUT ))

#define na_object_set_tooltip( obj, tooltip )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLTIP, ( const void * )( tooltip ))
#define na_object_set_icon( obj, icon )                 na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ICON, ( const void * )( icon ))
#define na_object_set_description( obj, desc )          na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_DESCRIPTION, ( const void * )( desc ))
#define na_object_set_items( obj, list )                na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS, ( const void * )( list ))
#define na_object_set_items_slist( obj, slist )         na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SUBITEMS_SLIST, ( const void * )( slist ))
#define na_object_set_enabled( obj, enabled )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ENABLED, ( const void * ) GUINT_TO_POINTER( enabled ))
#define na_object_set_readonly( obj, readonly )         na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_READONLY, ( const void * ) GUINT_TO_POINTER( readonly ))
#define na_object_set_provider( obj, provider )         na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER, ( const void * )( provider ))
#define na_object_set_provider_data( obj, data )        na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PROVIDER_DATA, ( const void * )( data ))
#define na_object_set_iversion( obj, version )          na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_IVERSION, ( const void * ) GUINT_TO_POINTER( version ))
#define na_object_set_shortcut( obj, shortcut )         na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHORTCUT, ( const void * )( shortcut ))

#define na_object_get_item( obj, id )                   na_object_item_get_item( NA_OBJECT_ITEM( obj ),( const gchar * )( id ))
#define na_object_get_position( obj, child )            na_object_item_get_position( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ))
#define na_object_append_item( obj, child )             na_object_item_append_item( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ))
#define na_object_insert_at( obj, child, pos )          na_object_item_insert_at( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ), ( pos ))
#define na_object_insert_item( obj, child, sibling )    na_object_item_insert_item( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ), ( NAObjectId * )( sibling ))
#define na_object_remove_item( obj, child )             na_object_item_remove_item( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ))

#define na_object_get_items_count( obj )                na_object_item_get_items_count( NA_OBJECT_ITEM( obj ))
#define na_object_count_items( list, cm, ca, cp )       na_object_item_count_items( list, ( cm ), ( ca ), ( cp ), TRUE )
#define na_object_copyref_items( tree )                 na_object_item_copyref_items( tree )
#define na_object_free_items( tree )                    na_object_item_free_items( tree )

#define na_object_is_finally_writable( obj, r )			na_object_item_is_finally_writable( NA_OBJECT_ITEM( obj ), ( r ))
#define na_object_set_writability_status( obj, w, r )	na_object_item_set_writability_status( NA_OBJECT_ITEM( obj ), ( w ), ( r ))

/* NAObjectAction
 */
#define na_object_get_version( obj )                    (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_VERSION ))
#define na_object_is_target_selection( obj )            (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_SELECTION )))
#define na_object_is_target_location( obj )             (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_LOCATION )))
#define na_object_is_target_toolbar( obj )              (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_TOOLBAR )))
#define na_object_get_toolbar_label( obj )              (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_LABEL ))
#define na_object_is_toolbar_same_label( obj )          (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_SAME_LABEL )))
#define na_object_get_last_allocated( obj )             (( guint ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_LAST_ALLOCATED )))

#define na_object_set_version( obj, version )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_VERSION, ( const void * )( version ))
#define na_object_set_target_selection( obj, target )   na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_SELECTION, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_target_location( obj, target )    na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_LOCATION, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_target_toolbar( obj, target )     na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TARGET_TOOLBAR, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_toolbar_label( obj, label )       na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_LABEL, ( const void * )( label ))
#define na_object_set_toolbar_same_label( obj, same )   na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TOOLBAR_SAME_LABEL, ( const void * ) GUINT_TO_POINTER( same ))
#define na_object_set_last_allocated( obj, last )       na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_LAST_ALLOCATED, ( const void * ) GUINT_TO_POINTER( last ))

#define na_object_set_last_version( obj )               na_object_action_set_last_version( NA_OBJECT_ACTION( obj ))
#define na_object_reset_last_allocated( obj )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_LAST_ALLOCATED, ( const void * ) GUINT_TO_POINTER( 0 ))
#define na_object_attach_profile( obj, profile )        na_object_action_attach_profile( NA_OBJECT_ACTION( obj ), NA_OBJECT_PROFILE( profile ))

/* NAObjectProfile
 */
#define na_object_get_path( obj )                       (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PATH ))
#define na_object_get_parameters( obj )                 (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARAMETERS ))
#define na_object_get_working_dir( obj )                (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_WORKING_DIR ))
#define na_object_get_execution_mode( obj )             (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_EXECUTION_MODE ))
#define na_object_get_startup_notify( obj )             (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_STARTUP_NOTIFY )))
#define na_object_get_startup_class( obj )              (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_STARTUP_WMCLASS ))
#define na_object_get_execute_as( obj )                 (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_EXECUTE_AS ))

#define na_object_set_path( obj, path )                 na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PATH, ( const void * )( path ))
#define na_object_set_parameters( obj, parms )          na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_PARAMETERS, ( const void * )( parms ))
#define na_object_set_working_dir( obj, uri )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_WORKING_DIR, ( const void * )( uri ))
#define na_object_set_execution_mode( obj, mode )       na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_EXECUTION_MODE, ( const void * )( mode ))
#define na_object_set_startup_notify( obj, notify )     na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_STARTUP_NOTIFY, ( const void * ) GUINT_TO_POINTER( notify ))
#define na_object_set_startup_class( obj, class )       na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_STARTUP_WMCLASS, ( const void * )( class ))
#define na_object_set_execute_as( obj, user )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_EXECUTE_AS, ( const void * )( user ))

/* NAIContext
 */
#define na_object_check_mimetypes( obj )                na_icontext_check_mimetypes( NA_ICONTEXT( obj ))

#define na_object_get_basenames( obj )                  (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_BASENAMES ))
#define na_object_get_matchcase( obj )                  (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MATCHCASE )))
#define na_object_get_mimetypes( obj )                  (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MIMETYPES ))
#define na_object_get_all_mimetypes( obj )              (( gboolean ) GPOINTER_TO_UINT( na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MIMETYPES_IS_ALL )))
#define na_object_get_folders( obj )                    (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_FOLDERS ))
#define na_object_get_schemes( obj )                    (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SCHEMES ))
#define na_object_get_only_show_in( obj )               (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ONLY_SHOW ))
#define na_object_get_not_show_in( obj )                (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_NOT_SHOW ))
#define na_object_get_try_exec( obj )                   (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TRY_EXEC ))
#define na_object_get_show_if_registered( obj )         (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHOW_IF_REGISTERED ))
#define na_object_get_show_if_true( obj )               (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHOW_IF_TRUE ))
#define na_object_get_show_if_running( obj )            (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHOW_IF_RUNNING ))
#define na_object_get_selection_count( obj )            (( gchar * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SELECTION_COUNT ))
#define na_object_get_capabilities( obj )               (( GSList * ) na_ifactory_object_get_as_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_CAPABILITITES ))

#define na_object_set_basenames( obj, bnames )          na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_BASENAMES, ( const void * )( bnames ))
#define na_object_set_matchcase( obj, match )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MATCHCASE, ( const void * ) GUINT_TO_POINTER( match ))
#define na_object_set_mimetypes( obj, types )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MIMETYPES, ( const void * )( types ))
#define na_object_set_all_mimetypes( obj, all )         na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_MIMETYPES_IS_ALL, ( const void * ) GUINT_TO_POINTER( all ))
#define na_object_set_folders( obj, folders )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_FOLDERS, ( const void * )( folders ))
#define na_object_replace_folder( obj, old, new )       na_icontext_replace_folder( NA_ICONTEXT( obj ), ( const gchar * )( old ), ( const gchar * )( new ))
#define na_object_set_scheme( obj, scheme, add )        na_icontext_set_scheme( NA_ICONTEXT( obj ), ( const gchar * )( scheme ), ( add ))
#define na_object_set_schemes( obj, schemes )           na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SCHEMES, ( const void * )( schemes ))
#define na_object_set_only_show_in( obj, list )         na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_ONLY_SHOW, ( const void * )( list ))
#define na_object_set_only_desktop( obj, desktop, add ) na_icontext_set_only_desktop( NA_ICONTEXT( obj ), ( const gchar * )( desktop ), ( add ))
#define na_object_set_not_show_in( obj, list )          na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_NOT_SHOW, ( const void * )( list ))
#define na_object_set_not_desktop( obj, desktop, add )  na_icontext_set_not_desktop( NA_ICONTEXT( obj ), ( const gchar * )( desktop ), ( add ))
#define na_object_set_try_exec( obj, exec )             na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_TRY_EXEC, ( const void * )( exec ))
#define na_object_set_show_if_registered( obj, name )   na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHOW_IF_REGISTERED, ( const void * )( name ))
#define na_object_set_show_if_true( obj, exec )         na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHOW_IF_TRUE, ( const void * )( exec ))
#define na_object_set_show_if_running( obj, name )      na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SHOW_IF_RUNNING, ( const void * )( name ))
#define na_object_set_selection_count( obj, cond )      na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_SELECTION_COUNT, ( const void * )( cond ))
#define na_object_set_capabilities( obj, cap )          na_ifactory_object_set_from_void( NA_IFACTORY_OBJECT( obj ), NAFO_DATA_CAPABILITITES, ( const void * )( cap ))

#ifdef NA_ENABLE_DEPRECATED
#define na_object_set_modified( obj, modified )         na_iduplicable_set_modified( NA_IDUPLICABLE( obj ), ( modified ))
#endif

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_OBJECT_API_H__ */
