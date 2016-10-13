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

#ifndef __CORE_NA_ABOUT_H__
#define __CORE_NA_ABOUT_H__

/* @title NAAbout
 * @short_description: The #NAAbout API
 * @include: runtime/na-about.h
 *
 * These functions displays the 'About Caja-Actions' dialog box,
 * and provides contant information about the application.
 */

#include <gtk/gtk.h>

G_BEGIN_DECLS

void         na_about_display( GtkWindow *parent );

gchar       *na_about_get_application_name( void );
const gchar *na_about_get_icon_name( void );
gchar       *na_about_get_copyright( gboolean console );

G_END_DECLS

#endif /* __CORE_NA_IABOUT_H__ */
