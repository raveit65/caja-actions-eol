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

#ifndef __CACT_EXPORT_FORMAT_H__
#define __CACT_EXPORT_FORMAT_H__

/**
 * SECTION: cact_export_format
 * @short_description: Displays the list of available export formats.
 * @include: cact/cact-export-format.h
 */

#include <gtk/gtk.h>

#include <core/na-export-format.h>
#include <core/na-pivot.h>

G_BEGIN_DECLS

enum {
	/* ask for export format dialog box
	 * only display the 'ask_label' short export format label
	 * do not display the full description
	 * do not propose the 'Ask me' choice
	 */
	EXPORT_FORMAT_DISPLAY_ASK = 1,

	/* export assistant
	 * display the assistant short label in bold
	 * display the full description
	 * propose the 'Ask me' choice
	 */
	EXPORT_FORMAT_DISPLAY_ASSISTANT,

	/* preferences editor
	 * display the assistant short label
	 * do not display the full description
	 * propose the 'Ask me' choice
	 */
	EXPORT_FORMAT_DISPLAY_PREFERENCES,
};

void            cact_export_format_init_display( const NAPivot *pivot, GtkWidget *container, guint mode );
void            cact_export_format_select( const GtkWidget *container, GQuark format );
NAExportFormat *cact_export_format_get_selected( const GtkWidget *container );

G_END_DECLS

#endif /* __CACT_EXPORT_FORMAT_H__ */
