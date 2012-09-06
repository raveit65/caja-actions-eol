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

#ifndef __CAJA_ACTIONS_API_NA_IEXPORTER_H__
#define __CAJA_ACTIONS_API_NA_IEXPORTER_H__

/**
 * SECTION: na_iexporter
 * @short_description: #NAIExporter interface definition.
 * @include: caja-actions/na-iexporter.h
 *
 * The #NAIExporter interface exports items to the outside world.
 *
 * Caja-Actions v 2.30 - API version:  1
 */

#include "na-object-item.h"

G_BEGIN_DECLS

#define NA_IEXPORTER_TYPE						( na_iexporter_get_type())
#define NA_IEXPORTER( instance )				( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_IEXPORTER_TYPE, NAIExporter ))
#define NA_IS_IEXPORTER( instance )				( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_IEXPORTER_TYPE ))
#define NA_IEXPORTER_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_IEXPORTER_TYPE, NAIExporterInterface ))

typedef struct NAIExporter                 NAIExporter;
typedef struct NAIExporterFileParms        NAIExporterFileParms;
typedef struct NAIExporterBufferParms      NAIExporterBufferParms;

typedef struct NAIExporterInterfacePrivate NAIExporterInterfacePrivate;

/* When listing available export formats, the instance returns a GList
 * of these structures.
 * This structure must be implemented by each #NAIExporter implementation
 * (see e.g. io-xml/naxml-formats.c)
 */
typedef struct {
	gchar *format;					/* format identifier (ascii) */
	gchar *label;					/* short label to be displayed in dialog (UTF-8 localized) */
	gchar *description;				/* full description of the format (UTF-8 localized)
									 * mainly used in the export assistant */
}
	NAIExporterFormat;

typedef struct {
	GTypeInterface               parent;
	NAIExporterInterfacePrivate *private;

	/**
	 * get_version:
	 * @instance: this #NAIExporter instance.
	 *
	 * Returns: the version of this interface supported by the I/O provider.
	 *
	 * Defaults to 1.
	 */
	guint                     ( *get_version )( const NAIExporter *instance );

	/**
	 * get_name:
	 * @instance: this #NAIExporter instance.
	 *
	 * Returns: the name to be displayed for this instance, as a
	 * newly allocated string which should be g_free() by the caller.
	 */
	gchar *                   ( *get_name )   ( const NAIExporter *instance );

	/**
	 * get_formats:
	 * @instance: this #NAIExporter instance.
	 *
	 * Returns: a list of #NAIExporterFormat structures which describe the
	 * formats supported by @instance.
	 *
	 * Defaults to %NULL (no format at all).
	 *
	 * The returned list is owned by the @instance. It must not be
	 * released by the caller.
	 *
	 * To avoid any collision, the format id is allocated by the
	 * Caja-Actions maintainer team. If you wish develop a new
	 * export format, and so need a new format id, please contact the
	 * maintainers (see #caja-actions.doap).
	 */
	const NAIExporterFormat * ( *get_formats )( const NAIExporter *instance );

	/**
	 * to_file:
	 * @instance: this #NAIExporter instance.
	 * @parms: a #NAIExporterFileParms structure.
	 *
	 * Exports the specified 'exported' to the target 'folder' in the required
	 * 'format'.
	 *
	 * Returns: the status of the operation.
	 */
	guint                     ( *to_file )    ( const NAIExporter *instance, NAIExporterFileParms *parms );

	/**
	 * to_buffer:
	 * @instance: this #NAIExporter instance.
	 * @parms: a #NAIExporterFileParms structure.
	 *
	 * Exports the specified 'exported' to a newly allocated 'buffer' in
	 * the required 'format'. The allocated 'buffer' should be g_free()
	 * by the caller.
	 *
	 * Returns: the status of the operation.
	 */
	guint                     ( *to_buffer )  ( const NAIExporter *instance, NAIExporterBufferParms *parms );
}
	NAIExporterInterface;

/* The reasons for which an item may not have been exported
 */
enum {
	NA_IEXPORTER_CODE_OK = 0,
	NA_IEXPORTER_CODE_INVALID_ITEM,
	NA_IEXPORTER_CODE_INVALID_TARGET,
	NA_IEXPORTER_CODE_INVALID_FORMAT,
	NA_IEXPORTER_CODE_UNABLE_TO_WRITE,
	NA_IEXPORTER_CODE_ERROR,
};

/* parameters via a structure
 * ... when exporting to a file
 */
struct NAIExporterFileParms {
	guint         version;				/* i 1: version of this structure */
	NAObjectItem *exported;				/* i 1: exported NAObjectItem-derived object */
	gchar        *folder;				/* i 1: URI of the target folder */
	GQuark        format;				/* i 1: export format as a GQuark */
	gchar        *basename;				/*  o1: basename of the exported file */
	GSList       *messages;				/* io1: a #GSList list of localized strings;
										 *       the provider may append messages to this list,
										 *       but shouldn't reinitialize it. */
};

/* parameters via a structure
 * ... when exporting to a buffer
 */
struct NAIExporterBufferParms {
	guint         version;				/* i 1: version of this structure */
	NAObjectItem *exported;				/* i 1: exported NAObjectItem-derived object */
	GQuark        format;				/* i 1: export format as a GQuark */
	gchar        *buffer;				/*  o1: buffer which contains the exported object */
	GSList       *messages;				/* io1: a #GSList list of localized strings;
										 *       the provider may append messages to this list,
										 *       but shouldn't reinitialize it. */
};

GType na_iexporter_get_type( void );

G_END_DECLS

#endif /* __CAJA_ACTIONS_API_NA_IEXPORTER_H__ */
