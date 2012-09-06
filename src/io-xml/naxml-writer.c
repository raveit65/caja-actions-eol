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

#include <gio/gio.h>
#include <libxml/tree.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-data-types.h>
#include <api/na-object-api.h>
#include <api/na-ifactory-provider.h>
#include <api/na-iio-provider.h>

#include <io-mateconf/nagp-keys.h>

#include "naxml-formats.h"
#include "naxml-keys.h"
#include "naxml-writer.h"

typedef struct ExportFormatFn ExportFormatFn;

/* private class data
 */
struct NAXMLWriterClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NAXMLWriterPrivate {
	gboolean         dispose_has_run;
	NAIExporter     *provider;
	NAObjectItem    *exported;
	GSList          *messages;

	/* positionning these at document level
	 */
	xmlDocPtr        doc;
	ExportFormatFn  *fn_str;
	gchar           *buffer;

	xmlNodePtr       root_node;
	xmlNodePtr       list_node;

	/* nodes created in write_data_schema_v2(), used in write_data_schema_v1()
	 */
	xmlNodePtr       schema_node;
	xmlNodePtr       locale_node;
};

/* the association between an export format and the functions
 */
struct ExportFormatFn {
	gchar  *format;
	gchar  *root_node;
	gchar  *list_node;
	void ( *write_list_attribs_fn )( NAXMLWriter *, const NAObjectItem * );
	gchar  *element_node;
	void ( *write_data_fn )( NAXMLWriter *, const NAObjectId *, const NADataBoxed * );
	void ( *write_type_fn )( NAXMLWriter *, const NAObjectItem *, const NADataDef *, const gchar * );
};

static GObjectClass *st_parent_class = NULL;

static GType           register_type( void );
static void            class_init( NAXMLWriterClass *klass );
static void            instance_init( GTypeInstance *instance, gpointer klass );
static void            instance_dispose( GObject *object );
static void            instance_finalize( GObject *object );

static void            write_data_schema_v1( NAXMLWriter *writer, const NAObjectId *object, const NADataBoxed *boxed );
static void            write_data_schema_v1_element( NAXMLWriter *writer, const NADataDef *def );
static void            write_type_schema_v1( NAXMLWriter *writer, const NAObjectItem *object, const NADataDef *def, const gchar *value );
static void            write_data_schema_v2( NAXMLWriter *writer, const NAObjectId *object, const NADataBoxed *boxed );
static void            write_data_schema_v2_element( NAXMLWriter *writer, const NADataDef *def, const gchar *object_id, const gchar *value_str );
static void            write_type_schema_v2( NAXMLWriter *writer, const NAObjectItem *object, const NADataDef *def, const gchar *value );
static void            write_list_attribs_dump( NAXMLWriter *writer, const NAObjectItem *object );
static void            write_data_dump( NAXMLWriter *writer, const NAObjectId *object, const NADataBoxed *boxed );
static void            write_data_dump_element( NAXMLWriter *writer, const NADataDef *def, const NADataBoxed *boxed, const gchar *entry, const gchar *value_str );
static void            write_type_dump( NAXMLWriter *writer, const NAObjectItem *object, const NADataDef *def, const gchar *value );

static xmlDocPtr       build_xml_doc( NAXMLWriter *writer );
static ExportFormatFn *find_export_format_fn( GQuark format );
static gchar          *get_output_fname( const NAObjectItem *item, const gchar *folder, GQuark format );
static void            output_xml_to_file( const gchar *xml, const gchar *filename, GSList **msg );
static void            write_type( NAXMLWriter *writer, NAObjectItem *object, const NADataGroup *groups );
static guint           writer_to_buffer( NAXMLWriter *writer );

static ExportFormatFn st_export_format_fn[] = {

	{ NAXML_FORMAT_MATECONF_SCHEMA_V1,
					NAXML_KEY_SCHEMA_ROOT,
					NAXML_KEY_SCHEMA_LIST,
					NULL,
					NAXML_KEY_SCHEMA_NODE,
					write_data_schema_v1,
					write_type_schema_v1 },

	{ NAXML_FORMAT_MATECONF_SCHEMA_V2,
					NAXML_KEY_SCHEMA_ROOT,
					NAXML_KEY_SCHEMA_LIST,
					NULL,
					NAXML_KEY_SCHEMA_NODE,
					write_data_schema_v2,
					write_type_schema_v2 },

	{ NAXML_FORMAT_MATECONF_ENTRY,
					NAXML_KEY_DUMP_ROOT,
					NAXML_KEY_DUMP_LIST,
					write_list_attribs_dump,
					NAXML_KEY_DUMP_NODE,
					write_data_dump,
					write_type_dump },

	{ NULL }
};

