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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "na-iabout.h"

/* private interface data
 */
struct NAIAboutInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;

static GType      register_type( void );
static void       interface_base_init( NAIAboutInterface *klass );
static void       interface_base_finalize( NAIAboutInterface *klass );

static gchar     *v_get_application_name( NAIAbout *instance );
static GtkWindow *v_get_toplevel( NAIAbout *instance );

GType
na_iabout_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = register_type();
	}

	return( iface_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "na_iabout_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NAIAboutInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NAIAbout", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NAIAboutInterface *klass )
{
	static const gchar *thisfn = "na_iabout_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NAIAboutInterfacePrivate, 1 );

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NAIAboutInterface *klass )
{
	static const gchar *thisfn = "na_iabout_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
}

static gchar *
v_get_application_name( NAIAbout *instance )
{
	if( NA_IABOUT_GET_INTERFACE( instance )->get_application_name ){
		return( NA_IABOUT_GET_INTERFACE( instance )->get_application_name( instance ));
	}

	return( NULL );
}

static GtkWindow *
v_get_toplevel( NAIAbout *instance )
{
	if( NA_IABOUT_GET_INTERFACE( instance )->get_toplevel ){
		return( NA_IABOUT_GET_INTERFACE( instance )->get_toplevel( instance ));
	}

	return( NULL );
}

/**
 * na_iabout_display:
 * @instance: the #NAIAbout implementor.
 *
 * Displays the About dialog.
 */
void
na_iabout_display( NAIAbout *instance )
{
	static const gchar *thisfn = "na_iabout_display";
	gchar *application_name;
	gchar *icon_name, *license_i18n, *copyright;
	GtkWindow *toplevel;

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
		N_( "Caja Actions Configuration Tool is free software; you can "
			"redistribute it and/or modify it under the terms of the GNU General "
			"Public License as published by the Free Software Foundation; either "
			"version 2 of the License, or (at your option) any later version." ),
		N_( "Caja Actions Configuration Tool is distributed in the hope that it "
			"will be useful, but WITHOUT ANY WARRANTY; without even the implied "
			"warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See "
			"the GNU General Public License for more details." ),
		N_( "You should have received a copy of the GNU General Public License along "
			"with Caja Actions Configuration Tool ; if not, write to the Free "
			"Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, "
			"MA 02110-1301, USA." ),
		NULL
	};

	g_debug( "%s: instance=%p (%s)", thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ));
	g_return_if_fail( NA_IS_IABOUT( instance ));

	if( st_initialized && !st_finalized ){

		application_name = v_get_application_name( instance );
		toplevel = v_get_toplevel( instance );

		icon_name = na_iabout_get_icon_name();
		copyright = na_iabout_get_copyright( FALSE );
		license_i18n = g_strjoinv( "\n\n", license );

		gtk_show_about_dialog( toplevel,
				"artists", artists,
				"authors", authors,
				"comments", _( "A graphical interface to create and edit your Caja actions." ),
				"copyright", copyright,
				"documenters", documenters,
				"license", license_i18n,
				"logo-icon-name", icon_name,
				"program-name", application_name,
				"translator-credits", _( "The MATE Translation Project <mate-i18n@gnome.org>" ),
				"version", PACKAGE_VERSION,
				"website", "http://www.caja-actions.org",
				"wrap-license", TRUE,
				NULL );

		g_free( application_name );
		g_free( license_i18n );
		g_free( copyright );
		g_free( icon_name );
	}
}

/**
 * na_iabout_get_icon_name:
 *
 * Returns: the name of the default icon for the application, as a newly
 * allocated string which should be g_free() by the caller.
 */
gchar *
na_iabout_get_icon_name( void )
{
	return( g_strdup( PACKAGE ));
}

/**
 * na_iabout_get_copyright:
 * @console: whether the string is to be printed on a console.
 *
 * Returns: the copyright string, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
na_iabout_get_copyright( gboolean console )
{
	gchar *copyright;
	gchar *symbol;

	symbol = g_strdup( console ? "(C)" : "\xc2\xa9");

	copyright = g_strdup_printf(
			_( "Copyright %s 2005 The MATE Foundation\n"
				"Copyright %s 2006, 2007, 2008 Frederic Ruaudel <grumz@grumz.net>\n"
				"Copyright %s 2009, 2010 Pierre Wieser <pwieser@trychlos.org>" ), symbol, symbol, symbol );

	g_free( symbol );

	return( copyright );
}
