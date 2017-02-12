/*
 * Caja-Actions
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

#ifndef __CADP_DESKTOP_FILE_H__
#define __CADP_DESKTOP_FILE_H__

/**
 * SECTION: cadp_desktop_file
 * @short_description: #CappDesktopFile class definition.
 * @include: cadp-desktop-file.h
 *
 * This class encapŝulates the EggDesktopFile structure, adding some
 * private properties. An instance of this class is associated with
 * every #NAObjectItem for this provider.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define CADP_TYPE_DESKTOP_FILE                ( cadp_desktop_file_get_type())
#define CADP_DESKTOP_FILE( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CADP_TYPE_DESKTOP_FILE, CappDesktopFile ))
#define CADP_DESKTOP_FILE_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CADP_TYPE_DESKTOP_FILE, CappDesktopFileClass ))
#define CADP_IS_DESKTOP_FILE( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CADP_TYPE_DESKTOP_FILE ))
#define CADP_IS_DESKTOP_FILE_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CADP_TYPE_DESKTOP_FILE ))
#define CADP_DESKTOP_FILE_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CADP_TYPE_DESKTOP_FILE, CappDesktopFileClass ))

typedef struct _CappDesktopFilePrivate        CappDesktopFilePrivate;

typedef struct {
	/*< private >*/
	GObject                 parent;
	CappDesktopFilePrivate *private;
}
	CappDesktopFile;

typedef struct _CappDesktopFileClassPrivate   CappDesktopFileClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass                 parent;
	CappDesktopFileClassPrivate *private;
}
	CappDesktopFileClass;

/* standard suffix for desktop files
 */
#define CADP_DESKTOP_FILE_SUFFIX		".desktop"

GType            cadp_desktop_file_get_type         ( void );

CappDesktopFile *cadp_desktop_file_new              ( void );
CappDesktopFile *cadp_desktop_file_new_from_path    ( const gchar *path );
CappDesktopFile *cadp_desktop_file_new_from_uri     ( const gchar *uri );
CappDesktopFile *cadp_desktop_file_new_for_write    ( const gchar *path );

GKeyFile        *cadp_desktop_file_get_key_file     ( const CappDesktopFile *ndf );
gchar           *cadp_desktop_file_get_key_file_uri ( const CappDesktopFile *ndf );
gboolean         cadp_desktop_file_write            ( CappDesktopFile *ndf );

gchar           *cadp_desktop_file_get_file_type    ( const CappDesktopFile *ndf );
gchar           *cadp_desktop_file_get_id           ( const CappDesktopFile *ndf );
GSList          *cadp_desktop_file_get_profiles     ( const CappDesktopFile *ndf );

gboolean         cadp_desktop_file_has_profile      ( const CappDesktopFile *ndf, const gchar *profile_id );

void             cadp_desktop_file_remove_key       ( const CappDesktopFile *ndf, const gchar *group, const gchar *key );
void             cadp_desktop_file_remove_profile   ( const CappDesktopFile *ndf, const gchar *profile_id );

gboolean         cadp_desktop_file_get_boolean      ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, gboolean *key_found, gboolean default_value );
gchar           *cadp_desktop_file_get_locale_string( const CappDesktopFile *ndf, const gchar *group, const gchar *key, gboolean *key_found, const gchar *default_value );
gchar           *cadp_desktop_file_get_string       ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, gboolean *key_found, const gchar *default_value );
GSList          *cadp_desktop_file_get_string_list  ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, gboolean *key_found, const gchar *default_value );
guint            cadp_desktop_file_get_uint         ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, gboolean *key_found, guint default_value );

void             cadp_desktop_file_set_boolean      ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, gboolean value );
void             cadp_desktop_file_set_locale_string( const CappDesktopFile *ndf, const gchar *group, const gchar *key, const gchar *value );
void             cadp_desktop_file_set_string       ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, const gchar *value );
void             cadp_desktop_file_set_string_list  ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, GSList *value );
void             cadp_desktop_file_set_uint         ( const CappDesktopFile *ndf, const gchar *group, const gchar *key, guint value );

G_END_DECLS

#endif /* __CADP_DESKTOP_FILE_H__ */