GType
naxml_writer_get_type( void )
{
	static GType object_type = 0;

	if( !object_type ){
		object_type = register_type();
	}

	return( object_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "naxml_writer_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAXMLWriterClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAXMLWriter ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NAXMLWriter", &info, 0 );

	return( type );
}

static void
class_init( NAXMLWriterClass *klass )
{
	static const gchar *thisfn = "naxml_writer_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAXMLWriterClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "naxml_writer_instance_init";
	NAXMLWriter *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_return_if_fail( NAXML_IS_WRITER( instance ));
	self = NAXML_WRITER( instance );

	self->private = g_new0( NAXMLWriterPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "naxml_writer_instance_dispose";
	NAXMLWriter *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NAXML_IS_WRITER( object ));
	self = NAXML_WRITER( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	NAXMLWriter *self;

	g_return_if_fail( NAXML_IS_WRITER( object ));
	self = NAXML_WRITER( object );

	/* do not release self->private->buffer as the pointer has been
	 * returned to the caller
	 */

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * naxml_writer_export_to_buffer:
 * @instance: this #NAIExporter instance.
 * @parms: a #NAIExporterBufferParms structure.
 *
 * Export the specified 'item' to a newly allocated buffer.
 */
guint
naxml_writer_export_to_buffer( const NAIExporter *instance, NAIExporterBufferParms *parms )
{
	static const gchar *thisfn = "naxml_writer_export_to_buffer";
	NAXMLWriter *writer;
	guint code;

	g_debug( "%s: instance=%p, parms=%p", thisfn, ( void * ) instance, ( void * ) parms );

	code = NA_IEXPORTER_CODE_OK;

	if( !parms->exported || !NA_IS_OBJECT_ITEM( parms->exported )){
		code = NA_IEXPORTER_CODE_INVALID_ITEM;
	}

	if( code == NA_IEXPORTER_CODE_OK ){
		writer = NAXML_WRITER( g_object_new( NAXML_WRITER_TYPE, NULL ));

		writer->private->provider = ( NAIExporter * ) instance;
		writer->private->exported = parms->exported;
		writer->private->messages = parms->messages;
		writer->private->fn_str = find_export_format_fn( parms->format );
		writer->private->buffer = NULL;

		if( !writer->private->fn_str ){
			code = NA_IEXPORTER_CODE_INVALID_FORMAT;

		} else {
			code = writer_to_buffer( writer );
			if( code == NA_IEXPORTER_CODE_OK ){
				parms->buffer = writer->private->buffer;
			}
		}

		g_object_unref( writer );
	}

	g_debug( "%s: returning code=%u", thisfn, code );
	return( code );
}

/**
 * naxml_writer_export_to_file:
 * @instance: this #NAIExporter instance.
 * @parms: a #NAIExporterFileParms structure.
 *
 * Export the specified 'item' to a newly created file.
 */
guint
naxml_writer_export_to_file( const NAIExporter *instance, NAIExporterFileParms *parms )
{
	static const gchar *thisfn = "naxml_writer_export_to_file";
	NAXMLWriter *writer;
	gchar *filename;
	guint code;

	g_debug( "%s: instance=%p, parms=%p", thisfn, ( void * ) instance, ( void * ) parms );

	code = NA_IEXPORTER_CODE_OK;

	if( !parms->exported || !NA_IS_OBJECT_ITEM( parms->exported )){
		code = NA_IEXPORTER_CODE_INVALID_ITEM;
	}

	if( code == NA_IEXPORTER_CODE_OK ){
		writer = NAXML_WRITER( g_object_new( NAXML_WRITER_TYPE, NULL ));

		writer->private->provider = ( NAIExporter * ) instance;
		writer->private->exported = parms->exported;
		writer->private->messages = parms->messages;
		writer->private->fn_str = find_export_format_fn( parms->format );
		writer->private->buffer = NULL;

		if( !writer->private->fn_str ){
			code = NA_IEXPORTER_CODE_INVALID_FORMAT;

		} else {
			code = writer_to_buffer( writer );

			if( code == NA_IEXPORTER_CODE_OK ){
				filename = get_output_fname( parms->exported, parms->folder, parms->format );

				if( filename ){
					parms->basename = g_path_get_basename( filename );
					output_xml_to_file(
							writer->private->buffer, filename, parms->messages ? &writer->private->messages : NULL );
					g_free( filename );
				}
			}

			g_free( writer->private->buffer );
		}

		g_object_unref( writer );
	}

	g_debug( "%s: returning code=%u", thisfn, code );
	return( code );
}

static xmlDocPtr
build_xml_doc( NAXMLWriter *writer )
{
	writer->private->doc = xmlNewDoc( BAD_CAST( "1.0" ));

	writer->private->root_node = xmlNewNode( NULL, BAD_CAST( writer->private->fn_str->root_node ));
	xmlDocSetRootElement( writer->private->doc, writer->private->root_node );

	na_ifactory_provider_write_item(
			NA_IFACTORY_PROVIDER( writer->private->provider ),
			writer,
			NA_IFACTORY_OBJECT( writer->private->exported ),
			writer->private->messages ? & writer->private->messages : NULL );

	return( writer->private->doc );
}

guint
naxml_writer_write_start( const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object, GSList **messages  )
{
	NAXMLWriter *writer;
	NADataGroup *groups;

	g_debug( "naxml_writer_write_start: object=%p (%s)", ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	if( NA_IS_OBJECT_ITEM( object )){
		na_object_dump( object );

		writer = NAXML_WRITER( writer_data );

		writer->private->list_node = xmlNewChild( writer->private->root_node, NULL, BAD_CAST( writer->private->fn_str->list_node ), NULL );

		if( writer->private->fn_str->write_list_attribs_fn ){
			( *writer->private->fn_str->write_list_attribs_fn )( writer, NA_OBJECT_ITEM( object ));
		}

		groups = na_ifactory_object_get_data_groups( object );
		write_type( writer, NA_OBJECT_ITEM( object ), groups );
	}

	return( NA_IIO_PROVIDER_CODE_OK );
}

guint
naxml_writer_write_data( const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object, const NADataBoxed *boxed, GSList **messages )
{
	NAXMLWriter *writer;

	/*NADataDef *def = na_data_boxed_get_data_def( boxed );
	g_debug( "naxml_writer_write_data: def=%s", def->name );*/

	/* do no export empty values
	 */
	if( na_data_boxed_is_set( boxed )){

		writer = NAXML_WRITER( writer_data );

		writer->private->schema_node = NULL;
		writer->private->locale_node = NULL;

		( *writer->private->fn_str->write_data_fn )( writer, NA_OBJECT_ID( object ), boxed );
	}

	return( NA_IIO_PROVIDER_CODE_OK );
}

guint
naxml_writer_write_done( const NAIFactoryProvider *provider, void *writer_data, const NAIFactoryObject *object, GSList **messages  )
{
	return( NA_IIO_PROVIDER_CODE_OK );
}

static void
write_data_schema_v1( NAXMLWriter *writer, const NAObjectId *object, const NADataBoxed *boxed )
{
	NADataDef *def;

	write_data_schema_v2( writer, object, boxed );

	def = na_data_boxed_get_data_def( boxed );

	write_data_schema_v1_element( writer, def );
}

static void
write_data_schema_v1_element( NAXMLWriter *writer, const NADataDef *def )
{
	if( !writer->private->locale_node ){
		writer->private->locale_node = xmlNewChild( writer->private->schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LOCALE ), NULL );
		xmlNewProp( writer->private->locale_node, BAD_CAST( "name" ), BAD_CAST( "C" ));
	}

	xmlNewChild( writer->private->schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_OWNER ), BAD_CAST( PACKAGE_TARNAME ));
	xmlNewChild( writer->private->locale_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LOCALE_SHORT ), BAD_CAST( def->short_label ));
	xmlNewChild( writer->private->locale_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LOCALE_LONG ), BAD_CAST( def->long_label ));
}

