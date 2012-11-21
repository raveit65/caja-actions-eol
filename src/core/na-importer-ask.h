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

#ifndef __CORE_NA_IMPORTER_ASK_H__
#define __CORE_NA_IMPORTER_ASK_H__

/* @title: NAImporterAsk
 * @short_description: The #NAImporterAsk Class Definition
 * @include: core/na-importer-ask.h
 *
 * This class creates and manages a dialog. It is ran each time an
 * imported action has the same Id as an existing one, and the user
 * want to be asked to know what to do with it.
 */

#include <gtk/gtk.h>

#include <api/na-object-item.h>

#include "na-pivot.h"

G_BEGIN_DECLS

#define NA_TYPE_IMPORTER_ASK                ( na_importer_ask_get_type())
#define NA_IMPORTER_ASK( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, NA_TYPE_IMPORTER_ASK, NAImporterAsk ))
#define NA_IMPORTER_ASK_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, NA_TYPE_IMPORTER_ASK, NAImporterAskClass ))
#define NA_IS_IMPORTER_ASK( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_TYPE_IMPORTER_ASK ))
#define NA_IS_IMPORTER_ASK_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_TYPE_IMPORTER_ASK ))
#define NA_IMPORTER_ASK_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_TYPE_IMPORTER_ASK, NAImporterAskClass ))

typedef struct _NAImporterAskPrivate        NAImporterAskPrivate;

typedef struct {
	/*< private >*/
	GObject               parent;
	NAImporterAskPrivate *private;
}
	NAImporterAsk;

typedef struct _NAImporterAskClassPrivate   NAImporterAskClassPrivate;

typedef struct {
	/*< private >*/
	GObjectClass               parent;
	NAImporterAskClassPrivate *private;
}
	NAImporterAskClass;

typedef struct {
	GtkWindow     *parent;
	gchar         *uri;
	guint          count;
	gboolean       keep_choice;
	const NAPivot *pivot;
}
	NAImporterAskUserParms;

GType na_importer_ask_get_type( void );

guint na_importer_ask_user( const NAObjectItem *importing, const NAObjectItem *existing, NAImporterAskUserParms *parms );

G_END_DECLS

#endif /* __CORE_NA_IMPORTER_ASK_H__ */
