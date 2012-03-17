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
#include <libxml/tree.h>
#include <string.h>

#include <api/na-core-utils.h>
#include <api/na-mateconf-utils.h>
#include <api/na-data-types.h>
#include <api/na-ifactory-provider.h>
#include <api/na-object-api.h>

#include <io-mateconf/nagp-keys.h>

#include "naxml-keys.h"
#include "naxml-reader.h"

/* private class data
 */
struct NAXMLReaderClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* the association between a document root node key and the functions
 */
typedef struct {
	gchar     *root_key;
	gchar     *list_key;
	gchar     *element_key;
	gchar     *key_entry;
	guint      key_length;
	guint   ( *fn_root_parms )     ( NAXMLReader *, xmlNode * );
	guint   ( *fn_list_parms )     ( NAXMLReader *, xmlNode * );
	guint   ( *fn_element_parms )  ( NAXMLReader *, xmlNode * );
	guint   ( *fn_element_content )( NAXMLReader *, xmlNode * );
	gchar * ( *fn_get_value )      ( NAXMLReader *, xmlNode *, const NADataDef *def );
}
	RootNodeStr;

/* private instance data
 * main naxml_reader_import_from_uri() function is called once for each file
 * to import. We thus have one NAXMLReader object per import operation.
 */
struct NAXMLReaderPrivate {
	gboolean          dispose_has_run;

	/* data provided by the caller
	 */
	NAIImporter      *importer;
	NAIImporterUriParms *parms;

	/* data dynamically set during the import operation
	 */
	gboolean      type_found;
	GList        *nodes;
	RootNodeStr  *root_node_str;
	gchar        *item_id;

	/* following values are reset and reused while iterating on each
	 * element nodes of the imported item (cf. reset_node_data())
	 */
	gboolean      node_ok;
};

extern NAXMLKeyStr naxml_schema_key_schema_str[];
extern NAXMLKeyStr naxml_dump_key_entry_str[];

static GObjectClass *st_parent_class = NULL;

static GType         register_type( void );
static void          class_init( NAXMLReaderClass *klass );
static void          instance_init( GTypeInstance *instance, gpointer klass );
static void          instance_dispose( GObject *object );
static void          instance_finalize( GObject *object );

static NAXMLReader  *reader_new( void );

static guint         schema_parse_schema_content( NAXMLReader *reader, xmlNode *node );
static void          schema_check_for_id( NAXMLReader *reader, xmlNode *iter );
static void          schema_check_for_type( NAXMLReader *reader, xmlNode *iter );
static gchar        *schema_read_value( NAXMLReader *reader, xmlNode *node, const NADataDef *def );

static guint         dump_parse_list_parms( NAXMLReader *reader, xmlNode *node );
static guint         dump_parse_entry_content( NAXMLReader *reader, xmlNode *node );
static void          dump_check_for_type( NAXMLReader *reader, xmlNode *key_node );
static gchar        *dump_read_value( NAXMLReader *reader, xmlNode *node, const NADataDef *def );

static RootNodeStr st_root_node_str[] = {

	{ NAXML_KEY_SCHEMA_ROOT,
			NAXML_KEY_SCHEMA_LIST,
			NAXML_KEY_SCHEMA_NODE,
			NAXML_KEY_SCHEMA_NODE_APPLYTO,
			6,
			NULL,
			NULL,
			NULL,
			schema_parse_schema_content,
			schema_read_value },

	{ NAXML_KEY_DUMP_ROOT,
			NAXML_KEY_DUMP_LIST,
			NAXML_KEY_DUMP_NODE,
			NAXML_KEY_DUMP_NODE_KEY,
			1,
			NULL,
			dump_parse_list_parms,
			NULL,
			dump_parse_entry_content,
			dump_read_value },

	{ NULL }
};

#define ERR_ITEM_ID_ALREADY_EXISTS	_( "Item ID %s already exists." )
#define ERR_ITEM_ID_NOT_FOUND		_( "Item ID not found." )
#define ERR_MENU_UNWAITED			_( "Unwaited key path %s while importing a menu." )
#define ERR_NODE_ALREADY_FOUND		_( "Element %s at line %d already found, ignored." )
#define ERR_NODE_INVALID_ID			_( "Invalid item ID: waited for %s, found %s at line %d." )
#define ERR_NODE_UNKNOWN			_( "Unknown element %s found at line %d while waiting for %s." )
/* i18n: do not translate keywords 'Action' nor 'Menu' */
#define ERR_NODE_UNKNOWN_TYPE		_( "Unknown type %s found at line %d, while waiting for Action or Menu." )
#define ERR_ROOT_UNKNOWN			_( "Invalid XML root element %s found at line %d while waiting for %s." )
#define ERR_XMLDOC_UNABLE_TOPARSE	_( "Unable to parse XML file: %s." )
#define WARN_UNDEALT_NODE			_( "Node %s at line %d has not been dealt with." )

static gboolean      read_data_is_path_adhoc_for_object( NAXMLReader *reader, const NAIFactoryObject *object, xmlChar *text );
static NADataBoxed  *read_data_boxed_from_node( NAXMLReader *reader, xmlChar *text, xmlNode *parent, const NADataDef *def );
static void          read_done_object_action( NAXMLReader *reader, NAObjectAction *action );
static void          read_done_object_profile( NAXMLReader *reader, NAObjectProfile *profile );
static gchar        *read_done_get_next_profile_id( NAXMLReader *reader );
static void          read_done_load_profile( NAXMLReader *reader, const gchar *profile_id );