static void
write_type_schema_v1( NAXMLWriter *writer, const NAObjectItem *object, const NADataDef *def, const gchar *value )
{
	gchar *object_id;

	object_id = na_object_get_id( object );
	write_data_schema_v2_element( writer, def, object_id, value );
	write_data_schema_v1_element( writer, def );

	g_free( object_id );
}

/*
 * <schema>
 *  <key>/schemas/apps/caja-actions/configurations/entry</key>
 *  <applyto>/apps/caja-actions/configurations/item_id/profile_id/entry</applyto>
 */
static void
write_data_schema_v2( NAXMLWriter *writer, const NAObjectId *object, const NADataBoxed *boxed )
{
	gchar *object_id;
	NADataDef *def;
	gchar *value_str;

	def = na_data_boxed_get_data_def( boxed );
	value_str = na_data_boxed_get_as_string( boxed );

	/* boolean value must be lowercase
	 */
	if( def->type == NAFD_TYPE_BOOLEAN ){
		gchar *tmp = g_ascii_strdown( value_str, -1 );
		g_free( value_str );
		value_str = tmp;
	}

	object_id = na_object_get_id( object );

	if( NA_IS_OBJECT_PROFILE( object )){
		NAObjectItem *action = na_object_get_parent( object );
		gchar *id = na_object_get_id( action );
		gchar *tmp = g_strdup_printf( "%s/%s", id, object_id );
		g_free( id );
		g_free( object_id );
		object_id = tmp;
	}

	write_data_schema_v2_element( writer, def, object_id, value_str );

	g_free( value_str );
	g_free( object_id );
}

