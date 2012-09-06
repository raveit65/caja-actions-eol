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

#ifndef __CACT_CLIPBOARD_H__
#define __CACT_CLIPBOARD_H__

/*
 * SECTION: cact_clipboard.
 * @short_description: #CactClipboard class definition.
 * @include: cact/cact-clipboard.h
 *
 * This is just a convenience class to extract clipboard functions
 * from main window code. There is a unique object which manages all
 * clipboard buffers.
 */

#include <gtk/gtk.h>

#include "base-window.h"

G_BEGIN_DECLS

#define CACT_CLIPBOARD_TYPE					( cact_clipboard_get_type())
#define CACT_CLIPBOARD( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_CLIPBOARD_TYPE, CactClipboard ))
#define CACT_CLIPBOARD_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, CACT_CLIPBOARD_TYPE, CactClipboardClass ))
#define CACT_IS_CLIPBOARD( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_CLIPBOARD_TYPE ))
#define CACT_IS_CLIPBOARD_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_CLIPBOARD_TYPE ))
#define CACT_CLIPBOARD_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_CLIPBOARD_TYPE, CactClipboardClass ))

typedef struct CactClipboardPrivate      CactClipboardPrivate;

typedef struct {
	GObject               parent;
	CactClipboardPrivate *private;
}
	CactClipboard;

typedef struct CactClipboardClassPrivate CactClipboardClassPrivate;

typedef struct {
	GObjectClass               parent;
	CactClipboardClassPrivate *private;
}
	CactClipboardClass;

/* drag and drop formats
 */
enum {
	CACT_XCHANGE_FORMAT_CACT = 0,
	CACT_XCHANGE_FORMAT_XDS,
	CACT_XCHANGE_FORMAT_APPLICATION_XML,
	CACT_XCHANGE_FORMAT_TEXT_PLAIN,
	CACT_XCHANGE_FORMAT_URI_LIST
};

/* mode indicator
 */
enum {
	CLIPBOARD_MODE_CUT = 1,
	CLIPBOARD_MODE_COPY
};

GType          cact_clipboard_get_type( void );

CactClipboard *cact_clipboard_new( BaseWindow *window );

void           cact_clipboard_dnd_set( CactClipboard *clipboard, guint target, GList *rows, const gchar *folder, gboolean copy );
GList         *cact_clipboard_dnd_get_data( CactClipboard *clipboard, gboolean *copy );
gchar         *cact_clipboard_dnd_get_text( CactClipboard *clipboard, GList *rows );
void           cact_clipboard_dnd_drag_end( CactClipboard *clipboard );
void           cact_clipboard_dnd_clear( CactClipboard *clipboard );

void           cact_clipboard_primary_set( CactClipboard *clipboard, GList *items, gint mode );
GList         *cact_clipboard_primary_get( CactClipboard *clipboard, gboolean *relabel );
void           cact_clipboard_primary_counts( CactClipboard *clipboard, guint *actions, guint *profiles, guint *menus );

void           cact_clipboard_dump( CactClipboard *clipboard );

G_END_DECLS

#endif /* __CACT_CLIPBOARD_H__ */
