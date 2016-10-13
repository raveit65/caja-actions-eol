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
#include <gtk/gtk.h>
#include <libintl.h>

#include <api/na-iexporter.h>

#include "cadp-formats.h"

typedef struct {
	gchar *format;
	gchar *label;
	gchar *description;
	gchar *image;
}
	CappExportFormat;

static CappExportFormat cadp_formats[] = {

	/* DESKTOP_V1: the initial desktop format as described in
	 * http://www.caja-actions.org/?q=node/377
	 */
	{ CADP_FORMAT_DESKTOP_V1,
			N_( "Export as a ._desktop file" ),
			N_( "This format has been introduced with v 3.0 series. " \
				"and should be your newly preferred format when exporting items.\n" \
				"It let you easily share your actions with the whole world, " \
				"including with users of other desktop environments, " \
				"as long as their own application implements the DES-EMA specification " \
				"which describes this format.\n" \
				"The exported .desktop file may later be imported via :\n" \
				"- Import assistant of the Caja-Actions Configuration Tool,\n" \
				"- drag-n-drop into the Caja-Actions Configuration Tool,\n" \
				"- or by copying it into a XDG_DATA_DIRS/file-manager/actions directory." ),
			"export-desktop.png" },

	{ NULL }
};

#if 0
static void on_pixbuf_finalized( const NAIExporter* exporter, GObject *pixbuf );
#endif

/**
 * cadp_formats_get_formats:
 * @exporter: this #NAIExporter provider.
 *
 * Returns: a #GList of the #NAIExporterFormatv2 supported export formats.
 *
 * This list should be cadp_formats_free_formats() by the caller.
 *
 * Since: 3.2
 */
GList *
cadp_formats_get_formats( const NAIExporter* exporter )
{
#if 0
	static const gchar *thisfn = "cadp_formats_get_formats";
#endif
	GList *str_list;
	NAIExporterFormatv2 *str;
	guint i;
	gint width, height;
	gchar *fname;

	str_list = NULL;

	if( !gtk_icon_size_lookup( GTK_ICON_SIZE_DIALOG, &width, &height )){
		width = height = 48;
	}

	for( i = 0 ; cadp_formats[i].format ; ++i ){
		str = g_new0( NAIExporterFormatv2, 1 );
		str->version = 2;
		str->provider = NA_IEXPORTER( exporter );
		str->format = g_strdup( cadp_formats[i].format );
		str->label = g_strdup( gettext( cadp_formats[i].label ));
		str->description = g_strdup( gettext( cadp_formats[i].description ));
		if( cadp_formats[i].image ){
			fname = g_strdup_printf( "%s/%s", PROVIDER_DATADIR, cadp_formats[i].image );
			str->pixbuf = gdk_pixbuf_new_from_file_at_size( fname, width, height, NULL );
			g_free( fname );
#if 0
			/* do not set weak reference on a graphical object provided by a plugin
			 * if the windows does not have its own builder, it may happens that the
			 * graphical object be finalized when destroying toplevels at common
			 * builder finalization time, and so after the plugins have been shutdown
			 */
			if( str->pixbuf ){
				g_debug( "%s: allocating pixbuf at %p", thisfn, str->pixbuf );
				g_object_weak_ref( G_OBJECT( str->pixbuf ), ( GWeakNotify ) on_pixbuf_finalized, ( gpointer ) exporter );
			}
#endif
		}
		str_list = g_list_prepend( str_list, str );
	}

	return( str_list );
}

#if 0
static void
on_pixbuf_finalized( const NAIExporter* exporter, GObject *pixbuf )
{
	g_debug( "cadp_formats_on_pixbuf_finalized: exporter=%p, pixbuf=%p", ( void * ) exporter, ( void * ) pixbuf );
}
#endif

/**
 * cadp_formats_free_formats:
 * @formats: a #GList to be freed.
 *
 * Returns: a #GList of the #NAIExporterFormatv2 supported export formats.
 *
 * This list should be cadp_format_free_formats() by the caller.
 *
 * Since: 3.2
 */
void
cadp_formats_free_formats( GList *formats )
{
	GList *is;
	NAIExporterFormatv2 *str;

	for( is = formats ; is ; is = is->next ){
		str = ( NAIExporterFormatv2 * ) is->data;
		g_free( str->format );
		g_free( str->label );
		g_free( str->description );
		if( str->pixbuf ){
			g_object_unref( str->pixbuf );
		}
		g_free( str );
	}

	g_list_free( formats );
}
