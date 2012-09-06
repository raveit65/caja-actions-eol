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

#include <core/na-exporter.h>
#include <core/na-export-format.h>

#include "cact-iprefs.h"
#include "cact-export-format.h"

#define EXPORT_FORMAT_PROP_OBJECT		"cact-export-format-prop-object"
#define EXPORT_FORMAT_PROP_BUTTON		"cact-export-format-prop-button"

#define ASKME_LABEL						N_( "_Ask me" )
#define ASKME_DESCRIPTION				N_( "You will be asked for the format to choose each time an item is about to be exported." )

static const NAIExporterFormat st_ask_str = { "Ask", ASKME_LABEL, ASKME_DESCRIPTION };

static void draw_in_vbox( const NAExportFormat *format, GtkWidget *vbox, guint mode, gint id );
static void format_weak_notify( NAExportFormat *format, GObject *vbox );
static void select_default_iter( GtkWidget *widget, void *quark_ptr );
static void get_selected_iter( GtkWidget *widget, NAExportFormat **format );

/**
 * cact_export_format_init_display:
 * @pivot: the #NAPivot instance.
 * @vbox: the #GtkVBox in which the display must be drawn.
 * @mode: the type of the display.
 *
 * Displays the available export formats in the VBox.
 * Should only be called once per dialog box instance.
 */
void
cact_export_format_init_display( const NAPivot *pivot, GtkWidget *vbox, guint mode )
{
	static const gchar *thisfn = "cact_export_format_init_display";
	GList *formats, *ifmt;
	NAExportFormat *format;

	formats = na_exporter_get_formats( pivot );

	for( ifmt = formats ; ifmt ; ifmt = ifmt->next ){
		draw_in_vbox( NA_EXPORT_FORMAT( ifmt->data ), vbox, mode, -1 );
	}

	na_exporter_free_formats( formats );

	switch( mode ){

		case EXPORT_FORMAT_DISPLAY_PREFERENCES:
		case EXPORT_FORMAT_DISPLAY_ASSISTANT:
			format = na_export_format_new( &st_ask_str, NULL );
			draw_in_vbox( format, vbox, mode, IPREFS_EXPORT_FORMAT_ASK );
			g_object_unref( format );
			break;

		/* this is the mode to be used when we are about to export an item
		 * and the user preference is 'Ask me'; obviously, we don't propose
		 * here to ask him another time :)
		 */
		case EXPORT_FORMAT_DISPLAY_ASK:
			break;

		default:
			g_warning( "%s: mode=%d: unknown mode", thisfn, mode );
	}
}

/*
 * container
 *  +- vbox
 *  |   +- radio button
 *  |   +- hbox
 *  |   |   +- description
 */
