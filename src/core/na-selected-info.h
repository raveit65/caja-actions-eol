/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu selected_infos.
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

#ifndef __CORE_NA_SELECTED_INFO_H__
#define __CORE_NA_SELECTED_INFO_H__

/* @title: NASelectedInfo
 * @short_description: The #NASelectedInfo Class Definition
 * @include: core/na-selected-info.h
 *
 * An object is instantiated for each Caja selected item, in order
 * to gather some common properties for the selected item, mainly its
 * mime type for example.
 *
 * This class should be replaced by CajaFileInfo class, as soon as
 * the required Caja version will have the
 * caja_file_info_create_for_uri() API (after 2.28)
 */

#include <libcaja-extension/caja-file-info.h>

G_BEGIN_DECLS

#define NA_TYPE_SELECTED_INFO                ( na_selected_info_get_type())
#define NA_SELECTED_INFO( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_SELECTED_INFO, NASelectedInfo ))
#define NA_SELECTED_INFO_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_SELECTED_INFO, NASelectedInfoClass ))
#define NA_IS_SELECTED_INFO( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_SELECTED_INFO ))
#define NA_IS_SELECTED_INFO_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_SELECTED_INFO ))
#define NA_SELECTED_INFO_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_SELECTED_INFO, NASelectedInfoClass ))

typedef struct _NASelectedInfoPrivate        NASelectedInfoPrivate;

typedef struct {
	/*< private >*/
	GObject                parent;
	NASelectedInfoPrivate *private;
}
	NASelectedInfo;

typedef struct _NASelectedInfoClassPrivate   NASelectedInfoClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass                parent;
	NASelectedInfoClassPrivate *private;
}
	NASelectedInfoClass;

GType           na_selected_info_get_type( void );

GList          *na_selected_info_get_list_from_item( CajaFileInfo *item );
GList          *na_selected_info_get_list_from_list( GList *caja_selection );
GList          *na_selected_info_copy_list         ( GList *files );
void            na_selected_info_free_list         ( GList *files );

gchar          *na_selected_info_get_basename  ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_dirname   ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_mime_type ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_path      ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_uri       ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_uri_host  ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_uri_user  ( const NASelectedInfo *nsi );
guint           na_selected_info_get_uri_port  ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_uri_scheme( const NASelectedInfo *nsi );
gboolean        na_selected_info_is_directory  ( const NASelectedInfo *nsi );
gboolean        na_selected_info_is_regular    ( const NASelectedInfo *nsi );
gboolean        na_selected_info_is_executable ( const NASelectedInfo *nsi );
gboolean        na_selected_info_is_local      ( const NASelectedInfo *nsi );
gboolean        na_selected_info_is_owner      ( const NASelectedInfo *nsi, const gchar *user );
gboolean        na_selected_info_is_readable   ( const NASelectedInfo *nsi );
gboolean        na_selected_info_is_writable   ( const NASelectedInfo *nsi );

NASelectedInfo *na_selected_info_create_for_uri( const gchar *uri, const gchar *mimetype, gchar **errmsg );

G_END_DECLS

#endif /* __CORE_NA_SELECTED_INFO_H__ */
