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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include <api/na-core-utils.h>
#include <api/na-iimporter.h>
#include <api/na-object-api.h>

/* private interface data
 */
struct _NAIImporterInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static guint st_initializations = 0;	/* interface initialization count */

static GType register_type( void );
static void  interface_base_init( NAIImporterInterface *klass );
static void  interface_base_finalize( NAIImporterInterface *klass );
static guint iimporter_get_version( const NAIImporter *instance );

#ifdef NA_ENABLE_DEPRECATED
static void  renumber_label_item( NAIImporterManageImportModeParms *parms );
#endif

/**
 * na_iimporter_get_type:
 *
 * Returns: the #GType type of this interface.
 */
GType
na_iimporter_get_type( void )
{
	static GType type = 0;

	if( !type ){
		type = register_type();
	}

	return( type );
}

/*
 * na_iimporter_register_type:
 *
 * Registers this interface.
 */
static GType
register_type( void )
{
	static const gchar *thisfn = "na_iimporter_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIImporterInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIImporter", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIImporterInterface *klass )
{
	static const gchar *thisfn = "na_iimporter_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass%p (%s)", thisfn, ( void * ) klass, G_OBJECT_CLASS_NAME( klass ));

		klass->private = g_new0( NAIImporterInterfacePrivate, 1 );

		klass->get_version = iimporter_get_version;
		klass->import_from_uri = NULL;
	}

	st_initializations += 1;
}

static void
interface_base_finalize( NAIImporterInterface *klass )
{
	static const gchar *thisfn = "na_iimporter_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

static guint
iimporter_get_version( const NAIImporter *instance )
{
	return( 1 );
}

/**
 * na_iimporter_import_from_uri:
 * @importer: this #NAIImporter instance.
 * @parms: a #NAIImporterImportFromUriParmsv2 structure.
 *
 * Tries to import a #NAObjectItem from the URI specified in @parms, returning
 * the result in <structfield>@parms->imported</structfield>.
 *
 * Note that, starting with &prodname; 3.2, the @parms argument is no more a
 * #NAIImporterImportFromUriParms pointer, but a #NAIImporterImportFromUriParmsv2
 * one.
 *
 * Returns: the return code of the operation.
 *
 * Since: 2.30
 */

guint
na_iimporter_import_from_uri( const NAIImporter *importer, NAIImporterImportFromUriParmsv2 *parms )
{
	static const gchar *thisfn = "na_iimporter_import_from_uri";
	guint code;

	g_return_val_if_fail( NA_IS_IIMPORTER( importer ), IMPORTER_CODE_PROGRAM_ERROR );
	g_return_val_if_fail( parms && parms->version == 2, IMPORTER_CODE_PROGRAM_ERROR );

	code = IMPORTER_CODE_NOT_WILLING_TO;

	g_debug( "%s: importer=%p (%s), parms=%p", thisfn,
			( void * ) importer, G_OBJECT_TYPE_NAME( importer), ( void * ) parms );

	if( NA_IIMPORTER_GET_INTERFACE( importer )->import_from_uri ){
		code = NA_IIMPORTER_GET_INTERFACE( importer )->import_from_uri( importer, parms );
	}

	return( code );
}

#ifdef NA_ENABLE_DEPRECATED
/**
 * na_iimporter_manage_import_mode:
 * @parms: a #NAIImporterManageImportModeParms struct.
 *
 * Returns: the #NAIImporterImportStatus status of the operation:
 *
 * <itemizedlist>
 *   <listitem>
 *     <para>
 *       IMPORTER_CODE_OK if we can safely insert the action:
 *     </para>
 *     <itemizedlist>
 *       <listitem>
 *         <para>the id doesn't already exist</para>
 *       </listitem>
 *       <listitem>
 *         <para>or the id already exists, but import mode is renumber</para>
 *       </listitem>
 *       <listitem>
 *         <para>or the id already exists, but import mode is override</para>
 *       </listitem>
 *     </itemizedlist>
 *   </listitem>
 *   <listitem>
 *     <para>
 *       IMPORTER_CODE_CANCELLED if user chooses to cancel the operation
 *     </para>
 *   </listitem>
 * </itemizedlist>
 *
 * Since: 2.30
 * Deprecated: 3.2
 */
guint
na_iimporter_manage_import_mode( NAIImporterManageImportModeParms *parms )
{
	static const gchar *thisfn = "na_iimporter_manage_import_mode";
	guint code;
	NAObjectItem *exists;
	guint mode;
	gchar *id;

	g_return_val_if_fail( parms->imported != NULL, IMPORTER_CODE_CANCELLED );

	code = IMPORTER_CODE_OK;
	exists = NULL;
	mode = 0;
	parms->exist = FALSE;
	parms->import_mode = parms->asked_mode;

	if( parms->check_fn ){
		exists = ( *parms->check_fn )( parms->imported, parms->check_fn_data );

	} else {
		renumber_label_item( parms );
		na_core_utils_slist_add_message( &parms->messages, "%s", _( "Item was renumbered because the caller did not provide any check function." ));
		parms->import_mode = IMPORTER_MODE_RENUMBER;
	}

	g_debug( "%s: exists=%p", thisfn, exists );

	if( exists ){
		parms->exist = TRUE;

		if( parms->asked_mode == IMPORTER_MODE_ASK ){
			if( parms->ask_fn ){
				mode = ( *parms->ask_fn )( parms->imported, exists, parms->ask_fn_data );

			} else {
				renumber_label_item( parms );
				na_core_utils_slist_add_message( &parms->messages, "%s", _( "Item was renumbered because the caller did not provide any ask user function." ));
				parms->import_mode = IMPORTER_MODE_RENUMBER;
			}

		} else {
			mode = parms->asked_mode;
		}
	}

	/* mode is only set if asked mode is ask user and an ask function was provided
	 * or if asked mode was not ask user
	 */
	if( mode ){
		parms->import_mode = mode;

		switch( mode ){
			case IMPORTER_MODE_RENUMBER:
				renumber_label_item( parms );
				if( parms->asked_mode == IMPORTER_MODE_ASK ){
					na_core_utils_slist_add_message( &parms->messages, "%s", _( "Item was renumbered due to user request." ));
				}
				break;

			case IMPORTER_MODE_OVERRIDE:
				if( parms->asked_mode == IMPORTER_MODE_ASK ){
					na_core_utils_slist_add_message( &parms->messages, "%s", _( "Existing item was overriden due to user request." ));
				}
				break;

			case IMPORTER_MODE_NO_IMPORT:
			default:
				id = na_object_get_id( parms->imported );
				na_core_utils_slist_add_message( &parms->messages, _( "Item %s already exists." ), id );
				if( parms->asked_mode == IMPORTER_MODE_ASK ){
					na_core_utils_slist_add_message( &parms->messages, "%s", _( "Import was canceled due to user request." ));
				}
				g_free( id );
				code = IMPORTER_CODE_CANCELLED;
		}
	}

	return( code );
}

/*
 * renumber the item, and set a new label
 */
static void
renumber_label_item( NAIImporterManageImportModeParms *parms )
{
	gchar *label, *tmp;

	na_object_set_new_id( parms->imported, NULL );

	label = na_object_get_label( parms->imported );

	/* i18n: the action has been renumbered during import operation */
	tmp = g_strdup_printf( "%s %s", label, _( "(renumbered)" ));

	na_object_set_label( parms->imported, tmp );

	g_free( tmp );
	g_free( label );
}
#endif /* NA_ENABLE_DEPRECATED */