static guint         reader_parse_xmldoc( NAXMLReader *reader );
static guint         iter_on_root_children( NAXMLReader *reader, xmlNode *root );
static guint         iter_on_list_children( NAXMLReader *reader, xmlNode *first );

static void          add_message( NAXMLReader *reader, const gchar *format, ... );
static gchar        *build_key_node_list( NAXMLKeyStr *strlist );
static gchar        *build_root_node_list( void );
static gchar        *get_value_from_child_node( xmlNode *node, const gchar *child );
static gchar        *get_value_from_child_child_node( xmlNode *node, const gchar *first, const gchar *second );
static gboolean      is_profile_path( NAXMLReader *reader, xmlChar *text );
static guint         manage_import_mode( NAXMLReader *reader );
static void          publish_undealt_nodes( NAXMLReader *reader );
static void          renumber_label_item( NAXMLReader *reader );
static void          reset_node_data( NAXMLReader *reader );
static xmlNode      *search_for_child_node( xmlNode *node, const gchar *key );
static int           strxcmp( const xmlChar *a, const char *b );

GType
naxml_reader_get_type( void )
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
	static const gchar *thisfn = "naxml_reader_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NAXMLReaderClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAXMLReader ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NAXMLReader", &info, 0 );

	return( type );
}

static void
class_init( NAXMLReaderClass *klass )
{
	static const gchar *thisfn = "naxml_reader_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAXMLReaderClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "naxml_reader_instance_init";
	NAXMLReader *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_return_if_fail( NAXML_IS_READER( instance ));
	self = NAXML_READER( instance );

	self->private = g_new0( NAXMLReaderPrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->importer = NULL;
	self->private->parms = NULL;
	self->private->type_found = FALSE;
	self->private->nodes = NULL;
	self->private->root_node_str = NULL;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "naxml_reader_instance_dispose";
	NAXMLReader *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NAXML_IS_READER( object ));
	self = NAXML_READER( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		g_list_free( self->private->nodes );

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "naxml_reader_instance_finalize";
	NAXMLReader *self;

	g_debug( "%s: object=%p", thisfn, ( void * ) object );
	g_return_if_fail( NAXML_IS_READER( object ));
	self = NAXML_READER( object );

	g_free( self->private->item_id );

	reset_node_data( self );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static NAXMLReader *
reader_new( void )
{
	return( g_object_new( NAXML_READER_TYPE, NULL ));
}

/**
 * naxml_reader_import_uri:
 * @instance: the #NAIImporter provider.
 * @parms: a #NAIImporterUriParms structure.
 *
 * Imports an item.
 *
 * Returns: the import operation code.
 */
guint
naxml_reader_import_from_uri( const NAIImporter *instance, NAIImporterUriParms *parms )
{
	static const gchar *thisfn = "naxml_reader_import_from_uri";
	NAXMLReader *reader;
	guint code;

	g_debug( "%s: instance=%p, parms=%p", thisfn, ( void * ) instance, ( void * ) parms );

	g_return_val_if_fail( NA_IS_IIMPORTER( instance ), IMPORTER_CODE_PROGRAM_ERROR );

	reader = reader_new();
	reader->private->importer = ( NAIImporter * ) instance;
	reader->private->parms = parms;

	parms->imported = NULL;

	code = reader_parse_xmldoc( reader );

	if( code == IMPORTER_CODE_OK ){
		g_assert( NA_IS_OBJECT_ITEM( reader->private->parms->imported ));
		code = manage_import_mode( reader );
	}

	if( code != IMPORTER_CODE_OK ){
		if( reader->private->parms->imported ){
			g_object_unref( reader->private->parms->imported );
			reader->private->parms->imported = NULL;
		}
	}

	if( code == IMPORTER_CODE_OK ){
		publish_undealt_nodes( reader );
	}

	g_object_unref( reader );

	return( code );
}

/*
 * check that the file is a valid XML document
 * and that the root node can be identified as a schema or a dump
 */
static guint
reader_parse_xmldoc( NAXMLReader *reader )
{
	RootNodeStr *istr;
	gboolean found;
	guint code;

	xmlDoc *doc = xmlParseFile( reader->private->parms->uri );

	if( !doc ){
		xmlErrorPtr error = xmlGetLastError();
		add_message( reader,
				ERR_XMLDOC_UNABLE_TOPARSE, error->message );
		xmlResetError( error );
		code = IMPORTER_CODE_NOT_WILLING_TO;

	} else {
		xmlNode *root_node = xmlDocGetRootElement( doc );

		istr = st_root_node_str;
		found = FALSE;

		while( istr->root_key && !found ){
			if( !strxcmp( root_node->name, istr->root_key )){
				found = TRUE;
				reader->private->root_node_str = istr;
				code = iter_on_root_children( reader, root_node );
			}
			istr++;
		}

		if( !found ){
			gchar *node_list = build_root_node_list();
			add_message( reader,
						ERR_ROOT_UNKNOWN,
						( const char * ) root_node->name, root_node->line, node_list );
			g_free( node_list );
			code = IMPORTER_CODE_NOT_WILLING_TO;
		}

		xmlFreeDoc (doc);
	}

	xmlCleanupParser();
	return( code );
}

/*
 * parse a XML tree
 * - must have one child on the named 'first_child' key (others are warned)
 * - then iter on child nodes of this previous first named which must ne 'next_child'
 */
static guint
iter_on_root_children( NAXMLReader *reader, xmlNode *root )
{
	static const gchar *thisfn = "naxml_reader_iter_on_root_children";
	xmlNodePtr iter;
	gboolean found;
	guint code;

	g_debug( "%s: reader=%p, root=%p", thisfn, ( void * ) reader, ( void * ) root );

	code = IMPORTER_CODE_OK;

	/* deal with properties attached to the root node
	 */
	if( reader->private->root_node_str->fn_root_parms ){
		code = ( *reader->private->root_node_str->fn_root_parms )( reader, root );
	}

	/* iter through the first level of children (list)
	 * we must have only one occurrence of this first 'list' child
	 */
	found = FALSE;
	for( iter = root->children ; iter && code == IMPORTER_CODE_OK ; iter = iter->next ){

		if( iter->type != XML_ELEMENT_NODE ){
			continue;
		}

		if( strxcmp( iter->name, reader->private->root_node_str->list_key )){
			add_message( reader,
					ERR_NODE_UNKNOWN,
					( const char * ) iter->name, iter->line, reader->private->root_node_str->list_key );
			continue;
		}

		if( found ){
			add_message( reader, ERR_NODE_ALREADY_FOUND, ( const char * ) iter->name, iter->line );
			continue;
		}

		found = TRUE;
		code = iter_on_list_children( reader, iter );
	}

	return( code );
}

/*
 * iter on 'schema/entry' element nodes
 * each node should correspond to an elementary data of the imported item
 * other nodes are warned (and ignored)
 *
 * we have to iterate a first time through all nodes to be sure to find
 * a potential 'type' indication - this is needed in order to allocate an
 * action or a menu - if not found at the end of this first pass, we default
 * to allocate an action
 *
 * this first pass is also used to check nodes
 *
 * - for each node, check that
 *   > 'schema/entry' childs are in the list of known schema/entry child nodes
 *   > 'schema/entry' childs appear only once per node
 *     -> this requires a per-node 'found' flag which is reset for each node
 *   > schema has an 'applyto' child node
 *     -> only checkable at the end of the schema
 *
 * - check that each data, identified by the 'applyto' value, appears only once
 *   applyto node -> elementary data + id item + (optionally) id profile
 *   elementary data -> group (action,  menu, profile)
 *   -> this requires a 'found' flag for each group+data reset at item level
 *      as the item may not be allocated yet, we cannot check that data
 *      is actually relevant with the to-be-imported item
 *
 * each schema 'applyto' node let us identify a data and its value
 */
static guint
iter_on_list_children( NAXMLReader *reader, xmlNode *list )
{
	static const gchar *thisfn = "naxml_reader_iter_on_list_children";
	guint code;
	xmlNode *iter;

	g_debug( "%s: reader=%p, list=%p", thisfn, ( void * ) reader, ( void * ) list );

	code = IMPORTER_CODE_OK;

	/* deal with properties attached to the list node
	 */
	if( reader->private->root_node_str->fn_list_parms ){
		code = ( *reader->private->root_node_str->fn_list_parms )( reader, list );
	}

	/* each occurrence should correspond to an elementary data
	 * we run first to determine the type, and allocate the object
	 * we then rely on NAIFactoryProvider to actually read the data
	 */
	for( iter = list->children ; iter && code == IMPORTER_CODE_OK ; iter = iter->next ){

		if( iter->type != XML_ELEMENT_NODE ){
			continue;
		}

		if( strxcmp( iter->name, reader->private->root_node_str->element_key )){
			add_message( reader,
					ERR_NODE_UNKNOWN,
					( const char * ) iter->name, iter->line, reader->private->root_node_str->element_key );
			continue;
		}

		reset_node_data( reader );

		if( reader->private->root_node_str->fn_element_parms ){
			code = ( *reader->private->root_node_str->fn_element_parms )( reader, iter );
			if( code != IMPORTER_CODE_OK ){
				continue;
			}
		}

		if( reader->private->root_node_str->fn_element_content ){
			code = ( *reader->private->root_node_str->fn_element_content )( reader, iter );
			if( code != IMPORTER_CODE_OK ){
				continue;
			}
		}

		if( reader->private->node_ok ){
			reader->private->nodes = g_list_prepend( reader->private->nodes, iter );
		}
	}

	/* check that we have a not empty id
	 */
	if( code == IMPORTER_CODE_OK ){
		if( !reader->private->item_id || !strlen( reader->private->item_id )){
			add_message( reader, ERR_ITEM_ID_NOT_FOUND );
			code = 	IMPORTER_CODE_NO_ITEM_ID;
		}
	}

	/* if type not found, then suppose that we have an action
	 */
	if( code == IMPORTER_CODE_OK ){

		if( !reader->private->type_found ){
			reader->private->parms->imported = NA_OBJECT_ITEM( na_object_action_new());
		}
	}

	/* now load the data
	 */
	if( code == IMPORTER_CODE_OK ){

		na_object_set_id( reader->private->parms->imported, reader->private->item_id );

		na_ifactory_provider_read_item(
				NA_IFACTORY_PROVIDER( reader->private->importer ),
				reader,
				NA_IFACTORY_OBJECT( reader->private->parms->imported ),
				&reader->private->parms->messages );
	}

	return( code );
}

void
naxml_reader_read_start( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, GSList **messages  )
{
	static const gchar *thisfn = "naxml_reader_read_start";

	g_return_if_fail( NA_IS_IFACTORY_PROVIDER( provider ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	g_debug( "%s: provider=%p, reader_data=%p, object=%p (%s), messages=%p",
			thisfn,
			( void * ) provider,
			( void * ) reader_data,
			( void * ) object, G_OBJECT_TYPE_NAME( object ),
			( void * ) messages );
}

/*
 * this callback function is called by NAIFactoryObject once for each
 * serializable data for the object
 */
NADataBoxed *
naxml_reader_read_data( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, const NADataDef *def, GSList **messages )
{
	static const gchar *thisfn = "naxml_reader_read_data";
	xmlNode *parent_node;
	GList *ielt;

	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( provider ), NULL );
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	g_debug( "%s: reader_data=%p, object=%p (%s), data=%s",
			thisfn, ( void * ) reader_data, ( void * ) object, G_OBJECT_TYPE_NAME( object ), def->name );

	if( !def->mateconf_entry || !strlen( def->mateconf_entry )){
		g_warning( "%s: MateConf entry is not set for NADataDef %s", thisfn, def->name );
		return( NULL );
	}

	NADataBoxed *boxed = NULL;
	NAXMLReader *reader = NAXML_READER( reader_data );

	/*g_debug( "naxml_reader_read_data: nodes=%p (count=%d)",
				 ( void * ) reader->private->nodes, g_list_length( reader->private->nodes ));*/
	for( ielt = reader->private->nodes ; ielt && !boxed ; ielt = ielt->next ){

		parent_node = ( xmlNode * ) ielt->data;
		xmlNode *entry_node = search_for_child_node( parent_node, reader->private->root_node_str->key_entry );

		if( !entry_node ){
			g_warning( "%s: no '%s' child in node at line %u", thisfn, reader->private->root_node_str->key_entry, parent_node->line );

		} else {
			xmlChar *text = xmlNodeGetContent( entry_node );
			/*g_debug( "naxml_reader_read_data: trying %s", ( const gchar * ) text );*/

			if( read_data_is_path_adhoc_for_object( reader, object, text )){
				boxed = read_data_boxed_from_node( reader, text, parent_node, def );
			}

			xmlFree( text );
		}
	}

	if( boxed ){
		reader->private->nodes = g_list_remove( reader->private->nodes, parent_node );
	}

	return( boxed );
}

static gboolean
read_data_is_path_adhoc_for_object( NAXMLReader *reader, const NAIFactoryObject *object, xmlChar *text )
{
	gboolean adhoc;
	GSList *path_slist;
	guint path_length;
	gchar *node_profile_id;
	gchar *factory_profile_id;

	adhoc = TRUE;
	path_slist = na_core_utils_slist_from_split(( const gchar * ) text, "/" );
	path_length = g_slist_length( path_slist );

	if( NA_IS_OBJECT_ITEM( object )){
		if( path_length != reader->private->root_node_str->key_length ){
			adhoc = FALSE;
		}

	} else if( !is_profile_path( reader, text )){
		adhoc = FALSE;
		/*g_debug( "%s not adhoc as not profile path", ( const gchar * ) text );*/

	} else {
		gchar *key_dirname = g_path_get_dirname(( const gchar * ) text );
		node_profile_id = g_path_get_basename( key_dirname );
		g_free( key_dirname );

		factory_profile_id = na_object_get_id( object );

		if( strcmp( node_profile_id, factory_profile_id ) != 0 ){
			adhoc = FALSE;
			/*g_debug( "%s not adhoc (%s) as not searched profile %s",
					( const gchar * ) text, node_profile_id, factory_profile_id );*/
		}

		g_free( factory_profile_id );
		g_free( node_profile_id );
}

	na_core_utils_slist_free( path_slist );

	return( adhoc );
}

static NADataBoxed *
read_data_boxed_from_node( NAXMLReader *reader, xmlChar *text, xmlNode *parent, const NADataDef *def )
{
	NADataBoxed *boxed;
	gchar *entry;
	gchar *value;

	boxed = NULL;
	entry = g_path_get_basename(( const gchar * ) text );

	/*g_debug( "read_data_boxed_from_node: node_entry=%s def_mateconf=%s",
			entry, def->mateconf_entry );*/

	/* read the value
	 */
	if( !strcmp( entry, def->mateconf_entry )){

		if( reader->private->root_node_str->fn_get_value ){
			value = ( *reader->private->root_node_str->fn_get_value )( reader, parent, def );
			boxed = na_data_boxed_new( def );
			na_data_boxed_set_from_string( boxed, value );
			g_free( value );
		}
	}

	g_free( entry );

	return( boxed );
}

/*
 * all serializable data of the object has been readen
 */
void
naxml_reader_read_done( const NAIFactoryProvider *provider, void *reader_data, const NAIFactoryObject *object, GSList **messages  )
{
	static const gchar *thisfn = "naxml_reader_read_done";

	g_return_if_fail( NA_IS_IFACTORY_PROVIDER( provider ));
	g_return_if_fail( NA_IS_IFACTORY_OBJECT( object ));

	g_debug( "%s: provider=%p, reader_data=%p, object=%p (%s), messages=%p",
			thisfn,
			( void * ) provider,
			( void * ) reader_data,
			( void * ) object, G_OBJECT_TYPE_NAME( object ),
			( void * ) messages );

	if( NA_IS_OBJECT_ACTION( object )){
		read_done_object_action( NAXML_READER( reader_data ), NA_OBJECT_ACTION( object ));
	}

	if( NA_IS_OBJECT_PROFILE( object )){
		read_done_object_profile( NAXML_READER( reader_data ), NA_OBJECT_PROFILE( object ));
	}

	g_debug( "quitting naxml_read_done for %s at %p", G_OBJECT_TYPE_NAME( object ), ( void * ) object );
}

/*
 * if we have detected a pre-v2 action, then the action_read_done() function
 * has already allocated and define the corresponding profile
 * -> deals here with v2 and post, i.e. with profiles
 *
 * Also note that profiles order has been introduced in 2.29 serie
 */
static void
read_done_object_action( NAXMLReader *reader, NAObjectAction *action )
{
	GSList *order, *ip;
	gchar *profile_id;

	if( !na_object_get_items_count( reader->private->parms->imported )){

		/* first attach potential ordered profiles
		 */
		order = na_object_get_items_slist( reader->private->parms->imported );
		for( ip = order ; ip ; ip = ip->next ){
			read_done_load_profile( reader, ( const gchar * ) ip->data );
		}

		/* then attach unordered ones
		 */
		while( 1 ){
			profile_id = read_done_get_next_profile_id( reader );

			if( profile_id ){
				read_done_load_profile( reader, profile_id );
				g_free( profile_id );

			} else {
				break;
			}
		}
	}
}

static void
read_done_object_profile( NAXMLReader *reader, NAObjectProfile *profile )
{
	na_object_attach_profile( reader->private->parms->imported, profile );
}

/*
 * return the first profile id found in the nodes
 */
static gchar *
read_done_get_next_profile_id( NAXMLReader *reader )
{
	gchar *profile_id;
	GList *ip;

	profile_id = NULL;

	/*g_debug( "read_done_get_next_profile_id: nodes=%p (count=%d)",
			( void * ) reader->private->nodes, g_list_length( reader->private->nodes ));*/

	for( ip = reader->private->nodes ; ip && !profile_id ; ip = ip->next ){
		xmlNode *parent_node = ( xmlNode * ) ip->data;
		xmlNode *entry_node = search_for_child_node( parent_node, reader->private->root_node_str->key_entry );
		xmlChar *text = xmlNodeGetContent( entry_node );

		/*g_debug( "text=%s, is_profile=%s",
					( const gchar * ) text, is_profile_path( reader, text ) ? "True":"False" );*/

		if( is_profile_path( reader, text )){
			gchar *name = g_path_get_dirname(( const gchar * ) text );
			profile_id = g_path_get_basename( name );
			g_free( name );

			if( na_object_get_item( reader->private->parms->imported, profile_id )){
				g_free( profile_id );
				profile_id = NULL;
			}
		}

		xmlFree( text );
	}

	return( profile_id );
}

static void
read_done_load_profile( NAXMLReader *reader, const gchar *profile_id )
{
	/*g_debug( "naxml_reader_read_done_load_profile: profile_id=%s", profile_id );*/

	NAObjectProfile *profile = na_object_profile_new();

	na_object_set_id( profile, profile_id );

	na_ifactory_provider_read_item(
			NA_IFACTORY_PROVIDER( reader->private->importer ),
			reader,
			NA_IFACTORY_OBJECT( profile ),
			&reader->private->parms->messages );
}

/*
 * 'key' and 'applyto' keys: check the id
 * 'applyto' key: check for type
 * returns set node_ok if:
 * - each key appears is known and appears only once
 * - there is an applyto key
 */
static guint
schema_parse_schema_content( NAXMLReader *reader, xmlNode *schema )
{
	xmlNode *iter;
	NAXMLKeyStr *str;
	int i;
	guint code;

	code = IMPORTER_CODE_OK;

	for( iter = schema->children ; iter && code == IMPORTER_CODE_OK ; iter = iter->next ){

		if( iter->type != XML_ELEMENT_NODE ){
			continue;
		}

		str = NULL;
		for( i = 0 ; naxml_schema_key_schema_str[i].key && !str ; ++i ){
			if( !strxcmp( iter->name, naxml_schema_key_schema_str[i].key )){
				str = naxml_schema_key_schema_str+i;
			}
		}

		if( !str ){
			gchar *node_list = build_key_node_list( naxml_schema_key_schema_str );
			add_message( reader,
					ERR_NODE_UNKNOWN,
					( const char * ) iter->name, iter->line, node_list );
			g_free( node_list );
			reader->private->node_ok = FALSE;
			continue;
		}

		if( str->reader_found ){
			add_message( reader,
					ERR_NODE_ALREADY_FOUND,
					( const char * ) iter->name, iter->line );
			reader->private->node_ok = FALSE;
			continue;
		}

		str->reader_found = TRUE;

		/* set the item id the first time, check after
		 */
		if( !strxcmp( iter->name, NAXML_KEY_SCHEMA_NODE_KEY ) ||
			!strxcmp( iter->name, NAXML_KEY_SCHEMA_NODE_APPLYTO )){

			schema_check_for_id( reader, iter );

			if( !reader->private->node_ok ){
				continue;
			}
		}

		/* search for the type of the item
		 */
		if( !strxcmp( iter->name, NAXML_KEY_SCHEMA_NODE_APPLYTO )){

			schema_check_for_type( reader, iter );

			if( !reader->private->node_ok ){
				continue;
			}
		}
	}

	return( code );
}

/*
 * check the id on 'key' and 'applyto' keys
 */
static void
schema_check_for_id( NAXMLReader *reader, xmlNode *iter )
{
	guint idx = 0;

	if( !strxcmp( iter->name, NAXML_KEY_SCHEMA_NODE_KEY )){
		idx = 1;
	}

	xmlChar *text = xmlNodeGetContent( iter );
	gchar **path_elts = g_strsplit(( const gchar * ) text, "/", -1 );
	gchar *id = g_strdup( path_elts[ reader->private->root_node_str->key_length+idx-2 ] );
	g_strfreev( path_elts );
	xmlFree( text );

	if( reader->private->item_id ){
		if( strcmp( reader->private->item_id, id ) != 0 ){
			add_message( reader,
					ERR_NODE_INVALID_ID,
					reader->private->item_id, id, iter->line );
			reader->private->node_ok = FALSE;
		}
	} else {
		reader->private->item_id = g_strdup( id );
	}

	g_free( id );
}

/*
 * check 'applyto' key for 'Type'
 */
static void
schema_check_for_type( NAXMLReader *reader, xmlNode *iter )
{
	xmlChar *text = xmlNodeGetContent( iter );

	gchar *entry = g_path_get_basename(( const gchar * ) text );

	if( !strcmp( entry, NAGP_ENTRY_TYPE )){
		reader->private->type_found = TRUE;
		gchar *type = get_value_from_child_node( iter->parent, NAXML_KEY_SCHEMA_NODE_DEFAULT );

		if( !strcmp( type, NAGP_VALUE_TYPE_ACTION )){
			reader->private->parms->imported = NA_OBJECT_ITEM( na_object_action_new());

		} else if( !strcmp( type, NAGP_VALUE_TYPE_MENU )){
			reader->private->parms->imported = NA_OBJECT_ITEM( na_object_menu_new());

		} else {
			add_message( reader, ERR_NODE_UNKNOWN_TYPE, type, iter->line );
			reader->private->node_ok = FALSE;
		}

		g_free( type );
	}

	g_free( entry );
	xmlFree( text );
}

static gchar *
schema_read_value( NAXMLReader *reader, xmlNode *node, const NADataDef *def )
{
	gchar *value;

	if( def->localizable ){
		value = get_value_from_child_child_node( node, NAXML_KEY_SCHEMA_NODE_LOCALE, NAXML_KEY_SCHEMA_NODE_LOCALE_DEFAULT );
	} else {
		value = get_value_from_child_node( node, NAXML_KEY_SCHEMA_NODE_DEFAULT );
	}

	return( value );
}

/*
 * first run: do nothing
 * second run: get the id
 */
static guint
dump_parse_list_parms( NAXMLReader *reader, xmlNode *node )
{
	guint code;

	code = IMPORTER_CODE_OK;

	xmlChar *path = xmlGetProp( node, ( const xmlChar * ) NAXML_KEY_DUMP_LIST_PARM_BASE );
	reader->private->item_id = g_path_get_basename(( const gchar * ) path );
	xmlFree( path );

	return( code );
}

/*
 * first_run: only search for a 'Type' key, and allocate the item
 * second run: load data
 */
static guint
dump_parse_entry_content( NAXMLReader *reader, xmlNode *entry )
{
	xmlNode *iter;
	NAXMLKeyStr *str;
	int i;
	guint code;

	code = IMPORTER_CODE_OK;

	for( iter = entry->children ; iter && code == IMPORTER_CODE_OK ; iter = iter->next ){

		if( iter->type != XML_ELEMENT_NODE ){
			continue;
		}

		str = NULL;
		for( i = 0 ; naxml_dump_key_entry_str[i].key && !str ; ++i ){
			if( !strxcmp( iter->name, naxml_dump_key_entry_str[i].key )){
				str = naxml_dump_key_entry_str+i;
			}
		}

		if( !str ){
			gchar *node_list = build_key_node_list( naxml_dump_key_entry_str );
			add_message( reader,
					ERR_NODE_UNKNOWN,
					( const char * ) iter->name, iter->line, node_list );
			g_free( node_list );
			reader->private->node_ok = FALSE;
			continue;
		}

		if( str->reader_found ){
			add_message( reader,
					ERR_NODE_ALREADY_FOUND,
					( const char * ) iter->name, iter->line );
			reader->private->node_ok = FALSE;
			continue;
		}

		str->reader_found = TRUE;

		/* search for the type of the item
		 */
		if( !strxcmp( iter->name, NAXML_KEY_DUMP_NODE_KEY )){

			dump_check_for_type( reader, iter );

			if( !reader->private->node_ok ){
				continue;
			}
		}
	}

	return( code );
}

/*
 * check for 'Type'
 */
static void
dump_check_for_type( NAXMLReader *reader, xmlNode *key_node )
{
	xmlChar *key_content = xmlNodeGetContent( key_node );

	if( !strxcmp( key_content, NAGP_ENTRY_TYPE )){
		reader->private->type_found = TRUE;
		gchar *type = get_value_from_child_child_node( key_node->parent, NAXML_KEY_DUMP_NODE_VALUE, NAXML_KEY_DUMP_NODE_VALUE_TYPE_STRING );

		if( !strcmp( type, NAGP_VALUE_TYPE_ACTION )){
			reader->private->parms->imported = NA_OBJECT_ITEM( na_object_action_new());

		} else if( !strcmp( type, NAGP_VALUE_TYPE_MENU )){
			reader->private->parms->imported = NA_OBJECT_ITEM( na_object_menu_new());

		} else {
			add_message( reader, ERR_NODE_UNKNOWN_TYPE, type, key_node->line );
			reader->private->node_ok = FALSE;
		}

		g_free( type );
	}

	xmlFree( key_content );
}

/*
 * string list is converted to GSList, then to a MateConf string
 */
static gchar *
dump_read_value( NAXMLReader *reader, xmlNode *node, const NADataDef *def )
{
	gchar *string;
	GSList *slist;
	xmlNode *value_node;
	xmlNode *list_node;
	xmlNode *vv_node;
	xmlChar *text;
	xmlNode *it;

	string = NULL;

	switch( def->type ){
		case NAFD_TYPE_STRING:
		case NAFD_TYPE_LOCALE_STRING:
		case NAFD_TYPE_UINT:
		case NAFD_TYPE_BOOLEAN:
			string = get_value_from_child_child_node( node, NAXML_KEY_DUMP_NODE_VALUE, NAXML_KEY_DUMP_NODE_VALUE_TYPE_STRING );
			break;

		case NAFD_TYPE_STRING_LIST:
			slist = NULL;
			value_node = search_for_child_node( node, NAXML_KEY_DUMP_NODE_VALUE );

			if( value_node ){
				list_node = search_for_child_node( value_node, NAXML_KEY_DUMP_NODE_VALUE_LIST );

				if( list_node ){
					vv_node = search_for_child_node( list_node, NAXML_KEY_DUMP_NODE_VALUE );

					for( it = vv_node->children ; it ; it = it->next ){

						if( it->type == XML_ELEMENT_NODE &&
								!strxcmp( it->name, NAXML_KEY_DUMP_NODE_VALUE_TYPE_STRING )){

							text = xmlNodeGetContent( it );
							slist = g_slist_append( slist, ( gchar * ) text );
						}

					}
				}
			}
			string = na_mateconf_utils_slist_to_string( slist );
			na_core_utils_slist_free( slist );
			break;

		case NAFD_TYPE_POINTER:
		default:
			break;
	}

	return( string );
}

static void
add_message( NAXMLReader *reader, const gchar *format, ... )
{
	va_list va;
	gchar *tmp;

	va_start( va, format );
	tmp = g_markup_vprintf_escaped( format, va );
	va_end( va );

	reader->private->parms->messages = g_slist_append( reader->private->parms->messages, tmp );
}

static gchar *
build_key_node_list( NAXMLKeyStr *strlist )
{
	NAXMLKeyStr *next;

	NAXMLKeyStr *istr = strlist;
	GString *string = g_string_new( "" );

	while( istr->key ){
		next = istr+1;
		if( string->len ){
			if( next->key ){
				string = g_string_append( string, ", " );
			} else {
				string = g_string_append( string, " or " );
			}
		}
		string = g_string_append( string, istr->key );
		istr++;
	}

	return( g_string_free( string, FALSE ));
}

static gchar *
build_root_node_list( void )
{
	RootNodeStr *next;

	RootNodeStr *istr = st_root_node_str;
	GString *string = g_string_new( "" );

	while( istr->root_key ){
		next = istr+1;
		if( string->len ){
			if( next->root_key ){
				string = g_string_append( string, ", " );
			} else {
				string = g_string_append( string, " or " );
			}
		}
		string = g_string_append( string, istr->root_key );
		istr++;
	}

	return( g_string_free( string, FALSE ));
}

static gchar *
get_value_from_child_node( xmlNode *node, const gchar *child )
{
	gchar *value = NULL;

	xmlNode *value_node = search_for_child_node( node, child );
	if( value_node ){
		xmlChar *value_value = xmlNodeGetContent( value_node );
		if( value_value ){
			value = g_strdup(( const char * ) value_value );
			xmlFree( value_value );
		}
	}

	return( value );
}

static gchar *
get_value_from_child_child_node( xmlNode *node, const gchar *first, const gchar *second )
{
	gchar *value = NULL;

	xmlNode *first_node = search_for_child_node( node, first );
	if( first_node ){
		xmlNode *second_node = search_for_child_node( first_node, second );
		if( second_node ){
			xmlChar *value_value = xmlNodeGetContent( second_node );
			if( value_value ){
				value = g_strdup(( const char * ) value_value );
				xmlFree( value_value );
			}
		}
	}

	return( value );
}

static gboolean
is_profile_path( NAXMLReader *reader, xmlChar *text )
{
	gboolean is_profile;
	GSList *path_slist;
	guint path_length;

	path_slist = na_core_utils_slist_from_split(( const gchar * ) text, "/" );
	path_length = g_slist_length( path_slist );

	is_profile = ( path_length == 1+reader->private->root_node_str->key_length );

	na_core_utils_slist_free( path_slist );

	return( is_profile );
}

/*
 * returns IMPORTER_CODE_OK if we can safely insert the action
 * - the id doesn't already exist
 * - the id already exist, but import mode is renumber
 * - the id already exists, but import mode is override
 *
 * returns IMPORTER_CODE_CANCELLED if user chooses to cancel the operation
 */
static guint
manage_import_mode( NAXMLReader *reader )
{
	guint code;
	NAObjectItem *exists;
	guint mode;
	gchar *id;

	code = IMPORTER_CODE_OK;
	exists = NULL;

	if( reader->private->parms->check_fn ){
		exists = ( *reader->private->parms->check_fn )
						( reader->private->parms->imported, reader->private->parms->check_fn_data );
	}

	if( exists ){
		if( reader->private->parms->mode == IMPORTER_MODE_ASK ){
			mode = na_iimporter_ask_user( reader->private->importer, reader->private->parms, exists );

		} else {
			mode = reader->private->parms->mode;
		}

		switch( mode ){
			case IMPORTER_MODE_RENUMBER:
				renumber_label_item( reader );
				if( reader->private->parms->mode == IMPORTER_MODE_ASK ){
					add_message( reader, "%s", _( "Item was renumbered due to user request." ));
				}
				break;

			case IMPORTER_MODE_OVERRIDE:
				if( reader->private->parms->mode == IMPORTER_MODE_ASK ){
					add_message( reader, "%s", _( "Existing item was overriden due to user request." ));
				}
				break;

			case IMPORTER_MODE_NO_IMPORT:
			default:
				id = na_object_get_id( reader->private->parms->imported );
				add_message( reader, ERR_ITEM_ID_ALREADY_EXISTS, id );
				if( reader->private->parms->mode == IMPORTER_MODE_ASK ){
					add_message( reader, "%s", _( "Import was canceled due to user request." ));
				}
				g_free( id );
				code = IMPORTER_CODE_CANCELLED;
		}
	}

	return( code );
}

static void
publish_undealt_nodes( NAXMLReader *reader )
{
	GList *iter;
	xmlChar *text;

	g_debug( "naxml_reader_publish_undealt_nodes: count=%d", g_list_length( reader->private->nodes ));

	for( iter = reader->private->nodes ; iter ; iter = iter->next ){
		xmlNode *node = ( xmlNode * ) iter->data;
		text = xmlNodeGetContent( node );
		add_message( reader, WARN_UNDEALT_NODE, ( const gchar * ) text, node->line );
		xmlFree( text );
	}
}

/*
 * renumber the item, and set a new label
 */
static void
renumber_label_item( NAXMLReader *reader )
{
	gchar *label, *tmp;

	na_object_set_new_id( reader->private->parms->imported, NULL );

	label = na_object_get_label( reader->private->parms->imported );

	/* i18n: the action has been renumbered during import operation */
	tmp = g_strdup_printf( "%s %s", label, _( "(renumbered)" ));

	na_object_set_label( reader->private->parms->imported, tmp );

	g_free( tmp );
	g_free( label );
}

/*
 * data are reset before first run on nodes for an item
 */
static void
reset_node_data( NAXMLReader *reader )
{
	int i;

	for( i=0 ; naxml_schema_key_schema_str[i].key ; ++i ){
		naxml_schema_key_schema_str[i].reader_found = FALSE;
	}

	for( i=0 ; naxml_dump_key_entry_str[i].key ; ++i ){
		naxml_dump_key_entry_str[i].reader_found = FALSE;
	}

	reader->private->node_ok = TRUE;
}

static xmlNode *
search_for_child_node( xmlNode *node, const gchar *key )
{
	xmlNode *iter;

	for( iter = node->children ; iter ; iter = iter->next ){
		if( iter->type == XML_ELEMENT_NODE ){
			if( !strxcmp( iter->name, key )){
				return( iter );
			}
		}
	}

	return( NULL );
}

/*
 * note that up to v 1.10 included, key check was made via a call to
 * g_ascii_strncasecmp, which was doubly wrong:
 * - because XML is case sensitive by definition
 * - because this did not detect a key longer that the reference.
 */
static int
strxcmp( const xmlChar *a, const char *b )
{
	xmlChar *xb = xmlCharStrdup( b );
	int ret = xmlStrcmp( a, xb );
	xmlFree( xb );
	return( ret );
}