static void
draw_in_vbox( const NAExportFormat *format, GtkWidget *container, guint mode, gint id )
{
	static GtkRadioButton *first_button = NULL;
	GtkVBox *vbox;
	gchar *description;
	GtkHBox *hbox;
	GtkRadioButton *button;
	guint size, spacing;
	gchar *markup, *label;
	GtkLabel *desc_label;

	vbox = GTK_VBOX( gtk_vbox_new( FALSE, 0 ));
	gtk_box_pack_start( GTK_BOX( container ), GTK_WIDGET( vbox ), FALSE, TRUE, 0 );
	description = na_export_format_get_description( format );
	g_object_set( G_OBJECT( vbox ), "tooltip-text", description, NULL );
	g_object_set( G_OBJECT( vbox ), "spacing", 6, NULL );

	button = GTK_RADIO_BUTTON( gtk_radio_button_new( NULL ));
	if( first_button ){
		g_object_set( G_OBJECT( button ), "group", first_button, NULL );
	} else {
		first_button = button;
	}
	gtk_box_pack_start( GTK_BOX( vbox ), GTK_WIDGET( button ), FALSE, TRUE, 0 );

	label = NULL;
	markup = NULL;
	switch( mode ){

		case EXPORT_FORMAT_DISPLAY_ASK:
		case EXPORT_FORMAT_DISPLAY_PREFERENCES:
		case EXPORT_FORMAT_DISPLAY_ASSISTANT:
			label = na_export_format_get_label( format );
			markup = g_markup_printf_escaped( "%s", label );
			gtk_button_set_label( GTK_BUTTON( button ), label );
			g_object_set( G_OBJECT( button ), "use_underline", TRUE, NULL );
			break;

		/* this work fine, but it appears that this is not consistant with import assistant */
		/*case EXPORT_FORMAT_DISPLAY_ASSISTANT:
			radio_label = GTK_LABEL( gtk_label_new( NULL ));
			label = na_export_format_get_label( format );
			markup = g_markup_printf_escaped( "<b>%s</b>", label );
			gtk_label_set_markup( radio_label, markup );
			gtk_container_add( GTK_CONTAINER( button ), GTK_WIDGET( radio_label ));
			break;*/
	}

	desc_label = NULL;
	switch( mode ){

		case EXPORT_FORMAT_DISPLAY_ASSISTANT:
			hbox = GTK_HBOX( gtk_hbox_new( TRUE, 0 ));
			gtk_box_pack_start( GTK_BOX( vbox ), GTK_WIDGET( hbox ), FALSE, TRUE, 0 );

			gtk_widget_style_get( GTK_WIDGET( button ), "indicator-size", &size, NULL );
			gtk_widget_style_get( GTK_WIDGET( button ), "indicator-spacing", &spacing, NULL );
			size += 2*spacing;

			desc_label = GTK_LABEL( gtk_label_new( description ));
			g_object_set( G_OBJECT( desc_label ), "xpad", size, NULL );
			g_object_set( G_OBJECT( desc_label ), "xalign", 0, NULL );
			gtk_box_pack_start( GTK_BOX( hbox ), GTK_WIDGET( desc_label ), TRUE, TRUE, 4 );
			break;
	}

	g_object_set_data( G_OBJECT( vbox ), EXPORT_FORMAT_PROP_BUTTON, button );
	g_object_set_data( G_OBJECT( vbox ), EXPORT_FORMAT_PROP_OBJECT, g_object_ref(( gpointer ) format ));
	g_object_weak_ref( G_OBJECT( vbox ), ( GWeakNotify ) format_weak_notify, ( gpointer ) format );

	g_free( markup );
	g_free( label );
	g_free( description );
}

static void
format_weak_notify( NAExportFormat *format, GObject *vbox )
{
	static const gchar *thisfn = "cact_export_format_weak_notify";

	g_debug( "%s: format=%p (%s), vbox=%p",
			thisfn, ( void * ) format, G_OBJECT_TYPE_NAME( format ), ( void * ) vbox );

	g_object_unref( format );
}

/**
 * cact_export_format_select:
 * @container: the #GtkVBox in which the display has been drawn.
 * @format: the #GQuark which must be used as a default value.
 *
 * Select the default value.
 */
void
cact_export_format_select( const GtkWidget *container, GQuark format )
{
	void *quark_ptr;

	quark_ptr = GUINT_TO_POINTER( format );
	gtk_container_foreach( GTK_CONTAINER( container ), ( GtkCallback ) select_default_iter, quark_ptr );
}

static void
select_default_iter( GtkWidget *widget, void *quark_ptr )
{
	NAExportFormat *format;
	GQuark format_quark;
	GtkRadioButton *button;

	format_quark = ( GQuark ) GPOINTER_TO_UINT( quark_ptr );
	format = NA_EXPORT_FORMAT( g_object_get_data( G_OBJECT( widget ), EXPORT_FORMAT_PROP_OBJECT ));

	if( na_export_format_get_quark( format ) == format_quark ){
		button = GTK_RADIO_BUTTON( g_object_get_data( G_OBJECT( widget ), EXPORT_FORMAT_PROP_BUTTON ));
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );
	}
}

/**
 * cact_export_format_get_selected:
 * @container: the #GtkVBox in which the display has been drawn.
 *
 * Returns: the currently selected value, as a #NAExportFormat object.
 *
 * The returned #NAExportFormat is owned by #CactExportFormat, and
 * should not be released by the caller.
 */
NAExportFormat *
cact_export_format_get_selected( const GtkWidget *container )
{
	NAExportFormat *format;

	format = NULL;
	gtk_container_foreach( GTK_CONTAINER( container ), ( GtkCallback ) get_selected_iter, &format );
	g_debug( "cact_export_format_get_selected: format=%p", ( void * ) format );

	return( format );
}

static void
get_selected_iter( GtkWidget *widget, NAExportFormat **format )
{
	GtkRadioButton *button;

	if( !( *format  )){
		button = GTK_RADIO_BUTTON( g_object_get_data( G_OBJECT( widget ), EXPORT_FORMAT_PROP_BUTTON ));
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
			*format = NA_EXPORT_FORMAT( g_object_get_data( G_OBJECT( widget ), EXPORT_FORMAT_PROP_OBJECT ));
		}
	}
}
