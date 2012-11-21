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

#ifndef __CORE_NA_IMPORTER_H__
#define __CORE_NA_IMPORTER_H__

/* @title: NAIImporter
 * @short_description: The #NAIImporter Internal Functions
 * @include: core/na-importer.h
 *
 * Internal Caja-Actions code should never directly call a
 * #NAIImporter interface method, but rather should call the
 * corresponding na_importer_xxx() functions.
 *
 * Importing items is a three-phase operation:
 *
 * - first, just try to find an i/o provider which is willing to import
 *   the item;
 *   at this time, only some uris have been successfully imported
 *
 * - check then for existence of each imported item;
 *   depending of the preferred import mode, this may be an interactive
 *   process;
 *   at this time, the importation of some objects may have been cancelled
 *   by the user
 *
 * - last, and depending of the exact individual import mode of each item,
 *   insert new items or override existing ones in the referential of the
 *   import context.
 */

#include <gtk/gtk.h>

#include <api/na-iimporter.h>
#include <api/na-object-item.h>

#include "na-ioption.h"
#include "na-pivot.h"

G_BEGIN_DECLS

#ifndef NA_ENABLE_DEPRECATED
/*
 * NAImporterImportMode:
 * @IMPORTER_MODE_NO_IMPORT: a "do not import" mode.
 * @IMPORTER_MODE_RENUMBER:  reallocate a new id when the imported one already exists.
 * @IMPORTER_MODE_OVERRIDE:  override the existing id with the imported one.
 * @IMPORTER_MODE_ASK:       ask the user for what to do with this particular item.
 *
 * Define the mode of an import operation.
 *
 * Since: 3.2
 *
 * This same enum used to be defined as NAIImporterImportMode in api/na-iimporter.h
 * header. The enum has been deprecated there in N-A 3.2 when the NAIImporter v2
 * interface was defined. It has so been moved here with the NAImporterImportMode
 * name.
 */
typedef enum {
	IMPORTER_MODE_NO_IMPORT = 1,
	IMPORTER_MODE_RENUMBER,
	IMPORTER_MODE_OVERRIDE,
	IMPORTER_MODE_ASK
}
	NAImporterImportMode;
#endif

/*
 * NAImporterCheckFn:
 * @imported: the currently imported #NAObjectItem -derived object.
 * @fn_data: some data to be passed to the function.
 *
 * The library takes care of checking for duplicates inside of the imported
 * population.
 *
 * The caller may provide this function in order to check for duplicates
 * in its own import context, e.g. (and typically) by checking for a same
 * identifier in the main window items tree view.
 *
 * The function should return the already existing item which has the same id
 * than the currently being imported one, or %NULL if the imported id will be
 * unique.
 *
 * If the caller does not provide its own check function, then each imported
 * item will be systematically renumbered (allocated a new identifier).
 *
 * Returns: the already existing #NAObjectItem with same id, or %NULL.
 *
 * Since: 3.2
 */
typedef NAObjectItem * ( *NAImporterCheckFn )( const NAObjectItem *, void * );

typedef struct {
	GSList             *uris;				/* the list of uris to import */
	NAImporterCheckFn   check_fn;			/* the check_for_duplicate function */
	void               *check_fn_data;		/* data to be passed to the check_fn function */
	guint               preferred_mode;		/* preferred import mode, defaults to NA_IPREFS_IMPORT_PREFERRED_MODE */
	GtkWindow          *parent_toplevel;	/* parent toplevel */
}
	NAImporterParms;

typedef struct {

	/* phase 1: import into memory from i/o provider
	 */
	gchar        *uri;					/* the imported uri */
	NAObjectItem *imported;				/* the imported NAObjectItem-derived object, or %NULL */
	NAIImporter  *importer;				/* the importer module, or %NULL */

	/* phase 2: check for pre-existence
	 */
	gboolean      exist;				/* whether the imported Id already existed */
	guint         mode;					/* the actual mode in effect for this import */
	GSList       *messages;				/* a #GSList list of localized strings */
}
	NAImporterResult;

GList     *na_importer_import_from_uris( const NAPivot *pivot, NAImporterParms *parms );

void       na_importer_free_result     ( NAImporterResult *result );

GList     *na_importer_get_modes       ( void );
void       na_importer_free_modes      ( GList *modes );

NAIOption *na_importer_get_ask_mode    ( void );

G_END_DECLS

#endif /* __CORE_NA_IMPORTER_H__ */