/*
 * <schema>
 *  <key>/schemas/apps/caja-actions/configurations/entry</key>
 *  <applyto>/apps/caja-actions/configurations/item_id/profile_id/entry</applyto>
 */
static void
write_data_schema_v2_element( NAXMLWriter *writer, const NADataDef *def, const gchar *object_id, const gchar *value_str )
{
	xmlChar *content;
	xmlNodePtr parent_value_node;

	writer->private->schema_node = xmlNewChild( writer->private->list_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE ), NULL );

	content = BAD_CAST( g_build_path( "/", NAGP_SCHEMAS_PATH, def->mateconf_entry, NULL ));
	xmlNewChild( writer->private->schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_KEY ), content );
	xmlFree( content );

	content = BAD_CAST( g_build_path( "/", NAGP_CONFIGURATIONS_PATH, object_id, def->mateconf_entry, NULL ));
	xmlNewChild( writer->private->schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_APPLYTO ), content );
	xmlFree( content );

	xmlNewChild( writer->private->schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_TYPE ), BAD_CAST( na_data_types_get_mateconf_dump_key( def->type )));
	if( def->type == NAFD_TYPE_STRING_LIST ){
		xmlNewChild( writer->private->schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LISTTYPE ), BAD_CAST( "string" ));
	}

	parent_value_node = writer->private->schema_node;

	if( def->localizable ){
		writer->private->locale_node = xmlNewChild( writer->private->schema_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_LOCALE ), NULL );
		xmlNewProp( writer->private->locale_node, BAD_CAST( "name" ), BAD_CAST( "C" ));
		parent_value_node = writer->private->locale_node;
	}

	content = xmlEncodeSpecialChars( writer->private->doc, BAD_CAST( value_str ));
	xmlNewChild( parent_value_node, NULL, BAD_CAST( NAXML_KEY_SCHEMA_NODE_DEFAULT ), content );
	xmlFree( content );
}

/*
 * <schema>
 *  <key>/schemas/apps/caja-actions/configurations/entry</key>
 *  <applyto>/apps/caja-actions/configurations/item_id/profile_id/entry</applyto>
 */
static void
write_type_schema_v2( NAXMLWriter *writer, const NAObjectItem *object, const NADataDef *def, const gchar *value )
{
	gchar *object_id;

	object_id = na_object_get_id( object );
	write_data_schema_v2_element( writer, def, object_id, value );

	g_free( object_id );
}

static void
write_list_attribs_dump( NAXMLWriter *writer, const NAObjectItem *object )
{
	gchar *id;
	gchar *path;

	id = na_object_get_id( object );
	path = g_build_path( "/", NAGP_CONFIGURATIONS_PATH, id, NULL );
	xmlNewProp( writer->private->list_node, BAD_CAST( NAXML_KEY_DUMP_LIST_PARM_BASE ), BAD_CAST( path ));

	g_free( path );
	g_free( id );
}

static void
write_data_dump( NAXMLWriter *writer, const NAObjectId *object, const NADataBoxed *boxed )
{
	gchar *entry;
	NADataDef *def;
	gchar *value_str;

	def = na_data_boxed_get_data_def( boxed );
	value_str = na_data_boxed_get_as_string( boxed );

	/* boolean value must be lowercase
	 */
	if( def->type == NAFD_TYPE_BOOLEAN ){
		gchar *tmp = g_ascii_strdown( value_str, -1 );
		g_free( value_str );
		value_str = tmp;
	}

	if( NA_IS_OBJECT_PROFILE( object )){
		gchar *id = na_object_get_id( object );
		entry = g_strdup_printf( "%s/%s", id, def->mateconf_entry );
		g_free( id );
	} else {
		entry = g_strdup( def->mateconf_entry );
	}

	write_data_dump_element( writer, def, boxed, entry, value_str );

	g_free( entry );
	g_free( value_str );
}

