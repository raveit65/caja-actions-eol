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

#ifndef __NACT_IPREFS_H__
#define __NACT_IPREFS_H__

/**
 * SECTION: nact_iprefs
 * @short_description: #NactIPrefs interface definition.
 * @include: nact/nact-iprefs.h
 *
 * This interface cooperates with #NactWindow to manage preferences.
 */

#include "base-window.h"

G_BEGIN_DECLS

#define NACT_IPREFS_TYPE						( nact_iprefs_get_type())
#define NACT_IPREFS( object )					( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_IPREFS_TYPE, NactIPrefs ))
#define NACT_IS_IPREFS( object )				( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_IPREFS_TYPE ))
#define NACT_IPREFS_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NACT_IPREFS_TYPE, NactIPrefsInterface ))

typedef struct NactIPrefs                 NactIPrefs;

typedef struct NactIPrefsInterfacePrivate NactIPrefsInterfacePrivate;

typedef struct {
	GTypeInterface              parent;
	NactIPrefsInterfacePrivate *private;
}
	NactIPrefsInterface;

#define IPREFS_EXPORT_ITEMS_FOLDER_URI			"export-folder-uri"
#define IPREFS_EXPORT_FORMAT					"export-format"
#define IPREFS_EXPORT_ASK_LAST_FORMAT			"export-ask-user-last-format"

#define IPREFS_EXPORT_FORMAT_DEFAULT			"MateConfEntry"

#define IPREFS_IMPORT_ITEMS_FOLDER_URI			"import-folder-uri"

#define IPREFS_ASSIST_ESC_QUIT					"assistant-esc-quit"
#define IPREFS_ASSIST_ESC_CONFIRM				"assistant-esc-confirm"

/* these are special export formats
 */
enum {
	IPREFS_EXPORT_NO_EXPORT = 1,
	IPREFS_EXPORT_FORMAT_MATECONF_SCHEMA
};

GType nact_iprefs_get_type( void );

GQuark nact_iprefs_get_export_format( const BaseWindow *window, const gchar *pref );
void   nact_iprefs_set_export_format( const BaseWindow *window, const gchar *pref, GQuark format );

void  nact_iprefs_migrate_key       ( const BaseWindow *window, const gchar *old_key, const gchar *new_key );

void  nact_iprefs_write_bool        ( const BaseWindow *window, const gchar *key, gboolean value );
void  nact_iprefs_write_string      ( const BaseWindow *window, const gchar *name, const gchar *value );

G_END_DECLS

#endif /* __NACT_IPREFS_H__ */
