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

#ifndef __CACT_ASSISTANT_EXPORT_H__
#define __CACT_ASSISTANT_EXPORT_H__

/**
 * SECTION: cact_assistant_export
 * @short_description: #CactAssistantExport class definition.
 * @include: cact/cact-assistant-export.h
 *
 * Rationale:
 *
 * Up to v 1.10.x, actions are exported as config_uuid.schemas. These
 * are actually full MateConf schema exports of the form :
 *  <schemalist>
 *   <schema>
 *    <key>/schemas/apps/caja-actions/..../uuid/label</key>
 *    <applyto>/apps/..../label</applyto>
 *
 * I don't know why Frederic had chosen to export as schema. But this
 * implies that :
 * - if all actions are imported via mateconftool-2 --install-schema-file,
 *   then the schema will be repeated once for each imported action (as
 *   schema is attached here to the uuid), which is obviously a waste
 *   of resources
 * - it seems that MateConfClient refuses to delete only the 'applyto' key
 *   when there is a corresponding schema key ..? (to be confirmed)
 *
 * Considering exporting schemas, we have two goals with this :
 * - make the files lighter
 * - keep the compatibility with previous mode
 *
 * which means exporting the minimal schema file which can be imported
 * with v1.10 and previous series., and via mateconftool-2 --install-schema-file.
 */

#include "base-assistant.h"

G_BEGIN_DECLS

#define CACT_TYPE_ASSISTANT_EXPORT                ( cact_assistant_export_get_type())
#define CACT_ASSISTANT_EXPORT( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, CACT_TYPE_ASSISTANT_EXPORT, CactAssistantExport ))
#define CACT_ASSISTANT_EXPORT_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, CACT_TYPE_ASSISTANT_EXPORT, CactAssistantExportClass ))
#define CACT_IS_ASSISTANT_EXPORT( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, CACT_TYPE_ASSISTANT_EXPORT ))
#define CACT_IS_ASSISTANT_EXPORT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), CACT_TYPE_ASSISTANT_EXPORT ))
#define CACT_ASSISTANT_EXPORT_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), CACT_TYPE_ASSISTANT_EXPORT, CactAssistantExportClass ))

typedef struct _CactAssistantExportPrivate        CactAssistantExportPrivate;

typedef struct {
	/*< private >*/
	BaseAssistant               parent;
	CactAssistantExportPrivate *private;
}
	CactAssistantExport;

typedef struct _CactAssistantExportClassPrivate   CactAssistantExportClassPrivate;

typedef struct {
	/*< private >*/
	BaseAssistantClass               parent;
	CactAssistantExportClassPrivate *private;
}
	CactAssistantExportClass;

GType cact_assistant_export_get_type( void );

void  cact_assistant_export_run( BaseWindow *parent );

G_END_DECLS

#endif /* __CACT_ASSISTANT_EXPORT_H__ */
