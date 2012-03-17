/*
 * Caja Actions
 * A Caja extension which offers configurable context menu selected_infos.
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

#ifndef __CORE_NA_SELECTED_INFO_H__
#define __CORE_NA_SELECTED_INFO_H__

/**
 * SECTION: na_selected_info
 * @short_description: #NASelectedInfo class definition.
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

#define NA_SELECTED_INFO_TYPE					( na_selected_info_get_type())
#define NA_SELECTED_INFO( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_SELECTED_INFO_TYPE, NASelectedInfo ))
#define NA_SELECTED_INFO_CLASS( klass )			( G_TYPE_CHECK_CLASS_CAST( klass, NA_SELECTED_INFO_TYPE, NASelectedInfoClass ))
#define NA_IS_SELECTED_INFO( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_SELECTED_INFO_TYPE ))
#define NA_IS_SELECTED_INFO_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_SELECTED_INFO_TYPE ))
#define NA_SELECTED_INFO_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_SELECTED_INFO_TYPE, NASelectedInfoClass ))

typedef struct NASelectedInfoPrivate      NASelectedInfoPrivate;

typedef struct {
	GObject                parent;
	NASelectedInfoPrivate *private;
}
	NASelectedInfo;

typedef struct NASelectedInfoClassPrivate NASelectedInfoClassPrivate;

typedef struct {
	GObjectClass                parent;
	NASelectedInfoClassPrivate *private;
}
	NASelectedInfoClass;

GType           na_selected_info_get_type( void );

GList          *na_selected_info_get_list_from_item( CajaFileInfo *item );
GList          *na_selected_info_get_list_from_list( GList *caja_selection );
GList          *na_selected_info_copy_list         ( GList *list );
void            na_selected_info_free_list         ( GList *list );

GFile          *na_selected_info_get_location  ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_mime_type ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_path      ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_uri       ( const NASelectedInfo *nsi );
gchar          *na_selected_info_get_uri_scheme( const NASelectedInfo *nsi );
gboolean        na_selected_info_is_directory  ( const NASelectedInfo *nsi );

NASelectedInfo *na_selected_info_create_for_uri( const gchar *uri, const gchar *mimetype );

G_END_DECLS

#endif /* __CORE_NA_SELECTED_INFO_H__ */