static void
write_data_dump_element( NAXMLWriter *writer, const NADataDef *def, const NADataBoxed *boxed, const gchar *entry, const gchar *value_str )
{
	xmlNodePtr entry_node;
	xmlNodePtr value_node;
	xmlNodePtr value_list_node, value_list_value_node;
	GSList *list, *is;
	xmlChar *encoded_content;

	entry_node = xmlNewChild( writer->private->list_node, NULL, BAD_CAST( writer->private->fn_str->element_node ), NULL );

	xmlNewChild( entry_node, NULL, BAD_CAST( NAXML_KEY_DUMP_NODE_KEY ), BAD_CAST( entry ));

	value_node = xmlNewChild( entry_node, NULL, BAD_CAST( NAXML_KEY_DUMP_NODE_VALUE ), NULL );

	if( def->type == NAFD_TYPE_STRING_LIST ){
		value_list_node = xmlNewChild( value_node, NULL, BAD_CAST( NAXML_KEY_DUMP_NODE_VALUE_LIST ), NULL );
		xmlNewProp( value_list_node, BAD_CAST( NAXML_KEY_DUMP_NODE_VALUE_LIST_PARM_TYPE ), BAD_CAST( NAXML_KEY_DUMP_NODE_VALUE_TYPE_STRING ));
		value_list_value_node = xmlNewChild( value_list_node, NULL, BAD_CAST( NAXML_KEY_DUMP_NODE_VALUE ), NULL );
		list = ( GSList * ) na_data_boxed_get_as_void( boxed );

		for( is = list ; is ; is = is->next ){
			encoded_content = xmlEncodeSpecialChars( writer->private->doc, BAD_CAST(( gchar * ) is->data ));
			xmlNewChild( value_list_value_node, NULL, BAD_CAST( NAXML_KEY_DUMP_NODE_VALUE_TYPE_STRING ), encoded_content );
			xmlFree( encoded_content );
		}

	} else {
		encoded_content = xmlEncodeSpecialChars( writer->private->doc, BAD_CAST( value_str ));
		xmlNewChild( value_node, NULL, BAD_CAST( na_data_types_get_mateconf_dump_key( def->type )), encoded_content );
		xmlFree( encoded_content );
	}
}

static void
write_type_dump( NAXMLWriter *writer, const NAObjectItem *object, const NADataDef *def, const gchar *value )
{
	write_data_dump_element( writer, def, NULL, def->mateconf_entry, value );
}

static ExportFormatFn *
find_export_format_fn( GQuark format )
{
	ExportFormatFn *found;
	ExportFormatFn *i;

	found = NULL;
	i = st_export_format_fn;

	while( i->format && !found ){
		if( g_quark_from_string( i->format ) == format ){
			found = i;
		}
		i++;
	}

	return( found );
}

/*
 * get_output_fname:
 * @item: the #NAObjectItme-derived object to be exported.
 * @folder: the URI of the directoy where to write the output XML file.
 * @format: the export format.
 *
 * Returns: a filename suitable for writing the output XML.
 *
 * As we don't want overwrite already existing files, the candidate
 * filename is incremented until we find an available filename.
 *
 * The returned string should be g_free() by the caller.
 *
 * Note that this function is always subject to race condition, as it
 * is possible, though unlikely, that the given file be created
 * between our test of inexistance and the actual write.
 */
