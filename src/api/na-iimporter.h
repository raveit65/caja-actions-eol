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

#ifndef __CAJA_ACTIONS_API_NA_IIMPORTER_H__
#define __CAJA_ACTIONS_API_NA_IIMPORTER_H__

/**
 * SECTION: na_iimporter
 * @short_description: #NAIImporter interface definition.
 * @include: caja-actions/na-iimporter.h
 *
 * The #NAIImporter interface imports  items from the outside world.
 *
 * Caja-Actions v 2.30 - API version:  1
 */

#include <gtk/gtk.h>

#include "na-object-item.h"

G_BEGIN_DECLS

#define NA_IIMPORTER_TYPE						( na_iimporter_get_type())
#define NA_IIMPORTER( instance )				( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_IIMPORTER_TYPE, NAIImporter ))
#define NA_IS_IIMPORTER( instance )				( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_IIMPORTER_TYPE ))
#define NA_IIMPORTER_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_IIMPORTER_TYPE, NAIImporterInterface ))

typedef struct NAIImporter                 NAIImporter;
typedef struct NAIImporterUriParms         NAIImporterUriParms;
typedef struct NAIImporterListParms        NAIImporterListParms;

typedef struct NAIImporterInterfacePrivate NAIImporterInterfacePrivate;

typedef struct {
	GTypeInterface               parent;
	NAIImporterInterfacePrivate *private;

	/**
	 * get_version:
	 * @instance: the #NAIImporter provider.
	 *
	 * Returns: the version of this interface supported by the I/O provider.
	 *
	 * Defaults to 1.
	 */
	guint ( *get_version )( const NAIImporter *instance );

	/**
	 * import_from_uri:
	 * @instance: the #NAIImporter provider.
	 * @parms: a #NAIImporterUriParms structure.
	 *
	 * Imports an item.
	 *
	 * Returns: the return code of the operation.
	 */
	guint ( *from_uri )   ( const NAIImporter *instance, NAIImporterUriParms *parms );
}
	NAIImporterInterface;

/* import mode
 */
enum {
	IMPORTER_MODE_NO_IMPORT = 1,		/* this is a "do not import anything" mode */
	IMPORTER_MODE_RENUMBER,
	IMPORTER_MODE_OVERRIDE,
	IMPORTER_MODE_ASK
};

/* return code
 */
enum {
	IMPORTER_CODE_OK = 0,
	IMPORTER_CODE_PROGRAM_ERROR,
	IMPORTER_CODE_NOT_WILLING_TO,
	IMPORTER_CODE_NO_ITEM_ID,
	IMPORTER_CODE_NO_ITEM_TYPE,
	IMPORTER_CODE_UNKNOWN_ITEM_TYPE,
	IMPORTER_CODE_CANCELLED
};

/* the function which should be provider by the caller in order the
 * #NAIImporter provider be able to check for pre-existance of the
 * imported item
 *
 * the function should return the already existing item which has the
 * same id than the currently being imported one, or %NULL if the
 * imported id will be unique
 */
typedef NAObjectItem * ( *NAIImporterCheckFn )( const NAObjectItem *, const void *fn_data );

/* parameters via a structure
 * ... when importing a single uri
 */
struct NAIImporterUriParms {
	guint              version;			/* i 1: version of this structure */
	gchar             *uri;				/* i 1: uri of the file to be imported */
	guint              mode;			/* i 1: import mode */
	GtkWindow         *window;			/* i 1: a window which will act as a parent of the ask dialog */
	NAObjectItem      *imported;		/*  o1: eventually imported NAObjectItem-derived object */
	NAIImporterCheckFn check_fn;		/* i 1: a function to check the existance of the imported item */
	void              *check_fn_data;	/* i 1: data function */
	GSList            *messages;		/* io1: a #GSList list of localized strings;
										 *       the provider may append messages to this list,
										 *       but shouldn't reinitialize it. */
};

/* ... when importing a list of uris
 */
struct NAIImporterListParms {
	guint              version;			/* i 1: version of this structure */
	GSList            *uris;			/* i 1: list of uris of the files to be imported */
	guint              mode;			/* i 1: import mode */
	GtkWindow         *window;			/* i 1: a window which will act as a parent of the ask dialog */
	GList             *imported;		/*  o1: list of eventually imported NAObjectItem-derived objects */
	NAIImporterCheckFn check_fn;		/* i 1: a function to check the existance of each imported item */
	void              *check_fn_data;	/* i 1: data function */
	GSList            *messages;		/* io1: a #GSList list of localized strings;
										 *       the provider may append messages to this list,
										 *       but shouldn't reinitialize it. */
};

GType na_iimporter_get_type( void );

guint na_iimporter_ask_user( const NAIImporter *importer, const NAIImporterUriParms *parms, const NAObjectItem *existing );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IIMPORTER_H__ */
