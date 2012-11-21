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
#include <libintl.h>

#include "na-about.h"

/*
 * na_about_display:
 * @toplevel: the parent window.
 *
 * Displays the About dialog.
 */
void
na_about_display( GtkWindow *toplevel )
{
	gchar *application_name, *copyright;
	int i;
	GString *license_i18n;

	static const gchar *artists[] = {
		"Ulisse Perusin <uli.peru@gmail.com>",
		"DragonArtz - http://www.dragonartz.net/",
		NULL
	};

	static const gchar *authors[] = {
		"Frederic Ruaudel <grumz@grumz.net>",
		"Rodrigo Moya <rodrigo@mate-db.org>",
		"Pierre Wieser <pwieser@trychlos.org>",
		NULL
	};

	static const gchar *documenters[] = {
		NULL
	};

	static gchar *license[] = {
		N_( "Caja-Actions Configuration Tool is free software; you can "
			"redistribute it and/or modify it under the terms of the GNU General "
			"Public License as published by the Free Software Foundation; either "
			"version 2 of the License, or (at your option) any later version." ),
		N_( "Caja-Actions Configuration Tool is distributed in the hope that it "
			"will be useful, but WITHOUT ANY WARRANTY; without even the implied "
			"warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See "
			"the GNU General Public License for more details." ),
		N_( "You should have received a copy of the GNU General Public License along "
			"with Caja-Actions Configuration Tool ; if not, write to the Free "
			"Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, "
			"MA 02110-1301, USA." ),
		NULL
	};

	application_name = na_about_get_application_name();
	copyright = na_about_get_copyright( FALSE );

	i = 0;
	license_i18n = g_string_new( "" );
	while( license[i] ){
		g_string_append_printf( license_i18n, "%s\n\n", gettext( license[i] ));
		i += 1;
	}

	gtk_show_about_dialog( toplevel,
			"artists", artists,
			"authors", authors,
			/* i10n: "Caja actions" here is not a typo; this does not refer to the
			 * Caja-Actions product, but to the actions which are to be added to Caja */
			"comments", _( "A graphical interface to create and edit your Caja actions." ),
			"copyright", copyright,
			"documenters", documenters,
			"license", license_i18n->str,
			"logo-icon-name", na_about_get_icon_name(),
			"program-name", application_name,
			"translator-credits", _( "The MATE Translation Project <mate-i18n@gnome.org>" ),
			"version", PACKAGE_VERSION,
			"website", "http://www.caja-actions.org",
			"wrap-license", TRUE,
			NULL );

	g_free( application_name );
	g_string_free( license_i18n, TRUE );
	g_free( copyright );
}

/*
 * na_about_get_application_name:
 *
 * Returns: the name of the application, as a newly allocated string
 * which should be g_free() by the caller.
 */
gchar *
na_about_get_application_name( void )
{
	/* i18n: title of the About dialog box, when seen from Caja file manager */
	return( g_strdup( _( "Caja-Actions" )));
}

/*
 * na_about_get_icon_name:
 *
 * Returns: the name of the default icon for the application.
 *
 * This name is owned by the package, and should not be released by the caller.
 *
 * cf. Makefile: PACKAGE = caja-actions
 */
const gchar *
na_about_get_icon_name( void )
{
	return( PACKAGE );
}

/*
 * na_about_get_copyright:
 * @console: whether the string is to be printed on a console.
 *
 * Returns: the copyright string, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
na_about_get_copyright( gboolean console )
{
	gchar *copyright;
	gchar *symbol;

	symbol = g_strdup( console ? "(C)" : "\xc2\xa9");

	copyright = g_strdup_printf(
			_( "Copyright %s 2005 The MATE Foundation\n"
				"Copyright %s 2006-2008 Frederic Ruaudel <grumz@grumz.net>\n"
				"Copyright %s 2009-2012 Pierre Wieser <pwieser@trychlos.org>" ), symbol, symbol, symbol );

	g_free( symbol );

	return( copyright );
}