static gchar *
get_output_fname( const NAObjectItem *item, const gchar *folder, GQuark format )
{
	static const gchar *thisfn = "naxml_writer_get_output_fname";
	gchar *item_id;
	gchar *canonical_fname = NULL;
	gchar *canonical_ext = NULL;
	gchar *candidate_fname;
	gint counter;

	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), NULL );
	g_return_val_if_fail( folder, NULL );
	g_return_val_if_fail( strlen( folder ), NULL );

	item_id = na_object_get_id( item );

	if( format == g_quark_from_string( NAXML_FORMAT_MATECONF_SCHEMA_V1 )){
		canonical_fname = g_strdup_printf( "config_%s", item_id );
		canonical_ext = g_strdup( "schemas" );

	} else if( format == g_quark_from_string( NAXML_FORMAT_MATECONF_SCHEMA_V2 )){
		canonical_fname = g_strdup_printf( "config-%s", item_id );
		canonical_ext = g_strdup( "schema" );

	} else if( format == g_quark_from_string( NAXML_FORMAT_MATECONF_ENTRY )){
		canonical_fname = g_strdup_printf( "%s-%s", NA_IS_OBJECT_ACTION( item ) ? "action" : "menu", item_id );
		canonical_ext = g_strdup( "xml" );

	} else {
		g_warning( "%s: unknown format: %s", thisfn, g_quark_to_string( format ));
	}

	g_free( item_id );
	g_return_val_if_fail( canonical_fname, NULL );

	candidate_fname = g_strdup_printf( "%s/%s.%s", folder, canonical_fname, canonical_ext );

	if( !na_core_utils_file_exists( candidate_fname )){
		g_free( canonical_fname );
		g_free( canonical_ext );
		return( candidate_fname );
	}

	for( counter = 0 ; ; ++counter ){
		g_free( candidate_fname );
		candidate_fname = g_strdup_printf( "%s/%s_%d.%s", folder, canonical_fname, counter, canonical_ext );
		if( !na_core_utils_file_exists( candidate_fname )){
			break;
		}
	}

	g_free( canonical_fname );
	g_free( canonical_ext );

	return( candidate_fname );
}

/*
 * output_xml_to_file:
 * @xml: the xml buffer.
 * @filename: the full path of the output filename as an URI.
 * @msg: a GSList to append messages.
 *
 * Exports an item to the given filename.
 */
static void
output_xml_to_file( const gchar *xml, const gchar *filename, GSList **msg )
{
	static const gchar *thisfn = "naxml_writer_output_xml_to_file";
	GFile *file;
	GFileOutputStream *stream;
	GError *error = NULL;
	gchar *errmsg;

	g_return_if_fail( xml );
	g_return_if_fail( filename && g_utf8_strlen( filename, -1 ));

	file = g_file_new_for_uri( filename );

	stream = g_file_replace( file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &error );
	if( error ){
		errmsg = g_strdup_printf( "%s: g_file_replace: %s", thisfn, error->message );
		g_warning( "%s", errmsg );
		if( msg ){
			*msg = g_slist_append( *msg, errmsg );
		}
		g_error_free( error );
		if( stream ){
			g_object_unref( stream );
		}
		g_object_unref( file );
		return;
	}

	g_output_stream_write( G_OUTPUT_STREAM( stream ), xml, g_utf8_strlen( xml, -1 ), NULL, &error );
	if( error ){
		errmsg = g_strdup_printf( "%s: g_output_stream_write: %s", thisfn, error->message );
		g_warning( "%s", errmsg );
		if( msg ){
			*msg = g_slist_append( *msg, errmsg );
		}
		g_error_free( error );
		g_object_unref( stream );
		g_object_unref( file );
		return;
	}

	g_output_stream_close( G_OUTPUT_STREAM( stream ), NULL, &error );
	if( error ){
		errmsg = g_strdup_printf( "%s: g_output_stream_close: %s", thisfn, error->message );
		g_warning( "%s", errmsg );
		if( msg ){
			*msg = g_slist_append( *msg, errmsg );
		}
		g_error_free( error );
		g_object_unref( stream );
		g_object_unref( file );
		return;
	}

	g_object_unref( stream );
	g_object_unref( file );
}

/* at end of write_start (list_node already created)
 * explicitly write the 'Type' node
 */
static void
write_type( NAXMLWriter *writer, NAObjectItem *object, const NADataGroup *groups )
{
	const NADataDef *def;
	const gchar *svalue;

	writer->private->schema_node = NULL;
	writer->private->locale_node = NULL;
	def = na_data_def_get_data_def( groups, NA_FACTORY_OBJECT_ITEM_GROUP, NAFO_DATA_TYPE );
	svalue = NA_IS_OBJECT_ACTION( object ) ? NAGP_VALUE_TYPE_ACTION : NAGP_VALUE_TYPE_MENU;

	( *writer->private->fn_str->write_type_fn )( writer, object, def, svalue );
}

static guint
writer_to_buffer( NAXMLWriter *writer )
{
	guint code;
	xmlDocPtr doc;
	xmlChar *text;
	int textlen;

	code = NA_IEXPORTER_CODE_OK;
	doc = build_xml_doc( writer );

	xmlDocDumpFormatMemoryEnc( doc, &text, &textlen, "UTF-8", 1 );
	writer->private->buffer = g_strdup(( const gchar * ) text );

	xmlFree( text );
	xmlFreeDoc (doc);
	xmlCleanupParser();

	return( code );
}
